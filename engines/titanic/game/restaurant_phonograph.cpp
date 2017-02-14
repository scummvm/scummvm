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

#include "titanic/game/restaurant_phonograph.h"
#include "titanic/core/room_item.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CRestaurantPhonograph, CPhonograph)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(PhonographPlayMsg)
	ON_MESSAGE(PhonographStopMsg)
	ON_MESSAGE(PhonographReadyToPlayMsg)
	ON_MESSAGE(EjectCylinderMsg)
	ON_MESSAGE(QueryPhonographState)
	ON_MESSAGE(LockPhonographMsg)
END_MESSAGE_MAP()

CRestaurantPhonograph::CRestaurantPhonograph() : CPhonograph(),
	_fieldF8(1), _field114(0) {}

void CRestaurantPhonograph::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeQuotedLine(_string3, indent);

	file->writeNumberLine(_field114, indent);

	CPhonograph::save(file, indent);
}

void CRestaurantPhonograph::load(SimpleFile *file) {
	file->readNumber();
	_fieldF8 = file->readNumber();
	_string2 = file->readString();
	_string3 = file->readString();
	_field114 = file->readNumber();

	CPhonograph::load(file);
}

bool CRestaurantPhonograph::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (!_fieldF8 && !_isPlaying) {
		CQueryCylinderHolderMsg holderMsg;
		holderMsg.execute(this);

		if (!holderMsg._isOpen) {
			CPhonographPlayMsg playMsg;
			playMsg.execute(this);
		} else if (holderMsg._isPresent) {
			CEjectCylinderMsg ejectMsg;
			ejectMsg.execute(this);

			_fieldE8 = true;
			if (_field114) {
				loadFrame(_fieldEC);
				playSound(_ejectSoundName);
			}
		}
	}

	return true;
}

bool CRestaurantPhonograph::PhonographPlayMsg(CPhonographPlayMsg *msg) {
	if (_isPlaying) {
		if (findView() == getView() && (!_fieldE8 || !_field114)) {
			loadFrame(_fieldEC);
			playSound(_ejectSoundName);
		}

		CQueryCylinderNameMsg nameMsg;
		nameMsg.execute(this);
		CRestaurantMusicChanged musicMsg(nameMsg._name);
		musicMsg.execute(findRoom());
	} else {
		loadFrame(_fieldF0);
	}

	return true;
}

bool CRestaurantPhonograph::PhonographStopMsg(CPhonographStopMsg *msg) {
	bool flag = _isPlaying;
	CPhonograph::PhonographStopMsg(msg);

	if (_isPlaying) {
		loadFrame(_fieldF0);
		if (flag)
			playSound(_string3);
	} else {
		loadFrame(_fieldEC);
	}

	return true;
}

bool CRestaurantPhonograph::PhonographReadyToPlayMsg(CPhonographReadyToPlayMsg *msg) {
	if (_fieldE8) {
		CPhonographPlayMsg playMsg;
		playMsg.execute(this);
		_fieldE8 = false;
	}

	return true;
}

bool CRestaurantPhonograph::EjectCylinderMsg(CEjectCylinderMsg *msg) {
	if (_isPlaying) {
		CPhonographStopMsg stopMsg;
		stopMsg.execute(this);
	}

	return true;
}

bool CRestaurantPhonograph::QueryPhonographState(CQueryPhonographState *msg) {
	msg->_value = _fieldF8;
	return true;
}

bool CRestaurantPhonograph::LockPhonographMsg(CLockPhonographMsg *msg) {
	_fieldF8 = msg->_value;
	return true;
}

} // End of namespace Titanic
