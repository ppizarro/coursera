#**********************
#*
#* Progam Name: MP1. Membership Protocol.
#*
#* Current file: Makefile
#* About this file: Build Script.
#* 
#***********************

CFLAGS =  -Wall -g -std=c++11

all: Application

Application: MP1Node.o EmulNet.o Application.o Log.o Params.o Member.o  
	g++ -o Application MP1Node.o EmulNet.o Application.o Log.o Params.o Member.o ${CFLAGS}

MP1Node.o: MP1Node.cpp MP1Node.h Log.h Params.h Member.h EmulNet.h Queue.h
	g++ -c MP1Node.cpp ${CFLAGS}

EmulNet.o: EmulNet.cpp EmulNet.h Params.h Member.h
	g++ -c EmulNet.cpp ${CFLAGS}

Application.o: Application.cpp Application.h Member.h Log.h Params.h Member.h EmulNet.h Queue.h 
	g++ -c Application.cpp ${CFLAGS}

Log.o: Log.cpp Log.h Params.h Member.h
	g++ -c Log.cpp ${CFLAGS}

Params.o: Params.cpp Params.h 
	g++ -c Params.cpp ${CFLAGS}

Member.o: Member.cpp Member.h
	g++ -c Member.cpp ${CFLAGS}

clean:
	rm -rf *.o Application dbg.log msgcount.log stats.log machine.log
