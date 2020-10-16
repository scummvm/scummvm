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
#include "petka/objects/object_cursor.h"
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

	_itemsLocation[0] = Common::Point(120, 145);
	_itemsLocation[1] = Common::Point(240, 145);
	_itemsLocation[2] = Common::Point(360, 145);
	_itemsLocation[3] = Common::Point(100, 220);
	_itemsLocation[4] = Common::Point(240, 220);
	_itemsLocation[5] = Common::Point(380, 220);
}

void QObjectCase::update(int time) {
	if (!_isShown || _clickedObjIndex == kInvalidButton)
		return;
	_time += time;
	FlicDecoder *flc = g_vm->resMgr()->getFlic(kFirstButtonResourceId + _clickedObjIndex);
	if (flc) {
		while (_time >= (int32)flc->getDelay()) {
			flc->setFrame(-1);
			_time -= flc->getDelay();
			g_vm->videoSystem()->addDirtyMskRects(*flc);
		}
	}
}

void QObjectCase::draw() {
	if (!_isShown)
		return;
	QObject::draw();
	if (_clickedObjIndex != kInvalidButton) {
		FlicDecoder *flc = g_vm->resMgr()->getFlic(kFirstButtonResourceId + _clickedObjIndex);
		Graphics::Surface *s = flc->getCurrentFrame()->convertTo(g_system->getScreenFormat(), flc->getPalette());

		QSystem *sys = g_vm->getQSystem();

		const Common::List<Common::Rect> &dirty = g_vm->videoSystem()->rects();
		const Common::Array<Common::Rect> &mskRects = flc->getMskRects();

		for (Common::List<Common::Rect>::const_iterator it = dirty.begin(); it != dirty.end(); ++it) {
			for (uint i = 0; i < mskRects.size(); ++i) {
				Common::Rect destRect = mskRects[i].findIntersectingRect(*it);
				Common::Rect srcRect = destRect;
				srcRect.translate(-_x + sys->_xOffset, -_y);
				g_vm->videoSystem()->transBlitFrom(*s, srcRect, destRect, flc->getTransColor(s->format));
			}
		}

		s->free();
		delete s;
	}
}

void QObjectCase::show(bool v) {
	QSystem *sys = g_vm->getQSystem();
	_x = sys->_xOffset;
	QObject::show(v);
	if (v) {
		addItemObjects();

		QMessageObject *obj = sys->findObject(kPoloska);
		obj->_z = kPoloskaZ;
		obj->_x = sys->_xOffset;

		sys->_mainInterface->_objs.push_back(obj);
	} else {
		removeObjects(true);
		sys->_currInterface->_startIndex = 0;
	}
}

bool QObjectCase::isInPoint(Common::Point p) {
	return _isShown;
}

void QObjectCase::onMouseMove(Common::Point p) {
	p.x -= _x;
	FlicDecoder *flc = g_vm->resMgr()->getFlic(kExitCaseResourceId);
	if (*(const byte *)flc->getCurrentFrame()->getBasePtr(p.x, p.y) != 0) {
		if (_clickedObjIndex != kCloseButton && _clickedObjIndex != kInvalidButton) {
			flc = g_vm->resMgr()->getFlic(kFirstButtonResourceId + _clickedObjIndex);
			flc->setFrame(1);
			g_vm->videoSystem()->addDirtyMskRects(*flc);
		}
		_clickedObjIndex = kCloseButton;
	} else {
		uint i;
		for (i = 0; i < kButtonsCount; ++i) {
			flc = g_vm->resMgr()->getFlic(kFirstButtonResourceId + i);
			if (flc->getMskRects()[0].contains(p)) {
				break;
			}
		}

		if (_clickedObjIndex != i && _clickedObjIndex != kInvalidButton) {
			flc = g_vm->resMgr()->getFlic(kFirstButtonResourceId + _clickedObjIndex);
			flc->setFrame(1);
			g_vm->videoSystem()->addDirtyMskRects(*flc);
		}

		if (i == kButtonsCount && _clickedObjIndex != kInvalidButton) {
			_clickedObjIndex = kInvalidButton;
		} else if (i != _clickedObjIndex) {
			if ((i != kChapayevButton || g_vm->getQSystem()->getChapay()->_isShown) && (i != kMapButton || g_vm->getQSystem()->_room->_showMap)) {
				flc = g_vm->resMgr()->getFlic(kFirstButtonResourceId + i);
				g_vm->videoSystem()->addDirtyMskRects(*flc);
				_clickedObjIndex = i;
			} else {
				_clickedObjIndex = kInvalidButton;
			}
		}
	}
}

void QObjectCase::onClick(Common::Point p) {
	switch (_clickedObjIndex) {
		case kChapayevButton:
			g_vm->getQSystem()->setCursorAction(kActionObjUseChapayev);
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
	QSystem *sys = g_vm->getQSystem();
	Common::Array<QVisibleObject *> &objs = sys->_mainInterface->_objs;
	removeObjects(false);

	for (uint i = 0; i < objs.size(); ++i) {
		if (objs[i]->_resourceId == kCaseResourceId) {
			sys->_currInterface->_startIndex = i;
		}
	}

	const uint size = (_itemIndex + kItemsOnPage >= _items.size()) ? _items.size() : (_itemIndex + kItemsOnPage);
	for (uint i = _itemIndex; i < size; ++i) {
		QMessageObject *obj = sys->findObject(_items[i]);
		obj->_x = sys->_xOffset + _itemsLocation[i - _itemIndex].x;
		obj->_y = _itemsLocation[i - _itemIndex].y;
		obj->_z = kItemZ;
		g_vm->resMgr()->getFlic(obj->_resourceId);
		objs.push_back(obj);
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

} // End of namespace Petka
