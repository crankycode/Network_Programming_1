#used to compile normal C code
CC = gcc

#compile flags for C code
CFLAGS = -ansi -pedantic -Wall -Idir -Ldir
CLIBS = -lsocket -lnsl
#Debug with gdb
DEBUG = -g

#The program we are compiling

CLIENT = gethttp.c
CLIENT_NAME = gethttp

#Make commands for C
all:
	$(CC) $(CFLAGS) -o $(CLIENT_NAME) $(CLIENT) $(CLIBS) $(DEBUG)

#Clean exe file
clean:
	rm $(CLIENT_NAME)
