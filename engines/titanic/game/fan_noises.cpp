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

#include "titanic/game/fan_noises.h"
#include "titanic/core/room_item.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CFanNoises, CGameObject)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(SetVolumeMsg)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

CFanNoises::CFanNoises() : CGameObject(), _state(-1),
	_soundHandle(0), _soundPercent(70), _soundBalance(0), _soundSeconds(0),
	_stopSeconds(0), _startFlag(true) {
}

void CFanNoises::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_state, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_soundPercent, indent);
	file->writeNumberLine(_soundBalance, indent);
	file->writeNumberLine(_soundSeconds, indent);
	file->writeNumberLine(_stopSeconds, indent);
	file->writeNumberLine(_startFlag, indent);

	CGameObject::save(file, indent);
}

void CFanNoises::load(SimpleFile *file) {
	file->readNumber();
	_state = file->readNumber();
	_soundHandle = file->readNumber();
	_soundPercent = file->readNumber();
	_soundBalance = file->readNumber();
	_soundSeconds = file->readNumber();
	_stopSeconds = file->readNumber();
	_startFlag = file->readNumber();

	CGameObject::load(file);
}

bool CFanNoises::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (getParent() == msg->_newRoom) {
		if (_soundHandle != -1) {
			if (isSoundActive(_soundHandle))
				stopSound(_soundHandle, _stopSeconds);
			_soundHandle = -1;
			_startFlag = false;
		}

		switch (_state) {
		case 1:
			_soundHandle = playSound(TRANSLATE("b#60.wav", "b#40.wav"), 0, _soundBalance, true);
			setSoundVolume(_soundHandle, _soundPercent, _soundSeconds);
			_startFlag = true;
			break;
		case 2:
			_soundHandle = playSound(TRANSLATE("b#58.wav", "b#38.wav"), 0, _soundBalance, true);
			setSoundVolume(_soundHandle, _soundPercent, _soundSeconds);
			_startFlag = true;
			break;
		default:
			break;
		}
	}

	return true;
}

bool CFanNoises::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	if (getParent() == msg->_oldRoom && _soundHandle != -1) {
		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle, _stopSeconds);

		_soundHandle = -1;
		_startFlag = false;
	}

	return true;
}

bool CFanNoises::StatusChangeMsg(CStatusChangeMsg *msg) {
	if (msg->_newStatus >= -1 && msg->_newStatus <= 2) {
		int oldState = _state;
		_state = msg->_newStatus;

		switch (msg->_newStatus) {
		case -1:
		case 0:
			if (_soundHandle != -1) {
				if (isSoundActive(_soundHandle))
					stopSound(_soundHandle, 1);
				_soundHandle = -1;
				_startFlag = false;
			}

			switch (oldState) {
			case 1:
			case 2:
				playSound(TRANSLATE("b#59.wav", "b#39.wav"), _soundPercent, _soundBalance);
				break;
			default:
				break;
			}
			break;

		case 1:
			if (_soundHandle != -1) {
				if (isSoundActive(_soundHandle))
					stopSound(_soundHandle, 1);
				_soundHandle = -1;
				_startFlag = false;
			}

			switch (oldState) {
			case 0:
			case 2:
				_soundHandle = playSound(TRANSLATE("b#60.wav", "b#40.wav"), _soundPercent, _soundBalance, true);
				_startFlag = true;
				break;
			default:
				break;
			}
			break;

		case 2:
			if (_soundHandle != -1) {
				if (isSoundActive(_soundHandle))
					stopSound(_soundHandle, 1);
				_soundHandle = -1;
				_startFlag = false;
			}

			if (oldState == 1) {
				_soundHandle = playSound(TRANSLATE("b#58.wav", "b#38.wav"), _soundPercent, _soundBalance, true);
				_startFlag = true;
			}
			break;

		default:
			break;
		}
	}

	return true;
}

bool CFanNoises::SetVolumeMsg(CSetVolumeMsg *msg) {
	_soundPercent = msg->_volume;

	if (_soundHandle != -1 && isSoundActive(_soundHandle))
		setSoundVolume(_soundHandle, _soundPercent, msg->_secondsTransition);

	return true;
}

bool CFanNoises::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_startFlag) {
		_startFlag = false;
		_soundHandle = -1;

		switch (_state) {
		case 1:
			playSound(TRANSLATE("b#60.wav", "b#40.wav"), 0, _soundBalance, true);
			setSoundVolume(_soundHandle, _soundPercent, _soundSeconds);
			_startFlag = true;
			break;

		case 2:
			playSound(TRANSLATE("b#58.wav", "b#38.wav"), 0, _soundBalance, true);
			setSoundVolume(_soundHandle, _soundPercent, _soundSeconds);
			_startFlag = true;
			break;

		default:
			break;
		}
	}

	return true;
}

} // End of namespace Titanic
