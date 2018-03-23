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

enum Opcodes {
  OP_ADD, //                        ((byte)  0, AddOp.class),
  // deprecated operation
  OP_RENAME_OLD, //                 ((byte)  1, RenameOldOp.class),
  OP_DELETE, //                    ((byte)  2, DeleteOp.class),
  OP_MKDIR, //                      ((byte)  3, MkdirOp.class),
  OP_SET_REPLICATION, //            ((byte)  4, SetReplicationOp.class),
  OP_DATANODE_ADD, //   ((byte)  5), // obsolete
  OP_DATANODE_REMOVE, //((byte)  6), // obsolete
  OP_SET_PERMISSIONS, //            ((byte)  7, SetPermissionsOp.class),
  OP_SET_OWNER, //                  ((byte)  8, SetOwnerOp.class),
  OP_CLOSE, //                      ((byte)  9, CloseOp.class),
  OP_SET_GENSTAMP_V1, //            ((byte) 10, SetGenstampV1Op.class),
  OP_SET_NS_QUOTA, //               ((byte) 11, SetNSQuotaOp.class), // obsolete
  OP_CLEAR_NS_QUOTA, //             ((byte) 12, ClearNSQuotaOp.class), // obsolete
  OP_TIMES, //                      ((byte) 13, TimesOp.class), // set atime, mtime
  OP_SET_QUOTA, //                  ((byte) 14, SetQuotaOp.class),
  OP_RENAME, //                     ((byte) 15, RenameOp.class),
  OP_CONCAT_DELETE, //              ((byte) 16, ConcatDeleteOp.class),
  OP_SYMLINK, //                    ((byte) 17, SymlinkOp.class),
  OP_GET_DELEGATION_TOKEN, //       ((byte) 18, GetDelegationTokenOp.class),
  OP_RENEW_DELEGATION_TOKEN, //     ((byte) 19, RenewDelegationTokenOp.class),
  OP_CANCEL_DELEGATION_TOKEN, //    ((byte) 20, CancelDelegationTokenOp.class),
  OP_UPDATE_MASTER_KEY, //          ((byte) 21, UpdateMasterKeyOp.class),
  OP_REASSIGN_LEASE, //             ((byte) 22, ReassignLeaseOp.class),
  OP_END_LOG_SEGMENT, //            ((byte) 23, EndLogSegmentOp.class),
  OP_START_LOG_SEGMENT, //          ((byte) 24, StartLogSegmentOp.class),
  OP_UPDATE_BLOCKS, //              ((byte) 25, UpdateBlocksOp.class),
  OP_CREATE_SNAPSHOT, //            ((byte) 26, CreateSnapshotOp.class),
  OP_DELETE_SNAPSHOT, //            ((byte) 27, DeleteSnapshotOp.class),
  OP_RENAME_SNAPSHOT, //            ((byte) 28, RenameSnapshotOp.class),
  OP_ALLOW_SNAPSHOT, //             ((byte) 29, AllowSnapshotOp.class),
  OP_DISALLOW_SNAPSHOT, //          ((byte) 30, DisallowSnapshotOp.class),
  OP_SET_GENSTAMP_V2=31, //            ((byte) 31, SetGenstampV2Op.class),
  OP_ALLOCATE_BLOCK_ID, //         ((byte) 32, AllocateBlockIdOp.class),
  OP_ADD_BLOCK, //                  ((byte) 33, AddBlockOp.class),
  OP_ADD_CACHE_DIRECTIVE, //        ((byte) 34, AddCacheDirectiveInfoOp.class),
  OP_REMOVE_CACHE_DIRECTIVE, //     ((byte) 35, RemoveCacheDirectiveInfoOp.class),
  OP_ADD_CACHE_POOL, //             ((byte) 36, AddCachePoolOp.class),
  OP_MODIFY_CACHE_POOL, //          ((byte) 37, ModifyCachePoolOp.class),
  OP_REMOVE_CACHE_POOL, //          ((byte) 38, RemoveCachePoolOp.class),
  OP_MODIFY_CACHE_DIRECTIVE, //     ((byte) 39, ModifyCacheDirectiveInfoOp.class),
  OP_SET_ACL, //                    ((byte) 40, SetAclOp.class),
  OP_ROLLING_UPGRADE_START, //      ((byte) 41, RollingUpgradeStartOp.class),
  OP_ROLLING_UPGRADE_FINALIZE, //   ((byte) 42, RollingUpgradeFinalizeOp.class),
  OP_SET_XATTR, //                  ((byte) 43, SetXAttrOp.class),
  OP_REMOVE_XATTR, //               ((byte) 44, RemoveXAttrOp.class),
  OP_SET_STORAGE_POLICY, //         ((byte) 45, SetStoragePolicyOp.class),
  OP_TRUNCATE, //                   ((byte) 46, TruncateOp.class),
  OP_APPEND, //                     ((byte) 47, AppendOp.class),
  OP_SET_QUOTA_BY_STORAGETYPE, //   ((byte) 48, SetQuotaByStorageTypeOp.class),
  OP_ADD_ERASURE_CODING_POLICY, //  ((byte) 49, AddErasureCodingPolicyOp.class),
  OP_ENABLE_ERASURE_CODING_POLICY, //((byte) 50, EnableErasureCodingPolicyOp.class),
  OP_DISABLE_ERASURE_CODING_POLICY, //((byte) 51,
      //DisableErasureCodingPolicyOp.class),
  OP_REMOVE_ERASURE_CODING_POLICY, //((byte) 52, RemoveErasureCodingPolicyOp.class),
  // Note that the current range of the valid OP code is 0~127
  OP_INVALID      //              ((byte) -1);


};


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

 1. Log version, layoutVersion (Integer)
 2. layout flags: is an integer and is zero

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
	sReader.readByte(&opcode);

	cout << "opcode=" << (int)opcode << endl;


	sReader.close();

	return 0;

}
