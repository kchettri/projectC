/*
 * Copyright
 *  message.h
 */
#ifndef MESSAGE_H_
#define MESSAGE_H_

/* C++ headers */
#include <array>
#include <string>

/* locals */
#include "defs.h"

enum MessageType {
	READ,
	WRITE,
	STATUS,
	INVALID,
	LIST,
	EXIT,
	HELP,
	UPLOAD,
	DOWNLOAD,
	CSDISCOVER
};

static std::array<std::string, 10> messageTypeStringArr {
	"read",
	"write",
	"status",
	"invalid",
	"list",
	"exit",
	"help",
	"upload",
	"download",
	"csdiscover"
};

class Data {
private:
    byte* databuf;
    int length;

public:
    static int BLOCK_SIZE;
    Data() {
        databuf = new byte[BLOCK_SIZE];
        length = 0;
    }

    unsigned char* getDataBuf() {
        return databuf;
    }

    int getLength() {
        return length;
    }

    void setLength(int len) {
        length = len;
    }

    void freeDataBuf()  {
        delete[] databuf;
    }
};

/* Message exchanged by client and server */
class Message {
private:
	static const std::string fieldSeparator;
	int serialsize;
public:
	MessageType mtype;
	std::string remoteFileName; /* remote fileName*/
	std::string localFileName;  /* local fileName*/

	std::string statusString;
	std::string messageInString;
	std::string chunkServerHostName;
	std::string chunkServerPortNum;

	Message() {} 
	Message(std::string s) {
		messageInString = s;
	}

	const char * getMessageTypeString(MessageType m);
	MessageType getMessageType(char *msgVal);
	std::string serialize();
	static Message deserialize(std::string msg);
	void printMessage();
	static void printAllMessageTypes();
};

#endif /* MESSAGE_H_ */

