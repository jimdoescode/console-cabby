cabby: cabby.o
	g++ -g -o cabby cabby.o

cabby.o: cabby.cpp rlutil.h
	g++ -g -c cabby.cpp

clean:
	rm -f *.o cabby
