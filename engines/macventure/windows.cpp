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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/windows.h"

namespace MacVenture {

BorderBounds borderBounds(MVWindowType type) {
	switch (type) {
	case MacVenture::kDocument:
		break;
	case MacVenture::kDBox:
		break;
	case MacVenture::kPlainDBox:
		return BorderBounds(3, 3, 3, 3);
	case MacVenture::kAltBox:
		return BorderBounds(0, 0, 0, 0); // Hand-tested
	case MacVenture::kNoGrowDoc:
		return BorderBounds(1, 20, 1, 1);
	case MacVenture::kMovableDBox:
		break;
	case MacVenture::kZoomDoc:
		return BorderBounds(1, 20, 17, 1);
	case MacVenture::kZoomNoGrow:
		break;
	case MacVenture::kInvWindow:
		return BorderBounds(1, 20, 17, 17);
	case MacVenture::kRDoc16:
		break;
	case MacVenture::kRDoc4:
		return BorderBounds(1, 20, 1, 1);
	case MacVenture::kRDoc6:
		break;
	case MacVenture::kRDoc10:
		break;
	default:
		break;
	}

	return BorderBounds(0, 0, 0, 0);
}
} // End of namespace MacVenture
