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
#include "agds/agds.h"
#include "common/debug.h"

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
	debug("resource block %04x %04x %04x %04x,"
		" resources table with %u entries", unk1, unk2, unk3, unk4, resCount);

	_object->readStringTable(resOffset, resCount);
}

void Process::setSystemVariable() {
	int16 valueIndex = pop();
	const Object::StringEntry &name = popString();

	if (valueIndex != -1) {
		const Object::StringEntry &value = _object->getString(valueIndex);
		debug("setSystemVariable %s to %s", name.string.c_str(), value.string.c_str());
	} else {
		debug("resetSystemVariable %s", name.string.c_str());
	}
}

void Process::loadPicture() {
	const Object::StringEntry &name = popString();
	debug("loadPicture stub %s", name.string.c_str());
	push(100500); //dummy
}

void Process::appendToSharedStorage() {
	const Object::StringEntry &value = popString();
	int index = _engine->appendToSharedStorage(value.string);
	debug("appendToSharedStorage %s -> %d", value.string.c_str(), index);
	push(index);
}


}
