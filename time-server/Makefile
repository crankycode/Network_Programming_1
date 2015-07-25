#used to compile normal C code
CC = gcc

#compile flags for C code
CFLAGS = -ansi -pedantic -Wall -Idir -Ldir
CLIBS = -lsocket -lnsl
#Debug with gdb
DEBUG = -g

#The program we are compiling
SERVER = timeserver.c myconfiglib.c
SERVER_NAME = timeserver
SERVER_HEADER = myconfiglib.h

#Make commands for C

all:
	$(CC) $(CFLAGS) -o $(SERVER_NAME) $(SERVER) $(SERVER_HEADER) $(CLIBS) $(DEBUG)

#Clean exe file
clean:
	rm $(SERVER_NAME)
