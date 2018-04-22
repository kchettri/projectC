/*
	socketbuf used to create istream to read from.
 */

#include <istream>

/* projectC locals */
#include "defs.h"

class socketbuf : public std::streambuf {
private: 
	int socketfd;
public:
	socketbuf() {}
	~socketbuf() {}

	void setSocketfd(int socketfd); 

protected:
	int underflow() override;
	std::streambuf* setbuf(char* s, std::streamsize n) override;
	int uflow() override;
};

