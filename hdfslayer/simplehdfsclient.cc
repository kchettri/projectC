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
		TCPClient tclient;
		tclient.connectToServer("127.0.0.1", 5646);

		string s = createReadMessage("/tmp/first_file");
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
};

int main() {
	SimpleHDFSClient shdfsclient;
	shdfsclient.connectToMaster(); //simple hello protocol
}

