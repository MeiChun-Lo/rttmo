LIBS:=`pkg-config --libs opencv` 
CFLAGS:=`pkg-config --cflags opencv`

all: rttmo

rttmo: main.cpp tmo.cpp tmo.h 
	g++  -O3  -fopenmp main.cpp tmo.cpp -o rttmo $(LIBS) $(CFLAGS)

clean:
	rm -f rttmo *.o
