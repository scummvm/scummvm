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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/soft_render_surface.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/ultima8.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/screen.h"

namespace Ultima {
namespace Ultima8 {

uint8 RenderSurface::_gamma10toGamma22[256];
uint8 RenderSurface::_gamma22toGamma10[256];

RenderSurface::RenderSurface(Graphics::ManagedSurface *s) : _pixels(nullptr), _pixels00(nullptr),
															_ox(0), _oy(0), _width(0), _height(0), _pitch(0),
															_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0),
															_surface(s) {
	_clipWindow.setWidth(_width = _surface->w);
	_clipWindow.setHeight(_height = _surface->h);
	_pitch = _surface->pitch;

	SetPixelsPointer();
}

//
// RenderSurface::~RenderSurface()
//
// Desc: Destructor
//
RenderSurface::~RenderSurface() {
}

void RenderSurface::SetPixelsPointer()
{
	uint8 *pix00 = _pixels00;

	if (_flipped) {
		pix00 += -_pitch * (_height - 1);
	}

	_pixels = pix00 + _ox * _surface->format.bytesPerPixel + _oy * _pitch;
}

//
// RenderSurface::BeginPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
bool RenderSurface::BeginPainting() {
	if (!_lockCount) {

		if (_surface) {
			// Pixels pointer
			Graphics::Surface s = _surface->getSubArea(Common::Rect(0, 0, _surface->w, _surface->h));
			_pixels00 = static_cast<uint8 *>(s.getPixels());

			_pitch = _surface->pitch;
			if (_flipped)
				_pitch = -_pitch;
		}
		// else, nothing to lock.
	}

	_lockCount++;

	if (_pixels00 == nullptr) {
		error("Error: Surface Locked with NULL RenderSurface::_pixels pointer!");
		return false;
	}

	// Origin offset pointers
	SetPixelsPointer();

	// No error
	return true;
}

//
// RenderSurface::EndPainting()
//
// Desc: Prepare the surface for drawing this frame (in effect lock it for drawing)
// Returns: Non Zero on error
//
bool RenderSurface::EndPainting() {
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

			// Render the screen if this is it (slight hack..)
			Graphics::Screen *screen = dynamic_cast<Graphics::Screen *>(_surface);
			if (screen)
				screen->update();
		}
		// else, nothing to unlock.
	}

	// No error
	return true;
}

//
// void RenderSurface::CreateNativePalette(Palette* palette)
//
// Desc: Create a palette of colours native to the surface
//
void RenderSurface::CreateNativePalette(Palette *palette, int maxindex) {
	const Graphics::PixelFormat &format = _surface->format;
	if (maxindex == 0)
		maxindex = 256;
	for (int i = 0; i < maxindex; i++) {
		int32 r, g, b;

		// Normal palette
		palette->_native_untransformed[i] = format.RGBToColor(palette->_palette[i * 3 + 0],
															  palette->_palette[i * 3 + 1],
															  palette->_palette[i * 3 + 2]);

		r = palette->_matrix[0] * palette->_palette[i * 3 + 0] +
			palette->_matrix[1] * palette->_palette[i * 3 + 1] +
			palette->_matrix[2] * palette->_palette[i * 3 + 2] +
			palette->_matrix[3] * 255;
		if (r < 0)
			r = 0;
		if (r > 0x7F800)
			r = 0x7F800;

		g = palette->_matrix[4] * palette->_palette[i * 3 + 0] +
			palette->_matrix[5] * palette->_palette[i * 3 + 1] +
			palette->_matrix[6] * palette->_palette[i * 3 + 2] +
			palette->_matrix[7] * 255;
		if (g < 0)
			g = 0;
		if (g > 0x7F800)
			g = 0x7F800;

		b = palette->_matrix[8] * palette->_palette[i * 3 + 0] +
			palette->_matrix[9] * palette->_palette[i * 3 + 1] +
			palette->_matrix[10] * palette->_palette[i * 3 + 2] +
			palette->_matrix[11] * 255;
		if (b < 0)
			b = 0;
		if (b > 0x7F800)
			b = 0x7F800;

		// Transformed normal palette
		// FIXME - Wont work on non SDL SRS Implementations
		palette->_native[i] = format.RGBToColor(static_cast<uint8>(r >> 11),
												static_cast<uint8>(g >> 11),
												static_cast<uint8>(b >> 11));

		// Transformed XFORM palette (Uses the TEX32 format)
		if (TEX32_A(palette->_xform_untransformed[i])) {
			r = palette->_matrix[0] * TEX32_R(palette->_xform_untransformed[i]) +
				palette->_matrix[1] * TEX32_G(palette->_xform_untransformed[i]) +
				palette->_matrix[2] * TEX32_B(palette->_xform_untransformed[i]) +
				palette->_matrix[3] * 255;
			if (r < 0)
				r = 0;
			if (r > 0x7F800)
				r = 0x7F800;

			g = palette->_matrix[4] * TEX32_R(palette->_xform_untransformed[i]) +
				palette->_matrix[5] * TEX32_G(palette->_xform_untransformed[i]) +
				palette->_matrix[6] * TEX32_B(palette->_xform_untransformed[i]) +
				palette->_matrix[7] * 255;
			if (g < 0)
				g = 0;
			if (g > 0x7F800)
				g = 0x7F800;

			b = palette->_matrix[8] * TEX32_R(palette->_xform_untransformed[i]) +
				palette->_matrix[9] * TEX32_G(palette->_xform_untransformed[i]) +
				palette->_matrix[10] * TEX32_B(palette->_xform_untransformed[i]) +
				palette->_matrix[11] * 255;
			if (b < 0)
				b = 0;
			if (b > 0x7F800)
				b = 0x7F800;

			palette->_xform[i] = TEX32_PACK_RGBA(static_cast<uint8>(r >> 11),
												 static_cast<uint8>(g >> 11),
												 static_cast<uint8>(b >> 11),
												 TEX32_A(palette->_xform_untransformed[i]));
		} else
			palette->_xform[i] = 0;
	}
}

//
// void RenderSurface::GetSurfaceDims(Rect &r)
//
// Desc: Get the Surface Dimentions (and logical origin)
// r: Rect object to fill
//
void RenderSurface::GetSurfaceDims(Rect &r) const {
	r.moveTo(_ox, _oy);
	r.setWidth(_width);
	r.setHeight(_height);
}

//
// void RenderSurface::SetOrigin(int32 x, int32 y)
//
// Desc: Set the Phyiscal Pixel to be the logical origin
//
void RenderSurface::SetOrigin(int32 x, int32 y) {
	// Adjust the clipping window
	_clipWindow.translate(_ox - x, _oy - y);

	// Set the origin
	_ox = x;
	_oy = y;

	// The new pointers
	SetPixelsPointer();
}

//
// void RenderSurface::GetOrigin(int32 &x, int32 &y)
//
// Desc: Get the Phyiscal Pixel that is the logical origin
//
void RenderSurface::GetOrigin(int32 &x, int32 &y) const {
	// Set the origin
	x = _ox;
	y = _oy;
}

//
// void RenderSurface::GetClippingRect(Rect &r)
//
// Desc: Get the Clipping Rectangle
// r: Rect object to fill
//
void RenderSurface::GetClippingRect(Rect &r) const {
	r = _clipWindow;
}

//
// void RenderSurface::GetClippingRect(Rect &r)
//
// Desc: Set the Clipping Rectangle
// r: Rect object that contains new Clipping Rectangle
//
void RenderSurface::SetClippingRect(const Rect &r) {
	// What we need to do is to clip the clipping rect to the phyiscal screen
	_clipWindow = r;
	_clipWindow.clip(Rect(-_ox, -_oy, -_ox + _width, -_oy + _height));
}

//
// void RenderSurface::SetFlipped(bool _flipped)
//
// Desc: Flip the surface
//
void RenderSurface::SetFlipped(bool wantFlipped) {
	// Flipping is not terrible complex
	// But is a bit of a pain to set up

	// First we check to see if we are currently _flipped
	if (wantFlipped == _flipped)
		return;

	_flipped = wantFlipped;

	// What we 'need' to do is negate the pitches, and flip the clipping window
	// We keep the 'origin' in the same position relative to the clipping window

	_oy -= _clipWindow.top;
	_clipWindow.setHeight(_height - _clipWindow.top + _clipWindow.height());
	_oy += _clipWindow.top;

	_pitch = -_pitch;

	SetPixelsPointer();
}

//
// bool RenderSurface::IsFlipped() const
//
// Desc: Has the render surface been _flipped?
//
bool RenderSurface::IsFlipped() const {
	return _flipped;
}

//
// RenderSurface::Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h)
//
// Desc: Fill buffer (using a RGB colour)
//
void RenderSurface::Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) {
	Rect rect(sx, sy, sx + w, sy + h);
	rect.clip(_clipWindow);
	rgb = _surface->format.RGBToColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
	_surface->fillRect(Common::Rect(rect.left + _ox, rect.top + _oy, rect.right + _ox, rect.bottom + _oy), rgb);
}

//
// RenderSurface::DrawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey);
//
// Desc: Draw a (non-antialiased) line from (sx,sy) to (ex,ey) with color rgb
//
void RenderSurface::DrawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey) {
	rgb = _surface->format.RGBToColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
	_surface->drawLine(sx + _ox, sy + _oy, ex + _ox, ey + _oy, rgb);
}

//
// RenderSurface::Blit(Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, bool alpha_blend)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
void RenderSurface::Blit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, bool alpha_blend) {
	Common::Point dpoint = Common::Point(_ox + dx, _oy + dy);
	if (alpha_blend) {
		_surface->transBlitFrom(src, srcRect, dpoint);
	} else {
		_surface->blitFrom(src, srcRect, dpoint);
	}
}

//
// RenderSurface::SetVideoMode()
//
// Desc: Create a standard RenderSurface
// Returns: Created RenderSurface or 0
//

RenderSurface *RenderSurface::SetVideoMode(uint32 width, uint32 height, int bpp) {
	// Set up the pixel format to use
	Graphics::PixelFormat pixelFormat;

	if (bpp == 16) {
		pixelFormat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	} else if (bpp == 32) {
		pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	} else {
		error("Only 16 bit and 32 bit video modes supported");
	}

	// Set up screen mode
	initGraphics(width, height, &pixelFormat);

	// Set up blitting surface
	Graphics::ManagedSurface *surface = new Graphics::Screen(width, height, pixelFormat);
	assert(surface);

	// Now create the SoftRenderSurface
	RenderSurface *surf;
	if (pixelFormat.bytesPerPixel == 4) surf = new SoftRenderSurface<uint32>(surface);
	else surf = new SoftRenderSurface<uint16>(surface);

	// Initialize gamma correction tables
	for (int i = 0; i < 256; i++) {
		_gamma22toGamma10[i] = static_cast<uint8>(0.5 + (pow(i / 255.0, 2.2 / 1.0) * 255.0));
		_gamma10toGamma22[i] = static_cast<uint8>(0.5 + (pow(i / 255.0, 1.0 / 2.2) * 255.0));
	}

	return surf;
}

// Create a SecondaryRenderSurface with an associated Texture object
RenderSurface *RenderSurface::CreateSecondaryRenderSurface(uint32 width, uint32 height) {
	const Graphics::PixelFormat &format = Ultima8Engine::get_instance()->getScreen()->format;

	// Now create the SoftRenderSurface
	RenderSurface *surf;

	// TODO: Change this
	Graphics::ManagedSurface *managedSurface = new Graphics::ManagedSurface(width, height, format);
	if (format.bytesPerPixel == 4)
		surf = new SoftRenderSurface<uint32>(managedSurface);
	else
		surf = new SoftRenderSurface<uint16>(managedSurface);
	return surf;
}

} // End of namespace Ultima8
} // End of namespace Ultima
