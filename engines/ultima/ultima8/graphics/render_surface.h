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

#ifndef ULTIMA8_GRAPHICS_RENDERSURFACE_H
#define ULTIMA8_GRAPHICS_RENDERSURFACE_H

#include "graphics/managed_surface.h"
#include "ultima/ultima8/misc/rect.h"

namespace Ultima {
namespace Ultima8 {

class Shape;

struct Palette;
struct Rect;

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

	// Create from a managed surface
	RenderSurface(Graphics::ManagedSurface *s, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	// Update the Pixels Pointer
	void SetPixelsPointer();

public:
	static uint8 _gamma10toGamma22[256];
	static uint8 _gamma22toGamma10[256];

	//! Create a standard RenderSurface
	static RenderSurface *SetVideoMode(uint32 width, uint32 height);

	//! Create a SecondaryRenderSurface with an associated Texture object
	static RenderSurface *CreateSecondaryRenderSurface(uint32 width, uint32 height);

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
	void GetSurfaceDims(Rect &) const;

	//! Get Clipping Rectangle
	void GetClippingRect(Rect &) const;

	//! Set Clipping Rectangle
	void SetClippingRect(const Rect &);

	//! Flip the surface
	void SetFlipped(bool flipped);

	//! Has the render surface been flipped?
	bool IsFlipped() const;

	//! Get a reference to the underlying surface that's being encapsulated
	Graphics::ManagedSurface *getRawSurface() const {
		return _surface;
	};

	//
	// Surface Filling
	//

	//! Fill the region with a color in the TEX32_PACK_RGB format
	void fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) {
		fill32(rgb, Rect(sx, sy, sx + w, sy + h));
	}

	//! Fill the region with a color in the TEX32_PACK_RGB format
	void fill32(uint32 rgb, const Rect &r);

	//! Fill the region doing alpha blending with a color in the TEX32_PACK_RGBA format
	void fillBlended(uint32 rgba, const Rect &r);

	//! Fill the region with a color in the TEX32_PACK_RGB format
	void frameRect32(uint32 rgb, const Rect &r);

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
	// Basic Line Drawing
	//

	// Draw a line with a color in the TEX32_PACK_RGB format
	void drawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey);


	//
	// Basic Texture Blitting
	//

	//! Blit a region from a Texture (Alpha == 0 -> skipped)
	void Blit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, bool alpha_blend = false);

	//! Blit a region from a Texture with a Colour blend (AlphaTex == 0 -> skipped. AlphaCol32 -> Blend Factors)
	void FadedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend = false);

	//! Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
	void MaskedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend = false);

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
