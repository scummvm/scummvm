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

#include "graphics/colormasks.h"

#include "petka/petka.h"
#include "petka/q_interface.h"
#include "petka/q_system.h"

namespace Petka {

QSystem::QSystem()
	: _cursor(nullptr), _case(nullptr), _star(nullptr), _mainInterface(nullptr),
	_currInterface(nullptr), _prevInterface(nullptr), _field48(0) {}

QSystem::~QSystem() {

}

static Common::String readString(Common::ReadStream &readStream) {
	uint32 stringSize = readStream.readUint32LE();
	byte *data = (byte *)malloc(stringSize + 1);
	readStream.read(data, stringSize);
	data[stringSize] = '\0';
	Common::String str((char *)data);
	return str;
}

static void readObject(QMessageObject &obj, Common::SeekableReadStream &stream,
	const Common::INIFile &namesIni, const Common::INIFile &castIni) {
	obj._id = stream.readUint16LE();
	obj._name = readString(stream);
	obj._reactions.resize(stream.readUint32LE());

	for (uint i = 0; i < obj._reactions.size(); ++i) {
		QReaction *reaction = &obj._reactions[i];
		reaction->opcode = stream.readUint16LE();
		reaction->status = stream.readByte();
		reaction->senderId = stream.readUint16LE();
		reaction->messages.resize(stream.readUint32LE());
		for (uint j = 0; j < reaction->messages.size(); ++j) {
			QMessage *msg = &reaction->messages[j];
			msg->objId = stream.readUint16LE();
			msg->opcode = stream.readUint16LE();
			msg->arg1 = stream.readUint16LE();
			msg->arg2 = stream.readUint16LE();
			msg->arg3 = stream.readUint16LE();
		}
	}

	namesIni.getKey(obj._name, "all", obj._nameOnScreen);

	Common::String rgbString;
	if (castIni.getKey(obj._name, "all", rgbString)) {
		int r, g, b;
		sscanf(rgbString.c_str(), "%d %d %d", &r, &g, &b);
		obj._dialogColor = Graphics::RGBToColor<Graphics::ColorMasks<888>>((byte)r, (byte)g, (byte)b);
	}
}


bool QSystem::init() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(g_vm->openFile("script.dat", true));
	if (!stream)
		return false;
	_field48 = 1;
	Common::ScopedPtr<Common::SeekableReadStream> namesStream(g_vm->openFile("Names.ini", false));
	Common::ScopedPtr<Common::SeekableReadStream> castStream(g_vm->openFile("Cast.ini", false));
	Common::ScopedPtr<Common::SeekableReadStream> bgsStream(g_vm->openFile("BGs.ini", false));

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
		readObject(_objs[i], *stream, namesIni, castIni);
		_allObjects.push_back(&_objs[i]);
	}
	for (uint i = 0; i < bgsCount; ++i) {
		readObject(_bgs[i], *stream, namesIni, castIni);
		_allObjects.push_back(&_bgs[i]);
	}

	addMessageForAllObjects(kTotalInit);

	_cursor.reset(new QObjectCursor());
	_case.reset(new QObjectCase());
	_star.reset(new QObjectStar());

	_allObjects.push_back(_cursor.get());
	_allObjects.push_back(_case.get());
	_allObjects.push_back(_star.get());

	_mainInterface.reset(new QInterfaceMain());
	_startupInterface.reset(new QInterfaceStartup());
	_startupInterface->start();
	_currInterface = _startupInterface.get();
	return true;
}

void QSystem::addMessage(const QMessage &msg) {
	_messages.push_back(msg);
}

void QSystem::addMessage(uint16 objId, uint16 opcode, int16 arg1, int16 arg2, int16 arg3, int32 unk, QMessageObject *sender) {
	_messages.push_back({objId, opcode, arg1, arg2, arg3, sender, unk});
}

void QSystem::addMessageForAllObjects(uint16 opcode, int16 arg1, int16 arg2, int16 arg3, int32 unk, QMessageObject *sender) {
	for (uint i = 0; i < _allObjects.size(); ++i) {
		_messages.push_back({_allObjects[i]->_id, opcode, arg1, arg2, arg3, sender, unk});
	}
}

QMessageObject *QSystem::findObject(int16 id) {
	for (uint i = 0; i < _allObjects.size(); ++i) {
		if (_allObjects[i]->_id == id)
			return _allObjects[i];
	}
	return nullptr;
}

QMessageObject *QSystem::findObject(const Common::String &name) {
	for (uint i = 0; i < _allObjects.size(); ++i) {
		if (_allObjects[i]->_name == name)
			return _allObjects[i];
	}
	return nullptr;
}

void QSystem::update() {
	for (Common::List<QMessage>::iterator it = _messages.begin(); it != _messages.end();) {
		bool removeMsg = false;
		for (uint j = 0; j < _allObjects.size(); ++j) {
			if (it->objId == _allObjects[j]->_id) {
				_allObjects[j]->processMessage(*it);
				removeMsg = true;
				break;
			}
		}
		if (removeMsg) {
			it = _messages.erase(it);
		} else {
			++it;
		}
	}
}

}
