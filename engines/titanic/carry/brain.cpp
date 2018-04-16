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

#include "titanic/carry/brain.h"
#include "titanic/game/brain_slot.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBrain, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(PassOnDragStartMsg)
	ON_MESSAGE(PETGainedObjectMsg)
END_MESSAGE_MAP()

CBrain::CBrain() : CCarry(), _pieceAdded(false), _perchGained(false) {
}

void CBrain::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writePoint(_pos1, indent);
	file->writeNumberLine(_pieceAdded, indent);
	file->writeNumberLine(_perchGained, indent);

	CCarry::save(file, indent);
}

void CBrain::load(SimpleFile *file) {
	file->readNumber();
	_pos1 = file->readPoint();
	_pieceAdded = file->readNumber();
	_perchGained = file->readNumber();

	CCarry::load(file);
}

bool CBrain::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CBrainSlot *slot = dynamic_cast<CBrainSlot *>(msg->_other);
	if (!slot)
		return CCarry::UseWithOtherMsg(msg);

	if (isEquals("CentralCore")) {
		setVisible(false);
		petMoveToHiddenRoom();
		CAddHeadPieceMsg headpieceMsg(getName());
		headpieceMsg.execute("CentralCoreSlot");
	} else if (!slot->_occupied && slot->getName() != "CentralCoreSlot") {
		// Brain card goes into vacant slot
		setVisible(false);
		petMoveToHiddenRoom();
		CAddHeadPieceMsg headpieceMsg(getName());
		headpieceMsg.execute(msg->_other);
		playSound(TRANSLATE("z#116.wav", "z#647.wav"));
		setPosition(Point(0, 0));
		setVisible(false);
		_pieceAdded = true;
	} else {
		// Trying to put brain card into an already occupied slot
		petAddToInventory();
	}

	return true;
}

bool CBrain::VisibleMsg(CVisibleMsg *msg) {
	setVisible(msg->_visible);
	return true;
}

bool CBrain::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg))
		return false;

	if (_pieceAdded) {
		CTakeHeadPieceMsg headpieceMsg(getName());
		headpieceMsg.execute("TitaniaControl");

		_pieceAdded = false;
		setVisible(true);
		moveToView();

		setPosition(Point(msg->_mousePos.x - _bounds.width() / 2,
			msg->_mousePos.y - _bounds.height() / 2));
	}

	return CCarry::MouseDragStartMsg(msg);
}

bool CBrain::PassOnDragStartMsg(CPassOnDragStartMsg *msg) {
	if (_pieceAdded) {
		CTakeHeadPieceMsg headpieceMsg(getName());
		headpieceMsg.execute("TitaniaControl");
		_pieceAdded = false;

		setVisible(true);
		moveToView();
		setPosition(Point(msg->_mousePos.x - _bounds.width() / 2,
			msg->_mousePos.y - _bounds.height() / 2));
	}

	return CCarry::PassOnDragStartMsg(msg);
}

bool CBrain::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	if (!_perchGained) {
		if (getName() == "Perch") {
			incParrotResponse();
			_perchGained = true;
		}
	}

	return true;
}

} // End of namespace Titanic
