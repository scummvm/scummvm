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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/soft_render_surface.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/graphics/texture.h"
#include "graphics/screen.h"

namespace Ultima {
namespace Ultima8 {

///////////////////////////
//                       //
// BaseSoftRenderSurface //
//                       //
///////////////////////////

//
// BaseSoftRenderSurface::BaseSoftRenderSurface(Graphics::Surface *s)
//
// Desc: Constructor for BaseSoftRenderSurface from a SDL_Surface
//
BaseSoftRenderSurface::BaseSoftRenderSurface(Graphics::ManagedSurface *s) :
	_pixels(0), _pixels00(0), _zBuffer(0), _zBuffer00(0),
	_bytesPerPixel(0), _bitsPerPixel(0), _formatType(0),
	_ox(0), _oy(0), _width(0), _height(0), _pitch(0), _zPitch(0),
	_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0),
	_sdlSurf(s), _rttTex(0) {
	_clipWindow.ResizeAbs(_width = _sdlSurf->w, _height = _sdlSurf->h);
	_pitch = _sdlSurf->pitch;
	_bitsPerPixel = _sdlSurf->format.bpp();
	_bytesPerPixel = _sdlSurf->format.bytesPerPixel;

	RenderSurface::format.s_bpp = _bitsPerPixel;
	RenderSurface::format.s_bytes_per_pixel = _bytesPerPixel;
	RenderSurface::format.r_loss = _sdlSurf->format.rLoss;
	RenderSurface::format.g_loss = _sdlSurf->format.gLoss;
	RenderSurface::format.b_loss = _sdlSurf->format.bLoss;
	RenderSurface::format.a_loss = _sdlSurf->format.aLoss;
	RenderSurface::format.r_loss16 = format.r_loss + 8;
	RenderSurface::format.g_loss16 = format.g_loss + 8;
	RenderSurface::format.b_loss16 = format.b_loss + 8;
	RenderSurface::format.a_loss16 = format.a_loss + 8;
	RenderSurface::format.r_shift = _sdlSurf->format.rShift;
	RenderSurface::format.g_shift = _sdlSurf->format.gShift;
	RenderSurface::format.b_shift = _sdlSurf->format.bShift;
	RenderSurface::format.a_shift = _sdlSurf->format.aShift;
	RenderSurface::format.r_mask = _sdlSurf->format.rMax() << _sdlSurf->format.rShift;
	RenderSurface::format.g_mask = _sdlSurf->format.gMax() << _sdlSurf->format.gShift;
	RenderSurface::format.b_mask = _sdlSurf->format.bMax() << _sdlSurf->format.bShift;
	RenderSurface::format.a_mask = _sdlSurf->format.aMax() << _sdlSurf->format.aShift;

	SetPixelsPointer();

	// Trickery to get the alpha channel
	if (format.a_mask == 0 && _bytesPerPixel == 4) {
		uint32 mask = ~(format.r_mask | format.g_mask | format.b_mask);

		// Using all bits????
		if (!mask) return;

		// Check the mask to make sure that it's 'sane'
		int i;
		int last = 0;
		int first = 0;
		int zero = 32;

		for (i = 0; i < 32; i++) {
			if ((1 << i) & mask) {
				last = first = i;
				break;
			}
		}

		for (; i < 32; i++) {
			if ((1 << i) & mask)
				last = i;
			else if (i < zero)
				zero = i;
		}

		// something screwy going on
		if (zero < last) return;

		// Set it
		format.a_shift = first;
		format.a_loss = 8 - (last + 1 - first);
		format.a_loss16 = format.a_loss + 8;
		format.a_mask = mask;
	}
}


//
// BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, int bpp, int rsft, int gsft, int bsft)
//
// Desc: Constructor for Generic BaseSoftRenderSurface
//
BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, int bpp,
        int rsft, int gsft, int bsft, int asft) :
	_pixels(0), _pixels00(0), _zBuffer(0), _zBuffer00(0),
	_bytesPerPixel(0), _bitsPerPixel(0), _formatType(0),
	_ox(0), _oy(0), _width(0), _height(0), _pitch(0), _zPitch(0),
	_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0), _sdlSurf(0), _rttTex(0) {
	_clipWindow.ResizeAbs(_width = w, _height = h);

	switch (bpp) {
	case 15:
		format.r_loss = 3;
		format.g_loss = 3;
		format.b_loss = 3;
		format.a_loss = 7;
		bpp = 16;
		break;

	case 16:
		format.r_loss = 3;
		format.g_loss = 2;
		format.b_loss = 3;
		format.a_loss = 0;
		break;

	case 32:
		format.r_loss = 0;
		format.g_loss = 0;
		format.b_loss = 0;
		format.a_loss = 0;
		break;

	default:
		break;
	}

	_pitch = w * bpp / 8;
	_bitsPerPixel = bpp;
	_bytesPerPixel = bpp / 8;

	RenderSurface::format.s_bpp = bpp;
	RenderSurface::format.s_bytes_per_pixel = _bytesPerPixel;
	RenderSurface::format.r_loss16 = format.r_loss + 8;
	RenderSurface::format.g_loss16 = format.g_loss + 8;
	RenderSurface::format.b_loss16 = format.b_loss + 8;
	RenderSurface::format.a_loss16 = format.a_loss + 8;
	RenderSurface::format.r_shift = rsft;
	RenderSurface::format.g_shift = gsft;
	RenderSurface::format.b_shift = bsft;
	RenderSurface::format.a_shift = asft;
	RenderSurface::format.r_mask = (0xFF >> format.r_loss) << rsft;
	RenderSurface::format.g_mask = (0xFF >> format.g_loss) << gsft;
	RenderSurface::format.b_mask = (0xFF >> format.b_loss) << bsft;
	RenderSurface::format.a_mask = (0xFF >> format.a_loss) << asft;

	SetPixelsPointer();
}


//
// BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, uint8 *buf)
//
// Desc: Constructor for Generic BaseSoftRenderSurface which matches screen params
//
BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, uint8 *buf) :
	_pixels(0), _pixels00(0), _zBuffer(0), _zBuffer00(0),
	_bytesPerPixel(0), _bitsPerPixel(0), _formatType(0),
	_ox(0), _oy(0), _width(0), _height(0), _pitch(0), _zPitch(0),
	_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0), _sdlSurf(0), _rttTex(0) {
	_clipWindow.ResizeAbs(_width = w, _height = h);

	int bpp = RenderSurface::format.s_bpp;

	_pitch = w * bpp / 8;
	_bitsPerPixel = bpp;
	_bytesPerPixel = bpp / 8;
	_pixels00 = buf;

	SetPixelsPointer();
}

//
// BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, uint8 *buf)
//
// Desc: Constructor for Generic BaseSoftRenderSurface which matches screen params
//
BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h) :
	_pixels(0), _pixels00(0), _zBuffer(0), _zBuffer00(0),
	_bytesPerPixel(0), _bitsPerPixel(0), _formatType(0),
	_ox(0), _oy(0), _width(0), _height(0), _pitch(0), _zPitch(0),
	_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0), _sdlSurf(0), _rttTex(0) {
	_clipWindow.ResizeAbs(_width = w, _height = h);

	int bpp = RenderSurface::format.s_bpp;

	_pitch = w * bpp / 8;
	_bitsPerPixel = bpp;
	_bytesPerPixel = bpp / 8;
	_pixels00 = new uint8[_pitch * _height];

	_rttTex = new Texture;
	_rttTex->_buffer = reinterpret_cast<uint32 *>(_pixels00);
	_rttTex->_width = _width;
	_rttTex->_height = _height;
	_rttTex->_format = TEX_FMT_NATIVE;
	_rttTex->CalcLOG2s();

	SetPixelsPointer();
}


//
// BaseSoftRenderSurface::~BaseSoftRenderSurface()
//
// Desc: Destructor
//
BaseSoftRenderSurface::~BaseSoftRenderSurface() {
	if (_rttTex) {
		delete _rttTex;
		_rttTex = 0;

		delete [] _pixels00;
		_pixels00 = 0;

		delete [] _zBuffer00;
		_zBuffer00 = 0;
	}
}


//
// BaseSoftRenderSurface::BeginPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
ECode BaseSoftRenderSurface::BeginPainting() {
	if (!_lockCount) {

		if (_sdlSurf) {
			// Pixels pointer
			Graphics::Surface s = _sdlSurf->getSubArea(Common::Rect(0, 0, _sdlSurf->w, _sdlSurf->h));
			_pixels00 = static_cast<uint8 *>(s.getPixels());

			_pitch = _sdlSurf->pitch;
			if (_flipped) _pitch = -_pitch;
		} else  {
			ECode ret = GenericLock();
			if (ret.failed()) return ret;
		}
	}

	_lockCount++;

	if (_pixels00 == 0) {
		// TODO: SetLastError(GR_SOFT_ERROR_LOCKED_NULL_PIXELS, "Surface Locked with NULL BaseSoftRenderSurface::_pixels pointer!");
		perr << "Error: Surface Locked with NULL BaseSoftRenderSurface::_pixels pointer!" << Std::endl;
		return GR_SOFT_ERROR_LOCKED_NULL_PIXELS;
	}

	// Origin offset pointers
	SetPixelsPointer();

	// No error
	return P_NO_ERROR;
}


//
// BaseSoftRenderSurface::EndPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
ECode BaseSoftRenderSurface::EndPainting() {
	// Already Unlocked
	if (!_lockCount) {
		// TODO: SetLastError(GR_SOFT_ERROR_BEGIN_END_MISMATCH, "BeginPainting()/EndPainting() Mismatch!");
		perr << "Error: BeginPainting()/EndPainting() Mismatch!" << Std::endl;
		return GR_SOFT_ERROR_BEGIN_END_MISMATCH;
	}

	// Decrement counter
	--_lockCount;

	if (!_lockCount) {
		if (_sdlSurf) {
			// Clear pointers
			_pixels = _pixels00 = 0;

			// Render the screen
			Graphics::Screen *screen = dynamic_cast<Graphics::Screen *>(_sdlSurf);
			assert(screen);
			screen->update();

		} else {
			ECode ret = GenericUnlock();
			if (ret.failed()) return ret;
		}
	}

	// No error
	return P_NO_ERROR;
}

//
// Texture *BaseSoftRenderSurface::GetSurfaceAsTexture()
//
// Desc: Get the surface as a Texture. Only valid for SecondaryRenderSurfaces
//
Texture *BaseSoftRenderSurface::GetSurfaceAsTexture() {
	if (!_rttTex) {
		perr << "Error: GetSurfaceAsTexture(): Surface doesn't render-to-texture" << Std::endl;
	}

	return _rttTex;
}

//
// void BaseSoftRenderSurface::CreateNativePalette(Palette* palette)
//
// Desc: Create a palette of colours native to the surface
//
void BaseSoftRenderSurface::CreateNativePalette(Palette *palette) {
	for (int i = 0; i < 256; i++) {
		int32 r, g, b;

		// Normal palette
		palette->_native_untransformed[i] = PACK_RGB8(palette->_palette[i * 3 + 0],
		                                   palette->_palette[i * 3 + 1],
		                                   palette->_palette[i * 3 + 2]);

		r = palette->_matrix[0] * palette->_palette[i * 3 + 0] +
		    palette->_matrix[1] * palette->_palette[i * 3 + 1] +
		    palette->_matrix[2] * palette->_palette[i * 3 + 2] +
		    palette->_matrix[3] * 255;
		if (r < 0) r = 0;
		if (r > 0x7F800) r = 0x7F800;

		g = palette->_matrix[4] * palette->_palette[i * 3 + 0] +
		    palette->_matrix[5] * palette->_palette[i * 3 + 1] +
		    palette->_matrix[6] * palette->_palette[i * 3 + 2] +
		    palette->_matrix[7] * 255;
		if (g < 0) g = 0;
		if (g > 0x7F800) g = 0x7F800;

		b = palette->_matrix[8] * palette->_palette[i * 3 + 0] +
		    palette->_matrix[9] * palette->_palette[i * 3 + 1] +
		    palette->_matrix[10] * palette->_palette[i * 3 + 2] +
		    palette->_matrix[11] * 255;
		if (b < 0) b = 0;
		if (b > 0x7F800) b = 0x7F800;

		// Transformed normal palette
		// FIXME - Wont work on non SDL SRS Implementations
		palette->_native[i] = PACK_RGB8(static_cast<uint8>(r >> 11),
		                               static_cast<uint8>(g >> 11),
		                               static_cast<uint8>(b >> 11));

		// Transformed XFORM palette (Uses the TEX32 format)
		if (TEX32_A(palette->_xform_untransformed[i])) {
			r = palette->_matrix[0] * TEX32_R(palette->_xform_untransformed[i]) +
			    palette->_matrix[1] * TEX32_G(palette->_xform_untransformed[i]) +
			    palette->_matrix[2] * TEX32_B(palette->_xform_untransformed[i]) +
			    palette->_matrix[3] * 255;
			if (r < 0) r = 0;
			if (r > 0x7F800) r = 0x7F800;

			g = palette->_matrix[4] * TEX32_R(palette->_xform_untransformed[i]) +
			    palette->_matrix[5] * TEX32_G(palette->_xform_untransformed[i]) +
			    palette->_matrix[6] * TEX32_B(palette->_xform_untransformed[i]) +
			    palette->_matrix[7] * 255;
			if (g < 0) g = 0;
			if (g > 0x7F800) g = 0x7F800;

			b = palette->_matrix[8] * TEX32_R(palette->_xform_untransformed[i]) +
			    palette->_matrix[9] * TEX32_G(palette->_xform_untransformed[i]) +
			    palette->_matrix[10] * TEX32_B(palette->_xform_untransformed[i]) +
			    palette->_matrix[11] * 255;
			if (b < 0) b = 0;
			if (b > 0x7F800) b = 0x7F800;

			palette->_xform[i] = TEX32_PACK_RGBA(static_cast<uint8>(r >> 11),
			                                    static_cast<uint8>(g >> 11),
			                                    static_cast<uint8>(b >> 11),
			                                    TEX32_A(palette->_xform_untransformed[i]));
		} else
			palette->_xform[i] = 0;
	}
}

//
// void BaseSoftRenderSurface::GetSurfaceDims(Rect &r)
//
// Desc: Get the Surface Dimentions (and logical origin)
// r: Rect object to fill
//
void BaseSoftRenderSurface::GetSurfaceDims(Rect &r) const {
	r.Set(_ox, _oy, _width, _height);
}

//
// void BaseSoftRenderSurface::SetOrigin(int32 x, int32 y)
//
// Desc: Set the Phyiscal Pixel to be the logical origin
//
void BaseSoftRenderSurface::SetOrigin(int32 x, int32 y) {
	// Adjust the clipping window
	_clipWindow.MoveRel(_ox - x, _oy - y);

	// Set the origin
	_ox = x;
	_oy = y;

	// The new pointers
	SetPixelsPointer();
}

//
// void BaseSoftRenderSurface::GetOrigin(int32 &x, int32 &y)
//
// Desc: Get the Phyiscal Pixel that is the logical origin
//
void BaseSoftRenderSurface::GetOrigin(int32 &x, int32 &y) const {
	// Set the origin
	x = _ox;
	y = _oy;
}

//
// void BaseSoftRenderSurface::GetClippingRect(Rect &r)
//
// Desc: Get the Clipping Rectangle
// r: Rect object to fill
//
void BaseSoftRenderSurface::GetClippingRect(Rect &r) const {
	r = _clipWindow;
}

//
// void BaseSoftRenderSurface::GetClippingRect(Rect &r)
//
// Desc: Set the Clipping Rectangle
// r: Rect object that contains new Clipping Rectangle
//
void BaseSoftRenderSurface::SetClippingRect(const Rect &r) {
	// What we need to do is to clip the clipping rect to the phyiscal screen
	_clipWindow = r;
	_clipWindow.Intersect(-_ox, -_oy, _width, _height);
}

//
// int16 BaseSoftRenderSurface::CheckClipped(Rect &r)
//
// Desc: Check for a clipped rectangle
// Returns: -1 if off screen,
//           0 if not clipped,
//           1 if clipped
//
int16 BaseSoftRenderSurface::CheckClipped(const Rect &c) const {
	Rect r = c;
	r.Intersect(_clipWindow);

	// Clipped away to the void
	if (!r.IsValid()) return -1;
	else if (r == c) return 0;
	else return 1;
}

//
// void BaseSoftRenderSurface::SetFlipped(bool _flipped)
//
// Desc: Flip the surface
//
void BaseSoftRenderSurface::SetFlipped(bool wantFlipped) {
	// Flipping is not terrible complex
	// But is a bit of a pain to set up

	// First we check to see if we are currently _flipped
	if (wantFlipped == _flipped) return;

	_flipped = wantFlipped;

	// What we 'need' to do is negate the pitches, and flip the clipping window
	// We keep the 'origin' in the same position relative to the clipping window

	_oy -= _clipWindow.y;
	_clipWindow.y = _height - (_clipWindow.y + _clipWindow.h);
	_oy += _clipWindow.y;

	_pitch = -_pitch;
	_zPitch = -_zPitch;

	SetPixelsPointer();

}

//
// bool BaseSoftRenderSurface::IsFlipped() const
//
// Desc: Has the render surface been _flipped?
//
bool BaseSoftRenderSurface::IsFlipped() const {
	return _flipped;
}

} // End of namespace Ultima8
} // End of namespace Ultima
