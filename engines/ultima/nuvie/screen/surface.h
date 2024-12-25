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

#ifndef NUVIE_SCREEN_SURFACE_H
#define NUVIE_SCREEN_SURFACE_H

#include "graphics/pixelformat.h"

namespace Ultima {
namespace Nuvie {

class OpenGL;

class RenderSurface {
private:
	uint8   *buffer;                // If the buffer is created, this is it
	uint16  *zbuffer_priv;
	Graphics::ManagedSurface *_rawSurface;
	DisposeAfterUse::Flag _disposeSurface;
public:
	OpenGL  *opengl;                // OpenGL surface

	// Pixel Format (also see 'Colour shifting values' later)
	int     bytes_per_pixel;        // 2 or 4
	int     bits_per_pixel;         // 16 or 32
	int     format_type;            // 16, 555, 565, 32 or 888

	uint8   *pixels;                // What we draw to
	uint16  *zbuffer;               // Z Buffer

	uint32  colour32[256];          // Palette as 16/32 bit colours

	// Dimensions
	uint32  w, h;                   // Surface width and height
	uint32  pitch;                  // Surface pitch

	// Guardband
	uint32  gl, gr;                 // Guard left and right (left goes negetive)
	uint32  gt, gb;                 // Guard top and bottom (up goes negetive)

	uint32  lock_count;             // Number of locks on surface


public:

	// Default constructor for no created surface
	RenderSurface();

	// Constructor for custom buffer
	RenderSurface(uint32 width, uint32 height, uint32 bpp, uint8 *p);

	// Constructor for surface (with optional guardband)
	RenderSurface(uint32 width, uint32 height, uint32 bpp, sint32 gb = 0);

	// Constructor for sdl surface
	RenderSurface(Graphics::ManagedSurface *surf);

	// Constructor for opengl surface
	RenderSurface(OpenGL *ogl);


	// Destructor
	virtual ~RenderSurface();

	// Create a 16 Bit Z Buffer for this surface
	void create_zbuffer();

	// Set the buffer format from Graphics::PixelFormat
	void set_format(const Graphics::PixelFormat *format);

	// Set a custom 565 format
	void set_format565(int rsft = 11, int gsft = 5, int bsft = 0);

	// Set a custom 555 format
	void set_format555(int rsft = 10, int gsft = 5, int bsft = 0);

	// Set a custom 888 format
	void set_format888(int rsft = 16, int gsft = 8, int bsft = 0);

	// Get the pixel format
	const Graphics::PixelFormat &getFormat() const {
		return _rawSurface->format;
	}

	// Draw Lines
	void draw_line(int sx, int sy, int ex, int ey, unsigned char col);
	void draw_3d_line(int x, int y, int sx, int sy, int sz, int ex, int ey, int ez, unsigned char col);

	// Colour shifting values
	static uint8  Rloss;
	static uint8  Gloss;
	static uint8  Bloss;
	static uint8  Rloss16;
	static uint8  Gloss16;
	static uint8  Bloss16;
	static uint8  Rshift;
	static uint8  Gshift;
	static uint8  Bshift;
	static uint32 Rmask;
	static uint32 Gmask;
	static uint32 Bmask;


	//
	// Shape Painting
	//

	// Display a shape. Non translucent
	//virtual void display(Shape* s, int x, int y, FrameID frame) = 0;

	// Display a shape. Translucent
//	virtual void display_translucent(Shape* s, int x, int y, FrameID frame) = 0;

	// Display a shape. Flipped
//	virtual void display_flipped(Shape* s, int x, int y, FrameID frame, bool trans = false) = 0;



	//FIX virtual void display8(uint8 *buf, int x, int y) = 0;

	Graphics::ManagedSurface *get_sdl_surface();
	const unsigned char *get_pixels();

	static Graphics::ManagedSurface *createSurface(int w, int h,
		const Graphics::PixelFormat &format);
private:

	// Draw Lines
	void draw_line16(int sx, int sy, int ex, int ey, unsigned char col);

	// Draw Lines
	void draw_line32(int sx, int sy, int ex, int ey, unsigned char col);

};

RenderSurface *CreateRenderSurface(uint32 width, uint32 height, uint32 bpp, uint8 *p);
RenderSurface *CreateRenderSurface(uint32 width, uint32 height, uint32 bpp, sint32 gb = 0);
RenderSurface *CreateRenderSurface(Graphics::ManagedSurface *surf);
RenderSurface *CreateRenderSurface(OpenGL *ogl);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
