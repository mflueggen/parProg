
OBJ = main.o
CPPFLAGS = -std=c++17 -O3 -lOpenCL

heatmap: $(OBJ)
	$(CC) $(CPPFLAGS) -o $@ $(OBJ) 

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm heatmap $(OBJ)

