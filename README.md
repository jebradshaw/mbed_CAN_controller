# mbed_CAN_controller
mbed CAN bus controller example. Uses CAN bus to perform closed loop control by reading angular position from a BNO055 IMU sensor and transmitting servo commands "sXXXX\r" where XXXX is a pulse width in microseconds. The controller can also receive set points
