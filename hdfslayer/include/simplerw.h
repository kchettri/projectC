/*
 
  simplerw.h
 
 */

#ifndef SIMPLERW_H_
#define SIMPLERW_H_

//std headers
#include <iostream>
#include <fstream>

#include "defs.h"


class SimpleReader {
private: 
    ifstream readerStreamObj; 
public:
    void init(string filename);
	bool isEOF();
	int readCharArray(char *buffer, int length);
	int readString(string& str, int length);
    int readByte(byte* b); 
	int readInt16BigEndian(int16 *a);
    int readInt(int* a); 
	int readIntBigEndian(int *a);
	int readLong64BigEndian(long64 *l);
    int readLong64(long64* l); 
	ifstream& getIfStream();
    //FSImage readFSImage();
    void close();
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
    //void writeFSImage(FSImage fsImgageObj);
};


#endif /* SIMPLERW_H_ */

