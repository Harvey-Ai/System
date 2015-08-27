LIBRARY=-lGL -lglut -lGLU
all:
	g++ $(LIBRARY) OpenGLTester.cpp -o OpenGLTester
	g++ $(LIBRARY) glutTester.cpp -o glutTester

.PHONY: clean

clean:
	-rm testOpenGL
	-rm glutTester
