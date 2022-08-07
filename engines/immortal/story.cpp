/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* [Alternate Name: Level Subsystem/Script Data]
 * --- Story File ---
 * A story file (as defined in story.h) is a set of ROM
 * data that describes the properties of a level. This includes
 * the coordinates for each room, the doors in the level,
 * the torches, objects, monsters, etc. It also included the string
 * data in the source code, but technically it was writing those
 * strings to a separate string bank, so they weren't contiguous
 * with the story files. These story files are read in when loading
 * a new level, and are used to construct the room object, the monster
 * objects, and everything in the rooms.
 */

#include "immortal/immortal.h"

namespace Immortal {

void ImmortalEngine::loadStoryFiles() {
	/* The way I am doing this, there will be essentially duplicate data
	 * for the in-room objects. This is because the original also
	 * effectively duplicated data. There was less overhead of course,
	 * as these structs are considered classes by c++. However I think
	 * that logically speaking, this is what the original was doing.
	 * It's not ideal to do it this way, but my guess for why the source
	 * didn't just read directly (ex. door object data will never change,
	 * it is ROM) is that the story bank was too far from the general work
	 * memory used for the object data. Could be something else though.
	 */

	// Level 0: Intro 1

	_stories[0]._levelNum = 0;		// These aren't really needed anymore
	_stories[0]._partNum = 1;
	
	int univRoom = 4;               // The room the player starts in when beginning this level
	uint8 univRoomX = 512;
	uint8 univRoomY = 416;
	int byteArray[] = {-1, -1, kStoryNull, 2, 0, univRoom, (704 / 64),(544 / 32)};
	_stories[0]._UnivAt = UnivAt(1024 / 8, 480 / 8, (1152 - univRoomX) / 2, 464 - univRoomY, byteArray);

	// All of the rooms for level 0
	SRoom rooms[8] = {SRoom(384, 256, kRoomFlag0), SRoom(512, 64, kRoomFlag0),  SRoom(640, 160, kRoomFlag0),  SRoom(768, 224, kRoomFlag0),
					  SRoom(univRoomX, univRoomY, kRoomFlag0), SRoom(960, 512, kRoomFlag0), SRoom(1024, 352, kRoomFlag0), SRoom(896, 64, kRoomFlag0)};
	_stories[0]._rooms = rooms;

	// All of the doors for level 0
	SDoor doors[7] = {SDoor(kLeft, 704, 224, 0, 2, false), SDoor(kRight, 576, 352, 4, 0, true),
					  SDoor(kRight, 704,96, 2, 1, false), SDoor(kRight, 960,128, 7, 2, false),
					  SDoor(kRight, 1088,160, 3, 7, false), SDoor(kRight, 1088,320, 6, 3, false),
					  SDoor(kRight, 896,416, 4, 3, false)};
	_stories[0]._doors = doors;

	// All of the flames for level 0
	// Macro for flames is (x - roomx), (y - roomy), pattern number
	SFlame f5[2] = {SFlame(512 - 384, (240 + 32) - 256, kFlameOff), SFlame(672 - 384, (240 + 32) - 256, kFlameOff)};
	SFlame f7[3] = {SFlame(576 - 384, (112 + 32) - 256, kFlameNormal), SFlame(576 - 384, (112 + 32) - 256, kFlameNormal), SFlame(928 - 384, (48 + 32) - 256, kFlameNormal)};
	SFlame f8[1] = {SFlame(800 - 640, (144 + 32) - 160, kFlameNormal)};
	SFlame f9[3] = {SFlame(768 - 768, (304 + 32) - 224, kFlameNormal), SFlame((928 - 768), (304 + 32) - 224, kFlameNormal), SFlame((1024 - 768), (240 + 32) - 224, kFlameNormal)};
	SFlame fA[3] = {SFlame(672 - 512, (400 + 32) - 416, kFlameNormal), SFlame((800 - 64) - 512, (496 - 32) - 416, kFlameNormal), SFlame(576 - 512, (528 + 32) - 416, kFlameNormal)};
	SFlame fD[1] = {SFlame(1024 - 960, (496 + 32) - 512, kFlameNormal)};
	SFlame fE[1] = {SFlame(1184 - 1024, 432 - 352, kFlameCandle)};
	SFlame fF[1] = {SFlame(1024 - 896, (144 + 32) - 64, kFlameNormal)};
	SFlame *flames[8] = {f5, f7, f8, f9, fA, fD, fE, fF};
	_stories[0]._flames = flames;

	// All of the objects for level 0
	SObj o5[3];
	SObj o7[1];
	SObj o8[1];
	SObj o9[9];
	SObj oE[4];
	SObj *objects[8] = {o5, o7, o8, o9, nullptr, nullptr, oE, nullptr};
	_stories[0]._objects = objects;

	// All of the monsters for level 0
	SMonster m5[2];
	SMonster m9[3];
	SMonster mE[1];
	SMonster mF[1];
	SMonster *monsters[8] = {m5, nullptr, nullptr, m9, nullptr, nullptr, mE, mF};
	_stories[0]._monsters = monsters;
}

} // namespace Immortal
































