/*
 * Copyright
 *
 * simplehdfs.cc
 *
 *  Simple HDFS server.
 */

/* C++ header */
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <fstream>
#include <sstream>

/* boost headers */
#include <boost/filesystem.hpp>

/*
 * C headers
 */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

/* locals */
#include "defs.h"
#include "message.h"

using namespace std;

class SimpleHDFSMaster: public Logger {

/* Format of hdfs metdata:
 *
 *  remoteFileName | filepath | chunkid | chunkserver
 */

private:
	vector<string> chunkservers;
	string hdfsroot, dataroot;
	const int HDFSPORT = 5646;
	vector<SimpleHDFSChunkServer> chunkServers;

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

	void serverMain() {
		TCPServer tserver;
		tserver.bindToPort(HDFSPORT);

		/* Create chunkserver threads */
		SimpleHDFSChunkServer chunkServer("6800");
		thread chunkServerThread(&SimpleHDFSChunkServer::chunkMain, chunkServer);
		chunkServers.push_back(chunkServer);
		Message replyMessage;
		string replyMessageString;

		while (true) {
            log("HDFSmaster: listening to client connection at port: " + to_string(HDFSPORT));
            //Currently hdfsmaster only accepts one client connection at a time.
            //TODO: Add support to accept multiple client connections.
            tserver.acceptClientConnection();

            //Message Loop
            while (true) {
                log("HDFSMaster: Waiting for Message in Master");
                string str = tserver.getMessage();

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

	SimpleHDFSChunkServer getChunkServer(string remoteFileName) {
	    /*
	     * Find the right chunkserver instead of returning just the one right now.
	     */
		return chunkServers.at(0);
	}

	Message getCSDiscoverMessage(Message requestMessage) {
	    Message replyMessage;
        SimpleHDFSChunkServer cserver = getChunkServer(requestMessage.remoteFileName);

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
	SimpleHDFSMaster shdfsmaster;
	shdfsmaster.init();
	shdfsmaster.serverMain();
	return 0;
}

