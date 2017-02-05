TEST=slab-tester.c
DEF=objects.def
SOURCE=slab.c list.c slabbuild.c slabobj.c#Add other source files here
CC=gcc
CFLAGS=-g -I. -Wall -Werror
TARGET=slab-tester
DEBFLAGS=-O2
default: all

CFLAGS += $(DEBFLAGS)

all: $(TARGET)

$(TARGET): $(SOURCE:.c=.o) $(TEST:.c=.o) $(DEF)
	$(CC) $(CFLAGS) -o $@ $(SOURCE:.c=.o) $(TEST:.c=.o)

$(TEST:.c=.o): $(TEST) $(DEF) 
	$(CC) -c $(CFLAGS) -o $@ $<
clean:
	rm -f *.o
	rm -f $(TARGET)

