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

#include "titanic/carry/arm.h"
#include "titanic/messages/messages.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CArm, CCarry)
	ON_MESSAGE(PuzzleSolvedMsg)
	ON_MESSAGE(TranslateObjectMsg)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(MouseDragStartMsg)
	ON_MESSAGE(MaitreDHappyMsg)
	ON_MESSAGE(PETGainedObjectMsg)
	ON_MESSAGE(MouseDragMoveMsg)
END_MESSAGE_MAP()

CArm::CArm() : CCarry(), _string6("Key"),
	_field138(0), _field158(0), _field16C(3), _field170(0),
	_armRect(220, 208, 409, 350) {
}

void CArm::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string6, indent);
	file->writeNumberLine(_field138, indent);
	file->writeNumberLine(_hookedRect.left, indent);
	file->writeNumberLine(_hookedRect.top, indent);
	file->writeNumberLine(_hookedRect.right, indent);
	file->writeNumberLine(_hookedRect.bottom, indent);

	file->writeQuotedLine(_string7, indent);
	file->writeNumberLine(_field158, indent);
	file->writeNumberLine(_armRect.left, indent);
	file->writeNumberLine(_armRect.top, indent);
	file->writeNumberLine(_armRect.right, indent);
	file->writeNumberLine(_armRect.bottom, indent);
	file->writeNumberLine(_field16C, indent);
	file->writeNumberLine(_field170, indent);

	CCarry::save(file, indent);
}

void CArm::load(SimpleFile *file) {
	file->readNumber();
	_string6 = file->readString();
	_field138 = file->readNumber();
	_hookedRect.left = file->readNumber();
	_hookedRect.top = file->readNumber();
	_hookedRect.right = file->readNumber();
	_hookedRect.bottom = file->readNumber();

	_string7 = file->readString();
	_field158 = file->readNumber();
	_armRect.left = file->readNumber();
	_armRect.top = file->readNumber();
	_armRect.right = file->readNumber();
	_armRect.bottom = file->readNumber();
	_field16C = file->readNumber();
	_field170 = file->readNumber();

	CCarry::load(file);
}

bool CArm::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	_field138 = 0;
	_fieldE0 = 1;

	CString name = getName();
	if (name == "Arm1") {
		CActMsg actMsg("LoseArm");
		actMsg.execute("MaitreD");
		CPuzzleSolvedMsg solvedMsg;
		solvedMsg.execute("AuditoryCentre");
	} else if (name == "Arm2") {
		CPuzzleSolvedMsg solvedMsg;
		solvedMsg.execute("Key");
	}

	return true;
}

bool CArm::TranslateObjectMsg(CTranslateObjectMsg *msg) {
	Point newPos(_bounds.left - msg->_delta.x, _bounds.top - msg->_delta.y);
	setPosition(newPos);
	return true;
}

bool CArm::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	if (_string6 != "None") {
		CShowTextMsg textMsg("The arm is already holding something.");
		textMsg.execute("PET");
		return false;
	} else if (msg->_other->getName() == "GondolierLeftLever") {
		CIsHookedOnMsg hookedMsg(_hookedRect, 0, getName());
		hookedMsg._rect.translate(_bounds.left, _bounds.top);
		hookedMsg.execute("GondolierLeftLever");

		if (hookedMsg._result) {
			_string7 = "GondolierLeftLever";
		} else {
			petAddToInventory();
		}
	} else if (msg->_other->getName() == "GondolierRightLever") {
		CIsHookedOnMsg hookedMsg(_hookedRect, 0, getName());
		hookedMsg._rect.translate(_bounds.left, _bounds.top);
		hookedMsg.execute("GondolierRightLever");

		if (hookedMsg._result) {
			_string7 = "GondolierRightLever";
		} else {
			petAddToInventory();
		}
	}

	return true;
}

bool CArm::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!_fieldE0) {
		CShowTextMsg textMsg("You can't get this.");
		textMsg.execute("PET");
	} else if (checkStartDragging(msg)) {
		_tempPos = msg->_mousePos - _bounds;
		setPosition(msg->_mousePos - _tempPos);

		if (!_string7.empty()) {
			CActMsg actMsg("Unhook");
			actMsg.execute(_string7);
			_string7.clear();
		}

		loadFrame(_visibleFrame);
		return true;
	}

	return false;
}

bool CArm::MaitreDHappyMsg(CMaitreDHappyMsg *msg) {
	CGameObject *petItem;
	if (find(getName(), &petItem, FIND_PET)) {
		if (!_field158)
			playSound("z#47.wav", 100, 0, 0);
		if (_string6 == "Key" || _string6 == "AuditoryCentre") {
			CGameObject *child = static_cast<CGameObject *>(getFirstChild());
			if (child) {
				child->setVisible(true);
				petAddToInventory();
			}

			_visibleFrame = _field170;
			loadFrame(_visibleFrame);
			_string6 = "None";
			petInvChange();
		}
	}

	_field158 = 1;
	_fieldE0 = 1;
	return true;
}

bool CArm::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	if (_field158) {
		if (_string6 == "Key" || _string6 == "AuditoryCentre") {
			CCarry *child = static_cast<CCarry *>(getFirstChild());
			if (child) {
				_visibleFrame = _field170;
				loadFrame(_visibleFrame);
				child->setVisible(true);
				child->petAddToInventory();
			}

			_string6 = "None";
		}
	}

	return true;
}

bool CArm::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	setPosition(msg->_mousePos - _tempPos);

	if (_string6 != "None" && compareViewNameTo("FrozenArboretum.Node 5.S")) {
		loadFrame(_armRect.contains(msg->_mousePos) ?
			_field16C : _visibleFrame);
	}

	return true;
}

} // End of namespace Titanic
