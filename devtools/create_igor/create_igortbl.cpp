/* ScummVM Tools
 * Copyright (C) 2007 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "resource_ids.h"

/*
	uint32 : 'ITBL'
	uint32 : version/tag (6; the engine also reads versions 4 and 5)
	uint32 : offset to strings table
	uint8  : number of game versions
	repeat (number of game versions) {
		uint32 : borland overlay size
		uint32 : offset to resources table
		uint32 : offset to sounds table
	}
	embedded data blobs (version 6), referenced by resource entries with source 2
	repeat (number of resources tables) {
		uint16 : number of entries
		repeat (number of entries) {
			uint16 : id
			uint32 : offset (source 2: absolute offset inside this file)
			uint32 : length
			uint8  : source (0: resource container, 2: embedded in this file)
		}
	}
	repeat (number of sounds tables) {
		uint16 : number of entries
		repeat (number of entries) {
			uint32 : offset
		}
	}
	uint16 : number of strings
	repeat (number of strings) {
		uint8 : id
		uint8 : language (0:any, 1:english, 2:spanish)
		uint8 : strlen
		char[] : string
	}
*/

#define MAX_TABLES 16
#define TABLE_SIZE(x) (sizeof(x)/sizeof(x[0]))

typedef unsigned char   uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

// Where the data of a resource is read from. Catalog rows leave it out for the
// resource container (the version's overlay file).
enum {
	SOURCE_CONTAINER = 0,
	SOURCE_TBL = 2 // data embedded in IGOR.TBL (see _embeddedBlobs)
};

struct ResourceEntry {
	int id;
	uint32 offs;
	uint32 size;
	uint8 source;
};

static struct ResourceEntry _resourceEntriesEngDemo100[] = {
#include "resource_en_demo100.h"
};

static struct ResourceEntry _resourceEntriesEngDemo110[] = {
#include "resource_en_demo110.h"
};

static struct ResourceEntry _resourceEntriesEngFloppy[] = {
#include "resource_en_floppy.h"
};

static struct ResourceEntry _resourceEntriesSpaCd[] = {
#include "resource_sp_cdrom.h"
};

static struct ResourceEntry _resourceEntriesSpaFloppy[] = {
#include "resource_sp_floppy.h"
};

static struct {
	struct ResourceEntry *p;
	int count;
} _resourceEntriesList[] = {
	{ _resourceEntriesEngDemo100, TABLE_SIZE(_resourceEntriesEngDemo100) },
	{ _resourceEntriesEngDemo110, TABLE_SIZE(_resourceEntriesEngDemo110) },
	{ _resourceEntriesEngFloppy, TABLE_SIZE(_resourceEntriesEngFloppy) },
	{ _resourceEntriesSpaCd,      TABLE_SIZE(_resourceEntriesSpaCd) },
	{ _resourceEntriesSpaFloppy,  TABLE_SIZE(_resourceEntriesSpaFloppy) },
	{ 0, 0 }
};

static const uint32 _soundEntriesEngDemo[] = {
#include "fsd_en_demo.h"
};

static const uint32 _soundEntriesEngFloppy[] = {
#include "fsd_en_floppy.h"
};

static const uint32 _soundEntriesSpaCd[] = {
#include "fsd_sp_cdrom.h"
};

static const uint32 _soundEntriesSpaFloppy[] = {
#include "fsd_sp_floppy.h"
};

static struct {
	const uint32 *p;
	int count;
} _soundEntriesList[] = {
	{ _soundEntriesEngDemo, TABLE_SIZE(_soundEntriesEngDemo) },
	{ _soundEntriesEngFloppy, TABLE_SIZE(_soundEntriesEngFloppy) },
	{ _soundEntriesSpaCd,   TABLE_SIZE(_soundEntriesSpaCd) },
	{ _soundEntriesSpaFloppy, TABLE_SIZE(_soundEntriesSpaFloppy) },
	{ 0, 0 }
};

enum {
	STR_LANG_ANY = 0,
	STR_LANG_ENG = 1,
	STR_LANG_SPA = 2
};

struct StringEntry {
	int id;
	uint8 language;
	const char *str;
};

static struct StringEntry _stringEntries[] = {
#include "strings.h"
};

struct GameVersion {
	uint32 borlandOverlaySize;
	struct ResourceEntry *resourceEntries;
	const uint32 *soundEntries;
};

static const struct GameVersion _gameVersions[] = {
	{ 4086790, _resourceEntriesEngDemo100, _soundEntriesEngDemo },
	{ 4094103, _resourceEntriesEngDemo110, _soundEntriesEngDemo },
	{ 9115648, _resourceEntriesSpaCd,      _soundEntriesSpaCd   },
	{ 11199335, _resourceEntriesSpaFloppy, _soundEntriesSpaFloppy },
	{ 8819339, _resourceEntriesEngFloppy, _soundEntriesEngFloppy }
};

static const uint32 ITBL_TAG = 0x4954424C;
static const uint32 CURRENT_VERSION = 6;
static const uint32 DEFAULT_OFFSET = 0x12345678;

struct TablePtrOffset {
	const void *p;
	uint32 offset;
};

static int _tablePtrOffsetCount = 0;
struct TablePtrOffset _tablePtrOffset[MAX_TABLES];

static void addPtrOffset(FILE *fp, const void *p) {
	assert(_tablePtrOffsetCount < MAX_TABLES);
	_tablePtrOffset[_tablePtrOffsetCount].p = p;
	_tablePtrOffset[_tablePtrOffsetCount].offset = ftell(fp);
	++_tablePtrOffsetCount;
}

static uint32 getPtrOffset(const void *p) {
	int i;
	uint32 fileOffset = 0;

	for (i = 0; i < _tablePtrOffsetCount; ++i) {
		if (_tablePtrOffset[i].p == p) {
			fileOffset = _tablePtrOffset[i].offset;
			break;
		}
	}
	assert(fileOffset != 0);
	return fileOffset;
}

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

int compareResourceEntry(const void *a, const void *b) {
	return ((struct ResourceEntry *)a)->id - ((struct ResourceEntry *)b)->id;
}

// Data that the game keeps outside its resource container (text tables of the
// Spanish floppy executable). It is stored in the file right after the header and
// referenced by the resource rows marked SOURCE_TBL.
#include "exe_blobs.h"

struct EmbeddedBlob {
	int id;
	const unsigned char *data;
	uint32 size;
	uint32 offset;
};

static struct EmbeddedBlob _embeddedBlobs[] = {
	{ TXT_MainTable, TXT_MainTable_data, sizeof(TXT_MainTable_data), 0 },
	{ TXT_MainVerbs, TXT_MainVerbs_data, sizeof(TXT_MainVerbs_data), 0 },
	{ DAT_FloppySentenceColors, DAT_FloppySentenceColors_data, sizeof(DAT_FloppySentenceColors_data), 0 }
};

static void writeEmbeddedBlobs(FILE *fp) {
	unsigned i;
	for (i = 0; i < TABLE_SIZE(_embeddedBlobs); ++i) {
		_embeddedBlobs[i].offset = ftell(fp);
		fwrite(_embeddedBlobs[i].data, 1, _embeddedBlobs[i].size, fp);
	}
}

static const struct EmbeddedBlob *findEmbeddedBlob(int id) {
	unsigned i;
	for (i = 0; i < TABLE_SIZE(_embeddedBlobs); ++i) {
		if (_embeddedBlobs[i].id == id) {
			return &_embeddedBlobs[i];
		}
	}
	assert(0);
	return 0;
}

static void writeResourceEntriesTable(FILE *fp, struct ResourceEntry *re, int count) {
	int i;

	qsort(re, count, sizeof(struct ResourceEntry), compareResourceEntry);
	writeUint16BE(fp, count);
	for (i = 0; i < count; ++i, ++re) {
		uint32 offs = re->offs, size = re->size;
		if (re->source == SOURCE_TBL) {
			const struct EmbeddedBlob *blob = findEmbeddedBlob(re->id);
			assert(blob->size == size);
			offs = blob->offset;
		}
		writeUint16BE(fp, re->id);
		writeUint32BE(fp, offs);
		writeUint32BE(fp, size);
		writeByte(fp, re->source);
	}
}

static void writeSoundEntriesTable(FILE *fp, const uint32 *fsd, int count) {
	int i;

	writeUint16BE(fp, count);
	for (i = 0; i < count; ++i) {
		writeUint32BE(fp, fsd[i]);
	}
}

int compareStringEntry(const void *a, const void *b) {
	return ((struct StringEntry *)a)->id - ((struct StringEntry *)b)->id;
}

static void writeStringEntriesTable(FILE *fp, struct StringEntry *se, int count) {
	int i, len;

	qsort(se, count, sizeof(struct StringEntry), compareStringEntry);
	writeUint16BE(fp, count);
	for (i = 0; i < count; ++i, ++se) {
		writeUint16BE(fp, se->id);
		writeByte(fp, se->language);
		len = strlen(se->str);
		assert(len < 256);
		writeByte(fp, len);
		fwrite(se->str, 1, len, fp);
	}
}

static void createTableFile(FILE *fp) {
	int i, gameVersionsCount;

	gameVersionsCount = TABLE_SIZE(_gameVersions);

	/* header */
	writeUint32BE(fp, ITBL_TAG);
	writeUint32BE(fp, CURRENT_VERSION);
	writeUint32BE(fp, DEFAULT_OFFSET); /* strings table offset */
	writeByte(fp, gameVersionsCount);

	/* game versions */
	for (i = 0; i < gameVersionsCount; ++i) {
		writeUint32BE(fp, _gameVersions[i].borlandOverlaySize);
		writeUint32BE(fp, DEFAULT_OFFSET); /* resource table offset */
		writeUint32BE(fp, DEFAULT_OFFSET); /* sound table offset */
	}

	/* embedded data */
	writeEmbeddedBlobs(fp);

	/* resources tables */
	for (i = 0; _resourceEntriesList[i].p; ++i) {
		addPtrOffset(fp, _resourceEntriesList[i].p);
		writeResourceEntriesTable(fp, _resourceEntriesList[i].p, _resourceEntriesList[i].count);
	}

	/* sounds tables */
	for (i = 0; i < (int)TABLE_SIZE(_soundEntriesList); ++i) {
		addPtrOffset(fp, _soundEntriesList[i].p);
		writeSoundEntriesTable(fp, _soundEntriesList[i].p, _soundEntriesList[i].count);
	}

	/* strings table */
	addPtrOffset(fp, _stringEntries);
	writeStringEntriesTable(fp, _stringEntries, TABLE_SIZE(_stringEntries));

	/* fix offsets */
	fseek(fp, 8, SEEK_SET);
	writeUint32BE(fp, getPtrOffset(_stringEntries));
	for (i = 0; i < gameVersionsCount; ++i) {
		fseek(fp, 17 + i * 12, SEEK_SET);
		writeUint32BE(fp, getPtrOffset(_gameVersions[i].resourceEntries));
		writeUint32BE(fp, getPtrOffset(_gameVersions[i].soundEntries));
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
