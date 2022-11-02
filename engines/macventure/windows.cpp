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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/windows.h"
#include "graphics/macgui/macwindowborder.h"

namespace MacVenture {

BorderBounds borderBounds(MVWindowType type) {
	switch (type) {
	case MacVenture::kDocument:
		break;
	case MacVenture::kDBox:
		break;
	case MacVenture::kPlainDBox:
		return BorderBounds(1, 1, 1, 2);
	case MacVenture::kAltBox:
		return BorderBounds(2, 2, 2, 2); // Hand-tested
	case MacVenture::kNoGrowDoc:
		return BorderBounds(2, 20, 2, 2);
	case MacVenture::kMovableDBox:
		break;
	case MacVenture::kZoomDoc:
		return BorderBounds(2, 21, 17, 2, 15, 0);
	case MacVenture::kZoomNoGrow:
		break;
	case MacVenture::kInvWindow:
		//return BorderBounds(4, 10, 10, 10);
		return BorderBounds(4, 21, 19, 18, 15, 15);
	case MacVenture::kRDoc16:
		break;
	case MacVenture::kRDoc4:
		return BorderBounds(2, 20, 3, 3);
	case MacVenture::kRDoc6:
		break;
	case MacVenture::kRDoc10:
		break;
	default:
		break;
	}

	return BorderBounds(0, 0, 0, 0);
}

Graphics::BorderOffsets borderOffsets(MVWindowType type) {
	BorderBounds bbs = borderBounds(type);

	Graphics::BorderOffsets offsets;
	offsets.left = bbs.leftOffset;
	offsets.right = bbs.rightOffset;
	offsets.top = bbs.topOffset;
	offsets.bottom = bbs.bottomOffset;

	offsets.titleTop = -1;
	offsets.titleBottom = -1;
	offsets.titlePos = 0;
	offsets.dark = false;
	offsets.upperScrollHeight = 0;
	offsets.lowerScrollHeight = 0;

	switch (type) {
	case MacVenture::kDocument:
		break;
	case MacVenture::kDBox:
		break;
	case MacVenture::kPlainDBox:
		break;
	case MacVenture::kAltBox:
		break;
	case MacVenture::kNoGrowDoc:
		offsets.titleTop = 2;
		offsets.titleBottom = 0;
		offsets.titlePos = 29;
		break;
	case MacVenture::kMovableDBox:
		break;
	case MacVenture::kZoomDoc:
		offsets.titleTop = 3;
		offsets.titleBottom = 0;
		offsets.titlePos = 25;

		offsets.upperScrollHeight = 20;
		offsets.lowerScrollHeight = 20;
		break;
	case MacVenture::kZoomNoGrow:
		offsets.titleTop = 0;
		offsets.titleBottom = 0;
		offsets.titlePos = 0;
		break;
	case MacVenture::kInvWindow:
		offsets.titleTop = 3;
		offsets.titleBottom = 0;
		offsets.titlePos = 36;

		offsets.upperScrollHeight = 20;
		offsets.lowerScrollHeight = 20;
		break;
	case MacVenture::kRDoc4:
		offsets.titleTop = 2;
		offsets.titleBottom = 0;
		offsets.titlePos = 22;
		break;
	default:
		break;
	}
	return offsets;
}
} // End of namespace MacVenture
