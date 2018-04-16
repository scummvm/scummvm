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
#include "titanic/translation.h"

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
	_soundActive(false) {
}

void CGondolierMixer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_soundHandle1, indent);
	file->writeNumberLine(_soundHandle2, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeQuotedLine(_soundName1, indent);
	file->writeQuotedLine(_soundName2, indent);
	file->writeNumberLine(_soundActive, indent);

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
	_soundActive = file->readNumber();

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
			_soundHandle1 = playSound(_soundName1, _volume1 * _slider1 / 10, 0, true);
			_soundActive = true;
		}

		if (_soundHandle2 == -1) {
			_soundHandle2 = playSound(_soundName2, _volume2 * _slider2 / 10, 0, true);
			_soundActive = true;
		}
	}

	return true;
}

bool CGondolierMixer::TurnOff(CTurnOff *msg) {
	// TODO: The stopSound calls should really be 2 seconds,
	// but doing so stops the changeover of mixers working when
	// going from the Arboretum room to FrozenArboretum.
	if (_soundHandle1 != -1) {
		if (isSoundActive(_soundHandle1))
			stopSound(_soundHandle1, 0);

		_soundHandle1 = -1;
		_soundActive = false;
	}

	if (_soundHandle2 != -1) {
		if (isSoundActive(_soundHandle2))
			stopSound(_soundHandle2, 0);

		_soundHandle2 = -1;
		_soundActive = false;
	}

	return true;
}

bool CGondolierMixer::SetVolumeMsg(CSetVolumeMsg *msg) {
	if (!_puzzleSolved) {
		_volume1 = _volume2 = msg->_volume;

		if (_soundHandle1 != -1 && isSoundActive(_soundHandle1))
			setSoundVolume(_soundHandle1, msg->_volume * _slider1 / 10, 2);
		if (_soundHandle2 != -1 && isSoundActive(_soundHandle2))
			setSoundVolume(_soundHandle2, msg->_volume * _slider2 / 10, 2);
	}

	return true;
}

bool CGondolierMixer::SignalObject(CSignalObject *msg) {
	if (!_puzzleSolved) {
		if (msg->_strValue == "Fly") {
			_slider1 = CLIP(msg->_numValue, 0, 10);

			if (!_rightSliderHooked) {
				_slider2 = 10 - _slider1;
				CStatusChangeMsg statusMsg;
				statusMsg._newStatus = _slider2;
				statusMsg.execute("GondolierRightLever");
			}
		}

		if (msg->_strValue == "Tos") {
			_slider2 = CLIP(msg->_numValue, 0, 10);

			if (!_leftSliderHooked) {
				_slider1 = 10 - _slider2;
				CStatusChangeMsg statusMsg;
				statusMsg._newStatus = _slider1;
				statusMsg.execute("GondolierLeftLever");
			}
		}

		if (!_slider1 && !_slider2 && _leftSliderHooked && _rightSliderHooked) {
			_puzzleSolved = true;
			CStatusChangeMsg statusMsg;
			statusMsg._newStatus = 1;
			statusMsg.execute("GondolierFace");
			CTurnOff offMsg;
			offMsg.execute(this);
			CVisibleMsg visibleMsg;
			visibleMsg.execute("Mouth");

			playSound(TRANSLATE("z#47.wav", "z#578.wav"));
		} else {
			CSetVolumeMsg volumeMsg(_volume1, 2);
			volumeMsg.execute(this);
		}
	}

	return true;
}

bool CGondolierMixer::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_soundActive) {
		_soundActive = 0;
		_soundHandle1 = _soundHandle2 = -1;
		CTurnOn onMsg;
		onMsg.execute(this);
	}

	return true;
}

} // End of namespace Titanic
