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

#include "titanic/game/broken_pellerator.h"
#include "titanic/core/view_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBrokenPellerator, CBrokenPellBase)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CBrokenPellerator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeQuotedLine(_string4, indent);
	file->writeQuotedLine(_string5, indent);

	CBrokenPellBase::save(file, indent);
}

void CBrokenPellerator::load(SimpleFile *file) {
	file->readNumber();
	_string2 = file->readString();
	_string3 = file->readString();
	_string4 = file->readString();
	_string5 = file->readString();

	CBrokenPellBase::load(file);
}

bool CBrokenPellerator::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_v1) {
		changeView(_v2 ? _string5 : _string4);
	} else {
		if (_v2) {
			playMovie(28, 43, 0);
		} else {
			playMovie(0, 14, MOVIE_NOTIFY_OBJECT);
		}

		_v1 = true;
	}

	return true;
}

bool CBrokenPellerator::LeaveViewMsg(CLeaveViewMsg *msg) {
	CString name = msg->_newView->getNodeViewName();
	if (name == "Node 3.S" || name == "Node 3.N") {
		_v1 = false;
		loadFrame(0);
	}

	return true;
}

bool CBrokenPellerator::ActMsg(CActMsg *msg) {
	if (msg->_action == "PlayerGetsHose") {
		_v2 = 1;
		loadFrame(43);

		CStatusChangeMsg statusMsg;
		statusMsg.execute("PickupHose");
	} else {
		_fieldE0 = 0;
		bool closeFlag = msg->_action == "Close";
		if (msg->_action == "CloseLeft") {
			closeFlag = true;
			_fieldE0 = 1;
		}
		if (msg->_action == "CloseRight") {
			closeFlag = true;
			_fieldE0 = 2;
		}

		if (closeFlag) {
			if (_v1) {
				_v1 = false;
				if (_v2)
					playMovie(43, 57, MOVIE_NOTIFY_OBJECT);
				else
					playMovie(14, 28, MOVIE_NOTIFY_OBJECT);
			} else {
				switch (_fieldE0) {
				case 1:
					changeView(_string2);
					break;
				case 2:
					changeView(_string3);
					break;
				default:
					break;
				}

				_fieldE0 = 0;
			}
		}
	}

	return true;
}

bool CBrokenPellerator::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == 14) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 1;
		statusMsg.execute("PickUpHose");
	}

	if (msg->_endFrame == 28) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = 0;
		statusMsg.execute("PickUpHose");
	}

	switch (_fieldE0) {
	case 1:
		changeView(_string2);
		break;
	case 2:
		changeView(_string3);
		break;
	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
