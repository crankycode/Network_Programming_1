#used to compile normal C code
CC = gcc

#compile flags for C code
CFLAGS = -ansi -pedantic -Wall -Idir -Ldir
CLIBS = -lsocket -lnsl
#Debug with gdb
DEBUG = -g

#The program we are compiling
SERVER = timeserver.c
SERVER_NAME = timeserver

CLIENT = myconfiglib.c timeclient.c
CLIENT_NAME = timeclient
CLIENT_HEADER = myconfiglib.h

#Make commands for C

all:
	$(CC) $(CFLAGS) -o $(CLIENT_NAME) $(CLIENT) $(CLIENT_HEADER) $(CLIBS) $(DEBUG)

#Clean exe file
clean:
	rm $(SERVER_NAME) $(CLIENT_NAME)
