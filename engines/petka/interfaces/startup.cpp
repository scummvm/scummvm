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

#include "petka/flc.h"
#include "petka/obj.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/startup.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/sound.h"
#include "petka/petka.h"
#include "petka/video.h"

namespace Petka {

void InterfaceStartup::start() {
	g_vm->getQSystem()->update();
	g_vm->getQSystem()->_field48 = 0;

	QObjectBG *bg = (QObjectBG *)g_vm->getQSystem()->findObject("STARTUP");
	_objs.push_back(bg);

	Sound *s = g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(bg->_musicId), Audio::Mixer::SoundType::kMusicSoundType);
	s->play(true);

	const Common::Array<BGInfo> &infos = g_vm->getQSystem()->_mainInterface->_bgs;

	for (uint i = 0; i < infos.size(); ++i) {
		if (infos[i].objId != bg->_id) {
			continue;
		}
		for (uint j = 0; j < infos[i].attachedObjIds.size(); ++j) {
			QMessageObject *obj = g_vm->getQSystem()->findObject(infos[i].attachedObjIds[j]);
			obj->_z = 1;
			obj->_x = 0;
			obj->_y = 0;
			obj->_field24 = 1;
			obj->_field20 = 1;
			obj->_field28 = 1;
			obj->_animate = 0;
			obj->_isShown = 0;
			_objs.push_back(obj);
		}
	}


	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	cursor->_resourceId = 5007;
	cursor->_isShown = true;
	cursor->_animate = false;
	_objs.push_back(g_vm->getQSystem()->_cursor.get());
	cursor->setCursorPos(cursor->_x, cursor->_y, 0);
}

void InterfaceStartup::onLeftButtonDown(const Common::Point p) {
	if (!_objUnderCursor)
		return;
	switch (_objUnderCursor->_resourceId) {
	case 4981:
		g_system->quit();
		break;
	case 4982:
		g_vm->playVideo(g_vm->openFile("credits.avi", false));
		break;
	case 4983:

		break;
	case 4984:
		// new game
		break;
	}
}

void InterfaceStartup::onMouseMove(const Common::Point p) {
	_objUnderCursor = nullptr;
	bool found = false;
	for (int i = _objs.size() - 1; i >= 0; --i) {
		QMessageObject *obj = (QMessageObject *)_objs[i];
		if (obj->_resourceId != 4901 && obj->_resourceId != 4980) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(obj->_resourceId);
			if (flc) {
				bool clicked = false;
				if (!found && obj->isInPoint(p.x, p.y)) {
					found = true;
					clicked = true;
					obj->_isShown = true;
					_objUnderCursor = obj;
				}
				obj->_isShown = clicked;
				if (flc->getCurFrame() == -1) {
					flc->decodeNextFrame();
				}
				Common::Rect dirty(flc->getBounds());
				dirty.translate(obj->_x, obj->_y);
				g_vm->videoSystem()->addDirtyRect(dirty);
			}
		}
	}

	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	cursor->_animate = _objUnderCursor != nullptr;
	cursor->_isShown = true;
	cursor->setCursorPos(p.x, p.y, 0);
}

} // End of namespace Petka


