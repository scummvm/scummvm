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

#include <common/system.h>
#include "common/stream.h"
#include "common/events.h"
#include "common/ini-file.h"

#include "petka/flc.h"
#include "petka/objects/object.h"
#include "petka/interfaces/main.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/sound.h"
#include "petka/petka.h"
#include "petka/video.h"

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
			obj->_field14 = stream->readSint32LE();
			obj->_field18 = stream->readSint32LE();
		}
	}
}

void InterfaceMain::start() {
	g_vm->getQSystem()->update();
	g_vm->getQSystem()->_isIniting = 0;

	Common::ScopedPtr<Common::SeekableReadStream> bgsStream(g_vm->openFile("BGs.ini", false));
	Common::INIFile bgsIni;
	bgsIni.loadFromStream(*bgsStream);
	Common::String startRoom;
	bgsIni.getKey("StartRoom", "Settings", startRoom);
	loadRoom(g_vm->getQSystem()->findObject(startRoom)->_id, false);
}

void InterfaceMain::loadRoom(int id, bool fromSave) {
	QSystem *sys = g_vm->getQSystem();
	stop();
	QObjectBG *room = (QObjectBG *)sys->findObject(_roomId);
	if (room) {
		if (!fromSave)
			sys->addMessageForAllObjects(kLeaveBG, 0, 0, 0, 0, room);
		g_vm->resMgr()->clearUnneeded();
		g_vm->soundMgr()->removeSoundsWithType(Audio::Mixer::kSFXSoundType);
		const BGInfo *info = findBGInfo(room->_id);
		if (info) {
			for (uint i = 0; i < _objs.size();) {
				bool removed = false;
				if (_roomId == ((QMessageObject *) _objs[i])->_id) {
					_objs.remove_at(i);
					removed = true;
				} else {
					for (uint j = 0; j < info->attachedObjIds.size(); ++j) {
						if (info->attachedObjIds[j] == ((QMessageObject *) _objs[i])->_id) {
							g_vm->soundMgr()->removeSound(g_vm->resMgr()->findSoundName(_objs[i]->_resourceId));
							_objs.remove_at(i);
							removed = true;
							break;
						}
					}
				}
				if (!removed)
					++i;
			}
		}
	}


	_roomId = id;
	const BGInfo *info = findBGInfo(id);
	room = (QObjectBG *)sys->findObject(info->objId);
	g_vm->resMgr()->loadBitmap(room->_resourceId);
	_objs.push_back(room);
	for (uint i = 0; i < info->attachedObjIds.size(); ++i) {
		QMessageObject *obj = g_vm->getQSystem()->findObject(info->attachedObjIds[i]);
		obj->_sound = g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(obj->_resourceId), Audio::Mixer::kSFXSoundType);
		obj->_hasSound = obj->_sound != nullptr;
		obj->_startSound = false;
		if (obj->_isShown || obj->_isActive)
			g_vm->resMgr()->loadFlic(obj->_resourceId);
		_objs.push_back(obj);
	}
	if (sys->_musicId != room->_musicId) {
		g_vm->soundMgr()->removeSound(g_vm->resMgr()->findSoundName(sys->_musicId));
		g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(room->_musicId), Audio::Mixer::kMusicSoundType)->play(true);
		sys->_musicId = room->_musicId;
	}
	if (sys->_fxId != room->_fxId) {
		g_vm->soundMgr()->removeSound(g_vm->resMgr()->findSoundName(sys->_fxId));
		g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(room->_fxId), Audio::Mixer::kSFXSoundType)->play(true);
		sys->_fxId = room->_fxId;
	}
	if (!fromSave)
		g_vm->getQSystem()->addMessageForAllObjects(kInitBG, 0, 0, 0, 0, room);
}

const BGInfo *InterfaceMain::findBGInfo(int id) const {
	for (uint i = 0; i < _bgs.size(); ++i) {
		if (_bgs[i].objId == id)
			return &_bgs[i];
	}
	return nullptr;
}

} // End of namespace Petka

