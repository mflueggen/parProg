CC = g++
OBJ = main.o
CPPFLAGS = -DWITH_OMP -std=c++17 -O3 -fopenmp

heatmap: $(OBJ)
	$(CC) $(CPPFLAGS) -o $@ $(OBJ) 

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm heatmap $(OBJ)

