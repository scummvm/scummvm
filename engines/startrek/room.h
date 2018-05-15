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

#ifndef STARTREK_ROOM_H
#define STARTREK_ROOM_H

#include "common/rect.h"
#include "common/ptr.h"
#include "common/str.h"

using Common::SharedPtr;


namespace StarTrek {

class StarTrekEngine;

// Offsets of data in RDF files

const int RDF_WARP_ROOM_INDICES = 0x22;
const int RDF_ROOM_ENTRY_POSITIONS = 0x2a;
const int RDF_BEAM_IN_POSITIONS = 0xaa;

class Room {

public:
	Room(StarTrekEngine *vm, Common::String name);
	~Room();

	// Helper stuff for RDF access
	uint16 readRdfWord(int offset);

	// Scale-related stuff (rename these later)
	int16 getVar06() { return readRdfWord(0x06); }
	int16 getVar08() { return readRdfWord(0x08); }
	int16 getVar0a() { return readRdfWord(0x0a); }
	int16 getVar0c() { return readRdfWord(0x0c); }

	// Warp-related stuff
	int16 getFirstWarpPolygonOffset() { return readRdfWord(0x16); }
	int16 getWarpPolygonEndOffset()   { return readRdfWord(0x18); }
	int16 getFirstDoorPolygonOffset() { return readRdfWord(0x1a); }
	int16 getDoorPolygonEndOffset()   { return readRdfWord(0x1c); }

	Common::Point getBeamInPosition(int crewmanIndex);

	byte *_rdfData;

private:
	StarTrekEngine *_vm;
};

}

#endif
