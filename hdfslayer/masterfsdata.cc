
/**
 *  Format of the metadata in namenode according to
 *  org.apache.hadoop.hdfs.server.namenode.FSImageFormat
 *
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
 *   childrenSize: int,
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

class FSImage


