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

#ifndef IMMORTAL_STORY_H
#define IMMORTAL_STORY_H

namespace Immortal {

enum ObjFlag : uint8 {
	kObjUsesFireButton = 0x40,
	kObjIsInvisible    = 0x20,
	kObjIsRunning      = 0x10,
	kObjIsChest        = 0x08,
	kObjIsOnGround     = 0x04,
	kObjIsF1           = 0x02,
	kObjIsF2           = 0x01
};

enum Str {
	kStrOldGame,
	kStrEnterCertificate,
	kStrBadCertificate,
	kStrCertificate,
	kStrCertificate2,
	kStrTitle0,
	kStrTitle4,
	kStrGold,
	kStrYouWin,
	kStrGameOver
};

struct Pickup {
	//pointer to function
	int _param;
};

struct Use {
	//pointer to function
	int _param;
};

struct ObjType {
	Str _str;
	Str _desc;
	int _size;
	Pickup _pickup;
	Use _use;
	Use _run;
};

} // namespace immortal

#endif