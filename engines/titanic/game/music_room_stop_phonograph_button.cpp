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

#include "titanic/game/music_room_stop_phonograph_button.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMusicRoomStopPhonographButton, CEjectPhonographButton)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

void CMusicRoomStopPhonographButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_ticks, indent);
	CEjectPhonographButton::save(file, indent);
}

void CMusicRoomStopPhonographButton::load(SimpleFile *file) {
	file->readNumber();
	_ticks = file->readNumber();
	CEjectPhonographButton::load(file);
}

bool CMusicRoomStopPhonographButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_ejected) {
		loadFrame(1);
		playSound(_soundName);
		_readyFlag = true;

		CPhonographStopMsg stopMsg;
		stopMsg.execute(getParent(), nullptr, MSGFLAG_SCAN);
		if (stopMsg._cylinderPresent) {
			_ticks = getTicksCount();
		} else {
			CEjectCylinderMsg ejectMsg;
			ejectMsg.execute(getParent(), nullptr, MSGFLAG_SCAN);
			_ejected = true;
		}
	}

	return true;
}

bool CMusicRoomStopPhonographButton::FrameMsg(CFrameMsg *msg) {
	if (_readyFlag && _ticks && msg->_ticks >= (_ticks + 100)) {
		loadFrame(0);
		playSound(_readySoundName);
		_ticks = 0;
		_readyFlag = false;
	}

	return true;
}

} // End of namespace Titanic
