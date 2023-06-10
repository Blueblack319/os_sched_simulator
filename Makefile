all: main.o user_mode.o kernel_mode.o process.o utils.o
	g++ main.o user_mode.o kernel_mode.o process.o utils.o -o project3

main.o: main.cpp
	g++ --std=c++17 -c main.cpp

kernel_mode.o: kernel_mode.cpp kernel_mode.h
	g++ --std=c++17 -c kernel_mode.cpp

user_mode.o: user_mode.cpp user_mode.h
	g++ --std=c++17 -c user_mode.cpp

process.o: process.cpp process.h
	g++ --std=c++17 -c process.cpp

utils.o: utils.cpp utils.h
	g++ --std=c++17 -c utils.cpp

clean:
	rm *.o project3