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
 */

#ifndef STARTREK_ROOM_H
#define STARTREK_ROOM_H

#include "common/ptr.h"
#include "common/str.h"

using Common::SharedPtr;


namespace StarTrek {

class StarTrekEngine;

class Room {

public:
	Room(StarTrekEngine *vm, Common::String name);
	~Room();

	uint16 readRdfWord(int offset);

	byte *_rdfData;

private:
	StarTrekEngine *_vm;
};

}

#endif
