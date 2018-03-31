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

bool
SimpleReader::isEOF() {
	if ((readerStreamObj.rdstate() & fstream::eofbit ) != 0 )
		return true;
	else 
		return false;
}

void
SimpleReader::close() {
    readerStreamObj.close();
}

int
SimpleReader::readByte(byte *b) {
	if (isEOF()) return -1;
    readerStreamObj.read((char *)b, sizeof(byte));
	return 0;
}

int 
SimpleReader::readInt16BigEndian(int16 *a)  {
	if (isEOF()) return -1;
    unsigned char buffer[2];    
    readerStreamObj.read((char *)buffer, 2); 

    *a =  (int) buffer[1] + 
           ((int) buffer[0] << 8); 
	return 0;
}

int
SimpleReader::readInt(int *a) {
	if (isEOF()) return -1;
    readerStreamObj.read((char *)a, sizeof(int));
	return 0;
}

int 
SimpleReader::readIntBigEndian(int *a)  {
	if (isEOF()) return -1;
    unsigned char buffer[4];    
    readerStreamObj.read((char *)buffer, 4); 

    *a =  (int) buffer[3] + 
           ((int) buffer[2] << 8) +
           ((int) buffer[1] << 16)  +   
           ((int) buffer[0] << 24); 
	return 0;
}

int
SimpleReader::readLong64(long64 *l) {
	if (isEOF()) return -1;
    readerStreamObj.read((char *)l, sizeof(long64));
	return 0;
}

int
SimpleReader::readLong64BigEndian(long64 *l) {
	if (isEOF()) return -1;
	unsigned char buffer[8];
	readerStreamObj.read((char *)buffer, 8); 

	*l = (long64) buffer[7] + 
		 ((long64) buffer[6] << 8) + 
		 ((long64) buffer[5] << 16) + 
		 ((long64) buffer[4] << 24) + 
		 ((long64) buffer[3] << 32) + 
		 ((long64) buffer[2] << 40) + 
		 ((long64) buffer[1] << 48) + 
		 ((long64) buffer[0] << 56);
	return 0;
}

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


