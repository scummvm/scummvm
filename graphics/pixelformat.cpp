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
#include "common/algorithm.h"

namespace Graphics {

Common::String PixelFormat::toString() const {
	if (bytesPerPixel == 1)
		return "CLUT8";

	// We apply a trick to simplify the code here. We encode all the shift,
	// loss, and component name in the component entry. By having the shift as
	// highest entry we can sort according to shift.
	// This works because in valid RGB PixelFormats shift values needs to be
	// distinct except when the loss is 8. However, components with loss value
	// of 8 are not printed, thus their position does not matter.
	int component[4];
	component[0] = (rShift << 16) | (rLoss << 8) | 'R';
	component[1] = (gShift << 16) | (gLoss << 8) | 'G';
	component[2] = (bShift << 16) | (bLoss << 8) | 'B';
	component[3] = (aShift << 16) | (aLoss << 8) | 'A';

	// Sort components according to descending shift value.
	Common::sort(component, component + ARRAYSIZE(component), Common::Greater<int>());

	Common::String letters, digits;
	for (int i = 0; i < ARRAYSIZE(component); ++i) {
		const int componentLoss = (component[i] >> 8) & 0xFF;
		// A loss of 8 means that the component does not exist.
		if (componentLoss == 8) {
			continue;
		}

		const char componentName = component[i] & 0xFF;

		letters += componentName;
		digits += '0' + 8 - componentLoss;
	}

	return letters + digits + '@' + ('0' + bytesPerPixel);
}

} // End of namespace Graphics
