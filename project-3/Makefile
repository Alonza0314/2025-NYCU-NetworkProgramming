make:
	g++ -o np_multi_proc np_multi_proc.cpp

run:
	rm -f np_multi_proc
	g++ -o np_multi_proc np_multi_proc.cpp
	clear
	./np_multi_proc

clean:
	rm -f np_multi_proc

build:
	rm -rf bin
	mkdir -p bin
	g++ -o bin/noop fromTA/noop.cpp
	g++ -o bin/number fromTA/number.cpp
	g++ -o bin/removetag fromTA/removetag.cpp
	g++ -o bin/removetag0 fromTA/removetag0.cpp

copy:
	cp /bin/ls /bin/cat /bin/wc bin/
