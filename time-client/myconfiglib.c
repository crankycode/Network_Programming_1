#include "myconfiglib.h"

void toLowerCase(char *str)
{
  char *temp = str;
  int  i     = 0;
  for(i = 0; i < strlen(str); i++) {
   /* remove any '\n' or '\r' */
	 if(str[i] == '\n' || str[i] == '\r')
		str[i] = '\0';
	 str[i] = tolower(str[ i ]);
  }
  str = temp;
}

void parseConfigFile(char* filename, Configfile* cfg) 
{
  FILE *pFile;
  char line[90];
  char *tok;
  char *newline;

  memset(cfg,'\0',sizeof(Configfile));
  /* open file */
  if((pFile =fopen(filename,"r"))==NULL)
  {
      fprintf(stderr,"Could not open '%s'.\n",filename);  
      exit(0);
  }

  /* readin line by line */
  while(fgets(line,sizeof(line),pFile)!=NULL)
  {
	 tok = strtok(line,DELIM);
	 toLowerCase(tok);
      while(tok!=NULL)
      {  
        if(strchr(tok,'#')!=NULL || strcmp(tok,"\r\n") == SUCCESS ||
			  strcmp(tok,"") == SUCCESS) {
          break;
        }
        else if(strcmp(tok,"server_address") == SUCCESS || 
                strcmp(tok,"address") == SUCCESS) {
           tok = strtok(NULL, DELIM);
			  if((newline = strstr(tok,"\r\n")) != NULL)
				 newline[0] = '\0';
           strcpy(cfg->address,tok);
           break;
        }
        else if(strcmp(tok,"server_name") == SUCCESS) {
           tok = strtok(NULL, DELIM);
        if((newline = strstr(tok,"\r\n")) != NULL)
				 newline[0] = '\0';
           strcpy(cfg->address,tok);
           break;
        }
        else if(strcmp(tok,"server_port") == SUCCESS || 
                strcmp(tok,"port") == SUCCESS ) {
           tok = strtok(NULL, DELIM);
           cfg->port = atoi(tok);
           break;
        }
        else if(strcmp(tok,"print_message_details") == SUCCESS) {
           tok = strtok(NULL, DELIM);
           toLowerCase(tok);
           if(strcmp(tok,"on") == SUCCESS) {
            cfg->print_message_details = TRUE;
            break;
           }
           else if(strcmp(tok,"off") == SUCCESS) {
            cfg->print_message_details = FALSE;
            break;
           }
           
            fprintf(stderr,"please use 'ON' or 'OFF' ");
            fprintf(stderr,"for 'print_message_details'\n");
            break;
        }
        else if(strcmp(tok,"request_count") == SUCCESS) {
           tok = strtok(NULL, DELIM);
           cfg->request_count = atoi(tok);
           break;
        }
        else if(strcmp(tok,"request_timeout") == SUCCESS) {
           tok = strtok(NULL, DELIM);
           cfg->request_timeout = atoi(tok);
           break;
        }
        else if(strcmp(tok,"support_timeout") == SUCCESS) {
           tok = strtok(NULL, DELIM);
           toLowerCase(tok);
           if(strcmp(tok,"on") == SUCCESS) {
            cfg->support_timeout = TRUE;
            break;
           }
           else if(strcmp(tok,"off") == SUCCESS) {
            cfg->support_timeout = FALSE;
            break;
           }
           
            fprintf(stderr,"please use 'ON' or 'OFF' ");
            fprintf(stderr,"for 'support_timeout'\n");
            break;
        }
    
      }
  }  
  fclose(pFile);
}

