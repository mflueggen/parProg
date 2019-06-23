CC = g++
CPPFLAGS = -std=c++17 -O3 -DNDEBUG
LDFLAGS = -lOpenCL


OBJ = main.o

heatmap: $(OBJ)
	$(CC) $(CPPFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm heatmap $(OBJ)

