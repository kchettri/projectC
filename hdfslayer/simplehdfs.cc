/*
 * simplehdfs.cc
 *
 *  Simple HDFS server
 */

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class SimpleHDFSMaster {

/* Format of hdfs metdata:
 *
 *  filename | chunkid | chunkserver
 */

private:

vector<string> hdfsservers;
string hdfsroot, dataroot;

public:

	void init() {
		hdfsroot = "/home/projectC/hdfsroot";
		dataroot = hdfsroot + "/dataroot";
		//metadata file
		ofstream mfile;
		mfile.open((hdfsroot + "/mfile").c_str());
		mfile << "Init text";
		mfile.close();
	}

	void read() {

	}

	void write() {

	}

};

class SimpleHDFSChunkServer {

/* Format of metadata:
 *
 *	filename | chunkid | localfilename
 */

private:
	string chunkserverName;
	int nameCounter;
	string getNewFileName() {

		}

public:

	void init() {
		hdfsroot = "/home/projectC/hdfsroot";
		dataroot = hdfsroot + "/dataroot";
		//metadata file
		ofstream mfile;
		mfile.open((hdfsroot + "/mfile").c_str());
		mfile << "Init text";
		mfile.close();
	}

	void read() {

	}

	void write() {

	}
}

int  main() {
	SimpleHDFSMaster shdfsmaster;
	shdfsmaster.init();
	return 0;
}

