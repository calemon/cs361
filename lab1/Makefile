CXX=/home/smarz1/Programs/x86_64/bin/g++
CXXFLAGS=-Wall -std=c++17 -g -O0
PROGRAM_OUTPUT=lab1

default: main
all: main

main: driver.o lab1.o
	$(CXX) $(CXXFLAGS) -o $(PROGRAM_OUTPUT) lab1.o driver.o

driver.o: driver.cpp lab1.h
	$(CXX) $(CXXFLAGS) -o driver.o -c driver.cpp

lab1.o: lab1.cpp lab1.h
	$(CXX) $(CXXFLAGS) -o lab1.o -c lab1.cpp

clean:
	rm -f lab1 *.o
