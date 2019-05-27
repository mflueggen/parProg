CXXFLAGS = -O3 -std=c++17 -fopenmp
LDLIBS = -lcrypt
BIN = decrypt

all: $(BIN)

clean:
	rm -rf $(BIN)

timer: CXXFLAGS += -DTIMER
timer: clean decrypt
