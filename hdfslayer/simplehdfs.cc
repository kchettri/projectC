/*
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

class TCPServer {

private:
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	byte buffer[1024] = {0};
	const char *hello = "Hello from server";
	int serverportnum;

public:
	void bindToPort(int portnum) {
		int opt = 1;
		serverportnum = portnum;
		// Creating socket file descriptor
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
			cout << "socket failed";
			exit(EXIT_FAILURE);
		}
		// Forcefully attaching socket to the port 5646
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
													  &opt, sizeof(opt))) {
			cout << "setsockopt";
			exit(EXIT_FAILURE);
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(portnum);

		// Forcefully attaching socket to the port 5646
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
			cout << "bind failed";
			exit(EXIT_FAILURE);
		}
	}

	void acceptClientConnection() {
	    int addrlen = sizeof(address);
        if (listen(server_fd, 3) < 0) {
            cout << "listen";
            exit(EXIT_FAILURE);
        }
        //cout << "Listening on the socket on port: " << serverportnum << endl;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen))<0) {
            cout << "accept";
            exit(EXIT_FAILURE);
        }
	}

	string getMessage() {
	    valread = read(new_socket , buffer, 1024);

	    if(valread <= 0) {
	        return "";
	    }

		string bufferString(buffer);
		return bufferString;
	}

	int sendMessage(string s) {
		return send(new_socket , s.c_str() , s.length() , 0 );
	}

	void closeClientConnection() {
	    close(new_socket);
	}

	/* storage formats already encode the data here, all we do is store it as it is received in the
	 * file
	 */
	Data getData() {
	   Data d;
	   valread = read(new_socket , d.getDataBuf(), d.BLOCK_SIZE);
	   d.setLength(valread);
       return d;
	}
};

class Logger {
public:
    void log(string str) {
        cout << str << endl << flush;
    }
};

class SimpleHDFSChunkServer: public Logger {

/* Format of metadata:
 *
 *	filename | chunkid | localfilename
 */
private:
	string chunkserverName;
	int nameCounter;
	string chunkroot, chunkdataroot;
	string getNewFileName() {}
	string portNum;
	int portNumInt;
	TCPServer tserver;

public:

	SimpleHDFSChunkServer(string pnum) {
		portNum = pnum;
		portNumInt = stoi(portNum);
		chunkserverName = "127.0.0.1";
		tserver.bindToPort(portNumInt);

		chunkroot = "/home/projectC/hdfsroot";
        chunkdataroot = chunkroot + "/dataroot";
	}

	void init() {
		chunkroot = "/home/projectC/hdfsroot";
		chunkdataroot = chunkroot + "/dataroot/";
		//metadata file
		ofstream mfile;
		mfile.open((chunkroot + "/mfile").c_str());
		mfile << "Init text";
		mfile.close();
	}

	void chunkMain() {
		while(true) {
            log("HDFSChunkServer: Chunkserver accepting connection at port: "
                 + portNum);
            tserver.acceptClientConnection();

            while(true) {
                //data transfer loop
                log("HDFSChunkServer: Waiting for Message in ChunkServer");
                string str = tserver.getMessage();
                //end of message
                if(str.length() == 0) {
                    log("HDFSChunkServer: Zero length string returned. Terminating client connection. ");
                    break;
                }
                string originalMessage(str.c_str());

                //strtok changes the string that it tokenizes
                Message clientMessage = Message::deserialize(str);
                clientMessage.printMessage();

                Message replyMessage;
                string replyMessageString;

                switch(clientMessage.mtype) {
                    case READ:
                        replyMessage = readFile(clientMessage);
                        replyMessageString = replyMessage.serialize();
                        log("HDFSChunkServer: replyMessageString=" + replyMessageString);
                        tserver.sendMessage(replyMessage.serialize());
                        break;

                    case WRITE:
                        break;

                    case LIST: //this can only happen in Master
                        break;

                    case UPLOAD:
                        uploadFile(clientMessage.fileName);
                        break;
                }
            }
            tserver.closeClientConnection();
		}
	}

	void sendMessage(Message msg) {
	    string msgString = msg.serialize();
        log("HDFSChunkServer: sending Message =" + msgString);
        tserver.sendMessage(msgString);
	}

	void uploadFile(string fileName) {
	    //TODO: send status
	    // - should send error in case space is not available
	    // - should send error in case of other conditions


        //currently only append to the file;
        //later support inserts
        ofstream foutput (chunkdataroot + fileName, ofstream::binary);
        while(true) {
            Data d = tserver.getData();
            if (d.getLength() <= 0) {
                foutput.write((const char *)d.getDataBuf(), d.getLength());
            }
        }
        foutput.close();
	}

	Message readFile(Message requestMessage) {
		Message replyMessage;

		//read the actual file, for now just send the status
		replyMessage.mtype = STATUS;
		replyMessage.statusString = "ReadMessage called for file: " + requestMessage.fileName;
		return replyMessage;
	}

	string getHostName() {
		return "127.0.0.1";
	}

	int getPortNumInt() {
		return portNumInt;
	}

	string getPortNum() {
		return portNum;
	}

	string getChunkServerName() {
		return chunkserverName;
	}

	void read() {}

	void write() {}
};


class SimpleHDFSMaster: public Logger {

/* Format of hdfs metdata:
 *
 *  filename | filepath | chunkid | chunkserver
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
                        replyMessage = readFile(clientMessage);
                        replyMessageString = replyMessage.serialize();
                        log("HDFSMaster: replyMessageString=" + replyMessageString);
                        tserver.sendMessage(replyMessage.serialize());
                        break;

                    case WRITE:
                        break;

                    case LIST:
                        break;

                    case UPLOAD:
                        replyMessage = uploadFile(clientMessage);
                        replyMessageString = replyMessage.serialize();
                        log("HDFSMaster: replyMessageString=" + replyMessageString);
                        tserver.sendMessage(replyMessage.serialize());
                        break;
                }
            }
            tserver.closeClientConnection();
		}

		chunkServerThread.join();
	}

	SimpleHDFSChunkServer getChunkServer(string fileName) {
		return chunkServers.at(0);
	}

	Message getCSDiscoverMessage(Message requestMessage) {
	    Message replyMessage;
        SimpleHDFSChunkServer cserver = getChunkServer(requestMessage.fileName);

        replyMessage.mtype = CSDISCOVER;
        replyMessage.chunkServerHostName = cserver.getChunkServerName();
        replyMessage.chunkServerPortNum = cserver.getPortNum();
        return replyMessage;
	}

	Message uploadFile(Message requestMessage) {
	    return getCSDiscoverMessage(requestMessage);
	}

	Message readFile(Message requestMessage) {
	    return getCSDiscoverMessage(requestMessage);
	}

	void writeFile() {}

};

int  main() {
	SimpleHDFSMaster shdfsmaster;
	shdfsmaster.init();
	shdfsmaster.serverMain();
	return 0;
}

