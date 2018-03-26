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

#ifndef GRAPHICS_MODE_H
#define GRAPHICS_MODE_H

#include "common/array.h"
#include "common/frac.h"

namespace Graphics {

/**
 * Represents a hardware video mode.
 */
struct Mode {
	int16 width;  ///< The width in pixels
	int16 height; ///< The height in pixels
	frac_t par;   ///< The pixel aspect ratio (pixel width / pixel height)

	Mode(const int16 w, const int16 h) :
		width(w),
		height(h) {
			if ((w == 320 && h == 200) || (w == 640 && h == 400))
				par = intToFrac(5) / 6;
			else
				par = intToFrac(1);
		}
	Mode(const int16 w, const int16 h, const frac_t ar) :
		width(w),
		height(h),
		par(ar) {}

	int16 correctedHeight() const {
		return fracToInt(height * par);
	}

	/// Return a Mode with height scaled by the pixel aspect ratio correction (and the par set to 1).
	/// This can for example be used to compare two corrected modes.
	Mode corrected() const {
		return Mode(width, correctedHeight(), intToFrac(1));
	}
};

typedef Common::Array<Mode> ModeList;

}

#endif
