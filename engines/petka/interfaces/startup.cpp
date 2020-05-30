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

#include "petka/flc.h"
#include "petka/objects/object_cursor.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/save_load.h"
#include "petka/interfaces/startup.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/sound.h"
#include "petka/petka.h"
#include "petka/video.h"

namespace Petka {

const char *const kStartupObjName = "STARTUP";
const char *const kCreditsVideoName = "credits.avi";

enum {
	kExit = 4981,
	kCredits = 4982,
	kLoad = 4983,
	kNewGame = 4984,
	kStartupCursorId = 4901,
	kBackgroundId = 4980
};

void InterfaceStartup::start(int id) {
	g_vm->getQSystem()->update();
	g_vm->getQSystem()->_isIniting = 0;

	QObjectBG *bg = (QObjectBG *)g_vm->getQSystem()->findObject(kStartupObjName);
	_objs.push_back(bg);

	Sound *s = g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(bg->_musicId), Audio::Mixer::kMusicSoundType);
	s->play(true);

	const BGInfo *info = g_vm->getQSystem()->_mainInterface->findBGInfo(bg->_id);
	for (uint i = 0; i < info->attachedObjIds.size(); ++i) {
		QMessageObject *obj = g_vm->getQSystem()->findObject(info->attachedObjIds[i]);
		obj->_z = 1;
		obj->_x = 0;
		obj->_y = 0;
		obj->_frame = 1;
		obj->_animate = 0;
		obj->_isShown = 0;
		_objs.push_back(obj);
	}

	initCursor(kStartupCursorId, 1, 0);
}

void InterfaceStartup::onLeftButtonDown(const Common::Point p) {
	if (!_objUnderCursor)
		return;
	switch (_objUnderCursor->_resourceId) {
	case kExit:
		g_system->quit();
		break;
	case kCredits:
		g_vm->playVideo(g_vm->openFile(kCreditsVideoName, false));
		break;
	case kLoad:
		g_vm->getQSystem()->_saveLoadInterface->start(kLoadMode);
		break;
	case kNewGame:
		g_vm->loadPart(1);
		break;
	}
}

void InterfaceStartup::onMouseMove(const Common::Point p) {
	_objUnderCursor = nullptr;
	bool found = false;
	for (int i = _objs.size() - 1; i > 0; --i) {
		QMessageObject *obj = (QMessageObject *)_objs[i];
		if (obj->_resourceId != kStartupCursorId && obj->_resourceId != kBackgroundId) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(obj->_resourceId);
			if (flc) {
				bool show = false;
				if (!found && obj->isInPoint(p.x, p.y)) {
					found = true;
					show = true;
					_objUnderCursor = obj;
				}
				if (obj->_isShown != show) {
					obj->_isShown = show;
					flc->setFrame(1);
					g_vm->videoSystem()->addDirtyRect(flc->getBounds());
				}
			}
		}
	}

	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	cursor->_animate = _objUnderCursor != nullptr;
	cursor->_isShown = true;
	cursor->setCursorPos(p.x, p.y, 0);
}

void InterfaceStartup::stop() {
	QObjectBG *bg = (QObjectBG *)g_vm->getQSystem()->findObject(kStartupObjName);
	Common::String sound = g_vm->resMgr()->findSoundName(bg->_musicId);
	g_vm->soundMgr()->removeSound(sound);
	Interface::stop();
}

} // End of namespace Petka
