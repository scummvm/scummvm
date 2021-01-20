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

#ifndef ULTIMA8_GRAPHICS_SOFTRENDERSURFACE_H
#define ULTIMA8_GRAPHICS_SOFTRENDERSURFACE_H

#include "ultima/ultima8/graphics/base_soft_render_surface.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima8 {

//
// SoftRenderSurface
//
// Desc: The class for software rendering in Pentagram
//
template<class uintX> class SoftRenderSurface : public BaseSoftRenderSurface {
public:

	// Create from a managed surface
	SoftRenderSurface(Graphics::ManagedSurface *managed);

	//
	// Surface Filling
	//

	// Fill buffer (using a RGB colour)
	void Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) override;

	//! Fill alpha channel
	void FillAlpha(uint8 alpha, int32 sx, int32 sy, int32 w, int32 h) override;

	// Fill the region doing alpha blending
	void FillBlended(uint32 rgba, int32 sx, int32 sy, int32 w, int32 h) override;

	//
	// The rule for painting methods:
	//
	// First arg are the source object to 'draw' with
	// Next args are any other required data to define the 'source'
	// Next args are the destination position
	//

	//
	// Basic Shape Painting
	//

	// Paint a Shape
	void Paint(const Shape *s, uint32 frame, int32 x, int32 y, bool untformed_pal = false) override;

	// Paint an Shape without clipping
	void PaintNoClip(const Shape *s, uint32 frame, int32 x, int32 y, bool untformed_pal = false) override;

	// Paint a Translucent Shape.
	void PaintTranslucent(const Shape *s, uint32 frame, int32 x, int32 y, bool untformed_pal = false) override;

	// Paint a Mirrored Shape
	void PaintMirrored(const Shape *s, uint32 frame, int32 x, int32 y, bool trans = false, bool untformed_pal = false) override;

	// Paint a Invisible Shape
	void PaintInvisible(const Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, bool untformed_pal = false) override;

	// Paint a Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	void PaintHighlight(const Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal = false) override;

	// Paint a Invisible Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	void PaintHighlightInvis(const Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal = false) override;

	//
	// Basic Line Drawing
	//

	// Draw a RGB Line
	void DrawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey) override;


	//
	// Basic Texture Blitting
	//

	// Blit a region from a Texture (Alpha == 0 -> skipped)
	void Blit(const Graphics::ManagedSurface *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, bool alpha_blend = false) override;

	// Blit a region from a Texture with a Colour blend (AlphaTex == 0 -> skipped. AlphaCol32 -> Blend Factors)
	void FadedBlit(const Graphics::ManagedSurface *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend = false) override;

	// Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
	void MaskedBlit(const Graphics::ManagedSurface *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend = false) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
