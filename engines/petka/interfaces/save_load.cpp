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

#include "petka/petka.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/video.h"
#include "petka/objects/object_cursor.h"
#include "petka/interfaces/save_load.h"

namespace Petka {

const uint kFirstSaveLoadPageId = 4990;
const Common::Rect kSavesRects[] = {Common::Rect(43, 84, 151, 166), Common::Rect(43, 209, 151, 291),
									Common::Rect(43, 335, 151, 417), Common::Rect(358, 75, 466, 157),
									Common::Rect(360, 200, 468, 282), Common::Rect(359, 325, 467, 407)};
const Common::Rect kNextPageRect(596, 403, 624, 431);
const Common::Rect kPrevPageRect(10, 414, 38, 442);

void InterfaceSaveLoad::startSaveLoad(bool saveMode) {
	_loadMode = !saveMode;
	QObjectBG *bg = (QObjectBG *)g_vm->getQSystem()->findObject("SAVELOAD");
	_objs.push_back(bg);
	bg->_resourceId = kFirstSaveLoadPageId + _page + (_loadMode ? 0 : 5);

	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	_objs.push_back(cursor);
	_savedCursorId = cursor->_resourceId;
	cursor->_resourceId = 4901;
	cursor->_isShown = 1;
	cursor->_animate = 0;
	cursor->setCursorPos(cursor->_x, cursor->_y, 0);

	g_vm->getQSystem()->_currInterface = this;
	g_vm->videoSystem()->makeAllDirty();
}

void InterfaceSaveLoad::stop() {
	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	cursor->_resourceId = _savedCursorId;
	g_vm->getQSystem()->_currInterface = g_vm->getQSystem()->_prevInterface;
	g_vm->getQSystem()->_currInterface->onMouseMove(Common::Point(cursor->_x, cursor->_y));
	g_vm->videoSystem()->makeAllDirty();
}

void InterfaceSaveLoad::onLeftButtonDown(const Common::Point p) {
	int index = findSaveLoadRectIndex(p);
	if (index == -1) {
		if (kPrevPageRect.contains(p) && _page > 0) {
			_page--;
		} else if (kNextPageRect.contains(p) && _page < 2) {
			_page++;
		}
		stop();
		startSaveLoad(_loadMode == 0);
	} else {

	}
}

void InterfaceSaveLoad::onRightButtonDown(const Common::Point p) {
	stop();
}

void InterfaceSaveLoad::onMouseMove(const Common::Point p) {
	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	cursor->_animate = findSaveLoadRectIndex(p) != -1 || kNextPageRect.contains(p) || kPrevPageRect.contains(p);
	cursor->setCursorPos(p.x, p.y, 0);
}

int InterfaceSaveLoad::findSaveLoadRectIndex(const Common::Point p) {
	int i = 0;
	do {
		if (kSavesRects[i].contains(p)) {
			return i;
		}
	} while (++i < sizeof(kSavesRects) / sizeof(Common::Rect));
	return -1;
}

} // End of namespace Petka

