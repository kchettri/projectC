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
    void readByte(byte* b); 
    void readInt(int* a); 
	void readIntBigEndian(int *a);
    void readLong64(long64* l); 
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

