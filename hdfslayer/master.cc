/*
 * Copyright
 *  master.cc
 * Simple HDFS server master.
 */

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

using namespace std;

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

	void getHeader(TCPServer& tserver) {
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
        intVar = tserver.getByteArray(buffer, read_size); 

		log("ByteArray size read =" + to_string(intVar));
		if(intVar > 0) {
			cout << "magic:"; 
			for (int i=0; i < 4; i++) {
				cout << buffer[i];
			}				
			cout << endl;
		 }
	
		read_size = 1;
        if(tserver.getByteArray(buffer, read_size) > 0) { 
			cout << "version: " << (int) buffer[0] << endl;	
		}

		read_size = 1;
        if(tserver.getByteArray(buffer, read_size) > 0) { 
			cout << "service class: " << (int) buffer[0] << endl;	
		}

		read_size = 1;
        if(tserver.getByteArray(buffer, read_size) > 0) { 
			cout << "authprotocol: " << (int) buffer[0] << endl;	
		}

				
	}

	void serverMain() {
		TCPServer tserver;
		tserver.bindToPort(HDFSPORT);

		/* Create chunkserver threads */
		ChunkServer chunkServer("6800");
		thread chunkServerThread(&ChunkServer::chunkMain, chunkServer);
		chunkServers.push_back(chunkServer);
		Message replyMessage;
		string replyMessageString;

		while (true) {
            log("HDFSmaster: listening to client connection at port: " + to_string(HDFSPORT));
            //Currently hdfsmaster only accepts one client connection at a time.
            //TODO: Add support to accept multiple client connections.
            tserver.acceptClientConnection();
			getHeader(tserver);

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
	Master shdfsmaster;
	shdfsmaster.init();
	shdfsmaster.serverMain();
	return 0;
}

