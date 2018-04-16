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

#include "titanic/npcs/character.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCharacter, CGameObject)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

CCharacter::CCharacter() : CGameObject(), _startFrame(0), _endFrame(0), _fieldC4(1) {
}

void CCharacter::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_startFrame, indent);
	file->writeNumberLine(_endFrame, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeQuotedLine(_charName, indent);

	CGameObject::save(file, indent);
}

void CCharacter::load(SimpleFile *file) {
	file->readNumber();
	_startFrame = file->readNumber();
	_endFrame = file->readNumber();
	_fieldC4 = file->readNumber();
	_charName = file->readString();

	CGameObject::load(file);
}

bool CCharacter::LeaveViewMsg(CLeaveViewMsg *msg) {
	CTurnOff offMsg;
	offMsg.execute(this);

	return true;
}

bool CCharacter::TurnOn(CTurnOn *msg) {
	if (!_fieldC4)
		_fieldC4 = 1;

	return true;
}

bool CCharacter::TurnOff(CTurnOff *msg) {
	CString charName = getName();
	if (charName == "Deskbot" || charName == "Barbot" || charName == "SuccUBus") {
		_fieldC4 = 0;
	}

	return true;
}

} // End of namespace Titanic
