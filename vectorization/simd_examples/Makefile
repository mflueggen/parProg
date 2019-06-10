ARCH=$(shell uname -m)

ifeq ($(ARCH),ppc64le)
	CFLAGS += -maltivec -mabi=altivec
else
    CFLAGS += -msse2
endif

TARGETS=calc_abs squared_2d_abs

all: $(TARGETS)

clean:
	$(RM) $(TARGETS)

.PHONY: clean all
