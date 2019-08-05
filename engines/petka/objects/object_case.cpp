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

#include "petka/interfaces/main.h"
#include "petka/objects/object_case.h"
#include "petka/flc.h"
#include "petka/petka.h"
#include "petka/q_manager.h"
#include "petka/video.h"
#include "petka/objects/heroes.h"
#include "petka/q_system.h"

namespace Petka {

// Полоска в чемодане
const char *const kPoloska = "\xCF\xEE\xEB\xEE\xF1\xEA\xE0\x20\xE2\x20\xF7\xE5\xEC\xEE\xE4\xE0\xED\xE5";

const Common::Point itemsLocation[] = {{120, 145}, {240, 145}, {360, 145},
									   {100, 220}, {240, 220}, {380, 220}};

QObjectCase::QObjectCase() {
	_itemIndex = 0;
	_isShown = false;
	_updateZ = false;
	_id = 4099;
	_resourceId = 6000;
	_z = 980;
	_clickedObjIndex = -1;
}

void QObjectCase::update(int time) {
	if (!_isShown || _clickedObjIndex >= 6)
		return;
	_time += time;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(6001 + _clickedObjIndex);
	if (flc) {
		while (_time >= flc->getDelay()) {
			flc->setFrame(-1);
			_time -= flc->getDelay();
			g_vm->videoSystem()->addDirtyRectFromMsk(Common::Point(0, 0), *flc);
		}
	}
}

void QObjectCase::show(bool v) {
	_x = 0; // sys->xOffset
	QObject::show(v);
	if (v) {
		fillWithItems();
		QMessageObject *obj = g_vm->getQSystem()->findObject(kPoloska);
		obj->_z = _z + 2;
		obj->_x = 0; // sys->xOffset
		g_vm->getQSystem()->_mainInterface->_objs.push_back(obj);
	} else {
		Common::Array<QVisibleObject *> &objs = g_vm->getQSystem()->_mainInterface->_objs;
		for (int i = 0; i < objs.size();) {
			if (objs[i]->_z == _z + 1 || objs[i]->_z == _z + 2) {
				objs.remove_at(i);
			} else {
				++i;
			}
		}
		g_vm->getQSystem()->_currInterface->_startIndex = 0;
	}
}

bool QObjectCase::isInPoint(int x, int y) {
	return _isShown;
}

void QObjectCase::onMouseMove(int x, int y) {
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(6009);
	if (*(byte *)flc->getCurrentFrame()->getBasePtr(x - _x, y - _y) != 0) {
		if (_clickedObjIndex != 3 && _clickedObjIndex < 6 && _clickedObjIndex >= 0) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(6001 + _clickedObjIndex);
			flc->setFrame(1);
			g_vm->videoSystem()->addDirtyRectFromMsk(Common::Point(0, 0), *flc);
		}
		_clickedObjIndex = 3;
	} else {
		int i;
		for (i = 0; i < 6; ++i) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(6001 + i);
			if (flc->getMskRects()[0].contains(Common::Point(x - _x, y))) {
				break;
			}
		}

		if (_clickedObjIndex != i && _clickedObjIndex < 6 && _clickedObjIndex >= 0) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(6001 + _clickedObjIndex);
			flc->setFrame(1);
			g_vm->videoSystem()->addDirtyRectFromMsk(Common::Point(0, 0), *flc);
		}

		if (i == 6) {
			if (_clickedObjIndex < 6 && _clickedObjIndex >= 0) {
				_clickedObjIndex = -1;
			}
		} else if (i != _clickedObjIndex) {
			if ((i || g_vm->getQSystem()->_chapayev->_isShown) && (i != 2 || g_vm->getQSystem()->_room->_showMap)) {
				FlicDecoder *flc = g_vm->resMgr()->loadFlic(6001 + i);
				g_vm->videoSystem()->addDirtyRectFromMsk(Common::Point(0, 0), *flc);
				_clickedObjIndex = i;
			} else {
				_clickedObjIndex = -1;
			}
		}
	}
}

void QObjectCase::onClick(int x, int y) {
	switch (_clickedObjIndex) {
		case 0:
			// setChapayev()
			break;
		case 1:
			// loadPanel()
			break;
		case 2:
			// loadMap()
			break;
		case 3:
			show(0);
			break;
		case 4:
			if (_items.size() > _itemIndex + 6) {
				_itemIndex += 6;
				fillWithItems();
				g_vm->videoSystem()->makeAllDirty();
			}
			break;
		case 5:
			if (_itemIndex > 0) {
				_itemIndex = (_itemIndex - 6 <= 0) ? 0 : _itemIndex - 6;
				fillWithItems();
				g_vm->videoSystem()->makeAllDirty();
			}
			break;
		default:
			break;
	}
}

void QObjectCase::fillWithItems() {
	Common::Array<QVisibleObject *> &objs = g_vm->getQSystem()->_mainInterface->_objs;
	for (int i = 0; i < objs.size();) {
		if (objs[i]->_z == _z + 1) {
			objs.remove_at(i);
		} else {
			++i;
		}
	}

	for (int i = 0; i < objs.size(); ++i) {
		if (objs[i]->_resourceId == 6000) {
			g_vm->getQSystem()->_currInterface->_startIndex = i;
		}
	}

	for (int i = _itemIndex; i < _itemIndex + 6 >= _items.size() ? _items.size() : _itemIndex + 6; ++i) {
		QMessageObject *obj = g_vm->getQSystem()->findObject(_items[i]);
		obj->_z = _z + 1;
		objs.push_back(obj);
		g_vm->resMgr()->loadFlic(obj->_resourceId);
		_x = itemsLocation[i - _itemIndex].x;
		_y = itemsLocation[i - _itemIndex].y;
	}
}

}
