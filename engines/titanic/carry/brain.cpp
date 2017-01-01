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

namespace Titanic {

BEGIN_MESSAGE_MAP(CBrain, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(VisibleMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(PassOnDragStartMsg)
	ON_MESSAGE(PETGainedObjectMsg)
END_MESSAGE_MAP()

CBrain::CBrain() : CCarry(), _field134(0), _field138(0) {
}

void CBrain::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writePoint(_pos1, indent);
	file->writeNumberLine(_field134, indent);
	file->writeNumberLine(_field138, indent);

	CCarry::save(file, indent);
}

void CBrain::load(SimpleFile *file) {
	file->readNumber();
	_pos1 = file->readPoint();
	_field134 = file->readNumber();
	_field138 = file->readNumber();

	CCarry::load(file);
}

bool CBrain::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CBrainSlot *slot = dynamic_cast<CBrainSlot *>(msg->_other);
	if (slot) {
		if (slot->getName() == "CentralCore") {
			setVisible(false);
			petMoveToHiddenRoom();
			CAddHeadPieceMsg headpieceMsg(getName());
			headpieceMsg.execute("CentralCoreSlot");
		}
		else if (!slot->_value1 && slot->getName() == "CentralCoreSlot") {
			setVisible(false);
			petMoveToHiddenRoom();
			CAddHeadPieceMsg headpieceMsg(getName());
			headpieceMsg.execute(msg->_other);
			playSound("z#116.wav");
			setPosition(Point(0, 0));
			setVisible(false);
			_field134 = 1;
		}

		return true;
	}
	else {
		return CCarry::UseWithOtherMsg(msg);
	}
}

bool CBrain::VisibleMsg(CVisibleMsg *msg) {
	setVisible(msg->_visible);
	return true;
}

bool CBrain::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!checkStartDragging(msg))
		return false;

	if (_field134) {
		CTakeHeadPieceMsg headpieceMsg(getName());
		headpieceMsg.execute("TitaniaControl");

		_field134 = 0;
		setVisible(true);
		moveToView();

		setPosition(Point(msg->_mousePos.x - _bounds.width() / 2,
			msg->_mousePos.y - _bounds.height() / 2));
	}

	return CCarry::MouseDragStartMsg(msg);
}

bool CBrain::PassOnDragStartMsg(CPassOnDragStartMsg *msg) {
	if (_field134) {
		CTakeHeadPieceMsg headpieceMsg(getName());
		headpieceMsg.execute("TitaniaControl");
		_field134 = 0;

		setVisible(true);
		moveToView();
		setPosition(Point(msg->_mousePos.x - _bounds.width() / 2,
			msg->_mousePos.y - _bounds.height() / 2));
	}

	return CCarry::PassOnDragStartMsg(msg);
}

bool CBrain::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	if (!_field138) {
		if (getName() == "Perch") {
			stateInc38();
			_field138 = 1;
		}
	}

	return true;
}

} // End of namespace Titanic
