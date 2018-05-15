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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_OBJECT_H
#define STARTREK_OBJECT_H

#include "startrek/common.h"
#include "startrek/sprite.h"

#include "common/scummsys.h"

namespace StarTrek {

class StarTrekEngine;
class FileStream;


struct Object {
	uint16 spriteDrawn;
	char animationString3[16];
	uint16 animType;
	Sprite sprite;
	char animationString4[10];
	uint16 scale;
	SharedPtr<FileStream> animFile;
	uint16 numAnimFrames;
	uint16 animFrame;
	uint32 frameToStartNextAnim;
	Common::Point pos;
	uint16 field60;
	uint16 field62;
	uint16 walkingIntoRoom; // Walking or beaming into a room?
	uint16 field66;
	char animationString2[8];
	uint16 field70;
	uint16 field72;
	uint16 field74;
	uint16 field76;
	int16 iwSrcPosition;
	int16 iwDestPosition;

	// Fixed-point position values (16.16) used while walking.
	FixedInt granularPosX;
	FixedInt granularPosY;

	// Fixed-point speed values (16.16).
	FixedInt speedX;
	FixedInt speedY;

	Common::Point dest; // Position object is walking toward
	uint16 field90;
	byte field92;

	// Can 'n', 's', 'e', 'w', or 0 for uninitialized?
	// Can also be capitalized?
	char direction;

	uint16 field94;
	uint16 field96;
	char animationString[9];
	uint8  fielda1;
	uint16 fielda2;
	uint16 fielda4;
	uint16 fielda6;
};

}

#endif

