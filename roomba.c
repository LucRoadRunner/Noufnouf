// **********************************************************************
// $Header: /home/lulu/cvsroot/roomba/roomba.c,v 1.22 2016/02/22 19:30:39 lulu Exp $
// **********************************************************************
// Roomba server to receive commands via keyboard / socket port 5678 and transmit to roomba
// Should work on INTEL x86 or ARM architecture 
// Tested on PC Pentium 4 and Raspberry Pi Model B.

#include <roomba.h>
struct timeval stimeout;
struct timeval t[3],beacon;
static Position position,delta;
struct sockaddr_in serv_addr, cli_addr;
struct termios oldtio,oldtioo,newtio;
struct termios stdinio;

static int i,j,k,n,possms=0,counter=0;
static int flag_sensor=0;
static int fd,fdmax;
static int slct;
static int chksum;
static int sockfd, newsockfd,clilen;
static int port=5678;

static short nbpackets;
static signed short charge_level,charge_max;

fd_set readfds,writefds;

static long delta_t,delta_cmd,pausep;
static signed short speed_left,speed_right;
static unsigned speed_avg=150; //150 mm/sec
static unsigned radius=60; //60 cm
static S2c speed;

static unsigned char buffer[1024];
static unsigned char *pbyte;
static  unsigned char radius_fleft[5];
static  unsigned char radius_fright[5];
static  unsigned char radius_bright[5];
static  unsigned char radius_bleft[5];
static  unsigned char spinleft[5];
static  unsigned char spinright[5];
static  unsigned char front[5];
static  unsigned char back[5];
static  unsigned char stop[]={145,0,0,0,0};
static  unsigned char start[]={128};
static  unsigned char baud[]={129,11};
static  unsigned char passive[]={128};
static  unsigned char full[]={132};
static  unsigned char safe[]={131};
static  unsigned char dock[]={143};
static  unsigned char led[]={139,8,255,255};
static  unsigned char start_sensor[]={150,1};
static  unsigned char stop_sensor[]={150,0};
static  unsigned char stream_sensor[]={148,7,41,42,8,21,25,26,27};
static  unsigned char single_sensor[]={142,6};
static  unsigned char reset[]={7};
static  unsigned char schedule[]={167,20,0,0,0,0,8,15,0,0,8,30,0,0,0,0};
static  unsigned char daytime[]={168,1,20,19};
static  unsigned char display[]={164,72,69,76,79};
static  unsigned char song[]={141,5};
static  unsigned char brosse_l[]={144,0,64,0};
static  unsigned char brosse_off[]={144,0,0,0};
static  char brosse_r[]={144,0,-64,0};
static  unsigned char digits[5];
static  unsigned char sms[]="    Hello  \0";

FILE *roombalog,*roombascript;


// Wakeup through BlueTooth 
//void roomba_wakeup(int fd) {
// write(fd,"$$$",3);
// tcflush(fd,TCIFLUSH);
// sleep(1);
// write(fd,"S@,8080\r",8);
// usleep(500000);
// write(fd,"S&,8000\r",8);
// usleep(500000);
// write(fd,"S&,8080\r",8);
// sleep(1);
// write(fd,"---\r",4);
// tcflush(fd,TCIFLUSH);
//}


// Wakeup through direct Serial port on USB

void roomba_wakeup(int fd) {
 int status;
 if (ioctl(fd, TIOCMGET, &status) == -1)
 {
  printf("Nobody to wake up ! :-(\n");  
  exit(1);
 }
 else
 {
  printf("RS232 Get Status = %d \n",status);


  status=0  ; ioctl(fd,TIOCMSET,&status); usleep(500000);
  printf("RS232 Status = %d \n",status);

  status=status | TIOCM_DTR ; ioctl(fd,TIOCMSET,&status); usleep(500000);
  printf("RS232 Status = %d \n",status);

  status=0  ; ioctl(fd,TIOCMSET,&status); usleep(500000);
  printf("RS232 Status = %d \n",status);

  status=status | TIOCM_DTR ; ioctl(fd,TIOCMSET,&status); usleep(500000);
  printf("RS232 Status = %d \n",status);



/*  status=0 ;
  ioctl(fd,TIOCMSET,&status);
  usleep(100000);
  printf("RS232 Status = %d \n",status);
*/

  printf("Wakey Wakey !!! :-) \n");
   }
}


char* bytecopy(char* dest, char* src,short length) {
 short i;
 for (i = 0; i < length; i++) {
  dest[i] = src[i];
 }
 return dest;
}

char* set_speed_cmd(S2c speed_left, S2c speed_right) {
 static char cmd[6];
 cmd[0]=(char)145;
 cmd[1]=speed_right.ch[1];
 cmd[2]=speed_right.ch[0];
 cmd[3]=speed_left.ch[1];
 cmd[4]=speed_left.ch[0];
 cmd[5]=0;
 return (char*)cmd;
}

void set_speeds(short speed_avg,short radius) {
 S2c speed_left,speed_right;
 float speed_min,speed_max; 

 speed_min=(short)(((float)speed_avg*((float)radius-HALF_WHEEL_SPACING)/(float)radius));
 speed_max=(short)(((float)speed_avg*((float)radius+HALF_WHEEL_SPACING)/(float)radius));

 //front
 speed_left.sh=speed_avg;
 speed_right.sh=speed_avg;
 bytecopy(front,set_speed_cmd(speed_left,speed_right),5);

 //back
 speed_left.sh=speed_right.sh=-speed_avg;
 bytecopy(back,set_speed_cmd(speed_left,speed_right),5);

 //spinleft
 speed_left.sh=-speed_avg;
 speed_right.sh=speed_avg;
 bytecopy(spinleft,set_speed_cmd(speed_left,speed_right),5);

 //spinright
 speed_left.sh=speed_avg;
 speed_right.sh=-speed_avg;
 bytecopy(spinright,set_speed_cmd(speed_left,speed_right),5);

 //radius_fleft
 speed_left.sh=speed_min;
 speed_right.sh=speed_max;
 bytecopy(radius_fleft,set_speed_cmd(speed_left,speed_right),5);

 //radius_fright
 speed_left.sh=speed_max;
 speed_right.sh=speed_min;
 bytecopy(radius_fright,set_speed_cmd(speed_left,speed_right),5);

 //radius_bleft;
 speed_left.sh=-speed_min;
 speed_right.sh=-speed_max;
 bytecopy(radius_bleft,set_speed_cmd(speed_left,speed_right),5);

 //radius_bright
 speed_left.sh=-speed_max;
 speed_right.sh=-speed_min;
 bytecopy(radius_bright,set_speed_cmd(speed_left,speed_right),5);
} 


int roomba_exit(int code) {
 tcdrain(0);
 tcdrain(fd);
 tcdrain(sockfd);
 tcdrain(newsockfd);
 close(fd);
 close(sockfd);
 close(newsockfd);
 fclose(roombalog);
 fclose(roombascript);
 //reset tty settings
 tcsetattr(0,TCSANOW,&oldtio);
 tcsetattr(1,TCSANOW,&oldtioo);
 tcflush(0,TCIFLUSH);
 tcflush(1,TCIFLUSH);
 _exit(code);
}

void roomba_reset_position(){
 position.speed_left=position.speed_right=0;
 position.x=position.y=position.angle=0;
 position.azimut=0;
 position.charge_level=0.0; 
}

void locate();
void go(int fd,unsigned char *cmd,int i) {
 //strncpy(position.cmd,cmd,strlen(cmd));
 gettimeofday(&t[3],NULL);
 delta_cmd=1000*(t[3].tv_sec-t[2].tv_sec)+(t[3].tv_usec-t[2].tv_usec)/1000;
 gettimeofday(&t[2],NULL);

 write(fd,cmd,i);
 tcflush(fd,TCIFLUSH);
}

void bump(int fd,int dir) {
 /// SLEEPING while bumbing is an isse for the select => play with select timeout.
 switch (dir) {
  case 1 :   GO(back); 
             position.speed_left=position.speed_right=-150; usleep(300000);
             GO(spinleft); 
             position.speed_left=-150; position.speed_right=150;
             usleep(332000); //22.5 °
             GO(stop);
             break;

   case 2 :  GO(back); 
             position.speed_left=position.speed_right=-150; usleep(300000);
             GO(spinleft); 
             position.speed_left=150; position.speed_right=-150;
             usleep(332000); //22.5 °
             GO(stop);
             break;

  case 3 :   GO(back); 
             position.speed_left=position.speed_right=-150; usleep(300000);
             GO(spinleft); 
             position.speed_left=-150; position.speed_right=150;
             usleep(13280000); //90 °
             GO(stop);
             break;
 }
}

// Check bumper;
void event_manager() {
 if (position.bumper && BUMPER) {
  bump(fd,position.bumper);
 }

 if ( (unsigned short) 10 < position.charge_level < (unsigned short) 85)
    {
      printf("Battery Low: %d \n",position.charge_level);
      //GO(dock);

    }
} 

void locate() {
 //delta ngle cw < 0
 //delta  angle ccw > 0
 gettimeofday(&t[1],NULL);
 delta_t=1000*(t[1].tv_sec-t[0].tv_sec)+(t[1].tv_usec-t[0].tv_usec)/1000;
 gettimeofday(&t[0],NULL);
 position.speed_avg=(short)fabs((float)(position.speed_left+position.speed_right)/2);
 /*position.speed_left=speed_left;
 position.speed_right=speed_right;
 */
 delta.angle=0.0;
 delta.azimut=0;

 if (position.speed_left == position.speed_right) {
  //FRONT  or BACK
  position.x-=(float)position.speed_avg*(float)delta_t*sin(position.angle)*1E-4; //distance en cm 
  if ( position.speed_left > 0) {
   position.y+=(float)position.speed_avg*(float)delta_t*cos(position.angle)*1E-4; //distance en cm 
  }
  else {
   position.y-=(float)position.speed_avg*(float)delta_t*cos(position.angle)*1E-4; //distance en cm 
  } 

 }
 else if ( abs(position.speed_left) == abs(position.speed_right) ) {
  if (speed_right <0) {
   //SPINRIGHT; //cw
   position.angle-=((float)position.speed_left*(float)delta_t*1E-4/(HALF_WHEEL_SPACING_SPIN)) ; 
   position.azimut=(int)((float)(360.0*position.angle/(2*M_PI)))%360;
  }
  else {
   //SPINLEFT; //ccw
   //position.angle+=(2*M_PI*(float)delta_t/5312.0) ; 
   position.angle+=((float)position.speed_right*(float)delta_t*1E-4/(HALF_WHEEL_SPACING_SPIN)) ; 
   position.azimut=(int)(360.0*position.angle/(2*M_PI))%360;
  }
 }
 else if ((position.speed_right > position.speed_left) && (position.speed_right > 0)) {
  //TURN radius FRONT LEFT = u ccw
  //TURNFLEFT; 
  delta.angle=fabs(1E-4*(float)abs(position.speed_avg)*(float)delta_t/radius);
  delta.azimut=(int)(360.0*delta.angle/(2*M_PI))%360;
  position.x=position.x+radius*(-cos(position.angle)+cos(delta.angle+position.angle));
  position.y=position.y+radius*(-sin(position.angle)+sin(delta.angle+position.angle));
 }
 else if ((position.speed_right < position.speed_left) && (position.speed_right > 0)) {
  //TURN radius FRONT RIGHT = o cw
  //TURNFRIGHT;
  delta.angle=-fabs(1E-4*(float)abs(position.speed_avg)*(float)delta_t/radius);
  delta.azimut=(int)(360.0*delta.angle/(2*M_PI))%360;
  position.x=position.x+radius*(cos(position.angle)-cos(position.angle+delta.angle));
  position.y=position.y+radius*(sin(position.angle)-sin(position.angle+delta.angle));
 } 
 else if ((position.speed_right < position.speed_left) && (position.speed_right < 0)) {
  //TURN radius BACK LEFT = n cw
  //TURNBLEFT;
  delta.angle=-fabs(1E-4*(float)position.speed_avg*(float)delta_t/radius);
  delta.azimut=(int)(360.0*delta.angle/(2*M_PI))%360;
  position.x=position.x+radius*(-cos(position.angle)+cos(delta.angle+position.angle));
  position.y=position.y+radius*(-sin(position.angle)+sin(delta.angle+position.angle));

 }
 else if ((position.speed_right > position.speed_left) && (position.speed_right < 0)) {
  //TURN radius BACK RIGHT = . ccw
  //TURNBRIGHT;
  delta.angle=fabs(1E-4*(float)position.speed_avg*(float)delta_t/radius);
  delta.azimut=(int)(360.0*delta.angle/(2*M_PI))%360;
  position.x=position.x+radius*(cos(position.angle)-cos(delta.angle+position.angle));
  position.y=position.y+radius*(sin(position.angle)-sin(delta.angle+position.angle));

 }
 position.azimut=(int)(360.0*position.angle/(2*M_PI))%360;
 position.angle+=delta.angle;
 
 fprintf(roombalog,"charging state=%3d charging level=%6d ",position.charging_state,position.charge_level);
 fprintf(roombalog,"speed_avg=%d speedl=%d speedr=%d delta_t=%d X=%f Y=%f azm=%d\n",position.speed_avg,position.speed_left,position.speed_right,delta_t,position.x,position.y,position.azimut);
}


void command_roomba(int inputf) {
   static char car[256];
    bzero(car,256);
    //printf("before read command\n");
    n=read(inputf,car,1);
    printf("cmd=%s \n",car);
    if (n<=0) {perror("On Read Socket.");}
    switch (car[0]) {
     case 'u' : GO(radius_fleft);break;
     case 'o' : GO(radius_fright);break;
     case ';' : GO(radius_bright);break;
     case 'n' : GO(radius_bleft);break;
     case 'l' : GO(led);break;
     case 'j' : GO(spinleft);break;
     case 'k' : GO(spinright);break;
     case 'i' : GO(front);break;
     case ',' : GO(back);break;
     case ' ' : GO(stop_sensor);GO(stop);break;
     case 's' : GO(start);break;
     case 'b' : GO(baud);break;
     case 'p' : GO(passive);break;
     case 'f' : GO(full);break;
     case 'a' : GO(safe);break;
     case 'E' : flag_sensor=0;GO(start_sensor);break;
     case 'D' : flag_sensor=255;GO(stop_sensor);break;
     case 'e' : flag_sensor=0;GO(stream_sensor);break;
     case 'g' : flag_sensor=1;GO(single_sensor);break;
     case 'd' : GO(dock);break;
     case 'c' : GO(song);break;
     case 'R' : roomba_reset_position();break;
     case 'r' : GO(reset);flag_sensor=0;break;
     case 'q' : GO(stop);sleep(1);close(fd);roomba_exit(0);break; //Stop Robot & Leave program
     case 'v' : GO(display);break;
     case 'w' : roomba_wakeup(fd);break;
     case 't' : GO(daytime);break;
     case 'T' : GO(schedule);break;
     case 'x' : GO(brosse_l);break;
     case 'C' : GO(brosse_r);break;
     case 'X' : GO(brosse_off);break;
     //missing command to change Speed and Radius  
     //missing calibration function for speed: FRONT/BACK/SPINL/SPINR/TURNFL/TURNFR/TURNBL/TURNBR 
    }
}



void serialize_stream_packet(unsigned char * buffer){
 bzero(buffer,sizeof(buffer));
 //tcdrain(fd);
  n=read(fd, buffer,1);
  //printf("%d ",buffer[0]);
  if (buffer[0] = 0x13) {
  k=1;
  while (k< 22)
  {
   n=0;
   n=read(fd, &buffer[k],22);
   k+=n;
  }
  chksum=0;
 // ****************
 // compute checksum
 // ****************
 for (k=0;k<21;k++) {
  chksum+=buffer[k];
//  printf("%d ",buffer[k]);
 }
 //printf("%d ",buffer[k]);
 chksum=256-(chksum % 256);
 //printf("c=%d  \n",chksum);
 // ******************************************
 // check header and checksum for valid packet 
 // ******************************************
 if ( buffer[0] == 0x13  && buffer[21] == chksum) {

  //Packet == 0x13 <nbpackets> <pid1> <data1>...<pidn> <datan> <chksum>
  //printf("got it ! \n");
  pbyte=buffer;
  pbyte++;
  nbpackets=pbyte[0];
  for(i=0;i<nbpackets;i++) {
   pbyte++;
   switch (pbyte[0]) {
    //bumper and wheels
    case 0x07 : pbyte++;
		position.bumper=pbyte[0];
 	        break;
    //Wall Seen
    case 0x08 : pbyte++;
                if (position.wall=1) {
		 position.wall=1; 
		} 
		else {
		 position.wall=0;
                }
		break;
    //Charging state;
    case 0x15 : pbyte++;
	 	position.charging_state=pbyte[0];
		break;
    //Charging Level
    case 0x19 : pbyte++;
	        charge_level= (unsigned short)(pbyte[0]*256+pbyte[1]);
                break;
    //Battery max 
    case 0x1A : pbyte++;
	        charge_max= (signed short)(pbyte[0]*256+pbyte[1]);
	        position.charge_level=(short)(100.0*(float)charge_level/(float)charge_max);
                pbyte++;
                break;
    //Wall power
    case 0x1B : pbyte++;
		position.wall_power=(signed short)(pbyte[0]*256+pbyte[1]);
                pbyte++;
		position;
		break;
    // Speed Right
    case 0x29 : pbyte++;
                position.speed_right=0;
                position.speed_right=(signed short)((char) pbyte[0] ) << 8 ;
                position.speed_right|=pbyte[1];
                pbyte++;
                break;
    // Speed Left
    case 0x2A : pbyte++;
                position.speed_left=0;
                position.speed_left=(signed short)((char) pbyte[0] ) << 8;
                position.speed_left|=pbyte[1];
                pbyte++;
   }
  }
 locate();
 }
 } 
}



int tscroll(int i,char *sms)
{
 int j;
 char msg[255];
 bzero(msg,255);
 bzero(digits,4);
 digits[0]=164; 
  for (j=1;j<5;j++)
  {
   if ((i+j-1) < strlen(sms))  
   {
    digits[j]=sms[i+j-1];
   }
   else
   {
    digits[j]=sms[j-1];
   }

  }
}



int main() {

 // Set Position to 0;
 roomba_reset_position() ;
 // Initialize speeds and radius  in all directions
 set_speeds(speed_avg,radius);
 
 // ************************************
 // Open Roomba roombalog and error file
 // ************************************
  
  roombalog=fopen("/var/log/roomba.log","w+");
  roombascript=fopen("/var/log/roomba.script","w+");

 // ************************************
 // * Open Roomba Modem and set tty raw 
 // ************************************

 fd=open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NDELAY );
 if (fd<0) {perror(MODEMDEVICE);roomba_exit(1);}

 // Set New Serial settings 
  newtio.c_cflag= BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag= IGNPAR;
  newtio.c_oflag=0;
  newtio.c_lflag=0;
  tcflush(fd,TCIFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);


//******************************************************
// Wake up roomba by flip flop on GPIO 7 // ddpin Roomba 
//******************************************************
 roomba_wakeup(fd);
 // ******************************************
 // * Open STDIN and set tty  RAW without echo
 // *******************************************
 tcgetattr(0,&oldtio);
 tcgetattr(1,&oldtioo);
 tcgetattr(0,&stdinio);
 stdinio.c_lflag &= ~ICANON;
 stdinio.c_lflag &= ~ECHO;
 stdinio.c_cc[VMIN] = 1;
 stdinio.c_cflag &= ~(CSIZE | PARENB);
 stdinio.c_cflag |= CS8;
 tcflush(0,TCIFLUSH);
 tcsetattr(0,TCSANOW,&stdinio);
 // *********************************
 // * Open Server Socket and set tty
 // *********************************
 sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
 if (sockfd <= 0) perror("Error Opening Socket");
 bzero((char *) &serv_addr,sizeof(serv_addr));
 serv_addr.sin_family = AF_INET;
 serv_addr.sin_addr.s_addr = INADDR_ANY;
 serv_addr.sin_port = htons(port);
 if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) perror("ERROR on Binding");
 //if (listen(sockfd,1)<0) perror("ERROR Listen failed");
 // ***************************
 // *  reset the FD selectors r/w   
 // ***************************
 FD_ZERO(&readfds);
 FD_ZERO(&writefds);
 // *****************
 // * Reset Roomba
 // *****************
  gettimeofday(&t[0],NULL);
  gettimeofday(&t[2],NULL);
  delta_cmd=0;
  //GO(reset);
  GO(start);
  sleep(1);
  GO(safe);

  flag_sensor = 0; 
  sleep(1);
  bzero(buffer,sizeof(buffer));
  printf("\nReady to talk to Roomba\n");
  //************************************** 
  // * Main loop read write with roomba
  //************************************** 
  fdmax=sockfd;
  for(;;) 
  {
   FD_SET(sockfd,&readfds);
   FD_SET(fd,&readfds);
   FD_SET(0,&readfds);
   stimeout.tv_sec=0; 
   stimeout.tv_usec=0; 
   slct=select(fdmax+1,&readfds,&writefds,NULL,&stimeout);
   if (slct == -1) {perror(MODEMDEVICE);roomba_exit(4);}
   else 
   // ***************************************************
   // Something to read from sensors / Keyboard / Network
   // ***************************************************
   if (slct) {
   // ***********************************************
   // * if anything to read on Roomba serial port
   // ***********************************************
    if (FD_ISSET(fd,&readfds)) {
     //printf("Got it \n");
     bzero(buffer,sizeof(buffer));
     // *********************************************
     // * stream_sensor flag_sensor == 0
     // *********************************************
     if (flag_sensor == 0) { 
      serialize_stream_packet(buffer) ;
     }
    // *********************************************
    // Roomba splash:  flag_Sensor=255
    // *********************************************
    else 
    {
     bzero(buffer,sizeof(buffer));
     n=read(fd,buffer,1024);
    }
   }
   // *******************************************
   // * if any command to read on the keyboard  
   // *******************************************
   if (FD_ISSET(0,&readfds)) {
    command_roomba(0);
   }
   // **********************************
   // * New connection request on socket
   // **********************************
   //else 
   //if (FD_ISSET(sockfd,&readfds)) {
    //newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
   // if (newsockfd < 0) perror("ERROR on Accept"); 
    //fdmax=newsockfd;
    //printf("fdmax=%d newsockfd=%d \n",fdmax,newsockfd);
    //FD_SET(newsockfd,&readfds);
   //}
   // **********************************
   // * New Roomba Data on New Socket
   // **********************************
   if (FD_ISSET(sockfd,&readfds)) {
    //FD_CLR(newsockfd,&readfds);
    command_roomba(sockfd);
    //n=write(sockfd,"Got it\0", 7);
    //sleep(5);
    //close(sockfd); 
    //fdmax=sockfd;
    }
   }
   // ************************************
   // Select Timeout / beacon  5ms =>  do some stuff : Recovery ?  Locate ? Issue ? Why not
   // ************************************
   if (flag_sensor == 0) 
   {
    usleep(1000);
    //event_manager();
    counter++;
    if (counter > 300)
    {
    //int n;
    counter=0;
    if (possms==strlen(sms)-1) { possms=0;}
    //for (n=1;n <5;n++) printf("%c",digits[n]);
    //printf("\n");
    tscroll(possms,sms);
    possms++;
    SMS(digits);
   }
  }

 }
 roomba_exit(0);
}
