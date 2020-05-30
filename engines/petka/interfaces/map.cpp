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
#include "common/str-enc.h"

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

void InterfaceMap::start(int id) {
	QSystem *sys = g_vm->getQSystem();
	if (!sys->_room->_showMap)
		return;

	_objs.clear();

	QObjectBG *bg = (QObjectBG *)sys->findObject(mapName);
	_roomResID = bg->_resourceId;
	_objs.push_back(bg);

	const BGInfo *info = g_vm->getQSystem()->_mainInterface->findBGInfo(id);
	for (uint i = 0; i < info->attachedObjIds.size(); ++i) {
		QMessageObject *obj = sys->findObject(info->attachedObjIds[i]);
		FlicDecoder *flc = g_vm->resMgr()->loadFlic(obj->_resourceId);
		flc->setFrame(1);
		obj->_z = 1;
		obj->_x = 0;
		obj->_y = 0;
		obj->_frame = 1;
		obj->_animate = obj->_isShown;
		_objs.push_back(obj);
	}

	QObjectCursor *cursor = sys->getCursor();
	_savedCursorId = cursor->_resourceId;
	_savedCursorActionType = cursor->_actionType;

	initCursor(4901, 1, 0);

	_savedXOffset = sys->_xOffset;
	_savedSceneWidth =  sys->_sceneWidth;

	g_vm->getQSystem()->addMessageForAllObjects(kInitBG, 0, 0, 0, 0, bg);

	g_vm->getQSystem()->_currInterface = this;
	g_vm->videoSystem()->updateTime();
	g_vm->videoSystem()->makeAllDirty();
}

void InterfaceMap::stop() {
	QSystem *sys = g_vm->getQSystem();
	QObjectCursor *cursor = sys->getCursor();

	if (_objUnderCursor)
		((QMessageObject *)_objUnderCursor)->_isShown = false;

	setText(Common::U32String(""), 0, 0);

	sys->_xOffset = _savedXOffset;
	sys->_sceneWidth = _savedSceneWidth;

	cursor->_resourceId = _savedCursorId;
	cursor->_actionType = _savedCursorActionType;

	sys->_currInterface = g_vm->getQSystem()->_prevInterface;
	sys->_currInterface->onMouseMove(Common::Point(cursor->_x, cursor->_y));

	Interface::stop();
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
	QVisibleObject *oldObj = _objUnderCursor;
	_objUnderCursor = nullptr;

	bool found = false;
	for (int i = _objs.size() - 1; i > 0; --i) {
		QMessageObject *obj = (QMessageObject *)_objs[i];
		if (obj->_resourceId != 4901 && obj->_resourceId != _roomResID) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(obj->_resourceId);
			if (flc) {
				bool show = false;
				if (!found && obj->isInPoint(p.x, p.y)) {
					found = true;
					show = true;
					_objUnderCursor = obj;
				}
				if (obj->_isShown != show)
					obj->show(obj->_isShown == 0);
			}
		}
	}

	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	cursor->_animate = _objUnderCursor != nullptr;
	cursor->_isShown = true;
	cursor->setCursorPos(p.x, p.y, 0);

	if (_objUnderCursor != oldObj && _objUnderCursor) {
		Graphics::PixelFormat fmt = g_system->getScreenFormat();
		QMessageObject *obj = (QMessageObject *)_objUnderCursor;
		if (!obj->_nameOnScreen.empty()) {
			setText(Common::convertToU32String(obj->_nameOnScreen.c_str(), Common::kWindows1251), fmt.RGBToColor(0xC0, 0xFF, 0xFF), fmt.RGBToColor(0xA, 0xA, 0xA));
		} else {
			setText(Common::convertToU32String(obj->_name.c_str(), Common::kWindows1251), fmt.RGBToColor(0x80, 0, 0), fmt.RGBToColor(0xA, 0xA, 0xA));
		}
	} else if (oldObj && !_objUnderCursor) {
		setText(Common::U32String(""), 0, 0);
	}
}

void InterfaceMap::onRightButtonDown(const Common::Point p) {
	stop();
}

} // End of namespace Petka
