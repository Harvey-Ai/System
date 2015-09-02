
all: pipe fifo


pipe:
	g++ pipe.cpp -o pipe

fifo:
	g++ fifoWriter.cpp -o fifoWriter
	g++ fifoReader.cpp -o fifoReader

.PHONY: clean

clean:
	@-rm fifoWriter
	@-rm fifoReader
	@-rm pipe
	@-rm fifoFile

