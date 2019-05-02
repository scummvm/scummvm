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

#include "titanic/carry/mouth.h"
#include "titanic/game/head_slot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMouth, CHeadPiece)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(PETGainedObjectMsg)
END_MESSAGE_MAP()

CMouth::CMouth() : CHeadPiece() {
}

void CMouth::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CHeadPiece::save(file, indent);
}

void CMouth::load(SimpleFile *file) {
	file->readNumber();
	CHeadPiece::load(file);
}

bool CMouth::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CHeadSlot *slot = dynamic_cast<CHeadSlot *>(msg->_other);
	if (!slot)
		return CHeadPiece::UseWithOtherMsg(msg);

	_flag = true;
	setVisible(false);
	setPosition(Point(0, 0));
	petMoveToHiddenRoom();

	CAddHeadPieceMsg addMsg(getName());
	if (addMsg._value != "NULL")
		addMsg.execute("MouthSlot");

	return true;
}

bool CMouth::MovieEndMsg(CMovieEndMsg *msg) {
	return true;
}

bool CMouth::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	_visibleFrame = 2;
	loadFrame(2);
	setVisible(true);
	if (!_field13C) {
		incParrotResponse();
		_field13C = true;
	}

	// WORKAROUND: If Mouth is removed from Titania after inserting,
	// message the Titania control so it can be flagged as removed
	CTakeHeadPieceMsg headpieceMsg(getName());
	headpieceMsg.execute("TitaniaControl");

	return true;
}

} // End of namespace Titanic
