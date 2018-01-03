/*
 * tcpserver.h
 *
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_

class TCPServer {

private:
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    byte buffer[1024] = {0};
    const char *hello = "Hello from server";
    int serverportnum;

public:
    void bindToPort(int portnum);
    void acceptClientConnection();
    std::string getMessage();
    int sendMessage(std::string s);
    void closeClientConnection();
    Data getData();
};

#endif /* TCPSERVER_H_ */
