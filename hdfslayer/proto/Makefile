##
## Makefile to compile proto files using protocol buffer
##
##

#include common definitions
include ../Makefile.common

STATICLIBS = libprotobuf.a

PROTOINCLUDE = -I=hadoop.common -I=hadoop.common.client -I=hadoop.hdfs -I=hadoop.hdfs.client

HADOOP_HDFS_PROTOCFILES = hadoop.hdfs/fsimage.proto hadoop.hdfs/editlog.proto
HADOOP_HDFS_PROTOSRCDIR = hadoop.hdfs/src/
HADOOP_HDFS_PROTOSRC = hadoop.hdfs/src/fsimage.pb.cc hadoop.hdfs/src/editlog.pb.cc
HADOOP_HDFS_PROTOOBJS = $(HADOOP_HDFS_PROTOSRC:.cc=.o)

HADOOP_HDFS_CLIENT_PROTOCFILES = hadoop.hdfs.client/acl.proto hadoop.hdfs.client/hdfs.proto hadoop.hdfs.client/xattr.proto
HADOOP_HDFS_CLIENT_PROTOSRCDIR = hadoop.hdfs.client/src/
HADOOP_HDFS_CLIENT_PROTOSRC = hadoop.hdfs.client/src/acl.pb.cc hadoop.hdfs.client/src/hdfs.pb.cc hadoop.hdfs.client/src/xattr.pb.cc
HADOOP_HDFS_CLIENT_PROTOOBJS = $(HADOOP_HDFS_CLIENT_PROTOSRC:.cc=.o)

HADOOP_COMMON_PROTOCFILES = hadoop.common/Security.proto hadoop.common/RpcHeader.proto hadoop.common/IpcConnectionContext.proto
HADOOP_COMMON_PROTOSRCDIR = hadoop.common/src/
HADOOP_COMMON_PROTOSRC = hadoop.common/src/Security.pb.cc hadoop.common/src/RpcHeader.pb.cc hadoop.common/src/IpcConnectionContext.pb.cc
HADOOP_COMMON_PROTOOBJS = $(HADOOP_COMMON_PROTOSRC:.cc=.o)

PROTO_SRC_HEADERS = -I$(HADOOP_HDFS_PROTOSRCDIR) -I$(HADOOP_HDFS_CLIENT_PROTOSRCDIR) -I$(HADOOP_COMMON_PROTOSRCDIR)

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

