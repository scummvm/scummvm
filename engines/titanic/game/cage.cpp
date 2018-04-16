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

#include "titanic/game/cage.h"
#include "titanic/npcs/parrot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCage, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(PreEnterViewMsg)
	ON_MESSAGE(MouseMoveMsg)
END_MESSAGE_MAP()

int CCage::_v1;
bool CCage::_open;

void CCage::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_open, indent);

	CBackground::save(file, indent);
}

void CCage::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_open = file->readNumber();

	CBackground::load(file);
}

bool CCage::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (CParrot::_state != PARROT_IN_CAGE && !CParrot::_coreReplaced) {
		CActMsg actMsg(_open ? "Open" : "Shut");
		actMsg.execute(this);
	}

	return true;
}

bool CCage::ActMsg(CActMsg *msg) {
	if (msg->_action == "Shut") {
		if (!_open) {
			playClip("Shut", MOVIE_STOP_PREVIOUS | MOVIE_NOTIFY_OBJECT);
			disableMouse();
		}
	} else if (msg->_action == "Open") {
		if (_open) {
			playClip("Open", MOVIE_STOP_PREVIOUS | MOVIE_NOTIFY_OBJECT);
			disableMouse();
		}
	} else if (msg->_action == "CoreReplaced") {
		CActMsg actMsg("Shut");
		actMsg.execute(this);
	} else if (msg->_action == "OpenNow") {
		loadFrame(0);
		_open = false;
	}

	return true;
}

bool CCage::MovieEndMsg(CMovieEndMsg *msg) {
	enableMouse();
	_open = clipExistsByEnd("Shut", msg->_endFrame);

	CStatusChangeMsg statusMsg;
	statusMsg._newStatus = _open ? 1 : (CParrot::_state == PARROT_IN_CAGE ? 1 : 0);
	statusMsg.execute("PerchCoreHolder");

	return true;
}

bool CCage::PreEnterViewMsg(CPreEnterViewMsg *msg) {
	loadSurface();
	_open = CParrot::_state != PARROT_IN_CAGE;
	loadFrame(_open ? 8 : 0);

	return true;
}

bool CCage::MouseMoveMsg(CMouseMoveMsg *msg) {
	_cursorId = CParrot::_state != PARROT_IN_CAGE && !CParrot::_coreReplaced ? CURSOR_ACTIVATE : CURSOR_ARROW;
	return true;
}

} // End of namespace Titanic
