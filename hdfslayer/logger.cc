/*
 * Copyright
 *  logger.cc
 *
 */

/* C++ header */
#include <iostream>
#include <sstream>

/* locals */
#include "logger.h"

using namespace std;

void Logger::loginit(string module) {
    this->module = module;
}
void Logger::log(string str) {
    ostringstream strstream;
    strstream << module << ": " << str << endl;
    cout << strstream.str() << flush;
}

