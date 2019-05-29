CXXFLAGS = -O3 -std=c++17 -fopenmp
LDLIBS = -lcrypt
BIN = decrypt
timer: CXXFLAGS += -DTIMER

all: $(BIN)

clean:
	rm -rf $(BIN)

timer: clean decrypt
