/*
 * logger.cc
 *
 *  Created on: Jan 2, 2018
 *      Author: kamal
 */

class Logger {
private:
    string module;
public:
    void loginit(string module) {
        this->module = module;
    }
    void log(string str) {
        ostringstream strstream;
        strstream << module << ": " << str << endl;
        cout << strstream.str() << flush;
    }
};



