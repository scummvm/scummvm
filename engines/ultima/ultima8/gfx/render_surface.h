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

#ifndef ULTIMA8_GFX_RENDERSURFACE_H
#define ULTIMA8_GFX_RENDERSURFACE_H

#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima8 {

class Shape;

//
// RenderSurface
//
// Desc: The base class for rendering in Pentagram
//
class RenderSurface {
private:
	// Frame buffer
	uint8 *_pixels;   // Pointer to logical pixel 0,0

	// Dimensions
	int32 _ox, _oy;        // Physical Pixel for Logical Origin
	int32 _pitch;          // Frame buffer pitch (bytes) (could be negated)
	bool _flipped;

	// Clipping Rectangle
	Common::Rect _clipWindow;

	// Locking count
	uint32 _lockCount; // Number of locks on surface

	Graphics::ManagedSurface *_surface;
	DisposeAfterUse::Flag _disposeAfterUse;

	// Update the Pixels Pointer
	void SetPixelsPointer();

public:
	// Create a render surface
	RenderSurface(int width, int height, const Graphics::PixelFormat &format);

	// Create from a managed surface
	RenderSurface(Graphics::ManagedSurface *s, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	~RenderSurface();

	//
	// Being/End Painting
	//

	//! Begin painting to the buffer. MUST BE CALLED BEFORE DOING ANYTHING TO THE SURFACE!
	// \note Can be called multiple times
	// \return true on success, false on failure
	bool BeginPainting();

	//! Finish paining to the buffer.
	// \note MUST BE CALLED FOR EACH CALL TO BeginPainting()
	// \return true on success, false on failure
	bool EndPainting();

	//
	// Surface Properties
	//

	//! Set the Origin of the Surface
	void SetOrigin(int32 x, int32 y);

	//! Set the Origin of the Surface
	void GetOrigin(int32 &x, int32 &y) const;

	//! Get the Surface Dimensions
	Common::Rect32 getSurfaceDims() const;

	//! Get Clipping Rectangle
	Common::Rect32 getClippingRect() const;

	//! Set Clipping Rectangle
	void setClippingRect(const Common::Rect32 &);

	//! Flip the surface
	void SetFlipped(bool flipped);

	//! Has the render surface been flipped?
	bool IsFlipped() const;

	//! Get a reference to the underlying surface that's being encapsulated
	Graphics::ManagedSurface *getRawSurface() const {
		return _surface;
	};

	//! Fill the region with a color in the pixel format
	void fillRect(const Common::Rect32 &r, uint32 color);

	//! Fill the region with a color in the pixel format
	void frameRect(const Common::Rect32 &r, uint32 color);

	// Draw a line with a color in the pixel format
	void drawLine(int32 sx, int32 sy, int32 ex, int32 ey, uint32 color);

	//! Fill the region with a color in the TEX32_PACK_RGB format
	void fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) {
		fill32(rgb, Common::Rect32(sx, sy, sx + w, sy + h));
	}

	//! Fill the region with a color in the TEX32_PACK_RGB format
	void fill32(uint32 rgb, const Common::Rect32 &r);

	//! Fill the region doing alpha blending with a color in the TEX32_PACK_RGBA format
	void fillBlended(uint32 rgba, const Common::Rect32 &r);

	//! Fill the region with a color in the TEX32_PACK_RGB format
	void frameRect32(uint32 rgb, const Common::Rect32 &r);

	// Draw a line with a color in the TEX32_PACK_RGB format
	void drawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey);

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

	//! Paint a Shape
	void Paint(const Shape *s, uint32 frame, int32 x, int32 y, bool mirrored = false);

	//! Paint a Translucent Shape.
	void PaintTranslucent(const Shape *s, uint32 frame, int32 x, int32 y, bool mirrored = false);

	//! Paint an Invisible Shape
	void PaintInvisible(const Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored);

	//! Paint a Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	void PaintHighlight(const Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, uint32 col32);

	//! Paint a Invisible Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	void PaintHighlightInvis(const Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, uint32 col32);

	//
	// Basic Texture Blitting
	//

	//! Blit a region from a Texture (Alpha == 0 -> skipped)
	void Blit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, bool alpha_blend = false);

	void CrossKeyBlitMap(const Graphics::Surface &src, const Common::Rect &srcRect, int32 dx, int32 dy, const uint32 *map, const uint32 key);

	//! Blit a region from a Texture with a Colour blend (AlphaTex == 0 -> skipped. AlphaCol32 -> Blend Factors)
	void FadedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend = false);

	//! Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
	void MaskedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend = false);

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
