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

#ifndef VCRUISE_AD2044_UI_H
#define VCRUISE_AD2044_UI_H

#include "common/rect.h"

namespace VCruise {

enum class AD2044InterfaceRectID {
	ActiveItemRender,
	ExamineButton,

	InventoryRender0,
	InventoryRender1,
	InventoryRender2,
	InventoryRender3,
	InventoryRender4,
	InventoryRender5,

};

namespace AD2044Interface {

Common::Rect getRectForUI(AD2044InterfaceRectID rectID);
Common::Rect getFirstInvSlotRect();
uint getInvSlotSpacing();


} // End of namespace AD2044Interface

} // End of namespace VCruise

#endif
