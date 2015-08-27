

all : reader writer
	g++ main.cpp -c -o main.o
	g++ reader.o writer.o main.o -o test

reader:
	g++ reader.cpp -c -o reader.o

writer:
	g++ write.cpp -c -o writer.o
