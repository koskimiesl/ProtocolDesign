CC = g++
CFLAGS = -lncurses -std=c++98 -pthread

objects_client = comm.o icp.o state.o helpers.o client.o screen.o
objects_server = comm.o icp.o state.o helpers.o sensormsg.o server.o 

objects = comm.o icp.o state.o client.o server.o helpers.o sensormsg.o screen.o

PROGS = server client

all:$(PROGS)

server:$(objects_server)
	$(CC) -o server $(objects_server) $(CFLAGS) 

client:$(objects_client)
	$(CC) -o client $(objects_client) $(CFLAGS)

server.o:server.cc
	$(CC) -c $^ $(CFLAGS)

client.o:client.cc
	$(CC) -c $^ $(CFLAGS)

comm.o:comm.cc
	$(CC) -c $^ $(CFLAGS)

icp.o:icp.cc
	$(CC) -c $^ $(CFLAGS)

state.o:state.cc
	$(CC) -c $^ $(CFLAGS) 

helpers.o:helpers.cc
	$(CC) -c $^ $(CFLAGS)

sensormsg.o:sensormsg.cc
	$(CC) -c $^ $(CFLAGS)

screen.o:screen.cc
	$(CC) -c $^ $(CFLAGS)

.PHONY :clean

clean:
	rm server client $(objects)

