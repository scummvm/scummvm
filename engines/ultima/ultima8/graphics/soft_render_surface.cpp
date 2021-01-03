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
#include "ultima/ultima8/graphics/texture.h"
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
	: BaseSoftRenderSurface(s) {
}


//
// SoftRenderSurface::Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h)
//
// Desc: Fill buffer (using a RGB colour)
//
template<class uintX> void SoftRenderSurface<uintX>::Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) {
	Rect rect(sx, sy, sx + w, sy + h);
	rect.clip(_clipWindow);
	rgb = PACK_RGB8((rgb >> 16) & 0xFF , (rgb >> 8) & 0xFF , rgb & 0xFF);
	_surface->fillRect(Common::Rect(rect.left + _ox, rect.top + _oy, rect.right + _ox, rect.bottom + _oy), rgb);
}

//
// SoftRenderSurface::FillAlpha(uint8 alpha, int32 sx, int32 sy, int32 w, int32 h)
//
// Desc: Fill alpha channel
//

//#define CHECK_ALPHA_FILLS

template<class uintX> void SoftRenderSurface<uintX>::FillAlpha(uint8 alpha, int32 sx, int32 sy, int32 w, int32 h) {
	Rect rect(sx, sy, sx + w, sy + h);
	rect.clip(_clipWindow);
	sx = rect.left;
	sy = rect.top;
	w = rect.width();
	h = rect.height();

	if (!w || !h || !RenderSurface::_format.aMask) return;

	// An optimization.
	if ((int)(w * sizeof(uintX)) == _pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = _pixels + sy * _pitch + sx * sizeof(uintX);
	uint8 *end = pixel + h * _pitch;

	uint8 *line_end = pixel + w * sizeof(uintX);
	int diff = _pitch - w * sizeof(uintX);

	uintX a = (((uintX)alpha) << RenderSurface::_format.aShift)&RenderSurface::_format.aMask;

#ifdef CHECK_ALPHA_FILLS
	uintX c;
	uintX m;
	if (a == 0) {
		c = (RenderSurface::_format.bMask >> 1)&RenderSurface::_format.bMask;
		m = RenderSurface::_format.bMask;
	} else {
		c = (RenderSurface::_format.rMask >> 1)&RenderSurface::_format.rMask;
		m = RenderSurface::_format.rMask;
	}
#endif

	while (pixel != end) {
		while (pixel != line_end) {
			uintX *dest = reinterpret_cast<uintX *>(pixel);
			*dest = (*dest & ~RenderSurface::_format.aMask) | a;
#ifdef CHECK_ALPHA_FILLS
			*dest = (*dest & ~m) | (c + (((*dest & m) >> 1)&m));
#endif
			pixel += sizeof(uintX);
		}

		line_end += _pitch;
		pixel += diff;
	}
}

template<class uintX> void SoftRenderSurface<uintX>::FillBlended(uint32 rgba, int32 sx, int32 sy, int32 w, int32 h) {
	if ((rgba & TEX32_A_MASK) == TEX32_A_MASK) {
		Fill32(rgba, sx, sy, w, h);
		return;
	} else if (!(rgba & TEX32_A_MASK)) {
		return;
	}

	Rect rect(sx, sy, sx + w, sy + h);
	rect.clip(_clipWindow);
	sx = rect.left;
	sy = rect.top;
	w = rect.width();
	h = rect.height();

	if (!w || !h) return;

	// An optimization.
	if ((int)(w * sizeof(uintX)) == _pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = _pixels + sy * _pitch + sx * sizeof(uintX);
	uint8 *end = pixel + h * _pitch;

	uint8 *line_end = pixel + w * sizeof(uintX);
	int diff = _pitch - w * sizeof(uintX);

	int alpha = TEX32_A(rgba) + 1;
	rgba = TEX32_PACK_RGBA16(TEX32_R(rgba) * alpha, TEX32_G(rgba) * alpha, TEX32_B(rgba) * alpha, 255 * alpha);

	while (pixel != end) {
		while (pixel != line_end) {
			uintX *dest = reinterpret_cast<uintX *>(pixel);
			uintX d = *dest;
			*dest = (d & RenderSurface::_format.aMask) | BlendPreModFast(rgba, d);
			pixel += sizeof(uintX);
		}

		line_end += _pitch;
		pixel += diff;
	}
}

//
// SoftRenderSurface::DrawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey);
//
// Desc: Draw a (non-antialiased) line from (sx,sy) to (ex,ey) with color rgb
//

template<class uintX> void SoftRenderSurface<uintX>::DrawLine32(uint32 rgb, int32 sx, int32 sy, int32 ex, int32 ey) {
	if (sy == ey) {
		int w;
		if (sx < ex) {
			w = ex - sx + 1;
		} else {
			w = sx - ex + 1;
			sx = ex;
		}
		Fill32(rgb, sx, sy, w, 1);
	} else if (sx == ex) {
		int h;
		if (sy < ey) {
			h = ey - sy + 1;
		} else {
			h = sy - ey + 1;
			sy = ey;
		}
		Fill32(rgb, sx, sy, 1, h);
	} else {
		int32 t;
		bool steep = ABS(ey - sy) > ABS(ex - sx);
		if (steep) {
			t = sx;
			sx = sy;
			sy = t;
			t = ex;
			ex = ey;
			ey = t;
		}
		if (sx > ex) {
			t = sx;
			sx = ex;
			ex = t;
			t = sy;
			sy = ey;
			ey = t;
		}
		int deltax = ex - sx;
		int deltay = ABS(ey - sy);
		int error = -deltax / 2;
		int y = sy;
		int ystep = (sy < ey) ? 1 : -1;
		for (int x = sx; x <= ex; ++x) {
			// TODO: don't use Fill32 here; it's too slow
			if (steep) {
				Fill32(rgb, y, x, 1, 1);
			} else {
				Fill32(rgb, x, y, 1, 1);
			}
			error += deltay;
			if (error > 0) {
				y += ystep;
				error -= deltax;
			}
		}
	}
}


//
// SoftRenderSurface::Blit(Graphics::ManagedSurface *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, bool alpha_blend)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
template<class uintX> void SoftRenderSurface<uintX>::Blit(const Graphics::ManagedSurface *tex, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, bool alpha_blend) {
	Common::Rect srect = Common::Rect(sx, sy, sx + w, sy + h);
	Common::Point dpoint = Common::Point(_ox + dx, _oy + dy);
	_surface->blitFrom(*tex, srect, dpoint);
}


//
// void SoftRenderSurface::FadedBlit(Graphics::ManagedSurface *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
template<class uintX> void SoftRenderSurface<uintX>::FadedBlit(const Graphics::ManagedSurface *tex, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {

	// Clamp or wrap or return?
	if (w > static_cast<int32>(tex->w))
		return;

	// Clamp or wrap or return?
	if (h > static_cast<int32>(tex->h))
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

	// Adjust source x and y
	if (px != dx) sx += dx - px;
	if (py != dy) sy += dy - py;

	uint8 *pixel = _pixels + dy * _pitch + dx * sizeof(uintX);
	uint8 *line_end = pixel + w * sizeof(uintX);
	uint8 *end = pixel + h * _pitch;
	int diff = _pitch - w * sizeof(uintX);

	uint32 a = TEX32_A(col32);
	uint32 ia = 256 - a;
	uint32 r = (TEX32_R(col32) * a);
	uint32 g = (TEX32_G(col32) * a);
	uint32 b = (TEX32_B(col32) * a);

	const Graphics::PixelFormat &texformat = tex->rawSurface().format;

	if (texformat.bpp() == 32) {
		const uint32 *texel = static_cast<const uint32 *>(tex->getBasePtr(sx, sy));
		int tex_diff = tex->w - w;

		while (pixel != end) {
			if (!alpha_blend) while (pixel != line_end) {
					if (*texel & TEX32_A_MASK) {
						*(reinterpret_cast<uintX *>(pixel)) = static_cast<uintX>(
						        PACK_RGB8(
						            (TEX32_R(*texel) * ia + r) >> 8,
						            (TEX32_G(*texel) * ia + g) >> 8,
						            (TEX32_B(*texel) * ia + b) >> 8
						        )
						                                      );
					}
					pixel += sizeof(uintX);
					texel++;
			}
			else while (pixel != line_end) {
					uint32 alpha = *texel & TEX32_A_MASK;
					if (alpha == 0xFF) {
						*(reinterpret_cast<uintX *>(pixel)) = static_cast<uintX>(
						        PACK_RGB8(
						            (TEX32_R(*texel) * ia + r) >> 8,
						            (TEX32_G(*texel) * ia + g) >> 8,
						            (TEX32_B(*texel) * ia + b) >> 8
						        )
						                                      );
					} else if (alpha) {
						uintX *dest = reinterpret_cast<uintX *>(pixel);

						uint32 src = *texel;
						uint32 dr, dg, db;
						UNPACK_RGB8(*dest, dr, dg, db);

						dr *= 256 - TEX32_A(src);
						dg *= 256 - TEX32_A(src);
						db *= 256 - TEX32_A(src);
						dr += TEX32_R(src) * ia + ((r * TEX32_A(src)) >> 8);
						dg += TEX32_G(src) * ia + ((g * TEX32_A(src)) >> 8);
						db += TEX32_B(src) * ia + ((b * TEX32_A(src)) >> 8);

						*dest = PACK_RGB16(dr, dg, db);
					}
					pixel += sizeof(uintX);
					texel++;
				}

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else if (texformat == _format) {
		const uintX *texel = reinterpret_cast<const uintX *>(tex->getBasePtr(sx, sy));
		int tex_diff = tex->w - w;

		while (pixel != end) {
			while (pixel != line_end) {
				// Uh, not supported right now
				//if (*texel & RenderSurface::a_mask)
				{
					*(reinterpret_cast<uintX *>(pixel)) = BlendHighlight(*texel, r, g, b, 1, ia);
				}
				pixel += sizeof(uintX);
				texel++;
			}

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else {
		error("FadedBlit not supported from %d bpp to %d bpp", texformat.bpp(), _format.bpp());
	}
}


//
// void SoftRenderSurface::MaskedBlit(Graphics::ManagedSurface *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend=false)
//
// Desc Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
//
//
template<class uintX> void SoftRenderSurface<uintX>::MaskedBlit(const Graphics::ManagedSurface *tex, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
	// Clamp or wrap or return?
	if (w > static_cast<int32>(tex->w))
		return;

	// Clamp or wrap or return?
	if (h > static_cast<int32>(tex->h))
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

	// Adjust source x and y
	if (px != dx) sx += dx - px;
	if (py != dy) sy += dy - py;

	uint8 *pixel = _pixels + dy * _pitch + dx * sizeof(uintX);
	uint8 *line_end = pixel + w * sizeof(uintX);
	uint8 *end = pixel + h * _pitch;
	int diff = _pitch - w * sizeof(uintX);

	uint32 a = TEX32_A(col32);
	uint32 ia = 256 - a;
	uint32 r = (TEX32_R(col32) * a);
	uint32 g = (TEX32_G(col32) * a);
	uint32 b = (TEX32_B(col32) * a);

	int texbpp = tex->rawSurface().format.bpp();

	if (texbpp == 32) {
		const uint32 *texel = static_cast<const uint32 *>(tex->getBasePtr(sx, sy));
		int tex_diff = tex->w - w;

		while (pixel != end) {
			if (!alpha_blend) {
				while (pixel != line_end) {
					uintX *dest = reinterpret_cast<uintX *>(pixel);

					if (*texel & TEX32_A_MASK) {
						if (!RenderSurface::_format.aMask || (*dest & RenderSurface::_format.aMask)) {
							*dest = static_cast<uintX>(
								PACK_RGB8(
									(TEX32_R(*texel) * ia + r) >> 8,
									(TEX32_G(*texel) * ia + g) >> 8,
									(TEX32_B(*texel) * ia + b) >> 8
								)
							);
						}
					}
					pixel += sizeof(uintX);
					texel++;
				}
			} else {
				while (pixel != line_end) {
					uintX *dest = reinterpret_cast<uintX *>(pixel);

					if (!RenderSurface::_format.aMask || (*dest & RenderSurface::_format.aMask)) {
						uint32 alpha = *texel & TEX32_A_MASK;
						if (alpha == 0xFF) {
							*dest = static_cast<uintX>(
							    PACK_RGB8(
							        (TEX32_R(*texel) * ia + r) >> 8,
							        (TEX32_G(*texel) * ia + g) >> 8,
							        (TEX32_B(*texel) * ia + b) >> 8
							    )
							);
						} else if (alpha) {
							uint32 src = *texel;
							uint32 dr, dg, db;
							UNPACK_RGB8(*dest, dr, dg, db);

							dr *= 256 - TEX32_A(src);
							dg *= 256 - TEX32_A(src);
							db *= 256 - TEX32_A(src);
							dr += TEX32_R(src) * ia + ((r * TEX32_A(src)) >> 8);
							dg += TEX32_G(src) * ia + ((g * TEX32_A(src)) >> 8);
							db += TEX32_B(src) * ia + ((b * TEX32_A(src)) >> 8);

							*dest = PACK_RGB16(dr, dg, db);
						}
					}
					pixel += sizeof(uintX);
					texel++;
				}
			}

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else if (texbpp == _format.bpp()) {
		const uintX *texel = reinterpret_cast<const uintX *>(tex->getBasePtr(sx, sy));
		int tex_diff = tex->w - w;

		while (pixel != end) {
			while (pixel != line_end) {
				uintX *dest = reinterpret_cast<uintX *>(pixel);

				// Uh, not completely supported right now
				//if ((*texel & RenderSurface::_format.a_mask) && (*dest & RenderSurface::_format.a_mask))
				if (*dest & RenderSurface::_format.aMask) {
					*dest = BlendHighlight(*texel, r, g, b, 1, ia);
				}
				pixel += sizeof(uintX);
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
#define BLEND_SHAPES(src,dst) BlendInvisible(src,dst)

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
#define BLEND_SHAPES(src,dst) BlendHighlight(src,cr,cg,cb,ca,255-ca)

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
#define BLEND_SHAPES(src,dst) BlendHighlightInvis(src,dst,cr,cg,cb,ca,255-ca)

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
