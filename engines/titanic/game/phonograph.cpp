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

#include "titanic/game/phonograph.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPhonograph, CMusicPlayer)
	ON_MESSAGE(PhonographPlayMsg)
	ON_MESSAGE(PhonographStopMsg)
	ON_MESSAGE(PhonographRecordMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(MusicHasStartedMsg)
END_MESSAGE_MAP()

CPhonograph::CPhonograph() : CMusicPlayer(),
		_fieldE0(false), _fieldE4(0), _fieldE8(0), _fieldEC(0),
		_fieldF0(0), _fieldF4(0) {
}

void CPhonograph::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_fieldE8, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_fieldF4, indent);

	CMusicPlayer::save(file, indent);
}

void CPhonograph::load(SimpleFile *file) {
	file->readNumber();
	_string2 = file->readString();
	_fieldE0 = file->readNumber();
	_fieldE4 = file->readNumber();
	_fieldE8 = file->readNumber();
	_fieldEC = file->readNumber();
	_fieldF0 = file->readNumber();
	_fieldF4 = file->readNumber();

	CMusicPlayer::load(file);
}

bool CPhonograph::PhonographPlayMsg(CPhonographPlayMsg *msg) {
	CQueryCylinderHolderMsg holderMsg;
	holderMsg.execute(this);
	if (!holderMsg._value2) {
		_fieldE0 = false;
		return true;
	}

	CQueryCylinderMsg cylinderMsg;
	cylinderMsg.execute(holderMsg._target);

	if (cylinderMsg._name.empty()) {
		_fieldE0 = false;
	} else if (cylinderMsg._name.hasPrefix("STMusic")) {
		CStartMusicMsg startMsg(this);
		startMsg.execute(this);
		_fieldE0 = true;
		msg->_value = 1;
	} else {
		stopGlobalSound(0, -1);
		playGlobalSound(cylinderMsg._name, -2, true, true, 0);
		_fieldE0 = true;
		msg->_value = 1;
	}

	return true;
}

bool CPhonograph::PhonographStopMsg(CPhonographStopMsg *msg) {
	CQueryCylinderHolderMsg holderMsg;
	holderMsg.execute(this);
	if (!holderMsg._value2)
		return true;

	_fieldE0 = false;
	CQueryCylinderMsg cylinderMsg;
	cylinderMsg.execute(holderMsg._target);

	if (_fieldE0) {
		if (!cylinderMsg._name.empty()) {
			if (cylinderMsg._name.hasPrefix("STMusic")) {
				CStopMusicMsg stopMsg;
				stopMsg.execute(this);
			} else {
				stopGlobalSound(msg->_value1, -1);
			}
			msg->_value2 = 1;
		}

		if (!msg->_value3)
			_fieldE0 = false;
	} else if (_fieldE4) {
		_fieldE4 = false;
		msg->_value2 = 1;
	}

	return true;
}

bool CPhonograph::PhonographRecordMsg(CPhonographRecordMsg *msg) {
	if (!_fieldE0 && !_fieldE4 && !_fieldE8) {
		CQueryCylinderHolderMsg holderMsg;
		holderMsg.execute(this);

		if (holderMsg._value2) {
			_fieldE4 = true;
			CErasePhonographCylinderMsg eraseMsg;
			eraseMsg.execute(holderMsg._target);
		} else {
			_fieldE4 = false;
		}
	}

	return true;
}

bool CPhonograph::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (_fieldE0) {
		CPhonographPlayMsg playMsg;
		playMsg.execute(this);
	}

	return true;
}

bool CPhonograph::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	if (_fieldE0) {
		CPhonographStopMsg stopMsg;
		stopMsg._value1 = 1;
		stopMsg.execute(this);
	}

	return true;
}

bool CPhonograph::MusicHasStartedMsg(CMusicHasStartedMsg *msg) {
	if (_fieldE4) {
		CQueryCylinderHolderMsg holderMsg;
		holderMsg.execute(this);
		if (holderMsg._value2) {
			CRecordOntoCylinderMsg recordMsg;
			recordMsg.execute(holderMsg._target);
		} else {
			_fieldE4 = false;
		}
	}

	return true;
}

} // End of namespace Titanic
