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
#include "titanic/translation.h"

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

	if (msg->_actionVal == 1) {
		CString numStr = "0";
		const char *const WAVE_NAMES1_EN[18] = {
			"z#181.wav", "z#211.wav", "z#203.wav", "z#202.wav", "z#201.wav",
			"z#200.wav", "z#199.wav", "z#198.wav", "z#197.wav", "z#196.wav",
			"z#210.wav", "z#209.wav", "z#208.wav", "z#207.wav", "z#206.wav",
			"z#205.wav", "z#204.wav", "z#145.wav"
		};
		const char *const WAVE_NAMES2_EN[30] = {
			"z#154.wav", "z#153.wav", "z#152.wav", "z#151.wav", "z#150.wav",
			"z#149.wav", "z#148.wav", "z#169.wav", "z#171.wav", "z#178.wav",
			"z#176.wav", "z#177.wav", "z#165.wav", "z#170.wav", "z#180.wav",
			"z#156.wav", "z#172.wav", "z#173.wav", "z#160.wav", "z#158.wav",
			"z#161.wav", "z#179.wav", "z#163.wav", "z#164.wav", "z#162.wav",
			"z#159.wav", "z#175.wav", "z#166.wav", "z#174.wav", "z#157.wav"
		};
		const char *const WAVE_NAMES1_DE[18] = {
			"z#712.wav", "z#741.wav", "z#733.wav", "z#732.wav", "z#731.wav",
			"z#730.wav", "z#729.wav", "z#728.wav", "z#727.wav", "z#726.wav",
			"z#740.wav", "z#739.wav", "z#738.wav", "z#737.wav", "z#736.wav",
			"z#735.wav", "z#734.wav", "z#701.wav"
		};
		const char *const WAVE_NAMES2_DE[31] = {
			"z#711.wav", "z#710.wav", "z#709.wav", "z#708.wav", "z#707.wav",
			"z#706.wav", "z#705.wav", "z#704.wav", "z#688.wav", "z#690.wav",
			"z#697.wav", "z#695.wav", "z#696.wav", "z#684.wav", "z#689.wav",
			"z#699.wav", "z#675.wav", "z#691.wav", "z#692.wav", "z#679.wav",
			"z#677.wav", "z#680.wav", "z#698.wav", "z#682.wav", "z#683.wav",
			"z#681.wav", "z#678.wav", "z#694.wav", "z#685.wav", "z#693.wav",
			"z#676.wav"
		};

		CProximity prox;
		prox._soundType = Audio::Mixer::kSpeechSoundType;

		int randVal = _nameIndex ? getRandomNumber(2) : 0;
		switch (randVal) {
		case 0:
		case 1:
			_soundHandle = playSound(TRANSLATE("z#189.wav", "z#719.wav"), prox);
			if (_nameIndex < 18) {
				queueSound(TRANSLATE(WAVE_NAMES1_EN[_nameIndex], WAVE_NAMES1_DE[_nameIndex]),
					_soundHandle, 100, 0, false, Audio::Mixer::kSpeechSoundType);
				++_nameIndex;
			} else {
				queueSound(TRANSLATE(WAVE_NAMES1_EN[getRandomNumber(17)], WAVE_NAMES1_DE[getRandomNumber(17)]),
					_soundHandle, 100, 0, false, Audio::Mixer::kSpeechSoundType);
			}
			break;

		case 2:
			_soundHandle = playSound(TRANSLATE("z#189.wav", "z#719.wav"), prox);
			queueSound(TRANSLATE(WAVE_NAMES2_EN[getRandomNumber(29)], WAVE_NAMES2_DE[getRandomNumber(30)]),
				_soundHandle, 100, 0, false, Audio::Mixer::kSpeechSoundType);
			break;

		default:
			break;
		}

		// Schedule another announcement for a random future time
		addTimer(1, 300000 + getRandomNumber(30000), 0);
		if (getRandomNumber(3) == 0)
			addTimer(2, 4000, 0);

	} else if (msg->_actionVal == 2) {
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
