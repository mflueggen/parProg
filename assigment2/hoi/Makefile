CC = g++
OBJ = main.o
LIBS = -fopenmp -lcrypto
CPPFLAGS = -DCONVERT_TO_BIG_ENDIAN -std=c++17 -O3

hoi: $(OBJ)
	$(CC) $(CPPFLAGS) $(LIBS) -o $@ $(OBJ)

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $(LIBS) $< 

clean:
	rm hoi $(OBJ)

