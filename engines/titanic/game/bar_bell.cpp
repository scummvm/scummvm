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

#include "titanic/game/bar_bell.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBarBell, CGameObject)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

CBarBell::CBarBell() : CGameObject(), _fieldBC(0),
	_volume(65), _soundVal3(0), _fieldC8(0), _fieldCC(0) {
}

void CBarBell::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_volume, indent);
	file->writeNumberLine(_soundVal3, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeNumberLine(_fieldCC, indent);

	CGameObject::save(file, indent);
}

void CBarBell::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_volume = file->readNumber();
	_soundVal3 = file->readNumber();
	_fieldC8 = file->readNumber();
	_fieldCC = file->readNumber();

	CGameObject::load(file);
}

bool CBarBell::EnterRoomMsg(CEnterRoomMsg *msg) {
	_fieldBC = 0;
	return true;
}

bool CBarBell::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if ((_fieldC8 % 3) == 2) {
		switch (_fieldBC) {
		case 0:
		case 1:
		case 5:
			playSound(TRANSLATE("c#54.wav", "c#38.wav"), _volume, _soundVal3);
			break;

		case 2:
			playSound(TRANSLATE("c#52.wav", "c#36.wav"), _volume, _soundVal3);
			break;

		case 3:
			playSound(TRANSLATE("c#53.wav", "c#37.wav"), _volume, _soundVal3);
			break;

		case 4:
			playSound(TRANSLATE("c#55.wav", "c#39.wav"), _volume, _soundVal3);
			break;

		default:
			playSound(TRANSLATE("c#51.wav", "c#35.wav"), _volume, _soundVal3);
			break;
		}
	} else if (_fieldBC >= 5) {
		if (_fieldBC == 6) {
			CActMsg actMsg("BellRing3");
			actMsg.execute("Barbot");
		}

		playSound(TRANSLATE("c#51.wav", "c#35.wav"), _volume, _soundVal3);
	} else {
		if (_fieldBC == 3) {
			CActMsg actMsg("BellRing1");
			actMsg.execute("Barbot");
		} else if (_fieldBC == 4) {
			CActMsg actMsg("BellRing2");
			actMsg.execute("Barbot");
		}

		playSound(TRANSLATE("c#54.wav", "c#38.wav"), _volume, _soundVal3);
	}

	return true;
}

bool CBarBell::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (!_fieldBC) {
		CTurnOn onMsg;
		onMsg.execute("Barbot");
	}

	++_fieldBC;
	return true;
}

bool CBarBell::ActMsg(CActMsg *msg) {
	if (msg->_action == "ResetCount") {
		_fieldBC = 0;
		++_fieldC8;
	}

	return true;
}

} // End of namespace Titanic
