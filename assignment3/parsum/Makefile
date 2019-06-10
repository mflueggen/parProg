CXXFLAGS = -O3 -std=c++17 -pthread
#LDLIBS =
BIN = parsum
timer: CXXFLAGS += -DTIMER

all: $(BIN)

clean:
	rm -rf $(BIN)

timer: clean $(BIN)