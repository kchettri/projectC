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
	UPLOAD
};

static array<string, 8> messageTypeStringArr {
	"read",
	"write",
	"status",
	"invalid",
	"list",
	"exit",
	"help",
	"upload"
};

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
		serialString +=fieldSeparator;
		serialString +=fileName;
		return serialString;
	}

	static Message deserialize(string msg) {
		Message m;
		char *token = strtok((char *) msg.c_str(), fieldSeparator.c_str());

		if(token == NULL) { cout << "Invalid command string" << endl; exit(1); }
		m.mtype = m.getMessageType(token);

		token = strtok(NULL, fieldSeparator.c_str());
		if (token != NULL) {
			if (m.mtype == READ || m.mtype == WRITE) {
				m.fileName = token;
			} else  if (m.mtype == STATUS) {
				m.statusString = token;
			}
		}
		return m;
	}

	void printMessage() {
		cout << "MessageType: " << getMessageTypeString(mtype);
		if (mtype == READ || mtype == WRITE) {
			cout << ", " << fileName << endl;
		} else  if (mtype == STATUS) {
			cout << ", " << statusString << endl;
		}
	}

/*
	"read",
	"write",
	"status",
	"invalid",
	"list",
	"exit",
	"help"
*/

	static void printAllMessageTypes() {
		cout << endl
			 << "Command types:" << endl
			 << " read <filename>" << endl
			 << " write <filename>" << endl
			 << " upload <localfilename> <remote filename>" << endl
			 << " list" << endl << endl;
	}
};



