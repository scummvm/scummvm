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
#include "titanic/translation.h"

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

CArm::CArm() : CCarry(), _heldItemName("Key"),
	_puzzleUnused(0), _armUnlocked(false), _arboretumFrame(3), _unlockedFrame(0),
	_armRect(220, 208, 409, 350) {
}

void CArm::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_heldItemName, indent);
	file->writeNumberLine(_puzzleUnused, indent);
	file->writeRect(_hookedRect, indent);
	file->writeQuotedLine(_hookedTarget, indent);
	file->writeNumberLine(_armUnlocked, indent);
	file->writeRect(_armRect, indent);
	file->writeNumberLine(_arboretumFrame, indent);
	file->writeNumberLine(_unlockedFrame, indent);

	CCarry::save(file, indent);
}

void CArm::load(SimpleFile *file) {
	file->readNumber();
	_heldItemName = file->readString();
	_puzzleUnused = file->readNumber();
	_hookedRect = file->readRect();
	_hookedTarget = file->readString();
	_armUnlocked = file->readNumber();
	_armRect = file->readRect();
	_arboretumFrame = file->readNumber();
	_unlockedFrame = file->readNumber();

	CCarry::load(file);
}

bool CArm::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	_puzzleUnused = 0;
	_canTake = true;

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
	Point newPos(_bounds.left + msg->_delta.x, _bounds.top + msg->_delta.y);
	setPosition(newPos);
	return true;
}

bool CArm::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	if (_heldItemName != "None") {
		CShowTextMsg textMsg(ARM_ALREADY_HOLDING);
		textMsg.execute("PET");
		return false;
	} else if (msg->_other->getName() == "GondolierLeftLever") {
		CIsHookedOnMsg hookedMsg(_hookedRect, 0, getName());
		hookedMsg._rect.translate(_bounds.left, _bounds.top);
		hookedMsg.execute("GondolierLeftLever");

		if (hookedMsg._isHooked) {
			_hookedTarget = "GondolierLeftLever";
		} else {
			petAddToInventory();
		}
	} else if (msg->_other->getName() == "GondolierRightLever") {
		CIsHookedOnMsg hookedMsg(_hookedRect, 0, getName());
		hookedMsg._rect.translate(_bounds.left, _bounds.top);
		hookedMsg.execute("GondolierRightLever");

		if (hookedMsg._isHooked) {
			_hookedTarget = "GondolierRightLever";
		} else {
			petAddToInventory();
		}
	} else {
		petAddToInventory();
	}

	return true;
}

bool CArm::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (!_canTake) {
		CShowTextMsg textMsg(YOU_CANT_GET_THIS);
		textMsg.execute("PET");
	} else if (checkStartDragging(msg)) {
		hideMouse();
		_centroid = msg->_mousePos - _bounds;
		setPosition(msg->_mousePos - _centroid);

		if (!_hookedTarget.empty()) {
			CActMsg actMsg("Unhook");
			actMsg.execute(_hookedTarget);
			_hookedTarget.clear();
		}

		loadFrame(_visibleFrame);
		return true;
	}

	return false;
}

bool CArm::MaitreDHappyMsg(CMaitreDHappyMsg *msg) {
	CGameObject *petItem;
	if (find(getName(), &petItem, FIND_PET)) {
		if (!_armUnlocked)
			playSound(TRANSLATE("z#47.wav", "z#578.wav"));
		if (_heldItemName == "Key" || _heldItemName == "AuditoryCentre") {
			CGameObject *heldItem = dynamic_cast<CGameObject *>(getFirstChild());
			if (heldItem) {
				heldItem->setVisible(true);
				heldItem->petAddToInventory();
			}

			_visibleFrame = _unlockedFrame;
			loadFrame(_visibleFrame);
			_heldItemName = "None";
			petInvChange();
		}
	}

	_armUnlocked = true;
	_canTake = true;
	return true;
}

bool CArm::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	if (_armUnlocked) {
		if (_heldItemName == "Key" || _heldItemName == "AuditoryCentre") {
			CCarry *child = dynamic_cast<CCarry *>(getFirstChild());
			if (child) {
				_visibleFrame = _unlockedFrame;
				loadFrame(_visibleFrame);
				child->setVisible(true);
				child->petAddToInventory();
			}

			_heldItemName = "None";
		}
	}

	return true;
}

bool CArm::MouseDragMoveMsg(CMouseDragMoveMsg *msg) {
	setPosition(msg->_mousePos - _centroid);

	if (_heldItemName == "None" && compareViewNameTo("FrozenArboretum.Node 5.S")) {
		loadFrame(_armRect.contains(msg->_mousePos) ?
			_arboretumFrame : _visibleFrame);
	}

	return true;
}

} // End of namespace Titanic
