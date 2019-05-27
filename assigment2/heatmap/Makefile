CC = g++
OBJ = main.o
LIBS = -fopenmp
CPPFLAGS = -DWITH_OMP -std=c++17 -O3

heatmap: $(OBJ)
	$(CC) $(CPPFLAGS) -o $@ $(OBJ) $(LIBS)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm heatmap $(OBJ)

