CFLAGS=-g

# To add new source files to the compile, take the file's name 
# (e.g.: myfile.c), replace the ".c" with ".o", and add an OBJECTS
# line for it below:

CC=gcc
CFLAGS=-ggdb -Wall -Werror
REPOS_SIZE=9000
TESTREPOS=TEST-REPOS	
BLIB=libblstore.a

TESTS=bl-test
TESTS+=#Add your tests here

OBJECTS=blockstore.o 
OBJECTS+=phyblockrw.o bitmap.o hashmap.o hmamap.o block_cache.o mapblockaddr.o inmem_bitmap.o gmamap.o

.SUFFIXES: -test

all: $(TESTS)

$(BLIB): $(BLIB)($(OBJECTS))
$(BLIB)(%.o): %.c

$(TESTS): %: %.o $(BLIB)
	$(CC) $(CFLAGS) -o $@ $@.o -lm $(BLIB)

test: clean $(TESTS)
	@for i in $(TESTS); do \
	    echo ./$$i -rs $(REPOS_SIZE) $(TESTREPOS); \
	    ./$$i -rs $(REPOS_SIZE) $(TESTREPOS); \
	done

clean:
	-rm -f *.a *.o *~ *-test $(TESTREPOS) $(BLIB)
