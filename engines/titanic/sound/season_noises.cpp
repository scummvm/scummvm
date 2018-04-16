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

#include "titanic/sound/season_noises.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSeasonNoises, CViewAutoSoundPlayer)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

CSeasonNoises::CSeasonNoises() : CViewAutoSoundPlayer(), _seasonNumber(SEASON_SUMMER),
	_springName("NULL"), _summerName("NULL"), _autumnName("NULL"), _winterName("NULL") {
}

void CSeasonNoises::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_seasonNumber, indent);
	file->writeQuotedLine(_springName, indent);
	file->writeQuotedLine(_summerName, indent);
	file->writeQuotedLine(_autumnName, indent);
	file->writeQuotedLine(_winterName, indent);

	CViewAutoSoundPlayer::save(file, indent);
}

void CSeasonNoises::load(SimpleFile *file) {
	file->readNumber();
	_seasonNumber = (Season)file->readNumber();
	_springName = file->readString();
	_summerName = file->readString();
	_autumnName = file->readString();
	_winterName = file->readString();

	CViewAutoSoundPlayer::load(file);
}

bool CSeasonNoises::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_seasonNumber = (Season)(((int)_seasonNumber + 1) % 4);
	CActMsg actMsg("Update");
	actMsg.execute(this);

	return true;
}

bool CSeasonNoises::EnterViewMsg(CEnterViewMsg *msg) {
	CActMsg actMsg("Update");
	actMsg.execute(this);
	return true;
}

bool CSeasonNoises::ActMsg(CActMsg *msg) {
	msg->_action = "Update";

	switch (_seasonNumber) {
	case SEASON_SUMMER:
		_filename = _summerName;
		break;
	case SEASON_AUTUMN:
		_filename = _autumnName;
		break;
	case SEASON_WINTER:
		_filename = _winterName;
		break;
	case SEASON_SPRING:
		_filename = _springName;
		break;
	default:
		break;
	}

	CSignalObject signalMsg;
	signalMsg._numValue = 2;
	signalMsg.execute(this);

	CTurnOn onMsg;
	onMsg.execute(this);

	return true;
}

bool CSeasonNoises::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_active) {
		_active = false;
		_soundHandle = -1;

		CActMsg actMsg("Update");
		actMsg.execute(this);
	}

	return true;
}

} // End of namespace Titanic
