
VPATH= .:../common/
CC=gcc
CFLAGS=-c -Wall -l -O2
LDFLAGS= -L
SOURCES=dbn_file_creator.c arg_get_arguments.c dbn_de_bruijn.c \
	bit_utilities.c platform_utilities.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dbn_file_creator

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) dbn_file_creator.o $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f *.o
	rm -f dbn_file_creator
