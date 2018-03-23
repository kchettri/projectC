/*
 * chunkserver.h
 *
 */

#ifndef CHUNKSERVER_H_
#define CHUNKSERVER_H_

/* C++ */
#include <string>

/* locals */
#include "logger.h"
#include "tcpserver.h"
#include "message.h"

class ChunkServer: public Logger {

/* Format of metadata:
 *
 *  remoteFileName | chunkid | localfilename
 */
private:
    std::string chunkserverName;
    int nameCounter;
    std::string chunkroot, chunkdataroot;
    std::string getNewFileName() {}
    std::string portNum;
    int portNumInt;
    TCPServer tserver;

public:
    ChunkServer(std::string pnum);
    void init();
    void chunkMain();
    void sendMessage(Message msg);
    void createFolderTree(std::string fullFilePath);
    void uploadFile(std::string remoteFileName);
    void downloadFile(std::string remoteFileName);
    Message readFile(Message requestMessage);
    std::string getHostName();
    int getPortNumInt();
    std::string getPortNum();
    std::string getChunkServerName();
};

#endif /* CHUNKSERVER_H_ */
