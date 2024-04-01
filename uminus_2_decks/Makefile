
VPATH= .:../common/
CC=gcc
CFLAGS=-c -Wall -l -O2
LDFLAGS= -L
SOURCES=uminus_2_decks.c arg_get_arguments.c bit_utilities.c \
	sequence_utilities.c deck_utilities.c platform_utilities.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=uminus_2_decks

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) uminus_2_decks.o $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f *.o
	rm -f uminus_2_decks
