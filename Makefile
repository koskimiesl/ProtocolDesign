CC = g++
CFLAGS = -std=c++98

objects_client = comm.o icp.o client.o state.o helpers.o
objects_server = comm.o icp.o server.o state.o helpers.o pubserver.o sensormsg.o

objects = comm.o icp.o state.o client.o server.o helpers.o pubserver.o sensormsg.o

PROGS = server client

all:$(PROGS)

server:$(objects_server)
	$(CC) $(CFLAGS) -pthread -o server $(objects_server)

client:$(objects_client)
	$(CC) $(CFLAGS) -o client $(objects_client)

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
	
pubserver.o:pubserver.cc
	$(CC) -c $^ $(CFLAGS)
	
sensormsg.o:sensormsg.cc
	$(CC) -c $^ $(CFLAGS)

.PHONY :clean

clean:
	rm server client $(objects)
