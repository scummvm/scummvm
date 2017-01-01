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

#include "titanic/messages/bilge_dispensor_event.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBilgeDispensorEvent, CAutoSoundEvent)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(StatusChangeMsg)
END_MESSAGE_MAP()

void CBilgeDispensorEvent::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CAutoSoundEvent::save(file, indent);
}

void CBilgeDispensorEvent::load(SimpleFile *file) {
	file->readNumber();
	CAutoSoundEvent::load(file);
}

bool CBilgeDispensorEvent::EnterRoomMsg(CEnterRoomMsg *msg) {
	_value1 = 0;
	return true;
}

bool CBilgeDispensorEvent::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	_value1 = -1;
	return true;
}

bool CBilgeDispensorEvent::FrameMsg(CFrameMsg *msg) {
	if (_value1 >= 0 && (_value1 & 0xffff) == 0x4000) {
		int volume = 20 + getRandomNumber(30);
		int val3 = getRandomNumber(20) - 10;

		if (getRandomNumber(2) == 0) {
			playSound("b#18.wav", volume, val3);
		}
	}

	CAutoSoundEvent::FrameMsg(msg);
	return true;
}

bool CBilgeDispensorEvent::StatusChangeMsg(CStatusChangeMsg *msg) {
	if (msg->_newStatus == 1)
		_value1 = -1;
	else if (msg->_newStatus == 2)
		_value1 = 0;

	return true;
}

} // End of namespace Titanic
