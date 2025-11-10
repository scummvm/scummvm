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
#ifndef PELROCK_TYPES_H
#define PELROCK_TYPES_H

namespace Pelrock {

enum Cursor {
	DEFAULT,
	HOTSPOT,
	EXIT,
	ANIMATION,
	COMBINATION
};

// enum HoverState {
// 	NONE,
// 	INTERACTIVE,
// 	HOSTPOT,
// 	SPECIAl;


// };

const int kCursorWidth = 16;
const int kCursorHeight = 18;
const int kCursorSize = 288;  // 16 * 18
const int kRoomStructSize = 104;
const int kNumRooms = 56;

struct Anim {
	int x;
	int y;
	int w;
	int h;
	int nframes;
	int curFrame;
	byte *animData;
};

struct Exit {
	uint16 x;
	uint16 y;
	byte w;
	byte h;
	uint16 targetRoom;
	uint16 targetX;
	uint16 targetY;
	uint16 targetDir;
	byte dir;
	byte flags;
};

struct AnimSet {
	int x;
	int y;
	int w;
	int h;
	int speed;
	int numAnims;
	Anim *animData;
};

struct HotSpot {
	int x;
	int y;
	int id;
	int w;
	int h;
	int extra;
};

struct WalkBox {
	int16 x;
	int16 y;
	int16 w;
	int16 h;
	byte flags;
};

enum GameState {
	GAME = 100,
	MENU = 101,
	CREDITS = 102,
	SAVELOAD = 103,
	SETTINGS = 104,
	EXTRA_SCREEN = 105,
	INTRO = 106,
	PROMOTE = 107,
};

// struct rectCam
// {
//     Common::List<int> vecinos;
//     bool marked;
//     int index;
//     int x;
//     int y;
//     int w;
//     int h;
// };

// struct defCam
// {
//     rectCam cams[];
// };

} // End of namespace Pelrock

#endif
