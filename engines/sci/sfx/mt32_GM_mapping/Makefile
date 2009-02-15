CC	=gcc
CFLAGS	=-O2 -Wall
objects =main.o

mtgm: $(objects)
	$(CC) -o mtgm $(objects)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

.PHONY: clean distclean
clean:
	 rm -f mtgm $(objects)
distclean:
	 rm -f mtgm $(objects) *~
