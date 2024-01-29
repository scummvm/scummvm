/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "vcruise/ad2044_ui.h"

namespace VCruise {

namespace AD2044Interface {

Common::Rect getRectForUI(AD2044InterfaceRectID rectID) {
	switch (rectID) {
	case AD2044InterfaceRectID::ActiveItemRender:
		return Common::Rect(512, 150, 588, 217);
	case AD2044InterfaceRectID::ExamineButton:
		return Common::Rect(495, 248, 595, 318);
	case AD2044InterfaceRectID::InventoryRender0:
		return Common::Rect(24, 394, 100, 461);
	case AD2044InterfaceRectID::InventoryRender1:
		return Common::Rect(119, 395, 195, 462);
	case AD2044InterfaceRectID::InventoryRender2:
		return Common::Rect(209, 393, 285, 460);
	case AD2044InterfaceRectID::InventoryRender3:
		return Common::Rect(302, 393, 378, 460);
	case AD2044InterfaceRectID::InventoryRender4:
		return Common::Rect(393, 394, 469, 461);
	case AD2044InterfaceRectID::InventoryRender5:
		return Common::Rect(481, 393, 557, 460);
	default:
		return Common::Rect();
	}
}

Common::Rect getFirstInvSlotRect() {
	return Common::Rect(21, 392, 96, 460);
}

uint getInvSlotSpacing() {
	return 92;
}

} // End of namespace AD2044Interface

} // End of namespace VCruise
