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

#include "agds/process.h"
#include "common/debug.h"
#include "common/memstream.h"

namespace AGDS {

void Process::enter(uint16 magic, uint16 size) {
	if (magic != 0xdead || size != 0x0c)
		error("invalid enter() magic: 0x%04x or size: %u", magic, size);
	uint16 unk1			= next16();
	uint16 unk2			= next16();
	uint16 unk3			= next16();
	unsigned resOffset	= next16();
	uint16 resCount		= next16();
	uint16 unk4			= next16();

	debug("resource block %04x %04x %04x %04x, resources table with %u entries", unk1, unk2, unk3, unk4, resCount);

	const Object::CodeType &code = _object->getCode();
	resOffset += 5 /*instruction*/ + 0x11 /*another header*/;
	if (resOffset >= code.size())
		error("invalid resource table offset");

	debug("resource table at %08x", resOffset);
	Common::MemoryReadStream stream(code.data() + resOffset, code.size() - resOffset);
	for(uint16 i = 0; i < resCount; ++i) {
		uint16 offset = stream.readUint16LE();
		uint16 flags = stream.readUint16LE();

		unsigned nameOffset = resOffset + offset;
		if (nameOffset > code.size())
			error("invalid resource name offset");

		const char * nameBegin = reinterpret_cast<const char *>(code.data() + nameOffset);
		const char * codeEnd = reinterpret_cast<const char *>(code.data() + code.size());
		const char * nameEnd = Common::find(nameBegin, codeEnd, 0);

		Common::String name(nameBegin, nameEnd - nameBegin);

		debug("resource table 1[%04u]: 0x%04x %s", i, flags, name.c_str());
	}
}

void Process::stub142() {
	int arg1 = pop();
	int arg2 = pop();
	debug("stub142: %d, %d", arg1, arg2);
}

void Process::loadPicture() {
	int32 id = pop();
	debug("loadPicture stub %d", id);
	push(100500); //dummy
}

}
