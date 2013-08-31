CC= g++
LD= g++
CC_FLAGS = -g -ansi -Wall -c
LD_FLAGS = -g -Wall

all: t-server ed-server

t-server: t-server.o common.o
	$(LD) $(LD_FLAGS) common.o t-server.o -lpthread -o 551server_threaded
ed-server: ed-server.o common.o socketlist.o
	$(LD) $(LD_FLAGS) common.o socketlist.o ed-server.o -o 551server_eventdriven

socketlist.o: socketlist.c
	$(CC) $(CC_FLAGS) socketlist.c
t-server.o: t-server.c
	$(CC) $(CC_FLAGS) t-server.c
ed-server.o: ed-server.c
	$(CC) $(CC_FLAGS) ed-server.c
common.o: common.c
	$(CC) $(CC_FLAGS) common.c

clean:
	rm -f *.o 551server_threaded 551server_eventdriven
