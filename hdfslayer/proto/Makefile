##
## Makefile to compile proto files using protocol buffer
##
##

#include common definitions
include ../Makefile.common

STATICLIBS = libprotobuf.a

# Location of the proto files
PROTOINCLUDE = \
    -I=$(INCLUDEROOTFOLDER)/proto/hadoop.common \
    -I=$(INCLUDEROOTFOLDER)/proto/hadoop.common.client \
    -I=$(INCLUDEROOTFOLDER)/proto/hadoop.hdfs \
    -I=$(INCLUDEROOTFOLDER)/proto/hadoop.hdfs.client

MAIN = main

all: $(MAIN)

MAINOBJ = main.o

$(MAIN): $(HADOOP_COMMON_PROTOOBJS) $(HADOOP_HDFS_CLIENT_PROTOOBJS) $(HADOOP_HDFS_PROTOOBJS) $(MAINOBJ)
	g++ -g -o main $^ $(STATICLIBS) $(LDFLAGS) $(PROTO_SRC_HEADERS)

%.o: %.cc
	g++ -g -c -o $@ $< $(CFLAGS) $(PROTO_SRC_HEADERS) $(INCLUDE)

## hadoop.hdfs proto files
$(HADOOP_HDFS_PROTOSRC): $(HADOOP_HDFS_PROTOCFILES)
	protoc --cpp_out=$(HADOOP_HDFS_PROTOSRCDIR) $(PROTOINCLUDE) $(HADOOP_HDFS_PROTOCFILES)

## hadoop.hdfs.client proto files
$(HADOOP_HDFS_CLIENT_PROTOSRC): $(HADOOP_HDFS_CLIENT_PROTOCFILES)
	protoc --cpp_out=$(HADOOP_HDFS_CLIENT_PROTOSRCDIR) $(PROTOINCLUDE) $(HADOOP_HDFS_CLIENT_PROTOCFILES)

## hadoop.common proto files
$(HADOOP_COMMON_PROTOSRC): $(HADOOP_COMMON_PROTOCFILES)
	protoc --cpp_out=$(HADOOP_COMMON_PROTOSRCDIR) $(PROTOINCLUDE) $(HADOOP_COMMON_PROTOCFILES)

clean: 
	rm $(MAIN) $(HADOOP_HDFS_PROTOSRCDIR)/*  $(HADOOP_HDFS_CLIENT_PROTOSRCDIR)/* $(HADOOP_COMMON_PROTOSRCDIR)/*

cleanmain: 
	rm main

