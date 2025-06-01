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

#ifndef ULTIMA8_GFX_PALETTE_H
#define ULTIMA8_GFX_PALETTE_H

#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "ultima/ultima8/gfx/pal_transforms.h"

namespace Common {
class ReadStream;
}

namespace Ultima {
namespace Ultima8 {

class Palette: public Graphics::Palette {
public:
	Palette() : Graphics::Palette(256) {}

	void load(Common::ReadStream &rs, Common::ReadStream &xformrs);
	void load(Common::ReadStream &rs);

	// Transform a single set of rgb values based on the current matrix.
	// Not designed for speed - just useful for one-offs.
	void transformRGB(int &r, int &g, int &b) const;

	// Update the palette maps based on the pixel format and the current transformation matrix
	void updateNativeMap(const Graphics::PixelFormat &format, int maxindex = 0);

	// Untransformed pixel format palette map
	uint32 _native_untransformed[256];

	// Transformed pixel format palette map
	uint32 _native[256];

	// Untransformed XFORM ARGB palette map
	uint32 _xform_untransformed[256];

	// Transformed XFORM ARGB palette map
	uint32 _xform[256];

	// Colour transformation matrix (for fades, hue shifts)
	// Applied by the RenderSurface (fixed -4.11)
	// R = R*matrix[0] + G*matrix[1] + B*matrix[2]  + matrix[3];
	// G = R*matrix[4] + G*matrix[5] + B*matrix[6]  + matrix[7];
	// B = R*matrix[8] + G*matrix[9] + B*matrix[10] + matrix[11];
	// A = A;
	int16 _matrix[12];

	// The current palette transform
	PalTransforms _transform;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
