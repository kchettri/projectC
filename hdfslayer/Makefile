all: bin/simplehdfs bin/simplehdfsclient

simplehdfs.o: simplehdfs.cc message.h
	g++ -std=c++11 -c simplehdfs.cc

bin/simplehdfs: simplehdfs.o
	g++ -o bin/simplehdfs simplehdfs.o

simplehdfsclient.o: simplehdfsclient.cc message.h
	g++ -std=c++11 -c simplehdfsclient.cc

bin/simplehdfsclient: simplehdfsclient.o
	g++ -o bin/simplehdfsclient simplehdfsclient.o

clean: 
	rm bin/simplehdfs bin/simplehdfsclient
