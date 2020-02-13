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

#ifndef ULTIMA8_GRAPHICS_BASESOFTRENDERSURFACE_H
#define ULTIMA8_GRAPHICS_BASESOFTRENDERSURFACE_H

#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/misc/rect.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima8 {

//
// Class BaseSoftRenderSurface
//
// Desc: The base abstact class for software rendering in Pentagram
//
class BaseSoftRenderSurface : public RenderSurface {
protected:
	// Frame buffer
	uint8           *pixels;                // Pointer to logical pixel 0,0
	uint8           *pixels00;              // Pointer to physical pixel 0,0

	// Depth Buffer
	uint16          *zbuffer;               // Pointer to logical pixel 0,0
	uint8           *zbuffer00;             // Pointer to physical pixel 0,0

	// Pixel Format (also see 'Colour shifting values' later)
	int             bytes_per_pixel;        // 2 or 4
	int             bits_per_pixel;         // 16 or 32
	int             format_type;            // 16, 555, 565, 32 or 888

	// Dimensions
	int32           ox, oy;                 // Physical Pixel for Logical Origin
	int32           width, height;          // Width and height
	int32           pitch;                  // Frame buffer pitch (bytes) (could be negated)
	int32           zpitch;                 // Z Buffer pitch (bytes) (could be negated)
	bool            flipped;

	// Clipping Rectangle
	Rect clip_window;

	// Locking count
	uint32          lock_count;             // Number of locks on surface

	Graphics::ManagedSurface *sdl_surf;

	// Renderint to a texture
	Texture         *rtt_tex;

	// Create from a SDL_Surface
	BaseSoftRenderSurface(Graphics::ManagedSurface *);

	// Create with Texture
	BaseSoftRenderSurface(int w, int h);

	// Create Generic
	BaseSoftRenderSurface(int w, int h, int bpp, int rsft, int gsft, int bsft, int asft);
	BaseSoftRenderSurface(int w, int h, uint8 *buf);
	virtual ECode GenericLock()  {
		return P_NO_ERROR;
	}
	virtual ECode GenericUnlock()  {
		return P_NO_ERROR;
	}

	// Update the Pixels Pointer
	void    SetPixelsPointer() {
		uint8 *pix00 = pixels00;
		uint8 *zbuf00 = zbuffer00;

		if (flipped) {
			pix00 += -pitch * (height - 1);
			zbuf00 += -zpitch * (height - 1);
		}

		pixels = pix00 + ox * bytes_per_pixel + oy * pitch;
		zbuffer = reinterpret_cast<uint16 *>(zbuf00 + ox + oy * zpitch);
	}

public:

	// Virtual Destructor
	~BaseSoftRenderSurface() override;

	//
	// Being/End Painting
	//

	// Begin painting to the buffer. MUST BE CALLED BEFORE DOING ANYTHING TO THE SURFACE!
	// Can be called multiple times
	// Returns Error Code on error. Check return code.....
	ECode BeginPainting() override;

	// Finish paining to the buffer. MUST BE CALLED FOR EACH CALL TO BeginPainting()
	// Returns Error Code on error. Check return code.....
	ECode EndPainting() override;

	// Get the surface as a Texture. Only valid for SecondaryRenderSurfaces
	Texture *GetSurfaceAsTexture() override;


	//
	// Surface Properties
	//

	// Set the Origin of the Surface
	void SetOrigin(int32 x, int32 y) override;

	// Set the Origin of the Surface
	void GetOrigin(int32 &x, int32 &y) const override;

	// Get the Surface Dimensions
	void GetSurfaceDims(Rect &) const override;

	// Get Clipping Rectangle
	void GetClippingRect(Rect &) const override;

	// Set Clipping Rectangle
	void SetClippingRect(const Rect &) override;

	// Check Clipped. -1 if off screen, 0 if not clipped, 1 if clipped
	int16 CheckClipped(const Rect &) const override;

	// Flip the surface
	void SetFlipped(bool flipped) override;

	// Has the render surface been flipped?
	bool IsFlipped() const override;

	//
	// Surface Palettes
	//
	// TODO: Make a Palette class
	// TODO: Handle Ultima8 and Crusader Xforms
	//

	// Set The Surface Palette
	// virtual void SetPalette(uint8 palette[768]);

	// Set The Surface Palette to be the one used by another surface
	// TODO: virtual void SetPalette(RenderSurface &);

	// Get The Surface Palette
	// TODO: virtual void GetPalette(uint8 palette[768]);

	void CreateNativePalette(Palette *palette) override;

	Graphics::ManagedSurface *getRawSurface() const override {
		return sdl_surf;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
