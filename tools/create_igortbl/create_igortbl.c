
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../../engines/igor/resource_ids.h"

/*
	uint32 : 'ITBL'
	uint32 : version/tag
	uint8  : number of game versions
	repeat (number of game versions) {
		uint32 : borland overlay size
		uint32 : offset to resource table
	}
	repeat (number of game versions) {
		repeat (number of resources tables) {
			uint16 : number of entries
			repeat (number of entries) {
				uint16 : id
				uint32 : offset
				uint32 : length
			}
		}
	}
*/

#define MAX_VERSIONS 2

typedef unsigned char   uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

struct ResourceEntry {
	int id;
	uint32 offs;
	uint32 size;
};

static const int _sizeOfResourceEntry = 10;

static const struct ResourceEntry _resourceEntriesEngDemo100[] = {
#include "resource_en_demo100.h"
	{ 0, 0, 0 }
};

static const struct ResourceEntry _resourceEntriesEngDemo110[] = {
#include "resource_en_demo110.h"
	{ 0, 0, 0 }
};

struct GameVersion {
	uint32 borlandOverlaySize;
	const struct ResourceEntry *resourceEntries;
};

static const struct GameVersion _gameVersions[] = {
	{ 4086790, &_resourceEntriesEngDemo100[0] },
	{ 4094103, &_resourceEntriesEngDemo110[0] },
	{ 0, 0 }
};

static void writeByte(FILE *fp, uint8 b) {
	fwrite(&b, 1, 1, fp);
}

static void writeUint16BE(FILE *fp, uint16 value) {
	writeByte(fp, (uint8)(value >> 8));
	writeByte(fp, (uint8)(value & 0xFF));
}

static void writeUint32BE(FILE *fp, uint32 value) {
	writeUint16BE(fp, (uint16)(value >> 16));
	writeUint16BE(fp, (uint16)(value & 0xFFFF));
}

static void writeResourceEntry(FILE *fp, const struct ResourceEntry *re) {
	writeUint16BE(fp, re->id);
	writeUint32BE(fp, re->offs);
	writeUint32BE(fp, re->size);
}

static const uint32 ITBL_TAG = 0x4954424C;
static const uint32 CURRENT_VERSION = 1;

static void createTableFile(FILE *fp) {
	int i, j, gameVersionsCount;
	uint32 resourceEntriesTableSize[MAX_VERSIONS];
	uint32 resourceEntriesTableOffs[MAX_VERSIONS];

	/* get resource entries tables size */
	for (i = 0; _gameVersions[i].resourceEntries; ++i) {
		assert(i <= MAX_VERSIONS);
		resourceEntriesTableSize[i] = 0;
		for (j = 0; _gameVersions[i].resourceEntries[j].id != 0;  ++j) {
			++resourceEntriesTableSize[i];
		}
	}
	gameVersionsCount = i;

	/* header */
	writeUint32BE(fp, ITBL_TAG);
	writeUint32BE(fp, CURRENT_VERSION);

	/* game versions header */
	writeByte(fp, gameVersionsCount);
	resourceEntriesTableOffs[0] = 9 + gameVersionsCount * 8;
	for (i = 1; i < gameVersionsCount; ++i) {
		resourceEntriesTableOffs[i] = resourceEntriesTableOffs[i - 1] + 2 + resourceEntriesTableSize[i - 1] * _sizeOfResourceEntry;
	}
	for (i = 0; i < gameVersionsCount; ++i) {
		writeUint32BE(fp, _gameVersions[i].borlandOverlaySize);
		writeUint32BE(fp, resourceEntriesTableOffs[i]);
	}

	/* resource entries tables */
	for (i = 0; _gameVersions[i].resourceEntries; ++i) {
		writeUint16BE(fp, resourceEntriesTableSize[i]);
		for (j = 0; _gameVersions[i].resourceEntries[j].id != 0;  ++j) {
			writeResourceEntry(fp, &_gameVersions[i].resourceEntries[j]);
		}
	}
}

int main(int argc, char *argv[]) {
	FILE *fp = fopen("IGOR.TBL", "wb");
	if (fp) {
		createTableFile(fp);
		fclose(fp);
	}
	return 0;
}
