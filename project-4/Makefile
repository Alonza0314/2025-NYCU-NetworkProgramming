CXX=g++
CXXFLAGS=-std=c++14 -Wall -pedantic -pthread -lboost_system
CXX_INCLUDE_DIRS=/usr/local/include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))

build:
	rm -rf bin
	rm -rf bin
	mkdir -p bin
	g++ -o bin/noop fromTA/command/delayedremovetag.cpp
	g++ -o bin/noop fromTA/command/noop.cpp
	g++ -o bin/number fromTA/command/number.cpp
	g++ -o bin/removetag fromTA/command/removetag.cpp
	g++ -o bin/removetag0 fromTA/command/removetag0.cpp

copy:
	cp /bin/ls /bin/cat /bin/wc bin/

part1:
	$(CXX) http_server.cpp -o http_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
	$(CXX) console.cpp -o console.cgi $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

part2:
	g++ cgi_server.cpp -o cgi_server -lws2_32 -lwsock32 -std=c++14

clean:
	rm -f http_server console.cgi cgi_server.exe

test:
	cp Makefile ../project-4-demo-sample-Alonza0314/src/313581047/Makefile
	cp http_server.cpp ../project-4-demo-sample-Alonza0314/src/313581047/http_server.cpp
	cp console.cpp ../project-4-demo-sample-Alonza0314/src/313581047/console.cpp
	cp http.hpp ../project-4-demo-sample-Alonza0314/src/313581047/http.hpp
	cp -r header ../project-4-demo-sample-Alonza0314/src/313581047/