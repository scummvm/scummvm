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

#include "petka/objects/text.h"
#include "petka/interfaces/interface.h"
#include "petka/q_system.h"
#include "petka/petka.h"
#include "petka/objects/object_cursor.h"
#include "petka/q_manager.h"
#include "petka/video.h"

namespace Petka {

Interface::Interface()
	: _objUnderCursor(nullptr), _startIndex(0) {}

void Interface::stop() {
	setText(Common::U32String(), 0, 0);
	g_vm->videoSystem()->makeAllDirty();
}

void Interface::setText(const Common::U32String &text, uint16 textColor, uint16 outlineColor) {
	removeTexts();
	if (!text.empty())
		_objs.push_back(new QText(text, textColor, outlineColor));
}

void Interface::setTextPhrase(const Common::U32String &text, uint16 textColor, uint16 outlineColor) {
	removeTexts();
	_objUnderCursor = nullptr;
	_objs.push_back(new QTextPhrase(text, textColor, outlineColor));
}

QVisibleObject *Interface::findObject(int resourceId) {
	for (uint i = 0; i < _objs.size(); ++i) {
		if (_objs[i]->_resourceId == resourceId) {
			return _objs[i];
		}
	}
	return nullptr;
}

void Interface::initCursor(int id, bool show, bool animate) {
	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
	_objs.push_back(cursor);
	cursor->_resourceId = id;
	cursor->_isShown = show;
	cursor->_animate = animate;
	cursor->_actionType = kActionLook;
	cursor->setPos(Common::Point(cursor->_x, cursor->_y), false);
}

void Interface::removeTexts() {
	for (uint i = 0; i < _objs.size();) {
		if (_objs[i]->_resourceId == -2) {
			g_vm->videoSystem()->addDirtyRect(((QText *)_objs[i])->getRect());
			g_vm->resMgr()->removeResource(-2);
			delete _objs[i];
			_objs.remove_at(i);
		} else {
			++i;
		}
	}
}

void Interface::update(uint time) {
	for (uint i = _startIndex; i < _objs.size(); ++i) {
		_objs[i]->update(time);
	}

	for (uint i = 0; i < _objs.size(); ++i) {
		_objs[i]->updateZ();
	}
	sort();
}

void Interface::draw() {
	for (uint i = 0; i < _objs.size(); ++i) {
		_objs[i]->draw();
	}
}

void Interface::sort() {
	for (uint i = 0; i < _objs.size() - 1; ++i) {
		uint minIndex = i;
		for (uint j = i + 1; j < _objs.size(); ++j) {
			if (_objs[j]->_z < _objs[minIndex]->_z) {
				minIndex = j;
			}
		}

		if (i != minIndex) {
			SWAP(_objs[i], _objs[minIndex]);
		}
	}
}

void SubInterface::start(int id) {
	QSystem *sys = g_vm->getQSystem();
	QObjectCursor *cursor = sys->getCursor();

	_savedCursorId = cursor->_resourceId;
	_savedCursorType = cursor->_actionType;

	initCursor(4901, true, false);

	_savedXOffset = sys->_xOffset;
	_savedSceneWidth = sys->_sceneWidth;

	sys->_xOffset = 0;
	sys->_sceneWidth = 640;

	g_vm->getQSystem()->_currInterface = this;
	g_vm->videoSystem()->updateTime();
	g_vm->videoSystem()->makeAllDirty();
}

void SubInterface::stop() {
	QSystem *sys = g_vm->getQSystem();
	QObjectCursor *cursor = sys->getCursor();

	sys->_xOffset = _savedXOffset;
	sys->_sceneWidth = _savedSceneWidth;

	cursor->_resourceId = _savedCursorId;
	cursor->_actionType = _savedCursorType;

	sys->_currInterface = g_vm->getQSystem()->_prevInterface;
	sys->_currInterface->onMouseMove(Common::Point(cursor->_x, cursor->_y));

	_objs.clear();

	Interface::stop();
}

} // End of namespace Petka
