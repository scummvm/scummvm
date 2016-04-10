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

#include "titanic/core/room_item.h"
#include "titanic/game/cdrom_tray.h"
#include "titanic/messages/messages.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCDROMTray, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(StatusChangeMsg)
END_MESSAGE_MAP()


CCDROMTray::CCDROMTray() : CGameObject(), _state(0) {
}

void CCDROMTray::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_state, indent);
	file->writeQuotedLine(_insertedCD, indent);

	CGameObject::save(file, indent);
}

void CCDROMTray::load(SimpleFile *file) {
	file->readNumber();
	_state = file->readNumber();
	_insertedCD = file->readString();

	CGameObject::load(file);
}

bool CCDROMTray::ActMsg(CActMsg *msg) {
	if (msg->_action == "ClickedOn") {
		if (_state) {
			if (_insertedCD == "None") {
				fn1(55, 65, 0);
				playSound("a#35.wav", 50, 0, 0);
				_state = 0;
			} else {
				CTreeItem *treeItem = getRoom()->findByName(_insertedCD);
				if (treeItem) {
					CActMsg actMsg("Ejected");
					actMsg.execute(treeItem);
				}

				_insertedCD = "None";
				loadFrame(52);
			}
		} else if (_insertedCD == "None") {
			fn1(44, 54, 0);
			playSound("a#34.wav", 50, 0, 0);
			_state = 1;
		} else if (_insertedCD == "newCD1" || _insertedCD == "newCD2") {
			fn1(22, 32, 0);
			playSound("a#34.wav", 50, 0, 0);
			_state = 1;
		} else if (_insertedCD == "newSTCD") {
			fn1(0, 10, 0);
			playSound("a#34.wav", 50, 0, 0);
			_state = 1;
		}
	} else if (_state) {
		if (msg->_action == "newCD1" || msg->_action == "newCD2") {
			fn1(33, 43, 4);
			playSound("a#35.wav", 50, 0, 0);
		} else if (msg->_action == "newSTCD") {
			fn1(11, 21, 4);
			playSound("a#35.wav", 50, 0, 0);
		} else {
			return true;
		}

		_insertedCD = msg->_action;
		_state = 0;
	}

	return true;
}

bool CCDROMTray::MovieEndMsg(CMovieEndMsg *msg) {
	CTreeItem *treeItem = getRoom()->findByName("newScreen");
	
	if (treeItem) {
		CActMsg actMsg(_insertedCD);
		actMsg.execute(treeItem);
	}

	return true;
}

bool CCDROMTray::StatusChangeMsg(CStatusChangeMsg *msg) {
	msg->_success = _state;
	return true;
}

} // End of namespace Titanic
