#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <curses.h>
#include <sys/time.h>
#include <netdb.h>

void error(char *msg)
{
    perror(msg);
//    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       _exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname((char *)argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        _exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
//    if (connect(sockfd,&serv_addr,sizeof(serv_addr)) < 0) 
//       error("ERROR connecting");
    printf("Please enter the message: \n");
    bzero(buffer,256);
    read(0,&buffer,1);
    n =sendto(sockfd,buffer,strlen(buffer),0,&serv_addr,sizeof(serv_addr));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
//    n = read(sockfd,buffer,255);
//    if (n < 0) 
//         error("ERROR reading from socket");
//    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
