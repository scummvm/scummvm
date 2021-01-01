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

#include "file.h"
#include "map.h"

#define MAP_WIDTH 16
#define MAP_HEIGHT 16
#define FLAG_IS_OUTDOORS 32768

#define MIRROR_COUNT 1
static const MirrorEntry MIRROR_TEXT[MIRROR_COUNT] = {
	{ "scummvm", 255, 7, 1, 0 }
};

static const byte MAZE_255[MAP_HEIGHT][MAP_WIDTH] = {
	{ 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 },
	{ 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9 },
	{ 9, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 9 },
	{ 9, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 4,14,14,14,14,14,14, 4, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 4,14, 6, 6, 6, 6,14, 4, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 4,14, 6, 7, 7, 6,14, 4, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 4,14, 6, 7, 7, 6,14, 4, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 4,14, 6, 6, 6, 6,14, 4, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 4,14,14,14,14,14,14, 4, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 9 },
	{ 9, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 9 },
	{ 9, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 9 },
	{ 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9 },
	{ 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9 }
};

static const byte WALL_TYPES_255[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const byte SURFACE_TYPES_255[16] = { 1, 1, 2, 3, 4, 0, 6, 7, 0, 9, 0, 0, 0, 0, 14, 15 };

/**
 * Write out new mirror entries
 */
static void writeMirrorText(CCArchive &cc) {
	Common::MemFile f;

	for (int idx = 0; idx < MIRROR_COUNT; ++idx) {
		const MirrorEntry &me = MIRROR_TEXT[idx];
		f.write(me._name, 28);
		f.writeByte(me._mapId);
		f.writeShort(me._posX);
		f.writeShort(me._posY);
		f.writeByte(me._direction);
	}

	cc.add("xeenmirr.ext", f);
}

/**
 * Write out the maze
 */
static void writeMaze(CCArchive &cc) {
	Common::MemFile f;

	// Wall data
	for (int y = 0; y < MAP_HEIGHT; ++y)
		for (int x = 0; x < MAP_WIDTH; ++x)
			f.writeWord(MAZE_255[y][x]);

	// Surface and flags
	for (int y = 0; y < MAP_HEIGHT; ++y)
		f.write(MAZE_255[y], MAP_WIDTH);

	f.writeWord(255);				// Maze number
	for (int idx = 0; idx < 4; ++idx)
		f.writeWord(0);				// No surrounding mazes
	f.writeWord(0);					// Maze flags 1
	f.writeWord(FLAG_IS_OUTDOORS);	// Maze flags 2
	f.write(WALL_TYPES_255, 16);
	f.write(SURFACE_TYPES_255, 16);
	f.writeByte(0);					// Floor type (unused)
	f.writeByte(7);					// Run position X
	f.writeByte(0, 8);				// Difficulties
	f.writeByte(0);					// Run position Y
	f.writeByte(0);					// Trap damage
	f.writeByte(0);					// Wall kind
	f.writeByte(0);					// Tavern tips
	f.writeByte(0, MAP_WIDTH * MAP_HEIGHT / 8);	// Seen tiles
	f.writeByte(0, MAP_WIDTH * MAP_HEIGHT / 8);	// Stepped on tiles

	cc.add("mazex255.dat", f);
}

/**
 * Write out the maze name
 */
static void writeMazeName(CCArchive &cc) {
	Common::MemFile f;
	char mazeName[33];
	memset(mazeName, 0, 33);
	strcpy(mazeName, "ScummVM");
	f.write(mazeName, 33);

	cc.add("xeenx255.txt", f);
}

/**
 * Write out maze events
 */
static void writeMazeEvents(CCArchive &cc) {
	Common::MemFile f;

	// Mirror events
	static const byte MIRROR_EVENTS[32] = {
		6,	7, 0, 2, 0,	40,		1,				// Play VOC: "Where to?"
		9,	7, 0, 2, 1, 21,		0, 3, 0, 0,		// Get destination
		5,	7, 0, 2, 2, 18,						// Exit
		8,	7, 0, 2, 3, 7,		0, 0, 0			// Teleport and exit
	};
	f.write(MIRROR_EVENTS, 32);

	// Bench 1 events
	static const byte BENCH1_EVENTS[32] = {
		10,  7, 8, 0, 0,  5,	1, 2, 3, 1, 2,	// NPC
		14,	7, 8, 0, 1, 20,		34, 10000 % 256, 10000 / 256, 0, 0, 0, 0, 0, 0, // Give gold
		5,	7, 8, 0, 2, 18						// Exit
	};
	static const byte BENCH2_EVENTS[30] = {
		10,  8, 8, 0, 0,  5,	1, 3, 3, 1, 2,	// NPC
		14,	8, 8, 0, 1, 20,		35, 1000 % 256, 1000 / 256, 0, 0, 0, 0,	// Give gems
		5,	8, 8, 0, 2, 18						// Exit
	};
	f.write(BENCH1_EVENTS, 32);
	f.write(BENCH2_EVENTS, 30);

	cc.add("mazex255.evt", f);
}

/**
 * Write out maze event text
 */
static void writeMazeText(CCArchive &cc) {
	Common::MemFile f;

	f.writeString("Where to?");
	f.writeString("Isle of ScummVM");
	f.writeString("You have done well to find this ancient isle. This will aid you on your journey.");
	f.writeString("It is my hope that this isle will be but the first of many such new destinations the mirror may take you.");

	cc.add("aazex255.txt", f);
}

/**
 * Write out the monster/object data
 */
static void writeMonstersObjects(CCArchive &cc) {
	Common::MemFile f;
	f.writeByte(8);			// Object sprites
	f.writeByte(2);
	f.writeByte(0xff, 14);

	f.writeByte(0xff, 16);	// Monster sprites
	f.writeByte(0xff, 16);	// Wall item sprites

	for (int idx = 0; idx < 6; ++idx) {
		switch (idx) {
		case 0:
			// Mirror
			f.writeShort(7);
			f.writeShort(0);
			f.writeByte(0);
			f.writeShort(2);
			// Benches
			f.writeShort(7);
			f.writeShort(8);
			f.writeShort(1);
			f.writeShort(0);
			f.writeShort(8);
			f.writeShort(8);
			f.writeShort(1);
			f.writeShort(0);
			break;
		case 2:
			// End of monster/objects
			f.writeShort(0);
			f.writeShort(0);
			f.writeByte(0);
			f.writeShort(0);
			break;
		case 4:
			f.writeByte(0x80);
			f.writeByte(0x80);
			f.writeByte(0);
			f.writeShort(0);
			break;
		default:
			f.writeShort(-1);
			f.writeShort(-1);
			f.writeByte(0xff);
			f.writeShort(-1);
			break;
		}
	}

	cc.add("mazex255.mob", f);
}

/**
 * Write out the data for the head danger senses
 */
static void writeHeadData(CCArchive &cc) {
	Common::MemFile f;
	f.writeByte(0, MAP_HEIGHT * MAP_HEIGHT * 2);
	cc.add("aazex255.hed", f);
}

/**
 * Write out the new ScummVM map
 */
void writeMap(CCArchive &cc) {
	writeMirrorText(cc);
	writeMaze(cc);
	writeMazeName(cc);
	writeMazeEvents(cc);
	writeMazeText(cc);
	writeMonstersObjects(cc);
	writeHeadData(cc);
}
