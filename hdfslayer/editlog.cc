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
// Every opcode has read and write functions, that reads/writes
// their respective fields
void readStartLogSegment(SimpleReader &sReader) {
//does nothing, no fields are present	
}

//OP_ADD
void readAdd(SimpleReader &sReader) {
	//fields
	int intVal; 
	int16 int16Val;
	long64 longVal;

	sReader.readLong64BigEndian(&longVal);
	cout << "inode=" << longVal << endl;
	
	sReader.readIntBigEndian(&intVal); 
	cout << "Length = " << intVal << endl;
	
	sReader.readLong64BigEndian(&longVal);
	cout << "longVal= " << longVal << endl;

	sReader.readInt16BigEndian(&int16Val); 
	cout << "path length=" << (int) int16Val << endl;
}


//OP_MKDIR, 
// currently all ops do not handle previous version of HDFS other than -64
void readMkDir(SimpleReader &sReader) {
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
	cout << "Length=" << intVal << endl;

	sReader.readLong64BigEndian(&longVal);
	cout << "inode=" << longVal << endl;

	//read string path
	//string encoding, length followed by char arrary of same length
	sReader.readInt16BigEndian(&int16Val); 
	cout << "string length=" << (int) int16Val << endl;

	//char *path = new char[int16Val + 1];
	//sReader.readCharArray(path, int16Val);
	//path[int16Val] = '\0';
	sReader.readString(str, int16Val);
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
	//

	//Read XAttrEditLogProto from the editlog
	hadoop::hdfs::XAttrEditLogProto xattrEditLogProto; 
	ifstream& editLogIfStream = sReader.getIfStream();
    google::protobuf::io::ZeroCopyInputStream* zRawInput = new google::protobuf::io::IstreamInputStream(&editLogIfStream);
    bool clean_eof;
    google::protobuf::util::ParseDelimitedFromZeroCopyStream(&xattrEditLogProto, zRawInput, &clean_eof);
	if (xattrEditLogProto.has_src()) {
		cout << "xattrEditLogProto has src: " << xattrEditLogProto.src() << endl; 
	} else  {
		cout << "xattrEditLogProto does not have src" << endl;
	}
	cout << "xattrEditLogProto xattrs size=" << xattrEditLogProto.xattrs_size() << endl;
	//zerocopyinputstream reads until the end of the file into a buffer,
	//so ifstream is closed after parsedelimitedFromZeroCopyStream is called. 
	//TODO: need to do something in order to recover the original ifstream os sReader
	
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
				exit(0);
				break;

			case OP_MKDIR: //3
				cout << "OP_MKDIR opcode mkdir" << endl;
				readMkDir(sReader); 
				break;

			case OP_START_LOG_SEGMENT: //24
				cout << "OP_START_LOG_SEGMENT opcode read" << endl;
				readStartLogSegment(sReader); 
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

	sReader.close();
	
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}


