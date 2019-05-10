raii_thread: main.o
	g++ -O3 -pthread -o raii_thread main.o

main.o: main.cpp
	g++ -std=c++17 -c main.cpp

clean:
	rm -f *.o raii_thread