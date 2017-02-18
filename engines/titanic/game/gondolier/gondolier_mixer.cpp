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

#include "titanic/game/gondolier/gondolier_mixer.h"
#include "titanic/core/room_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGondolierMixer, CGondolierBase)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(SetVolumeMsg)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

CGondolierMixer::CGondolierMixer() : CGondolierBase(),
	_soundName1("c#0.wav"), _soundName2("c#1.wav"),
	_soundHandle1(-1), _soundHandle2(-1), _fieldC4(0), _fieldC8(0),
	_fieldE4(false) {
}

void CGondolierMixer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_soundHandle1, indent);
	file->writeNumberLine(_soundHandle2, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeQuotedLine(_soundName1, indent);
	file->writeQuotedLine(_soundName2, indent);
	file->writeNumberLine(_fieldE4, indent);

	CGondolierBase::save(file, indent);
}

void CGondolierMixer::load(SimpleFile *file) {
	file->readNumber();
	_soundHandle1 = file->readNumber();
	_soundHandle2 = file->readNumber();
	_fieldC4 = file->readNumber();
	_fieldC8 = file->readNumber();
	_soundName1 = file->readString();
	_soundName2 = file->readString();
	_fieldE4 = file->readNumber();

	CGondolierBase::load(file);
}

bool CGondolierMixer::EnterRoomMsg(CEnterRoomMsg *msg) {
	CRoomItem *parentRoom = dynamic_cast<CRoomItem *>(getParent());
	if (parentRoom == msg->_newRoom) {
		CTurnOn onMsg;
		onMsg.execute(this);
	}

	return true;
}

bool CGondolierMixer::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	CRoomItem *parentRoom = dynamic_cast<CRoomItem *>(getParent());
	if (parentRoom == msg->_oldRoom) {
		CTurnOff offMsg;
		offMsg.execute(this);
	}

	return true;
}

bool CGondolierMixer::TurnOn(CTurnOn *msg) {
	if (!_puzzleSolved) {
		if (_soundHandle1 == -1) {
			_soundHandle1 = playSound(_soundName1, _volume1 * _v4 / 10, 0, true);
			_fieldE4 = true;
		}

		if (_soundHandle2 == -1) {
			_soundHandle2 = playSound(_soundName1, _volume2 * _v7 / 10, 0, true);
			_fieldE4 = true;
		}
	}

	return true;
}

bool CGondolierMixer::TurnOff(CTurnOff *msg) {
	if (_soundHandle1 != -1) {
		if (isSoundActive(_soundHandle1))
			stopSound(_soundHandle1, 2);

		_soundHandle1 = -1;
		_fieldE4 = false;
	}

	if (_soundHandle2 != -1) {
		if (isSoundActive(_soundHandle2))
			stopSound(_soundHandle2, 2);

		_soundHandle2 = -1;
		_fieldE4 = false;
	}

	return true;
}

bool CGondolierMixer::SetVolumeMsg(CSetVolumeMsg *msg) {
	if (!_puzzleSolved) {
		_volume1 = _volume2 = msg->_volume;

		if (_soundHandle1 != -1 && isSoundActive(_soundHandle1))
			setSoundVolume(_soundHandle1, msg->_volume * _v4 / 10, 2);
		if (_soundHandle2 != -1 && isSoundActive(_soundHandle2))
			setSoundVolume(_soundHandle2, msg->_volume * _v7 / 10, 2);
	}

	return true;
}

bool CGondolierMixer::SignalObject(CSignalObject *msg) {
	if (!_puzzleSolved) {
		if (msg->_strValue == "Fly") {
			_v4 = CLIP(msg->_numValue, 0, 10);

			if (!_leftSliderHooked) {
				_v7 = 10 - _v4;
				CStatusChangeMsg statusMsg;
				statusMsg._newStatus = _v7;
				statusMsg.execute("GondolierRightLever");
			}
		}

		if (msg->_strValue == "Tos") {
			_v7 = CLIP(msg->_numValue, 0, 10);

			if (!_rightSliderHooked) {
				_v4 = 10 - _v7;
				CStatusChangeMsg statusMsg;
				statusMsg._newStatus = _v4;
				statusMsg.execute("GondolierLeftLever");
			}
		}

		if (!_v4 && !_v7 && _rightSliderHooked && _leftSliderHooked) {
			_puzzleSolved = true;
			CStatusChangeMsg statusMsg;
			statusMsg._newStatus = 1;
			statusMsg.execute("GondolierFace");
			CTurnOff offMsg;
			offMsg.execute(this);
			CVisibleMsg visibleMsg;
			visibleMsg.execute("Mouth");

			playSound("z#47.wav");
		} else {
			CSetVolumeMsg volumeMsg(_volume1, 2);
			volumeMsg.execute(this);
		}
	}

	return true;
}

bool CGondolierMixer::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_fieldE4) {
		_fieldE4 = 0;
		_soundHandle1 = _soundHandle2 = -1;
		CTurnOn onMsg;
		onMsg.execute(this);
	}

	return true;
}

} // End of namespace Titanic
