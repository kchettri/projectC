/*
 *  simplehdfsclient.cc
 *
 *	Client for simple HDFS
 */

/* C++ headers */
#include <iostream>
#include <vector>

/* C headers */
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

/* locals */
#include "message.h"

using namespace std;

//move it to messgae.cc
const string Message::fieldSeparator = " ";

class TCPClient {

private:
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};

public:

	int connectToServer(const char *hostname, int portnum) {
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			cout << "Socket creation error " << endl;
			return -1;
		}

		memset(&serv_addr, '0', sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(portnum);

		// Convert IPv4 and IPv6 addresses from text to binary form
		if(inet_pton(AF_INET, hostname, &serv_addr.sin_addr)<=0) {
			cout << "\nInvalid address/ Address not supported" << endl;
			return -1;
		}

		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			cout << "\nConnection Failed \n" << endl;
			return -1;
		}
	}

	int sendMessage(string s) {
		send(sock , s.c_str() , s.length() , 0 );
		return 0;
	}

	string readMessage() {
		valread = read(sock , buffer, 1024);
		if (valread < 0) {
			cout << "Read error" << endl;
			exit(1);
		}
		cout << "readMessage returned = " << valread << endl;
		string replyString(buffer);
		return replyString;
	}

};

class SimpleHDFSClient {
private:
	TCPClient tclient;

public:
	string createReadMessage(string fileName) {
		Message msg;
		msg.mtype = READ;
		msg.fileName = fileName;
		return msg.serialize();
	}

	string createWriteMessage(string fileName) {
		Message msg;
		msg.mtype = WRITE;
		msg.fileName = fileName;
		return msg.serialize();
	}

	void printReplyString(string rString) {
		Message msg = Message::deserialize(rString);
		msg.printMessage();
	}

	void connectToMaster() {
		tclient.connectToServer("127.0.0.1", 5646);
	}

	void readFile(string fileName) {
		string s = createReadMessage(fileName);
		cout << "serialized string =" << s << endl;
		tclient.sendMessage(s);
		cout << "File READ message sent" << endl;

		string replyString = tclient.readMessage();
		if (replyString.length() == 0) {
			cout << "Server connection closed." << endl;
		} else {
			printReplyString(replyString);
			cout << "Reply received"   << replyString << endl;
		}
	}

	void cliLoop() {
		connectToMaster();
		string clicommand;
		while(true) {
			cout << "shdfscli# ";
			//cin >> std::noskipws >> clicommand;
			getline(cin, clicommand);
			string originalcliCommand(clicommand.c_str());
			Message msgObj = Message::deserialize(clicommand);

			switch(msgObj.mtype) {
				case READ:
					readFile(msgObj.fileName);
					break;

				case HELP:
					Message::printAllMessageTypes();
					break;

				case UPLOAD:
					//get chunkserver address and upload to chunkserver
					break;

				case EXIT:
					exit(0);
			}

		}
	}
};

int main() {
	SimpleHDFSClient shdfsclient;
	shdfsclient.cliLoop();
}

