
/* UDP client */

#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include "myconfiglib.h"

#define MAXBUF       1024
#define MAX_HOUR     23
#define MAX_MIN      59
#define MAX_SEC      59
#define MAX_DAY      31
#define MAX_MON      12
#define MAX_YEAR     9999
#define TIMEZONE_LEN 4
#define RAND_CHOOICE 3
#define MSG_LEN      16
#define MAGIC_NO     0xA3F0
#define REQUEST      0X52
#define REPLY        0XB4
#define EXTRA_YEAR   1900
#define EXTRA_MONTH  1
#define SYSTEM_TIME  0
#define VALID_TIME   1
#define INVALID_TIME 2
#define LISTENING    1
#define ERROR        -1
#define MAXHOSTNAME  30
#define FALSE        0
#define TRUE         1

typedef struct datagram {

uint16_t         mesgType;    /* magic number 0xA3F0, see below */
unsigned char    status;   
unsigned char    second;      /* 0..59, seconds portion of current time */
unsigned char    minute;      /* 0..59, minutes portion of current time */
unsigned char    hour;        /* 0..23, hours portion of current time */
unsigned char    day;         /* 1..31, day portion of current date */
unsigned char    month;       /* 1..12, month portion of current date */
uint32_t         year;        /* 1..9999, year portion of current date */
unsigned char    timezone[4]; /* time-zone code */
}Datagram;

int
validateReply(Datagram* msg)
{
/*  
  msg->mesgType = ntohs(msg->mesgType);
  msg->year     = ntohl(msg->year);
*/
  if(msg->mesgType !=MAGIC_NO || msg->status != REPLY)
     return FALSE;
  if(msg->second > MAX_SEC || msg->minute > MAX_MIN || msg->hour > MAX_HOUR ||
     msg->day > MAX_DAY || msg->month > MAX_MON || msg->year > MAX_YEAR)
     return FALSE;
     
  return TRUE;
}

void
initRequestMsg(Datagram* request)
{
  	 /* init structure with value */
    request->mesgType = 0xA3F0;
    request->status   = 0x52;
    request->second   = '0';
    request->minute   = '0';
    request->hour     = '0';
    request->day      = '0';
    request->month    = '0';
    request->year     = '0';
}

int main(int argc, char* argv[])
{
    int udpSocket      = 0;
    int retryCount     = 0;
    int selectResult   = 0;
    int timeout        = 0;
    int returnStatus   = 0;
    int addrlen        = 0;
    int firstRequest   = TRUE;
    int logging        = FALSE;
    int replyStatus    = FALSE;
    int invalidRequest = FALSE;
    char buf[MAXBUF];
    struct      in_addr address,**addrptr;
    struct      hostent         *answer;
    Datagram    request;
    Configfile  cfg;
    
    struct sockaddr_in udpServer; 
    
    struct timeval tv;
    fd_set stReadFDS;
   
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(1);
    }
    
    /* parse config file and set setting */
    parseConfigFile(argv[1],&cfg);
    logging = cfg.print_message_details;
    timeout = cfg.request_timeout;
    /* set timeout time */
    tv.tv_sec  = timeout;
    tv.tv_usec = 0;
   
    /* create a socket */
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (udpSocket == ERROR)
    {
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }

    /* get host address */
    if((inet_aton(cfg.address ,&udpServer.sin_addr)) == FALSE )
    {
      
      if (inet_aton(cfg.address, &address))
          answer = gethostbyaddr((char *) &address,sizeof(address), AF_INET);
      else
          answer = gethostbyname(cfg.address);
          
      if (!answer) {
      fprintf(stderr,"error looking up host : %s\n",cfg.address);
          return 1;
      }

      /* get the address in numberic form */
      for (addrptr = (struct in_addr **) answer->h_addr_list;*addrptr;addrptr++) 
      {
          strcpy(cfg.address,inet_ntoa(**addrptr));
          break;
      }
      udpServer.sin_addr.s_addr = inet_addr(cfg.address);
    }
    
    /* server address */ 
    udpServer.sin_family = AF_INET;
    udpServer.sin_port = htons(cfg.port);

    if (returnStatus == ERROR) {
        fprintf(stderr, "Could not send message!\n");
    }
    /* start sending request */
    else {
          
          /* keep requesting until reply is valid or reach max retry count */
          while( replyStatus != TRUE && retryCount < cfg.request_count)
          {
            /* select macro to set file desc */
            FD_ZERO(&stReadFDS);
            FD_SET(udpSocket, &stReadFDS);
            /* '0' = send request, selectResult > 0 = recvfrom  */
            selectResult = select(udpSocket+1, &stReadFDS, 0 , 0, &tv);
            /* init request msg with magic no. and request status */
            initRequestMsg(&request);
            addrlen = sizeof(udpServer);
            
            /* send request */
            if(selectResult == 0) 
            {
              if(firstRequest == FALSE && invalidRequest == FALSE && 
                 logging == TRUE)
                  fprintf(stderr,"timeclient: timeout - no reply \n");
                    
              /* send request */
              returnStatus = sendto(udpSocket, &request, sizeof(request)+1, 0, 
                                (struct sockaddr*)&udpServer,sizeof(udpServer));
                                       
              if(returnStatus == ERROR) {
                  fprintf(stderr, "error: Could not send confirmation!\n");
                  fprintf(stderr, "error: %s \n",gai_strerror(returnStatus));
                  exit(0);
              }
              /* reset invalid request flag */
              invalidRequest = FALSE;
              firstRequest   = FALSE;
              /* convert network address into string for printing out */
              inet_ntop(AF_INET,&(udpServer.sin_addr),buf,INET_ADDRSTRLEN);
              if(logging == TRUE)
                 fprintf(stderr,"timeclient: request to %s:%d\n",
                         buf,udpServer.sin_port);
            }  
            
            /* start receiving reply */
            else if(selectResult >0 || timeout == FALSE) 
            {
              /* receive reply */
              returnStatus = recvfrom(udpSocket, &request, sizeof(request)+1, 0, 
                                     (struct sockaddr*)&udpServer, &addrlen);
                                                
              if(returnStatus == ERROR) {
                 fprintf(stderr, "error: Could not receive confirmation !\n");
                 fprintf(stderr, "error: %s \n",gai_strerror(returnStatus));
              }
              
              /* validate reply */
              else if(validateReply(&request) == TRUE) {
                if(logging == TRUE)
                    fprintf(stderr,"timeclient: reply from %s:%d is good\n",buf, 
                            udpServer.sin_port);
                          
                 /* reset string buffer */       
                 memset(buf,'\0',MAXBUF);
                 
                 fprintf(stderr,"%d:%d:%d %d-%d-%d %s\n",request.hour,
                         request.minute,request.second,request.month,
                         request.day,request.year,request.timezone);
                 
                 replyStatus = TRUE;
                 break;
              }
              else if(validateReply(&request) == FALSE) {
                if(logging == TRUE)
                    fprintf(stderr,"timeclient: reply from %s:%d is invalid \n",
                            buf,udpServer.sin_port);
                 invalidRequest = TRUE;
                 /* reset string buffer */       
                 memset(buf,'\0',MAXBUF);     
              }
            }
                    
            retryCount++;
          }
    }

    /* cleanup */
    close(udpSocket);
    return 0;

}
