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

#include "titanic/game/announce.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CAnnounce, CGameObject)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

CAnnounce::CAnnounce() : _nameIndex(0), _soundHandle(0), _notActivatedFlag(true), _enabled(false) {
}

void CAnnounce::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_nameIndex, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_notActivatedFlag, indent);
	file->writeNumberLine(_enabled, indent);

	CGameObject::save(file, indent);
}

void CAnnounce::load(SimpleFile *file) {
	file->readNumber();
	_nameIndex = file->readNumber();
	_soundHandle = file->readNumber();
	_notActivatedFlag = file->readNumber();
	_enabled = file->readNumber();

	CGameObject::load(file);
}

bool CAnnounce::TimerMsg(CTimerMsg *msg) {
	if (!_enabled)
		return false;

	if (msg->_timerCtr == 1) {
		CString numStr = "0";
		CString waveNames1[20] = {
			"z#181.wav", "z#211.wav", "z#203.wav", "z#202.wav", "z#201.wav",
			"z#200.wav", "z#199.wav", "z#198.wav", "z#197.wav", "z#196.wav",
			"z#210.wav", "z#209.wav", "z#208.wav", "z#207.wav", "z#206.wav",
			"z#205.wav", "z#204.wav", "z#145.wav", "", ""
		};
		CString waveNames2[37] = {
			"z#154.wav", "z#153.wav", "z#152.wav", "z#151.wav", "z#150.wav",
			"z#149.wav", "z#148.wav", "z#169.wav", "z#171.wav", "z#178.wav",
			"z#176.wav", "z#177.wav", "z#165.wav", "z#170.wav", "z#180.wav",
			"z#156.wav", "z#172.wav", "z#173.wav", "z#160.wav", "z#158.wav",
			"z#161.wav", "z#179.wav", "z#163.wav", "z#164.wav", "z#162.wav",
			"z#159.wav", "z#175.wav", "z#166.wav", "z#174.wav", "z#157.wav",
			"", "", "", "", "", "", ""
		};

		int randVal = _nameIndex ? getRandomNumber(2) : 0;
		switch (randVal) {
		case 0:
		case 1:
			_soundHandle = playSound("z#189.wav");
			if (_nameIndex < 20) {
				queueSound(waveNames1[_nameIndex], _soundHandle);
				++_nameIndex;
			} else {
				queueSound(waveNames1[1 + getRandomNumber(17)], _soundHandle);
			}
			break;

		case 2:
			_soundHandle = playSound("z#189.wav");
			queueSound(waveNames2[1 + getRandomNumber(35)], _soundHandle);
			break;

		default:
			break;
		}

		// Schedule another announcement for a random future time
		addTimer(1, 300000 + getRandomNumber(30000), 0);
		if (getRandomNumber(3) == 0)
			addTimer(2, 4000, 0);

	} else if (msg->_timerCtr == 2) {
		CParrotSpeakMsg speakMsg;
		speakMsg._target = "Announcements";
		speakMsg.execute("PerchedParrot");
	}

	return true;
}

bool CAnnounce::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	// The very first time the player leaves the Embarklation Lobby, start announcements
	if (_notActivatedFlag) {
		addTimer(1, 1000, 0);
		_notActivatedFlag = false;
		_enabled = true;
	}

	return true;
}

bool CAnnounce::ActMsg(CActMsg *msg) {
	// Handle enabling or disabling announcements
	if (msg->_action == "Enable")
		_enabled = true;
	else if (msg->_action == "Disable")
		_enabled = false;

	return true;
}

} // End of namespace Titanic
