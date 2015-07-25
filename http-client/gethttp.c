#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <unistd.h> 
#include <strings.h>
#include <netinet/in.h> 
#include <fcntl.h>
#include <errno.h>

#define DELIM             " "
#define NEWLINE           '\n'
#define TRUE              1
#define FALSE             0
#define STATUS_CODE_TOKEN 1
#define BUFFERSIZE        10*1024*1024
#define STATUS_NUM_1      9
#define STATUS_NUM_2      10
#define STATUS_NUM_3      11
#define TWO_MB            2097152
#define ONE_KB            1024
#define TEMP_SIZE         254
#define REQUIRED_ARG      3
#define STRING_LEN        40
#define HOST_LEN          80
#define URL_FILE_LEN      80
#define PORT_LEN          10
#define STATUS_CODE_BEGIN 9

typedef struct headerInfo {

char requestFileUrl[URL_FILE_LEN];
char port[PORT_LEN];
char host[HOST_LEN];

} HeaderInfo;

/* convert space to %20 */  
void
spaceToSpecialCharacter(char* src,char* dest)
{
  char* add = dest;
  while (*src)
  {
       char c=*src++;
       if (c == ' ') {
          *dest++='%'; *dest++='2'; *dest++='0';
       }
       else {
         *dest++=c;
       }
  }
  dest=add;
}

void
parseUserTypeAddress(char* src,char* hostAddr, char*requestFileUrl, char* portNo)
{
  char* add     = hostAddr;
  int   portNum = FALSE;
  while (*src)
  {
       char c=*src++;
       /* set port number */
       if(c == ':') {
			 c=*src++;
          while(c !='/')
          {
            *portNo++ = c;
            c=*src++;
          }
          portNum = TRUE;  
          src--;
       }
       if (c == '/') {
          /* set default port num */
          if(portNum == FALSE)
              strcpy(portNo,"80");
          hostAddr = add;
          strcpy(requestFileUrl,strstr(src-1,"/"));
          return;
       }
       else {
         *hostAddr++=c;
       }
  }
  hostAddr=add;
}

void
parseRequestFileUrl(char* src, char* dest)
{
  strcpy(dest,strstr(src,"/"));
}

int
printStatusCode(char* src, char statusNumOne, char  statusNumTwo, char statusNumThree)
{
  char  statusMsg[STRING_LEN] = "";
  char* temp                  = src;
  int   srcCount              = STATUS_CODE_BEGIN;
  int   statusMsgCount        = 0;

  printf("%c %c\n",src[STATUS_NUM_1],statusNumOne);

  if((src[STATUS_NUM_1] == statusNumOne) && (src[STATUS_NUM_2] >= statusNumTwo) 
    && (src[STATUS_NUM_3] >= statusNumThree))
  {
    while(src[srcCount] != '\n') 
    {
      statusMsg[statusMsgCount] = src[srcCount];
      srcCount++;
      statusMsgCount++;
    }
    fprintf(stderr,"%s\n",statusMsg);
    src = temp;
    return TRUE;
  }
  src = temp;
  return FALSE;
}

int
parseReplyFromServer(char* src,int recvBytes)
{
  char   statusMsg[STRING_LEN] = "";
  char* temp                   = src;
  
  memset(statusMsg,'\0',STRING_LEN);
  
  if(src[STATUS_NUM_1] =='2' && src[STATUS_NUM_2] >=0 && src[STATUS_NUM_3] >= 0)
  {
    fprintf(stderr,"%c%c%c success\n",src[STATUS_NUM_1],src[STATUS_NUM_2],
            src[STATUS_NUM_3]);
    src = temp;
    return TRUE;
  }
  
  if(printStatusCode(src,'3','0','0')) {
    src = temp;
    return TRUE;
  }
  else if( printStatusCode(src,'4','0','0')) {
	  src = temp;
	  return FALSE;
  }
  else if( printStatusCode(src,'5','0','0')) {
	  src = temp;
	  return FALSE;
  }
  src = temp;
  return FALSE;
}

int
startCreateFile(char *src, int recvFileSize, char *filename)
{
  FILE* pFile;
  int counter = 0;

  while(counter < recvFileSize)
  {
	  if(src[counter]   == '\r' && src[counter+1] == '\n' && 
		  src[counter+2]  == '\r' && src[counter+3] == '\n') 
	  {
      counter += 4;
      pFile = fopen(filename,"wb");
      fwrite(src+counter, 1, recvFileSize-counter, pFile);
      fclose(pFile);
      fprintf(stderr,"finish writing \n");
      return TRUE;
      break;
    }
    counter++;
  }
  return FALSE;
}

int 
main(int argc, char *argv[]) {

    int         simpleSocket    = 0;
    int         simplePort      = 0;
    int         returnStatus    = 0;
    int         recvBytes       = 0;
    int         createFile      = FALSE;
	  char        buffer          [TWO_MB] = "";
    char        temp            [TEMP_SIZE] = "";
    char        request         [ONE_KB]= "";

    struct      sockaddr_in     simpleServer;
    struct      hostent         *answer;
    struct      in_addr         **addrptr,address;
	  FILE        *gethttptx,     *gethttprx;
    HeaderInfo  header;

    memset(&header,'\0',sizeof(header));

    if (REQUIRED_ARG != argc) {

        fprintf(stderr,
        "gethttp: two arguments required, usage gethttp <url> <filename>\n");
         exit(1);
    }
    
    /* convert any space in address into %20 */
    spaceToSpecialCharacter(argv[1],temp);
    /* saperate address into "host addr" "file location" & port num */
    parseUserTypeAddress(temp,header.host,header.requestFileUrl,header.port);
    /* create a streaming socket */
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) {
        fprintf(stderr, "Could not create a socket \n");
        exit(1);
    }
       
    /* get host address */
    if (inet_aton(header.host, &address))
        answer = gethostbyaddr((char *) &address,sizeof(address), AF_INET);
    else
        answer = gethostbyname(header.host);
        
    if (!answer) {
        fprintf(stderr,"error looking up host : %s\n",header.host);
        return 1;
    }

    /* get the address in numberic form */
    for (addrptr = (struct in_addr **) answer->h_addr_list;*addrptr; addrptr++) 
    {
        strcpy(header.host,inet_ntoa(**addrptr));
        printf("%s \n",header.host);
        break;
    }
    
    /* convert port noumber to int */
    simplePort = atoi(header.port);

    /* setup the address structure */
    /* use the IP address sent as an argument for the server address  */
   
    bzero(&simpleServer, sizeof(simpleServer)); 
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr=inet_addr(header.host);
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, 
                           sizeof(simpleServer));

    if (returnStatus == 0) {
	    fprintf(stderr, "Connect successful!\n");
    }
    
    else {
        fprintf(stderr, "error not connect to address: %s\n",header.host);
        fprintf(stderr, "error: %s\n",gai_strerror(returnStatus));

        close(simpleSocket);
        exit(1);
    }
    
	 /* create request message */
    sprintf(request, "GET %s HTTP/1.0\r\nHOST:%s \r\n\r\n",
            header.requestFileUrl,header.host);

    returnStatus = write(simpleSocket, request, strlen(request)+1);
    if (returnStatus < 0)
    {
      fprintf(stderr, "Could not send filename to server!\n");
      exit(1);
    }
    
    /* write send data into gethttp-tx */
    gethttptx = fopen("gethttp-tx","wb");
    fwrite(request, 1, strlen(request), gethttptx);
    fclose(gethttptx);

    /* get the reply from the server   */
    recvBytes = read(simpleSocket, buffer, sizeof(buffer));
    
    /* write recieve data into gethttp-rx */
    gethttprx = fopen("gethttp-rx","wb");
    fwrite (buffer , 1 , recvBytes , gethttprx);
    fclose (gethttprx);
 
    /* if recvBytes is less than 0 == error */
    if ( recvBytes < 0 ) {
      fprintf(stderr, "Could not recieve reply from server!\n");
      exit(1);
    } 
    
    /* exit if file receive is bigger than buffer size */
    if ( recvBytes > TWO_MB) {
      fprintf(stderr, "Could not recieve reply from server!\n");
      exit(1);
    }
    /* print number of receive bytes */
    else {
      fprintf(stderr, "gethttp: %d bytes fetched\n",recvBytes);
      createFile = parseReplyFromServer(buffer,recvBytes);
      
      /* start creating the file */
      if(createFile == TRUE) {
        startCreateFile(buffer, recvBytes, argv[2]);
      }
    }
    close(simpleSocket);
    return 0;
}
