CC = g++
OBJ = md5.o main.o
LIBS = -fopenmp
CPPFLAGS = -DCONVERT_TO_BIG_ENDIAN -std=c++17 -O3

hoi: $(OBJ)
	$(CC) $(CPPFLAGS) $(LIBS) -o $@ $(OBJ)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $<

clean:
	rm hoi $(OBJ)

