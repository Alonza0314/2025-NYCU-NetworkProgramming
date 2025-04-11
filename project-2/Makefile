make:
	g++ -o np_simple np_simple.cpp
	g++ -o np_single_proc np_single_proc.cpp

make-simple:
	g++ -o np_simple np_simple.cpp

make-single-proc:
	g++ -o np_single_proc np_single_proc.cpp

run-simple:
	rm -f np_simple
	g++ -o np_simple np_simple.cpp
	clear
	./np_simple

run-single-proc:
	rm -f np_single_proc
	g++ -o np_single_proc np_single_proc.cpp
	clear
	./np_single_proc

clean:
	rm -f np_simple
	rm -f np_single_proc

clean-simple:
	rm -f np_simple

clean-single-proc:
	rm -f np_single_proc

build:
	rm -rf bin
	mkdir -p bin
	g++ -o bin/noop fromTA/noop.cpp
	g++ -o bin/number fromTA/number.cpp
	g++ -o bin/removetag fromTA/removetag.cpp
	g++ -o bin/removetag0 fromTA/removetag0.cpp

copy:
	cp /bin/ls /bin/cat /bin/wc bin/
