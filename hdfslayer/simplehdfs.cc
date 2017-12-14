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

/*
 * C headers
 */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

/* locals */
#include "message.h"

using namespace std;

//move it to message.cc
const string Message::fieldSeparator = " ";

class TCPServer {

private:
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	char buffer[1024] = {0};
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

	string getMessage() {
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
		valread = read(new_socket , buffer, 1024);
		string bufferString(buffer);
		return bufferString;
	}

	void sendMessage(string s) {
		send(new_socket , s.c_str() , s.length() , 0 );
	}

	/* storage formats already encode the data here, all we do is store it as it is received in the
	 * file
	 */
	string getData() {}
};

class SimpleHDFSChunkServer {

/* Format of metadata:
 *
 *	filename | chunkid | localfilename
 */
private:
	string chunkserverName;
	int nameCounter;
	string chunkroot, chunkdataroot;
	string getNewFileName() {}
	int portNum;

public:

	SimpleHDFSChunkServer(int pnum) {
		portNum = pnum;
	}

	void init() {
		chunkroot = "/home/projectC/hdfsroot";
		chunkdataroot = chunkroot + "/dataroot";
		//metadata file
		ofstream mfile;
		mfile.open((chunkroot + "/mfile").c_str());
		mfile << "Init text";
		mfile.close();
	}

	void chunkMain() {
		TCPServer tserver;
		tserver.bindToPort(portNum);
		cout << "Chunkserver listening at port: " << portNum << endl;
		while(true) {
			//data transfer loop
			string str = tserver.getMessage();
			string originalMessage(str.c_str());

			//strtok changes the string that it tokenizes
			Message clientMessage = Message::deserialize(str);
			clientMessage.printMessage();
		}
	}

	void read() {}

	void write() {}
};


class SimpleHDFSMaster {

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

	void LookUpFile(string fileName) {

	}

	void serverMain() {
		TCPServer tserver;
		tserver.bindToPort(HDFSPORT);
		cout << "HDFSmaster listening at port: " << HDFSPORT << endl;

		/* Create chunkserver threads */
		SimpleHDFSChunkServer chunkServer(6800);
		thread chunkServerThread(&SimpleHDFSChunkServer::chunkMain, chunkServer);

		//Message Loop
		while (true) {
			string str = tserver.getMessage();
			string originalMessage(str.c_str());

			//strtok changes the string that it tokenizes
			Message clientMessage = Message::deserialize(str);
			clientMessage.printMessage();

			switch(clientMessage.mtype) {
				case READ:
					break;

				case WRITE:
					break;

				case LIST:
					break;

				case UPLOAD:
					break;
			}
		}
		chunkServerThread.join();
	}

	void readFile() {

	}

	void writeFile() {}

};

int  main() {
	SimpleHDFSMaster shdfsmaster;
	shdfsmaster.init();
	shdfsmaster.serverMain();
	return 0;
}

