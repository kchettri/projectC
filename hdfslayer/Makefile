##
## Common makefile for master, chunkserver, and client
##
##

LDFLAGS = -lpthread -lboost_system -lboost_filesystem -std=c++11
CFLAGS = -std=c++11
INCLUDE = -I.

COMMONSRCS = message.cc
COMMONOBJS = $(COMMONSRCS:.cc=.o)

CHUNKSERVER = bin/chunkserver
CHUNKSRCS = chunkserver.cc
CHUNKDEPS = ## Add headers
CHUNKOBJS = $(CHUNKSRCS:.cc=.o)

MASTER = bin/master
MASTERSRCS = master.cc tcpserver.cc logger.cc ${CHUNKSRCS}
MASTERDEPS = ## Add headers
MASTEROBJS = $(MASTERSRCS:.cc=.o) 

CLIENT = bin/simplehdfsclient 
CLIENTSRCS = simplehdfsclient.cc
CLIENTDEPS = ## Add headers
CLIENTOBJS = $(CLIENTSRCS:.cc=.o)


all: $(MASTER) $(CLIENT) 

%.o: %.cc
	g++ -c -o $@ $< $(CFLAGS) $(INCLUDE)

$(MASTER): $(MASTEROBJS) $(COMMONOBJS)
	g++ -o $@ $^ $(LDFLAGS)

$(CLIENT): $(CLIENTOBJS) $(COMMONOBJS)
	g++ -o $@ $^ $(LDFLAGS)	

clean: 
	rm $(MASTER) $(CLIENT) $(MASTEROBJS) $(CLIENTOBJS) $(COMMONOBJS)

