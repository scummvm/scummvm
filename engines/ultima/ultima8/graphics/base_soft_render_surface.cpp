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
// Desc: Constructor for BaseSoftRenderSurface from a managed surface
//
BaseSoftRenderSurface::BaseSoftRenderSurface(Graphics::ManagedSurface *s) :
	_pixels(nullptr), _pixels00(nullptr), _zBuffer(nullptr),
	_zBuffer00(nullptr), _bytesPerPixel(0), _bitsPerPixel(0), _formatType(0),
	_ox(0), _oy(0), _width(0), _height(0), _pitch(0), _zPitch(0),
	_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0),
	_surface(s), _rttTex(nullptr) {
	_clipWindow.setWidth(_width = _surface->w);
	_clipWindow.setHeight(_height = _surface->h);
	_pitch = _surface->pitch;
	_bitsPerPixel = _surface->format.bpp();
	_bytesPerPixel = _surface->format.bytesPerPixel;

	RenderSurface::_format.bytesPerPixel = _bytesPerPixel;
	RenderSurface::_format.rLoss = _surface->format.rLoss;
	RenderSurface::_format.gLoss = _surface->format.gLoss;
	RenderSurface::_format.bLoss = _surface->format.bLoss;
	RenderSurface::_format.aLoss = _surface->format.aLoss;
	RenderSurface::_format.rLoss16 = _format.rLoss + 8;
	RenderSurface::_format.gLoss16 = _format.gLoss + 8;
	RenderSurface::_format.bLoss16 = _format.bLoss + 8;
	RenderSurface::_format.aLoss16 = _format.aLoss + 8;
	RenderSurface::_format.rShift = _surface->format.rShift;
	RenderSurface::_format.gShift = _surface->format.gShift;
	RenderSurface::_format.bShift = _surface->format.bShift;
	RenderSurface::_format.aShift = _surface->format.aShift;
	RenderSurface::_format.rMask = _surface->format.rMax() << _surface->format.rShift;
	RenderSurface::_format.gMask = _surface->format.gMax() << _surface->format.gShift;
	RenderSurface::_format.bMask = _surface->format.bMax() << _surface->format.bShift;
	RenderSurface::_format.aMask = _surface->format.aMax() << _surface->format.aShift;

	SetPixelsPointer();

	// Trickery to get the alpha channel
	if (_format.aMask == 0 && _bytesPerPixel == 4) {
		uint32 mask = ~(_format.rMask | _format.gMask | _format.bMask);

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
		_format.aShift = first;
		_format.aLoss = 8 - (last + 1 - first);
		_format.aLoss16 = _format.aLoss + 8;
		_format.aMask = mask;
	}
}


//
// BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, int bpp, int rsft, int gsft, int bsft)
//
// Desc: Constructor for Generic BaseSoftRenderSurface
//
BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, int bpp,
        int rsft, int gsft, int bsft, int asft) :
	_pixels(nullptr), _pixels00(nullptr), _zBuffer(nullptr),
	_zBuffer00(nullptr), _bytesPerPixel(0), _bitsPerPixel(0), _formatType(0),
	_ox(0), _oy(0), _width(0), _height(0), _pitch(0), _zPitch(0),
	_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0), _surface(nullptr),
	_rttTex(nullptr) {
	_clipWindow.setWidth(_width = w);
	_clipWindow.setHeight(_height = h);

	switch (bpp) {
	case 15:
		_format.rLoss = 3;
		_format.gLoss = 3;
		_format.bLoss = 3;
		_format.aLoss = 7;
		bpp = 16;
		break;

	case 16:
		_format.rLoss = 3;
		_format.gLoss = 2;
		_format.bLoss = 3;
		_format.aLoss = 0;
		break;

	case 32:
		_format.rLoss = 0;
		_format.gLoss = 0;
		_format.bLoss = 0;
		_format.aLoss = 0;
		break;

	default:
		break;
	}

	_pitch = w * bpp / 8;
	_bitsPerPixel = bpp;
	_bytesPerPixel = bpp / 8;

	RenderSurface::_format.bytesPerPixel = _bytesPerPixel;
	RenderSurface::_format.rLoss16 = _format.rLoss + 8;
	RenderSurface::_format.gLoss16 = _format.gLoss + 8;
	RenderSurface::_format.bLoss16 = _format.bLoss + 8;
	RenderSurface::_format.aLoss16 = _format.aLoss + 8;
	RenderSurface::_format.rShift = rsft;
	RenderSurface::_format.gShift = gsft;
	RenderSurface::_format.bShift = bsft;
	RenderSurface::_format.aShift = asft;
	RenderSurface::_format.rMask = (0xFF >> _format.rLoss) << rsft;
	RenderSurface::_format.gMask = (0xFF >> _format.gLoss) << gsft;
	RenderSurface::_format.bMask = (0xFF >> _format.bLoss) << bsft;
	RenderSurface::_format.aMask = (0xFF >> _format.aLoss) << asft;

	SetPixelsPointer();
}


//
// BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, uint8 *buf)
//
// Desc: Constructor for Generic BaseSoftRenderSurface which matches screen params
//
BaseSoftRenderSurface::BaseSoftRenderSurface(int w, int h, uint8 *buf) :
	_pixels(nullptr), _pixels00(nullptr), _zBuffer(nullptr),
	_zBuffer00(nullptr), _bytesPerPixel(0), _bitsPerPixel(0), _formatType(0),
	_ox(0), _oy(0), _width(0), _height(0), _pitch(0), _zPitch(0),
	_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0),
	_surface(nullptr), _rttTex(nullptr) {
	_clipWindow.setWidth(_width = w);
	_clipWindow.setHeight(_height = h);

	int bpp = RenderSurface::_format.bpp();

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
	_pixels(nullptr), _pixels00(nullptr), _zBuffer(nullptr), _zBuffer00(nullptr),
	_bytesPerPixel(0), _bitsPerPixel(0), _formatType(0),
	_ox(0), _oy(0), _width(0), _height(0), _pitch(0), _zPitch(0),
	_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0), _surface(nullptr),
	_rttTex(nullptr) {
	_clipWindow.setWidth(_width = w);
	_clipWindow.setHeight(_height = h);

	int bpp = RenderSurface::_format.bpp();

	_pitch = w * bpp / 8;
	_bitsPerPixel = bpp;
	_bytesPerPixel = bpp / 8;
	_pixels00 = new uint8[_pitch * _height];

	_rttTex = new Texture;
	_rttTex->setPixels(_pixels00);
	_rttTex->w = _width;
	_rttTex->h = _height;
	_rttTex->_format = TEX_FMT_NATIVE;
	_rttTex->format = RenderSurface::getPixelFormat();
	_rttTex->pitch = _pitch;
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
		delete [] _pixels00;
		delete [] _zBuffer00;
	}
}


//
// BaseSoftRenderSurface::BeginPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
bool BaseSoftRenderSurface::BeginPainting() {
	if (!_lockCount) {

		if (_surface) {
			// Pixels pointer
			Graphics::Surface s = _surface->getSubArea(Common::Rect(0, 0, _surface->w, _surface->h));
			_pixels00 = static_cast<uint8 *>(s.getPixels());

			_pitch = _surface->pitch;
			if (_flipped) _pitch = -_pitch;
		} else  {
			if (!GenericLock())
				return false;
		}
	}

	_lockCount++;

	if (_pixels00 == nullptr) {
		error("Error: Surface Locked with NULL BaseSoftRenderSurface::_pixels pointer!");
		return false;
	}

	// Origin offset pointers
	SetPixelsPointer();

	// No error
	return true;
}


//
// BaseSoftRenderSurface::EndPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
bool BaseSoftRenderSurface::EndPainting() {
	// Already Unlocked
	if (!_lockCount) {
		error("Error: BeginPainting()/EndPainting() Mismatch!");
		return false;
	}

	// Decrement counter
	--_lockCount;

	if (!_lockCount) {
		if (_surface) {
			// Clear pointers
			_pixels = _pixels00 = 0;

			// Render the screen
			Graphics::Screen *screen = dynamic_cast<Graphics::Screen *>(_surface);
			assert(screen);
			screen->update();

		} else {
			if (!GenericUnlock())
				return false;
		}
	}

	// No error
	return true;
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
void BaseSoftRenderSurface::CreateNativePalette(Palette *palette, int maxindex) {
	if (maxindex == 0)
		maxindex = 256;
	for (int i = 0; i < maxindex; i++) {
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
	r.moveTo(_ox, _oy);
	r.setWidth(_width);
	r.setHeight(_height);
}

//
// void BaseSoftRenderSurface::SetOrigin(int32 x, int32 y)
//
// Desc: Set the Phyiscal Pixel to be the logical origin
//
void BaseSoftRenderSurface::SetOrigin(int32 x, int32 y) {
	// Adjust the clipping window
	_clipWindow.translate(_ox - x, _oy - y);

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
	_clipWindow.clip(Rect(-_ox, -_oy, -_ox + _width, -_oy + _height));
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
	r.clip(_clipWindow);

	// Clipped away to the void
	if (!r.isValidRect())
		return -1;
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

	_oy -= _clipWindow.top;
	_clipWindow.setHeight(_height - _clipWindow.top + _clipWindow.height());
	_oy += _clipWindow.top;

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
