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

#ifndef GARGOYLE_SCOTT
#define GARGOYLE_SCOTT

/*
 *	Controlling block
 */

#include "common/scummsys.h"
#include "gargoyle/glk/glk.h"

namespace Gargoyle {
namespace Scott {

#define LIGHT_SOURCE	9	// Always 9 how odd
#define CARRIED		255		// Carried
#define DESTROYED	0		// Destroyed
#define DARKBIT		15
#define LIGHTOUTBIT	16		// Light gone out

#define YOUARE		1		// You are not I am
#define SCOTTLIGHT	2		// Authentic Scott Adams light messages
#define DEBUGGING	4		// Info from database load
#define TRS80_STYLE	8		// Display in style used on TRS-80
#define PREHISTORIC_LAMP 16	// Destroy the lamp (very old databases)

struct Header {
 	int Unknown;
	int NumItems;
	int NumActions;
	int NumWords;			// Smaller of verb/noun is padded to same size
	int NumRooms;
	int MaxCarry;
	int PlayerRoom;
	int Treasures;
	int WordLength;
	int LightTime;
	int NumMessages;
	int TreasureRoom;
};

struct Action {
	uint Vocab;
	uint Condition[5];
	uint action[2];
};

struct Room {
	char *Text;
	short Exits[6];
};

struct Item {
	char *Text;		// PORTABILITY WARNING: THESE TWO MUST BE 8 BIT VALUES.
	byte Location;
	byte InitialLoc;
	char *AutoGet;
};

struct Tail {
	int Version;
	int AdventureNumber;
	int Unknown;
};

/**
 * Scott Adams game interpreter
 */
class Scott : public Glk {
public:
	/**
	 * Constructor
	 */
	Scott() : Glk() {}

	/**
	 * Execute the game
	 */
	virtual void execute();
};

} // End of namespace Scott
} // End of namespace Gargoyle

#endif
