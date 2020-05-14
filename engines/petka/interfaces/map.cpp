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

#include "petka/q_manager.h"
#include "petka/flc.h"
#include "petka/petka.h"
#include "petka/q_system.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/map.h"
#include "petka/objects/object_cursor.h"
#include "petka/video.h"

namespace Petka {

const char *const mapName = "\xCA\xC0\xD0\xD2\xC0"; // КАРТА

void InterfaceMap::start() {
	if (!g_vm->getQSystem()->_room->_showMap)
		return;

	QObjectBG *bg = (QObjectBG *)g_vm->getQSystem()->findObject(mapName);
	_roomResID = bg->_resourceId;
	_objs.push_back(bg);

	const Common::Array<BGInfo> &infos = g_vm->getQSystem()->_mainInterface->_bgs;

	for (uint i = 0; i < infos.size(); ++i) {
		if (infos[i].objId != bg->_id) {
			continue;
		}
		for (uint j = 0; j < infos[i].attachedObjIds.size(); ++j) {
			QMessageObject *obj = g_vm->getQSystem()->findObject(infos[i].attachedObjIds[j]);
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(obj->_resourceId);
			flc->setFrame(1);
			obj->_z = 1;
			obj->_x = 0;
			obj->_y = 0;
			obj->_field24 = 1;
			obj->_field20 = 1;
			obj->_field28 = 1;
			obj->_animate = obj->_isShown;
			_objs.push_back(obj);
		}
		break;
	}

	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	_savedCursorId = cursor->_resourceId;
	_savedCursorActionType = cursor->_actionType;
	cursor->_isShown = true;
	cursor->_resourceId = 4901;
	cursor->_animate = 0;
	cursor->_actionType;

	_objs.push_back(cursor);
	cursor->setCursorPos(cursor->_x, cursor->_y, false);

	_savedXOffset = 0; // g_vm->getQSystem()->xOffset;
	_savedSceneWidth = 640; // g_vm->getQSystem()->savedSceneWidth

	g_vm->getQSystem()->addMessageForAllObjects(kInitBG, 0, 0, 0, 0, bg);

	g_vm->getQSystem()->_currInterface = this;
	g_vm->videoSystem()->updateTime();
	g_vm->videoSystem()->makeAllDirty();
}

void InterfaceMap::stop() {
	if (_objUnderCursor)
		((QMessageObject *)_objUnderCursor)->_isShown = false;
	//setText("", 0xFFFFC0, 0xA0A0A);

	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	cursor->_resourceId = _savedCursorId;
	cursor->_actionType = _savedCursorActionType;

	g_vm->getQSystem()->_currInterface = g_vm->getQSystem()->_prevInterface;
	g_vm->getQSystem()->_currInterface->onMouseMove(Common::Point(cursor->_x, cursor->_y));
}

void InterfaceMap::onLeftButtonDown(const Common::Point p) {
	for (int i = _objs.size() - 1; i >= 0; --i) {
		if (_objs[i]->isInPoint(p.x, p.y)) {
			_objs[i]->onClick(p.x, p.y);
			break;
		}
	}
}

void InterfaceMap::onMouseMove(const Common::Point p) {
	_objUnderCursor = nullptr;

	bool found = false;
	for (int i = _objs.size() - 1; i > 0; --i) {
		QMessageObject *obj = (QMessageObject *)_objs[i];
		if (obj->_resourceId != 4901 && obj->_resourceId != _roomResID) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(obj->_resourceId);
			if (flc) {
				int frame = 1;
				if (!found && obj->isInPoint(p.x, p.y)) {
					found = true;
					frame = 2;
					_objUnderCursor = obj;
				}
				if (obj->_field20 != frame) {
					obj->_isShown = frame == 2;
					flc->setFrame(1);
					g_vm->videoSystem()->addDirtyRect(Common::Point(obj->_x, obj->_y), *flc);
					obj->_field20 = frame;
					obj->_field24 = frame;
				}
			}
		}
	}

	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	cursor->_animate = _objUnderCursor != nullptr;
	cursor->_isShown = true;
	cursor->setCursorPos(p.x, p.y, 0);
}

} // End of namespace Petka
