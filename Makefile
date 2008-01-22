

CFLAGS = -ggdb -Wall
#LDFLAGS = -lpthread

CC = gcc


OBJ = $(SRC:.c=.o)

SRC = mdrain.c
APPL = mdrain


$(APPL): $(OBJ)

all:	$(APPL)

clean:
	rm -f *.o $(APPL)

distclean: clean
	rm -f *~

install: all
	install mdrain /usr/bin

uninstall:
	rm -f /usr/bin/mdrain
