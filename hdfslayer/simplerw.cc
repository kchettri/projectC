/*
  Implementation of SimpleReader and SimpleWriter classes. 
*/

// locals

using namespace std;

#include "include/simplerw.h"


/* BEGIN: SimpleReader implementation */

void
SimpleReader::init(string filename) {
    readerStreamObj.open(filename, ios::in | ios::binary);
}

void
SimpleReader::close() {
    readerStreamObj.close();
}

void
SimpleReader::readByte(byte *b) {
    readerStreamObj.read((char *)b, sizeof(byte));
}

void
SimpleReader::readInt(int *a) {
    readerStreamObj.read((char *)a, sizeof(int));
}

void SimpleReader::readIntBigEndian(int *a)  {
    unsigned char buffer[4];    
    readerStreamObj.read((char *)buffer, 4); 

    *a =  (int) buffer[3] + 
           ((int) buffer[2] << 8) +
           ((int) buffer[1] << 16)  +   
           ((int) buffer[0] << 24); 
}

void
SimpleReader::readLong64(long64 *l) {
    readerStreamObj.read((char *)l, sizeof(long64));
}


/*
void
SimpleReader::readFSImage(FSImage *fsImageObj) {
    readerStreamObj.read((char *)fsImageObj, sizeof(FSImage));
}
*/

/* END: SimpleReader implementation*/



/* BEGIN: SimpleWriter implementation */

SimpleWriter::SimpleWriter() {
}

void
SimpleWriter::init(string filename) {
    writerStreamObj.open(filename, ios::out | ios::binary);
}

void
SimpleWriter::writeByte(byte b) {
    writerStreamObj.write((char *)&b, sizeof(byte));
}

void
SimpleWriter::writeInt(int a) {
    writerStreamObj.write((char *)&a, sizeof(int));
}

void
SimpleWriter::writeLong64(long64 l) {
    writerStreamObj.write((char *)&l, sizeof(long64));
}

void
SimpleWriter::close() {
    writerStreamObj.close();
}

/*
void
SimpleWriter::writeFSImage(FSImage fsImageObj) {
    writerStreamObj.write((char *)&fsImageObj, sizeof(FSImage));
}
*/

/* END: SimpleWriter implementation */


