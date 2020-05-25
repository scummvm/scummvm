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

#ifndef ULTIMA8_GRAPHICS_RENDERSURFACE_H
#define ULTIMA8_GRAPHICS_RENDERSURFACE_H

#include "graphics/pixelformat.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima8 {

class Texture;
class Shape;
class ShapeFont;
struct FixedWidthFont;

struct Palette;
struct Rect;
class Scaler;

#define UNPACK_RGB8(pix,r,g,b) { r = (((pix)&RenderSurface::_format.rMask)>>RenderSurface::_format.rShift)<<RenderSurface::_format.rLoss; g = (((pix)&RenderSurface::_format.gMask)>>RenderSurface::_format.gShift)<<RenderSurface::_format.gLoss; b = (((pix)&RenderSurface::_format.bMask)>>RenderSurface::_format.bShift)<<RenderSurface::_format.bLoss; }
#define PACK_RGB8(r,g,b) ((((r)>>RenderSurface::_format.rLoss)<<RenderSurface::_format.rShift) | (((g)>>RenderSurface::_format.gLoss)<<RenderSurface::_format.gShift) | (((b)>>RenderSurface::_format.bLoss)<<RenderSurface::_format.bShift))
#define PACK_RGB16(r,g,b) ((((r)>>RenderSurface::_format.rLoss16)<<RenderSurface::_format.rShift) | (((g)>>RenderSurface::_format.gLoss16)<<RenderSurface::_format.gShift) | (((b)>>RenderSurface::_format.bLoss16)<<RenderSurface::_format.bShift))

#define UNPACK_RGBA8(pix,r,g,b,a) { r = (((pix)&RenderSurface::_format.rMask)>>RenderSurface::_format.rShift)<<RenderSurface::_format.rLoss; g = (((pix)&RenderSurface::_format.gMask)>>RenderSurface::_format.gShift)<<RenderSurface::_format.gLoss; b = (((pix)&RenderSurface::_format.bMask)>>RenderSurface::_format.bShift)<<RenderSurface::_format.bLoss; ; a = (((pix)&RenderSurface::_format.aMask)>>RenderSurface::_format.aShift)<<RenderSurface::_format.aLoss; }
#define PACK_RGBA8(r,g,b,a) ((((r)>>RenderSurface::_format.rLoss)<<RenderSurface::_format.rShift) | (((g)>>RenderSurface::_format.gLoss)<<RenderSurface::_format.gShift) | (((b)>>RenderSurface::_format.bLoss)<<RenderSurface::_format.bShift) | (((a)>>RenderSurface::_format.aLoss)<<RenderSurface::_format.aShift))
#define PACK_RGBA16(r,g,b,a) ((((r)>>RenderSurface::_format.rLoss16)<<RenderSurface::_format.rShift) | (((g)>>RenderSurface::_format.gLoss16)<<RenderSurface::_format.gShift) | (((b)>>RenderSurface::_format.bLoss16)<<RenderSurface::_format.bShift) | (((a)>>RenderSurface::_format.aLoss16)<<RenderSurface::_format.aShift))

//
// RenderSurface
//
// Desc: The base abstact class for rendering in Pentagram
//
class RenderSurface {
public:
	struct U8PixelFormat : Graphics::PixelFormat {
		// Extend with some extra attributes
		byte  rLoss16, gLoss16, bLoss16, aLoss16;
		uint32  rMask,   gMask,   bMask,   aMask;

		inline U8PixelFormat() : Graphics::PixelFormat(),
			rLoss16(0), gLoss16(0), bLoss16(0), aLoss16(0),
			rMask(0), gMask(0), bMask(0), aMask(0)
		{
		}
	};

	static U8PixelFormat _format;

	static uint8 _gamma10toGamma22[256];
	static uint8 _gamma22toGamma10[256];

	//! Create a standard RenderSurface
	static RenderSurface *SetVideoMode(uint32 width, uint32 height, int bpp);

	//! Create a SecondaryRenderSurface with an associated Texture object
	static RenderSurface *CreateSecondaryRenderSurface(uint32 width, uint32 height);

	// Virtual Destructor
	virtual ~RenderSurface();

	static Graphics::PixelFormat getPixelFormat();

	//
	// Being/End Painting
	//

	//! Begin painting to the buffer. MUST BE CALLED BEFORE DOING ANYTHING TO THE SURFACE!
	// \note Can be called multiple times
	// \return true on success, false on failure
	virtual bool BeginPainting() = 0;

	//! Finish paining to the buffer.
	// \note MUST BE CALLED FOR EACH CALL TO BeginPainting()
	// \return true on success, false on failure
	virtual bool EndPainting() = 0;

	//! Get the surface as a Texture. Only valid for SecondaryRenderSurfaces
	// \note Do not delete the texture.
	// \note Do not assume anything about the contents of the Texture object.
	// \note It should only be used with Painting and Blitting methods.
	virtual Texture *GetSurfaceAsTexture() = 0;

	//
	// Surface Properties
	//

	//! Set the Origin of the Surface
	virtual void SetOrigin(int32 x, int32 y) = 0;

	//! Set the Origin of the Surface
	virtual void GetOrigin(int32 &x, int32 &y) const = 0;

	//! Get the Surface Dimensions
	virtual void GetSurfaceDims(Rect &) const = 0;

	//! Get Clipping Rectangle
	virtual void GetClippingRect(Rect &) const = 0;

	//! Set Clipping Rectangle
	virtual void SetClippingRect(const Rect &) = 0;

	//! Check Clipped. -1 if off screen, 0 if not clipped, 1 if clipped
	virtual int16 CheckClipped(const Rect &) const = 0;

	//! Flip the surface
	virtual void SetFlipped(bool flipped) = 0;

	//! Has the render surface been flipped?
	virtual bool IsFlipped() const = 0;

	//! Get a reference to the underlying surface that's being encapsulated
	virtual Graphics::ManagedSurface *getRawSurface() const = 0;

	//
	// Surface Palettes
	//
	// TODO: Handle Ultima8 and Crusader Xforms
	//

	// Set The Surface Palette
	// virtual void SetPalette(uint8 palette[768]) = 0;

	// Set The Surface Palette to be the one used by another surface
	// TODO: virtual void SetPalette(RenderSurface &) = 0;

	// Get The Surface Palette
	// TODO: virtual void GetPalette(uint8 palette[768]) = 0;

	virtual void CreateNativePalette(Palette *palette) = 0;


	//
	// Surface Filling
	//

	//! Fill buffer (using a palette index)
	virtual void Fill8(uint8 index, int32 sx, int32 sy, int32 w, int32 h) = 0;

	//! Fill buffer (using a RGB colour)
	virtual void Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) = 0;

	//! Fill alpha channel
	virtual void FillAlpha(uint8 alpha, int32 sx, int32 sy, int32 w, int32 h) = 0;

	//! Fill the region doing alpha blending
	virtual void FillBlended(uint32 rgba, int32 sx, int32 sy, int32 w, int32 h) = 0;

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
	// TODO: virtual void Paint(CachedShape* s, uint32 frame, int32 x, int32 y) = 0;
	virtual void Paint(Shape *s, uint32 frame, int32 x, int32 y, bool untformed_pal = false) = 0;

	//! Paint a Shape without clipping
	// TODO: virtual void PaintNoClip(CachedShape*s, uint32 frame, int32 x, int32 y) = 0;
	virtual void PaintNoClip(Shape *s, uint32 frame, int32 x, int32 y, bool untformed_pal = false) = 0;

	//! Paint a Translucent Shape.
	// TODO: virtual void PaintTranslucent(CachedShape* s, uint32 frame, int32 x, int32 y) = 0;
	virtual void PaintTranslucent(Shape *s, uint32 frame, int32 x, int32 y, bool untformed_pal = false) = 0;

	//! Paint a Mirrored Shape
	// TODO: virtual void PaintMirrored(CachedShape* s, uint32 frame, int32 x, int32 y, bool trans = false) = 0;
	virtual void PaintMirrored(Shape *s, uint32 frame, int32 x, int32 y, bool trans = false, bool untformed_pal = false) = 0;

	//! Paint an Invisible Shape
	// TODO: virtual void PaintInvisible(CachedShape* s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored) = 0;
	virtual void PaintInvisible(Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, bool untformed_pal = false) = 0;

	//! Paint a Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	// TODO: virtual void PaintHighlight(CachedShape* s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, uint32 col32);
	virtual void PaintHighlight(Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal = false) = 0;

	//! Paint a Invisible Highlighted Shape of using the 32 Bit Colour col32 (0xAARRGGBB Alpha is blend level)
	// TODO: virtual void PaintHighlightInvis(CachedShape* s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, uint32 col32);
	virtual void PaintHighlightInvis(Shape *s, uint32 frame, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal = false) = 0;

	//
	// Basic Line Drawing
	//

	// Draw a Line (using a palette index)
	// TODO: virtual void DrawLine8(uint8 index, int32 sx, int32 sy, int32 ex, int32 ey) = 0;

	// Draw a RGB Line
	virtual void DrawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey) = 0;


	//
	// Basic Font Drawing
	//
	//! Draw FixedWidthFont
	virtual void PrintTextFixed(FixedWidthFont *, const char *text, int x, int y) = 0;

	//! Draw a fixed width character from a FixedWidthFont
	virtual void PrintCharFixed(FixedWidthFont *, int character, int x, int y) = 0;


	//
	// Basic Texture Blitting
	//

	//! Blit a region from a Texture (Alpha == 0 -> skipped)
	virtual void Blit(Texture *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, bool alpha_blend = false) = 0;

	//! Blit a region from a Texture with a Colour blend (AlphaTex == 0 -> skipped. AlphaCol32 -> Blend Factors)
	virtual void FadedBlit(Texture *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend = false) = 0;

	//! Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
	virtual void MaskedBlit(Texture *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend = false) = 0;

	//! Blit a stretched region from a Texture (Alpha == 0 -> skipped???)
	virtual void StretchBlit(Texture *, int32 sx, int32 sy, int32 sw, int32 sh, int32 dx, int32 dy, int32 dw, int32 dh, bool clampedges = false) = 0;

	//! Blit a region from a Texture using a scaler
	virtual bool ScalerBlit(Texture *, int32 sx, int32 sy, int32 sw, int32 sh, int32 dx, int32 dy, int32 dw, int32 dh, const Scaler *, bool clampedges = false) = 0;


	////////////////////////////////////////
	// TODO: Add in Abstract 3d code Here //
	////////////////////////////////////////
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
