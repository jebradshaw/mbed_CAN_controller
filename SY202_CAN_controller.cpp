// Program to test the CAN bus using pins 29 and 30 on the mbed connected
// to an MCP2551 CAN transceiver bus IC
//  Note that this program will only continue to transmit while the TX message is <= 8 bytes long

#include "mbed.h"

#define THIS_CAN_ID     3   //Address of this CAN device
#define DEST_CAN_ID     1   //Address of destination

Serial pc(USBTX, USBRX);    //tx, and rx for tera term
DigitalOut led1(LED1);      //heartbeat
DigitalOut led2(LED2);      //CAN read activity
DigitalOut led3(LED3);      //CAN write activity
CAN can(p30,p29);      //CAN interface
Ticker pulse;

float set_point_r=0.0;

void alive(void){
    led1 = !led1;
    if(led1)
        pulse.attach(&alive, .2); // the address of the function to be attached (flip) and the interval (2 seconds)     
    else
        pulse.attach(&alive, 1.3); // the address of the function to be attached (flip) and the interval (2 seconds)
}

int CAN_to_Str(CANMessage msg, float *fltVal){
    char tempStr[8];
    int i=0;
    //pc.printf("can to str %s", msg.data);
    do{
        tempStr[i] = (char)msg.data[i];
        i++;
    }while((tempStr[i] != '/0' || tempStr[i] != '/r' || tempStr[i] != '/n') && (i < 8));
    tempStr[i] = 0;
    float tempValF;
    int err = sscanf(tempStr, "b%f", &tempValF);
    *fltVal = tempValF;
    if(err>0)
        return 0;
    else
        return -1;
}

int main() {
    CANMessage msg_read;    
    CANMessage msg_write;
    int servoPulse=0;
    char msg_send[20];
    wait(.2);
    
    pulse.attach(&alive, 2.0); // the address of the function to be attached (alive) and the interval (2 seconds)
    msg_read.len = 64;
    msg_write.len = 64;
    can.frequency(500000);
    pc.baud(115200);    
            
    pc.printf("%s\r\n", __FILE__);
    pc.printf("Enter ServoPulse in microseconds: ");
    int pulseAng = 1500;
    while(1) {
        
        while(can.read(msg_read)){ //if message is available, read into msg
            if(msg_read.id == THIS_CAN_ID){
                if(msg_read.data[0] == 'b'){
                    pc.printf("Message Read: %s   ",  msg_read.data);
                    float angle;
                    int err = CAN_to_Str(msg_read, &angle);
                    pc.printf("err=%d  angle = %.3f\r\n", err, angle);
                    
                    float error = set_point_r - angle;
                    pc.printf("error = %f\r\n", error);
                    
                    pulseAng += (int)(-error * 250.0); 
                    
                    sprintf(msg_send, "s%d\r", pulseAng);    //format output message string
    
                    if(can.write(CANMessage(DEST_CAN_ID, msg_send, strlen(msg_send)))){                
                        pc.printf("\r\nMessage Sent: %s\r\n", msg_send);
                        led3 = !led3;
                    }
                    else{
                        can.reset();
                    }
                    wait(.02);                   
                }
            }
            led2 = !led2;
        } 
                
        if(pc.readable()){
            char c = pc.getc();
            
            //get servo pulse
            if(c == 'v' || c == 'V'){
                pc.scanf("%d", &servoPulse);
                pc.printf("%d\r\n", servoPulse);
                sprintf(msg_send, "s%d", servoPulse);    //format output message string
    
                if(can.write(CANMessage(DEST_CAN_ID, msg_send, strlen(msg_send)))){                
                    pc.printf("\r\nMessage Sent: %s\r\n", msg_send);
                    led3 = !led3;
                }
                else{
                    can.reset();
                }
            }
            
            //get set point in radians
            if(c == 's' || c == 'S'){                
                pc.scanf("%f", &set_point_r);
                pc.printf("Set Point = %f\r\n", set_point_r);
            }            
            
            while(pc.readable()){
                char c = pc.getc();    
            }
        }                                       
    }//while(1)
}//main
