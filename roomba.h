// *********************************************************************************************************************
// $Header: /home/lulu/cvsroot/roomba/roomba.h,v 1.14 2016/02/22 19:30:39 lulu Exp $
// *********************************************************************************************************************

#include <stdlib.h>
#include <curses.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <curses.h>
#include <sys/time.h>
#include <math.h>

#define BAUDRATE 115200
//#define BAUDRATE 19200
//Bluetooth serail connection device
//#define MODEMDEVICE "/dev/rfcomm3"

//USB Serial device 
#define MODEMDEVICE "/dev/ttyUSB0"
#define FALSE 0
#define TRUE 1

#define BUMPER 0x03
//Need a calibration function / matrix [speed,radius_spin]-[speed_left,speed_right,HALF_WHEEL_SPACING_SPIN]
//Calibration speed/radius 150 mm/sec-600 mm=> [150,60]-[150,150,12.5] => Very Rough error estimate 
// To avoid calibration => Would need kind of beacons and measuring power signals : parallax 
// Kind of GPS sensor (bad inside house) / Loran (not on sea side) / IR Light House (requires direct visibility on short distance) 
#define HALF_WHEEL_SPACING 11.7
#define HALF_WHEEL_SPACING_SPIN 12.5   

#define GO(X) go(fd,(unsigned char *) &X,sizeof(X));fprintf(roombascript,"delta_t=%d command=%s\n",delta_cmd, #X)
#define SINGING(X) printf(#X "\n");go(fd,(char *) &X,sizeof(X));
#define SMS(X);go(fd,(char *) &X,sizeof(X));

#define FRONT printf("front\n ")
#define BACK  printf("back\n ")
#define SPINLEFT printf("spinleft\n ")
#define SPINRIGHT printf("spinright\n ")
#define TURNFLEFT printf("turn front left\n ")
#define TURNFRIGHT printf("turn front right\n ")
#define TURNBLEFT printf("turn back left\n ")
#define TURNBRIGHT printf("turn back right\n ")

typedef union S2c {
	           char ch[1];
	 	   short sh;
        	 } S2c;

typedef struct Position {
	char cmd[255];
  	float x;
	float y;	
	float distance;
	float angle;
	int azimut;
	short speed_left;
	short speed_right;
	short speed_avg;
	short radius;
	char bumper;
 	short charging_state;
 	short charge_level;
	short wall;
   	short wall_power;
	short vwall;
	short vwall_power;	
	char cliff_left;
	short cliff_left_power;
	char cliff_front_left;
	short cliff_front_left_power;
	char cliff_front_right;
	short cliff_front_right_power;
	char cliff_right;
	short cliff_right_power;
	char infrared;

} Position;
