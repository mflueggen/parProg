INFO_FLAGS_GCC   = -fopt-info-vec-all
INFO_FLAGS_CLANG = -Rpass="loop|vect" -Rpass-missed="loop|vect" -Rpass-analysis="loop|vect"

# Change me for Clang
INFO_FLAGS = $(INFO_FLAGS_GCC)

CFLAGS=-O2 -ftree-vectorize $(INFO_FLAGS) -mavx -mtune=native
CXXFLAGS=$(CFLAGS) -std=c++17

C_FILES=$(shell find . -iname "*.c")
O_FILES=$(C_FILES:%.c=%.o)

all: $(O_FILES) autovector

clean:
	$(RM) $(O_FILES) autovector

.PHONY: all clean
