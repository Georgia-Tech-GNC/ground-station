_EXE=.exe
STATIC=-static

ifeq "$(COMSPEC)" ""
_EXE=
STATIC=
endif

CXX_AVAILABLE := $(shell $(CXX) --version 2>/dev/null)

C_PATH_LIB = ../lib/protocol/protocol.c ../lib/protocol/crc_hash.c ../lib/protocol/packet_encode.c

CFLAGS=$(C_PATH_LIB) -O2 -Wall -I../src -I../lib/protocol -I../lib/tst -I../ -I../includes $(STATIC) $(COMPACT) $(NOCOLOR) $(DEBUG)
LIBS=

TESTS_SRC_C=$(wildcard t_*.c)
TESTS_C=$(TESTS_SRC_C:.c=)

ifneq ($(CXX_AVAILABLE),)
  TESTS_SRC_CPP=$(wildcard t_*.cpp)
  TESTS_CPP=$(TESTS_SRC_CPP:.cpp=)
	CXXFLAGS=$(CFLAGS)
endif

TESTS_SRC=$(TST_SRC_C) $(TST_SRC_CPP)
TESTS=$(TESTS_C) $(TESTS_CPP)

# targets
all: $(TESTS)

SRCDIR=../lib/tst

runtests: all
	./tstrun

MAKEFLAGS += --no-builtin-rules

%.o: %.c
	$(CC) $(CFLAGS) -o $*.o -c $< 

%: %.o functions.o
	$(CC) -o $* $< functions.o -lm

.PRECIOUS: %.o

clean:
	rm -f $(TESTS) $(TESTS:=.exe) $(TESTS:=.o) test.log functions.o