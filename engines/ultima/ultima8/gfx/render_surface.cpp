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

#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/gfx/texture.h"
#include "graphics/blit.h"

namespace Ultima {
namespace Ultima8 {

RenderSurface::RenderSurface(int width, int height, const Graphics::PixelFormat &format) :
		_pixels(nullptr), _ox(0), _oy(0), _pitch(0),
		_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0),
		_disposeAfterUse(DisposeAfterUse::YES) {

	_surface = new Graphics::ManagedSurface(width, height, format);
	_clipWindow.setWidth(_surface->w);
	_clipWindow.setHeight(_surface->h);

	SetPixelsPointer();
}

RenderSurface::RenderSurface(Graphics::ManagedSurface *s, DisposeAfterUse::Flag disposeAfterUse) :
		_pixels(nullptr), _ox(0), _oy(0), _pitch(0),
		_flipped(false), _clipWindow(0, 0, 0, 0), _lockCount(0),
		_surface(s), _disposeAfterUse(disposeAfterUse) {

	_clipWindow.setWidth(_surface->w);
	_clipWindow.setHeight(_surface->h);

	SetPixelsPointer();
}

//
// RenderSurface::~RenderSurface()
//
// Desc: Destructor
//
RenderSurface::~RenderSurface() {
	if (_disposeAfterUse == DisposeAfterUse::YES)
		delete _surface;
}

void RenderSurface::SetPixelsPointer()
{
	_pixels = static_cast<uint8 *>(_surface->getBasePtr(_ox, _oy));
	_pitch = _surface->pitch;

	if (_flipped) {
		_pixels = static_cast<uint8 *>(_surface->getBasePtr(_ox, _surface->h - 1 - _oy));
		_pitch = -_pitch;
	}

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
		_pixels = nullptr;
	}

	// No error
	return true;
}

//
// void RenderSurface::GetSurfaceDims(Rect &r)
//
// Desc: Get the Surface Dimensions (and logical origin)
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
	_flipped = wantFlipped;

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

void RenderSurface::fillRect(const Rect &r, uint32 color) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_clipWindow);
	rect.translate(_ox, _oy);
	_surface->fillRect(rect, color);
}

void RenderSurface::frameRect(const Rect& r, uint32 color) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_clipWindow);
	rect.translate(_ox, _oy);
	_surface->frameRect(rect, color);
}

void RenderSurface::drawLine(int32 sx, int32 sy, int32 ex, int32 ey, uint32 color) {
	_surface->drawLine(sx + _ox, sy + _oy, ex + _ox, ey + _oy, color);
}

void RenderSurface::fill32(uint32 rgb, const Rect &r) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_clipWindow);
	rect.translate(_ox, _oy);
	rgb = _surface->format.RGBToColor(TEX32_R(rgb), TEX32_G(rgb), TEX32_B(rgb));
	_surface->fillRect(rect, rgb);
}

namespace {

template<typename uintX>
void inline fillBlendedLogic(uint8 *pixels, int32 pitch, uint32 rgba, const Common::Rect &rect, const Graphics::PixelFormat &format) {
	int32 w = rect.width();
	int32 h = rect.height();

	if (!w || !h)
		return;

	uint32 sa = TEX32_A(rgba);
	uint32 sr = TEX32_R(rgba);
	uint32 sg = TEX32_G(rgba);
	uint32 sb = TEX32_B(rgba);
	uint32 ia = 256 - TEX32_A(rgba);

	uint8 *pixel = pixels + rect.top * pitch + rect.left * format.bytesPerPixel;
	int diff = pitch - w * format.bytesPerPixel;

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			uintX *dest = reinterpret_cast<uintX *>(pixel);
			uint8 dr, dg, db, da;
			format.colorToARGB(*dest, da, dr, dg, db);

			if (da) {
				dr = (dr * ia + sr * sa) >> 8;
				dg = (dg * ia + sg * sa) >> 8;
				db = (db * ia + sb * sa) >> 8;

				*dest = format.ARGBToColor(da, dr, dg, db);
			}
			pixel += format.bytesPerPixel;
		}

		pixel += diff;
	}
}

} // End of anonymous namespace

void RenderSurface::fillBlended(uint32 rgba, const Rect &r) {
	int alpha = TEX32_A(rgba);
	if (alpha == 0xFF) {
		fill32(rgba, r);
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

void RenderSurface::frameRect32(uint32 rgb, const Rect &r) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_clipWindow);
	rect.translate(_ox, _oy);
	rgb = _surface->format.RGBToColor(TEX32_R(rgb), TEX32_G(rgb), TEX32_B(rgb));
	_surface->frameRect(rect, rgb);
}

void RenderSurface::drawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey) {
	rgb = _surface->format.RGBToColor(TEX32_R(rgb), TEX32_G(rgb), TEX32_B(rgb));
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

void RenderSurface::CrossKeyBlitMap(const Graphics::Surface& src, const Common::Rect& srcRect, int32 dx, int32 dy, const uint32* map, const uint32 key) {
	byte *dstPixels = reinterpret_cast<byte *>(_surface->getBasePtr(_ox + dx, _oy + dy));
	const byte *srcPixels = reinterpret_cast<const byte *>(src.getBasePtr(srcRect.left, srcRect.top));
	Graphics::crossKeyBlitMap(dstPixels, srcPixels, _surface->pitch, src.pitch, srcRect.width(), srcRect.height(), _surface->format.bytesPerPixel, map, key);
}

namespace {

template<typename uintDst, typename uintSrc>
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

	uint32 a = TEX32_A(col32);
	uint32 ia = 256 - a;
	uint32 r = (TEX32_R(col32) * a);
	uint32 g = (TEX32_G(col32) * a);
	uint32 b = (TEX32_B(col32) * a);

	uint8 *dstPixels = pixels + dy * pitch + dx * sizeof(uintDst);
	int dstStep = sizeof(uintDst);
	int dstDelta = pitch - w * sizeof(uintDst);

	const uint8 *srcPixels = reinterpret_cast<const uint8 *>(src.getBasePtr(sx, sy));
	int srcStep = sizeof(uintSrc);
	int srcDelta = src.pitch - w * sizeof(uintSrc);

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			uint8 sa, sr, sg, sb;
			const uint32 color = *(reinterpret_cast<const uintSrc *>(srcPixels));
			src.format.colorToARGB(color, sa, sr, sg, sb);

			if (sa == 0xFF || (sa  && !alpha_blend)) {
				uintDst *dest = reinterpret_cast<uintDst *>(dstPixels);
				*dest = format.RGBToColor((sr * ia + r) >> 8,
										  (sg * ia + g) >> 8,
										  (sb * ia + b) >> 8);
			} else if (sa) {
				uintDst *dest = reinterpret_cast<uintDst *>(dstPixels);

				uint8 r2, g2, b2;
				format.colorToRGB(*dest, r2, g2, b2);

				uint32 dr = r2 * (256 - sa);
				uint32 dg = g2 * (256 - sa);
				uint32 db = b2 * (256 - sa);
				dr += sr * ia + ((r * sa) >> 8);
				dg += sg * ia + ((g * sa) >> 8);
				db += sb * ia + ((b * sa) >> 8);

				*dest = format.RGBToColor(dr >> 8, dg >> 8, db >> 8);
			}

			srcPixels += srcStep;
			dstPixels += dstStep;
		}

		srcPixels += srcDelta;
		dstPixels += dstDelta;
	}
}

} // End of anonymous namespace

//
// void RenderSurface::FadedBlit(Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
void RenderSurface::FadedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
	if (_surface->format.bytesPerPixel == 4) {
		if (src.format.bytesPerPixel == 4) {
			fadedBlitLogic<uint32, uint32>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
		}
		else if (src.format.bytesPerPixel == 2) {
			fadedBlitLogic<uint32, uint16>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
		}
		else {
			error("FadedBlit not supported from %d bpp to %d bpp", src.format.bpp(), _surface->format.bpp());
		}
	} else if (_surface->format.bytesPerPixel == 2) {
		if (src.format.bytesPerPixel == 4) {
			fadedBlitLogic<uint16, uint32>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
		}
		else if (src.format.bytesPerPixel == 2) {
			fadedBlitLogic<uint16, uint16>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
		}
		else {
			error("FadedBlit not supported from %d bpp to %d bpp", src.format.bpp(), _surface->format.bpp());
		}
	}
	else {
		error("FadedBlit not supported from %d bpp to %d bpp", src.format.bpp(), _surface->format.bpp());
	}
}

namespace {

template<typename uintDst, typename uintSrc>
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

	uint32 a = TEX32_A(col32);
	uint32 ia = 256 - a;
	uint32 r = (TEX32_R(col32) * a);
	uint32 g = (TEX32_G(col32) * a);
	uint32 b = (TEX32_B(col32) * a);

	uint32 aMask = format.aMax() << format.aShift;

	uint8 *dstPixels = pixels + dy * pitch + dx * sizeof(uintDst);
	int dstStep = sizeof(uintDst);
	int dstDelta = pitch - w * sizeof(uintDst);

	const uint8 *srcPixels = reinterpret_cast<const uint8 *>(src.getBasePtr(sx, sy));
	int srcStep = sizeof(uintSrc);
	int srcDelta = src.pitch - w * sizeof(uintSrc);

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			uintDst *dest = reinterpret_cast<uintDst *>(dstPixels);
			if (!aMask || (*dest & aMask)) {
				uint8 sa, sr, sg, sb;
				const uint32 color = *(reinterpret_cast<const uintSrc *>(srcPixels));
				src.format.colorToARGB(color, sa, sr, sg, sb);

				if (sa == 0xFF || (sa && !alpha_blend)) {
					*dest = format.RGBToColor((sr * ia + r) >> 8,
											  (sg * ia + g) >> 8,
											  (sb * ia + b) >> 8);
				} else if (sa) {
					uint8 r2, g2, b2;
					format.colorToRGB(*dest, r2, g2, b2);

					uint32 dr = r2 * (256 - sa);
					uint32 dg = g2 * (256 - sa);
					uint32 db = b2 * (256 - sa);
					dr += sr * ia + ((r * sa) >> 8);
					dg += sg * ia + ((g * sa) >> 8);
					db += sb * ia + ((b * sa) >> 8);

					*dest = format.RGBToColor(dr >> 8, dg >> 8, db >> 8);
				}
			}

			srcPixels += srcStep;
			dstPixels += dstStep;
		}

		srcPixels += srcDelta;
		dstPixels += dstDelta;
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
	if (_surface->format.bytesPerPixel == 4) {
		if (src.format.bytesPerPixel == 4) {
			maskedBlitLogic<uint32, uint32>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
		} else if (src.format.bytesPerPixel == 2) {
			maskedBlitLogic<uint32, uint16>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
		} else {
			error("MaskedBlit not supported from %d bpp to %d bpp", src.format.bpp(), _surface->format.bpp());
		}
	} else if (_surface->format.bytesPerPixel == 2) {
		if (src.format.bytesPerPixel == 4) {
			maskedBlitLogic<uint16, uint32>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
		} else if (src.format.bytesPerPixel == 2) {
			maskedBlitLogic<uint16, uint16>(_pixels, _pitch, _clipWindow, _surface->format, src, srcRect, dx, dy, col32, alpha_blend);
		} else {
			error("MaskedBlit not supported from %d bpp to %d bpp", src.format.bpp(), _surface->format.bpp());
		}
	} else {
		error("MaskedBlit not supported from %d bpp to %d bpp", src.format.bpp(), _surface->format.bpp());
	}
}

namespace {

template<typename uintX>
void inline paintLogic(uint8 *pixels, int32 pitch,
					   const Common::Rect &clipWindow,
					   const Graphics::PixelFormat &format,
					   const ShapeFrame *frame, int32 x, int32 y, bool mirrored,
					   const uint32 *map) {
	const Graphics::Surface &src = frame->getSurface();
	Common::Rect srcRect(0, 0, src.w, src.h);
	Common::Rect dstRect(x, y, x, y);

	if (mirrored) {
		dstRect.right += frame->_xoff + 1;
		dstRect.left = dstRect.right - srcRect.width();

		if (dstRect.left < clipWindow.left) {
			srcRect.right += dstRect.left - clipWindow.left;
			dstRect.left = clipWindow.left;
		}

		if (dstRect.right > clipWindow.right) {
			srcRect.left += dstRect.right - clipWindow.right;
			dstRect.right = clipWindow.right;
		}
	} else {
		dstRect.left -= frame->_xoff;
		dstRect.right = dstRect.left + srcRect.width();

		if (dstRect.left < clipWindow.left) {
			srcRect.left -= dstRect.left - clipWindow.left;
			dstRect.left = clipWindow.left;
		}

		if (dstRect.right > clipWindow.right) {
			srcRect.right -= dstRect.right - clipWindow.right;
			dstRect.right = clipWindow.right;
		}
	}

	dstRect.top -= frame->_yoff;
	dstRect.bottom = dstRect.top + srcRect.height();

	if (dstRect.top < clipWindow.top) {
		srcRect.top -= dstRect.top - clipWindow.top;
		dstRect.top = clipWindow.top;
	}

	if (dstRect.bottom > clipWindow.bottom) {
		srcRect.bottom -= dstRect.bottom - clipWindow.bottom;
		dstRect.bottom = clipWindow.bottom;
	}

	const int srcStep = sizeof(uint8);
	int dstStep = sizeof(uintX);

	if (mirrored) {
		x = dstRect.right - 1;
		y = dstRect.top;
		dstStep = -dstStep;
	} else {
		x = dstRect.left;
		y = dstRect.top;
	}

	const int w = srcRect.width();
	const int h = srcRect.height();
	const int srcDelta = src.pitch - (w * srcStep);
	const int dstDelta = pitch - (w * dstStep);

	const uint8 keycolor = frame->_keycolor;
	const uint8 *srcPixels = reinterpret_cast<const uint8 *>(src.getBasePtr(srcRect.left, srcRect.top));
	uint8 *dstPixels = reinterpret_cast<uint8 *>(pixels + x * sizeof(uintX) + pitch * y);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			const uint8 color = *srcPixels;
			if (color != keycolor) {
				uintX *dstpix = reinterpret_cast<uintX *>(dstPixels);
				*dstpix = static_cast<uintX>(map[color]);
			}
			srcPixels += srcStep;
			dstPixels += dstStep;
		}

		srcPixels += srcDelta;
		dstPixels += dstDelta;
	}
}

template<typename uintX>
void inline paintBlendedLogic(uint8 *pixels, int32 pitch,
							  const Common::Rect &clipWindow,
							  const Graphics::PixelFormat &format,
							  const ShapeFrame *frame, int32 x, int32 y,
							  bool mirrored, bool invisible, uint32 highlight,
							  const uint32 *map, const uint32 *xform_map) {
	const Graphics::Surface &src = frame->getSurface();
	Common::Rect srcRect(0, 0, src.w, src.h);
	Common::Rect dstRect(x, y, x, y);

	if (mirrored) {
		dstRect.right += frame->_xoff + 1;
		dstRect.left = dstRect.right - srcRect.width();

		if (dstRect.left < clipWindow.left) {
			srcRect.right += dstRect.left - clipWindow.left;
			dstRect.left = clipWindow.left;
		}

		if (dstRect.right > clipWindow.right) {
			srcRect.left += dstRect.right - clipWindow.right;
			dstRect.right = clipWindow.right;
		}
	} else {
		dstRect.left -= frame->_xoff;
		dstRect.right = dstRect.left + srcRect.width();

		if (dstRect.left < clipWindow.left) {
			srcRect.left -= dstRect.left - clipWindow.left;
			dstRect.left = clipWindow.left;
		}

		if (dstRect.right > clipWindow.right) {
			srcRect.right -= dstRect.right - clipWindow.right;
			dstRect.right = clipWindow.right;
		}
	}

	dstRect.top -= frame->_yoff;
	dstRect.bottom = dstRect.top + srcRect.height();

	if (dstRect.top < clipWindow.top) {
		srcRect.top -= dstRect.top - clipWindow.top;
		dstRect.top = clipWindow.top;
	}

	if (dstRect.bottom > clipWindow.bottom) {
		srcRect.bottom -= dstRect.bottom - clipWindow.bottom;
		dstRect.bottom = clipWindow.bottom;
	}

	const int srcStep = sizeof(uint8);
	int dstStep = sizeof(uintX);

	if (mirrored) {
		x = dstRect.right - 1;
		y = dstRect.top;
		dstStep = -dstStep;
	} else {
		x = dstRect.left;
		y = dstRect.top;
	}

	const int w = srcRect.width();
	const int h = srcRect.height();
	const int srcDelta = src.pitch - (w * srcStep);
	const int dstDelta = pitch - (w * dstStep);

	const uint8 keycolor = frame->_keycolor;
	const uint8 *srcPixels = reinterpret_cast<const uint8 *>(src.getBasePtr(srcRect.left, srcRect.top));
	uint8 *dstPixels = reinterpret_cast<uint8 *>(pixels + x * sizeof(uintX) + pitch * y);

	uint8 dr, dg, db;
	uint8 sr, sg, sb;

	if (highlight) {
		uint32 ca = TEX32_A(highlight);
		uint32 cr = TEX32_R(highlight);
		uint32 cg = TEX32_G(highlight);
		uint32 cb = TEX32_B(highlight);
		uint32 ica = 255 - ca;

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				const uint8 color = *srcPixels;
				if (color != keycolor) {
					uintX *dstpix = reinterpret_cast<uintX *>(dstPixels);
					format.colorToRGB(*dstpix, dr, dg, db);

					if (xform_map && xform_map[color]) {
						uint32 val = xform_map[color];

						uint32 ia = 256 - TEX32_A(val);
						uint32 r = (dr * ia + 256 * TEX32_R(val)) >> 8;
						uint32 g = (dg * ia + 256 * TEX32_G(val)) >> 8;
						uint32 b = (db * ia + 256 * TEX32_B(val)) >> 8;

						sr = r > 0xFF ? 0xFF : r;
						sg = g > 0xFF ? 0xFF : g;
						sb = b > 0xFF ? 0xFF : b;
					} else {
						format.colorToRGB(map[color], sr, sg, sb);
					}

					if (invisible) {
						dr = (((sr * ica + cr * ca) >> 1) + (dr << 7)) >> 8;
						dg = (((sg * ica + cg * ca) >> 1) + (dg << 7)) >> 8;
						db = (((sb * ica + cb * ca) >> 1) + (db << 7)) >> 8;
					} else {
						dr = (sr * ica + cr * ca) >> 8;
						dg = (sg * ica + cg * ca) >> 8;
						db = (sb * ica + cb * ca) >> 8;
					}
					*dstpix = static_cast<uintX>(format.RGBToColor(dr, dg, db));
				}
				srcPixels += srcStep;
				dstPixels += dstStep;
			}

			srcPixels += srcDelta;
			dstPixels += dstDelta;
		}
	} else if (invisible) {
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				const uint8 color = *srcPixels;
				if (color != keycolor) {
					uintX *dstpix = reinterpret_cast<uintX *>(dstPixels);
					format.colorToRGB(*dstpix, dr, dg, db);

					if (xform_map && xform_map[color]) {
						uint32 val = xform_map[color];

						uint32 ia = 256 - TEX32_A(val);
						uint32 r = (dr * ia + 256 * TEX32_R(val)) >> 8;
						uint32 g = (dg * ia + 256 * TEX32_G(val)) >> 8;
						uint32 b = (db * ia + 256 * TEX32_B(val)) >> 8;

						sr = r > 0xFF ? 0xFF : r;
						sg = g > 0xFF ? 0xFF : g;
						sb = b > 0xFF ? 0xFF : b;
					} else {
						format.colorToRGB(map[color], sr, sg, sb);
					}

					dr = (sr * 128 + dr * 128) >> 8;
					dg = (sg * 128 + dg * 128) >> 8,
					db = (sb * 128 + db * 128) >> 8;

					*dstpix = static_cast<uintX>(format.RGBToColor(dr, dg, db));
				}
				srcPixels += srcStep;
				dstPixels += dstStep;
			}

			srcPixels += srcDelta;
			dstPixels += dstDelta;
		}
	} else {
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				const uint8 color = *srcPixels;
				if (color != keycolor) {
					uintX *dstpix = reinterpret_cast<uintX *>(dstPixels);
					if (xform_map && xform_map[color]) {
						format.colorToRGB(*dstpix, dr, dg, db);

						uint32 val = xform_map[color];
						uint32 ia = 256 - TEX32_A(val);
						uint32 r = (dr * ia + 256 * TEX32_R(val)) >> 8;
						uint32 g = (dg * ia + 256 * TEX32_G(val)) >> 8;
						uint32 b = (db * ia + 256 * TEX32_B(val)) >> 8;

						dr = r > 0xFF ? 0xFF : r;
						dg = g > 0xFF ? 0xFF : g;
						db = b > 0xFF ? 0xFF : b;
						*dstpix = static_cast<uintX>(format.RGBToColor(dr, dg, db));
					} else {
						*dstpix = static_cast<uintX>(map[color]);
					}
				}
				srcPixels += srcStep;
				dstPixels += dstStep;
			}

			srcPixels += srcDelta;
			dstPixels += dstDelta;
		}
	}
}

} // End of anonymous namespace

//
// void RenderSurface::Paint(Shape*s, uint32 framenum, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Clips but doesn't do anything else
//
void RenderSurface::Paint(const Shape *s, uint32 framenum, int32 x, int32 y, bool mirrored) {
	const ShapeFrame *frame = s->getFrame(framenum);
	if (!frame || !s->getPalette())
		return;

	const uint32 *map = s->getPalette()->_native;

	if (_surface->format.bytesPerPixel == 4)
		paintLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, map);
	else if (_surface->format.bytesPerPixel == 2)
		paintLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, map);
}

//
// void RenderSurface::PaintTranslucent(Shape*s, uint32 framenum, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Clips and XForms
//
void RenderSurface::PaintTranslucent(const Shape *s, uint32 framenum, int32 x, int32 y, bool mirrored) {
	const ShapeFrame *frame = s->getFrame(framenum);
	if (!frame || !s->getPalette())
		return;

	const uint32 *map = s->getPalette()->_native;
	const uint32 *xform_map = s->getPalette()->_xform;

	if (_surface->format.bytesPerPixel == 4)
		paintBlendedLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, false, 0, map, xform_map);
	else if (_surface->format.bytesPerPixel == 2)
		paintBlendedLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, false, 0, map, xform_map);
}

//
// void RenderSurface::PaintInvisible(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Invisible, Clips, and conditionally Flips and Xforms
//
void RenderSurface::PaintInvisible(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored) {
	const ShapeFrame *frame = s->getFrame(framenum);
	if (!frame || !s->getPalette())
		return;

	const uint32 *map = s->getPalette()->_native;
	const uint32 *xform_map = trans ? s->getPalette()->_xform : nullptr;

	if (_surface->format.bytesPerPixel == 4)
		paintBlendedLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, true, 0, map, xform_map);
	else if (_surface->format.bytesPerPixel == 2)
		paintBlendedLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, true, 0, map, xform_map);
}

//
// void RenderSurface::PaintHighlight(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Highlights, Clips, and conditionally Flips and Xforms
//
void RenderSurface::PaintHighlight(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32) {
	const ShapeFrame *frame = s->getFrame(framenum);
	if (!frame || !s->getPalette())
		return;

	const uint32 *map = s->getPalette()->_native;
	const uint32 *xform_map = trans ? s->getPalette()->_xform : nullptr;

	if (_surface->format.bytesPerPixel == 4)
		paintBlendedLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, false, col32, map, xform_map);
	else if (_surface->format.bytesPerPixel == 2)
		paintBlendedLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, false, col32, map, xform_map);
}

//
// void RenderSurface::PaintHighlightInvis(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Highlights, Clips, and conditionally Flips and Xforms. 50% translucent
//
void RenderSurface::PaintHighlightInvis(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32) {
	const ShapeFrame *frame = s->getFrame(framenum);
	if (!frame || !s->getPalette())
		return;

	const uint32 *map = s->getPalette()->_native;
	const uint32 *xform_map = trans ? s->getPalette()->_xform : nullptr;

	if (_surface->format.bytesPerPixel == 4)
		paintBlendedLogic<uint32>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, true, col32, map, xform_map);
	else if (_surface->format.bytesPerPixel == 2)
		paintBlendedLogic<uint16>(_pixels, _pitch, _clipWindow, _surface->format, frame, x, y, mirrored, true, col32, map, xform_map);
}

} // End of namespace Ultima8
} // End of namespace Ultima
