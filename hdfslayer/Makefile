##
## Makefile for master, chunkserver, and client
##
##

## include common compilation/linker flag definitions
include Makefile.common

COMMONSRCS = message.cc simplerw.cc socketbuf.cc
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

EDITLOG = bin/editlog
EDITLOGSRCS = editlog.cc
EDITLOGDEPS = 
EDITLOGOBJS = $(EDITLOGSRCS:.cc=.o)

SUBDIRS=proto

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS) $(MASTER) $(CLIENT) $(EDITLOG)

$(SUBDIRS):
	make -C $@

%.o: %.cc
	g++ -g -c -o $@ $< $(CFLAGS) $(INCLUDE)

$(MASTER): $(MASTEROBJS) $(COMMONOBJS)
	g++ -o $@ $^ $(LDFLAGS)

$(CLIENT): $(CLIENTOBJS) $(COMMONOBJS)
	g++ -o $@ $^ $(LDFLAGS)	

$(EDITLOG): $(EDITLOGOBJS) $(COMMONOBJS) 
	g++ -g -o $@ $^ $(PROTOBUF_OBJS) $(LDFLAGS)	$(STATICLIBS) 

clean: 
	rm $(MASTER) $(CLIENT) $(MASTEROBJS) $(CLIENTOBJS) $(COMMONOBJS) $(EDITLOGOBJS)
	make clean -C $(SUBDIRS)

