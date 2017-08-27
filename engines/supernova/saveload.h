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

#ifndef SAVELOAD_H
#define SAVELOAD_H

namespace Supernova {

const int kNumRoom0 =  1;
const int kNumRoom1 = 16;
const int kNumRoom2 =  9;
const int kNumRoom3 = 21;

struct OriginalObject {
	char *_name;
	char *_description;
	byte  _id;
	int16 _type;
	char  _click;
	char  _click2;
	char  _section;
	char  _exitroom;
	char  _direction;
};

struct OriginalRoom {
	char *_file;
	byte _shown[40];
	uint16 _funcptr[3];
	OriginalObject _object[25];
};

struct OriginalGameSaveState {
	uint16 _ptrCurrentRoom;
	uint16 _inventory[30];
	int16  _inventorySize;
	int16  _inventoryScroll;
	OriginalRoom _roomlist0[kNumRoom0];
	OriginalRoom _roomlist1[kNumRoom1];
	OriginalRoom _roomlist2[kNumRoom2];
	OriginalRoom _roomlist3[kNumRoom3];
};

// TODO: Create a table with mapping of offsets to corresponding strings in
//       object definitions to keep save files valid. (msn.exe:0xD400..)

}

#endif // SAVELOAD_H
