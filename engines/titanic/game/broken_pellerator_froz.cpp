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
	file->writeQuotedLine(_string2, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeQuotedLine(_string4, indent);
	file->writeQuotedLine(_string5, indent);

	CBrokenPellBase::save(file, indent);
}

void CBrokenPelleratorFroz::load(SimpleFile *file) {
	file->readNumber();
	_string2 = file->readString();
	_string3 = file->readString();
	_string4 = file->readString();
	_string5 = file->readString();

	CBrokenPellBase::load(file);
}

bool CBrokenPelleratorFroz::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_v1) {
		changeView(_v2 ? _string5 : _string4);
	} else {
		_v1 = true;
		if (_v2) {
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
		_v1 = false;
		loadFrame(0);
	}

	return true;
}

bool CBrokenPelleratorFroz::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayerGetsHose") {
		_v2 = 1;
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("FPickUpHose");
	} else {
		_exitAction = 0;
		bool closeFlag = msg->_action == "Close";
		if (msg->_action == "CloseLeft") {
			closeFlag = true;
			_exitAction = 1;
		}
		if (msg->_action == "CloseRight") {
			closeFlag = true;
			_exitAction = 2;
		}

		if (closeFlag) {
			if (_v1) {
				_v1 = false;
				if (_v2)
					playMovie(29, 42, MOVIE_NOTIFY_OBJECT);
				else
					playMovie(72, 84, MOVIE_NOTIFY_OBJECT);
			} else {
				switch (_exitAction) {
				case 1:
					changeView(_string2);
					break;
				case 2:
					changeView(_string3);
					break;
				default:
					break;
				}

				_exitAction = 0;
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

	if (_exitAction == 1) {
		changeView(_string2);
		_exitAction = 0;
	} else if (_exitAction == 2) {
		changeView(_string3);
		_exitAction = 0;
	}

	return true;
}

} // End of namespace Titanic
