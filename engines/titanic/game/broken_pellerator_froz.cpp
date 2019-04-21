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

#include "titanic/game/broken_pellerator_froz.h"
#include "titanic/core/view_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBrokenPelleratorFroz, CBrokenPellBase)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CBrokenPelleratorFroz::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_exitLeft, indent);
	file->writeQuotedLine(_exitRight, indent);
	file->writeQuotedLine(_closeUpWithoutHose, indent);
	file->writeQuotedLine(_closeUpWithHose, indent);

	CBrokenPellBase::save(file, indent);
}

void CBrokenPelleratorFroz::load(SimpleFile *file) {
	file->readNumber();
	_exitLeft = file->readString();
	_exitRight = file->readString();
	_closeUpWithoutHose = file->readString();
	_closeUpWithHose = file->readString();

	CBrokenPellBase::load(file);
}

bool CBrokenPelleratorFroz::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_pelleratorOpen) {
		changeView(_gottenHose ? _closeUpWithHose : _closeUpWithoutHose);
	} else {
		_pelleratorOpen = true;
		if (_gottenHose) {
			playMovie(0, 13, 0);
		} else {
			playMovie(43, 55, MOVIE_NOTIFY_OBJECT);
		}
	}

	return true;
}

bool CBrokenPelleratorFroz::LeaveViewMsg(CLeaveViewMsg *msg) {
	CString name = msg->_newView->getNodeViewName();

	if (name == "Node 3.S" || name == "Node 3.E") {
		_pelleratorOpen = false;
		loadFrame(0);
	}

	return true;
}

bool CBrokenPelleratorFroz::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayerGetsHose") {
		_gottenHose = true;
		loadFrame(29);
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("FPickUpHose");
	} else {
		_closeAction = CLOSE_NONE;
		bool closeFlag = msg->_action == "Close";
		if (msg->_action == "CloseLeft") {
			closeFlag = true;
			_closeAction = CLOSE_LEFT;
		}
		if (msg->_action == "CloseRight") {
			closeFlag = true;
			_closeAction = CLOSE_RIGHT;
		}

		if (closeFlag) {
			if (_pelleratorOpen) {
				_pelleratorOpen = false;
				if (_gottenHose)
					playMovie(29, 42, MOVIE_NOTIFY_OBJECT);
				else
					playMovie(72, 84, MOVIE_NOTIFY_OBJECT);
			} else {
				switch (_closeAction) {
				case CLOSE_LEFT:
					changeView(_exitLeft);
					break;
				case CLOSE_RIGHT:
					changeView(_exitRight);
					break;
				default:
					break;
				}

				_closeAction = CLOSE_NONE;
			}
		}
	}

	return true;
}

bool CBrokenPelleratorFroz::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == 55) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 1;
		statusMsg.execute("FPickUpHose");
	}

	if (msg->_endFrame == 84) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("FPickUpHose");
	}

	if (_closeAction == CLOSE_LEFT) {
		changeView(_exitLeft);
		_closeAction = CLOSE_NONE;
	} else if (_closeAction == CLOSE_RIGHT) {
		changeView(_exitRight);
		_closeAction = CLOSE_NONE;
	}

	return true;
}

} // End of namespace Titanic
