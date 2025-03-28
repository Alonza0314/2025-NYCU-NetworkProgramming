make:
	g++ -o npshell npshell.cpp

run:
	rm -f npshell
	g++ -o npshell npshell.cpp
	clear
	./npshell

build:
	rm -rf bin
	mkdir -p bin
	g++ -o bin/noop fromTA/noop.cpp
	g++ -o bin/number fromTA/number.cpp
	g++ -o bin/removetag fromTA/removetag.cpp
	g++ -o bin/removetag0 fromTA/removetag0.cpp

copy:
	cp /bin/ls /bin/cat /bin/wc bin/

clean:
	rm -f npshell
