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

const uint kCaseZ = 980;
const uint kItemZ = kCaseZ + 1;
const uint kPoloskaZ = kCaseZ + 2;

const uint kButtonsCount = 6;
const uint kItemsOnPage = 6;

const uint kObjectCaseId = 4099;

const uint kCaseResourceId = 6000;
const uint kFirstButtonResourceId = 6001;
const uint kExitCaseResourceId = 6009;

enum {
	kChapayevButton = 0,
	kPanelButton,
	kMapButton,
	kCloseButton,
	kNextPageButton,
	kPrevPageButton,
	kInvalidButton
};

QObjectCase::QObjectCase() {
	_id = kObjectCaseId;
	_resourceId = kCaseResourceId;

	_z = kCaseZ;

	_itemIndex = 0;
	_clickedObjIndex = kInvalidButton;

	_isShown = false;
	_updateZ = false;

	_itemsLocation[kChapayevButton] = Common::Point(120, 145);
	_itemsLocation[kPanelButton] = Common::Point(240, 145);
	_itemsLocation[kMapButton] = Common::Point(360, 145);
	_itemsLocation[kCloseButton] = Common::Point(100, 220);
	_itemsLocation[kNextPageButton] = Common::Point(240, 220);
	_itemsLocation[kPrevPageButton] = Common::Point(380, 220);
}

void QObjectCase::update(int time) {
	if (!_isShown || _clickedObjIndex == kInvalidButton)
		return;
	_time += time;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(kFirstButtonResourceId + _clickedObjIndex);
	if (flc) {
		while (_time >= flc->getDelay()) {
			flc->setFrame(-1);
			_time -= flc->getDelay();
			g_vm->videoSystem()->addDirtyRectFromMsk(Common::Point(0, 0), *flc);
		}
	}
}

void QObjectCase::draw() {
	if (!_isShown)
		return;
	QObject::draw();
	if (_clickedObjIndex != kInvalidButton) {
		FlicDecoder *flc = g_vm->resMgr()->loadFlic(kFirstButtonResourceId + _clickedObjIndex);
		Graphics::Surface *s = flc->getCurrentFrame()->convertTo(g_system->getScreenFormat(), flc->getPalette());

		const Common::List<Common::Rect> &dirty = g_vm->videoSystem()->rects();
		const Common::Array<Common::Rect> &mskRects = flc->getMskRects();

		for (Common::List<Common::Rect>::const_iterator it = dirty.begin(); it != dirty.end(); ++it) {
			for (uint i = 0; i < mskRects.size(); ++i) {
				Common::Rect rect = mskRects[i].findIntersectingRect(*it);
				debug("%d %d", rect.width(), rect.height());
				g_vm->videoSystem()->screen().transBlitFrom(*s, rect, rect, flc->getTransColor(s->format));
			}
		}

		s->free();
		delete s;
	}
}

void QObjectCase::show(bool v) {
	_x = 0; // sys->xOffset
	QObject::show(v);
	if (v) {
		addItemObjects();

		QMessageObject *obj = g_vm->getQSystem()->findObject(kPoloska);
		obj->_z = kPoloskaZ;
		obj->_x = 0; // sys->xOffset

		g_vm->getQSystem()->_mainInterface->_objs.push_back(obj);
	} else {
		removeObjects(true);
		g_vm->getQSystem()->_currInterface->_startIndex = 0;
	}
}

bool QObjectCase::isInPoint(int x, int y) {
	return _isShown;
}

void QObjectCase::onMouseMove(int x, int y) {
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(kExitCaseResourceId);
	if (*(const byte *)flc->getCurrentFrame()->getBasePtr(x - _x, y - _y) != 0) {
		if (_clickedObjIndex != kCloseButton && _clickedObjIndex != kInvalidButton) {
			flc = g_vm->resMgr()->loadFlic(kFirstButtonResourceId + _clickedObjIndex);
			flc->setFrame(1);
			g_vm->videoSystem()->addDirtyRectFromMsk(Common::Point(0, 0), *flc);
		}
		_clickedObjIndex = kCloseButton;
	} else {
		uint i;
		for (i = 0; i < kButtonsCount; ++i) {
			flc = g_vm->resMgr()->loadFlic(kFirstButtonResourceId + i);
			if (flc->getMskRects()[0].contains(Common::Point(x - _x, y))) {
				break;
			}
		}

		if (_clickedObjIndex != i && _clickedObjIndex != kInvalidButton) {
			flc = g_vm->resMgr()->loadFlic(kFirstButtonResourceId + _clickedObjIndex);
			flc->setFrame(1);
			g_vm->videoSystem()->addDirtyRectFromMsk(Common::Point(0, 0), *flc);
		}

		if (i == kButtonsCount && _clickedObjIndex != kInvalidButton) {
			_clickedObjIndex = kInvalidButton;
		} else if (i != _clickedObjIndex) {
			if ((i != kChapayevButton || g_vm->getQSystem()->_chapayev->_isShown) && (i != kMapButton || g_vm->getQSystem()->_room->_showMap)) {
				flc = g_vm->resMgr()->loadFlic(kFirstButtonResourceId + i);
				g_vm->videoSystem()->addDirtyRectFromMsk(Common::Point(0, 0), *flc);
				_clickedObjIndex = i;
			} else {
				_clickedObjIndex = kInvalidButton;
			}
		}
	}
}

void QObjectCase::onClick(int x, int y) {
	switch (_clickedObjIndex) {
		case kChapayevButton:
			g_vm->getQSystem()->setChapayev();
			break;
		case kPanelButton:
			g_vm->getQSystem()->togglePanelInterface();
			break;
		case kMapButton:
			g_vm->getQSystem()->toggleMapInterface();
			break;
		case kCloseButton:
			show(false);
			break;
		case kNextPageButton:
			nextPage();
			break;
		case kPrevPageButton:
			prevPage();
			break;
		default:
			break;
	}
}

void QObjectCase::addItemObjects() {
	Common::Array<QVisibleObject *> &objs = g_vm->getQSystem()->_mainInterface->_objs;
	removeObjects(false);

	for (uint i = 0; i < objs.size(); ++i) {
		if (objs[i]->_resourceId == kCaseResourceId) {
			g_vm->getQSystem()->_currInterface->_startIndex = i;
		}
	}

	const uint size = (_itemIndex + kItemsOnPage >= _items.size()) ? _items.size() : (_itemIndex + kItemsOnPage);
	for (uint i = _itemIndex; i < size; ++i) {
		QMessageObject *obj = g_vm->getQSystem()->findObject(_items[i]);
		obj->_z = kItemZ;
		objs.push_back(obj);
		g_vm->resMgr()->loadFlic(obj->_resourceId);
		_x = _itemsLocation[i - _itemIndex].x;
		_y = _itemsLocation[i - _itemIndex].y;
	}
}

void QObjectCase::addItem(uint16 id) {
	_items.push_back(id);
	reshow();
}

void QObjectCase::removeItem(uint16 id) {
	for (uint i = 0; i < _items.size(); ++i) {
		if (_items[i] == id) {
			_items.remove_at(i);
		}
	}

	_itemIndex = (_items.size() < kItemsOnPage) ? 0 : (_items.size() - kItemsOnPage);

	reshow();
}

void QObjectCase::transformItem(uint16 oldItem, uint16 newItem) {
	for (uint i = 0; i < _items.size(); ++i) {
		if (_items[i] == oldItem) {
			_items[i] = newItem;
		}
	}

	reshow();
}

void QObjectCase::nextPage() {
	if (_items.size() > _itemIndex + kItemsOnPage) {
		_itemIndex += kItemsOnPage;
		addItemObjects();
		g_vm->videoSystem()->makeAllDirty();
	}
}

void QObjectCase::prevPage() {
	if (_itemIndex > 0) {
		_itemIndex = (_itemIndex <= kItemsOnPage) ? 0 : _itemIndex - kItemsOnPage;
		addItemObjects();
		g_vm->videoSystem()->makeAllDirty();
	}
}

void QObjectCase::removeObjects(bool removePoloska) {
	Common::Array<QVisibleObject *> &objs = g_vm->getQSystem()->_mainInterface->_objs;
	for (uint i = 0; i < objs.size();) {
		if (objs[i]->_z == kItemZ || (removePoloska && objs[i]->_z == kPoloskaZ)) {
			objs.remove_at(i);
		} else {
			++i;
		}
	}
}

void QObjectCase::reshow() {
	if (_isShown) {
		show(false);
		show(true);
	}
}

}
