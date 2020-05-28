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
#include "common/system.h"

#include "graphics/colormasks.h"

#include "petka/petka.h"
#include "petka/interfaces/startup.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/save_load.h"
#include "petka/interfaces/sequence.h"
#include "petka/interfaces/panel.h"
#include "petka/interfaces/map.h"
#include "petka/objects/object_cursor.h"
#include "petka/objects/object_case.h"
#include "petka/objects/object_star.h"
#include "petka/objects/heroes.h"
#include "petka/big_dialogue.h"
#include "petka/q_system.h"
#include "petka/video.h"

namespace Petka {

QSystem::QSystem()
	: _cursor(nullptr), _case(nullptr), _star(nullptr), _petka(nullptr), _chapayev(nullptr),
	_mainInterface(nullptr), _currInterface(nullptr), _prevInterface(nullptr), _isIniting(0),
	_sceneWidth(640) {}

QSystem::~QSystem() {

}

bool QSystem::init() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(g_vm->openFile("script.dat", true));
	if (!stream)
		return false;
	_isIniting = 1;
	Common::ScopedPtr<Common::SeekableReadStream> namesStream(g_vm->openFile("Names.ini", true));
	Common::ScopedPtr<Common::SeekableReadStream> castStream(g_vm->openFile("Cast.ini", true));
	Common::ScopedPtr<Common::SeekableReadStream> bgsStream(g_vm->openFile("BGs.ini", true));

	Common::INIFile namesIni;
	Common::INIFile castIni;
	Common::INIFile bgsIni;

	namesIni.allowNonEnglishCharacters();
	castIni.allowNonEnglishCharacters();
	bgsIni.allowNonEnglishCharacters();

	if (namesStream)
		namesIni.loadFromStream(*namesStream);
	if (castStream)
		castIni.loadFromStream(*castStream);
	if (bgsStream)
		bgsIni.loadFromStream(*bgsStream);

	uint32 objsCount = stream->readUint32LE() - 2;
	uint32 bgsCount = stream->readUint32LE();

	_objs.resize(objsCount);
	_bgs.resize(bgsCount);

	_petka.reset(new QObjectPetka());
	_petka->readScriptData(*stream);
	_petka->readInisData(namesIni, castIni, nullptr);
	_allObjects.push_back(_petka.get());

	_chapayev.reset(new QObjectChapayev());
	_chapayev->readScriptData(*stream);
	_chapayev->readInisData(namesIni, castIni, nullptr);
	_allObjects.push_back(_chapayev.get());

	for (uint i = 0; i < objsCount; ++i) {
		_objs[i].readScriptData(*stream);
		_objs[i].readInisData(namesIni, castIni, nullptr);
		_allObjects.push_back(&_objs[i]);
	}
	for (uint i = 0; i < bgsCount; ++i) {
		_bgs[i].readScriptData(*stream);
		_bgs[i].readInisData(namesIni, castIni, &bgsIni);
		_allObjects.push_back(&_bgs[i]);
	}

	addMessageForAllObjects(kTotalInit);

	_cursor.reset(new QObjectCursor());
	_case.reset(new QObjectCase());
	_star.reset(new QObjectStar());

	_allObjects.push_back(_cursor.get());
	_allObjects.push_back(_case.get());
	_allObjects.push_back(_star.get());

	_mainInterface.reset(new InterfaceMain());
	_startupInterface.reset(new InterfaceStartup());
	_saveLoadInterface.reset(new InterfaceSaveLoad());
	_sequenceInterface.reset(new InterfaceSequence());
	_panelInterface.reset(new InterfacePanel());
	_mapInterface.reset(new InterfaceMap());
	if (g_vm->getPart() == 0) {
		_prevInterface = _currInterface = _startupInterface.get();
	} else {
		_prevInterface = _currInterface = _mainInterface.get();
	}
	_currInterface->start(0);
	return true;
}

void QSystem::addMessage(const QMessage &msg) {
	_messages.push_back(msg);
}

void QSystem::addMessage(uint16 objId, uint16 opcode, int16 arg1, int16 arg2, int16 arg3, int32 unk, QMessageObject *sender) {
	_messages.push_back(QMessage(objId, opcode, arg1, arg2, arg3, sender, unk));
}

void QSystem::addMessageForAllObjects(uint16 opcode, int16 arg1, int16 arg2, int16 arg3, int32 unk, QMessageObject *sender) {
	for (uint i = 0; i < _allObjects.size(); ++i) {
		_messages.push_back(QMessage(_allObjects[i]->_id, opcode, arg1, arg2, arg3, sender, unk));
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
		QMessageObject *obj = findObject(it->objId);
		if (obj && !obj->_holdMessages) {
			obj->processMessage(*it);
			it = _messages.erase(it);
		} else {
			++it;
		}
	}
}

void QSystem::togglePanelInterface() {
	if (_currInterface != _startupInterface.get() && _star->_isActive) {
		_case->show(0);
		if (_currInterface == _panelInterface.get()) {
			_currInterface->stop();
		} else if (_currInterface == _mainInterface.get()) {
			_panelInterface->start(0);
		}
	}
}

void QSystem::toggleMapInterface() {
	if (_currInterface != _startupInterface.get() && _star->_isActive && _room->_showMap) {
		_case->show(0);
		if (_currInterface == _mapInterface.get()) {
			_currInterface->stop();
		} else if (_currInterface == _mainInterface.get()) {
			_currInterface->setText(Common::U32String(""), 0, 0);
			_mapInterface->start(0);
		}
	}
}

void QSystem::setChapayev() {
	if (_star->_isActive && _currInterface == _mainInterface.get() && _chapayev->_isShown) {
		_cursor->setAction(kActionObjUseChapayev);
	}
}

static Common::String readString(Common::ReadStream *s) {
	Common::String string;
	uint32 len = s->readUint32LE();
	char *buffer = (char *)malloc(len);
	s->read(buffer, len);
	string = Common::String(buffer, len);
	free(buffer);
	return string;
}

static void writeString(Common::WriteStream *s, const Common::String &string) {
	s->writeUint32LE(string.size());
	s->write(string.c_str(), string.size());
}

void QSystem::load(Common::ReadStream *s) {
	uint count = s->readUint32LE();
	for (uint i = 0; i < count; ++i) {
		QMessageObject *obj = findObject(readString(s));
		obj->_holdMessages = s->readUint32LE();
		obj->_status = s->readUint32LE();
		obj->_resourceId = s->readUint32LE();
		obj->_z = s->readUint32LE();
		obj->_x = s->readUint32LE();
		obj->_y = s->readUint32LE();
		obj->_isShown = s->readUint32LE();
		obj->_isActive = s->readUint32LE();
		obj->_animate = s->readUint32LE();
	}

	uint itemSize = s->readUint32LE();
	_case->_items.clear();
	for (uint i = 0; i < itemSize; ++i) {
		_case->_items.push_back(s->readSint32LE());
	}

	_room = (QObjectBG *)findObject(readString(s));
	if (_room) {
		_mainInterface->loadRoom(_room->_id, true);
	}

	g_vm->getBigDialogue()->load(s);

	_cursor->_resourceId = s->readUint32LE();
	_cursor->_actionType = s->readUint32LE();
	int invObjId = s->readSint32LE();
	if (invObjId != -1) {
		_cursor->_invObj = findObject(invObjId);
	} else {
		_cursor->_invObj = nullptr;
	}

	g_vm->videoSystem()->makeAllDirty();
}

void QSystem::save(Common::WriteStream *s) {
	s->writeUint32LE(_allObjects.size() - 3);
	for (uint i = 0; i < _allObjects.size() - 3; ++i) {
		writeString(s, _allObjects[i]->_name);
		s->writeUint32LE(_allObjects[i]->_holdMessages);
		s->writeUint32LE(_allObjects[i]->_status);
		s->writeUint32LE(_allObjects[i]->_resourceId);
		s->writeUint32LE(_allObjects[i]->_z);
		s->writeUint32LE(_allObjects[i]->_x);
		s->writeUint32LE(_allObjects[i]->_y);
		s->writeUint32LE(_allObjects[i]->_isShown);
		s->writeUint32LE(_allObjects[i]->_isActive);
		s->writeUint32LE(_allObjects[i]->_animate);
	}

	s->writeUint32LE(_case->_items.size());
	for (uint i = 0; i < _case->_items.size(); ++i) {
		s->writeSint32LE(_case->_items[i]);
	}

	writeString(s, _room->_name);

	// heroes (no impl)

	g_vm->getBigDialogue()->save(s);

	s->writeUint32LE(_cursor->_resourceId);
	s->writeUint32LE(_cursor->_actionType);
	if (_cursor->_invObj) {
		s->writeSint32LE(_cursor->_invObj->_resourceId);
	} else {
		s->writeSint32LE(-1);
	}
}

}
