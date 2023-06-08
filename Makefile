CC      = cc
CFLAGS  = -std=c99 -g3 -Wall -Wextra -Wpedantic
LDFLAGS = -lm -lraylib

all: bdvd

bdvd: bdvd.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f bdvd

compile_flags.txt: Makefile
	rm -f $@
	@for f in $(CFLAGS);  do echo $$f | tee -a $@; done
	@for f in $(LDFLAGS); do echo $$f | tee -a $@; done
