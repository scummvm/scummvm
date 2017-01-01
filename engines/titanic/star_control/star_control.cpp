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

#include "titanic/support/screen_manager.h"
#include "titanic/star_control/star_control.h"
#include "titanic/star_control/dmatrix.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/star_control_sub6.h"
#include "titanic/star_control/star_control_sub12.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CStarControl, CGameObject)
	ON_MESSAGE(MouseMoveMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(KeyCharMsg)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

CStarControl::CStarControl() : _fieldBC(0), _field80B0(0),
		_starRect(20, 10, 620, 350) {
	CStarControlSub6::init();
	CStarControlSub12::init();
	DMatrix::init();
}

CStarControl::~CStarControl() {
	CStarControlSub6::deinit();
	CStarControlSub12::deinit();
	DMatrix::deinit();
}

void CStarControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(0, indent);
	_starField.save(file, indent);
	_view.save(file, indent);
	CGameObject::save(file, indent);
}

void CStarControl::load(SimpleFile *file) {
	int val = file->readNumber();

	if (!val) {
		_starField.load(file);
		if (!_starField.initDocument())
			error("Couldn't initialise the StarField document");

		_view.load(file, 0);
		CScreenManager *screenManager = CScreenManager::setCurrent();
		if (!screenManager)
			error("There's no screen  manager during loading");

		_view.setup(screenManager, &_starField, this);
		_view.reset();

		_fieldBC = 1;
	}

	CGameObject::load(file);
}

void CStarControl::draw(CScreenManager *screenManager) {
	if (_visible)
		_view.draw(screenManager);
}

bool CStarControl::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_visible && _starRect.contains(msg->_mousePos)) {
		_view.MouseButtonDownMsg(0, Point(msg->_mousePos.x - 20,
			msg->_mousePos.y - 10));
		return true;
	} else {
		return false;
	}
}

bool CStarControl::MouseMoveMsg(CMouseMoveMsg *msg) {
	if (_visible && _starRect.contains(msg->_mousePos)) {
		_view.MouseMoveMsg(0, Point(msg->_mousePos.x - 20,
			msg->_mousePos.y - 10));
		makeDirty();
		return true;
	} else {
		return false;
	}
}

bool CStarControl::KeyCharMsg(CKeyCharMsg *msg) {
	if (_visible)
		_view.KeyCharMsg(msg->_key);

	return false;
}

bool CStarControl::FrameMsg(CFrameMsg *msg) {
	if (_visible) {
		Point pt = getMousePos();
		if (_starRect.contains(pt))
			_view.MouseMoveMsg(0, pt);

		newFrame();
		makeDirty();
		return true;
	} else {
		return false;
	}
}

void CStarControl::newFrame() {
	// TODO
}

void CStarControl::fn1(int action) {
	// TODO
}

bool CStarControl::fn4() {
	return _starField.get6();
}

bool CStarControl::canSetStarDestination() const {
	return _view.canSetStarDestination();
}

void CStarControl::starDestinationSet() {
	_view.starDestinationSet();
}

} // End of namespace Titanic
