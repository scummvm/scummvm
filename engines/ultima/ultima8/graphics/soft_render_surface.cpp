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
#include "ultima/ultima8/graphics/soft_render_surface.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/graphics/xform_blend.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

///////////////////////
//                   //
// SoftRenderSurface //
//                   //
///////////////////////


//
// SoftRenderSurface::SoftRenderSurface(Graphics::Surface *s)
//
// Desc: Create a SoftRenderSurface from a managed surface
//
template<class uintX> SoftRenderSurface<uintX>::SoftRenderSurface(Graphics::ManagedSurface *s)
	: RenderSurface(s) {
}

//
// SoftRenderSurface::FillAlpha(uint8 alpha, int32 sx, int32 sy, int32 w, int32 h)
//
// Desc: Fill alpha channel
//

//#define CHECK_ALPHA_FILLS

template<class uintX> void SoftRenderSurface<uintX>::FillAlpha(uint8 alpha, const Rect &r) {
	const Graphics::PixelFormat &format = _surface->format;
	uint32 aMask = format.aMax() << format.aShift;
	Rect rect = r;
	rect.clip(_clipWindow);
	int32 w = rect.width();
	int32 h = rect.height();

	if (!w || !h || !aMask)
		return;

	// An optimization.
	if (w * format.bytesPerPixel == _pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = _pixels + rect.top * _pitch + rect.left * format.bytesPerPixel;
	uint8 *end = pixel + h * _pitch;

	uint8 *line_end = pixel + w * format.bytesPerPixel;
	int diff = _pitch - w * format.bytesPerPixel;

	uint32 a = (((uint32)alpha) << format.aShift) & aMask;

#ifdef CHECK_ALPHA_FILLS
	uint32 c;
	uint32 m;
	if (a == 0) {
		c = (format.bMask >> 1)&format.bMask;
		m = format.bMask;
	} else {
		c = (format.rMask >> 1)&format.rMask;
		m = format.rMask;
	}
#endif

	while (pixel != end) {
		while (pixel != line_end) {
			uintX *dest = reinterpret_cast<uintX *>(pixel);
			*dest = (*dest & ~aMask) | a;
#ifdef CHECK_ALPHA_FILLS
			*dest = (*dest & ~m) | (c + (((*dest & m) >> 1)&m));
#endif
			pixel += format.bytesPerPixel;
		}

		line_end += _pitch;
		pixel += diff;
	}
}

template<class uintX> void SoftRenderSurface<uintX>::FillBlended(uint32 rgba, const Rect &r) {
	int alpha = TEX32_A(rgba);
	if (alpha == 0xFF) {
		Fill32(rgba, r);
		return;
	} else if (!alpha) {
		return;
	}

	Rect rect = r;
	rect.clip(_clipWindow);
	int32 w = rect.width();
	int32 h = rect.height();

	if (!w || !h) return;

	const Graphics::PixelFormat &format = _surface->format;

	// An optimization.
	if (w * format.bytesPerPixel == _pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = _pixels + rect.top * _pitch + rect.left * format.bytesPerPixel;
	uint8 *end = pixel + h * _pitch;

	uint8 *line_end = pixel + w * format.bytesPerPixel;
	int diff = _pitch - w * format.bytesPerPixel;

	uint32 aMask = format.aMax() << format.aShift;
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

		line_end += _pitch;
		pixel += diff;
	}
}

//
// void SoftRenderSurface::FadedBlit(Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
template<class uintX> void SoftRenderSurface<uintX>::FadedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
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

	Rect rect(dx, dy, dx + w, dy + h);
	rect.clip(_clipWindow);
	dx = rect.left;
	dy = rect.top;
	w = rect.width();
	h = rect.height();

	if (!w || !h) return;

	int32 sx = srcRect.left;
	int32 sy = srcRect.top;

	// Adjust source x and y
	if (px != dx) sx += dx - px;
	if (py != dy) sy += dy - py;

	const Graphics::PixelFormat &format = _surface->format;

	uint8 *pixel = _pixels + dy * _pitch + dx * format.bytesPerPixel;
	uint8 *line_end = pixel + w * format.bytesPerPixel;
	uint8 *end = pixel + h * _pitch;
	int diff = _pitch - w * format.bytesPerPixel;

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
			if (!alpha_blend) while (pixel != line_end) {
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
			else while (pixel != line_end) {
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

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else if (texformat == format) {
		const uintX *texel = reinterpret_cast<const uintX *>(src.getBasePtr(sx, sy));
		int tex_diff = src.w - w;

		while (pixel != end) {
			while (pixel != line_end) {
				// Uh, not supported right now
				//if (TEX32_A(*texel))
				{
					*(reinterpret_cast<uintX *>(pixel)) = BlendHighlight(*texel, r, g, b, 1, ia, format);
				}
				pixel += format.bytesPerPixel;
				texel++;
			}

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else {
		error("FadedBlit not supported from %d bpp to %d bpp", texformat.bpp(), format.bpp());
	}
}


//
// void SoftRenderSurface::MaskedBlit(Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend=false)
//
// Desc Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
//
//
template<class uintX> void SoftRenderSurface<uintX>::MaskedBlit(const Graphics::ManagedSurface &src, const Common::Rect &srcRect, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
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

	Rect rect(dx, dy, dx + w, dy + h);
	rect.clip(_clipWindow);
	dx = rect.left;
	dy = rect.top;
	w = rect.width();
	h = rect.height();

	if (!w || !h)
		return;

	int32 sx = srcRect.left;
	int32 sy = srcRect.top;

	// Adjust source x and y
	if (px != dx) sx += dx - px;
	if (py != dy) sy += dy - py;

	const Graphics::PixelFormat &format = _surface->format;

	uint8 *pixel = _pixels + dy * _pitch + dx * format.bytesPerPixel;
	uint8 *line_end = pixel + w * format.bytesPerPixel;
	uint8 *end = pixel + h * _pitch;
	int diff = _pitch - w * format.bytesPerPixel;

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

			line_end += _pitch;
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
				//if ((*texel & format.a_mask) && (*dest & format.a_mask))
				if (*dest & aMask) {
					*dest = BlendHighlight(*texel, r, g, b, 1, ia, format);
				}
				pixel += format.bytesPerPixel;
				texel++;
			}

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else {
		error("unsupported texture format %d bpp", texbpp);
	}
}


//
// void SoftRenderSurface::Paint(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Clips but doesn't do anything else
//
template<class uintX> void SoftRenderSurface<uintX>::Paint(const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#include "ultima/ultima8/graphics/soft_render_surface.inl"
}


//
// void SoftRenderSurface::PaintNoClip(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Doesn't clip
//
template<class uintX> void SoftRenderSurface<uintX>::PaintNoClip(const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#define NO_CLIPPING
#include "ultima/ultima8/graphics/soft_render_surface.inl"
#undef NO_CLIPPING
}


//
// void SoftRenderSurface::PaintTranslucent(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Clips and XForms
//
template<class uintX> void SoftRenderSurface<uintX>::PaintTranslucent(const Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#define XFORM_SHAPES
#include "ultima/ultima8/graphics/soft_render_surface.inl"
#undef XFORM_SHAPES
}


//
// void SoftRenderSurface::PaintMirrored(Shape*s, uint32 framenum, int32 x, int32 y, bool trans)
//
// Desc: Standard shape drawing functions. Clips, Flips and conditionally XForms
//
template<class uintX> void SoftRenderSurface<uintX>::PaintMirrored(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool untformed_pal) {
#define FLIP_SHAPES
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans

#include "ultima/ultima8/graphics/soft_render_surface.inl"

#undef FLIP_SHAPES
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
}


//
// void SoftRenderSurface::PaintInvisible(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Invisible, Clips, and conditionally Flips and Xforms
//

template<class uintX> void SoftRenderSurface<uintX>::PaintInvisible(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, bool untformed_pal) {
#define FLIP_SHAPES
#define FLIP_CONDITIONAL mirrored
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans
#define BLEND_SHAPES(src, dst) BlendInvisible(src, dst, format)

#include "ultima/ultima8/graphics/soft_render_surface.inl"

#undef FLIP_SHAPES
#undef FLIP_CONDITIONAL
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
#undef BLEND_SHAPES
}


//
// void SoftRenderSurface::PaintHighlight(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Highlights, Clips, and conditionally Flips and Xforms
//

template<class uintX> void SoftRenderSurface<uintX>::PaintHighlight(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
#define FLIP_SHAPES
#define FLIP_CONDITIONAL mirrored
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans
#define BLEND_SHAPES(src, dst) BlendHighlight(src, cr, cg, cb, ca, 255 - ca, format)

	uint32 ca = TEX32_A(col32);
	uint32 cr = TEX32_R(col32);
	uint32 cg = TEX32_G(col32);
	uint32 cb = TEX32_B(col32);

#include "ultima/ultima8/graphics/soft_render_surface.inl"

#undef FLIP_SHAPES
#undef FLIP_CONDITIONAL
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
#undef BLEND_SHAPES
}

//
// void SoftRenderSurface::PaintHighlightInvis(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Highlights, Clips, and conditionally Flips and Xforms. 50% translucent
//

template<class uintX> void SoftRenderSurface<uintX>::PaintHighlightInvis(const Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
#define FLIP_SHAPES
#define FLIP_CONDITIONAL mirrored
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans
#define BLEND_SHAPES(src, dst) BlendHighlightInvis(src, dst, cr, cg, cb, ca, 255 - ca, format)

	uint32 ca = TEX32_A(col32);
	uint32 cr = TEX32_R(col32);
	uint32 cg = TEX32_G(col32);
	uint32 cb = TEX32_B(col32);

#include "ultima/ultima8/graphics/soft_render_surface.inl"

#undef FLIP_SHAPES
#undef FLIP_CONDITIONAL
#undef XFORM_SHAPES
#undef XFORM_CONDITIONAL
#undef BLEND_SHAPES
}

//
// Instantiate the SoftRenderSurface Class
//
template class SoftRenderSurface<uint16>;
template class SoftRenderSurface<uint32>;

} // End of namespace Ultima8
} // End of namespace Ultima
