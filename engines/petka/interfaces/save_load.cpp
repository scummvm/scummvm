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

InterfaceSaveLoad::InterfaceSaveLoad() {
	_page = 0;
	_loadMode = false;

	_saveRects[0] = Common::Rect(43, 84, 151, 166);
	_saveRects[1] = Common::Rect(43, 209, 151, 291);
	_saveRects[2] = Common::Rect(43, 335, 151, 417);
	_saveRects[3] = Common::Rect(358, 75, 466, 157);
	_saveRects[4] = Common::Rect(360, 200, 468, 282);
	_saveRects[5] = Common::Rect(359, 325, 467, 407);
	_nextPageRect = Common::Rect(596, 403, 624, 431);
	_prevPageRect = Common::Rect(10, 414, 38, 442);
}

void InterfaceSaveLoad::start(int id) {
	QSystem *sys = g_vm->getQSystem();

	_loadMode = (id == kLoadMode);

	QObjectBG *bg = (QObjectBG *)sys->findObject("SAVELOAD");
	_objs.push_back(bg);
	bg->_resourceId = kFirstSaveLoadPageId + _page + (_loadMode ? 0 : 5);

	SubInterface::start(id);
}

void InterfaceSaveLoad::onLeftButtonDown(Common::Point p) {
	int index = findSaveLoadRectIndex(p);
	if (index == -1) {
		if (_prevPageRect.contains(p) && _page > 0) {
			_page--;
		} else if (_nextPageRect.contains(p) && _page < 2) {
			_page++;
		}
		stop();
		start(_loadMode ? kLoadMode : kSaveMode);
	} else {

	}
}

void InterfaceSaveLoad::onRightButtonDown(Common::Point p) {
	stop();
}

void InterfaceSaveLoad::onMouseMove(Common::Point p) {
	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	cursor->_animate = findSaveLoadRectIndex(p) != -1 || _nextPageRect.contains(p) || _prevPageRect.contains(p);
	cursor->setPos(p, false);
}

int InterfaceSaveLoad::findSaveLoadRectIndex(Common::Point p) {
	for (uint i = 0; i < ARRAYSIZE(_saveRects); ++i) {
		if (_saveRects[i].contains(p)) {
			return i;
		}
	}
	return -1;
}

} // End of namespace Petka

