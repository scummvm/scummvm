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

#include "common/array.h"
#include "common/str.h"

#ifndef NANCY_SCRIPT_H
#define NANCY_SCRIPT_H

namespace Common {
struct IFFChunk;
}

namespace Nancy {

class NancyEngine;

#define ID_DATA     MKTAG('D', 'A', 'T', 'A')
#define ID_PCAL     MKTAG('P', 'C', 'A', 'L')

class IFF {
public:
	IFF(NancyEngine *vm, const Common::String &name) : _name(name), _vm(vm) { };
	~IFF();

	bool load();
	const byte *getChunk(uint32 id, uint &size) const;

private:
	Common::String idToString(uint32 id);
	bool callback(Common::IFFChunk &chunk);

	struct Chunk {
		uint32 id;
		byte *buf;
		uint32 size;
	};

	Common::Array<Chunk> _chunks;
	const Common::String _name;
	NancyEngine *_vm;
};

} // End of namespace Nancy

#endif
