/*
  Editlog parser for Namenode editlogs.
 */

using namespace std;
//std headers
#include <iostream> 

//locals
#include "include/simplerw.h"

//protobuf
#include "proto/hadoop.hdfs/src/editlog.pb.h"
/* protocol buffer delimited parser support */
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/delimited_message_util.h>


class LayoutVersion {
    int curVersion;
	int ancestorVersion; 
	int compatibleVersion;
	string description;
public:
	LayoutVersion() {}
    LayoutVersion(int curVersion, int compatibleVersion, string description) 
						: curVersion(curVersion),
						 ancestorVersion(curVersion + 1),
						 compatibleVersion(compatibleVersion),
						 description(description)  {}

    int getCurVersion() const {
		return curVersion;
	}

    int getAncestorVersion() const {
 		return ancestorVersion;
    }

	int getCompatibleVersion() const {
		return compatibleVersion;
	}
	
	string getDescription() const {
		return description;
	}

	static const LayoutVersion TRUNCATE;
	static const LayoutVersion APPEND_NEW_BLOCK;
	static const LayoutVersion QUOTA_BY_STORAGE_TYPE;
	static const LayoutVersion ERASURE_CODING;
	static const LayoutVersion INVALID;
};

const LayoutVersion LayoutVersion::TRUNCATE = LayoutVersion(-61, -61, "Truncate");
const LayoutVersion LayoutVersion::APPEND_NEW_BLOCK = LayoutVersion(-62, -61, "Support appending to new block");
const LayoutVersion LayoutVersion::QUOTA_BY_STORAGE_TYPE = LayoutVersion(-63, -61, "Support quota for specific storage types");
const LayoutVersion LayoutVersion::ERASURE_CODING = LayoutVersion(-64, -61, "Support erasure coding");
const LayoutVersion LayoutVersion::INVALID = LayoutVersion();

LayoutVersion getCurrentLayoutVersion(int logVersion) {
	LayoutVersion layoutVersions[] = { LayoutVersion::TRUNCATE, 
									   LayoutVersion::APPEND_NEW_BLOCK, 
									   LayoutVersion::QUOTA_BY_STORAGE_TYPE, 
									   LayoutVersion::ERASURE_CODING 
									 }; 
	int layoutVersionNums[] = { -61, -62, -63, -64 };

	for (int i=0 ; i < 4; i++) {
		if(layoutVersionNums[i] == logVersion) {
			return layoutVersions[i];	
		}	
	}

	return LayoutVersion::INVALID;
}

/* FSEditLogOpcodes.java */
enum Opcodes {
  OP_ADD = 0, 						//AddOp.class),
  OP_RENAME_OLD = 1,				//RenameOldOp.class),
  OP_DELETE = 2,					//DeleteOp.class),
  OP_MKDIR = 3, 					//MkdirOp.class),
  OP_SET_REPLICATION = 4, 			//SetReplicationOp.class),
  OP_DATANODE_ADD = 5,				//obsolete
  OP_DATANODE_REMOVE = 6, 			//obsolete
  OP_SET_PERMISSIONS = 7,			//SetPermissionsOp.class),
  OP_SET_OWNER = 8,					//SetOwnerOp.class),
  OP_CLOSE = 9,						//CloseOp.class),
  OP_SET_GENSTAMP_V1 = 10,			//SetGenstampV1Op.class),
  OP_SET_NS_QUOTA = 11,				//SetNSQuotaOp.class), // obsolete
  OP_CLEAR_NS_QUOTA = 12,			//ClearNSQuotaOp.class), // obsolete
  OP_TIMES = 13, 					//TimesOp.class), // set atime, mtime
  OP_SET_QUOTA = 14,				//SetQuotaOp.class),
  OP_RENAME = 15,					//RenameOp.class),
  OP_CONCAT_DELETE = 16, 			//ConcatDeleteOp.class),
  OP_SYMLINK = 17,					//SymlinkOp.class),
  OP_GET_DELEGATION_TOKEN = 18,		//GetDelegationTokenOp.class),
  OP_RENEW_DELEGATION_TOKEN = 19, 	//RenewDelegationTokenOp.class),
  OP_CANCEL_DELEGATION_TOKEN = 20,  //CancelDelegationTokenOp.class),
  OP_UPDATE_MASTER_KEY = 21,  		//UpdateMasterKeyOp.class),
  OP_REASSIGN_LEASE = 22, 			//ReassignLeaseOp.class),
  OP_END_LOG_SEGMENT = 23, 			// EndLogSegmentOp.class),
  OP_START_LOG_SEGMENT = 24, 		//StartLogSegmentOp.class),
  OP_UPDATE_BLOCKS = 25, 			//UpdateBlocksOp.class),
  OP_CREATE_SNAPSHOT = 26, 			//CreateSnapshotOp.class),
  OP_DELETE_SNAPSHOT = 27, 			//DeleteSnapshotOp.class),
  OP_RENAME_SNAPSHOT = 28, 			//RenameSnapshotOp.class),
  OP_ALLOW_SNAPSHOT = 29, 			//AllowSnapshotOp.class),
  OP_DISALLOW_SNAPSHOT = 30, 		//DisallowSnapshotOp.class),
  OP_SET_GENSTAMP_V2 = 31, 			//SetGenstampV2Op.class),
  OP_ALLOCATE_BLOCK_ID = 32, 		//AllocateBlockIdOp.class),
  OP_ADD_BLOCK = 33, 				//AddBlockOp.class),
  OP_ADD_CACHE_DIRECTIVE = 34, 		//AddCacheDirectiveInfoOp.class),
  OP_REMOVE_CACHE_DIRECTIVE = 35, 	//RemoveCacheDirectiveInfoOp.class),
  OP_ADD_CACHE_POOL = 36, 			//AddCachePoolOp.class),
  OP_MODIFY_CACHE_POOL = 37, 		//ModifyCachePoolOp.class),
  OP_REMOVE_CACHE_POOL = 38, 		//RemoveCachePoolOp.class),
  OP_MODIFY_CACHE_DIRECTIVE = 39, 	//ModifyCacheDirectiveInfoOp.class),
  OP_SET_ACL = 40, 					//SetAclOp.class),
  OP_ROLLING_UPGRADE_START = 41, 	//RollingUpgradeStartOp.class),
  OP_ROLLING_UPGRADE_FINALIZE = 42, //RollingUpgradeFinalizeOp.class),
  OP_SET_XATTR = 43, 				//SetXAttrOp.class),
  OP_REMOVE_XATTR = 44, 			//RemoveXAttrOp.class),
  OP_SET_STORAGE_POLICY = 45, 		//SetStoragePolicyOp.class),
  OP_TRUNCATE = 46, 				//TruncateOp.class),
  OP_APPEND = 47, 					//AppendOp.class),
  OP_SET_QUOTA_BY_STORAGETYPE = 48, //SetQuotaByStorageTypeOp.class),
  OP_ADD_ERASURE_CODING_POLICY = 49, //AddErasureCodingPolicyOp.class),
  OP_ENABLE_ERASURE_CODING_POLICY = 50, //EnableErasureCodingPolicyOp.class),
  OP_DISABLE_ERASURE_CODING_POLICY = 51,  //DisableErasureCodingPolicyOp.class),
  OP_REMOVE_ERASURE_CODING_POLICY = 52,  //RemoveErasureCodingPolicyOp.class),
  // Note that the current range of the valid OP code is 0~127
  OP_INVALID = -1,
  OP_INVALID_BYTE = 255
};

// OP_START_LOG_SEGMENT
void readStartLogSegment(SimpleReader &sReader) {
	//does nothing, no fields are present	
}

//OP_END_LOG_SEGMENT
void readEndLogSegment(SimpleReader& sReader) {
	//does nothing, no fields
}

void readBlocks(SimpleReader &sReader) {
	int intVal; 
	int16 int16Val;
	long64 longVal;
	string str;

	sReader.readIntBigEndian(&intVal); 
	cout << "Num of blocks= " << intVal << endl;

	for (int i=0; i < intVal; i++) {
		sReader.readLong64BigEndian(&longVal);
		cout << "blockid= " << longVal << endl;
		sReader.readLong64BigEndian(&longVal);
		cout << "numBytes= " << longVal << endl;
		sReader.readLong64BigEndian(&longVal);
		cout << "generationtimestamp= " << longVal << endl;
	}
}

void readBlocksCompactArray(SimpleReader& sReader) {
	int intVal; 
	int16 int16Val;
	long64 longVal;
	string str;

	sReader.readVarLong64(&longVal); 
	int size = (int) longVal;
	cout << "Num of blocks= " << size << endl;
	for (int i=0; i < size; i++) {
		sReader.readLong64BigEndian(&longVal);
		cout << "blockid= " << longVal << endl;
		sReader.readVarLong64(&longVal); 
		cout << "numBytes= " << longVal << endl;
		sReader.readVarLong64(&longVal); 
		cout << "generationtimestamp= " << longVal << endl;
	}
}

void readPermissionStatus(SimpleReader &sReader) {
	byte byteVal;
	int intVal; 
	int16 int16Val;
	long64 longVal;
	string str;

	//read permission status;
	// String in permission status is stored as Text.class, which accepts 
	// a different encoder/decoder if present
  	//username = Text.readString(in, Text.DEFAULT_MAX_LEN);
    //groupname = Text.readString(in, Text.DEFAULT_MAX_LEN);
    //permission = FsPermission.read(in);
	sReader.readByte(&byteVal);
	sReader.readString(str, (int)byteVal);
	cout << "username=" << str << " length=" << (int)byteVal  << endl;
	sReader.readByte(&byteVal);
	sReader.readString(str, (int)byteVal);
	cout << "groupname=" << str << " length=" << (int)byteVal  << endl;
	sReader.readInt16BigEndian(&int16Val);
	cout << "Mode=" << (int)int16Val << endl;

}

bool
readDelimitedFrom(SimpleReader &sReader, google::protobuf::MessageLite* msgLite) {
	int msgSize =0; 
	sReader.readVarint32(&msgSize);
	cout << "protobuf Message size = " << msgSize << endl;
	if (msgSize > 0) {
		char* msgbuf = new char[msgSize];			
		sReader.readCharArray(msgbuf, msgSize);
		msgLite->ParseFromArray(msgbuf, msgSize);
		delete msgbuf;
		return true;
	}
	return false;
}

//read rpcids
void readRpcIds(SimpleReader& sReader) {
	int intVal;

	int16 clientIdLength = 0; 
	sReader.readInt16BigEndian(&clientIdLength);
	cout << "clientIdLength =" << (int) clientIdLength << endl;
	char* clientIdCharArray = new char[clientIdLength];
	sReader.readCharArray(clientIdCharArray, clientIdLength);
	cout << "clientId = ";
	for(int i=0; i < clientIdLength; i++) {
		if ((unsigned int)clientIdCharArray[i] < 10) cout << "0";
		cout << std::hex << (+clientIdCharArray[i] & 0xFF);
	}
	cout << std::dec << endl;
	
	sReader.readIntBigEndian(&intVal); 
	cout << "rpccall id=" << intVal << endl;
}

//common for both OP_ADD and OP_CLOSE
void readAddClose(SimpleReader& sReader) {
	//fields
	int intVal; 
	int16 int16Val;
	long64 longVal;
	string str;
	bool bl;
	byte b;

	int16Val = 0; 
	cout << "length=" << int16Val << endl;

	sReader.readLong64BigEndian(&longVal);
	cout << "inode=" << longVal << endl;

	sReader.readStringEditlogInt16Encoding(str);
	cout << "path=" << str << endl;

	sReader.readInt16BigEndian(&int16Val); 
	cout << "replication =" << (int) int16Val << endl;
	sReader.readLong64BigEndian(&longVal);
	cout << "mtime= " << longVal << endl;


	sReader.readLong64BigEndian(&longVal);
	cout << "atime= " << longVal << endl;
	sReader.readLong64BigEndian(&longVal);
	cout << "blocksize= " << longVal << endl;

	//blocks
	readBlocks(sReader);
	readPermissionStatus(sReader);
}

//OP_CLOSE
void readClose(SimpleReader &sReader) {
	readAddClose(sReader);	
}

//OP_ADD
void readAdd(SimpleReader &sReader) {
	//fields
	int intVal; 
	int16 int16Val;
	long64 longVal;
	string str;
	bool bl;
	byte b;

	readAddClose(sReader);

	//Fields before this is also relevant to OP_CLOSE

	//Read XAttrEditLogProto from the editlog
	//this time, we need to read additional fields. 
	// create a readdelimitedFrom using sReader's ifstream and then 
	// dont close it at the end - need a custom implementation
	sReader.readIntBigEndian(&intVal); 
	cout << "ACL editlog entry size= " << intVal << endl;
	int editLogACLSize = intVal;
	for (int i=0; i< editLogACLSize; i++) {
		cout << "Number of editLogSize is greater than 0. Not handled !!" << endl;
		exit(1);
	}

	hadoop::hdfs::XAttrEditLogProto xattrEditLogProto; 
	if (readDelimitedFrom(sReader, &xattrEditLogProto)) {
		cout << "xattrEditLogProto entry is present in editlog!! Not handled. Exiting." << endl;
		exit(1);
	} else {
		cout << "No xattrEditLogProto entries" << endl;
	}

	string clientName; 
	string clientMachine;
	sReader.readStringEditlogInt16Encoding(clientName);
	sReader.readStringEditlogInt16Encoding(clientMachine);
	cout << "clientName=" << clientName << "  clientMachine=" << clientMachine << endl;
	
	sReader.readBoolean(&bl);
	cout << "overwrite=" << bl << endl;
	
	sReader.readByte(&b);
	cout << "storage policy id=" << (int) b << endl;

	sReader.readByte(&b);
	cout << "erasure coding policy id=" << (int) b << endl;

	readRpcIds(sReader);
}

//OP_ALLOCATE_BLOCK_ID: // = 32, 		//AllocateBlockIdOp.class),
void readAllocateBlockId(SimpleReader& sReader) {
	//fields
	int intVal;
	int16 int16Val;
	long64 longVal;
	byte byteVal;
	string str;

	sReader.readLong64BigEndian(&longVal);
	cout << "blockid=" << longVal << endl;
}

//OP_SET_GENSTAMP_V2: //31
void readSetGenStampv2(SimpleReader& sReader) {
	//fields
	int intVal;
	int16 int16Val;
	long64 longVal;
	byte byteVal;
	string str;

	sReader.readLong64BigEndian(&longVal);
	cout << "genstampv2=" << longVal << endl;
}


//"OP_ADD_BLOCK opcode read" << endl;
void readAddBlock(SimpleReader& sReader) {
	//fields
	int intVal;
	int16 int16Val;
	long64 longVal;
	byte byteVal;
	string str;

	sReader.readStringEditlogInt16Encoding(str);
	cout << "path=" << str << " length=" << str.length()  << endl;

	readBlocksCompactArray(sReader);
	readRpcIds(sReader);
}

//OP_RENAME_OLD
void readRenameOld(SimpleReader& sReader) {
	//fields
	int intVal;
	int16 int16Val;
	long64 longVal;
	byte byteVal;
	string str;

	intVal = 0;
	cout << "length = " << intVal << endl;

	sReader.readStringEditlogInt16Encoding(str);
	cout << "src=" << str << endl;
	sReader.readStringEditlogInt16Encoding(str);
	cout << "dest=" << str << endl;

	sReader.readLong64BigEndian(&longVal);
	cout << "timestamp=" << longVal << endl;

	readRpcIds(sReader);
}

//OP_MKDIR, 
// currently all ops do not handle previous version of HDFS other than -64
void readMkDir(SimpleReader &sReader, int fieldLength) {
	//fields
	int intVal;
	int16 int16Val;
	long64 longVal;
	byte byteVal;
	string str;

	//EDITLOG_OP_OPTIMIZATION when not supported, this contains
	//length field. 
	//Log version, -64 support EDITLOG_OP_OPTIMIZATION
	//sReader.readIntBigEndian(&intVal);

	int readmkdir_startposition = sReader.getCurrentPosition();

	intVal = 0;
	cout << "Length=" << intVal << endl;

	sReader.readLong64BigEndian(&longVal);
	cout << "inode=" << longVal << endl;

	//read string path
	//string encoding, length followed by char arrary of same length
	//sReader.readInt16BigEndian(&int16Val); 
	//cout << "string length=" << (int) int16Val << endl;

	//char *path = new char[int16Val + 1];
	//sReader.readCharArray(path, int16Val);
	//path[int16Val] = '\0';
	sReader.readStringEditlogInt16Encoding(str);
	//sReader.readString(str, int16Val);
	cout << "path=" << str << " length=" << str.length()  << endl;

	sReader.readLong64BigEndian(&longVal);
	cout << "timestamp=" << longVal << endl;

	//access time is supported, but it is not updated currently for 
	//performance reasons
	sReader.readLong64BigEndian(&longVal); 
	cout << "access time=" << longVal << endl;

	//read permission status;
	// String in permission status is stored as Text.class, which accepts 
	// a different encoder/decoder if present
  	//username = Text.readString(in, Text.DEFAULT_MAX_LEN);
    //groupname = Text.readString(in, Text.DEFAULT_MAX_LEN);
    //permission = FsPermission.read(in);
	sReader.readByte(&byteVal);
	sReader.readString(str, (int)byteVal);
	cout << "username=" << str << " length=" << (int)byteVal  << endl;
	sReader.readByte(&byteVal);
	sReader.readString(str, (int)byteVal);
	cout << "groupname=" << str << " length=" << (int)byteVal  << endl;
	sReader.readInt16BigEndian(&int16Val);
	cout << "Mode=" << (int)int16Val << endl;


	//ACL entries: 
	//First length, followed by length number of ACL entries
	sReader.readIntBigEndian(&intVal); 
	cout << "ACL size=" << intVal << endl;

	if(intVal == 0) { 
		cout << "ACL size is zero" << endl; 
	} else {
		cout << "ACL size is not zero. Need some processing. Exiting now." << endl;	
	}

	//Read XAttrEditLogProto from the editlog
	string filename = sReader.getFilename();
	ifstream editLogIfStream; 
	editLogIfStream.open(filename, ios::in | ios::binary);
	editLogIfStream.seekg(sReader.getCurrentPosition(), ios::beg);

	/*
	hadoop::hdfs::XAttrEditLogProto xattrEditLogProto; 
    google::protobuf::io::ZeroCopyInputStream* zRawInput = new google::protobuf::io::IstreamInputStream(&editLogIfStream);
	google::protobuf::io::CodedInputStream coded_input(zRawInput);
    bool clean_eof;
    google::protobuf::util::ParseDelimitedFromCodedStream(&xattrEditLogProto, &coded_input, &clean_eof);
	if (xattrEditLogProto.has_src()) {
		cout << "xattrEditLogProto has src: " << xattrEditLogProto.src() << endl; 
	} else  {
		cout << "xattrEditLogProto does not have src" << endl;
	}
	cout << "xattrEditLogProto xattrs size=" << xattrEditLogProto.xattrs_size() << endl;

	cout << "codedinputstream curposition:" << coded_input.CurrentPosition() << endl;
	editLogIfStream.close();
	*/
	//zerocopyinputstream reads until the end of the file into a buffer,
	//so ifstream is closed after parsedelimitedFromZeroCopyStream is called. 
	//TODO: need to do something in order to recover the original ifstream os sReader


	//rudimentary parsedelimitedFrom equivalent of java implementation
	hadoop::hdfs::XAttrEditLogProto xattrEditLogProto; 
	if (readDelimitedFrom(sReader, &xattrEditLogProto)) {
		cout << "xattrEditLogProto entry is present in editlog!! Not handled. Exiting." << endl;
		exit(1);
	}

	//recover the stream position by adding length
	//cout << "sReader currentPosition=" << sReader.getCurrentPosition() << endl;
	//sReader.setCurrentPosition(readmkdir_startposition + fieldLength);
	cout << "sReader currentPosition=" << sReader.getCurrentPosition() << endl;
}


/*
 About LayoutFeatures: 
 	- editlog follows LayoutFeature to determine the contents of editlog 

 Editlog Layout version supported by this implementation is: ERASURE_CODING

 Features that are compatible with ERASURE_CODING are (in reverse order of their
 occurance in Feature enum in NameNodeLayoutVersion.java) :

	Feature					LayoutVersion, AncestorLayoutVersion
	============================================================	
    NAMESPACE_QUOTA			-16, -15
    FILE_ACCESS_TIME		-17, -16
    DISKSPACE_QUOTA			-18, -17
    STICKY_BIT				-19, -18
    APPEND_RBW_DIR			-20, -19
    ATOMIC_RENAME			-21, -20
    CONCAT					-22, -21
    SYMLINKS				-23, -22
    DELEGATION_TOKEN		-24, -23
    FSIMAGE_COMPRESSION		-25, -24
    FSIMAGE_CHECKSUM		-26, -25
    REMOVE_REL13_DISK_LAYOUT_SUPPORT	-27, -26
    EDITS_CHECKSUM			-28, -27
    UNUSED					-29, -28
    FSIMAGE_NAME_OPTIMIZATION	-30, -29
    RESERVED_REL20_203		-31, -19
    RESERVED_REL20_204		-32, -31
    RESERVED_REL22			-33, -27
    RESERVED_REL23			-34, -30
    FEDERATION				-35, -34
    LEASE_REASSIGNMENT		-36, -35
    STORED_TXIDS			-37, -36
    TXID_BASED_LAYOUT		-38, -37
    EDITLOG_OP_OPTIMIZATION	-39, -38
    OPTIMIZE_PERSIST_BLOCKS	-40, -39
    RESERVED_REL1_2_0		-41, -32  Reserved: CONCAT
    ADD_INODE_ID			-42, -40
    SNAPSHOT				-43, -42
    RESERVED_REL1_3_0		-44, -41, Reserved: ADD_INODE_ID, SNAPSHOT, FSIMAGE_NAME_OPTIMIZATION
    OPTIMIZE_SNAPSHOT_INODES	-45, -43,
    SEQUENTIAL_BLOCK_ID		-46, -45
    EDITLOG_SUPPORT_RETRYCACHE	-47, -46
    EDITLOG_ADD_BLOCK		-48, -47 
    ADD_DATANODE_AND_STORAGE_UUIDS	-49, -48
    ADD_LAYOUT_FLAGS		-50, -49
    CACHING					-51, -50
    // Hadoop 2.4.0
    PROTOBUF_FORMAT			-52, -51
    EXTENDED_ACL			-53, -52
    RESERVED_REL2_4_0		-54, -51  Reserved Features: PROTOBUF_FORMAT, EXTENDED_ACL
  	ROLLING_UPGRADE			-55, -53
    EDITLOG_LENGTH			-56, -55
    XATTRS					-57, -56
    CREATE_OVERWRITE		-58, -57
    XATTRS_NAMESPACE_EXT	-59, -58
    BLOCK_STORAGE_POLICY	-60, -59
    TRUNCATE				-61, -60
    APPEND_NEW_BLOCK		-62, -61
    QUOTA_BY_STORAGE_TYPE	-63, -62
    ERASURE_CODING			-64, -63
*/
int main(int argc, char* argv[]) {
	
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	if (argc < 2) {
		cout << "Syntax: <progname> <filename>" << endl;
		exit(1);
	}

	string filename = argv[1];

	cout << "Filename: " << filename << endl;
	SimpleReader sReader; 
	sReader.init(filename);
									
/*
 Format- list of fields in sequential order: 

 For version less than -64, editlog supports EDITLOG_LENGTH feature, 
 so use format of LengthPrefixedReader

 Following opcode format is based on LengthPrefixedReader

 1. Log version, layoutVersion (Integer)
 2. layout flags: is an integer and is zero
 3. Opcode 

 The minimum Op has:
   1-byte opcode
   4-byte length
   8-byte txid
   0-byte body
   4-byte checksum

Getting editlog xml from offline viewer:

hdfs oev -i proto/edits_0000000000000000006-0000000000000000014 -o editlog14.xml

*/

	int logVersion = 0; 
	LayoutVersion editLogLayoutVersion; 
	sReader.readIntBigEndian(&logVersion);
	editLogLayoutVersion = getCurrentLayoutVersion(logVersion); 
	cout << "LogVersion: " << logVersion << " description:" << editLogLayoutVersion.getDescription() << endl;

	int layoutflags = 0; 
	sReader.readIntBigEndian(&layoutflags);
	cout << "Layoutflags: " << layoutflags << endl; 

	byte opcode; 
	long64 txid;
	int length; 
	int checksum;
	while (sReader.readByte(&opcode) == 0) {
		cout << "opcode=" << (int)opcode << endl;
		//LengthPrefixedReader
/*		
 The minimum Op has:
   1-byte opcode
   4-byte length
   8-byte txid
   0-byte body
   4-byte checksum
*/
		sReader.readIntBigEndian(&length);
		sReader.readLong64BigEndian(&txid);
		cout << "length=" << length << " txid= " << txid << endl;

		//checksum should be verified before reading the opcode, so that 
		// we dont end up reading corrupted opcode fields.
		switch((int)opcode) {

			case OP_ADD: //0
				cout << "OP_ADD opcode read" << endl;
				readAdd(sReader); 
				break;

			case OP_RENAME_OLD: //1
				cout << "OP_RENAME_OLD read" << endl;
				readRenameOld(sReader);
				break;

			case OP_MKDIR: //3
				cout << "OP_MKDIR opcode mkdir" << endl;
				readMkDir(sReader, length - 8 - 4); //length does not include opcode length and checksum length 
												    //so length - 4 - 8 is the size of the opcode, this size is needed 
													//to reset sReader position when doing protocol buffer reads which 
													//closes the sReader ifstream if used
				break;

			case OP_CLOSE: //9
				cout << "OP_CLOSE opcode read " << endl;
				readClose(sReader);
				break;
			
			case OP_END_LOG_SEGMENT: //23
				cout << "OP_END_LOG_SEGMENT read" << endl;
				readEndLogSegment(sReader);
				cout << "End of edit log segment" << endl;
				exit(1);
				break;

			case OP_START_LOG_SEGMENT: //24
				cout << "OP_START_LOG_SEGMENT opcode read" << endl;
				readStartLogSegment(sReader); 
				break;

			case OP_SET_GENSTAMP_V2: //31
				cout << "OP_SET_GENSTAMP_V2 opcode read" << endl;
				readSetGenStampv2(sReader);
				break;

  			case OP_ALLOCATE_BLOCK_ID: // = 32, 		//AllocateBlockIdOp.class),
				cout << "OP_ALLOCATE_BLOCK_ID opcode read" << endl;
				readAllocateBlockId(sReader);
				break;

  			case OP_ADD_BLOCK: // 33, 				//AddBlockOp.class),
				cout << "OP_ADD_BLOCK opcode read" << endl;
				readAddBlock(sReader);
				break;

			case OP_INVALID: // -1 
				cout << "INVALID opcode read" << endl; 
				break; 

			case OP_INVALID_BYTE: // -1 
				cout << "INVALID_BYTE opcode read" << endl; 
				break; 

			default:
				cout << "UNKNOWN opcode" << endl;
				exit(1);
				break; 
		}

		//checksum shows up after OP fields
		sReader.readIntBigEndian(&checksum);
		cout << "checksum = " << checksum << endl;

		if (opcode == (byte) OP_INVALID) 
			break;
	}

	sReader.closeFileStreamObj();
	
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}


