#ifndef _MYCONFIGLIB_H
#define _MYCONFIGLIB_H

/* System-wide header files. */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define DELIM " "
#define FALSE   0
#define TRUE    1
#define SUCCESS 0

typedef struct configfile {
  char      address[50];
  int       print_message_details;
  int       port;
  int       request_count;
  int       request_timeout;
  int       support_timeout;
} Configfile;

void parseConfigFile(char* filename, Configfile* cfg);
void toLowerCase(char* str);
#endif
