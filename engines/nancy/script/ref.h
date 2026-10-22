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

#ifndef NANCY_SCRIPT_REF_H
#define NANCY_SCRIPT_REF_H

#include "graphics/managed_surface.h"

namespace Nancy {
namespace Script {

// A "Ref" is an image scripts look things up in instead of showing it: the
// colors of its pixels say what is where. The map the player walks around on
// has one for every part of it, painted in the colors of where one can go.

// What to do about a place that is outside of the image
enum RefEdgeMode {
	kRefClamp	= 0,	// take the nearest pixel
	kRefWrap	= 1,	// start over at the other side
	kRefDismiss	= 2		// there is nothing there
};

struct RefImage {
	Graphics::ManagedSurface image;
	RefEdgeMode edgeMode = kRefClamp;

	// Returns false when there is no pixel at that place
	bool getPixel(int x, int y, uint32 &argb) const;
};

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_REF_H
