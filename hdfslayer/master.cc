/*
 * Copyright
 *  master.cc
 * Simple HDFS server master.
 */

using namespace std;

/* C++ header */
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <sstream>

/* boost headers */
#include <boost/filesystem.hpp>

/* locals */
#include "include/defs.h"
#include "include/message.h"
#include "include/logger.h"
#include "include/tcpserver.h"
#include "include/chunkserver.h"
#include "include/simplerw.h"

/* RPC protobuf datastructures */
#include "proto/hadoop.common/src/IpcConnectionContext.pb.h"
#include "proto/hadoop.common/src/RpcHeader.pb.h"

/* protocol buffer delimited parser support */
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/delimited_message_util.h>


class Master: public Logger {

/* Format of hdfs metdata:
 * remoteFileName | filepath | chunkid | chunkserver
 */
private:
	vector<string> chunkservers;
	string hdfsroot, dataroot;
	const int HDFSPORT = 9000; //from 
	vector<ChunkServer> chunkServers;

public:
	void init() {
		hdfsroot = "/home/projectC/hdfsroot";
		dataroot = hdfsroot + "/dataroot";
		//metadata file
		ofstream mfile;
		mfile.open((hdfsroot + "/mfile").c_str());
		mfile << "Init text";
		mfile.close();
		chunkservers.push_back("localhost");

		loginit("HDFSMaster");
	}

	void getHeader(SimpleReader& sReader) {
		int buffer_size = 1024;
		int intVar = 0;

		byte* buffer = new byte[buffer_size];
		
		/*
		  Client sends this header when connection is established. 

	     * Write the connection header - this is sent when connection is established
    	 * +----------------------------------+
		 * |  "hrpc" 4 bytes                  |      
     	 * +----------------------------------+
     	 * |  Version (1 byte)                |
     	 * +----------------------------------+
     	 * |  Service Class (1 byte)          |
     	 * +----------------------------------+
      	 * |  AuthProtocol (1 byte)           |      
     	 * +----------------------------------+
 		*/


		int read_size = 4;
        intVar = sReader.readByteArray(buffer, read_size); 

		log("ByteArray size read =" + to_string(intVar));
		if(intVar > 0) {
			cout << "magic:"; 
			for (int i=0; i < 4; i++) {
				cout << buffer[i];
			}				
			cout << endl;
		 }
	
		read_size = 1;
        if(sReader.readByteArray(buffer, read_size) >= 0) { 
			cout << "version: " << (int) buffer[0] << endl;	
		}

		read_size = 1;
        if(sReader.readByteArray(buffer, read_size) >= 0) { 
			cout << "service class: " << (int) buffer[0] << endl;	
		}

		read_size = 1;
        if(sReader.readByteArray(buffer, read_size) >= 0) { 
			cout << "authprotocol: " << (int) buffer[0] << endl;	
		}
	}

	void serverMain() {
		TCPServer tserver;
		tserver.bindToPort(HDFSPORT);

		//temp variables to use for SimpleReader
		int intVal;
		int16 int16Val;
		long64 longVal;
		byte byteVal;
		string str;

		/* Create chunkserver threads */
		ChunkServer chunkServer("6800");
		thread chunkServerThread(&ChunkServer::chunkMain, chunkServer);
		chunkServers.push_back(chunkServer);
		Message replyMessage;
		string replyMessageString;
		socketbuf sockBuf; 
		SimpleReader socketReader;
		SimpleWriter sWriter;

		while (true) {
            log("HDFSmaster: listening to client connection at port: " + to_string(HDFSPORT));
            //Currently hdfsmaster only accepts one client connection at a time.
            //TODO: Add support to accept multiple client connections.
            tserver.acceptClientConnection();
			sockBuf.setSocketfd(tserver.getSocketfd());
			socketReader.init(sockBuf);
			getHeader(socketReader);

			socketReader.readIntBigEndian(&intVal);
			cout << "Length = " << intVal << endl;

			//SASL support is present in the existing Hadoop client/server 
			//Authentication is not supported currently for submitting jobs (Kerberos based authentication)

			//Connection context is next: 
			//RpcRequestHeaderProto, followed by  IpcConnectionContextProto
			hadoop::common::RpcRequestHeaderProto requestHeaderPB;
			socketReader.readDelimitedFrom(&requestHeaderPB);
			
			if(requestHeaderPB.has_clientid()) {
				cout << "clientid=" ; 
				string clientid = requestHeaderPB.clientid();
				for (int i=0; i < clientid.length(); i++) {
					if ((unsigned int)clientid.at(i) < 10) cout << "0";
					cout << std::hex << (+clientid.at(i) & 0xFF);
					//cout << "Client id=" << requestHeaderPB.clientid() << endl;
				}
				cout << std::dec << endl;
				cout << "clientid length=" << clientid.length() << endl;

			} else {
				cout << "No client id in requestHeaderPB" << endl;
			}

			if(requestHeaderPB.has_callid()) {
				cout << "Call id=" << requestHeaderPB.callid() << endl;
			} else {
				cout << "No call id in requestHeaderPB" << endl;
			}


            //Message Loop
            while (true) {
                log("HDFSMaster: Waiting for Message in Master");

				string str="";
				//string str = tserver.getMessage();

                if (str.length() == 0) {
                    log("HDFSMaster: zero length string sent from client connection. Closing.");
                    break;
                }

                string originalMessage(str.c_str());

                //strtok changes the string that it tokenizes
                Message clientMessage = Message::deserialize(str);
                clientMessage.printMessage();

                switch(clientMessage.mtype) {
                    case READ:
                    case UPLOAD:
                    case DOWNLOAD:
                        replyMessage = getCSDiscoverMessage(clientMessage);  //readFile(clientMessage);
                        replyMessageString = replyMessage.serialize();
                        log("HDFSMaster: replyMessageString=" + replyMessageString);
                        tserver.sendMessage(replyMessage.serialize());
                        break;

                    case WRITE:
                        break;

                    case LIST:
                        break;
                }
            }
            tserver.closeClientConnection();
		}
		chunkServerThread.join();
	}

	ChunkServer getChunkServer(string remoteFileName) {
	    /*
	     * Find the right chunkserver instead of returning just the one right now.
	     */
		return chunkServers.at(0);
	}

	Message getCSDiscoverMessage(Message requestMessage) {
	    Message replyMessage;
        ChunkServer cserver = getChunkServer(requestMessage.remoteFileName);

        /* chunkserver discover will involve identifying the right chunk servers
         * among all the ones available.
         */
        switch(requestMessage.mtype) {
            case UPLOAD:
                break;

            case DOWNLOAD:
                break;
        }

        replyMessage.mtype = CSDISCOVER;
        replyMessage.chunkServerHostName = cserver.getChunkServerName();
        replyMessage.chunkServerPortNum = cserver.getPortNum();
        return replyMessage;
	}
};

int  main() {

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	Master shdfsmaster;
	shdfsmaster.init();
	shdfsmaster.serverMain();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;

}

