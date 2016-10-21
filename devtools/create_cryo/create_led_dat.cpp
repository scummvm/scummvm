#include <stdio.h>

#include "eden.h"
#include "eden_icons.h"
#include "eden_rooms.h"

template <typename T>
static void writeLE(FILE *f, T value) {
	for (int i = 0; i < sizeof(value); i++, value >>= 8) {
		unsigned char b = value & 0xFF;
		fwrite(&b, 1, 1, f);
	}
}

struct _icon_t : icon_t {
	void write(FILE *f) {
		writeLE<int16>(f, sx);
		writeLE<int16>(f, sy);
		writeLE<int16>(f, ex);
		writeLE<int16>(f, ey);
		writeLE<uint16>(f, cursor_id);
		writeLE<unsigned int>(f, action_id);
		writeLE<unsigned int>(f, object_id);
	}
};

static void emitIcons(FILE *f) {
	_icon_t *icons = (_icon_t*)gameIcons;
	for (int i = 0; i < kNumIcons; i++)
		icons[i].write(f);
}

struct _room_t : room_t {
	void write(FILE *f) {
		writeLE<byte>(f, ff_0);
		writeLE<byte>(f, exits[0]);
		writeLE<byte>(f, exits[1]);
		writeLE<byte>(f, exits[2]);
		writeLE<byte>(f, exits[3]);
		writeLE<byte>(f, flags);
		writeLE<uint16>(f, bank);
		writeLE<uint16>(f, party);
		writeLE<byte>(f, level);
		writeLE<byte>(f, video);
		writeLE<byte>(f, location);
		writeLE<byte>(f, background);
	}
};

static void emitRooms(FILE *f) {
	_room_t *rooms = (_room_t*)gameRooms;
	for (int i = 0; i < kNumRooms; i++)
		rooms[i].write(f);
}

static int emitData(char *outputFilename) {
	FILE *f = fopen(outputFilename, "w+b");
	if (!f) {
		printf("ERROR: Unable to create output file %s\n", outputFilename);
		return 1;
	}

	printf("Generating %s...\n", outputFilename);

	emitIcons(f);
	emitRooms(f);

	fclose(f);

	printf("Done!\n");

	return 0;
}

int main(int argc, char **argv) {

	if (argc > 1)
		return emitData(argv[1]);
	else
		printf("Usage: %s <output.dat>\n", argv[0]);

	return 0;
}
