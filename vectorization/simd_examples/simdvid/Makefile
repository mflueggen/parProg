# Host and port of running filter program
FPORT=9876
FHOST=localhost

# Host and port of running video display programm
VHOST=localhost
VPORT=9877

# Video file to be filtered (webcam if empty)
VIDEO=

ARCH=$(shell uname -m)

ifeq ($(ARCH),ppc64le)
	CFLAGS += -maltivec -mabi=altivec
else
	CFLAGS += -msse2
	# Or just go on using AVX
endif

all: filter

netrun: filter
	netcat -lp $(FPORT) | ./filter | netcat $(VHOST) $(VPORT)

send:
	while true; do ./simdvid.py -s $(VIDEO) | netcat $(FHOST) $(FPORT); done

receive:
	while true; do netcat -lp $(VPORT) | ./simdvid.py -r; done

stop:
	killall make

localrun: filter
	./simdvid.py -s $(VIDEO) | ./filter | ./simdvid.py -r

clean:
	$(RM) filter

.PHONY: all clean send netrun receive stop localrun
	
