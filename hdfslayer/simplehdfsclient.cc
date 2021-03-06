/*
 *  simplehdfsclient.cc
 *
 *  Client for simple HDFS
 */

/* C++ headers */
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

/* C headers */
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

/* locals */
#include "include/defs.h"
#include "include/message.h"

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
        int sentSize = send(sock , s.c_str() , s.length() , 0 );
        if (sentSize != s.length()) {
            return -1;
        } else {
            return sentSize;
        }
    }

    string readMessage() {
        valread = read(sock , buffer, 1024);
        if (valread < 0) {
            cout << "Read error" << endl;
            exit(1);
        }
        cout << "readMessage returned = " << valread << "bytes" << endl;
        string replyString(buffer);
        return replyString;
    }

    void closeConnection() {
        close(sock);
    }

    void sendData(Data d) {
       int numbytes = send(sock, d.getDataBuf(), d.getLength(), 0);
       if(numbytes <= 0 || numbytes != d.getLength()) {
           cout << "Error in sending databuffer, sentbytes=" << numbytes;
       }
    }
};

class SimpleHDFSClient {
private:
    TCPClient tclient;

public:
    string createReadMessage(string remoteFileName) {
        Message msg;
        msg.mtype = READ;
        msg.remoteFileName = remoteFileName;
        return msg.serialize();
    }

    string createWriteMessage(string remoteFileName) {
        Message msg;
        msg.mtype = WRITE;
        msg.remoteFileName = remoteFileName;
        return msg.serialize();
    }

    string createUploadMessage(string remoteFileName) {
        Message msg;
        msg.mtype = UPLOAD;
        msg.remoteFileName = remoteFileName;
        return msg.serialize();
    }

    string createDownloadMessage(string remoteFileName, string localFileName) {
        Message msg;
        msg.mtype = DOWNLOAD;
        msg.remoteFileName = remoteFileName;
        msg.localFileName = localFileName;
        return msg.serialize();
    }

    void printReplyString(string rString) {
        Message msg = Message::deserialize(rString);
        msg.printMessage();
    }

    void connectToMaster() {
        tclient.connectToServer("127.0.0.1", 5646);
    }

    void uploadFile(string remoteFileName) {
        string s = createUploadMessage(remoteFileName);
        cout << "uploadMessage serialized string ="  << s << endl;
        tclient.sendMessage(s);

        string replyString = tclient.readMessage();
        if (replyString.length() == 0) {
            cout << "Server connection closed." << endl;
        } else {
            //printReplyString(replyString);
            Message msg = Message::deserialize(replyString);

            TCPClient tClientChunkServer;
            tClientChunkServer.connectToServer(msg.chunkServerHostName.c_str(),
                                               stoi(msg.chunkServerPortNum));
            if(tClientChunkServer.sendMessage(s) <= 0) { //resend upload message to chunkserver
                cout << "Error sending UPLOAD message to chunkserver." << endl;
            }

            cout << "Sent message= " << s << endl;
            cout << " Connected to server: " << msg.chunkServerHostName.c_str()
                 << " portNum:" << msg.chunkServerPortNum << endl;

            Data d;
            ifstream finput(remoteFileName, ifstream::binary);

            //read file and send it
            while(true) {
                finput.read((char *)d.getDataBuf(),  Data::BLOCK_SIZE);
                int readLen = finput.gcount();
                if (readLen <= 0) {
                    cout << "Finished uploading file" << endl;
                    break;
                }
                cout << "Read file:" << remoteFileName << " length=" << readLen << endl;
                d.setLength(readLen);
                tClientChunkServer.sendData(d);
            }
            finput.close();
            tClientChunkServer.closeConnection();
        }
    }

    void downloadFile(string remoteFileName, string localFileName) {
        string s = createDownloadMessage(remoteFileName, localFileName);
        cout << "downloadMessage serialized string ="  << s << endl;
        tclient.sendMessage(s);

        string replyString = tclient.readMessage();
        if (replyString.length() == 0) {
            cout << "Server connection closed." << endl;
        } else {
            // connection to chunkserver is created and then terminated
            // after download is complete
            //printReplyString(replyString);
            Message msg = Message::deserialize(replyString);

            TCPClient tClientChunkServer;
            tClientChunkServer.connectToServer(msg.chunkServerHostName.c_str(),
                                               stoi(msg.chunkServerPortNum));
            if(tClientChunkServer.sendMessage(s) <= 0) { //resend download message to chunkserver
                cout << "Error sending DOWNLOAD message to chunkserver." << endl;
            }

            cout << "Sent message= " << s << endl;
            cout << " Connected to server: " << msg.chunkServerHostName.c_str()
                 << " portNum:" << msg.chunkServerPortNum << endl;

            /* Download data  */
            while(true) {
                break;
            }

            tClientChunkServer.closeConnection();
        }
    }

    void readFile(string remoteFileName) {
        string s = createReadMessage(remoteFileName);
        cout << "serialized string =" << s << endl;
        tclient.sendMessage(s);
        cout << "File READ message sent" << endl;

        string replyString = tclient.readMessage();
        if (replyString.length() == 0) {
            cout << "Server connection closed." << endl;
        } else {
            //printReplyString(replyString);

            Message msg = Message::deserialize(replyString);

            TCPClient tClientChunkServer;

            tClientChunkServer.connectToServer(msg.chunkServerHostName.c_str(),
                                               stoi(msg.chunkServerPortNum));

            tClientChunkServer.sendMessage(s);

            replyString = tClientChunkServer.readMessage();
            tClientChunkServer.closeConnection();

            cout << "Reply received from chunkServer="   << replyString << endl;
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
                    readFile(msgObj.remoteFileName);
                    break;

                case HELP:
                    Message::printAllMessageTypes();
                    break;

                case LIST:
                    break;

                case UPLOAD:
                    //get chunkserver address and upload to chunkserver
                    cout << "UPloading file: "  << msgObj.remoteFileName << endl;
                    uploadFile(msgObj.remoteFileName);
                    break;

                case DOWNLOAD:
                    cout << "Downloading file: "  << msgObj.remoteFileName << endl;
                    downloadFile(msgObj.remoteFileName, msgObj.localFileName);
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

