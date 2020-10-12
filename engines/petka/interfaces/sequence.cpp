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

#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/sound.h"
#include "petka/petka.h"
#include "petka/video.h"
#include "petka/objects/object.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/sequence.h"

namespace Petka {

InterfaceSequence::InterfaceSequence() {
	_fxId = 0;
	_musicId = 0;
}

void InterfaceSequence::start(int id) {
	removeObjects();

	g_system->getMixer()->pauseAll(true);

	QObjectBG* bg = (QObjectBG *)g_vm->getQSystem()->findObject(id);
	_objs.push_back(bg);

	playSound(bg->_musicId, Audio::Mixer::kMusicSoundType);
	playSound(bg->_fxId, Audio::Mixer::kSFXSoundType);

	const BGInfo *info = g_vm->getQSystem()->_mainInterface->findBGInfo(id);
	if (info) {
		for (uint i = 0; i < info->attachedObjIds.size(); ++i) {
			QMessageObject *obj = g_vm->getQSystem()->findObject(info->attachedObjIds[i]);
			g_vm->resMgr()->getFlic(obj->_resourceId);
			obj->loadSound();
			_objs.push_back(obj);
		}
	}

	g_vm->getQSystem()->_currInterface = this;
	g_vm->videoSystem()->makeAllDirty();
}

void InterfaceSequence::stop() {
	removeObjects();

	// original bug fix
	QObjectBG *room = g_vm->getQSystem()->_room;
	if (!room || room->_fxId != _fxId)
		g_vm->soundMgr()->removeSound(g_vm->resMgr()->findSoundName(_fxId));
	if (!room || room->_musicId != _musicId)
		g_vm->soundMgr()->removeSound(g_vm->resMgr()->findSoundName(_musicId));

	_fxId = 0;
	_musicId = 0;

	g_system->getMixer()->pauseAll(false);

	g_vm->getQSystem()->_currInterface = g_vm->getQSystem()->_mainInterface.get();
	Interface::stop();
}

void InterfaceSequence::onLeftButtonDown(Common::Point p) {
	QVisibleObject *obj = findObject(-2);
	if (obj) {
		obj->onClick(p);
	}
}

void InterfaceSequence::removeObjects() {
	removeTexts();
	for (uint i = 0; i < _objs.size(); ++i) {
		QMessageObject *obj = (QMessageObject *)_objs[i];
		obj->removeSound();
	}
	_objs.clear();
}

void InterfaceSequence::playSound(int id, Audio::Mixer::SoundType type) {
	int *soundId = (type == Audio::Mixer::kSFXSoundType) ? &_fxId : &_musicId;
	if (*soundId == id) {
		Sound *s = g_vm->soundMgr()->findSound(g_vm->resMgr()->findSoundName(id));
		if (s) {
			s->pause(false);
		}
	} else {
		g_vm->soundMgr()->removeSound(g_vm->resMgr()->findSoundName(*soundId));
		Sound *sound = g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(id), type);
		if (sound) {
			sound->play(true);
		}
		*soundId = id;
	}
}

} // End of namespace Petka
