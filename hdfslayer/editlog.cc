/*
  Editlog parser for Namenode editlogs.
 */

using namespace std;
//std headers
#include <iostream> 

//locals
#include "include/simplerw.h"

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
	sReader.readIntBigEndian(&intVal); 
	cout << "Length = " << intVal << endl;
	sReader.readLong64BigEndian(&longVal);
	cout << "longVal= " << longVal << endl;

	sReader.readInt16BigEndian(&int16Val); 
	cout << "path length=" << (int) int16Val << endl;
}


int main(int argc, char* argv[]) {

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
		//cout << "txid=" << std::hex << txid << std::dec << endl; 
		switch((int)opcode) {

			case OP_ADD: //0
				cout << "OP_ADD opcode read" << endl;
				readAdd(sReader); 
				exit(0);
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
				break; 
		}

		//checksum shows up after OP fields
		sReader.readIntBigEndian(&checksum);
		cout << "checksum = " << checksum << endl;

		if (opcode == (byte) OP_INVALID) 
			break;
	}

	sReader.close();

	return 0;
}
