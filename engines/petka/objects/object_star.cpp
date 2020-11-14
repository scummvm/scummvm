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

#include "common/rect.h"

#include "petka/objects/object_case.h"
#include "petka/objects/object_cursor.h"
#include "petka/objects/object_star.h"
#include "petka/q_system.h"
#include "petka/flc.h"
#include "petka/q_manager.h"
#include "petka/video.h"
#include "petka/petka.h"

namespace Petka {

//const uint kFirstCursorId = 5001;
const uint kCaseButtonIndex = 0;

QObjectStar::QObjectStar() {
	_isShown = false;
	_id = 4098;
	_resourceId = 5000;
	_z = 999;
	_updateZ = false;
	_isActive = true;

	_buttonRects[0] = Common::Rect(70, 74, 112, 112);
	_buttonRects[1] = Common::Rect(68, 0, 114, 41);
	_buttonRects[2] = Common::Rect(151, 51, 180, 97);
	_buttonRects[3] = Common::Rect(138, 125, 179, 166);
	_buttonRects[4] = Common::Rect(55, 145, 96, 175);
	_buttonRects[5] = Common::Rect(11, 79, 40, 118);
}

bool QObjectStar::isInPoint(Common::Point p) {
	return _isShown;
}

void QObjectStar::onMouseMove(Common::Point p) {
	uint frame = (findButtonIndex(p.x - _x, p.y - _y) + 1) % 7 + 1;
	FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
	if (flc && flc->getCurFrame() + 1 != (int32)frame) {
		g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
		flc->setFrame(frame);
	}
}

void QObjectStar::onClick(Common::Point p) {
	uint button = findButtonIndex(p.x - _x, p.y - _y);
	if (button == kCaseButtonIndex) {
		g_vm->getQSystem()->getCase()->show(true);
	} else if (button < ARRAYSIZE(_buttonRects)) {
		QObjectCursor *cursor = g_vm->getQSystem()->getCursor();
		cursor->setAction(button - 1);
	}
	show(false);
}

uint QObjectStar::findButtonIndex(int16 x, int16 y) const {
	uint i = 0;
	for (i = 0; i < ARRAYSIZE(_buttonRects); ++i) {
		if (_buttonRects[i].contains(x, y))
			return i;
	}
	return i;
}

void QObjectStar::setPos(Common::Point p, bool) {
	if (!_isShown) {
		QSystem *sys = g_vm->getQSystem();

		FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
		p.x = MAX<int16>(p.x - sys->_xOffset - flc->getWidth() / 2, 0);
		p.y = MAX<int16>(p.y - flc->getHeight() / 2, 0);

		_x = MIN<int16>(p.x, 639 - flc->getWidth()) + sys->_xOffset;
		_y = MIN<int16>(p.y, 479 - flc->getHeight());
	}
}

}
