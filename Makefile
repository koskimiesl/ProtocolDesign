CPP = g++
CC = gcc
CFLAGS = -lncurses -pthread

objects_client = comm.o client.o screen.o
objects_clientb = icp.o state.o helpers.o clientb.o
objects_server = comm.o sensormsg.o server.o 
objects_serverb = icp.o state.o helpers.o serverb.o list.o

objects = comm.o icp.o state.o client.o server.o helpers.o sensormsg.o screen.o serverb.o clientb.o list.o

PROGS = server client serverb clientb

all:$(PROGS)

server:$(objects_server)
	$(CPP) -o server $(objects_server) $(CFLAGS) 

client:$(objects_client)
	$(CPP) -o client $(objects_client) $(CFLAGS)

serverb:$(objects_serverb)
	$(CC) -o serverb $(objects_serverb) $(CFLAGS)

clientb:$(objects_clientb)
	$(CC) -o clientb $(objects_clientb) $(CFLAGS)

serverb.o:serverb.c
	$(CC) -c $^ $(CFLAGS)

server.o:server.cc
	$(CPP) -c $^ $(CFLAGS)

client.o:client.cc
	$(CPP) -c $^ $(CFLAGS)

clientb.o:clientb.c
	$(CC) -c $^ $(CFLAGS)

comm.o:comm.cc
	$(CPP) -c $^ $(CFLAGS)

icp.o:icp.c
	$(CC) -c $^ $(CFLAGS)

state.o:state.c
	$(CC) -c $^ $(CFLAGS) 

helpers.o:helpers.c
	$(CC) -c $^ $(CFLAGS)

list.o:list.c
	$(CC) -c $^ $(CFLAGS)

sensormsg.o:sensormsg.cc
	$(CPP) -c $^ $(CFLAGS)

screen.o:screen.cc
	$(CPP) -c $^ $(CFLAGS)

.PHONY :clean

clean:
	rm server serverb client clientb $(objects)

