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

#include "graphics/pixelformat.h"
#include "common/debug.h"

namespace Graphics {

Common::String PixelFormat::toString() {
	if (bytesPerPixel == 1)
		return "CLUT8";

	int component[4];
	char tmp[10];
	tmp[0] = tmp[1] = 0;

	component[0] = rShift;
	component[1] = gShift;
	component[2] = bShift;
	component[3] = aShift;

	Common::String letters, digits;

	for (int c = 0; c < 4; c++) {
		int compPos = -1;
		int minshift = 100;

		// Find minimal component
		for (int i = 0; i < 4; i++)
			if (component[i] >= 0 && component[i] < minshift) {
				minshift = component[i];
				compPos = i;
			}

		// Clean duplicates
		for (int i = 0; i < 4; i++)
			if (component[i] == minshift)
				component[i] = -1;

		switch (compPos) {
		case 0:
			if (rLoss != 8) {
				letters += "R";
				tmp[0] = '0' + 8 - rLoss;
				digits += tmp;
			}
			break;
		case 1:
			if (gLoss != 8) {
				letters += "G";
				tmp[0] = '0' + 8 - gLoss;
				digits += tmp;
			}
			break;
		case 2:
			if (bLoss != 8) {
				letters += "B";
				tmp[0] = '0' + 8 - bLoss;
				digits += tmp;
			}
			break;
		case 3:
			if (aLoss != 8) {
				letters += "A";
				tmp[0] = '0' + 8 - aLoss;
				digits += tmp;
			}
			break;
		default:
			break;
		}
	}

	return letters + digits;
}

} // End of namespace Graphics
