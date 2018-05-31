/*
  Implementation of SimpleReader and SimpleWriter classes. 
*/

using namespace std;

/* local headers */
#include "include/simplerw.h"

/* BEGIN: SimpleReader implementation */

void
SimpleReader::init(string filename) {
    this->filename = filename;
    fileReaderStreamObj.open(filename, ios::in | ios::binary);
    readerStreamObj = &fileReaderStreamObj;
}

void
SimpleReader::init(socketbuf& sockBuf) {
    readerStreamObj = new istream(&sockBuf); 
}

void
SimpleReader::closeFileStreamObj() {
    fileReaderStreamObj.close();
}

string 
SimpleReader::getFilename() {
    return filename;
}

ifstream&
SimpleReader::getIfStream() {
    return fileReaderStreamObj;
}

int
SimpleReader::getCurrentPosition() {
    return readerStreamObj->tellg();
}

void
SimpleReader::setCurrentPosition(int pos) {
    readerStreamObj->seekg(pos, ios::beg);
}

bool
SimpleReader::isEOF() {
    if ((readerStreamObj->rdstate() & fstream::eofbit ) != 0 )
        return true;
    else 
        return false;
}

int 
SimpleReader::readByteArray(byte *buffer, int length)  {
    return readCharArray((char *)buffer, length);
}

int 
SimpleReader::readCharArray(char *buffer, int length)  {
    if (isEOF()) return -1;
    readerStreamObj->read((char *)buffer, length);
    return length; //TODO: need to clarify this, whether to return length or status
    //return 0;
}

//reads string, and sets null to last char
int 
SimpleReader::readString(string& str, int length)  {
    if (isEOF()) return -1;
    char *buffer = new char[length + 1];
    readerStreamObj->read((char *)buffer, length);
    buffer[length] = '\0';
    str = buffer;
    delete buffer;
    return 0;
}

//reads string, and sets null to last char
int 
SimpleReader::readStringEditlogInt16Encoding(string& str)  {
    if (isEOF()) return -1;
    int16 length;
    readInt16BigEndian(&length);
    char *buffer = new char[length + 1];
    readerStreamObj->read((char *)buffer, length);
    buffer[length] = '\0';
    str = buffer;
    delete buffer;
    return 0;
}


int
SimpleReader::readByte(byte *b) {
    if (isEOF()) return -1;
    readerStreamObj->read((char *)b, sizeof(byte));
    return 0;
}

int
SimpleReader::readBoolean(bool *bl) {
    if (isEOF()) return -1;
    byte b; 
    readByte(&b);
    if ((int) b == 0) 
        *bl = false;
    else 
        *bl = true;
    return 0;
}

int 
SimpleReader::readInt16BigEndian(int16 *a)  {
    if (isEOF()) return -1;
    unsigned char buffer[2];    
    readerStreamObj->read((char *)buffer, 2); 

    *a =  (int) buffer[1] + 
           ((int) buffer[0] << 8); 
    return 0;
}

int
SimpleReader::readInt(int *a) {
    if (isEOF()) return -1;
    readerStreamObj->read((char *)a, sizeof(int));
    return 0;
}

int
SimpleReader::readVarint32(int* size) {
    if (isEOF()) return -1;
    byte b;
    int tempSize;
    readByte(&b);
    tempSize =  (int) b;
    if (tempSize < 0x80) {
        *size = tempSize;
        return 0; 
    }

    cout << "Unsupported varint32 encoding!!, size greater than 0x80" << endl;
    exit(1);
    return -1;
}

int
SimpleReader::decodeByteIntSize(sbyte b) {
    if (b >= -112) {
      return 1;
    } else if (b < -120) {
      return -119 - b;
    }
    return -111 - b;
}

int SimpleReader::isNegativeByteIntSize(sbyte b) {
    return b < -120 || (b >= -112 && b < 0);
}

int
SimpleReader::readVarLong64(long64 *l) {
    if (isEOF()) return -1;
    sbyte sb;
    byte b;
    readByte(&b);
    sb = (sbyte) b;

    int size = decodeByteIntSize(sb);
    if(size == 1) {
        *l = (long64) sb;
        return 0;
    }

    long64 lv = 0;
    for(int i=0; i < size - 1; i++) {
      readByte(&b);
      lv = lv << 8;
      lv = lv | (b & 0xFF);
    }
    long64 lvxor = -1;
    *l = (isNegativeByteIntSize(size)) ? lv ^ lvxor : lv;   
    return 0;
}

int 
SimpleReader::readIntBigEndian(int *a)  {
    if (isEOF()) return -1;
    unsigned char buffer[4];    
    readerStreamObj->read((char *)buffer, 4); 

    *a =  (int) buffer[3] + 
           ((int) buffer[2] << 8) +
           ((int) buffer[1] << 16)  +   
           ((int) buffer[0] << 24); 
    return 0;
}

int
SimpleReader::readLong64(long64 *l) {
    if (isEOF()) return -1;
    readerStreamObj->read((char *)l, sizeof(long64));
    return 0;
}

int
SimpleReader::readLong64BigEndian(long64 *l) {
    if (isEOF()) return -1;
    unsigned char buffer[8];
    readerStreamObj->read((char *)buffer, 8); 

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

bool
SimpleReader::readDelimitedFrom(google::protobuf::MessageLite* msgLite) {
    int msgSize =0;
    readVarint32(&msgSize);
    cout << "protobuf Message size = " << msgSize << endl;
    if (msgSize > 0) {
        char* msgbuf = new char[msgSize];
        readCharArray(msgbuf, msgSize);
        msgLite->ParseFromArray(msgbuf, msgSize);
        delete msgbuf;
        return true;
    }
    return false;
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
SimpleWriter::writeByteArray(byte* b, int len) {
    writerStreamObj.write((char *)b, len);
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


