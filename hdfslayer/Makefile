all: bin/simplehdfs bin/simplehdfsclient

simplehdfs.o: simplehdfs.cc message.h
	g++ -std=c++11 -pthread -c simplehdfs.cc

bin/simplehdfs: simplehdfs.o
	g++ -o bin/simplehdfs simplehdfs.o -pthread -lboost_system -lboost_filesystem

simplehdfsclient.o: simplehdfsclient.cc message.h
	g++ -std=c++11 -c simplehdfsclient.cc

bin/simplehdfsclient: simplehdfsclient.o
	g++ -o bin/simplehdfsclient simplehdfsclient.o -lboost_system

clean: 
	rm bin/simplehdfs bin/simplehdfsclient
