/*
 * chunkserver.cc
 *
 *  Created on: Jan 2, 2018
 *      Author: kamal
 */

class SimpleHDFSChunkServer: public Logger {

/* Format of metadata:
 *
 *  remoteFileName | chunkid | localfilename
 */
private:
    string chunkserverName;
    int nameCounter;
    string chunkroot, chunkdataroot;
    string getNewFileName() {}
    string portNum;
    int portNumInt;
    TCPServer tserver;

public:

    SimpleHDFSChunkServer(string pnum) {
        portNum = pnum;
        portNumInt = stoi(portNum);

        //bind to portNumber here:
        chunkserverName = "127.0.0.1";
        tserver.bindToPort(portNumInt);

        chunkroot = "/home/projectC/hdfsroot";
        chunkdataroot = chunkroot + "/dataroot";

        loginit("ChunkServer");
    }

    void init() {
        chunkroot = "/home/projectC/hdfsroot";
        chunkdataroot = chunkroot + "/dataroot/";
        //metadata file
        ofstream mfile;
        mfile.open((chunkroot + "/mfile").c_str());
        mfile << "Init text";
        mfile.close();
    }

    void chunkMain() {
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

    void sendMessage(Message msg) {
        string msgString = msg.serialize();
        log("HDFSChunkServer: sending Message =" + msgString);
        tserver.sendMessage(msgString);
    }

    void createFolderTree(string fullFilePath) {
        boost::filesystem::path p(fullFilePath.c_str());
        boost::filesystem::path dir = p.parent_path();
        boost::filesystem::create_directories(dir); //fullFilePath.c_str()); //folderTreeString);
    }

    void uploadFile(string remoteFileName) {
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

    void downloadFile(string remoteFileName) {
        cout <<"Chunkserver: Download called for " << remoteFileName << endl;
    }

    Message readFile(Message requestMessage) {
        Message replyMessage;

        //read the actual file, for now just send the status
        replyMessage.mtype = STATUS;
        replyMessage.statusString = "ReadMessage called for file: " + requestMessage.remoteFileName;
        return replyMessage;
    }

    string getHostName() {
        return "127.0.0.1";
    }

    int getPortNumInt() {
        return portNumInt;
    }

    string getPortNum() {
        return portNum;
    }

    string getChunkServerName() {
        return chunkserverName;
    }

    void read() {}

    void write() {}
};




