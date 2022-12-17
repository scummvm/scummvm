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

#ifndef ULTIMA8_GRAPHICS_XFORMBLEND_H
#define ULTIMA8_GRAPHICS_XFORMBLEND_H

#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/graphics/render_surface.h"

namespace Ultima {
namespace Ultima8 {

#ifndef P_FASTCALL
#ifdef _MSC_VER
#define P_FASTCALL __fastcall
#elif (defined(WIN32) && defined(FASTCALL))
#define P_FASTCALL FASTCALL
#else
#define P_FASTCALL
#endif
#endif

extern const uint8 U8XFormPal[1024];
extern const uint8 CruXFormPal[1024];

inline uint32 P_FASTCALL BlendPreModulated(uint32 src, uint32 dst, const Graphics::PixelFormat &format) {
	uint8 sr, sg, sb;
	format.colorToRGB(dst, sr, sg, sb);

	uint32 r = sr * (256 - TEX32_A(src));
	uint32 g = sg * (256 - TEX32_A(src));
	uint32 b = sb * (256 - TEX32_A(src));
	r += 256 * TEX32_R(src);
	g += 256 * TEX32_G(src);
	b += 256 * TEX32_B(src);
	r >>= 8;
	g >>= 8;
	b >>= 8;
	return format.RGBToColor(r > 0xFF ? 0xFF : r, g > 0xFF ? 0xFF : g, b > 0xFF ? 0xFF : b);
}

inline uint32 P_FASTCALL BlendPreModFast(uint32 src, uint32 dst, const Graphics::PixelFormat &format) {
	uint8 sr, sg, sb;
	format.colorToRGB(dst, sr, sg, sb);

	uint32 r = sr * (256 - TEX32_A(src));
	uint32 g = sg * (256 - TEX32_A(src));
	uint32 b = sb * (256 - TEX32_A(src));
	r += 256 * TEX32_R(src);
	g += 256 * TEX32_G(src);
	b += 256 * TEX32_B(src);
	return format.RGBToColor(r >> 8, g >> 8, b >> 8);
}

// This does the red highlight blending.
inline uint32 P_FASTCALL BlendHighlight(uint32 src, uint32 cr, uint32 cg, uint32 cb, uint32 ca, uint32 ica, const Graphics::PixelFormat &format) {
	uint8 sr, sg, sb;
	format.colorToRGB(src, sr, sg, sb);
	return format.RGBToColor((sr * ica + cr * ca) >> 8,
							 (sg * ica + cg * ca) >> 8,
							 (sb * ica + cb * ca) >> 8);
}

// This does the invisible blending. I've set it to about 40%
inline uint32 P_FASTCALL BlendInvisible(uint32 src, uint32 dst, const Graphics::PixelFormat &format) {
	uint8 sr, sg, sb;
	uint8 dr, dg, db;
	format.colorToRGB(src, sr, sg, sb);
	format.colorToRGB(dst, dr, dg, db);
	return format.RGBToColor((sr * 100 + dr * 156) >> 8,
							 (sg * 100 + dg * 156) >> 8,
							 (sb * 100 + db * 156) >> 8);
}

// This does the translucent highlight blending. (50%)
inline uint32 P_FASTCALL BlendHighlightInvis(uint32 src, uint32 dst, uint32 cr, uint32 cg, uint32 cb, uint32 ca, uint32 ica, const Graphics::PixelFormat &format) {
	uint8 sr, sg, sb;
	uint8 dr, dg, db;
	format.colorToRGB(src, sr, sg, sb);
	format.colorToRGB(dst, dr, dg, db);
	return format.RGBToColor((((sr * ica + cr * ca) >> 1) + (dr << 7)) >> 8,
							 (((sg * ica + cg * ca) >> 1) + (dg << 7)) >> 8,
							 (((sb * ica + cb * ca) >> 1) + (db << 7)) >> 8);
}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
