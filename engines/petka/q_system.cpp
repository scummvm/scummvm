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

#include "common/ini-file.h"
#include "common/substream.h"

#include "petka/petka.h"
#include "petka/q_object_case.h"
#include "petka/q_object_cursor.h"
#include "petka/q_system.h"

namespace Petka {

QSystem::QSystem(PetkaEngine &vm)
	: _vm(vm), _cursor(nullptr), _case(nullptr) {}

QSystem::~QSystem() {

}

Common::String readString(Common::ReadStream &readStream) {
	uint32 stringSize = readStream.readUint32LE();
	byte *data = (byte *)malloc(stringSize + 1);
	readStream.read(data, stringSize);
	data[stringSize] = '\0';
	Common::String str((char *)data);
	return str;
}

bool QSystem::init() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm.openFile("script.dat", true));
	if (!stream)
		return false;
	Common::ScopedPtr<Common::SeekableReadStream> namesStream(_vm.openFile("Names.ini", false));
	Common::ScopedPtr<Common::SeekableReadStream> castStream(_vm.openFile("Cast.ini", false));
	Common::ScopedPtr<Common::SeekableReadStream> bgsStream(_vm.openFile("BGs.ini", false));

	Common::INIFile namesIni;
	Common::INIFile castIni;
	Common::INIFile bgsIni;

	// fails because ini is broken for Russian letters
	namesIni.loadFromStream(*namesStream);
	castIni.loadFromStream(*castStream);
	bgsIni.loadFromStream(*bgsStream);

	uint32 objsCount = stream->readUint32LE();
	uint32 bgsCount = stream->readUint32LE();

	_objs.resize(objsCount);
	_bgs.resize(bgsCount);

	Common::String name;
	for (uint i = 0; i < objsCount; ++i) {
		_objs[i].deserialize(*stream, namesIni, castIni);
		_allObjects.push_back(&_objs[i]);
	}
	for (uint i = 0; i < bgsCount; ++i) {
		_bgs[i].deserialize(*stream, namesIni, castIni);
		_allObjects.push_back(&_bgs[i]);
	}

	addMessageForAllObjects(kTotalInit);

	_cursor.reset(new QObjectCursor());
	_case.reset(new QObjectCase());
	_allObjects.push_back(_cursor.get());
	_allObjects.push_back(_case.get());
	return true;
}

void QSystem::addMessage(const QMessage &msg) {
	_messages.push_back(msg);
}

void QSystem::addMessage(uint16 objId, uint16 opcode, int16 arg1, int16 arg2, int16 arg3, int16 unk1, int16 unk2) {
	_messages.push_back({objId, opcode, arg1, arg2, arg3, unk1, unk2});
}

void QSystem::addMessageForAllObjects(uint16 opcode, int16 arg1, int16 arg2, int16 arg3, int16 unk1, int16 unk2) {
	for (uint i = 0; i < _allObjects.size(); ++i) {
		_messages.push_back({_allObjects[i]->getId(), opcode, arg1, arg2, arg3, unk1, unk2});
	}
}

}
