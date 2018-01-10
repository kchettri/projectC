/*
 * tcpserver.cc
 *
 *  Created on: Jan 2, 2018
 *      Author: kamal
 */

/*
 * C headers
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

/* C++ headers */
#include <iostream>

/* locals */
#include "tcpserver.h"

using namespace std;

void
TCPServer::bindToPort(int portnum) {
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

void
TCPServer::acceptClientConnection() {
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

string
TCPServer::getMessage() {
    valread = read(new_socket, buffer, 1024);

    if(valread <= 0) {
        cout << "TCPServer returned zero length buffer" << endl;
        return "";
    }

    string bufferString((const char *)buffer);
    return bufferString;
}

int
TCPServer::sendMessage(string s) {
    return send(new_socket , s.c_str() , s.length() , 0 );
}

void
TCPServer::closeClientConnection() {
    close(new_socket);
}

/* storage formats already encode the data here, all we do is store it as it is received in the
 * file
 */
Data
TCPServer::getData() {
   Data d;
   valread = read(new_socket , d.getDataBuf(), d.BLOCK_SIZE);

   cout << "Data read = " << valread << endl;
   d.setLength(valread);
   return d;
}




