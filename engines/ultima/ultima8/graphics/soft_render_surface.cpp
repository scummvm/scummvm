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
#include "ultima/ultima8/graphics/fonts/fixed_width_font.h"
#include "ultima/ultima8/misc/memset_n.h"
#include "ultima/ultima8/graphics/xform_blend.h"
#include "ultima/ultima8/graphics/point_scaler.h"
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
// SoftRenderSurface::SoftRenderSurface(int w, int h, int bpp, int rsft, int gsft, int bsft)
//
// Desc: Create a Generic SoftRenderSurface
//
template<class uintX> SoftRenderSurface<uintX>::SoftRenderSurface(int w, int h, int bpp, int rsft, int gsft, int bsft, int asft)
	: BaseSoftRenderSurface(w, h, bpp, rsft, gsft, bsft, asft) {
}


//
// SoftRenderSurface::SoftRenderSurface(int w, int h)
//
// Desc: Create a Generic surface that matches current screen parameters
//
template<class uintX> SoftRenderSurface<uintX>::SoftRenderSurface(int w, int h, uint8 *buf)
	: BaseSoftRenderSurface(w, h, buf) {
}


//
// SoftRenderSurface::SoftRenderSurface(int w, int h)
//
// Desc: Create a Render to texture surface
//
template<class uintX> SoftRenderSurface<uintX>::SoftRenderSurface(int w, int h)
	: BaseSoftRenderSurface(w, h) {
}


//
// SoftRenderSurface::Fill8(uint8 index, int32 sx, int32 sy, int32 w, int32 h)
//
// Desc: Fill buffer (using a palette index) - Remove????
//
template<class uintX> void SoftRenderSurface<uintX>::Fill8(uint8 /*index*/, int32 /*sx*/, int32 /*sy*/, int32 /*w*/, int32 /*h*/) {
}


//
// SoftRenderSurface::Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h)
//
// Desc: Fill buffer (using a RGB colour)
//

template<class uintX> void SoftRenderSurface<uintX>::Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) {
	_clipWindow.IntersectOther(sx, sy, w, h);
	if (!w || !h) return;

	// An optimization.
	if ((w * sizeof(uintX)) == _pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = _pixels + sy * _pitch + sx * sizeof(uintX);
	uint8 *end = pixel + h * _pitch;

	rgb = PACK_RGB8((rgb >> 16) & 0xFF , (rgb >> 8) & 0xFF , rgb & 0xFF);

	uint8 *line_end = pixel + w * sizeof(uintX);
	int diff = _pitch - w * sizeof(uintX);

	while (pixel != end) {
		while (pixel != line_end) {
			*(reinterpret_cast<uintX *>(pixel)) = rgb;
			pixel += sizeof(uintX);
		}

		line_end += _pitch;
		pixel += diff;
	}
}

// 16 bit version
template<> void SoftRenderSurface<uint16>::Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) {
	_clipWindow.IntersectOther(sx, sy, w, h);
	if (!w || !h) return;

	// An optimization.
	if (2 * w == _pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = _pixels + sy * _pitch + sx * sizeof(uint16);
	uint8 *end = pixel + h * _pitch;

	rgb = PACK_RGB8((rgb >> 16) & 0xFF , (rgb >> 8) & 0xFF , rgb & 0xFF);

	while (pixel != end) {
		memset_16(pixel, rgb, w);
		pixel += _pitch;
	}
}

// 32 bit version
template<> void SoftRenderSurface<uint32>::Fill32(uint32 rgb, int32 sx, int32 sy, int32 w, int32 h) {
	_clipWindow.IntersectOther(sx, sy, w, h);
	if (!w || !h) return;

	// An optimization.
	if (4 * w == _pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = _pixels + sy * _pitch + sx * sizeof(uint32);
	uint8 *end = pixel + h * _pitch;

	rgb = PACK_RGB8((rgb >> 16) & 0xFF , (rgb >> 8) & 0xFF , rgb & 0xFF);

	while (pixel != end) {
		memset_32(pixel, rgb, w);
		pixel += _pitch;
	}
}


//
// SoftRenderSurface::FillAlpha(uint8 alpha, int32 sx, int32 sy, int32 w, int32 h)
//
// Desc: Fill alpha channel
//

//#define CHECK_ALPHA_FILLS

template<class uintX> void SoftRenderSurface<uintX>::FillAlpha(uint8 alpha, int32 sx, int32 sy, int32 w, int32 h) {
	_clipWindow.IntersectOther(sx, sy, w, h);
	if (!w || !h || !RenderSurface::_format.a_mask) return;

	// An optimization.
	if ((int)(w * sizeof(uintX)) == _pitch) {
		w *= h;
		h = 1;
	}

	uint8 *pixel = _pixels + sy * _pitch + sx * sizeof(uintX);
	uint8 *end = pixel + h * _pitch;

	uint8 *line_end = pixel + w * sizeof(uintX);
	int diff = _pitch - w * sizeof(uintX);

	uintX a = (((uintX)alpha) << RenderSurface::_format.a_shift)&RenderSurface::_format.a_mask;

#ifdef CHECK_ALPHA_FILLS
	uintX c;
	uintX m;
	if (a == 0) {
		c = (RenderSurface::_format.b_mask >> 1)&RenderSurface::_format.b_mask;
		m = RenderSurface::_format.b_mask;
	} else {
		c = (RenderSurface::_format.r_mask >> 1)&RenderSurface::_format.r_mask;
		m = RenderSurface::_format.r_mask;
	}
#endif

	while (pixel != end) {
		while (pixel != line_end) {
			uintX *dest = reinterpret_cast<uintX *>(pixel);
			*dest = (*dest & ~RenderSurface::_format.a_mask) | a;
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

	_clipWindow.IntersectOther(sx, sy, w, h);
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
			*dest = (d & RenderSurface::_format.a_mask) | BlendPreModFast(rgba, d);
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
// SoftRenderSurface::Blit(Texture *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, bool alpha_blend)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
template<class uintX> void SoftRenderSurface<uintX>::Blit(Texture *_tex, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, bool alpha_blend) {
	// Clamp or wrap or return?
	if (sx + w > static_cast<int32>(_tex->w))
		return;

	// Clamp or wrap or return?
	if (sy + h > static_cast<int32>(_tex->h))
		return;

	if (sx < 0 || sy < 0)
		return;

	// Clip to window
	int px = dx, py = dy;
	_clipWindow.IntersectOther(dx, dy, w, h);
	if (!w || !h) return;

	// Adjust source x and y
	if (px != dx) sx += dx - px;
	if (py != dy) sy += dy - py;

	uint8 *pixel = _pixels + dy * _pitch + dx * sizeof(uintX);
	uint8 *line_end = pixel + w * sizeof(uintX);
	uint8 *end = pixel + h * _pitch;
	int diff = _pitch - w * sizeof(uintX);

	if (_tex->_format == TEX_FMT_STANDARD) {
		uint32 *texel = (uint32 *)_tex->getBasePtr(sx, sy);
		int tex_diff = _tex->w - w;

		while (pixel != end) {
			if (!alpha_blend) while (pixel != line_end) {
					if (*texel & TEX32_A_MASK) {
						*(reinterpret_cast<uintX *>(pixel)) = static_cast<uintX>(PACK_RGB8(TEX32_R(*texel), TEX32_G(*texel), TEX32_B(*texel)));
					}
					pixel += sizeof(uintX);
					texel++;
				}
			else while (pixel != line_end) {
					uint32 alpha = *texel & TEX32_A_MASK;
					if (alpha == 0xFF) {
						*(reinterpret_cast<uintX *>(pixel)) = static_cast<uintX>(PACK_RGB8(TEX32_R(*texel), TEX32_G(*texel), TEX32_B(*texel)));
					} else if (alpha) {
						uintX *dest = reinterpret_cast<uintX *>(pixel);
						*dest = static_cast<uintX>(BlendPreModFast(*texel, *dest));
					}
					pixel += sizeof(uintX);
					texel++;
				}

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	} else if (_tex->_format == TEX_FMT_NATIVE) {
		uintX *texel = reinterpret_cast<uintX *>(_tex->getBasePtr(sx, sy));
		int tex_diff = _tex->w - w;

		while (pixel != end) {
			while (pixel != line_end) {
				// Uh, not supported right now
				//if (*texel & RenderSurface::a_mask)
				{
					*(reinterpret_cast<uintX *>(pixel)) = *texel;
				}
				pixel += sizeof(uintX);
				texel++;
			}

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	}

	/* Old complete code
	    // Clamp or wrap or return?
	#ifndef BLIT_WRAP
	    if (w > static_cast<int32>(_tex->w))
	#ifndef BLIT_CLIP
	        return;
	#else
	        w = _tex->w;
	#endif

	    // Clamp or wrap or return?
	    if (h > static_cast<int32>(_tex->_height))
	#ifndef BLIT_CLIP
	        return;
	#else
	        h = _tex->_height;
	#endif
	#endif

	    // Clip to window
	    int px = dx, py = dy;
	    _clipWindow.IntersectOther(dx,dy,w,h);
	    if (!w || !h) return;

	    // Adjust source x and y
	    if (px != dx) sx += dx - px;
	    if (py != dy) sy += dy - py;

	    uint8 *pixel = _pixels + dy * _pitch + dx * sizeof(uintX);
	    uint8 *line_end = pixel + w*sizeof(uintX);
	    uint8 *end = pixel + h * _pitch;
	    int diff = _pitch - w*sizeof(uintX);

	    uint32 *texel = _tex->_buffer + (sy * _tex->w + sx);
	#ifdef BLIT_WRAP
	    uint32 *texel_line_start = _tex->_buffer + sy * _tex->w;
	    uint32 *texel_line_end = _tex->_buffer + (sy+1) * _tex->w;
	    uint32 *texel_col_start = _tex->_buffer + sx;
	    uint32 *texel_col_end = _tex->_buffer + (_tex->_height * _tex->w + sx);
	#endif
	    int tex_diff = _tex->w - w;

	    //b = PACK_RGB8( (rgb>>16)&0xFF , (rgb>>8)&0xFF , rgb&0xFF );

	    while (pixel != end)
	    {
	        while (pixel != line_end)
	        {
	            if (*texel & TEX32_A_MASK)
	            {
	                *(reinterpret_cast<uintX*>(pixel)) = static_cast<uintX>(PACK_RGB8( TEX32_R(*texel), TEX32_G(*texel), TEX32_B(*texel) ));
	            }
	            pixel+=sizeof(uintX);
	            texel++;
	#ifdef BLIT_WRAP
	            if (texel == texel_line_end) texel = texel_line_start;
	#endif
	        }

	        line_end += _pitch;
	        pixel += diff;
	        texel+= tex_diff;
	#ifdef BLIT_WRAP
	        if (texel == texel_col_end) texel = texel_col_start;
	#endif
	    }

	*/


}


//
// void SoftRenderSurface::FadedBlit(Texture *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32)
//
// Desc: Blit a region from a Texture (Alpha == 0 -> skipped)
//
template<class uintX> void SoftRenderSurface<uintX>::FadedBlit(Texture *_tex, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
	// Clamp or wrap or return?
	if (w > static_cast<int32>(_tex->w))
		return;

	// Clamp or wrap or return?
	if (h > static_cast<int32>(_tex->h))
		return;

	// Clip to window
	int px = dx, py = dy;
	_clipWindow.IntersectOther(dx, dy, w, h);
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

	if (_tex->_format == TEX_FMT_STANDARD) {
		uint32 *texel = (uint32 *)_tex->getBasePtr(sx, sy);
		int tex_diff = _tex->w - w;

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
	} else if (_tex->_format == TEX_FMT_NATIVE) {
		uintX *texel = reinterpret_cast<uintX *>(_tex->getBasePtr(sx, sy));
		int tex_diff = _tex->w - w;

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
	}
}


//
// void SoftRenderSurface::MaskedBlit(Texture *, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend=false)
//
// Desc Blit a region from a Texture with a Colour blend masked based on DestAlpha (AlphaTex == 0 || AlphaDest == 0 -> skipped. AlphaCol32 -> Blend Factors)
//
//
template<class uintX> void SoftRenderSurface<uintX>::MaskedBlit(Texture *_tex, int32 sx, int32 sy, int32 w, int32 h, int32 dx, int32 dy, uint32 col32, bool alpha_blend) {
	// Clamp or wrap or return?
	if (w > static_cast<int32>(_tex->w))
		return;

	// Clamp or wrap or return?
	if (h > static_cast<int32>(_tex->h))
		return;

	// Clip to window
	int px = dx, py = dy;
	_clipWindow.IntersectOther(dx, dy, w, h);
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

	if (_tex->_format == TEX_FMT_STANDARD) {
		uint32 *texel = (uint32 *)_tex->getBasePtr(sx, sy);
		int tex_diff = _tex->w - w;

		while (pixel != end) {
			if (!alpha_blend) {
				while (pixel != line_end) {
					uintX *dest = reinterpret_cast<uintX *>(pixel);

					if (*texel & TEX32_A_MASK) {
						if (!RenderSurface::_format.a_mask || (*dest & RenderSurface::_format.a_mask)) {
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

					if (!RenderSurface::_format.a_mask || (*dest & RenderSurface::_format.a_mask)) {
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
	} else if (_tex->_format == TEX_FMT_NATIVE) {
		uintX *texel = reinterpret_cast<uintX *>(_tex->getBasePtr(sx, sy));
		int tex_diff = _tex->w - w;

		while (pixel != end) {
			while (pixel != line_end) {
				uintX *dest = reinterpret_cast<uintX *>(pixel);

				// Uh, not completely supported right now
				//if ((*texel & RenderSurface::_format.a_mask) && (*dest & RenderSurface::_format.a_mask))
				if (*dest & RenderSurface::_format.a_mask) {
					*dest = BlendHighlight(*texel, r, g, b, 1, ia);
				}
				pixel += sizeof(uintX);
				texel++;
			}

			line_end += _pitch;
			pixel += diff;
			texel += tex_diff;
		}
	}
}



//
// void SoftRenderSurface::StretchBlit(Texture *, int32 sx, int32 sy, int32 sw, int32 sh, int32 dx, int32 dy, int32 dw, int32 dh, bool clampedges)
//
// Desc: Blit a region from a Texture, and arbitrarily stretch it to fit the dest region
//
//
template<class uintX> void SoftRenderSurface<uintX>::StretchBlit(Texture *texture,
        int32 sx, int32 sy, int32 sw, int32 sh,
        int32 dx, int32 dy, int32 dw, int32 dh,
        bool clampedges) {
	// Nothing we can do
	if ((sh <= 0) || (dh <= 0) || (sw <= 0) || (dw <= 0)) return;

	// 1x No scaling needed
	if (dw == sw && sh == dh) {
		Blit(texture, sw, sy, sw, sh, dx, dy);
		return;
	}

	uint8 *pixel = _pixels + dy * _pitch + dx * sizeof(uintX);
	Ultima8Engine::get_instance()->point_scaler.Scale(texture, sx, sy, sw, sh, pixel, dw, dh, _pitch, clampedges);
}

//
// bool SoftRenderSurface::ScalerBlit(Texture *texure, int32 sx, int32 sy, int32 sw, int32 sh, int32 dx, int32 dy, int32 dw, int32 dh, const Scaler *scaler, bool clampedges)
//
// Desc: Blit a region from a Texture using a scaler
//
//
template<class uintX> bool SoftRenderSurface<uintX>::ScalerBlit(Texture *texture, int32 sx, int32 sy, int32 sw, int32 sh, int32 dx, int32 dy, int32 dw, int32 dh, const Scaler *scaler, bool clampedges) {
	// Nothing we can do
	if ((sh <= 0) || (dh <= 0) || (sw <= 0) || (dw <= 0)) return false;

	// 1x No scaling needed (but still do it anyway, could be a filter????)
	if (dw == sw && sh == dh) {
		Blit(texture, sw, sy, sw, sh, dx, dy);
		return true;
	}

	uint8 *pixel = _pixels + dy * _pitch + dx * sizeof(uintX);

	return scaler->Scale(texture, sx, sy, sw, sh, pixel, dw, dh, _pitch, clampedges);
}

//
// SoftRenderSurface::PrintCharFixed(FixedWidthFont *, char character, int x, int y)
//
// Desc: Draw a fixed width character from a Texture buffer
//
template<class uintX> void SoftRenderSurface<uintX>::PrintCharFixed(FixedWidthFont *font, int character, int x, int y) {
	if (character == ' ') return;   // Don't paint spaces

	int align_x = font->_alignX;
	int align_y = font->_alignY;
	int char_width = font->_width;
	int char_height = font->_height;
	Texture *texture = font->_tex;

	if (align_x == 16 && align_y == 16) {
		SoftRenderSurface::Blit(texture, (character & 0x0F) << 4, character & 0xF0, char_width, char_height, x, y);
	} else if (align_x == 8 && align_y == 8) {
		SoftRenderSurface::Blit(texture, (character & 0x0F) << 3, (character >> 1) & 0x78, char_width, char_height, x, y);
	} else {
		SoftRenderSurface::Blit(texture, (character & 0x0F) * align_x, (character & 0xF0 >> 4) * align_y, char_width, char_height, x, y);
	}
}


//
// SoftRenderSurface::PrintTextFixed(FixedWidthFont *, const char *text, int x, int y)
//
// Desc: Draw fixed width from a Texture buffer (16x16 characters fixed width and height)
//
template<class uintX> void SoftRenderSurface<uintX>::PrintTextFixed(FixedWidthFont *font, const char *text, int x, int y) {
	int align_x = font->_alignX;
	int align_y = font->_alignY;
	int char_width = font->_width;
	int char_height = font->_height;
	Texture *texture = font->_tex;

	int character;
	if (align_x == 16 && align_y == 16) while (0 != (character = *text)) {
			SoftRenderSurface::Blit(texture, (character & 0x0F) << 4, character & 0xF0, char_width, char_height, x, y);
			++text;
			x += char_width;
		}
	else if (align_x == 8 && align_y == 8) while (0 != (character = *text)) {
			SoftRenderSurface::Blit(texture, (character & 0x0F) << 3, (character >> 1) & 0x78, char_width, char_height, x, y);
			++text;
			x += char_width;
		}
	else while (0 != (character = *text)) {
			SoftRenderSurface::Blit(texture, (character & 0x0F) * align_x, (character & 0xF0 >> 4) * align_y, char_width, char_height, x, y);
			++text;
			x += char_width;
		}
}


//
// void SoftRenderSurface::Paint(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Clips but doesn't do anything else
//
template<class uintX> void SoftRenderSurface<uintX>::Paint(Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#include "ultima/ultima8/graphics/soft_render_surface.inl"
}


//
// void SoftRenderSurface::PaintNoClip(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Doesn't clip
//
template<class uintX> void SoftRenderSurface<uintX>::PaintNoClip(Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#define NO_CLIPPING
#include "ultima/ultima8/graphics/soft_render_surface.inl"
#undef NO_CLIPPING
}


//
// void SoftRenderSurface::PaintTranslucent(Shape*s, uint32 framenum, int32 x, int32 y)
//
// Desc: Standard shape drawing functions. Clips and XForms
//
template<class uintX> void SoftRenderSurface<uintX>::PaintTranslucent(Shape *s, uint32 framenum, int32 x, int32 y, bool untformed_pal) {
#define XFORM_SHAPES
#include "ultima/ultima8/graphics/soft_render_surface.inl"
#undef XFORM_SHAPES
}


//
// void SoftRenderSurface::PaintMirrored(Shape*s, uint32 framenum, int32 x, int32 y, bool trans)
//
// Desc: Standard shape drawing functions. Clips, Flips and conditionally XForms
//
template<class uintX> void SoftRenderSurface<uintX>::PaintMirrored(Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool untformed_pal) {
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

template<class uintX> void SoftRenderSurface<uintX>::PaintInvisible(Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, bool untformed_pal) {
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

template<class uintX> void SoftRenderSurface<uintX>::PaintHighlight(Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
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

template<class uintX> void SoftRenderSurface<uintX>::PaintHighlightInvis(Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
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
// void SoftRenderSurface::PaintHighlight(Shape* s, uint32 frame, int32 x, int32 y, bool mirrored)
//
// Desc: Standard shape drawing functions. Masked against Dest Alpha. Highlights, Clips, and conditionally Flips and Xforms
//

template<class uintX> void SoftRenderSurface<uintX>::PaintMasked(Shape *s, uint32 framenum, int32 x, int32 y, bool trans, bool mirrored, uint32 col32, bool untformed_pal) {
#define FLIP_SHAPES
#define FLIP_CONDITIONAL mirrored
#define XFORM_SHAPES
#define XFORM_CONDITIONAL trans
#define BLEND_SHAPES(src,dst) BlendHighlight(src,cr,cg,cb,ca,255-ca)
#define DESTALPHA_MASK

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
#undef DESTALPHA_MASK
}

//
// Instantiate the SoftRenderSurface Class
//
template class SoftRenderSurface<uint16>;
template class SoftRenderSurface<uint32>;

} // End of namespace Ultima8
} // End of namespace Ultima
