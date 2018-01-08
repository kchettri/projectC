/*
 * logger.cc
 *
 *  Created on: Jan 2, 2018
 *      Author: kamal
 */

using namespace std;

/* locals */
#include "logger.h"

void Logger::loginit(string module) {
    this->module = module;
}
void Logger::log(string str) {
    ostringstream strstream;
    strstream << module << ": " << str << endl;
    cout << strstream.str() << flush;
}

