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

	int sendMessage(const char *msg) {
		send(sock , msg , strlen(msg) , 0 );
		cout << "Hello message sent" << endl;
		valread = read( sock , buffer, 1024);
		cout << buffer << endl;
		return 0;
	}

};

class SimpleHDFSClient {

public:
	void connectToMaster() {
		TCPClient tclient;
		tclient.connectToServer("127.0.0.1", 5646);
		tclient.sendMessage("Hello from client");
	}
};

int main() {
	SimpleHDFSClient shdfsclient;
	shdfsclient.connectToMaster(); //simple hello protocol
}

