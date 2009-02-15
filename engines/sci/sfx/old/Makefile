CC	= gcc
CFLAGS	= -O2 -Wall
LIBS	= -L/usr/lib -lasound
objects = main.o midi_mt32.o midiout.o midiout_unixraw.o midiout_alsaraw.o

sciplaymidi: $(objects)
	$(CC) $(LIBS) -o sciplaymidi $(objects)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c
midi_mt32.o: midi_mt32.c midi_mt32.h midiout.h
	$(CC) $(CFLAGS) -c midi_mt32.c
midiout.o: midiout.c midiout.h midiout_unixraw.h midiout_alsaraw.h
	$(CC) $(CFLAGS) -c midiout.c
midiout_unixraw.o: midiout_unixraw.c midiout_unixraw.h
	$(CC) $(CFLAGS) -c midiout_unixraw.c
midiout_alsaraw.o: midiout_alsaraw.c midiout_alsaraw.h
	$(CC) $(CFLAGS) -c midiout_alsaraw.c

.PHONY: clean distclean
clean:
	 rm -f sciplaymidi $(objects)
distclean:
	 rm -f sciplaymidi $(objects) *~
