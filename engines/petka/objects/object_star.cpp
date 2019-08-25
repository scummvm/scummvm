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

const uint kFirstCursorId = 5001;
const uint kCaseButtonIndex = 0;
const Common::Rect kButtonsRects[] = {Common::Rect(70, 74, 112, 112),
									  Common::Rect(68, 0, 114, 41),
									  Common::Rect(151, 51, 180, 97),
									  Common::Rect(138, 125, 179, 166),
									  Common::Rect(55, 145, 96, 175),
									  Common::Rect(11, 79, 40, 118)};

static uint findButtonIndex(int16 x, int16 y) {
	uint i = 0;
	for (i = 0; i < sizeof(kButtonsRects) / sizeof(Common::Rect); ++i) {
		if (kButtonsRects[i].contains(x, y))
			return i;
	}
	return i;
}

QObjectStar::QObjectStar() {
	_isShown = false;
	_id = 4098;
	_resourceId = 5000;
	_z = 999;
	_updateZ = false;
	_isActive = true;
}

bool QObjectStar::isInPoint(int x, int y) {
	return _isShown;
}

void QObjectStar::onMouseMove(int x, int y) {
	uint frame = (findButtonIndex(x - _x, y - _y) + 1) % 7 + 1;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (flc && flc->getCurFrame() + 1 != frame) {
		g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
		flc->setFrame(frame);
	}
}

void QObjectStar::onClick(int x, int y) {
	uint button = findButtonIndex(x - _x, y - _y);
	if (button >= sizeof(kButtonsRects) / sizeof(Common::Rect)) {
		show(0);
		return;
	}
	if (button == kCaseButtonIndex) {
		g_vm->getQSystem()->_case->show(1);
	} else {
		QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
		cursor->show(0);
		cursor->_resourceId = button + kFirstCursorId;
		cursor->_actionType = button - 1;
		cursor->show(1);
	}
	show(0);
}

}
