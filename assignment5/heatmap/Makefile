CC = mpic++
OBJ = main.o
CPPFLAGS = -std=c++17 -O3

heatmap: $(OBJ)
	$(CC) $(CPPFLAGS) -o $@ $(OBJ) 

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm heatmap $(OBJ)

