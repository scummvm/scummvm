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

#include "titanic/carry/head_piece.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHeadPiece, CCarry)
	ON_MESSAGE(SenseWorkingMsg)
	ON_MESSAGE(PETGainedObjectMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

CHeadPiece::CHeadPiece() : CCarry(), _string6("Not Working"),
		_flag(0), _field13C(false) {
}

void CHeadPiece::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	file->writeQuotedLine(_string6, indent);
	file->writeNumberLine(_field13C, indent);

	CCarry::save(file, indent);
}

void CHeadPiece::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	_string6 = file->readString();
	_field13C = file->readNumber();

	CCarry::load(file);
}

bool CHeadPiece::SenseWorkingMsg(CSenseWorkingMsg *msg) {
	_string6 = msg->_value;
	return true;
}

bool CHeadPiece::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	_visibleFrame = 1;
	if (!_field13C) {
		incParrotResponse();
		_field13C = true;
	}

	// WORKAROUND: This fixes a bug in the original where if head pieces
	// were removed from Titania after adding, she would still reactivate
	CTakeHeadPieceMsg takeMsg(getName());
	takeMsg.execute("TitaniaControl");

	return true;
}

bool CHeadPiece::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkPoint(msg->_mousePos, false, true)) {
		return false;
	} else if (!_canTake) {
		return true;
	}

	if (_flag) {
		setVisible(true);
		moveToView();
		setPosition(Point(msg->_mousePos.x - _bounds.width() / 2,
			msg->_mousePos.y - _bounds.height() / 2));

		CTakeHeadPieceMsg takeMsg(getName());
		if (takeMsg._value != "NULL")
			takeMsg.execute("TitaniaControl");

		_flag = false;
	}

	return CCarry::MouseDragStartMsg(msg);
}

} // End of namespace Titanic
