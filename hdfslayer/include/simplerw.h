/*
	Implements reader and writer for istream objects.

	simplerw.h
 
 */

#ifndef SIMPLERW_H_
#define SIMPLERW_H_

/* C++ standard headers */
#include <iostream>
#include <fstream>

/* projectC locals */
#include "defs.h"
#include "socketbuf.h"

class SimpleReader {
private: 
    ifstream fileReaderStreamObj;
	istream* readerStreamObj; 
	string filename;
public:
    void init(string filename);
    void init(socketbuf& sockBuf);
	string getFilename();
	bool isEOF();
	int readCharArray(char *buffer, int length);
	int readByteArray(byte *buffer, int length);
	int readString(string& str, int length);
	int readStringEditlogInt16Encoding(string& str);
    int readByte(byte* b); 
	int readBoolean(bool *bl);
	int readInt16BigEndian(int16 *a);
    int readInt(int* a); 
	int readVarint32(int* size);
	int readIntBigEndian(int *a);
	int readLong64BigEndian(long64 *l);
    int readLong64(long64* l); 
	ifstream& getIfStream();
	int getCurrentPosition();
	void setCurrentPosition(int pos);

	/* variable length int/long decoding */
	int decodeByteIntSize(sbyte b);
	int isNegativeByteIntSize(sbyte b);
	int readVarLong64(long64 *l);

    void closeFileStreamObj();
};

class SimpleWriter {
private: 
    ofstream writerStreamObj;
public: 
    SimpleWriter();
    void init(string filename);
    void writeByte(byte b); 
    void writeInt(int a); 
    void writeLong64(long64 l); 
    void close();
};


#endif /* SIMPLERW_H_ */

