/*
 * message.h
 */

#include <array>

using namespace std;

enum MessageType {
	READ,
	WRITE,
	STATUS,
	INVALID,
	LIST,
	EXIT,
	HELP,
	UPLOAD,
	CSDISCOVER
};

static array<string, 9> messageTypeStringArr {
	"read",
	"write",
	"status",
	"invalid",
	"list",
	"exit",
	"help",
	"upload",
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

int Data::BLOCK_SIZE = 4096;

/* Message exchanged by client and server */
class Message {
private:
	static const string fieldSeparator;
	int serialsize;
public:
	MessageType mtype;
	string fileName;
	string statusString;
	string messageInString;
	string chunkServerHostName;
	string chunkServerPortNum;

	Message() { }

	Message(string s) {
		messageInString = s;
	}

	const char * getMessageTypeString(MessageType m) {
		return messageTypeStringArr[m].c_str();
	}

	MessageType getMessageType(char *msgVal) {
		if (msgVal == NULL) return INVALID;

		for (int i=0; i < messageTypeStringArr.size(); i++) {
			const char * cstr = messageTypeStringArr[i].c_str();
			if (strncmp(msgVal, cstr, strlen(cstr)) == 0) {
				return (MessageType)i;
			}
		}
		return INVALID;
	}

	string serialize() {
		string serialString = "";
		serialString += getMessageTypeString(mtype);

		if(mtype == READ || mtype == WRITE) {
			serialString +=fieldSeparator;
			serialString +=fileName;
		} else if (mtype == STATUS) {
			serialString +=fieldSeparator;
			serialString +=statusString;
		} else if (mtype == LIST) {

		} else if (mtype == EXIT) {

		} else if (mtype == UPLOAD) {
		    serialString += fieldSeparator;
		    serialString += fileName;
		} else if (mtype == CSDISCOVER) {
			serialString +=fieldSeparator;
			serialString +=chunkServerHostName;
			serialString +=fieldSeparator;
			serialString +=chunkServerPortNum;
		}
		return serialString;
	}

	static Message deserialize(string msg) {
		Message m;
		char *token = strtok((char *) msg.c_str(), fieldSeparator.c_str());

		if(token == NULL) { cout << "Invalid command string" << endl; exit(1); }
		m.mtype = m.getMessageType(token);

		vector<string> tokens;
		while((token = strtok(NULL, fieldSeparator.c_str())) != NULL) {
			tokens.push_back(token);
		}

		if (m.mtype == READ || m.mtype == WRITE) {
			m.fileName = tokens.at(0);
		} else  if (m.mtype == STATUS) {
			m.statusString = tokens.at(0);
		} else if (m.mtype == LIST) {

		} else if (m.mtype == EXIT) {

		} else if (m.mtype == UPLOAD) {
		    m.fileName = tokens.at(0);
		} else if (m.mtype == CSDISCOVER) {
			m.chunkServerHostName = tokens.at(0);
			m.chunkServerPortNum = tokens.at(1);
		}
		return m;
	}

	void printMessage() {
		cout << "MessageType: " << getMessageTypeString(mtype);
		if (mtype == READ || mtype == WRITE) {
			cout << ", " << fileName << endl;
		} else  if (mtype == STATUS) {
			cout << ", " << statusString << endl;
		} else if (mtype == LIST) {

		} else if (mtype == EXIT) {

		} else if (mtype == UPLOAD) {
		    cout << " , " << fileName;
		} else if (mtype == CSDISCOVER) {
			cout << " chunkServerHostName=" << chunkServerHostName ;
			cout << " chunkServerPortNum=" << chunkServerPortNum << endl;
		}
	}

	static void printAllMessageTypes() {
		cout << endl
			 << "Command types:" << endl
			 << " read <filename>" << endl
			 << " write <filename>" << endl
			 << " upload <localfilename> <remote filename>" << endl
			 << " list" << endl << endl;
	}
};


//space because command contains space characters and it is used to deserialize and create Message object
//space cannot appear in the field values, otherwise it will have error.
const string Message::fieldSeparator = " ";

