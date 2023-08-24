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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/graphics/xform_blend.h"
#include "ultima/ultima8/ultima8.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/screen.h"

namespace Ultima {
namespace Ultima8 {

uint8 RenderSurface::_gamma10toGamma22[256];
uint8 RenderSurface::_gamma22toGamma10[256];

RenderSurface::RenderSurface(Graphics::ManagedSurface *s) : _pixels(nullptr), _ox(0), _oy(0), _pitch(0),
															_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0),
															_surface(s) {
	_clipWindow.setWidth(_surface->w);
	_clipWindow.setHeight(_surface->h);
	_pitch = _surface->pitch;

	SetPixelsPointer();
}

//
// RenderSurface::~RenderSurface()
//
// Desc: Destructor
//
RenderSurface::~RenderSurface() {
	delete _surface;
}

void RenderSurface::SetPixelsPointer()
{
	uint8 *pix00 = static_cast<uint8 *>(_surface->getPixels());

	if (_flipped) {
		pix00 += -_pitch * (_surface->h - 1);
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
		_surface->markAllDirty();

		_pitch = _surface->pitch;
		if (_flipped)
			_pitch = -_pitch;
	}

	_lockCount++;

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
		// Clear pointers
		_pixels = 0;

		// Render the screen if this is it (slight hack..)
		Graphics::Screen *screen = dynamic_cast<Graphics::Screen *>(_surface);
		if (screen)
			screen->update();
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
	r.setWidth(_surface->w);
	r.setHeight(_surface->h);
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
	r = Rect(_clipWindow.left, _clipWindow.top, _clipWindow.right, _clipWindow.bottom);
}

//
// void RenderSurface::GetClippingRect(Rect &r)
//
// Desc: Set the Clipping Rectangle
// r: Rect object that contains new Clipping Rectangle
//
void RenderSurface::SetClippingRect(const Rect &r) {
	// What we need to do is to clip the clipping rect to the phyiscal screen
	_clipWindow = Common::Rect(r.left, r.top, r.right, r.bottom);
	_clipWindow.clip(Common::Rect(-_ox, -_oy, -_ox + _surface->w, -_oy + _surface->h));
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
	_clipWindow.setHeight(_surface->h - _clipWindow.top + _clipWindow.height());
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
void RenderSurface::Fill32(uint32 rgb, const Rect &r) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_clipWindow);
	rgb = _surface->format.RGBToColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
	_surface->fillRect(Common::Rect(rect.left + _ox, rect.top + _oy, rect.right + _ox, rect.bottom + _oy), rgb);
}


// #define CHECK_ALPHA_FILLS
namespace {

template<typename uintX>
void inline fillAlphaLogic(uint8 *pixels, int32 pitch, uint8 alpha, const Common::Rect &rect, const Graphics::PixelFormat &format) {
	uint32 aMask = format.aMax() << format.aShift;
	int32 w = rect.width();
	int32 h = rect.height();

	if (!w || !h || !aMask)
		return;

	// An optimization.
	if (w * format.bytesPerPixel == pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = pixels + rect.top * pitch + rect.left * format.bytesPerPixel;
	uint8 *end = pixel + h * pitch;

	uint8 *line_end = pixel + w * format.bytesPerPixel;
	int diff = pitch - w * format.bytesPerPixel;

	uint32 a = (((uint32)alpha) << format.aShift) & aMask;

#ifdef CHECK_ALPHA_FILLS
	uint32 c;
	uint32 m;
	if (a == 0) {
		c = (format.bMask >> 1) & format.bMask;
		m = format.bMask;
	} else {
		c = (format.rMask >> 1) & format.rMask;
		m = format.rMask;
	}
#endif

	while (pixel != end) {
		while (pixel != line_end) {
			uintX *dest = reinterpret_cast<uintX *>(pixel);
			*dest = (*dest & ~aMask) | a;
#ifdef CHECK_ALPHA_FILLS
			*dest = (*dest & ~m) | (c + (((*dest & m) >> 1) & m));
#endif
			pixel += format.bytesPerPixel;
		}

		line_end += pitch;
		pixel += diff;
	}
}

} // End of anonymous namespace

//
// RenderSurface::FillAlpha(uint8 alpha, Rect r)
//
// Desc: Fill alpha channel
//
void RenderSurface::FillAlpha(uint8 alpha, const Rect &r) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_clipWindow);

	if (_surface->format.bytesPerPixel == 4)
		fillAlphaLogic<uint32>(_pixels, _pitch, alpha, rect, _surface->format);
	else if (_surface->format.bytesPerPixel == 2)
		fillAlphaLogic<uint16>(_pixels, _pitch, alpha, rect, _surface->format);
}

namespace {

template<typename uintX>
void inline fillBlendedLogic(uint8 *pixels, int32 pitch, uint32 rgba, const Common::Rect &rect, const Graphics::PixelFormat &format) {
	int32 w = rect.width();
	int32 h = rect.height();

	if (!w || !h)
		return;

	// An optimization.
	if (w * format.bytesPerPixel == pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = pixels + rect.top * pitch + rect.left * format.bytesPerPixel;
	uint8 *end = pixel + h * pitch;

	uint8 *line_end = pixel + w * format.bytesPerPixel;
	int diff = pitch - w * format.bytesPerPixel;

	uint32 aMask = format.aMax() << format.aShift;
	int alpha = TEX32_A(rgba);
	rgba = TEX32_PACK_RGBA((TEX32_R(rgba) * alpha) >> 8,
						   (TEX32_G(rgba) * alpha) >> 8,
						   (TEX32_B(rgba) * alpha) >> 8,
						   (255 * alpha) >> 8);

	while (pixel != end) {
		while (pixel != line_end) {
			uintX *dest = reinterpret_cast<uintX *>(pixel);
			uint32 d = *dest;
			*dest = (d & aMask) | BlendPreModFast(rgba, d, format);
			pixel += format.bytesPerPixel;
		}

		line_end += pitch;
		pixel += diff;
	}
}

} // End of anonymous namespace

//
// RenderSurface::FillBlended(uint32 rgba, Rect r)
//
// Desc: Fill the region doing alpha blending
//
void RenderSurface::FillBlended(uint32 rgba, const Rect &r) {
	int alpha = TEX32_A(rgba);
	if (alpha == 0xFF) {
		Fill32(rgba, r);
		return;
	} else if (!alpha) {
		return;
	}

	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_clipWindow);

	if (_surface->format.bytesPerPixel == 4)
		fillBlendedLogic<uint32>(_pixels, _pitch, rgba, rect, _surface->format);
	else if (_surface->format.bytesPerPixel == 2)
		fillBlendedLogic<uint16>(_pixels, _pitch, rgba, rect, _surface->format);
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

namespace {

template<typename uintX>
void inline fadedBlitLogic(uint8 *pixels, int32 pitch,
						   const Common::Rect &clipWindow,
						   const Graphics::PixelFormat &format,
						   const Graphics::ManagedSurface &src,
						   const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
	int32 w = srcRect.width();
	int32 h = srcRect.height();

	// Clamp or wrap or return?
	if (w > static_cast<int32>(src.w))
		return;

	// Clamp or wrap or return?
	if (h > static_cast<int32>(src.h))
		return;

	// Clip to window
	int px = dx, py = dy;

	Common::Rect rect(dx, dy, dx + w, dy + h);
	rect.clip(clipWindow);
	dx = rect.left;
	dy = rect.top;
	w = rect.width();
	h = rect.height();

	if (!w || !h)
		return;

	int32 sx = srcRect.left;
	int32 sy = srcRect.top;

	// Adjust source x and y
	if (px != dx)
		sx += dx - px;
	if (py != dy)
		sy += dy - py;

	uint8 *pixel = pixels + dy * pitch + dx * format.bytesPerPixel;
	uint8 *line_end = pixel + w * format.bytesPerPixel;
	uint8 *end = pixel + h * pitch;
	int diff = pitch - w * format.bytesPerPixel;

	uint32 a = TEX32_A(col32);
	uint32 ia = 256 - a;
	uint32 r = (TEX32_R(col32) * a);
	uint32 g = (TEX32_G(col32) * a);
	uint32 b = (TEX32_B(col32) * a);

	const Graphics::PixelFormat &texformat = src.rawSurface().format;

	if (texformat.bpp() == 32) {
		const uint32 *texel = static_cast<const uint32 *>(src.getBasePtr(sx, sy));
		int tex_diff = src.w - w;

		while (pixel != end) {
			if (!alpha_blend)
				while (pixel != line_end) {
					if (TEX32_A(*texel)) {
						*(reinterpret_cast<uintX *>(pixel)) = static_cast<uintX>(
							format.RGBToColor(
								(TEX32_R(*texel) * ia + r) >> 8,
								(TEX32_G(*texel) * ia + g) >> 8,
								(TEX32_B(*texel) * ia + b) >> 8));
					}
					pixel += format.bytesPerPixel;
					texel++;
				}
			else
				while (pixel != line_end) {
					uint32 alpha = TEX32_A(*texel);
					if (alpha == 0xFF) {
						*(reinterpret_cast<uintX *>(pixel)) = static_cast<uintX>(
							format.RGBToColor(
								(TEX32_R(*texel) * ia + r) >> 8,
								(TEX32_G(*texel) * ia + g) >> 8,
								(TEX32_B(*texel) * ia + b) >> 8));
					} else if (alpha) {
						uintX *dest = reinterpret_cast<uintX *>(pixel);

						uint32 Tsrc = *texel;
						uint8 r2, g2, b2;
						format.colorToRGB(*dest, r2, g2, b2);

						uint32 dr = r2 * (256 - alpha);
						uint32 dg = g2 * (256 - alpha);
						uint32 db = b2 * (256 - alpha);
						dr += TEX32_R(Tsrc) * ia + ((r * alpha) >> 8);
						dg += TEX32_G(Tsrc) * ia + ((g * alpha) >> 8);
						db += TEX32_B(Tsrc) * ia + ((b * alpha) >> 8);

						*dest = format.RGBToColor(dr >> 8, dg >> 8, db >> 8);
					}
					pixel += format.bytesPerPixel;
					texel++;
				}

			line_end += pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else if (texformat == format) {
		const uintX *texel = reinterpret_cast<const uintX *>(src.getBasePtr(sx, sy));
		int tex_diff = src.w - w;

		while (pixel != end) {
			while (pixel != line_end) {
				// Uh, not supported right now
				// if (TEX32_A(*texel))
				{
					*(reinterpret_cast<uintX *>(pixel)) = BlendHighlight(*texel, r, g, b, 1, ia, format);
				}
				pixel += format.bytesPerPixel;
				texel++;
			}

			line_end += pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else {
		error("FadedBlit not supported from %d bpp to %d bpp", texformat.bpp(), format.bpp());
	}
}

} // End of anonymous namespace

//
// void RenderSurface::FadedBlit(Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
void RenderSurface::FadedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
	if (_surface->format.bytesPerPixel == 4)
		fadedBlitLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
	else if (_surface->format.bytesPerPixel == 2)
		fadedBlitLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
}

namespace {

template<typename uintX>
void inline maskedBlitLogic(uint8 *pixels, int32 pitch,
							const Common::Rect &clipWindow,
							const Graphics::PixelFormat &format,
							const Graphics::ManagedSurface &src,
							const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
	int32 w = srcRect.width();
	int32 h = srcRect.height();

	// Clamp or wrap or return?
	if (w > static_cast<int32>(src.w))
		return;

	// Clamp or wrap or return?
	if (h > static_cast<int32>(src.h))
		return;

	// Clip to window
	int px = dx, py = dy;

	Common::Rect rect(dx, dy, dx + w, dy + h);
	rect.clip(clipWindow);
	dx = rect.left;
	dy = rect.top;
	w = rect.width();
	h = rect.height();

	if (!w || !h)
		return;

	int32 sx = srcRect.left;
	int32 sy = srcRect.top;

	// Adjust source x and y
	if (px != dx)
		sx += dx - px;
	if (py != dy)
		sy += dy - py;

	uint8 *pixel = pixels + dy * pitch + dx * format.bytesPerPixel;
	uint8 *line_end = pixel + w * format.bytesPerPixel;
	uint8 *end = pixel + h * pitch;
	int diff = pitch - w * format.bytesPerPixel;

	uint32 a = TEX32_A(col32);
	uint32 ia = 256 - a;
	uint32 r = (TEX32_R(col32) * a);
	uint32 g = (TEX32_G(col32) * a);
	uint32 b = (TEX32_B(col32) * a);

	uint32 aMask = format.aMax() << format.aShift;
	int texbpp = src.rawSurface().format.bpp();

	if (texbpp == 32) {
		const uint32 *texel = static_cast<const uint32 *>(src.getBasePtr(sx, sy));
		int tex_diff = src.w - w;

		while (pixel != end) {
			if (!alpha_blend) {
				while (pixel != line_end) {
					uintX *dest = reinterpret_cast<uintX *>(pixel);

					if (TEX32_A(*texel)) {
						if (!aMask || (*dest & aMask)) {
							*dest = static_cast<uintX>(
								format.RGBToColor(
									(TEX32_R(*texel) * ia + r) >> 8,
									(TEX32_G(*texel) * ia + g) >> 8,
									(TEX32_B(*texel) * ia + b) >> 8));
						}
					}
					pixel += format.bytesPerPixel;
					texel++;
				}
			} else {
				while (pixel != line_end) {
					uintX *dest = reinterpret_cast<uintX *>(pixel);

					if (!aMask || (*dest & aMask)) {
						uint32 alpha = TEX32_A(*texel);
						if (alpha == 0xFF) {
							*dest = static_cast<uintX>(
								format.RGBToColor(
									(TEX32_R(*texel) * ia + r) >> 8,
									(TEX32_G(*texel) * ia + g) >> 8,
									(TEX32_B(*texel) * ia + b) >> 8));
						} else if (alpha) {
							uint32 Tsrc = *texel;
							uint8 r2, g2, b2;
							format.colorToRGB(*dest, r2, g2, b2);

							uint32 dr = r2 * (256 - alpha);
							uint32 dg = g2 * (256 - alpha);
							uint32 db = b2 * (256 - alpha);
							dr += TEX32_R(Tsrc) * ia + ((r * alpha) >> 8);
							dg += TEX32_G(Tsrc) * ia + ((g * alpha) >> 8);
							db += TEX32_B(Tsrc) * ia + ((b * alpha) >> 8);

							*dest = format.RGBToColor(dr >> 8, dg >> 8, db >> 8);
						}
					}
					pixel += format.bytesPerPixel;
					texel++;
				}
			}

			line_end += pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else if (texbpp == format.bpp()) {
		const uintX *texel = reinterpret_cast<const uintX *>(src.getBasePtr(sx, sy));
		int tex_diff = src.w - w;

		while (pixel != end) {
			while (pixel != line_end) {
				uintX *dest = reinterpret_cast<uintX *>(pixel);

				// Uh, not completely supported right now
				// if ((*texel & format.a_mask) && (*dest & format.a_mask))
				if (*dest & aMask) {
					*dest = BlendHighlight(*texel, r, g, b, 1, ia, format);
				}
				pixel += format.bytesPerPixel;
				texel++;
			}

			line_end += pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else {
		error("unsupported texture format %d bpp", texbpp);
	}
}

} // End of anonymous namespace

//
// void RenderSurface::MaskedBlit(Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend=false)
//
// Desc Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
//
//
void RenderSurface::MaskedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
	if (_surface->format.bytesPerPixel == 4)
		maskedBlitLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
	else if (_surface->format.bytesPerPixel == 2)
		maskedBlitLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
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

	// Initialize gamma correction tables
	for (int i = 0; i < 256; i++) {
		_gamma22toGamma10[i] = static_cast<uint8>(0.5 + (pow(i / 255.0, 2.2 / 1.0) * 255.0));
		_gamma10toGamma22[i] = static_cast<uint8>(0.5 + (pow(i / 255.0, 1.0 / 2.2) * 255.0));
	}

	return new RenderSurface(surface);
}

// Create a SecondaryRenderSurface with an associated Texture object
RenderSurface *RenderSurface::CreateSecondaryRenderSurface(uint32 width, uint32 height) {
	const Graphics::PixelFormat &format = Ultima8Engine::get_instance()->getScreen()->format;

	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface(width, height, format);
	return new RenderSurface(surface);
}

namespace {

template<typename uintX>

void inline paintLogic(uint8 *pixels, int32 pitch,
					   const Common::Rect &clipWindow,
					   const Graphics::PixelFormat &format,
					   const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#include "ultima/ultima8/graphics/render_surface.inl"
}

template<class uintX>
void inline paintNoClipLogic(uint8 *pixels, int32 pitch,
							 const Common::Rect &clipWindow,
							 const Graphics::PixelFormat &format,
							 const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#define NO_CLIPPING
#include "ultima/ultima8/graphics/render_surface.inl"
#undef NO_CLIPPING
}

template<class uintX>
void inline paintTranslucentLogic(uint8 *pixels, int32 pitch,
								  const Common::Rect &clipWindow,
								  const Graphics::PixelFormat &format,
								  const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#define XFORM_SHAPES
#include "ultima/ultima8/graphics/render_surface.inl"
#undef XFORM_SHAPES
}

template<class uintX>
void inline paintMirroredLogic(uint8 *pixels, int32 pitch,
							   const Common::Rect &clipWindow,
							   const Graphics::PixelFormat &format,
							   const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool untformed_pal) {
#define FLIP_SHAPES
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans

#include "ultima/ultima8/graphics/render_surface.inl"

#undef FLIP_SHAPES
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
}

template<class uintX>
void inline paintInvisibleLogic(uint8 *pixels, int32 pitch,
								const Common::Rect &clipWindow,
								const Graphics::PixelFormat &format,
								const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, bool untformed_pal) {
#define FLIP_SHAPES
#define FLIP_CONDITIONAL mirrored
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans
#define BLEND_SHAPES(src, dst) BlendInvisible(src, dst, format)

#include "ultima/ultima8/graphics/render_surface.inl"

#undef FLIP_SHAPES
#undef FLIP_CONDITIONAL
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
#undef BLEND_SHAPES
}

template<class uintX>
void inline paintHighlightLogic(uint8 *pixels, int32 pitch,
								const Common::Rect &clipWindow,
								const Graphics::PixelFormat &format,
								const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
#define FLIP_SHAPES
#define FLIP_CONDITIONAL mirrored
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans
#define BLEND_SHAPES(src, dst) BlendHighlight(src, cr, cg, cb, ca, 255 - ca, format)

	uint32 ca = TEX32_A(col32);
	uint32 cr = TEX32_R(col32);
	uint32 cg = TEX32_G(col32);
	uint32 cb = TEX32_B(col32);

#include "ultima/ultima8/graphics/render_surface.inl"

#undef FLIP_SHAPES
#undef FLIP_CONDITIONAL
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
#undef BLEND_SHAPES
}

template<class uintX>
void inline paintHighlightInvisLogic(uint8 *pixels, int32 pitch,
									 const Common::Rect &clipWindow,
									 const Graphics::PixelFormat &format,
									 const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
#define FLIP_SHAPES
#define FLIP_CONDITIONAL mirrored
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans
#define BLEND_SHAPES(src, dst) BlendHighlightInvis(src, dst, cr, cg, cb, ca, 255 - ca, format)

	uint32 ca = TEX32_A(col32);
	uint32 cr = TEX32_R(col32);
	uint32 cg = TEX32_G(col32);
	uint32 cb = TEX32_B(col32);

#include "ultima/ultima8/graphics/render_surface.inl"

#undef FLIP_SHAPES
#undef FLIP_CONDITIONAL
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
#undef BLEND_SHAPES
}

} // End of anonymous namespace

//
// void RenderSurface::Paint(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Clips but doesn't do anything else
//
void RenderSurface::Paint(const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
	if (_surface->format.bytesPerPixel == 4)
		paintLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, untformed_pal);
	else if (_surface->format.bytesPerPixel == 2)
		paintLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, untformed_pal);
}

//
// void RenderSurface::PaintNoClip(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Doesn't clip
//
void RenderSurface::PaintNoClip(const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
	if (_surface->format.bytesPerPixel == 4)
		paintNoClipLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, untformed_pal);
	else if (_surface->format.bytesPerPixel == 2)
		paintNoClipLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, untformed_pal);
}

//
// void RenderSurface::PaintTranslucent(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Clips and XForms
//
void RenderSurface::PaintTranslucent(const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
	if (_surface->format.bytesPerPixel == 4)
		paintTranslucentLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, untformed_pal);
	else if (_surface->format.bytesPerPixel == 2)
		paintTranslucentLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, untformed_pal);
}

//
// void RenderSurface::PaintMirrored(Shape*s, uint32 framenum, int32 x, int32 y, bool trans)
//
// Desc: Standard shape drawing functions. Clips, Flips and conditionally XForms
//
void RenderSurface::PaintMirrored(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool untformed_pal) {
	if (_surface->format.bytesPerPixel == 4)
		paintMirroredLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, trans, untformed_pal);
	else if (_surface->format.bytesPerPixel == 2)
		paintMirroredLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, trans, untformed_pal);
}

//
// void RenderSurface::PaintInvisible(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Invisible, Clips, and conditionally Flips and Xforms
//
void RenderSurface::PaintInvisible(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, bool untformed_pal) {
	if (_surface->format.bytesPerPixel == 4)
		paintInvisibleLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, trans, mirrored, untformed_pal);
	else if (_surface->format.bytesPerPixel == 2)
		paintInvisibleLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, trans, mirrored, untformed_pal);
}

//
// void RenderSurface::PaintHighlight(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Highlights, Clips, and conditionally Flips and Xforms
//
void RenderSurface::PaintHighlight(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
	if (_surface->format.bytesPerPixel == 4)
		paintHighlightLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, trans, mirrored, col32, untformed_pal);
	else if (_surface->format.bytesPerPixel == 2)
		paintHighlightLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, trans, mirrored, col32, untformed_pal);
}

//
// void RenderSurface::PaintHighlightInvis(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Highlights, Clips, and conditionally Flips and Xforms. 50% translucent
//
void RenderSurface::PaintHighlightInvis(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
	if (_surface->format.bytesPerPixel == 4)
		paintHighlightInvisLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, trans, mirrored, col32, untformed_pal);
	else if (_surface->format.bytesPerPixel == 2)
		paintHighlightInvisLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, s, framenum, x, y, trans, mirrored, col32, untformed_pal);
}

} // End of namespace Ultima8
} // End of namespace Ultima
