
all: server client

server: server.cpp scm_right
	g++ server.cpp -c -o server.o
	g++ server.o scm_rights_write.o scm_rights_read.o -o server

client: client.cpp 
	g++ client.cpp -o client

scm_right: scm_rights_writefd.cpp scm_rights_readfd.cpp
	g++ -c scm_rights_writefd.cpp -o scm_rights_write.o
	g++ -c scm_rights_readfd.cpp -o scm_rights_read.o


.PHONY: clean

clean:
	@-rm client
	@-rm server
	@-rm server.o
	@-rm scm_rights_write.o
	@-rm scm_rights_read.o
