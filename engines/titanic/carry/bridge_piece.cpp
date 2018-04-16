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

#include "titanic/carry/bridge_piece.h"
#include "titanic/game/ship_setting.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBridgePiece, CCarry)
	ON_MESSAGE(UseWithOtherMsg)
	ON_MESSAGE(PassOnDragStartMsg)
END_MESSAGE_MAP()

CBridgePiece::CBridgePiece() : CCarry(), _field140(0) {
}

void CBridgePiece::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string6, indent);
	file->writePoint(_pos3, indent);
	file->writeNumberLine(_field140, indent);

	CCarry::save(file, indent);
}

void CBridgePiece::load(SimpleFile *file) {
	file->readNumber();
	_string6 = file->readString();
	_pos3 = file->readPoint();
	_field140 = file->readNumber();

	CCarry::load(file);
}

bool CBridgePiece::UseWithOtherMsg(CUseWithOtherMsg *msg) {
	CShipSetting *shipSetting = dynamic_cast<CShipSetting *>(msg->_other);
	if (!shipSetting) {
		return CCarry::UseWithOtherMsg(msg);
	} else if (shipSetting->_itemName != "NULL") {
		petAddToInventory();
		return true;
	} else {
		setVisible(false);
		playSound(TRANSLATE("z#54.wav", "z#585.wav"));
		setPosition(shipSetting->_pos1);
		shipSetting->_itemName = getName();
		petMoveToHiddenRoom();

		CAddHeadPieceMsg headpieceMsg(shipSetting->getName() == _string6 ?
			"Enable" : "Disable");
		CSetFrameMsg frameMsg;

		CString name = getName();
		if (name == "ChickenBridge") {
			frameMsg._frameNumber = 1;
		} else if (name == "FanBridge") {
			frameMsg._frameNumber = 2;
		} else if (name == "SeasonBridge") {
			frameMsg._frameNumber = 3;
		} else if (name == "BeamBridge") {
			frameMsg._frameNumber = 4;
		}

		frameMsg.execute(shipSetting);
		headpieceMsg.execute(shipSetting);
		return true;
	}
}

bool CBridgePiece::PassOnDragStartMsg(CPassOnDragStartMsg *msg) {
	setVisible(true);
	moveToView();
	return CCarry::PassOnDragStartMsg(msg);
}

} // End of namespace Titanic
