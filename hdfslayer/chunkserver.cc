/*
 * chunkserver.cc
 *
 */

/* C++  */
#include <iostream>
#include <fstream>

/* boost headers */
#include <boost/filesystem.hpp>

/* locals */
#include "include/chunkserver.h"
#include "include/message.h"
#include "include/tcpserver.h"

using namespace std;

/* Format of metadata:
 *
 *  remoteFileName | chunkid | localfilename
 */
ChunkServer::ChunkServer(string pnum) {
    portNum = pnum;
    portNumInt = stoi(portNum);

    //bind to portNumber here:
    chunkserverName = "127.0.0.1";
    tserver.bindToPort(portNumInt);

    chunkroot = "/home/projectC/hdfsroot";
    chunkdataroot = chunkroot + "/dataroot";

    loginit("ChunkServer");
}

void ChunkServer::init() {
    chunkroot = "/home/projectC/hdfsroot";
    chunkdataroot = chunkroot + "/dataroot/";
    //metadata file
    ofstream mfile;
    mfile.open((chunkroot + "/mfile").c_str());
    mfile << "Init text";
    mfile.close();
}

void ChunkServer::chunkMain() {
    /*
     *  Listen for connection.
     *  - Accept connection from client, these connections are redirected from master.
     *  - Client always goes to master first to see which chunkserver to connect to.
     *  - After a connection is accepted, see what message it is requesting.
     *  - Based on that message, do the processing.
     */
    while(true) {
	log("HDFSChunkServer: Chunkserver accepting connection at port: "
	     + portNum);
	tserver.acceptClientConnection();

	//data transfer loop
	log("HDFSChunkServer: Waiting for Message in ChunkServer");
	string str = tserver.getMessage();
	cout << "ChunkServer: message string received ="  << str << endl;
	//end of message
	if(str.length() == 0) {
	    log("HDFSChunkServer: Zero length string returned. Terminating client connection. ");
	    //break;
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

	    case DOWNLOAD:
		downloadFile(clientMessage.remoteFileName);
		break;

	    case UPLOAD:
		uploadFile(clientMessage.remoteFileName);
		break;
	}
	tserver.closeClientConnection();
	log("Chunkserver: Finished serving client message: " + str);
    }
    cout << "Chunkserver: end of thread." << endl;
}

void ChunkServer::sendMessage(Message msg) {
    string msgString = msg.serialize();
    log("HDFSChunkServer: sending Message =" + msgString);
    tserver.sendMessage(msgString);
}

void ChunkServer::createFolderTree(string fullFilePath) {
    boost::filesystem::path p(fullFilePath.c_str());
    boost::filesystem::path dir = p.parent_path();
    boost::filesystem::create_directories(dir); //fullFilePath.c_str()); //folderTreeString);
}

void ChunkServer::uploadFile(string remoteFileName) {
    //TODO: send status
    // - should send error in case space is not available
    // - should send error in case of other conditions

    cout << "Chunkserver: uploadFile called for remoteFileName=" << remoteFileName << endl;
    //currently only append to the file;
    //later support inserts
    cout << "Creating folder tree for " << (chunkdataroot + remoteFileName) << endl;
    createFolderTree(chunkdataroot + remoteFileName);
    ofstream foutput (chunkdataroot + remoteFileName, ofstream::binary);
    cout << "Chunkserver: output remoteFileName= " << (chunkdataroot + remoteFileName) << endl;
    //while(true)
    for (int i=0; i < 3; i ++) { //loop only 3 times
	Data d = tserver.getData();
	if (d.getLength() > 0) {
	    foutput.write((const char *)d.getDataBuf(), d.getLength());
	} else {
	    break;
	}
    }
    foutput.close();
}

void ChunkServer::downloadFile(string remoteFileName) {
    cout <<"Chunkserver: Download called for " << remoteFileName << endl;
}

Message ChunkServer::readFile(Message requestMessage) {
    Message replyMessage;

    //read the actual file, for now just send the status
    replyMessage.mtype = STATUS;
    replyMessage.statusString = "ReadMessage called for file: " + requestMessage.remoteFileName;
    return replyMessage;
}

string ChunkServer::getHostName() {
    return "127.0.0.1";
}

int ChunkServer::getPortNumInt() {
    return portNumInt;
}

string ChunkServer::getPortNum() {
    return portNum;
}

string ChunkServer::getChunkServerName() {
    return chunkserverName;
}


