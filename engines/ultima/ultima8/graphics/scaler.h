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

#ifndef ULTIMA8_GRAPHICS_SCALER_H
#define ULTIMA8_GRAPHICS_SCALER_H

#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/graphics/render_surface.h"

namespace Ultima {
namespace Ultima8 {

/// Base Scaler class
class Scaler {
	friend class hqScaler;
protected:
	// Basic scaler function template
	typedef bool (*ScalerFunc)(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                           uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src);

	//
	// Basic scaler functions (filled in by the scalers constructor)
	//
	ScalerFunc  Scale16Nat;
	ScalerFunc  Scale16Sta;

	ScalerFunc  Scale32Nat;
	ScalerFunc  Scale32Sta;
	ScalerFunc  Scale32_A888;
	ScalerFunc  Scale32_888A;

	Scaler() : Scale16Nat(0), Scale16Sta(0), Scale32Nat(0), Scale32Sta(0),
		Scale32_A888(0), Scale32_888A(0) {}
public:
	//
	// Scaler Capabilites
	//

	virtual uint32    ScaleBits() const = 0;          //< bits for supported integer scaling
	virtual bool      ScaleArbitrary() const = 0;     //< supports arbitrary scaling of any degree

	//
	// Maybe one day... for now we just grab everything from RenderSurface
	// virtual bool SetDisplayFormat(const RenderSurface::Format &format);

	// Call this to scale a section of the screen
	inline bool Scale(Texture *texture, int32 sx, int32 sy, int32 sw, int32 sh,
	                  uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) const {
		// Check to see if we are doing valid integer scalings
		if (!ScaleArbitrary()) {
			uint32 scale_bits = ScaleBits();
			int x_factor = dw / sw;
			int y_factor = dh / sh;

			// Not integer
			if ((x_factor * sw) != dw || (y_factor * sh) != dh) return false;

			// Don't support this
			if (!(scale_bits & (1 << x_factor))) return false;

			// Don't support this
			if (!(scale_bits & (1 << y_factor))) return false;
		}

		if (RenderSurface::_format.s_bytes_per_pixel == 4) {
			if (texture->_format == TEX_FMT_NATIVE || (texture->_format == TEX_FMT_STANDARD &&
			        RenderSurface::_format.a_mask == TEX32_A_MASK && RenderSurface::_format.r_mask == TEX32_R_MASK &&
			        RenderSurface::_format.g_mask == TEX32_G_MASK && RenderSurface::_format.b_mask == TEX32_B_MASK)) {
				if (RenderSurface::_format.a_mask == 0xFF000000) {
					if (!Scale32_A888) return false;
					return Scale32_A888(texture, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
				} else if (RenderSurface::_format.a_mask == 0x000000FF) {
					if (!Scale32_888A) return false;
					return Scale32_888A(texture, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
				} else {
					if (!Scale32Nat) return false;
					return Scale32Nat(texture, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
				}
			} else if (texture->_format == TEX_FMT_STANDARD) {
				if (!Scale32Sta) return false;
				return Scale32Sta(texture, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
			}
		}
		if (RenderSurface::_format.s_bytes_per_pixel == 2) {
			if (texture->_format == TEX_FMT_NATIVE) {
				if (!Scale16Nat) return false;
				return Scale16Nat(texture, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
			} else if (texture->_format == TEX_FMT_STANDARD) {
				if (!Scale16Sta) return false;
				return Scale16Sta(texture, sx, sy, sw, sh, pixel, dw, dh, pitch, clamp_src);
			}
		}

		return false;
	}

	virtual ~Scaler() { }
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
