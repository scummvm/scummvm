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

#include "common/system.h"
#include "common/stream.h"
#include "common/events.h"
#include "common/ini-file.h"

#include "petka/flc.h"
#include "petka/objects/object_case.h"
#include "petka/objects/object_cursor.h"
#include "petka/objects/object_star.h"
#include "petka/interfaces/main.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/sound.h"
#include "petka/petka.h"
#include "petka/video.h"
#include "petka/objects/object_case.h"
#include "petka/objects/heroes.h"
#include "petka/objects/text.h"
#include "petka/walk.h"

namespace Petka {

InterfaceMain::InterfaceMain() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(g_vm->openFile("backgrnd.bg", true));
	if (!stream)
		return;
	_bgs.resize(stream->readUint32LE());
	for (uint i = 0; i < _bgs.size(); ++i) {
		_bgs[i].objId = stream->readUint16LE();
		_bgs[i].attachedObjIds.resize(stream->readUint32LE());
		for (uint j = 0; j < _bgs[i].attachedObjIds.size(); ++j) {
			_bgs[i].attachedObjIds[j] = stream->readUint16LE();
			QMessageObject *obj = g_vm->getQSystem()->findObject(_bgs[i].attachedObjIds[j]);
			obj->_x = stream->readSint32LE();
			obj->_y = stream->readSint32LE();
			obj->_z = stream->readSint32LE();
			obj->_walkX = stream->readSint32LE();
			obj->_walkY = stream->readSint32LE();
		}
	}

	_objs.push_back(g_vm->getQSystem()->getCursor());
	_objs.push_back(g_vm->getQSystem()->getCase());
	_objs.push_back(g_vm->getQSystem()->getStar());
}

void InterfaceMain::start(int id) {
	_objs.push_back(g_vm->getQSystem()->getPetka());
	_objs.push_back(g_vm->getQSystem()->getChapay());

	Common::ScopedPtr<Common::SeekableReadStream> bgsStream(g_vm->openFile("BGs.ini", true));
	Common::INIFile bgsIni;
	bgsIni.allowNonEnglishCharacters();
	bgsIni.loadFromStream(*bgsStream);
	Common::String startRoom;
	bgsIni.getKey("StartRoom", "Settings", startRoom);
	if (g_vm->getSaveSlot() == -1)
		loadRoom(g_vm->getQSystem()->findObject(startRoom)->_id, false);
}

void InterfaceMain::loadRoom(int id, bool fromSave) {
	QSystem *sys = g_vm->getQSystem();

	sys->_currInterface->stop();
	if (_roomId == id)
		return;

	unloadRoom(fromSave);

	const BGInfo *info = findBGInfo(id);
	QObjectBG *room = (QObjectBG *)sys->findObject(id);
	QManager *resMgr = g_vm->resMgr();

	_roomId = id;
	sys->_room = room;
	_objs.push_back(room);

	auto surface = resMgr->getSurface(room->_resourceId);
	if (surface) {
		sys->_sceneWidth = surface->w;
		sys->_xOffset = 0;
	}

	for (uint i = 0; i < info->attachedObjIds.size(); ++i) {
		QMessageObject *obj = sys->findObject(info->attachedObjIds[i]);
		obj->loadSound();
		if (obj->_isShown || obj->_isActive)
			g_vm->resMgr()->getFlic(obj->_resourceId);
		_objs.push_back(obj);
	}

	auto petka = sys->getPetka();
	auto chapay = sys->getChapay();

	auto bkgName = resMgr->findResourceName(room->_resourceId);

	petka->_walk->setBackground(bkgName);
	chapay->_walk->setBackground(bkgName);

	petka->setPos(Common::Point(petka->_x, petka->_y), false);
	chapay->setPos(Common::Point(chapay->_x, chapay->_y), false);

	playSound(room->_musicId, Audio::Mixer::kMusicSoundType);
	playSound(room->_fxId, Audio::Mixer::kSFXSoundType);

	if (!fromSave)
		sys->addMessageForAllObjects(kInitBG, 0, 0, 0, 0, room);
	g_vm->videoSystem()->updateTime();
}

void InterfaceMain::playSound(int id, Audio::Mixer::SoundType type) {
	int *sysId = (type == Audio::Mixer::kMusicSoundType) ? &g_vm->getQSystem()->_musicId : &g_vm->getQSystem()->_fxId;
	if (*sysId != id) {
		g_vm->soundMgr()->removeSound(g_vm->resMgr()->findSoundName(*sysId));
		Sound *sound = g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(id), Audio::Mixer::kMusicSoundType); // kMusicSoundType intended
		if (sound) {
			sound->play(true);
		}
		*sysId = id;
	}
}

const BGInfo *InterfaceMain::findBGInfo(int id) const {
	for (uint i = 0; i < _bgs.size(); ++i) {
		if (_bgs[i].objId == id)
			return &_bgs[i];
	}
	return nullptr;
}

void InterfaceMain::unloadRoom(bool fromSave) {
	if (_roomId == -1)
		return;
	QSystem *sys = g_vm->getQSystem();
	QObjectBG *room = (QObjectBG *)sys->findObject(_roomId);
	if (!room)
		return;

	if (!fromSave)
		sys->addMessageForAllObjects(kLeaveBG, 0, 0, 0, 0, room);

	g_vm->soundMgr()->removeSoundsWithType(Audio::Mixer::kSFXSoundType);
	g_vm->resMgr()->clearUnneeded();

	_objs.clear();

	_objs.push_back(sys->getCursor());
	_objs.push_back(sys->getCase());
	_objs.push_back(sys->getStar());
	_objs.push_back(sys->getPetka());
	_objs.push_back(sys->getChapay());
}

void InterfaceMain::onLeftButtonDown(Common::Point p) {
	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	if (!cursor->_isShown) {
		_dialog.next(-1);
		return;
	}

	for (int i = _objs.size() - 1; i >= 0; --i) {
		if (_objs[i]->isInPoint(p)) {
			_objs[i]->onClick(p);
			return;
		}
	}

	switch (cursor->_actionType) {
	case kActionWalk: {
		QObjectPetka *petka = g_vm->getQSystem()->getPetka();
		if (petka->_heroReaction) {
			for (uint i = 0; i < petka->_heroReaction->messages.size(); ++i) {
				if (petka->_heroReaction->messages[i].opcode == kGoTo) {
					QObjectChapayev *chapay = g_vm->getQSystem()->getChapay();
					chapay->stopWalk();
					break;
				}
			}
			delete petka->_heroReaction;
			petka->_heroReaction = nullptr;
		}
		petka->walk(p.x, p.y);
		break;
	}
	case kActionObjUseChapayev: {
		QObjectChapayev *chapay = g_vm->getQSystem()->getChapay();
		chapay->walk(p.x, p.y);
		break;
	}
	default:
		break;
	}
}

void InterfaceMain::onRightButtonDown(Common::Point p) {
	QObjectStar *star = g_vm->getQSystem()->getStar();
	QObjectCase *objCase = g_vm->getQSystem()->getCase();
	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	if (!star->_isActive)
		return;
	if (objCase->_isShown && cursor->_actionType == kActionObjUse) {
		cursor->setAction(kActionTake);
	} else {
		star->setPos(p, false);
		star->show(star->_isShown == 0);
	}
}

void InterfaceMain::onMouseMove(Common::Point p) {
	QMessageObject *prevObj = (QMessageObject *)_objUnderCursor;
	_objUnderCursor = nullptr;

	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	if (cursor->_isShown) {
		for (int i = _objs.size() - 1; i >= 0; --i) {
			if (_objs[i]->isInPoint(p)) {
				_objs[i]->onMouseMove(p);
				break;
			}
		}
	}

	cursor->_animate = _objUnderCursor != nullptr;
	cursor->setPos(p, true);

	if (prevObj != _objUnderCursor && _objUnderCursor && !_dialog.isActive()) {
		Graphics::PixelFormat fmt = g_system->getScreenFormat();
		QMessageObject *obj = (QMessageObject *)_objUnderCursor;
		if (!obj->_nameOnScreen.empty()) {
			setText(Common::convertToU32String(obj->_nameOnScreen.c_str(), Common::kWindows1251), fmt.RGBToColor(0xC0, 0xFF, 0xFF), fmt.RGBToColor(0xA, 0xA, 0xA));
		} else {
			setText(Common::convertToU32String(obj->_name.c_str(), Common::kWindows1251), fmt.RGBToColor(0x80, 0, 0), fmt.RGBToColor(0xA, 0xA, 0xA));
		}
	} else if (prevObj && !_objUnderCursor && !_dialog.isActive()) {
		setText(Common::U32String(), 0, 0);
	}
}

void InterfaceMain::setTextChoice(const Common::Array<Common::U32String> &choices, uint16 color, uint16 selectedColor) {
	removeTexts();
	_objUnderCursor = nullptr;
	_objs.push_back(new QTextChoice(choices, color, selectedColor));
}

void InterfaceMain::setTextDescription(const Common::U32String &text, int frame) {
	removeTexts();
	QObjectStar *star = g_vm->getQSystem()->getStar();
	star->_isActive = false;
	_objUnderCursor = nullptr;
	_hasTextDesc = true;
	_objs.push_back(new QTextDescription(text, frame));
}

void InterfaceMain::removeTextDescription() {
	_hasTextDesc = false;
	_objUnderCursor = nullptr;
	g_vm->getQSystem()->getStar()->_isActive = true;
	removeTexts();
}

void InterfaceMain::update(uint time) {
	QSystem *sys = g_vm->getQSystem();
	int xOff = sys->_xOffset;
	int reqOffset = sys->_reqOffset;
	if (xOff != reqOffset && ((xOff != sys->_sceneWidth - 640 && xOff < reqOffset) || (xOff > 0 && xOff > reqOffset))) {
		if (xOff <= reqOffset) {
			xOff += 8;
			xOff = MIN<int>(xOff, reqOffset);
		} else {
			xOff -= 8;
			xOff = MAX<int>(xOff, reqOffset);
		}
		sys->_xOffset = CLIP(xOff, 0, sys->_sceneWidth - 640);
		g_vm->videoSystem()->makeAllDirty();
	}
	Interface::update(time);
}

} // End of namespace Petka
