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

#include <cstdlib>

#include "ultima/ultima6/core/nuvie_defs.h"

#include "Surface.h"
#include "SDL.h"

#ifdef WANT_OPENGL
#include "OpenGL.h"
#endif

namespace Ultima {
namespace Ultima6 {

// Colour shifting values
uint8  RenderSurface::Rloss;
uint8  RenderSurface::Gloss;
uint8  RenderSurface::Bloss;
uint8  RenderSurface::Rloss16;
uint8  RenderSurface::Gloss16;
uint8  RenderSurface::Bloss16;
uint8  RenderSurface::Rshift;
uint8  RenderSurface::Gshift;
uint8  RenderSurface::Bshift;
uint32 RenderSurface::Rmask;
uint32 RenderSurface::Gmask;
uint32 RenderSurface::Bmask;

// Default constructor for no created surface
RenderSurface::RenderSurface() : buffer(0), zbuffer_priv(0), sdl_surface(NULL), opengl(0),
	bytes_per_pixel(0), bits_per_pixel(0), format_type(0),
	pixels(0), zbuffer(0), w(0), h(0), pitch(0),
	gl(0), gr(0), gt(0), gb(0),  lock_count(0) {
}

// Constructor for custom buffer
RenderSurface::RenderSurface(uint32 width, uint32 height, uint32 bpp, uint8 *p) : buffer(0), zbuffer_priv(0), sdl_surface(NULL), opengl(0),
	bytes_per_pixel(bpp / 8), bits_per_pixel(bpp),
	pixels(p), zbuffer(0), w(width), h(height), pitch(width),
	gl(0), gr(width), gt(0), gb(height), lock_count(0) {
	// Set default formats for the buffer
	if (bpp == 32) set_format888();
	else set_format565();
}

// Constructor for generic surface (with optional guardband)
RenderSurface::RenderSurface(uint32 width, uint32 height, uint32 bpp, sint32 guard) : buffer(0), zbuffer_priv(0), sdl_surface(NULL), opengl(0),
	bytes_per_pixel(bpp / 8), bits_per_pixel(bpp),
	pixels(0), zbuffer(0), w(width), h(height), pitch(width * (bpp / 8) + 2 * guard * (bpp / 8)),
	gl(-guard), gr(guard + width), gt(-guard), gb(guard + height), lock_count(0) {
	// Set default formats for the buffer
	if (bpp == 32) set_format888();
	else set_format565();

	buffer = new uint8[pitch * (height + 2 * gb)];
	pixels = buffer + (pitch * gb) + gb;
}

// Constructor for sdl surface
RenderSurface::RenderSurface(Graphics::ManagedSurface *surf) : buffer(0), zbuffer_priv(0), sdl_surface(NULL), opengl(0),
	bytes_per_pixel(0), bits_per_pixel(0),
	pixels((uint8 *) surf->pixels), zbuffer(0), w(surf->w), h(surf->h), pitch(surf->pitch),
	gl(0), gr(surf->w), gt(0), gb(surf->h), lock_count(0) {
	set_format(surf->format);
}

// Constructor for opengl surface
RenderSurface::RenderSurface(OpenGL *ogl) : buffer(0), zbuffer_priv(0), sdl_surface(NULL), opengl(ogl),
	bytes_per_pixel(0), bits_per_pixel(0), format_type(0),
	pixels(0), zbuffer(0), w(0), h(0), pitch(0),
	gl(0), gr(0), gt(0), gb(0), lock_count(0) {
}

RenderSurface::~RenderSurface() {
	if (buffer) delete [] buffer;
	if (zbuffer_priv) delete [] zbuffer_priv;
	if (sdl_surface) SDL_FreeSurface(sdl_surface);
}

//
// Set the buffer format from SDL_PixelFormat
//
void RenderSurface::set_format(const SDL_PixelFormat *fmt) {
	bits_per_pixel = fmt->BitsPerPixel;
	bytes_per_pixel = fmt->BytesPerPixel;

	Rloss = fmt->Rloss;
	Gloss = fmt->Gloss;
	Bloss = fmt->Bloss;
	Rloss16 = Rloss + 8;
	Gloss16 = Gloss + 8;
	Bloss16 = Bloss + 8;
	Rshift = fmt->Rshift;
	Gshift = fmt->Gshift;
	Bshift = fmt->Bshift;
	Rmask = fmt->Rmask;
	Gmask = fmt->Gmask;
	Bmask = fmt->Bmask;

	// RGB 565
	if (Rmask == 0xf800 && Gmask == 0x7e0 && Bmask == 0x1f)
		format_type = 565;
	// RGB 555
	else if (Rmask == 0x7c00 && Gmask == 0x3e0 && Bmask == 0x1f)
		format_type = 555;
	// RGB 888
	else if (Rmask == 0xFF0000 && Gmask == 0x00FF00 && Bmask == 0x0000FF)
		format_type = 888;
	// RGB 16 Bit Generic
	else if (bits_per_pixel == 16)
		format_type = 16;
	// RGB 32 Bit Generic
	else
		format_type = 32;
}

//
// Set a custom 565 format
//
void RenderSurface::set_format565(int rsft, int gsft, int bsft) {
	bits_per_pixel = 16;
	bytes_per_pixel = 2;

	// Static Colour shifting values
	Rloss = 3;
	Gloss = 2;
	Bloss = 3;
	Rloss16 = Rloss + 8;
	Gloss16 = Gloss + 8;
	Bloss16 = Bloss + 8;
	Rshift = rsft;
	Gshift = gsft;
	Bshift = bsft;
	Rmask = 0x1f << rsft;
	Gmask = 0x2f << rsft;
	Bmask = 0x1f << rsft;

	// RGB 565
	if (Rmask == 0xf800 && Gmask == 0x7e0 && Bmask == 0x1f)
		format_type = 565;
	// RGB 16 Bit Generic
	else if (bits_per_pixel == 16)
		format_type = 16;
}

//
// Set a custom 555 format
//
void RenderSurface::set_format555(int rsft, int gsft, int bsft) {
	bits_per_pixel = 16;
	bytes_per_pixel = 2;

	// Static Colour shifting values
	Rloss = 3;
	Gloss = 3;
	Bloss = 3;
	Rloss16 = Rloss + 8;
	Gloss16 = Gloss + 8;
	Bloss16 = Bloss + 8;
	Rshift = rsft;
	Gshift = gsft;
	Bshift = bsft;
	Rmask = 0x1f << rsft;
	Gmask = 0x1f << rsft;
	Bmask = 0x1f << rsft;

	// RGB 555
	if (Rmask == 0x7c00 && Gmask == 0x3e0 && Bmask == 0x1f)
		format_type = 555;
	// RGB 16 Bit Generic
	else if (bits_per_pixel == 16)
		format_type = 16;
}

//
// Set a custom 888 format
//
void RenderSurface::set_format888(int rsft, int gsft, int bsft) {
	bits_per_pixel = 32;
	bytes_per_pixel = 4;

	Rloss = 0;
	Gloss = 0;
	Bloss = 0;
	Rloss16 = Rloss + 8;
	Gloss16 = Gloss + 8;
	Bloss16 = Bloss + 8;
	Rshift = rsft;
	Gshift = gsft;
	Bshift = bsft;
	Rmask = 0xFF << rsft;
	Gmask = 0xFF << rsft;
	Bmask = 0xFF << rsft;

	// RGB 888
	if (Rmask == 0xFF0000 && Gmask == 0x00FF00 && Bmask == 0x0000FF)
		format_type = 888;
	// RGB 32 Bit Generic
	else
		format_type = 32;
}

void RenderSurface::draw_line(int sx, int sy, int ex, int ey, unsigned char col) {
	if (bytes_per_pixel == 4) draw_line32(sx, sy, ex, ey, col);
	else draw_line16(sx, sy, ex, ey, col);
}

#define LINE_FRACTION 65536L

void RenderSurface::draw_line16(int sx, int sy, int ex, int ey, unsigned char col) {
#ifdef WANT_OPENGL
	if (opengl) {
		opengl->draw_line(sx, sy + 1, 0, ex, ey + 1, 0, col);
		return;
	}
#endif

	int xinc = 1;
	int yinc = 1;

	if (sx == ex) {
		sx --;
		if (sy > ey) {
			yinc = -1;
			sy--;
		}
	} else {
		if (sx > ex) {
			sx--;
			xinc = -1;
		} else {
			ex--;
		}

		if (sy > ey) {
			yinc = -1;
			sy--;
			ey--;
		}
	}

	uint16 *pixptr = (uint16 *)(pixels + pitch * sy + sx * 2);
	uint16 *pixend = (uint16 *)(pixels + pitch * ey + ex * 2);
	int pitch = this->pitch * yinc / 2;

	int cury = sy;
	int curx = sx;
	int width = w;
	int height = h;
	bool no_clip = true;

	if (sx >= width && ex >= width) return;
	if (sy >= height && ey >= height) return;
	if (sx < 0 && ex < 0) return;
	if (sy < 0 && ey < 0) return;

	if (sy < 0 || sy >= height || sx < 0 || sx >= width) no_clip = false;
	if (ey < 0 || ey >= height || ex < 0 || ex >= width) no_clip = false;

	int col32 = colour32[col];

	// vertical
	if (sx == ex) {
		//std::cout << "Vertical" << std::endl;
		// start is below end
		while (pixptr != pixend) {
			if (no_clip || (cury >= 0 && cury < height)) *pixptr = col32;
			pixptr += pitch;
			cury += yinc;
		}
	}
	// Horizontal
	else if (sy == ey) {
		//std::cout << "Horizontal" << std::endl;
		while (pixptr != pixend) {
			if (no_clip || (curx >= 0 && curx < width)) *pixptr = col32;
			pixptr += xinc;
			curx += xinc;
		}
	}
	// Diagonal xdiff >= ydiff
	else if (std::labs(sx - ex) >= std::labs(sy - ey)) {
		//std::cout << "Diagonal 1" << std::endl;
		uint32 fraction = std::labs((LINE_FRACTION * (sy - ey)) / (sx - ex));
		uint32 ycounter = 0;

		for (; ;) {
			if ((no_clip || (cury >= 0 && cury < height && curx >= 0 && curx < width)))
				*pixptr = col32;
			pixptr += xinc;
			if (curx == ex) break;
			curx  += xinc;
			ycounter += fraction;

			// Need to work out if we need to change line
			if (ycounter > LINE_FRACTION) {
				ycounter -= LINE_FRACTION;
				pixptr += pitch;
				cury  += yinc;
			}
		}
	}
	// Diagonal ydiff > xdiff
	else {
		//std::cout << "Diagonal 2" << std::endl;
		uint32 fraction = std::labs((LINE_FRACTION * (sx - ex)) / (sy - ey));
		uint32 xcounter = 0;

		for (; ;) {
			if ((no_clip || (cury >= 0 && cury < height && curx >= 0 && curx < width)))
				*pixptr = col32;
			pixptr += pitch;
			if (cury == ey) break;
			cury  += yinc;
			xcounter += fraction;

			// Need to work out if we need to change line
			if (xcounter > LINE_FRACTION) {
				xcounter -= LINE_FRACTION;
				pixptr += xinc;
				curx += xinc;
			}
		}
	}

}

void RenderSurface::draw_line32(int sx, int sy, int ex, int ey, unsigned char col) {
#ifdef WANT_OPENGL
	if (opengl) {
		opengl->draw_line(sx, sy + 1, 0, ex, ey + 1, 0, col);
		return;
	}
#endif

	int xinc = 1;
	int yinc = 1;

	if (sx == ex) {
		sx --;
		if (sy > ey) {
			yinc = -1;
			sy--;
		}
	} else {
		if (sx > ex) {
			sx--;
			xinc = -1;
		} else {
			ex--;
		}

		if (sy > ey) {
			yinc = -1;
			sy--;
			ey--;
		}
	}

	uint32 *pixptr = (uint32 *)(pixels + pitch * sy + sx * 4);
	uint32 *pixend = (uint32 *)(pixels + pitch * ey + ex * 4);
	int pitch = this->pitch * yinc / 4;

	int cury = sy;
	int curx = sx;
	int width = w;
	int height = h;
	bool no_clip = true;

	if (sx >= width && ex >= width) return;
	if (sy >= height && ey >= height) return;
	if (sx < 0 && ex < 0) return;
	if (sy < 0 && ey < 0) return;

	if (sy < 0 || sy >= height || sx < 0 || sx >= width) no_clip = false;
	if (ey < 0 || ey >= height || ex < 0 || ex >= width) no_clip = false;

	int col32 = colour32[col];

	// vertical
	if (sx == ex) {
		//std::cout << "Vertical" << std::endl;
		// start is below end
		while (pixptr != pixend) {
			if (no_clip || (cury >= 0 && cury < height)) *pixptr = col32;
			pixptr += pitch;
			cury += yinc;
		}
	}
	// Horizontal
	else if (sy == ey) {
		//std::cout << "Horizontal" << std::endl;
		while (pixptr != pixend) {
			if (no_clip || (curx >= 0 && curx < width)) *pixptr = col32;
			pixptr += xinc;
			curx += xinc;
		}
	}
	// Diagonal xdiff >= ydiff
	else if (std::labs(sx - ex) >= std::labs(sy - ey)) {
		//std::cout << "Diagonal 1" << std::endl;
		uint32 fraction = std::labs((LINE_FRACTION * (sy - ey)) / (sx - ex));
		uint32 ycounter = 0;

		for (; ;) {
			if ((no_clip || (cury >= 0 && cury < height && curx >= 0 && curx < width)))
				*pixptr = col32;
			pixptr += xinc;
			if (curx == ex) break;
			curx  += xinc;
			ycounter += fraction;

			// Need to work out if we need to change line
			if (ycounter > LINE_FRACTION) {
				ycounter -= LINE_FRACTION;
				pixptr += pitch;
				cury  += yinc;
			}
		}
	}
	// Diagonal ydiff > xdiff
	else {
		//std::cout << "Diagonal 2" << std::endl;
		uint32 fraction = std::labs((LINE_FRACTION * (sx - ex)) / (sy - ey));
		uint32 xcounter = 0;

		for (; ;) {
			if ((no_clip || (cury >= 0 && cury < height && curx >= 0 && curx < width)))
				*pixptr = col32;
			pixptr += pitch;
			if (cury == ey) break;
			cury  += yinc;
			xcounter += fraction;

			// Need to work out if we need to change line
			if (xcounter > LINE_FRACTION) {
				xcounter -= LINE_FRACTION;
				pixptr += xinc;
				curx += xinc;
			}
		}
	}

}

//
//
//
void RenderSurface::draw_3d_line(int x, int y, int sx, int sy, int sz, int ex, int ey, int ez, unsigned char col) {
	int dispsx = x + (sx - sy) / 4;
	int dispsy = y + (sx + sy) / 8 - sz;
	int dispex = x + (ex - ey) / 4;
	int dispey = y + (ex + ey) / 8 - ez;

#ifdef WANT_OPENGL
	if (opengl) opengl->draw_line(dispsx, dispsy + 1, 0, dispex, dispey + 1, 0, col);
	else
#endif
		draw_line(dispsx, dispsy + 1, dispex, dispey + 1, col);
	//draw_line (0, 0, 800, 600, col);
}

void RenderSurface::create_zbuffer() {
#ifdef ENABLE_SOFTREND_EMU
	SoftRend::SetupInitialState();
#endif

	// Not in opengl, or if we alraedy have one
	if (opengl || zbuffer_priv) return;

	zbuffer = zbuffer_priv = new uint16[pitch * h];
}

RenderSurface *CreateRenderSurface(uint32 width, uint32 height, uint32 bpp, uint8 *p) {
	return new RenderSurface(width, height, bpp, p);
}

RenderSurface *CreateRenderSurface(uint32 width, uint32 height, uint32 bpp, sint32 gb) {
	return new RenderSurface(width, height, bpp, gb);
}

RenderSurface *CreateRenderSurface(Graphics::ManagedSurface *surf) {
	return new RenderSurface(surf);

}

RenderSurface *CreateRenderSurface(OpenGL *ogl) {
	return new RenderSurface(ogl);
}

Graphics::ManagedSurface *RenderSurface::get_sdl_surface() {
	if (sdl_surface == NULL)
		sdl_surface = SDL_CreateRGBSurfaceFrom(pixels, w, h, bits_per_pixel, pitch, Rmask, Gmask, Bmask, 0);

	return sdl_surface;
}

const unsigned char *RenderSurface::get_pixels() {
	return ((const unsigned char *)pixels);
}

} // End of namespace Ultima6
} // End of namespace Ultima
