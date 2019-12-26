/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "create_cryomni3d_dat.h"

struct Parts {
	size_t (*writeHeader)(FILE *f, uint32 offset, uint32 size);
	size_t (*writeData)(FILE *f);
	uint32 offset;
	uint32 size;
};

#define DEFINE_GAME_PLATFORM_LANG_FUNCS(game, platform, lang) \
    size_t write ## game ## _ ## platform ## _ ## lang ## _Header(FILE *f, \
                                   uint32 offset, uint32 size); \
    size_t write ## game ## _ ## platform ## _ ## lang ## _Data(FILE *f);
#define GAME_PLATFORM_LANG_PART(game, platform, lang) { write ## game ## _ ## platform ## _ ## lang ## _Header, \
    write ## game ## _ ## platform ## _ ## lang ## _Data, 0, 0 }

DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, FR)
DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, BR)
DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, DE)
DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, EN)
DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, ES)
DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, IT)
DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, JA)
DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, KO)
DEFINE_GAME_PLATFORM_LANG_FUNCS(Versailles, ALL, ZT)

static Parts gamesParts[] = {
	GAME_PLATFORM_LANG_PART(Versailles, ALL, FR),
	GAME_PLATFORM_LANG_PART(Versailles, ALL, BR),
	GAME_PLATFORM_LANG_PART(Versailles, ALL, DE),
	GAME_PLATFORM_LANG_PART(Versailles, ALL, EN),
	GAME_PLATFORM_LANG_PART(Versailles, ALL, ES),
	GAME_PLATFORM_LANG_PART(Versailles, ALL, IT),
	GAME_PLATFORM_LANG_PART(Versailles, ALL, JA),
	GAME_PLATFORM_LANG_PART(Versailles, ALL, KO),
	GAME_PLATFORM_LANG_PART(Versailles, ALL, ZT),
};

#define CRYOMNI3D_DAT_VER 1 // 32-bit integer

size_t writeFileHeader(FILE *f, uint16 games) {
	size_t headerSize = 0;
	fwrite("CY3DDATA", 8, 1, f);
	headerSize += 8;
	headerSize += writeUint16LE(f, CRYOMNI3D_DAT_VER);
	headerSize += writeUint16LE(f, games);
	// Dummy value to pad to 16 bytes
	headerSize += writeUint32LE(f, 0);
	assert((headerSize & PADDING_MASK) == 0);
	return headerSize;
}

size_t writeGameHeader(FILE *f, uint32 gameId, uint16 version, uint16 lang, uint32 platforms,
                       uint32 offset, uint32 size) {
	size_t headerSize = 0;
	headerSize += writeUint32BE(f, gameId); // BE to keep the tag readable
	headerSize += writeUint16LE(f, version);
	headerSize += writeUint16BE(f, lang); // BE to keep the tag readable
	headerSize += writeUint32LE(f, platforms);
	headerSize += writeUint32LE(f, offset);
	headerSize += writeUint32LE(f, size);
	return headerSize;
}

static int emitData(char *outputFilename) {
	FILE *f = fopen(outputFilename, "w+b");
	if (!f) {
		printf("ERROR: Unable to create output file %s\n", outputFilename);
		return 1;
	}

	printf("Generating %s...\n", outputFilename);

	writeFileHeader(f);

	for (unsigned int i = 0; i < ARRAYSIZE(gamesParts); i++) {
		gamesParts[i].writeHeader(f, 0xdeadfeed, 0xdeadfeed);
	}

	// Pad the games list
	writePadding(f);

	for (unsigned int i = 0; i < ARRAYSIZE(gamesParts); i++) {
		gamesParts[i].offset = ftell(f);
		gamesParts[i].size = gamesParts[i].writeData(f);
	}

	fseek(f, 0, SEEK_SET);

	writeFileHeader(f, ARRAYSIZE(gamesParts));

	for (unsigned int i = 0; i < ARRAYSIZE(gamesParts); i++) {
		gamesParts[i].writeHeader(f, gamesParts[i].offset, gamesParts[i].size);
	}

	fclose(f);

	printf("Done!\n");

	return 0;
}

int main(int argc, char **argv) {

	if (argc > 1) {
		return emitData(argv[1]);
	} else {
		printf("Usage: %s <output.dat>\n", argv[0]);
	}

	return 0;
}
