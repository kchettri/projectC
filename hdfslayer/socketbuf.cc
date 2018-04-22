/*
 	socketbuf used for istream to read from socket. 

	Refer to socketbuf C++ doc to know more about the functions 
	to override and their details.
 */

/* Standard C++ header */
#include <iostream>

/* C headers/functions in a different namespace */
namespace cstd {
  #include <unistd.h>
  #include <sys/socket.h>
  #include <stdlib.h>
  #include <netinet/in.h>
}


/* projectC local */
#include "include/socketbuf.h"

using namespace std;

void
socketbuf::setSocketfd(int socketfd) {
	this->socketfd = socketfd;
}

streambuf* 
socketbuf::setbuf(char* s, streamsize n) {
	
	int valread = cstd::read(socketfd, s, n);
	if(valread <= 0) {
        cout << "setbuf No data read from socket" << endl;
    }
	return this;
}

int
socketbuf::underflow() {
	byte b; 
	int valread = cstd::read(socketfd, &b, 1);

	if(valread <= 0) {
        cout << "No data read from socket" << endl;
		return std::char_traits<char>::eof();
    }

	return (int)b;
	//if the socket is close then return eof() 
	//char_traits<char>::eof().
}

int 
socketbuf::uflow() {
	return underflow();
}

