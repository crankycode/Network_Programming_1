
/* UDP Server */
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "myconfiglib.h"

#define MAX_HOUR     23
#define MAX_MIN      59
#define MAX_SEC      59
#define MAX_DAY      31
#define MAX_MON      12
#define MAX_YEAR     9999
#define TIMEZONE_LEN 4
#define RAND_CHOOICE 4
#define MSG_LEN      16
#define MAGIC_NO     0xA3F0
#define REQUEST      0X52
#define REPLY        0XB4
#define EXTRA_YEAR   1900
#define EXTRA_MONTH  1
#define SYSTEM_TIME  0
#define VALID_TIME   1
#define INVALID_TIME 2
#define IGNORE_REQ   3
#define LISTENING    1
#define ERROR        -1

typedef struct datagram {

uint16_t         mesgType; /* magic number 0xA3F0, see below */
unsigned char    status;   
unsigned char    second;   /* 0..59, seconds portion of current time */
unsigned char    minute;   /* 0..59, minutes portion of current time */
unsigned char    hour;     /* 0..23, hours portion of current time */
unsigned char    day;      /* 1..31, day portion of current date */
unsigned char    month;    /* 1..12, month portion of current date */
uint32_t         year;     /* 1..9999, year portion of current date */
unsigned char    timezone[TIMEZONE_LEN]; /* time-zone code */
}Datagram;

void
createMessage(Datagram *msg,uint16_t mes,int sta,int sec,int min,int hou,
              int day,int mon,int yea,const char *tim)
{ 
/* msg->mesgType = htons(mes);*/
  memset(msg,'\0',sizeof(Datagram));
  msg->mesgType = mes; 
  msg->status   = sta;
  msg->second   = sec;
  msg->minute   = min;
  msg->hour     = hou;
  msg->day      = day;
  msg->month    = mon;
  msg->year     = yea; 
  
/*  msg->year     = htonl(yea);  */
  memcpy(msg->timezone,tim,sizeof(char)*4);
  
}

void
mainProcess(int udpSocket, int portNo, struct sockaddr* udpClient,int logging)
{
  time_t rawtime;
  char   currTimeZone[TIMEZONE_LEN]          = "AEST"; 
  int             replyed                    = FALSE;

  int             returnStatus               = 0;
  int             msgType                    = 0;
  unsigned int    addrlen                    = 0;
  char            buf[MSG_LEN];
  struct tm*      timeinfo;
  Datagram message;
  /* system time */
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  while (LISTENING)
  {
    /* reset replyed flag */
    replyed = FALSE;
    /* convert requester ip addr to string */
    inet_ntop(AF_INET,&udpClient,buf,INET_ADDRSTRLEN);
    
    addrlen = sizeof(udpClient);
    returnStatus = recvfrom(udpSocket,&message, sizeof(message), 0, 
                            udpClient,&addrlen);

    if(returnStatus == ERROR) {
       fprintf(stderr,"error: %s \n",strerror(returnStatus));
       fprintf(stderr, "Could not receive message!\n");
    } 
    /* validate magic no = 0xA3F0 and status = 0X52 */
    else if((message.mesgType != MAGIC_NO || message.status != REQUEST) &&
             logging == TRUE) {
            printf("timeserver: invalid request from %s:%d\n",buf,portNo);
    }
    /* generate reply */
    else {
        
        if( logging == TRUE) 
          fprintf(stderr,"timeserver: valid request from %s:%d\n",buf,portNo);
        
        msgType = rand() % RAND_CHOOICE;
printf("%d \n",msgType);
        /* random generate reply 0 to 2 */
        switch(msgType) {
        /* current time */
          case SYSTEM_TIME:

          createMessage(&message,MAGIC_NO,REPLY,timeinfo->tm_sec,
                        timeinfo->tm_min,timeinfo->tm_hour,timeinfo->tm_mday,
                        timeinfo->tm_mon+EXTRA_MONTH,
                        timeinfo->tm_year+EXTRA_YEAR,
                        currTimeZone);

          returnStatus = sendto(udpSocket, &message, sizeof(message), 0, 
                                udpClient, addrlen);
          replyed = TRUE;
          break;
      
          /* valid time */
          case VALID_TIME:

          createMessage(&message,MAGIC_NO,REPLY,rand() % MAX_SEC,
                        rand() % MAX_MIN,rand() % MAX_HOUR,rand() % MAX_DAY,
                        rand() % MAX_MON,rand() % MAX_YEAR,
                        currTimeZone);
      
            returnStatus = sendto(udpSocket, &message, sizeof(message), 0, 
                                  udpClient, addrlen);         
          replyed = TRUE;
          break;
          
          /* invalid time */
          case INVALID_TIME:
          
          createMessage(&message,MAGIC_NO,REPLY,MAX_SEC + rand() % MAX_SEC,
                        MAX_MIN + rand() % MAX_MIN,MAX_HOUR + rand() % MAX_HOUR,
                        MAX_DAY + rand() % MAX_DAY,MAX_MON + rand() % MAX_MON,
                        MAX_YEAR + rand() % MAX_YEAR,currTimeZone);
      
            returnStatus = sendto(udpSocket, &message, sizeof(message), 0, 
                                  udpClient, addrlen);
          replyed = TRUE;
                                        
          break;
          
          /* ignore request */
          case IGNORE_REQ:
            replyed = FALSE;
          break;
        }
        if(returnStatus == ERROR) {
          fprintf(stderr, "error: Could not send confirmation!\n");
          fprintf(stderr, "error: %s \n",gai_strerror(returnStatus));
        }
        else if(replyed == TRUE && logging == TRUE){
			 fprintf(stderr,"timeserver: replying to %s:%d with \
%d:%d:%d %d-%d-%d \n",buf,portNo,message.hour,
						message.minute,message.second,message.month,
						message.day,message.year);
        }
    }
  }
                                 
}


int 
main(int argc, char* argv[])
{
  int udpSocket;
  int returnStatus = 0;
  int logging      = FALSE;
  Configfile cfg;
  struct sockaddr_in udpServer, udpClient; 

  /* check for the right number of arguments */
  if (argc < 2) {
      fprintf(stderr, "Usage: %s <port>\n", argv[0]);
      exit(1);
  }

  parseConfigFile(argv[1],&cfg);
  logging = cfg.print_message_details;
  /* create a socket */
  udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

  if (udpSocket == ERROR) {
      fprintf(stderr, "Could not create a socket!\n");
      exit(1);
  }
   
  else {
      printf("Socket created.\n");
  }

  /* setup the server address and port */
  udpServer.sin_family = AF_INET;
  /* use INADDR_ANY to bind to all local addresses */
  udpServer.sin_addr.s_addr = htonl(INADDR_ANY);

  /* use the port pased as argument */
  udpServer.sin_port = htons(cfg.port);
  /* bind to the socket */
  returnStatus = bind(udpSocket,(struct sockaddr*)&udpServer,sizeof(udpServer));

  if (returnStatus == 0) {
      fprintf(stderr, "timeserver: waiting on port %d\n",udpServer.sin_port);
  } 
  else {
      /* print error msg */
      fprintf(stderr,"error: %s \n",gai_strerror(returnStatus));
      fprintf(stderr, "error: could not bind to address!\n");
      close(udpSocket);
      exit(1);
  }

  mainProcess(udpSocket,udpServer.sin_port,(struct sockaddr*)&udpClient,logging);
    
  /*cleanup */
  close(udpSocket);
  return 0;
}
