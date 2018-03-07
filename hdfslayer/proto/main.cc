#include <iostream>
#include <fstream>

#include "hadoop.hdfs/src/fsimage.pb.h"

using namespace std;

int main(int argc, char* argv[]) {

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	if (argc != 2) {
		cerr << "Usage:  " << argv[0] << " fsimagefile" << endl;
		return -1;
	}
 
	hadoop::hdfs::fsimage::FileSummary fsSummary; 
	
	fstream imgFile(argv[1], ios::in | ios::binary);

	int fsize = 0;
    fsize = imgFile.tellg();
    imgFile.seekg( 0, std::ios::end );
    fsize = imgFile.tellg() - fsize;

	cout << "File size = " << fsize << endl;
	imgFile.seekg(fsize - 4, std::ios::beg);
	
	cout << "File pointer moved to = " << imgFile.tellg() << endl;

	int fileSummaryLength; 
	imgFile >> fileSummaryLength; 

	cout << "FileSummaryLength=" << fileSummaryLength << endl;

//	if (!fsSummary.ParseFromIstream(&imgFile)) {
//		cerr << "Failed to parse fsSummary from fsimagefile." << endl;
//		return -1;
//	}

	google::protobuf::ShutdownProtobufLibrary();

	return 0; 

}
