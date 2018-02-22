/**
 *  Format of the metadata in namenode according to
 *  org.apache.hadoop.hdfs.server.namenode.FSImageFormat
 *
 * In particular, the format of the FSImage looks like:
 * <pre>
 * FSImage {
 *   layoutVersion: int, namespaceID: int, numberItemsInFSDirectoryTree: long,
 *   namesystemGenerationStampV1: long, namesystemGenerationStampV2: long,
 *   generationStampAtBlockIdSwitch:long, lastAllocatedBlockId:
 *   long transactionID: long, snapshotCounter: int, numberOfSnapshots: int,
 *   numOfSnapshottableDirs: int,
 *   {FSDirectoryTree, FilesUnderConstruction, SecretManagerState} (can be compressed)
 * }
 *
 * FSDirectoryTree (if {@link Feature#FSIMAGE_NAME_OPTIMIZATION} is supported) {
 *   INodeInfo of root, numberOfChildren of root: int
 *   [list of INodeInfo of root's children],
 *   [list of INodeDirectoryInfo of root's directory children]
 * }
 *
 * FSDirectoryTree (if {@link Feature#FSIMAGE_NAME_OPTIMIZATION} not supported){
 *   [list of INodeInfo of INodes in topological order]
 * }
 *
 * INodeInfo {
 *   {
 *     localName: short + byte[]
 *   } when {@link Feature#FSIMAGE_NAME_OPTIMIZATION} is supported
 *   or
 *   {
 *     fullPath: byte[]
 *   } when {@link Feature#FSIMAGE_NAME_OPTIMIZATION} is not supported
 *   replicationFactor: short, modificationTime: long,
 *   accessTime: long, preferredBlockSize: long,
 *   numberOfBlocks: int (-1 for INodeDirectory, -2 for INodeSymLink),
 *   {
 *     nsQuota: long, dsQuota: long,
 *     {
 *       isINodeSnapshottable: byte,
 *       isINodeWithSnapshot: byte (if isINodeSnapshottable is false)
 *     } (when {@link Feature#SNAPSHOT} is supported),
 *     fsPermission: short, PermissionStatus
 *   } for INodeDirectory
 *   or
 *   {
 *     symlinkString, fsPermission: short, PermissionStatus
 *   } for INodeSymlink
 *   or
 *   {
 *     [list of BlockInfo]
 *     [list of FileDiff]
 *     {
 *       isINodeFileUnderConstructionSnapshot: byte,
 *       {clientName: short + byte[], clientMachine: short + byte[]} (when
 *       isINodeFileUnderConstructionSnapshot is true),
 *     } (when {@link Feature#SNAPSHOT} is supported and writing snapshotINode),
 *     fsPermission: short, PermissionStatus
 *   } for INodeFile
 * }
 *
 *  BlockInfo {
 *     this.blockId = in.readLong();
 *      this.numBytes = in.readLong();
 *      this.generationStamp = in.readLong();
 *   }
 *
 * INodeDirectoryInfo {
 *   fullPath of the directory: short + byte[],
 *   numberOfChildren: int, [list of INodeInfo of children INode],
 *   {
 *     numberOfSnapshots: int,
 *     [list of Snapshot] (when NumberOfSnapshots is positive),
 *     numberOfDirectoryDiffs: int,
 *     [list of DirectoryDiff] (NumberOfDirectoryDiffs is positive),
 *     number of children that are directories,
 *     [list of INodeDirectoryInfo of the directory children] (includes
 *     snapshot copies of deleted sub-directories)
 *   } (when {@link Feature#SNAPSHOT} is supported),
 * }
 *
 * Snapshot {
 *   snapshotID: int, root of Snapshot: INodeDirectoryInfo (its local name is
 *   the name of the snapshot)
 * }
 *
 * DirectoryDiff {
 *   full path of the root of the associated Snapshot: short + byte[],
    childrenSize: int,
 *   isSnapshotRoot: byte,
 *   snapshotINodeIsNotNull: byte (when isSnapshotRoot is false),
 *   snapshotINode: INodeDirectory (when SnapshotINodeIsNotNull is true), Diff
 * }
 *
 * Diff {
 *   createdListSize: int, [Local name of INode in created list],
 *   deletedListSize: int, [INode in deleted list: INodeInfo]
 * }
 *
 * FileDiff {
 *   full path of the root of the associated Snapshot: short + byte[],
 *   fileSize: long,
 *   snapshotINodeIsNotNull: byte,
 *   snapshotINode: INodeFile (when SnapshotINodeIsNotNull is true), Diff
 * }
 * </pre>
 */

 /* locals */
#include "defs.h"

/* C++ */
//#include <string>
#include <vector>
#include <iostream> 
#include <fstream>
#include <chrono>

using namespace std;

class SimpleReader {
private: 
    ifstream readerStreamObj; 
public:
    void init();
    void readByte(byte* b); 
    void readInt(int* a);
    void readLong64(long64* l);
    FSImage readFSImage();
    void close();
};

class SimpleWriter {
private: 
    ofstream writerStreamObj;
public: 
    SimpleWriter();
    void init();
    void writeByte(byte b); 
    void writeInt(int a);
    void writeLong64(long64 l);
    void close();
    void writeFSImage(FSImage fsImgageObj);
};

class ObjectIO {
public: 
    virtual void writeObj(SimpleWriter writerObj) = 0; 
    virtual void readObj(SimpleReader readerObj) = 0; 
    virtual void printObj() = 0; 
} 

class Diff : public ObjectIO {
public:
    int createdListSize; 
    int deletedListSize; 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};

class FileDiff : public ObjectIO {
public:
    char* snapshotFullPath; //full path of the root of the associated Snapshot: short + byte[],
    long64 fileSize; 
    byte snapshotINodeIsNotNull;
    //INodeFile snapshotINode; 
    Diff df; 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};

class BlockInfo : public ObjectIO {
public:
    long64 blockId;
    long64 numBytes;
    long64 generationStamp; 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};


class PermissionStatus : public ObjectIO{
public:

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};

class INodeFile : public ObjectIO {
public: 
    vector<BlockInfo> blkInfos;
    vector<FileDiff> fdiffs;
    byte isInNodeFileUnderConstructionSnapshot; 
    char* clientName; 
    char* clientMachine; //{clientName: short + byte[], clientMachine: short + byte[]} (when
			  // isINodeFileUnderConstructionSnapshot is true),
    int fsPermission; 
    PermissionStatus permStatus; 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};


class INodeDirectory : public ObjectIO {
public:
    long64 nsQuota; 
    long64 dsQuota;
    byte isINodeSnapshottable;
    byte isINodeWithSnapshot; // if isINodeSnapshottable is false
    int fsPermission; //short fsPermission
    PermissionStatus permStatus; 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};

class DirectoryDiff : public ObjectIO {
public:
    char* rootAssociatedSnapshot; //short + byte[]
    int childrenSize;
    byte isSnapshotRoot;
    byte snapshotINodeIsNotNull; //when isSnapshotRoot is false
    INodeDirectory snapshotINode; //when snapshotINodeIsNotNull is true
    Diff df;

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};

class INodeSymLink : public ObjectIO {
public: 
    char* symlinkString; 
    int fsPermission; //short 
    PermissionStatus permStatus; 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};

class INodeInfo : public ObjectIO {
public: 
    byte* fullPath; 
    int replicationFactor; 
    long64 modificationTime; 
    long64 accessTime; 
    long64 preferredBlockSize; 
    int numberOfBlocks; //-1 for INodeDirectory, -2 for INodeSymLink
    INodeDirectory inodeDir; 
    INodeSymLink inodeSym; 
    INodeFile inodeFile; 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
}; 

class INodeDirectoryInfo : public ObjectIO {
public: 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}

};

class FSDirectoryTree : public ObjectIO {
public: 
/*
 * FSDirectoryTree (if {@link Feature#FSIMAGE_NAME_OPTIMIZATION} is supported) {
 *   INodeInfo of root, numberOfChildren of root: int
 *   [list of INodeInfo of root's children],
 *   [list of INodeDirectoryInfo of root's directory children]
 * }
 */
    INodeInfo root; 
    int numberOfChildren; 
    vector<INodeInfo> rootChildren; 
    vector<INodeDirectoryInfo> rootDirectoryChildren; 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
}; 

class FilesUnderConstruction : public ObjectIO {
public: 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
}; 

class SecretManagerState : public ObjectIO {
public: 

    void writerObj(SimpleWriter writerObj) {}
    void readObj(SimpleReader readerObj) {}
    void printObj() {}
};

/* It is just a binary file at this point */
class FSImage : public ObjectIO {
public:
    int layoutVersion; 
    int namespaceID; 
    long64 numberItemsInFSDirectoryTree; 
    long64 namesystemGenerationStampV1; 
    long64 namesystemGenerationStampV2;
    long64 generationStampAtBlockIdSwitch; 
    long64 lastAllocatedBlockId; 
    long64 transactionID; 
    int snapshotCounter;
    int numberOfSnapshots; 
    int numOfSnapshottableDirs; 

    FSDirectoryTree fsdirectoryTree; 
    FilesUnderConstruction fsUnderConstruction; 
    SecretManagerState secmanState; 

    void init(); 
    void createInitialFsImageFile();

    void writeObj(SimpleWriter writerObj);
    void readObj(SimpleReader readerObj);
    void printObj();
};

SimpleWriter::SimpleWriter() {
}

void
SimpleWriter::init() {
    writerStreamObj.open("fsImage", ios::out | ios::binary);
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
SimpleWriter::writeFSImage(FSImage fsImageObj) {
    writerStreamObj.write((char *)&fsImageObj, sizeof(FSImage));
}

void
SimpleWriter::close() {
    writerStreamObj.close();
}

void 
SimpleReader::init() {
    readerStreamObj.open("fsImage", ios::in | ios::binary);
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

void  
SimpleReader::readLong64(long64 *l) {
    readerStreamObj.read((char *)l, sizeof(long64));
}

void
SimpleReader::readFSImage(FSImage *fsImageObj) {
    readerStreamObj.read((char *)fsImageObj, sizeof(FSImage));
}

void 
FSImage::init() {
    layoutVersion = 1; 
    namespaceID = 1; 
    numberItemsInFSDirectoryTree = 1; 
    namesystemGenerationStampV1 = getCurrentTime(); 
    namesystemGenerationStampV2 = getCurrentTime();
    generationStampAtBlockIdSwitch = 1; 
    lastAllocatedBlockId = 0; 
    transactionID = 1; 
    snapshotCounter = 0;
    numberOfSnapshots = 0; 
    numOfSnapshottableDirs = 0; 
}

void 
FSImage::createInitialFsImageFile() {
    long64 data = 8765; 
    long64 y; 
    ofstream fsImageFileWriter; 
    fsImageFileWriter.write((char *)&data, sizeof(long64));
    fsImageFileWriter.close();

    ifstream fsImageFileReader; 
    fsImageFileReader.open("fsImage", ios::in | ios::binary);
    fsImageFileReader.read((char *)&y, sizeof(long64));
    fsImageFileReader.close();
    cout << "y = " << y; 
}

void 
FSImage::printObj() {
    cout << "FSImage: " << endl
	 << "\tlayoutVersion: " << layoutVersion << endl
	 << "\tnamespaceID:   " << namespaceID << endl
	 << "\tnumberItemsInFSDirectoryTree: " << numberItemsInFSDirectoryTree << endl
	 << "\tnamesystemGenerationStampV1: " << namesystemGenerationStampV1 << endl
	 << "\tnamesystemGenerationStampV2: " << namesystemGenerationStampV2 << endl
	 << "\tgenerationStampAtBlockIdSwitch: " << generationStampAtBlockIdSwitch << endl
	 << "\tlastAllocatedBlockId: " << lastAllocatedBlockId << endl
	 << "\ttransactionID: " << transactionID << endl
	 << "\tsnapshotCounter: " << snapshotCounter << endl
	 << "\tnumberOfSnapshots: " << numberOfSnapshots << endl
	 << "\tnumOfSnapshottableDirs: " << numOfSnapshottableDirs << endl;
}

void 
FSImage::writeObj(SimpleWriter writerObj) {
    writerObj.writeInt(layoutVersion);  
    writerObj.writeInt(namespaceID); 
    writerObj.writeLong64(numberItemsInFSDirectoryTree);
    writerObj.writeLong64(namesystemGenerationStampV1);
    writerObj.writeLong64(namesystemGenerationStampV2);
    writerObj.writeLong64(generationStampAtBlockIdSwitch);
    writerObj.writeLong64(lastAllocatedBlockId);
    writerObj.writeLong64(transactionID);
    writerObj.writeInt(snapshotCounter);
    writerObj.writeInt(numberOfSnapshots);
    writerObj.writeInt(numOfSnapshottableDirs);

    fsdirectoryTree.writeObj(writerObj); 
    fsUnderConstruction.writeObj(writerObj); 
    secmanState.writeObj(writerObj); 
}

void 
FSImage::readObj(SimpleReader readerObj) {
    readerObj.readInt(layoutVersion);  
    readerObj.readInt(namespaceID); 
    readerObj.readLong64(numberItemsInFSDirectoryTree);
    readerObj.readLong64(namesystemGenerationStampV1);
    readerObj.readLong64(namesystemGenerationStampV2);
    readerObj.readLong64(generationStampAtBlockIdSwitch);
    readerObj.readLong64(lastAllocatedBlockId);
    readerObj.readLong64(transactionID);
    readerObj.readInt(snapshotCounter);
    readerObj.readInt(numberOfSnapshots);
    readerObj.readInt(numOfSnapshottableDirs);

    fsdirectoryTree.readObj(readerObj); 
    fsUnderConstruction.readObj(readerObj); 
    secmanState.readObj(readerObj); 
}

int main() {

    cout << "Size of FSImage: = " << sizeof(FSImage) << endl;
    FSImage fsImageObj, readFSImageObj; 
    fsImageObj.init();

    SimpleWriter writerObj; 
    writerObj.init();
    fsImageObj.writeObj(writerObj);
    writerObj.close();

    cout << "Original FSImage: " << endl;
    fsImageObj.printObj();
    
    SimpleReader readerObj;
    readerObj.init();
    readFSImageObj.readObj(readerObj); 
    readerObj.close();

    cout << "Read FSImage: " << endl;
    readFSImageObj.printObj();
}

