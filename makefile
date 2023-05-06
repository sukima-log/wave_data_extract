main: main.o
	g++ -o main -O2 -mcmodel=medium main.o

main.o: main.cpp
	g++ -c -O2 -mcmodel=medium main.cpp

.PHONY: clean
clean:
	rm -f *.o main *.out