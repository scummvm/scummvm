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

#include "graphics/thumbnail.h"

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
#include "petka/q_manager.h"
#include "petka/flc.h"

namespace Petka {

QSystem::QSystem(PetkaEngine &vm)
	: _vm(vm), _mainInterface(nullptr), _currInterface(nullptr), _prevInterface(nullptr),
	_totalInit(false), _sceneWidth(640), _room(nullptr), _xOffset(0), _reqOffset(0) {}

QSystem::~QSystem() {
	for (uint i = 0; i < _allObjects.size(); ++i) {
		delete _allObjects[i];
	}
}

bool QSystem::init() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm.openFile("script.dat", true));
	if (!stream)
		return false;
	Common::ScopedPtr<Common::SeekableReadStream> namesStream(_vm.openFile("Names.ini", true));
	Common::ScopedPtr<Common::SeekableReadStream> castStream(_vm.openFile("Cast.ini", true));
	Common::ScopedPtr<Common::SeekableReadStream> bgsStream(_vm.openFile("BGs.ini", true));

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

	uint32 objsCount = stream->readUint32LE();
	uint32 bgsCount = stream->readUint32LE();
	_allObjects.reserve(objsCount + bgsCount + 3);
	for (uint i = 0; i < objsCount + bgsCount; ++i) {
		QMessageObject *obj = nullptr;
		if (i == 0) {
			obj = new QObjectPetka;
		} else if (i == 1) {
			obj = new QObjectChapayev;
		} else if (i < objsCount) {
			obj = new QObject;
		} else {
			obj = new QObjectBG;
		}
		obj->readScriptData(*stream);
		obj->readInisData(namesIni, castIni, &bgsIni);
		_allObjects.push_back(obj);
	}

	_allObjects.push_back(new QObjectCursor);
	_allObjects.push_back(new QObjectCase);
	_allObjects.push_back(new QObjectStar);

	_mainInterface.reset(new InterfaceMain());
	_startupInterface.reset(new InterfaceStartup());
	_saveLoadInterface.reset(new InterfaceSaveLoad());
	_sequenceInterface.reset(new InterfaceSequence());
	_panelInterface.reset(new InterfacePanel());
	_mapInterface.reset(new InterfaceMap());

	if (_vm.getPart() == 0) {
		_prevInterface = _currInterface = _startupInterface.get();
	} else {
		_prevInterface = _currInterface = _mainInterface.get();
	}

	_totalInit = true;

	addMessageForAllObjects(kTotalInit);
	update();

	_totalInit = false;

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
	if (_currInterface != _startupInterface.get() && getStar()->_isActive) {
		getCase()->show(0);
		if (_currInterface == _panelInterface.get()) {
			_currInterface->stop();
		} else if (_currInterface == _mainInterface.get()) {
			_panelInterface->start(0);
		}
	}
}

void QSystem::toggleMapInterface() {
	if (_currInterface != _startupInterface.get() && getStar()->_isActive && _room->_showMap) {
		getCase()->show(false);
		if (_currInterface == _mapInterface.get()) {
			_currInterface->stop();
		} else if (_currInterface == _mainInterface.get()) {
			_currInterface->setText(Common::U32String(), 0, 0);
			_mapInterface->start(0);
		}
	}
}

void QSystem::setCursorAction(int action) {
	if (getStar()->_isActive && _currInterface == _mainInterface.get()) {
		if (action != kActionObjUseChapayev || getChapay()->_isShown) {
			getCursor()->setAction(action);

			// original bug fix
			_mainInterface->onMouseMove(g_system->getEventManager()->getMousePos());
		}
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
		/*obj->_z =*/ s->readUint32LE();
		obj->_x = s->readUint32LE();
		obj->_y = s->readUint32LE();
		obj->_isShown = s->readUint32LE();
		obj->_isActive = s->readUint32LE();
		obj->_animate = s->readUint32LE();
	}

	uint itemSize = s->readUint32LE();
	QObjectCase *objCase = getCase();
	objCase->_items.clear();
	for (uint i = 0; i < itemSize; ++i) {
		objCase->_items.push_back(s->readSint32LE());
	}

	_room = (QObjectBG *)findObject(readString(s));
	if (_room) {
		_mainInterface->loadRoom(_room->_id, true);
	}

	QObjectPetka *petka = getPetka();
	QObjectChapayev *chapayev = getChapay();

	Common::Point pos;
	pos.x = s->readSint32LE();
	pos.y = s->readSint32LE();

	petka->setPos(pos, false);

	_xOffset = CLIP<int>(pos.x - 320, 0, _sceneWidth - 640);

	pos.x = s->readSint32LE();
	pos.y = s->readSint32LE();

	chapayev->setPos(pos, false);

	_vm.getBigDialogue()->load(s);

	QObjectCursor *cursor = getCursor();
	cursor->_resourceId = s->readUint32LE();
	cursor->_actionType = s->readUint32LE();
	int invObjId = s->readSint32LE();
	if (invObjId != -1) {
		cursor->_invObj = findObject(invObjId);
	} else {
		cursor->_invObj = nullptr;
	}

	int imageId = s->readSint32LE();
	if (imageId != -1 && !(imageId % 100)) {
		addMessage(petka->_id, kImage, imageId, 1);
	}

	imageId = s->readSint32LE();
	if (imageId != -1 && !(imageId % 100)) {
		addMessage(chapayev->_id, kImage, imageId, 1);
	}

	getStar()->_isActive = true;

	_vm.videoSystem()->makeAllDirty();
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

	QObjectCase *objCase = getCase();
	s->writeUint32LE(objCase->_items.size());
	for (uint i = 0; i < objCase->_items.size(); ++i) {
		s->writeSint32LE(objCase->_items[i]);
	}

	writeString(s, _room->_name);

	QObjectPetka *petka = getPetka();
	QObjectChapayev *chapayev = getChapay();

	FlicDecoder *petkaFlc = _vm.resMgr()->getFlic(petka->_resourceId);
	FlicDecoder *chapayFlc = _vm.resMgr()->getFlic(chapayev->_resourceId);

	s->writeSint32LE(petka->_x - petkaFlc->getCurrentFrame()->w * petka->_k * -0.5);
	s->writeSint32LE(petka->_y + petkaFlc->getCurrentFrame()->h * petka->_k);

	s->writeSint32LE(chapayev->_x - chapayFlc->getCurrentFrame()->w * chapayev->_k * -0.5);
	s->writeSint32LE(chapayev->_y + chapayFlc->getCurrentFrame()->h * chapayev->_k);

	_vm.getBigDialogue()->save(s);

	QObjectCursor *cursor = getCursor();
	s->writeUint32LE(cursor->_resourceId);
	s->writeUint32LE(cursor->_actionType);
	if (cursor->_invObj) {
		s->writeSint32LE(cursor->_invObj->_id);
	} else {
		s->writeSint32LE(-1);
	}

	s->writeSint32LE(petka->_imageId);
	s->writeSint32LE(chapayev->_imageId);
}

QObjectPetka *QSystem::getPetka() const {
	return (QObjectPetka *)_allObjects[0];
}

QObjectChapayev *QSystem::getChapay() const {
	return (QObjectChapayev *)_allObjects[1];
}

QObjectCursor *QSystem::getCursor() const {
	return (QObjectCursor *)_allObjects[_allObjects.size() - 3];
}

QObjectCase *QSystem::getCase() const {
	return (QObjectCase *)_allObjects[_allObjects.size() - 2];
}

QObjectStar *QSystem::getStar() const {
	return (QObjectStar *)_allObjects.back();
}

void QSystem::onEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE: {
		Common::Point p = event.mouse;
		p.x += _xOffset;
		_currInterface->onMouseMove(p);
		break;
	}
	case Common::EVENT_LBUTTONDOWN: {
		Common::Point p = event.mouse;
		p.x += _xOffset;
		_currInterface->onLeftButtonDown(p);
		break;
	}
	case Common::EVENT_RBUTTONDOWN: {
		Common::Point p = event.mouse;
		p.x += _xOffset;
		_currInterface->onRightButtonDown(p);
		break;
	}
	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_1:
		case Common::KEYCODE_l:
			setCursorAction(kActionLook);
			break;
		case Common::KEYCODE_2:
		case Common::KEYCODE_w:
			setCursorAction(kActionWalk);
			break;
		case Common::KEYCODE_3:
		case Common::KEYCODE_g:
			setCursorAction(kActionTake);
			break;
		case Common::KEYCODE_4:
		case Common::KEYCODE_u:
			setCursorAction(kActionUse);
			break;
		case Common::KEYCODE_5:
		case Common::KEYCODE_t:
			setCursorAction(kActionTalk);
			break;
		case Common::KEYCODE_6:
		case Common::KEYCODE_c:
			setCursorAction(kActionObjUseChapayev);
			break;
		case Common::KEYCODE_i:
			toggleCase();
			break;
		case Common::KEYCODE_TAB:
		case Common::KEYCODE_m:
			toggleMapInterface();
			break;
		case Common::KEYCODE_o:
			togglePanelInterface();
			break;
		case Common::KEYCODE_ESCAPE:
			goPrevInterface();
			break;
		case Common::KEYCODE_F2: {
			InterfaceSaveLoad::saveScreen();
			startSaveLoad(kSaveMode);
			break;
		}
		case Common::KEYCODE_F3:
			startSaveLoad(kLoadMode);
			break;
		case Common::KEYCODE_r:
			if (event.kbd.flags & Common::KBD_ALT) {
				_mainInterface->_dialog.fixCursor(); // Buggy in original
			}
			break;
		default:
			break;
		}
	default:
		break;
	}
}

void QSystem::goPrevInterface() {
	getCase()->show(false);
	if (_currInterface != _startupInterface.get() && _currInterface != _sequenceInterface.get())
		_currInterface->stop();
}

void QSystem::toggleCase() {
	if (_currInterface == _mainInterface.get() && getStar()->_isActive) {
		QObjectCase *obj = getCase();
		obj->show(obj->_isShown == 0);
	}
}

void QSystem::startSaveLoad(int id) {
	if (_currInterface == _mainInterface.get() && getStar()->_isActive) {
		_saveLoadInterface->start(id);
	}
}

}
