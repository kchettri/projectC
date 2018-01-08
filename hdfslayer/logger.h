/*
 * logger.h
 *
 */

#ifndef LOGGER_H_
#define LOGGER_H_

class Logger {
private:
    std::string module;
public:
    void loginit(std::string module);
    void log(std::string str);
};

#endif /* LOGGER_H_ */
