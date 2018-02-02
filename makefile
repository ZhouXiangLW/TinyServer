server: ./src/strtools.o ./src/epolltools.o ./src/request.o ./src/server.o ./src/ostools.o
	g++ ./src/strtools.o ./src/epolltools.o ./src/request.o ./src/server.o ./src/ostools.o -o server

strtools.o: ./src/strtools.h ./src/strtools.cc
	g++ -c ./src/strtools.cc

epolltools.o: ./src/epolltools.h ./src/epolltools.cc
	g++ -c ./src/epolltools.cc

ostools.o: ./src/ostools.h ./src/ostools.cc
	g++ -c ./src/ostools.cc

request.o: ./src/request.h ./src/request.cc
	g++ -c ./src/request.cc

server.o: ./src/server.h ./src/server.cc
	g++ -c ./src/server.cc

clean:
	rm src/*.o