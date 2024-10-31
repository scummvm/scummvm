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

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy
#define FORBIDDEN_SYMBOL_EXCEPTION_strcat

// Following are pulled in for wchar.h header on XCode iOS-7
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetwc
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetws
#define FORBIDDEN_SYMBOL_EXCEPTION_asctime
#define FORBIDDEN_SYMBOL_EXCEPTION_clock
#define FORBIDDEN_SYMBOL_EXCEPTION_ctime
#define FORBIDDEN_SYMBOL_EXCEPTION_difftime
#define FORBIDDEN_SYMBOL_EXCEPTION_getdate
#define FORBIDDEN_SYMBOL_EXCEPTION_gmtime
#define FORBIDDEN_SYMBOL_EXCEPTION_localtime
#define FORBIDDEN_SYMBOL_EXCEPTION_mktime
#define FORBIDDEN_SYMBOL_EXCEPTION_time

#include "common/scummsys.h"

#ifdef USE_FREETYPE2

#include <wchar.h>
#include "ags/lib/alfont/alfont.h"
#include "ags/lib/allegro/color.h"
#include "ags/lib/allegro/draw.h"
#include "ags/lib/allegro/gfx.h"
#include "ags/lib/allegro/unicode.h"
#include "graphics/fonts/freetype.h"

#ifndef NO_FT213_AUTOHINT
#include "ags/lib/freetype-2.1.3/autohint/ahhint.h"
#endif

namespace AGS3 {

using Graphics::FreeType::Init_FreeType;
using Graphics::FreeType::Init_FreeType_With_Mem;
using Graphics::FreeType::Done_FreeType;
using Graphics::FreeType::Done_FreeType_With_Mem;
using Graphics::FreeType::Load_Glyph;
using Graphics::FreeType::Get_Glyph;
using Graphics::FreeType::Glyph_Copy;
using Graphics::FreeType::Glyph_To_Bitmap;
using Graphics::FreeType::Done_Glyph;
using Graphics::FreeType::Set_Pixel_Sizes;
using Graphics::FreeType::New_Face;
using Graphics::FreeType::New_Memory_Face;
using Graphics::FreeType::Done_Face;
using Graphics::FreeType::Get_Char_Index;
using Graphics::FreeType::Get_Kerning;


#undef TRUE
#undef FALSE
#define TRUE  -1
#define FALSE  0

/* structs */
struct _ALFONT_CACHED_GLYPH {
	char is_cached;
	int width, height, aawidth, aaheight;
	int left, top, aaleft, aatop;
	int advancex, advancey;
	int mono_available, aa_available;
	byte *bmp;
	byte *aabmp;
};

struct ALFONT_FONT {
	FT_Face face;           /* face */
	int face_h;             /* face height */
	int real_face_h;        /* real face height */
	int face_ascender;      /* face ascender */
	int real_face_extent_asc;  /* calculated max extent of glyphs (ascender) */
	int real_face_extent_desc; /* calculated max extent of glyphs (descender) */
	char *data;             /* if loaded from memory, the data chunk */
	int data_size;          /* and its size */
	int ch_spacing;         /* extra spacing */
	int num_fixed_sizes;    /* -1 if scalable, >=0 if fixed */
	_ALFONT_CACHED_GLYPH *cached_glyphs;  /* array to know which glyphs have been cached */
	int *fixed_sizes;       /* array with the fixed sizes */
	char *language;		  /* language */
	int type;				  /* Code Convert(Please Use TYPE_WIDECHAR for ASCII to UNICODE) */
	int outline_top;		  /* Font top outline width */
	int outline_bottom;	  /* Font bottom outline width */
	int outline_right;	  /* Font right outline width */
	int outline_left;		  /* Font left outline width */
	int outline_color;	  /* Font outline color */
	int outline_hollow;	  /* Font hollow(TRUE/FALSE) */
	int style;			  /* Font Style(STYLE_STANDARD/STYLE_ITALIC/STYLE_BOLD/STYLE_BOLDITALIC) */
	int underline;		  /* Font underline(TRUE/FALSE) */
	int underline_right;	  /* Extend right underline(TRUE/FALSE) */
	int underline_left;	  /* Extend left underline(TRUE/FALSE) */
	int background;		  /* Font Background Color(TRUE/FALSE) */
	int transparency;		  /* Font transparency(0-255) */
	int autofix;			  /* Font autofix(TRUE/FALSE) */
	int precedingchar;      /* preceding character for autofix*/
	int fixed_width;		  /* Font fixed width(TRUE/FALSE) */
};


/* global vars */
BITMAP *default_bmp; //Draw Font on default BITMAP;
static FT_Library ft_library;
static int alfont_textmode = 0;
static int alfont_inited = 0;
static FT_Memory ft_memory;
#ifndef NO_FT213_AUTOHINT
static FreeType213::AH_Hinter ft_hinter;
#endif

const char *alfont_get_name(ALFONT_FONT *f) {
	if (!f)
		return "";

	return ((FT_FaceRec *)(f->face))->family_name;
}


/*
   JJS: These functions replace the standard Allegro blender.
   Code is reverse-engineered from the alfont MSVC library.
   The blender functions are based on the originals with small modifications
   that enable correct drawing of anti-aliased fonts.
*/

/* original: _blender_trans15 in colblend.c */
uint32_t __skiptranspixels_blender_trans15(uint32_t x, uint32_t y, uint32_t n) {
	uint32_t result;

	if ((y & 0xFFFF) == 0x7C1F)
		return x;

	if (n)
		n = (n + 1) / 8;

	x = ((x & 0xFFFF) | (x << 16)) & 0x3E07C1F;
	y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

	result = ((x - y) * n / 32 + y) & 0x3E07C1F;

	return ((result & 0xFFFF) | (result >> 16));
}

/* original: _blender_trans16 in colblend.c */
uint32_t __skiptranspixels_blender_trans16(uint32_t x, uint32_t y, uint32_t n) {
	uint32_t result;

	if ((y & 0xFFFF) == 0xF81F)
		return x;

	if (n)
		n = (n + 1) / 8;

	x = ((x & 0xFFFF) | (x << 16)) & 0x7E0F81F;
	y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

	result = ((x - y) * n / 32 + y) & 0x7E0F81F;

	return ((result & 0xFFFF) | (result >> 16));
}

/* original: _blender_trans24 in colblend.c */
uint32_t __preservedalpha_blender_trans24(uint32_t x, uint32_t y, uint32_t n) {
	uint32_t res, g, alpha;

	alpha = (y & 0xFF000000);

	if ((y & 0xFFFFFF) == 0xFF00FF)
		return ((x & 0xFFFFFF) | (n << 24));

	if (n)
		n++;

	res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
	y &= 0xFF00;
	x &= 0xFF00;
	g = (x - y) * n / 256 + y;

	res &= 0xFF00FF;
	g &= 0xFF00;

	return res | g | alpha;
}

/* replaces set_trans_blender() */
void set_preservedalpha_trans_blender(int r, int g, int b, int a) {
	// TODO: The current putpixel() implementation does not support blending in DRAW_MODE_TRANS mode (which is not implemented),
	// so we can't just call set_blender_mode() here.
	// The actual blending is done by the apply_trans_blender() function, just before the putpixel() calls

	//set_blender_mode(__skiptranspixels_blender_trans15, __skiptranspixels_blender_trans16, __preservedalpha_blender_trans24, r, g, b, a);
	//set_blender_mode(kAlphaPreservedBlenderMode, r, g, b, a);
}

/* blends a pixel using the alternative blenders, this is a replacement
 * for the previous function using set_blender_mode
 */
int apply_trans_blender(BITMAP *bmp, int color1, int color2, int alpha) {
	switch (bitmap_color_depth(bmp)) {
	case 15:
		return __skiptranspixels_blender_trans15(color1, color2, alpha);
	case 16:
		return __skiptranspixels_blender_trans16(color1, color2, alpha);
	case 24:
	case 32:
		return __preservedalpha_blender_trans24(color1, color2, alpha);
	default:
		return color1;
	}
}

/* helpers */

static void _alfont_reget_fixed_sizes(ALFONT_FONT *f) {
	if (f->num_fixed_sizes < 0) {
		/* scalable font */
		f->fixed_sizes[0] = -1;
	} else {
		/* fixed */
		int i;
		for (i = 0; i < f->num_fixed_sizes; i++) {
			f->fixed_sizes[i] = f->face->available_sizes[i].height;
		}
		/* set last one to -1 */
		f->fixed_sizes[f->num_fixed_sizes] = -1;
	}
}


static void _alfont_uncache_glyphs(ALFONT_FONT *f) {
	if (f->cached_glyphs) {
		int i;
		for (i = 0; i < f->face->num_glyphs; i++) {
			if (f->cached_glyphs[i].is_cached) {
				f->cached_glyphs[i].is_cached = 0;
				if (f->cached_glyphs[i].bmp) {
					free(f->cached_glyphs[i].bmp);
					f->cached_glyphs[i].bmp = NULL;
				}
				if (f->cached_glyphs[i].aabmp) {
					free(f->cached_glyphs[i].aabmp);
					f->cached_glyphs[i].aabmp = NULL;
				}
			}
		}
	}
}


static void _alfont_uncache_glyph_number(ALFONT_FONT *f, int glyph_number) {
	if ((glyph_number < 0) || (glyph_number >= f->face->num_glyphs))
		return;
	if (f->cached_glyphs) {
		if (f->cached_glyphs[glyph_number].is_cached) {
			f->cached_glyphs[glyph_number].is_cached = 0;
			if (f->cached_glyphs[glyph_number].bmp) {
				free(f->cached_glyphs[glyph_number].bmp);
				f->cached_glyphs[glyph_number].bmp = NULL;
			}
			if (f->cached_glyphs[glyph_number].aabmp) {
				free(f->cached_glyphs[glyph_number].aabmp);
				f->cached_glyphs[glyph_number].aabmp = NULL;
			}
		}
	}
}


static void _alfont_delete_glyphs(ALFONT_FONT *f) {
	_alfont_uncache_glyphs(f);
	if (f->cached_glyphs) {
		free(f->cached_glyphs);
		f->cached_glyphs = NULL;
	}
}


static void _alfont_cache_glyph(ALFONT_FONT *f, int glyph_number) {
	if ((glyph_number < 0) || (glyph_number >= f->face->num_glyphs))
		return;

	/* if glyph not cached yet */
	if (!f->cached_glyphs[glyph_number].is_cached) {
		FT_Glyph new_glyph;

		/* load the font glyph */

#ifdef NO_FT213_AUTOHINT
		Load_Glyph(f->face, glyph_number, FT_LOAD_DEFAULT);
#else
		FT_Int32 load_flags = FT_LOAD_DEFAULT;
		FT_GlyphSlot slot = f->face->glyph;

		FreeType213::ah_hinter_load_glyph(ft_hinter, slot, f->face->size, glyph_number, FT_LOAD_DEFAULT);

		/* compute the advance */
		if (load_flags & FT_LOAD_VERTICAL_LAYOUT) {
			slot->advance.x = 0;
			slot->advance.y = slot->metrics.vertAdvance;
		} else {
			slot->advance.x = slot->metrics.horiAdvance;
			slot->advance.y = 0;
		}

		/* compute the linear advance in 16.16 pixels */
		if ((load_flags & FT_LOAD_LINEAR_DESIGN) == 0) {
			FT_UInt EM = f->face->units_per_EM;
			FT_Size_Metrics *metrics = &f->face->size->metrics;

			slot->linearHoriAdvance = FT_MulDiv(slot->linearHoriAdvance, (FT_Long)metrics->x_ppem << 16, EM);
			slot->linearVertAdvance = FT_MulDiv(slot->linearVertAdvance, (FT_Long)metrics->y_ppem << 16, EM);
		}
#endif

		Get_Glyph(f->face->glyph, &new_glyph);

		/* ok, this glyph is now cached */
		f->cached_glyphs[glyph_number].is_cached = 1;
		f->cached_glyphs[glyph_number].mono_available = 0;
		f->cached_glyphs[glyph_number].aa_available = 0;

		/* render the mono bmp */
		{
			FT_Bitmap *ft_bmp;
			FT_Glyph glyph;
			FT_BitmapGlyph bmp_glyph;

			Glyph_Copy(new_glyph, &glyph);

			/* only render glyph if it is not already a bitmap */
			if (glyph->format != ft_glyph_format_bitmap)
				Glyph_To_Bitmap(&glyph, ft_render_mode_mono, NULL, 1);

			/* the FT rendered bitmap */
			bmp_glyph = (FT_BitmapGlyph)glyph;
			ft_bmp = &bmp_glyph->bitmap;

			/* save only if the bitmap is really 1 bit */
			if (ft_bmp->pixel_mode == ft_pixel_mode_mono) {
				int memsize;

				f->cached_glyphs[glyph_number].mono_available = 1;

				/* set width, height, left, top */
				f->cached_glyphs[glyph_number].width = ft_bmp->width;
				f->cached_glyphs[glyph_number].height = ft_bmp->rows;
				f->cached_glyphs[glyph_number].left = bmp_glyph->left;
				f->cached_glyphs[glyph_number].top = bmp_glyph->top;

				/* allocate bitmap */
				memsize = ft_bmp->width * ft_bmp->rows * sizeof(unsigned char);
				if (memsize > 0)
					f->cached_glyphs[glyph_number].bmp = (byte *)malloc(memsize);
				else
					f->cached_glyphs[glyph_number].bmp = NULL;

				/* monochrome drawing */
				if (memsize > 0) {
					unsigned char *outbmp_p = f->cached_glyphs[glyph_number].bmp;
					unsigned char *bmp_p;
					int bmp_x, bmp_y, bit;

					/* copy the FT character bitmap to ours */
					bmp_p = ft_bmp->buffer;
					for (bmp_y = 0; bmp_y < (int)ft_bmp->rows; bmp_y++) {
						unsigned char *next_bmp_p;
						next_bmp_p = bmp_p + ft_bmp->pitch;
						bit = 7;
						for (bmp_x = 0; bmp_x < (int)ft_bmp->width; bmp_x++) {
							*outbmp_p = *bmp_p & (1 << bit);
							outbmp_p++;

							if (bit == 0) {
								bit = 7;
								bmp_p++;
							} else
								bit--;
						}
						bmp_p = next_bmp_p;
					}
				}
			}

			Done_Glyph(glyph);
		}


		/* render the aa bmp */
		{
			FT_Bitmap *ft_bmp;
			FT_Glyph glyph;
			FT_BitmapGlyph bmp_glyph;

			Glyph_Copy(new_glyph, &glyph);

			/* only render glyph if it is not already a bitmap */
			if (glyph->format != ft_glyph_format_bitmap)
				Glyph_To_Bitmap(&glyph, ft_render_mode_normal, NULL, 1);

			/* the FT rendered bitmap */
			bmp_glyph = (FT_BitmapGlyph)glyph;
			ft_bmp = &bmp_glyph->bitmap;

			/* save only if the bitmap is really 8 bit */
			if (ft_bmp->pixel_mode == ft_pixel_mode_grays) {
				int memsize;

				f->cached_glyphs[glyph_number].aa_available = 1;

				/* set width, height, left, top */
				f->cached_glyphs[glyph_number].aawidth = ft_bmp->width;
				f->cached_glyphs[glyph_number].aaheight = ft_bmp->rows;
				f->cached_glyphs[glyph_number].aaleft = bmp_glyph->left;
				f->cached_glyphs[glyph_number].aatop = bmp_glyph->top;

				/* allocate bitmap */
				memsize = ft_bmp->width * ft_bmp->rows * sizeof(unsigned char);
				if (memsize > 0)
					f->cached_glyphs[glyph_number].aabmp = (byte *)malloc(memsize);
				else
					f->cached_glyphs[glyph_number].aabmp = NULL;

				/* aa drawing */
				if (memsize > 0) {
					unsigned char *outbmp_p = f->cached_glyphs[glyph_number].aabmp;
					unsigned char *bmp_p;
					int bmp_y;
					unsigned char mul = 256 / ft_bmp->num_grays;
					/* we set it to 0 because it is faster to test for false */
					if (mul == 1)
						mul = 0;

					/* copy the FT character bitmap to ours */
					bmp_p = ft_bmp->buffer;
					for (bmp_y = 0; bmp_y < (int)ft_bmp->rows; bmp_y++) {
						unsigned char *next_bmp_p;
						next_bmp_p = bmp_p + ft_bmp->pitch;
						memcpy(outbmp_p, bmp_p, ft_bmp->width * sizeof(unsigned char));

						/* we have to change our pixels if the numgrays is not 256 */
						if (mul) {
							unsigned char *p = outbmp_p;
							unsigned char *p_end = p + ft_bmp->width;
							for (; p < p_end; p++)
								*p *= mul;
						}

						outbmp_p += ft_bmp->width;
						bmp_p = next_bmp_p;
					}
				}
			}

			Done_Glyph(glyph);
		}

		f->cached_glyphs[glyph_number].advancex = f->face->glyph->advance.x >> 6;
		f->cached_glyphs[glyph_number].advancey = f->face->glyph->advance.y >> 6;

		/* delete the glyph */
		Done_Glyph(new_glyph);
	}
}


static void _alfont_new_cache_glyph(ALFONT_FONT *f) {
	int i;

	if (!f->cached_glyphs)
		f->cached_glyphs = (_ALFONT_CACHED_GLYPH *)malloc(f->face->num_glyphs * sizeof(struct _ALFONT_CACHED_GLYPH));

	for (i = 0; i < f->face->num_glyphs; i++) {
		f->cached_glyphs[i].is_cached = 0;
		f->cached_glyphs[i].bmp = NULL;
		f->cached_glyphs[i].aabmp = NULL;
	}
}

static void _alfont_calculate_max_cbox(ALFONT_FONT *f, int max_glyphs) {
	(void) max_glyphs; // kept just in case, but this was used to load N glyphs

	FT_Long bbox_ymin = FT_MulFix(FT_DivFix(f->face->bbox.yMin, f->face->units_per_EM), f->face->size->metrics.y_ppem);
	FT_Long bbox_ymax = FT_MulFix(FT_DivFix(f->face->bbox.yMax, f->face->units_per_EM), f->face->size->metrics.y_ppem);

	f->real_face_extent_asc = (int)bbox_ymax;
	f->real_face_extent_desc = -(int)bbox_ymin;
}

/* API */

int alfont_set_font_size(ALFONT_FONT *f, int h) {
	return alfont_set_font_size_ex(f, h, 0);
}

int alfont_set_font_size_ex(ALFONT_FONT *f, int h, int flags) {
	int error, test_h, direction;
	int real_height = 0;
	/* check the font doesn't already use that w and h */
	if ((h == f->face_h) && (flags & ALFONT_FLG_FORCE_RESIZE) == 0)
		return ALFONT_OK;
	else if (h <= 0)
		return ALFONT_ERROR;

	/* keep changing the size until the real size is not the one */
	/* we want */
	test_h = h;
	direction = 0;
	while (1) {
		error = Set_Pixel_Sizes(f->face, 0, test_h);
		if (error)
			break;

		/* compare real height with asked height */
		real_height = abs(f->face->size->metrics.ascender >> 6) + abs(f->face->size->metrics.descender >> 6);

		// AGS COMPAT HACK: always choose the first result
		if ((flags & ALFONT_FLG_SELECT_NOMINAL_SZ) != 0)
			break;

		if (real_height == h) {
			/* we found the wanted height */
			break;
		}

		/* check the direction */
		if (direction == 0) {
			/* direction still not set */
			if (real_height > h)
				direction = -1;
			else
				direction = 1;
		}

		/* check we didn't overpass it */
		else if ((direction > 0) && (real_height > h)) {
			/* decrease one and found */
			test_h--;
			Set_Pixel_Sizes(f->face, 0, test_h);
			break;
		}

		/* check we didn't surpass it */
		else if ((direction < 0) && (real_height < h)) {
			break;
		}

		test_h += direction;

		/* check we arent at 0 */
		if (test_h <= 0) {
			error = TRUE;
			break;
		}
	}

	if (!error) {
		_alfont_uncache_glyphs(f);
		f->face_h = test_h;
		f->real_face_h = real_height;
		f->face_ascender = f->face->size->metrics.ascender >> 6;

		/* Precalculate actual glyphs vertical extent */
		if ((flags & ALFONT_FLG_PRECALC_MAX_CBOX) != 0) {
			_alfont_calculate_max_cbox(f, 256);
		}
		/* AGS COMPAT HACK: set ascender to the formal font height */
		if ((flags & ALFONT_FLG_ASCENDER_EQ_HEIGHT) != 0) {
			f->face_ascender = test_h;
			f->real_face_h = test_h + abs(f->face->size->metrics.descender >> 6);
		}

		return ALFONT_OK;
	} else {
		Set_Pixel_Sizes(f->face, 0, f->real_face_h);
		return ALFONT_ERROR;
	}
}


int alfont_get_font_height(ALFONT_FONT *f) {
	return f->face_h;
}

/* Return font height based on ascender + descender summation */
int alfont_get_font_real_height(ALFONT_FONT *f) {
	return f->real_face_h;
}

ALFONT_DLL_DECLSPEC void alfont_get_font_real_vextent(ALFONT_FONT *f, int *top, int *bottom) {
	*top = f->face_ascender - f->real_face_extent_asc; // may be negative
	*bottom = f->face_ascender + f->real_face_extent_desc;
}

void alfont_exit(void) {
	if (alfont_inited) {
		alfont_inited = 0;
#ifndef NO_FT213_AUTOHINT
		FreeType213::ah_hinter_done(ft_hinter);
#endif
		Done_FreeType_With_Mem(ft_library, ft_memory);
		memset(&ft_library, 0, sizeof(ft_library));
	}
}


int alfont_init(void) {
	if (alfont_inited)
		return 0;
	else {
		int error;
		memset(&ft_library, 0, sizeof(ft_library));
		error = Init_FreeType_With_Mem(&ft_library, &ft_memory);

		if (!error) {
#ifndef NO_FT213_AUTOHINT
			error = FreeType213::ah_hinter_new(ft_memory, &ft_hinter);
			if (error) {
				Done_FreeType_With_Mem(ft_library, ft_memory);
			}
#endif

			if (!error) {
				alfont_inited = 1;
			}
		}

		return error;
	}
}


ALFONT_FONT *alfont_load_font(const char *filepathname) {
	int error;

	/* try to allocate the memory */
	ALFONT_FONT *font = (ALFONT_FONT * )malloc(sizeof(ALFONT_FONT));

	if (font == NULL)
		return NULL;

	/* clear the struct */
	memset(font, 0, sizeof(ALFONT_FONT));
	font->cached_glyphs = NULL;

	/* we are loading from file, no mem buffer needed */
	font->data = NULL;
	font->data_size = 0;

	/* load the font */
	error = New_Face(ft_library, filepathname, 0, &font->face);

	if (error) {
		free(font);
		return NULL;
	}

	/* get if the font contains only fixed sizes */
	if (!(font->face->face_flags & FT_FACE_FLAG_SCALABLE))
		font->num_fixed_sizes = font->face->num_fixed_sizes;
	else
		font->num_fixed_sizes = -1;

	_alfont_new_cache_glyph(font);

	if (font->num_fixed_sizes < 0) {
		font->fixed_sizes = (int *)malloc(sizeof(int));
		_alfont_reget_fixed_sizes(font);

		alfont_set_font_size(font, 8);
	} else {
		font->fixed_sizes = (int *)malloc(sizeof(int) * (font->num_fixed_sizes + 1));
		_alfont_reget_fixed_sizes(font);

		/* set as current size the first found fixed size */
		alfont_set_font_size(font, font->fixed_sizes[0]);
	}

	alfont_set_char_extra_spacing(font, 0);

	//Initial Font attribute
	font->language = NULL;		   /* Initial Language */
	font->type = 0;				   /* Initial Code Convert */
	font->outline_top = 0;		   /* Initial Font top outline width */
	font->outline_bottom = 0;	   /* Initial Font bottom outline width */
	font->outline_left = 0;		   /* Initial Font left outline width */
	font->outline_right = 0;	   /* Initial Font right outline width */
	font->outline_color = 0;	   /* Initial Font outline color */
	font->outline_hollow = FALSE;  /* Initial Font hollow(TRUE/FALSE) */
	font->style = 0;			   /* Initial Font Style */
	font->underline = FALSE;	   /* Initial Font underline(TRUE/FALSE) */
	font->underline_right = FALSE; /* Initial Extend right underline(TRUE/FALSE) */
	font->underline_left = FALSE;  /* Initial Extend left underline(TRUE/FALSE) */
	font->background = FALSE;	   /* Initial Font Background Color(TRUE/FALSE) */
	font->transparency = 255;	   /* Initial Font transparency(0-255) */
	font->autofix = FALSE;		   /* Initial Font autofix(TRUE/FALSE) */
	font->precedingchar = 0;	   /* Initial preceding character */

	return font;
}


ALFONT_FONT *alfont_load_font_from_mem(const char *data, int data_len) {
	int error;
	char *new_data;

	/* try to allocate the memory */
	ALFONT_FONT *font = (ALFONT_FONT *)malloc(sizeof(ALFONT_FONT));
	new_data = (char *)malloc(data_len);

	if ((font == NULL) || (new_data == NULL)) {
		if (font)
			free(font);
		if (new_data)
			free(new_data);
		return NULL;
	}

	/* clear the struct */
	memset(font, 0, sizeof(ALFONT_FONT));
	font->cached_glyphs = NULL;

	/* copy user data to internal buffer */
	font->data = new_data;
	font->data_size = data_len;
	memcpy((void *)font->data, (const void *)data, data_len);

	/* load the font */
	error = New_Memory_Face(ft_library, (const FT_Byte *)font->data, font->data_size, 0, &font->face);

	if (error) {
		free(font->data);
		free(font);
		return NULL;
	}

	/* get if the font contains only fixed sizes */
	if (!(font->face->face_flags & FT_FACE_FLAG_SCALABLE))
		font->num_fixed_sizes = font->face->num_fixed_sizes;
	else
		font->num_fixed_sizes = -1;

	_alfont_new_cache_glyph(font);

	if (font->num_fixed_sizes < 0) {
		font->fixed_sizes = (int *)malloc(sizeof(int));
		_alfont_reget_fixed_sizes(font);

		alfont_set_font_size(font, 8);
	} else {
		font->fixed_sizes = (int *)malloc(sizeof(int) * (font->num_fixed_sizes + 1));
		_alfont_reget_fixed_sizes(font);

		/* set as current size the first found fixed size */
		alfont_set_font_size(font, font->fixed_sizes[0]);
	}

	alfont_set_char_extra_spacing(font, 0);

	//Initial Font attribute
	font->language = NULL;		   /* Initial Language */
	font->type = 0;				   /* Initial Code Convert */
	font->outline_top = 0;		   /* Initial Font top outline width */
	font->outline_bottom = 0;	   /* Initial Font bottom outline width */
	font->outline_left = 0;		   /* Initial Font left outline width */
	font->outline_right = 0;	   /* Initial Font right outline width */
	font->outline_color = 0;	   /* Initial Font outline color */
	font->outline_hollow = FALSE;  /* Initial Font hollow(TRUE/FALSE) */
	font->style = 0;			   /* Initial Font Style */
	font->underline = FALSE;	   /* Initial Font underline(TRUE/FALSE) */
	font->underline_right = FALSE; /* Initial Extend right underline(TRUE/FALSE) */
	font->underline_left = FALSE;  /* Initial Extend left underline(TRUE/FALSE) */
	font->background = FALSE;	   /* Initial Font Background Color(TRUE/FALSE) */
	font->transparency = 255;	   /* Initial Font transparency(0-255) */
	font->autofix = FALSE;		   /* Initial Font autofix(TRUE/FALSE) */
	font->precedingchar = 0;	   /* Initial preceding character */

	return font;
}


int alfont_text_mode(int mode) {
	int old_mode = alfont_textmode;
	alfont_textmode = mode;
	return old_mode;
}


void alfont_destroy_font(ALFONT_FONT *f) {
	if (f == NULL)
		return;

	/* delete old glyphs */
	_alfont_delete_glyphs(f);

	/* delete the face */
	Done_Face(f->face);

	if (f->fixed_sizes)
		free(f->fixed_sizes);

	/* deallocate the data */
	if (f->data)
		free(f->data);

	/* deallocate the language string*/
	if (f->language)
		free(f->language);

	free(f);
}


void alfont_textout_aa(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color) {
	alfont_textout_aa_ex(bmp, f, s, x, y, color, alfont_textmode);
}

//#define APPLY_FONT_KERNING

void alfont_textout_aa_ex(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color, int backg) {
	char *lpszW;
	char *lpszW_tmp;
	int x_tmp;
	int max_advancex = 0;
	char *lpszW_pointer = NULL; //used for freeing string
	char *s_pointer = NULL; //used for original string fixed by autofix
	char *s_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int character;
	int alpha_table[256];
	int first_x = 0, final_x = 0, final_y = 0;
	int curr_uformat = 0;
	int first_flag = TRUE; //First Char flag
	BITMAP *masked_bmp = nullptr; //the masked bmp used by Font hollow

#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (s == NULL) {
		return;
	}

	nLen = strlen(s) + 1;
	s_pointer = (char *)malloc(nLen * sizeof(char));
	memset(s_pointer, 0, nLen);
	strcpy(s_pointer, s);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the s string
	//If find the cutted character, store it from the converted s string and remove it from the original s string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(s) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the s string
				if (f->precedingchar != 0) {
					free(s_pointer);
					fromlen = strlen(s) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					s_pointer = (char *)malloc(tolen * sizeof(char));
					memset(s_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(s_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(s_pointer, s);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = s;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				s_pointer_temp = s_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*s_pointer_temp != '\0') {
						f->precedingchar = *s_pointer_temp;
						s_pointer_temp++;
					}
					//remove the final character
					s_pointer_temp--;
					*s_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(s_pointer) * 5 + 1;
#else
			nLen = strlen(s_pointer) + 1;
#endif

			//add the previous character to the s string
			if (f->precedingchar != 0) {
				free(s_pointer);
				nLen = strlen(s) + 1 + 1;
				s_pointer = (char *)malloc(nLen * sizeof(char));
				memset(s_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				snprintf(precedingchar_pointer, 2 * sizeof(char), "%c", f->precedingchar);
				strcpy(s_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(s_pointer, s);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, s_pointer, nLen);

			s_pointer_temp = s_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*s_pointer_temp != '\0') {
					f->precedingchar = *s_pointer_temp;
					s_pointer_temp++;
				}
				//remove the final character
				s_pointer_temp--;
				*s_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	//Font Code Convert

	if (f->type == 1) {
#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			lpszW_pointer = lpszW;
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)s_pointer) + 1;
		lpszW = (char *)malloc(nLen * sizeof(char));
		memset(lpszW, 0, nLen);
		lpszW_pointer = lpszW;
		wcstombs(lpszW, (const wchar_t *)s_pointer, nLen);
#endif
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open("UTF-16LE", f->language)) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			lpszW_pointer = lpszW;
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			} else {
				set_uformat(U_UNICODE);
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);

#ifdef ALFONT_LINUX
		nLen = strlen(s_pointer) * 5 + 1;
#else
		nLen = strlen(s_pointer) + 1;
#endif

		lpszW = (char *)malloc(nLen * sizeof(wchar_t));
		memset(lpszW, 0, nLen);
		lpszW_pointer = lpszW;
		mbstowcs((wchar_t *)lpszW, s_pointer, nLen);
#endif
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
		nLen = ustrlen(s_pointer) + 1;
#endif
		lpszW = (char *)s_pointer;
	}

	/* is it under or over or too far to the right of the clipping rect then
	   we can assume the string is clipped */
	if ((y + f->face_h < bmp->ct) || (y > bmp->cb) || (x > bmp->cr)) {
		if(s_pointer) free(s_pointer);
		s_pointer = NULL;
		return;
	}

	//build transparency
	if (f->transparency != 255) {
		if (bitmap_color_depth(bmp) > 8) {
			drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
			set_preservedalpha_trans_blender(0, 0, 0, f->transparency);
		}
	} else {
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	}

	/* if we are doing opaque mode, draw a rect and init our table*/
	if (backg >= 0) {
		int i, r, g, b, br, bg, bb, ir, ig, ib;
		int blendr, blendg, blendb;

		if (f->background == TRUE) {
			rectfill(bmp, x, y, x + alfont_text_length(f, s_pointer) - 1, y + f->face_h - 1, backg);
		}

		/* get the color rgb */
		r = getr(color); g = getg(color); b = getb(color);

		/* get the background rgb */
		br = getr(backg); bg = getg(backg); bb = getb(backg);

		/* get increments */
		ir = (r == br) ? 0 : (r + 1) - br;
		ig = (g == bg) ? 0 : (g + 1) - bg;
		ib = (b == bb) ? 0 : (b + 1) - bb;

		blendr = br << 8;
		blendg = bg << 8;
		blendb = bb << 8;

		/* blend both values and make our alpha table */
		for (i = 0; i < 256; i++) {
			alpha_table[i] = makecol(blendr >> 8, blendg >> 8, blendb >> 8);
			blendr += ir;
			blendg += ig;
			blendb += ib;
		}
	}

	/* draw char by char (using allegro unicode funcs) */
	acquire_bitmap(bmp);
#ifdef APPLY_FONT_KERNING
	int last_glyph_index = 0;
#endif

	if (f->fixed_width == TRUE) {
		lpszW_tmp = lpszW;
		x_tmp = x;
		max_advancex = 0;

		_alfont_uncache_glyphs(f);

#ifdef ALFONT_LINUX //Fix for Linux Unicode System(be converted)
		for (character = ugetxc(&lpszW_tmp); character != 0; character = ugetxc(&lpszW_tmp), character = ugetxc(&lpszW_tmp)) {
#else
		for (character = ugetxc(&lpszW_tmp); character != 0; character = ugetxc(&lpszW_tmp)) {
#endif
			int glyph_index_tmp;
			struct _ALFONT_CACHED_GLYPH cglyph_tmp;
#ifdef ALFONT_LINUX //Recover for Linux Unicode System Fixed
			if (f->type != 2) {
				lpszW_tmp--;
			}
#endif

			/* if left side of char farther than right side of clipping, we are done */
			if (x_tmp > bmp->cr)
				break;

			/* get the character out of the font */
			if (f->face->charmap)
				glyph_index_tmp = Get_Char_Index(f->face, character);
			else
				glyph_index_tmp = character;

			/* if out of existing glyph range -- skip it */
			if ((glyph_index_tmp < 0) || (glyph_index_tmp >= f->face->num_glyphs))
				continue;

			/* cache the glyph */
			_alfont_cache_glyph(f, glyph_index_tmp);
			cglyph_tmp = f->cached_glyphs[glyph_index_tmp];
			if (max_advancex < f->cached_glyphs[glyph_index_tmp].advancex)
				max_advancex = f->cached_glyphs[glyph_index_tmp].advancex;

			/* advance */
			if (cglyph_tmp.advancex)
				x_tmp += cglyph_tmp.advancex + f->ch_spacing;

		}
		}


#ifdef ALFONT_LINUX //Fix for Linux Unicode System(be converted)
	for (character = ugetxc(&lpszW); character != 0; character = ugetxc(&lpszW), character = ugetxc(&lpszW)) {
#else
	for (character = ugetxc(&lpszW); character != 0; character = ugetxc(&lpszW)) {
#endif
		int real_x, real_y, glyph_index;
		struct _ALFONT_CACHED_GLYPH cglyph;

#ifdef ALFONT_LINUX //Recover for Linux Unicode System Fixed
		if (f->type != 2) {
			lpszW--;
		}
#endif

		/* if left side of char farther than right side of clipping, we are done */
		if (x > bmp->cr)
			break;

		/* get the character out of the font */
		if (f->face->charmap)
			glyph_index = Get_Char_Index(f->face, character);
		else
			glyph_index = character;

		/* if out of existing glyph range -- skip it */
		if ((glyph_index < 0) || (glyph_index >= f->face->num_glyphs))
			continue;

		/* cache the glyph */
		_alfont_cache_glyph(f, glyph_index);
		if (f->fixed_width == TRUE)
			f->cached_glyphs[glyph_index].advancex = max_advancex;

		cglyph = f->cached_glyphs[glyph_index];

		/* calculate drawing coords */
		real_x = x + cglyph.aaleft;
		real_y = (y - cglyph.aatop) + f->face_ascender;

		/* apply kerning */
#ifdef APPLY_FONT_KERNING
		if (last_glyph_index) {
			FT_Vector v;
			Get_Kerning(f->face, last_glyph_index, glyph_index, ft_kerning_default, &v);
			real_x += v.x >> 6;
			real_y += v.y >> 6;
		}
		last_glyph_index = glyph_index;
#endif

		/* draw only if exists */
		if ((cglyph.aa_available) && (cglyph.aabmp)) {

			int bmp_x, bmp_y, outline_w;
			unsigned char *bmp_p = cglyph.aabmp;
			const int max_bmp_x = cglyph.aawidth + real_x;
			const int max_bmp_y = cglyph.aaheight + real_y;

			if (first_flag == TRUE) {
				first_x = max_bmp_x;
			}

			/* if in opaque mode */
			if (backg >= 0) {
				if (f->outline_hollow == TRUE) { //Set masked region
					//cancel transparency
					if (f->transparency != 255) {
						if (bitmap_color_depth(bmp) > 8) {
							drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
						}
					}
					if (f->style == 1) {
						masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + ((max_bmp_y - real_y) / 2) - real_x, max_bmp_y - real_y);
						clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					} else if (f->style == 2) {
						masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + 1 - real_x, max_bmp_y - real_y);
						clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					} else if (f->style == 3) {
						masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + ((max_bmp_y - real_y) / 2) + 1 - real_x, max_bmp_y - real_y);
						clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					} else {
						masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x - real_x, max_bmp_y - real_y);
						clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					}
					for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
						for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
							if (*bmp_p++) {
								if (first_x > bmp_x) first_x = bmp_x;
								if (final_x < bmp_x) final_x = bmp_x;
								if (final_y < bmp_y) final_y = bmp_y;
								if (f->style == 1) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y));
								} else if (f->style == 2) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + 1)) final_x = bmp_x + 1;
										if (final_x < (bmp_x)) final_x = bmp_x;
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + 1)) first_x = bmp_x + 1;
										if (first_x > (bmp_x)) first_x = bmp_x;
									}
									putpixel(masked_bmp, bmp_x + 1 - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + 1, bmp_y));
									putpixel(masked_bmp, bmp_x - real_x, bmp_y - real_y, getpixel(bmp, bmp_x, bmp_y));
								} else if (f->style == 3) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1 - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y));
									putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y));
								} else {
									putpixel(masked_bmp, bmp_x - real_x, bmp_y - real_y, getpixel(bmp, bmp_x, bmp_y));
								}
							}
						}
					}
					//restore transparency
					if (f->transparency != 255) {
						if (bitmap_color_depth(bmp) > 8) {
							drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
						}
					}
					bmp_p = cglyph.aabmp; //repointer to Font bmp pointer and draw outline
				}

				if ((f->outline_top > 0) || (f->outline_bottom > 0) || (f->outline_left > 0) || (f->outline_right > 0)) {
					for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
						for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
							const int alpha = *bmp_p++;
							if (alpha) {
								if (f->outline_right > 0) {
									for (outline_w = 0; outline_w < f->outline_right; outline_w++) {
										if (bmp_x + (outline_w + 1) <= max_bmp_x) {
											if (f->style == 1) {
												if (f->underline_right == TRUE) {
													if (final_x < (bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2);
												}
												putpixel(bmp, bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2), bmp_y, f->outline_color);
											} else if (f->style == 2) {
												if (f->underline_right == TRUE) {
													if (final_x < (bmp_x + (outline_w + 1) + 1)) final_x = bmp_x + (outline_w + 1) + 1;
												}
												putpixel(bmp, bmp_x + (outline_w + 1) + 1, bmp_y, f->outline_color);
											} else if (f->style == 3) {
												if (f->underline_right == TRUE) {
													if (final_x < (bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1;
												}
												putpixel(bmp, bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y, f->outline_color);
											} else {
												if (f->underline_right == TRUE) {
													if (final_x < (bmp_x + (outline_w + 1))) final_x = bmp_x + (outline_w + 1);
												}
												putpixel(bmp, bmp_x + (outline_w + 1), bmp_y, f->outline_color);
											}
										}
									}
								}
								if (f->outline_bottom > 0) {
									for (outline_w = 0; outline_w < f->outline_bottom; outline_w++) {
										if (bmp_y + (outline_w + 1) <= max_bmp_y) {
											if (f->style == 1) {
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y + (outline_w + 1), f->outline_color);
											} else if (f->style == 2) {
												putpixel(bmp, bmp_x + 1, bmp_y + (outline_w + 1), f->outline_color);
												putpixel(bmp, bmp_x, bmp_y + (outline_w + 1), f->outline_color);
											} else if (f->style == 3) {
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y + (outline_w + 1), f->outline_color);
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y + (outline_w + 1), f->outline_color);
											} else {
												putpixel(bmp, bmp_x, bmp_y + (outline_w + 1), f->outline_color);
											}
										}
									}
								}
								if (f->outline_left > 0) {
									for (outline_w = 0; outline_w < f->outline_left; outline_w++) {
										if (bmp_x - (outline_w + 1) >= real_x - 1) {
											if (f->style == 1) {
												if (f->underline_left == TRUE) {
													if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1);
												}
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1), bmp_y, f->outline_color);
											} else if (f->style == 2) {
												if (f->underline_left == TRUE) {
													if (first_x > (bmp_x - (outline_w + 1))) first_x = bmp_x - (outline_w + 1);
												}
												putpixel(bmp, bmp_x - (outline_w + 1), bmp_y, f->outline_color);
											} else if (f->style == 3) {
												if (f->underline_left == TRUE) {
													if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1);
												}
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1), bmp_y, f->outline_color);
											} else {
												if (f->underline_left == TRUE) {
													if (first_x > (bmp_x - (outline_w + 1))) first_x = bmp_x - (outline_w + 1);
												}
												putpixel(bmp, bmp_x - (outline_w + 1), bmp_y, f->outline_color);
											}
										}
									}
								}
								if (f->outline_top > 0) {
									for (outline_w = 0; outline_w < f->outline_top; outline_w++) {
										if (bmp_y - (outline_w + 1) >= real_y - 1) {
											if (f->style == 1) {
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y - (outline_w + 1), f->outline_color);
											} else if (f->style == 2) {
												putpixel(bmp, bmp_x + 1, bmp_y - (outline_w + 1), f->outline_color);
												putpixel(bmp, bmp_x, bmp_y - (outline_w + 1), f->outline_color);
											} else if (f->style == 3) {
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y - (outline_w + 1), f->outline_color);
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y - (outline_w + 1), f->outline_color);
											} else {
												putpixel(bmp, bmp_x, bmp_y - (outline_w + 1), f->outline_color);
											}
										}
									}
								}
							}
						}
					}
				}
				if (f->outline_hollow == FALSE) { //check if it will restore the masked region
					bmp_p = cglyph.aabmp; //repointer to Font bmp pointer
					for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
						for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
							const int alpha = *bmp_p++;

							if (alpha) {
								if (first_x > bmp_x) first_x = bmp_x;
								if (final_x < bmp_x) final_x = bmp_x;
								if (final_y < bmp_y) final_y = bmp_y;
								if (f->style == 1) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y, alpha_table[alpha]);
								} else if (f->style == 2) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + 1)) final_x = bmp_x + 1;
										if (final_x < (bmp_x)) final_x = bmp_x;
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + 1)) first_x = bmp_x + 1;
										if (first_x > (bmp_x)) first_x = bmp_x;
									}
									putpixel(bmp, bmp_x + 1, bmp_y, alpha_table[alpha]);
									putpixel(bmp, bmp_x, bmp_y, alpha_table[alpha]);
								} else if (f->style == 3) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y, alpha_table[alpha]);
									putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y, alpha_table[alpha]);
								} else {
									putpixel(bmp, bmp_x, bmp_y, alpha_table[alpha]);
								}
							}
						}
					}
				} else { //restore original pic
					//cancel transparency
					if (f->transparency != 255) {
						if (bitmap_color_depth(bmp) > 8) {
							drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
						}
					}
					if (f->style == 1) {
						masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + ((max_bmp_y - real_y) / 2) - real_x, max_bmp_y - real_y);
						destroy_bitmap(masked_bmp);
					} else if (f->style == 2) {
						masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + 1 - real_x, max_bmp_y - real_y);
						destroy_bitmap(masked_bmp);
					} else if (f->style == 3) {
						masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + ((max_bmp_y - real_y) / 2) + 1 - real_x, max_bmp_y - real_y);
						destroy_bitmap(masked_bmp);
					} else {
						masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x - real_x, max_bmp_y - real_y);
						destroy_bitmap(masked_bmp);
					}
					//restore transparency
					if (f->transparency != 255) {
						if (bitmap_color_depth(bmp) > 8) {
							drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
						}
					}
				}
			}

			/* if in transparent mode */
			else {
				if (f->outline_hollow == TRUE) { //set the masked region
					//cancel transparency
					if (f->transparency != 255) {
						if (bitmap_color_depth(bmp) > 8) {
							drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
						}
					}
					if (f->style == 1) {
						masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + ((max_bmp_y - real_y) / 2) - real_x, max_bmp_y - real_y);
						clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					} else if (f->style == 2) {
						masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + 1 - real_x, max_bmp_y - real_y);
						clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					} else if (f->style == 3) {
						masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + ((max_bmp_y - real_y) / 2) + 1 - real_x, max_bmp_y - real_y);
						clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					} else {
						masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x - real_x, max_bmp_y - real_y);
						clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					}
					for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
						for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
							if (*bmp_p++) {
								if (first_x > bmp_x) first_x = bmp_x;
								if (final_x < bmp_x) final_x = bmp_x;
								if (final_y < bmp_y) final_y = bmp_y;
								if (f->style == 1) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y));
								} else if (f->style == 2) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + 1)) final_x = bmp_x + 1;
										if (final_x < (bmp_x)) final_x = bmp_x;
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + 1)) first_x = bmp_x + 1;
										if (first_x > (bmp_x)) first_x = bmp_x;
									}
									putpixel(masked_bmp, bmp_x + 1 - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + 1, bmp_y));
									putpixel(masked_bmp, bmp_x - real_x, bmp_y - real_y, getpixel(bmp, bmp_x, bmp_y));
								} else if (f->style == 3) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1 - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y));
									putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y));
								} else {
									putpixel(masked_bmp, bmp_x - real_x, bmp_y - real_y, getpixel(bmp, bmp_x, bmp_y));
								}
							}
						}
					}
					//restore transparency
					if (f->transparency != 255) {
						if (bitmap_color_depth(bmp) > 8) {
							drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
						}
					}
					bmp_p = cglyph.aabmp; //repointer to Font bmp pointer and draw outline
				}

				if ((f->outline_top > 0) || (f->outline_bottom > 0) || (f->outline_left > 0) || (f->outline_right > 0)) {
					for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
						for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
							const int alpha = *bmp_p++;
							if (alpha) {
								if (f->outline_right > 0) {
									for (outline_w = 0; outline_w < f->outline_right; outline_w++) {
										if (bmp_x + (outline_w + 1) <= max_bmp_x) {
											if (f->style == 1) {
												if (f->underline_right == TRUE) {
													if (final_x < (bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2);
												}
												putpixel(bmp, bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2), bmp_y, f->outline_color);
											} else if (f->style == 2) {
												if (f->underline_right == TRUE) {
													if (final_x < (bmp_x + (outline_w + 1) + 1)) final_x = bmp_x + (outline_w + 1) + 1;
												}
												putpixel(bmp, bmp_x + (outline_w + 1) + 1, bmp_y, f->outline_color);
											} else if (f->style == 3) {
												if (f->underline_right == TRUE) {
													if (final_x < (bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1;
												}
												putpixel(bmp, bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y, f->outline_color);
											} else {
												if (f->underline_right == TRUE) {
													if (final_x < (bmp_x + (outline_w + 1))) final_x = bmp_x + (outline_w + 1);
												}
												putpixel(bmp, bmp_x + (outline_w + 1), bmp_y, f->outline_color);
											}
										}
									}
								}
								if (f->outline_bottom > 0) {
									for (outline_w = 0; outline_w < f->outline_bottom; outline_w++) {
										if (bmp_y + (outline_w + 1) <= max_bmp_y) {
											if (f->style == 1) {
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y + (outline_w + 1), f->outline_color);
											} else if (f->style == 2) {
												putpixel(bmp, bmp_x + 1, bmp_y + (outline_w + 1), f->outline_color);
												putpixel(bmp, bmp_x, bmp_y + (outline_w + 1), f->outline_color);
											} else if (f->style == 3) {
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y + (outline_w + 1), f->outline_color);
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y + (outline_w + 1), f->outline_color);
											} else {
												putpixel(bmp, bmp_x, bmp_y + (outline_w + 1), f->outline_color);
											}
										}
									}
								}
								if (f->outline_left > 0) {
									for (outline_w = 0; outline_w < f->outline_left; outline_w++) {
										if (bmp_x - (outline_w + 1) >= real_x - 1) {
											if (f->style == 1) {
												if (f->underline_left == TRUE) {
													if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1);
												}
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1), bmp_y, f->outline_color);
											} else if (f->style == 2) {
												if (f->underline_left == TRUE) {
													if (first_x > (bmp_x - (outline_w + 1))) first_x = bmp_x - (outline_w + 1);
												}
												putpixel(bmp, bmp_x - (outline_w + 1), bmp_y, f->outline_color);
											} else if (f->style == 3) {
												if (f->underline_left == TRUE) {
													if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1);
												}
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1), bmp_y, f->outline_color);
											} else {
												if (f->underline_left == TRUE) {
													if (first_x > (bmp_x - (outline_w + 1))) first_x = bmp_x - (outline_w + 1);
												}
												putpixel(bmp, bmp_x - (outline_w + 1), bmp_y, f->outline_color);
											}
										}
									}
								}
								if (f->outline_top > 0) {
									for (outline_w = 0; outline_w < f->outline_top; outline_w++) {
										if (bmp_y - (outline_w + 1) >= real_y - 1) {
											if (f->style == 1) {
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y - (outline_w + 1), f->outline_color);
											} else if (f->style == 2) {
												putpixel(bmp, bmp_x + 1, bmp_y - (outline_w + 1), f->outline_color);
												putpixel(bmp, bmp_x, bmp_y - (outline_w + 1), f->outline_color);
											} else if (f->style == 3) {
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y - (outline_w + 1), f->outline_color);
												putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y - (outline_w + 1), f->outline_color);
											} else {
												putpixel(bmp, bmp_x, bmp_y - (outline_w + 1), f->outline_color);
											}
										}
									}
								}
							}
						}
					}
				}
				if (f->outline_hollow == FALSE) { //check if it will restore the masked region
					bmp_p = cglyph.aabmp; //repointer to Font bmp pointer
					for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
						for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
							const int alpha = *bmp_p++;
							const int orig_color = color;
							if (alpha) {
								if (alpha >= 255)
									solid_mode();
								else {
									drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
									set_preservedalpha_trans_blender(0, 0, 0, alpha);
									// apply blending
									color = apply_trans_blender(bmp, color, getpixel(bmp, bmp_x, bmp_y), alpha);
								}
								if (first_x > bmp_x) first_x = bmp_x;
								if (final_x < bmp_x) final_x = bmp_x;
								if (final_y < bmp_y) final_y = bmp_y;
								if (f->style == 1) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y, color);
								} else if (f->style == 2) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + 1)) final_x = bmp_x + 1;
										if (final_x < (bmp_x)) final_x = bmp_x;
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + 1)) first_x = bmp_x + 1;
										if (first_x > (bmp_x)) first_x = bmp_x;
									}
									putpixel(bmp, bmp_x + 1, bmp_y, color);
									putpixel(bmp, bmp_x, bmp_y, color);
								} else if (f->style == 3) {
									if (f->underline_right == TRUE) {
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
										if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									if (f->underline_left == TRUE) {
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
										if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
									}
									putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y, color);
									putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y, color);
								} else {
									putpixel(bmp, bmp_x, bmp_y, color);
								}
							}
							if (color != orig_color) // restore original color
								color = orig_color;
						}
					}
				} else { //restore original pic
					//cancel transparency
					if (f->transparency != 255) {
						if (bitmap_color_depth(bmp) > 8) {
							drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
						}
					}
					if (f->style == 1) {
						masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + ((max_bmp_y - real_y) / 2) - real_x, max_bmp_y - real_y);
						destroy_bitmap(masked_bmp);
					} else if (f->style == 2) {
						masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + 1 - real_x, max_bmp_y - real_y);
						destroy_bitmap(masked_bmp);
					} else if (f->style == 3) {
						masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + ((max_bmp_y - real_y) / 2) + 1 - real_x, max_bmp_y - real_y);
						destroy_bitmap(masked_bmp);
					} else {
						masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x - real_x, max_bmp_y - real_y);
						destroy_bitmap(masked_bmp);
					}
					//restore transparency
					if (f->transparency != 255) {
						if (bitmap_color_depth(bmp) > 8) {
							drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
						}
					}
				}
			}
		}

		/* advance */
		if (cglyph.advancex)
			x += cglyph.advancex + f->ch_spacing;
		if (cglyph.advancey)
			y += cglyph.advancey + f->ch_spacing;

		first_flag = FALSE; //set first char flag is FALSE
	}
	//draw underline
	if ((f->underline) == TRUE) {
		if ((final_y + ((f->real_face_h) >> 5)) > 5) {
			if (f->outline_hollow == TRUE) { //set the masked region for underline
				int bmp_x, bmp_y;
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->underline_right == TRUE) { //If the underline_right is TRUE,extend right underline
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), final_x + f->ch_spacing - first_x + 1, final_y + ((f->real_face_h) >> 5) - final_y + 1);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					for (bmp_y = final_y; bmp_y <= final_y + ((f->real_face_h) >> 5); bmp_y++) {
						for (bmp_x = first_x; bmp_x <= final_x + f->ch_spacing; bmp_x++) {
							putpixel(masked_bmp, bmp_x - first_x, bmp_y - final_y, getpixel(bmp, bmp_x, bmp_y));
						}
					}
				} else { //If the underline_right is not TRUE,just draw underline
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), final_x - first_x + 1, final_y + ((f->real_face_h) >> 5) - final_y + 1);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					for (bmp_y = final_y; bmp_y <= final_y + ((f->real_face_h) >> 5); bmp_y++) {
						for (bmp_x = first_x; bmp_x <= final_x; bmp_x++) {
							putpixel(masked_bmp, bmp_x - first_x, bmp_y - final_y, getpixel(bmp, bmp_x, bmp_y));
						}
					}
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
			//Draw outline
			if ((f->outline_top > 0) || (f->outline_bottom > 0) || (f->outline_left > 0) || (f->outline_right > 0)) {
				if (f->outline_top > 0) {
					if (f->underline_right == TRUE) { //If the underline_right is TRUE
						rectfill(bmp, first_x, final_y - 1, final_x + f->ch_spacing, final_y + ((f->real_face_h) >> 5), f->outline_color);
					} else { //If the underline_right is FALSE
						rectfill(bmp, first_x, final_y - 1, final_x, final_y + ((f->real_face_h) >> 5), f->outline_color);
					}
				}
				if (f->outline_bottom > 0) {
					if (f->underline_right == TRUE) { //If the underline_right is TRUE
						rectfill(bmp, first_x, final_y, final_x + f->ch_spacing, final_y + ((f->real_face_h) >> 5) + 1, f->outline_color);
					} else { //If the underline_right is FALSE
						rectfill(bmp, first_x, final_y, final_x, final_y + ((f->real_face_h) >> 5) + 1, f->outline_color);
					}
				}
				if (f->outline_left > 0) {
					if (f->underline_left == FALSE) {
						rectfill(bmp, first_x - 1, final_y, final_x, final_y + ((f->real_face_h) >> 5), f->outline_color);
					}
				}
				if (f->outline_right > 0) {
					if (f->underline_right == FALSE) {
						rectfill(bmp, first_x, final_y, final_x + 1, final_y + ((f->real_face_h) >> 5), f->outline_color);
					}
				}
			}
			if (f->outline_hollow == FALSE) { //check if it is the masked region for outline
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					rectfill(bmp, first_x, final_y, final_x + f->ch_spacing, final_y + ((f->real_face_h) >> 5), color);
				} else { //If the underline_right is FALSE
					rectfill(bmp, first_x, final_y, final_x, final_y + ((f->real_face_h) >> 5), color);
				}
			} else {
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					masked_blit(masked_bmp, bmp, 0, 0, first_x, final_y, final_x + f->ch_spacing - first_x + 1, final_y + ((f->real_face_h) >> 5) - final_y + 1);
					destroy_bitmap(masked_bmp);
				} else { //If the underline_right is FALSE
					masked_blit(masked_bmp, bmp, 0, 0, first_x, final_y, final_x - first_x + 1, final_y + ((f->real_face_h) >> 5) - final_y + 1);
					destroy_bitmap(masked_bmp);
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
		} else {
			if (f->outline_hollow == TRUE) { //set the masked region for underline
				int bmp_x, bmp_y;
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), final_x + f->ch_spacing - first_x + 1, final_y + 5 - final_y + 1);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					for (bmp_y = final_y; bmp_y <= final_y + 5; bmp_y++) {
						for (bmp_x = first_x; bmp_x <= final_x + f->ch_spacing; bmp_x++) {
							putpixel(masked_bmp, bmp_x - first_x, bmp_y - final_y, getpixel(bmp, bmp_x, bmp_y));
						}
					}
				} else { //If the underline_right is FALSE
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), final_x - first_x + 1, final_y + 5 - final_y + 1);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					for (bmp_y = final_y; bmp_y <= final_y + 5; bmp_y++) {
						for (bmp_x = first_x; bmp_x <= final_x; bmp_x++) {
							putpixel(masked_bmp, bmp_x - first_x, bmp_y - final_y, getpixel(bmp, bmp_x, bmp_y));
						}
					}
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
			//Draw outline
			if ((f->outline_top > 0) || (f->outline_bottom > 0) || (f->outline_left > 0) || (f->outline_right > 0)) {
				if (f->outline_top > 0) {
					if (f->underline_right == TRUE) { //If the underline_right is TRUE
						rectfill(bmp, first_x, final_y - 1, final_x + f->ch_spacing, final_y + 5, f->outline_color);
					} else { //If the underline_right is FALSE
						rectfill(bmp, first_x, final_y - 1, final_x, final_y + 5, f->outline_color);
					}
				}
				if (f->outline_bottom > 0) {
					if (f->underline_right == TRUE) { //If the underline_right is TRUE
						rectfill(bmp, first_x, final_y, final_x + f->ch_spacing, final_y + 5, f->outline_color);
					} else { //If the underline_right is FALSE
						rectfill(bmp, first_x, final_y, final_x, final_y + 5, f->outline_color);
					}
				}
				if (f->outline_left > 0) {
					if (f->underline_left == FALSE) {
						rectfill(bmp, first_x - 1, final_y, final_x, final_y + 5, f->outline_color);
					}
				}
				if (f->outline_right > 0) {
					if (f->underline_right == FALSE) {
						rectfill(bmp, first_x, final_y, final_x + 1, final_y + 5, f->outline_color);
					}
				}
			}
			if (f->outline_hollow == FALSE) { //check if it is the masked region for outline
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					rectfill(bmp, first_x, final_y, final_x + f->ch_spacing, final_y + 5, color);
				} else { //If the underline_right is FALSE
					rectfill(bmp, first_x, final_y, final_x, final_y + 5, color);
				}
			} else {
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					masked_blit(masked_bmp, bmp, 0, 0, first_x, final_y, final_x + f->ch_spacing - first_x + 1, final_y + 5 - final_y + 1);
					destroy_bitmap(masked_bmp);
				} else { //If the underline_right is FALSE
					masked_blit(masked_bmp, bmp, 0, 0, first_x, final_y, final_x - first_x + 1, final_y + 5 - final_y + 1);
					destroy_bitmap(masked_bmp);
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
		}
	}

	release_bitmap(bmp);

	/* reset blender */
	if (backg < 0)
		solid_mode();

	if ((f->type == 1) || (f->type == 2)) {
		if (lpszW_pointer)
			free(lpszW_pointer);
	}

	if (s_pointer) {
		free(s_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	}


void alfont_textout(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color) {
	alfont_textout_ex(bmp, f, s, x, y, color, alfont_textmode);
}


void alfont_textout_ex(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color, int backg) {
	char *lpszW;
	char *lpszW_tmp;
	int x_tmp;
	int max_advancex = 0;
	char *lpszW_pointer = NULL; //used for freeing string
	char *s_pointer = NULL; //used for original string fixed by autofix
	char *s_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int character;
	int first_x = 0, final_x = 0, final_y = 0;
	int curr_uformat = 0;
	int first_flag = TRUE; //First Char flag
	BITMAP *masked_bmp = nullptr; //the masked bmp used by Font hollow
#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (s == NULL) {
		return;
	}

	nLen = strlen(s) + 1;
	s_pointer = (char *)malloc(nLen * sizeof(char));
	memset(s_pointer, 0, nLen);
	strcpy(s_pointer, s);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the s string
	//If find the cutted character, store it from the converted s string and remove it from the original s string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(s) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the s string
				if (f->precedingchar != 0) {
					free(s_pointer);
					fromlen = strlen(s) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					s_pointer = (char *)malloc(tolen * sizeof(char));
					memset(s_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(s_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(s_pointer, s);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = s;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				s_pointer_temp = s_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*s_pointer_temp != '\0') {
						f->precedingchar = *s_pointer_temp;
						s_pointer_temp++;
					}
					//remove the final character
					s_pointer_temp--;
					*s_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(s_pointer) * 5 + 1;
#else
			nLen = strlen(s_pointer) + 1;
#endif

			//add the previous character to the s string
			if (f->precedingchar != 0) {
				free(s_pointer);
				nLen = strlen(s) + 1 + 1;
				s_pointer = (char *)malloc(nLen * sizeof(char));
				memset(s_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				snprintf(precedingchar_pointer, 2 * sizeof(char), "%c", f->precedingchar);
				strcpy(s_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(s_pointer, s);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, s_pointer, nLen);

			s_pointer_temp = s_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*s_pointer_temp != '\0') {
					f->precedingchar = *s_pointer_temp;
					s_pointer_temp++;
				}
				//remove the final character
				s_pointer_temp--;
				*s_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	//Font Code Convert

	if (f->type == 1) {

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			lpszW_pointer = lpszW;
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)s_pointer) + 1;
		lpszW = (char *)malloc(nLen * sizeof(char));
		memset(lpszW, 0, nLen);
		lpszW_pointer = lpszW;
		wcstombs(lpszW, (const wchar_t *)s_pointer, nLen);
#endif
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open("UTF-16LE", f->language)) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			lpszW_pointer = lpszW;
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			} else {
				set_uformat(U_UNICODE);
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);

#ifdef ALFONT_LINUX
		nLen = strlen(s_pointer) * 5 + 1;
#else
		nLen = strlen(s_pointer) + 1;
#endif

		lpszW = (char *)malloc(nLen * sizeof(wchar_t));
		memset(lpszW, 0, nLen);
		lpszW_pointer = lpszW;
		mbstowcs((wchar_t *)lpszW, s_pointer, nLen);
#endif
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
		nLen = ustrlen(s_pointer) + 1;
#endif
		lpszW = (char *)s_pointer;
	}

	/* is it under or over or too far to the right of the clipping rect then
	   we can assume the string is clipped */
	if ((y + f->face_h < bmp->ct) || (y > bmp->cb) || (x > bmp->cr)) {
		if(s_pointer) free(s_pointer);
		s_pointer = NULL;
		return;
	}

	//build transparency
	if (f->transparency != 255) {
		if (bitmap_color_depth(bmp) > 8) {
			drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
			set_preservedalpha_trans_blender(0, 0, 0, f->transparency);
		}
	} else {
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	}

	/* if we are doing opaque mode, draw a rect */
	if (backg >= 0) {
		if (f->background == TRUE) {
			rectfill(bmp, x, y, x + alfont_text_length(f, s_pointer) - 1, y + f->face_h - 1, backg);
		}
	}

	/* draw char by char (using allegro unicode funcs) */
	acquire_bitmap(bmp);
#ifdef APPLY_FONT_KERNING
	int last_glyph_index = 0;
#endif

	if (f->fixed_width == TRUE) {
		lpszW_tmp = lpszW;
		x_tmp = x;
		max_advancex = 0;

		_alfont_uncache_glyphs(f);

#ifdef ALFONT_LINUX //Fix for Linux Unicode System(be converted)
		for (character = ugetxc(&lpszW_tmp); character != 0; character = ugetxc(&lpszW_tmp), character = ugetxc(&lpszW_tmp)) {
#else
		for (character = ugetxc(&lpszW_tmp); character != 0; character = ugetxc(&lpszW_tmp)) {
#endif
			int glyph_index_tmp;
			struct _ALFONT_CACHED_GLYPH cglyph_tmp;
#ifdef ALFONT_LINUX //Recover for Linux Unicode System Fixed
			if (f->type != 2) {
				lpszW_tmp--;
			}
#endif

			/* if left side of char farther than right side of clipping, we are done */
			if (x_tmp > bmp->cr)
				break;

			/* get the character out of the font */
			if (f->face->charmap)
				glyph_index_tmp = Get_Char_Index(f->face, character);
			else
				glyph_index_tmp = character;

			/* if out of existing glyph range -- skip it */
			if ((glyph_index_tmp < 0) || (glyph_index_tmp >= f->face->num_glyphs))
				continue;

			/* cache the glyph */
			_alfont_cache_glyph(f, glyph_index_tmp);
			cglyph_tmp = f->cached_glyphs[glyph_index_tmp];
			if (max_advancex < f->cached_glyphs[glyph_index_tmp].advancex)
				max_advancex = f->cached_glyphs[glyph_index_tmp].advancex;

			/* advance */
			if (cglyph_tmp.advancex)
				x_tmp += cglyph_tmp.advancex + f->ch_spacing;

		}
		}

#ifdef ALFONT_LINUX //Fix for Linux Unicode System(be converted)
	for (character = ugetxc(&lpszW); character != 0; character = ugetxc(&lpszW), character = ugetxc(&lpszW)) {
#else
	for (character = ugetxc(&lpszW); character != 0; character = ugetxc(&lpszW)) {
#endif
		int real_x, real_y, glyph_index;
		struct _ALFONT_CACHED_GLYPH cglyph;

#ifdef ALFONT_LINUX //Recover for Linux Unicode System Fixed
		if (f->type != 2) {
			lpszW--;
		}
#endif

		/* if left side of char farther than right side of clipping, we are done */
		if (x > bmp->cr)
			break;

		/* get the character out of the font */
		if (f->face->charmap)
			glyph_index = Get_Char_Index(f->face, character);
		else
			glyph_index = character;

		/* if out of existing glyph range -- skip it */
		if ((glyph_index < 0) || (glyph_index >= f->face->num_glyphs))
			continue;

		/* cache the glyph */
		_alfont_cache_glyph(f, glyph_index);
		if (f->fixed_width == TRUE)
			f->cached_glyphs[glyph_index].advancex = max_advancex;

		cglyph = f->cached_glyphs[glyph_index];

		/* calculate drawing coords */
		real_x = x + cglyph.left;
		real_y = (y - cglyph.top) + f->face_ascender;

		/* apply kerning */
#ifdef APPLY_FONT_KERNING
		if (last_glyph_index) {
			FT_Vector v;
			Get_Kerning(f->face, last_glyph_index, glyph_index, ft_kerning_default, &v);
			real_x += v.x >> 6;
			real_y += v.y >> 6;
		}
		last_glyph_index = glyph_index;
#endif

		/* draw only if exists */
		if ((cglyph.mono_available) && (cglyph.bmp)) {
			unsigned char *bmp_p = cglyph.bmp;
			/* monochrome drawing */
			int bmp_x, bmp_y, outline_w;

			/* copy the character bitmap to our allegro one */
			const int max_bmp_x = cglyph.width + real_x;
			const int max_bmp_y = cglyph.height + real_y;

			if (first_flag == TRUE) {
				first_x = max_bmp_x;
			}

			if (f->outline_hollow == TRUE) { //Set masked region
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->style == 1) {
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + ((max_bmp_y - real_y) / 2) - real_x, max_bmp_y - real_y);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
				} else if (f->style == 2) {
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + 1 - real_x, max_bmp_y - real_y);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
				} else if (f->style == 3) {
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x + ((max_bmp_y - real_y) / 2) + 1 - real_x, max_bmp_y - real_y);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
				} else {
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), max_bmp_x - real_x, max_bmp_y - real_y);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
				}
				for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
					for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
						if (*bmp_p++) {
							if (first_x > bmp_x) first_x = bmp_x;
							if (final_x < bmp_x) final_x = bmp_x;
							if (final_y < bmp_y) final_y = bmp_y;
							if (f->style == 1) {
								if (f->underline_right == TRUE) {
									if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
								}
								if (f->underline_left == TRUE) {
									if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
								}
								putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y));
							} else if (f->style == 2) {
								if (f->underline_right == TRUE) {
									if (final_x < (bmp_x + 1)) final_x = bmp_x + 1;
									if (final_x < (bmp_x)) final_x = bmp_x;
								}
								if (f->underline_left == TRUE) {
									if (first_x > (bmp_x + 1)) first_x = bmp_x + 1;
									if (first_x > (bmp_x)) first_x = bmp_x;
								}
								putpixel(masked_bmp, bmp_x + 1 - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + 1, bmp_y));
								putpixel(masked_bmp, bmp_x - real_x, bmp_y - real_y, getpixel(bmp, bmp_x, bmp_y));
							} else if (f->style == 3) {
								if (f->underline_right == TRUE) {
									if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
									if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
								}
								if (f->underline_left == TRUE) {
									if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
									if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
								}
								putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1 - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y));
								putpixel(masked_bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - real_x, bmp_y - real_y, getpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y));
							} else {
								putpixel(masked_bmp, bmp_x - real_x, bmp_y - real_y, getpixel(bmp, bmp_x, bmp_y));
							}
						}
					}
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
				bmp_p = cglyph.bmp; //repointer to Font bmp pointer and draw outline
			}

			if ((f->outline_top > 0) || (f->outline_bottom > 0) || (f->outline_left > 0) || (f->outline_right > 0)) {
				for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
					for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
						if (*bmp_p++) {
							if (f->outline_right > 0) {
								for (outline_w = 0; outline_w < f->outline_right; outline_w++) {
									if (bmp_x + (outline_w + 1) <= max_bmp_x) {
										if (f->style == 1) {
											if (f->underline_right == TRUE) {
												if (final_x < (bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2);
											}
											putpixel(bmp, bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2), bmp_y, f->outline_color);
										} else if (f->style == 2) {
											if (f->underline_right == TRUE) {
												if (final_x < (bmp_x + (outline_w + 1) + 1)) final_x = bmp_x + (outline_w + 1) + 1;
											}
											putpixel(bmp, bmp_x + (outline_w + 1) + 1, bmp_y, f->outline_color);
										} else if (f->style == 3) {
											if (f->underline_right == TRUE) {
												if (final_x < (bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1;
											}
											putpixel(bmp, bmp_x + (outline_w + 1) + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y, f->outline_color);
										} else {
											if (f->underline_right == TRUE) {
												if (final_x < (bmp_x + (outline_w + 1))) final_x = bmp_x + (outline_w + 1);
											}
											putpixel(bmp, bmp_x + (outline_w + 1), bmp_y, f->outline_color);
										}
									}
								}
							}
							if (f->outline_bottom > 0) {
								for (outline_w = 0; outline_w < f->outline_bottom; outline_w++) {
									if (bmp_y + (outline_w + 1) <= max_bmp_y) {
										if (f->style == 1) {
											putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y + (outline_w + 1), f->outline_color);
										} else if (f->style == 2) {
											putpixel(bmp, bmp_x + 1, bmp_y + (outline_w + 1), f->outline_color);
											putpixel(bmp, bmp_x, bmp_y + (outline_w + 1), f->outline_color);
										} else if (f->style == 3) {

											putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y + (outline_w + 1), f->outline_color);
											putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y + (outline_w + 1), f->outline_color);
										} else {
											putpixel(bmp, bmp_x, bmp_y + (outline_w + 1), f->outline_color);
										}
									}
								}
							}
							if (f->outline_left > 0) {
								for (outline_w = 0; outline_w < f->outline_left; outline_w++) {
									if (bmp_x - (outline_w + 1) >= real_x - 1) {
										if (f->style == 1) {
											if (f->underline_left == TRUE) {
												if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1);
											}
											putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1), bmp_y, f->outline_color);
										} else if (f->style == 2) {
											if (f->underline_left == TRUE) {
												if (first_x > (bmp_x - (outline_w + 1))) first_x = bmp_x - (outline_w + 1);
											}
											putpixel(bmp, bmp_x - (outline_w + 1), bmp_y, f->outline_color);
										} else if (f->style == 3) {
											if (f->underline_left == TRUE) {
												if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1);
											}
											putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) - (outline_w + 1), bmp_y, f->outline_color);
										} else {
											if (f->underline_left == TRUE) {
												if (first_x > (bmp_x - (outline_w + 1))) first_x = bmp_x - (outline_w + 1);
											}
											putpixel(bmp, bmp_x - (outline_w + 1), bmp_y, f->outline_color);
										}
									}
								}
							}
							if (f->outline_top > 0) {
								for (outline_w = 0; outline_w < f->outline_top; outline_w++) {
									if (bmp_y - (outline_w + 1) >= real_y - 1) {
										if (f->style == 1) {
											putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y - (outline_w + 1), f->outline_color);
										} else if (f->style == 2) {
											putpixel(bmp, bmp_x + 1, bmp_y - (outline_w + 1), f->outline_color);
											putpixel(bmp, bmp_x, bmp_y - (outline_w + 1), f->outline_color);
										} else if (f->style == 3) {
											putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y - (outline_w + 1), f->outline_color);
											putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y - (outline_w + 1), f->outline_color);
										} else {
											putpixel(bmp, bmp_x, bmp_y - (outline_w + 1), f->outline_color);
										}
									}
								}
							}
						}
					}
				}
			}
			if (f->outline_hollow == FALSE) { //check if it will restore the masked region
				bmp_p = cglyph.bmp; //repointer to Font bmp pointer
				for (bmp_y = real_y; bmp_y < max_bmp_y; bmp_y++) {
					for (bmp_x = real_x; bmp_x < max_bmp_x; bmp_x++) {
						if (*bmp_p++) {
							if (first_x > bmp_x) first_x = bmp_x;
							if (final_x < bmp_x) final_x = bmp_x;
							if (final_y < bmp_y) final_y = bmp_y;
							if (f->style == 1) {
								if (f->underline_right == TRUE) {
									if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
								}
								if (f->underline_left == TRUE) {
									if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
								}
								putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y, color);
							} else if (f->style == 2) {
								if (f->underline_right == TRUE) {
									if (final_x < (bmp_x + 1)) final_x = bmp_x + 1;
									if (final_x < (bmp_x)) final_x = bmp_x;
								}
								if (f->underline_left == TRUE) {
									if (first_x > (bmp_x + 1)) first_x = bmp_x + 1;
									if (first_x > (bmp_x)) first_x = bmp_x;
								}
								putpixel(bmp, bmp_x + 1, bmp_y, color);
								putpixel(bmp, bmp_x, bmp_y, color);
							} else if (f->style == 3) {
								if (f->underline_right == TRUE) {
									if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
									if (final_x < (bmp_x + ((max_bmp_y - bmp_y) / 2))) final_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
								}
								if (f->underline_left == TRUE) {
									if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2) + 1)) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2) + 1;
									if (first_x > (bmp_x + ((max_bmp_y - bmp_y) / 2))) first_x = bmp_x + ((max_bmp_y - bmp_y) / 2);
								}
								putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2) + 1, bmp_y, color);
								putpixel(bmp, bmp_x + ((max_bmp_y - bmp_y) / 2), bmp_y, color);
							} else {
								putpixel(bmp, bmp_x, bmp_y, color);
							}
						}
					}
				}
			} else { //restore original pic
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->style == 1) {
					masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + ((max_bmp_y - real_y) / 2) - real_x, max_bmp_y - real_y);
					destroy_bitmap(masked_bmp);
				} else if (f->style == 2) {
					masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + 1 - real_x, max_bmp_y - real_y);
					destroy_bitmap(masked_bmp);
				} else if (f->style == 3) {
					masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x + ((max_bmp_y - real_y) / 2) + 1 - real_x, max_bmp_y - real_y);
					destroy_bitmap(masked_bmp);
				} else {
					masked_blit(masked_bmp, bmp, 0, 0, real_x, real_y, max_bmp_x - real_x, max_bmp_y - real_y);
					destroy_bitmap(masked_bmp);
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
		}

		/* advance */
		if (cglyph.advancex)
			x += cglyph.advancex + f->ch_spacing;
		if (cglyph.advancey)
			y += cglyph.advancey + f->ch_spacing;

		first_flag = FALSE; //set first char flag is FALSE
	}

	//draw underline
	if ((f->underline) == TRUE) {
		if ((final_y + ((f->real_face_h) >> 5)) > 5) {
			if (f->outline_hollow == TRUE) { //set the masked region for underline
				int bmp_x, bmp_y;
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), final_x + f->ch_spacing - first_x + 1, final_y + ((f->real_face_h) >> 5) - final_y + 1);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					for (bmp_y = final_y; bmp_y <= final_y + ((f->real_face_h) >> 5); bmp_y++) {
						for (bmp_x = first_x; bmp_x <= final_x + f->ch_spacing; bmp_x++) {
							putpixel(masked_bmp, bmp_x - first_x, bmp_y - final_y, getpixel(bmp, bmp_x, bmp_y));
						}
					}
				} else { //If the underline_right is FALSE
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), final_x - first_x + 1, final_y + ((f->real_face_h) >> 5) - final_y + 1);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					for (bmp_y = final_y; bmp_y <= final_y + ((f->real_face_h) >> 5); bmp_y++) {
						for (bmp_x = first_x; bmp_x <= final_x; bmp_x++) {
							putpixel(masked_bmp, bmp_x - first_x, bmp_y - final_y, getpixel(bmp, bmp_x, bmp_y));
						}
					}
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
			//Draw outline
			if ((f->outline_top > 0) || (f->outline_bottom > 0) || (f->outline_left > 0) || (f->outline_right > 0)) {
				if (f->outline_top > 0) {
					if (f->underline_right == TRUE) { //If the underline_right is TRUE
						rectfill(bmp, first_x, final_y - 1, final_x + f->ch_spacing, final_y + ((f->real_face_h) >> 5), f->outline_color);
					} else { //If the underline_right is FALSE
						rectfill(bmp, first_x, final_y - 1, final_x, final_y + ((f->real_face_h) >> 5), f->outline_color);
					}
				}
				if (f->outline_bottom > 0) {
					if (f->underline_right == TRUE) { //If the underline_right is TRUE
						rectfill(bmp, first_x, final_y, final_x + f->ch_spacing, final_y + ((f->real_face_h) >> 5) + 1, f->outline_color);
					} else { //If the underline_right is FALSE
						rectfill(bmp, first_x, final_y, final_x, final_y + ((f->real_face_h) >> 5) + 1, f->outline_color);
					}
				}
				if (f->outline_left > 0) {
					if (f->underline_left == FALSE) {
						rectfill(bmp, first_x - 1, final_y, final_x, final_y + ((f->real_face_h) >> 5), f->outline_color);
					}
				}
				if (f->outline_right > 0) {
					if (f->underline_right == FALSE) {
						rectfill(bmp, first_x, final_y, final_x + 1, final_y + ((f->real_face_h) >> 5), f->outline_color);
					}
				}
			}
			if (f->outline_hollow == FALSE) { //check if it is the masked region for outline
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					rectfill(bmp, first_x, final_y, final_x + f->ch_spacing, final_y + ((f->real_face_h) >> 5), color);
				} else { //If the underline_right is FALSE
					rectfill(bmp, first_x, final_y, final_x, final_y + ((f->real_face_h) >> 5), color);
				}
			} else {
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					masked_blit(masked_bmp, bmp, 0, 0, first_x, final_y, final_x + f->ch_spacing - first_x + 1, final_y + ((f->real_face_h) >> 5) - final_y + 1);
					destroy_bitmap(masked_bmp);
				} else { //If the underline_right is FALSE
					masked_blit(masked_bmp, bmp, 0, 0, first_x, final_y, final_x - first_x + 1, final_y + ((f->real_face_h) >> 5) - final_y + 1);
					destroy_bitmap(masked_bmp);
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
		} else {
			if (f->outline_hollow == TRUE) { //set the masked region for underline
				int bmp_x, bmp_y;
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), final_x + f->ch_spacing - first_x + 1, final_y + 5 - final_y + 1);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					for (bmp_y = final_y; bmp_y <= final_y + 5; bmp_y++) {
						for (bmp_x = first_x; bmp_x <= final_x + f->ch_spacing; bmp_x++) {
							putpixel(masked_bmp, bmp_x - first_x, bmp_y - final_y, getpixel(bmp, bmp_x, bmp_y));
						}
					}
				} else { //If the underline_right is FALSE
					masked_bmp = create_bitmap_ex(bitmap_color_depth(bmp), final_x - first_x + 1, final_y + 5 - final_y + 1);
					clear_to_color(masked_bmp, bitmap_mask_color(bmp));
					for (bmp_y = final_y; bmp_y <= final_y + 5; bmp_y++) {
						for (bmp_x = first_x; bmp_x <= final_x; bmp_x++) {
							putpixel(masked_bmp, bmp_x - first_x, bmp_y - final_y, getpixel(bmp, bmp_x, bmp_y));
						}
					}
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
			//Draw outline
			if ((f->outline_top > 0) || (f->outline_bottom > 0) || (f->outline_left > 0) || (f->outline_right > 0)) {
				if (f->outline_top > 0) {
					if (f->underline_right == TRUE) { //If the underline_right is TRUE
						rectfill(bmp, first_x, final_y - 1, final_x + f->ch_spacing, final_y + 5, f->outline_color);
					} else { //If the underline_right is FALSE
						rectfill(bmp, first_x, final_y - 1, final_x, final_y + 5, f->outline_color);
					}
				}
				if (f->outline_bottom > 0) {
					if (f->underline_right == TRUE) { //If the underline_right is TRUE
						rectfill(bmp, first_x, final_y, final_x + f->ch_spacing, final_y + 5, f->outline_color);
					} else { //If the underline_right is FALSE
						rectfill(bmp, first_x, final_y, final_x, final_y + 5, f->outline_color);
					}
				}
				if (f->outline_left > 0) {
					if (f->underline_left == FALSE) {
						rectfill(bmp, first_x - 1, final_y, final_x, final_y + 5, f->outline_color);
					}
				}
				if (f->outline_right > 0) {
					if (f->underline_right == FALSE) {
						rectfill(bmp, first_x, final_y, final_x + 1, final_y + 5, f->outline_color);
					}
				}
			}
			if (f->outline_hollow == FALSE) { //check if it is the masked region for outline
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					rectfill(bmp, first_x, final_y, final_x + f->ch_spacing, final_y + 5, color);
				} else { //If the underline_right is FALSE
					rectfill(bmp, first_x, final_y, final_x, final_y + 5, color);
				}
			} else {
				//cancel transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
					}
				}
				if (f->underline_right == TRUE) { //If the underline_right is TRUE
					masked_blit(masked_bmp, bmp, 0, 0, first_x, final_y, final_x + f->ch_spacing - first_x + 1, final_y + 5 - final_y + 1);
					destroy_bitmap(masked_bmp);
				} else { //If the underline_right is FALSE
					masked_blit(masked_bmp, bmp, 0, 0, first_x, final_y, final_x - first_x + 1, final_y + 5 - final_y + 1);
					destroy_bitmap(masked_bmp);
				}
				//restore transparency
				if (f->transparency != 255) {
					if (bitmap_color_depth(bmp) > 8) {
						drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
					}
				}
			}
		}
	}

	release_bitmap(bmp);

	if ((f->type == 1) || (f->type == 2)) {
		if (lpszW_pointer)
			free(lpszW_pointer);
	}

	if (s_pointer) {
		free(s_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	}


int alfont_text_height(ALFONT_FONT * f) {
	return f->face_h;
}


int alfont_text_length(ALFONT_FONT * f, const char *str) {
	char *lpszW;
	char *lpszW_tmp;
	int max_advancex = 0;
	char *lpszW_pointer = NULL; //used for freeing string
	char *str_pointer = NULL; //used for original string fixed by autofix
	char *str_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int curr_uformat = 0;
	int total_length = 0, character, last_glyph_index;
	int glyph_index;
	int glyph_index_tmp;
#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (str == NULL) {
		return 0;
	}

	nLen = strlen(str) + 1;
	str_pointer = (char *)malloc(nLen * sizeof(char));
	memset(str_pointer, 0, nLen);
	strcpy(str_pointer, str);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the str string
	//If find the cutted character, store it from the converted str string and remove it from the original str string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(str) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the str string
				if (f->precedingchar != 0) {
					free(str_pointer);
					fromlen = strlen(str) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					str_pointer = (char *)malloc(tolen * sizeof(char));
					memset(str_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(str_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(str_pointer, str);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = str;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				str_pointer_temp = str_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*str_pointer_temp != '\0') {
						f->precedingchar = *str_pointer_temp;
						str_pointer_temp++;
					}
					//remove the final character
					str_pointer_temp--;
					*str_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(str_pointer) * 5 + 1;
#else
			nLen = strlen(str_pointer) + 1;
#endif

			//add the previous character to the str string
			if (f->precedingchar != 0) {
				free(str_pointer);
				nLen = strlen(str) + 1 + 1;
				str_pointer = (char *)malloc(nLen * sizeof(char));
				memset(str_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				snprintf(precedingchar_pointer, 2 * sizeof(char), "%c", f->precedingchar);
				strcpy(str_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(str_pointer, str);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, str_pointer, nLen);

			str_pointer_temp = str_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*str_pointer_temp != '\0') {
					f->precedingchar = *str_pointer_temp;
					str_pointer_temp++;
				}
				//remove the final character
				str_pointer_temp--;
				*str_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	//Font Code Convert


	if (f->type == 1) {

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			lpszW = (char *)str_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(str_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			lpszW_pointer = lpszW;
			sin = str_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)str_pointer;
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)str_pointer) + 1;
		lpszW = (char *)malloc(nLen * sizeof(char));
		memset(lpszW, 0, nLen);
		lpszW_pointer = lpszW;
		wcstombs(lpszW, (const wchar_t *)str_pointer, nLen);
#endif
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open("UTF-16LE", f->language)) == (iconv_t)-1) {
			lpszW = (char *)str_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(str_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			lpszW_pointer = lpszW;
			sin = str_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)str_pointer;
			} else {
				set_uformat(U_UNICODE);
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);

#ifdef ALFONT_LINUX
		nLen = strlen(str_pointer) * 5 + 1;
#else
		nLen = strlen(str_pointer) + 1;
#endif

		lpszW = (char *)malloc(nLen * sizeof(wchar_t));
		memset(lpszW, 0, nLen);
		lpszW_pointer = lpszW;
		mbstowcs((wchar_t *)lpszW, str_pointer, nLen);
#endif
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
		nLen = ustrlen(str_pointer) + 1;
#endif
		lpszW = (char *)str_pointer;
	}

	/* virtually draw char by char */
	last_glyph_index = 0;

	if (f->fixed_width == TRUE) {
		lpszW_tmp = lpszW;
		max_advancex = 0;

		_alfont_uncache_glyphs(f);

#ifdef ALFONT_LINUX //Fix for Linux Unicode System(be converted)
		for (character = ugetxc(&lpszW_tmp); character != 0; character = ugetxc(&lpszW_tmp), character = ugetxc(&lpszW_tmp)) {
#else
		for (character = ugetxc(&lpszW_tmp); character != 0; character = ugetxc(&lpszW_tmp)) {
#endif
			/* get the character out of the font */

#ifdef ALFONT_LINUX //Recover for Linux Unicode System Fixed
			if (f->type != 2) {
				lpszW_tmp--;
			}
#endif

			/* get the character out of the font */
			if (f->face->charmap)
				glyph_index_tmp = Get_Char_Index(f->face, character);
			else
				glyph_index_tmp = character;

			/* if out of existing glyph range -- skip it */
			if ((glyph_index_tmp < 0) || (glyph_index_tmp >= f->face->num_glyphs))
				continue;

			/* cache the glyph */
			_alfont_cache_glyph(f, glyph_index_tmp);
			if (max_advancex < f->cached_glyphs[glyph_index_tmp].advancex)
				max_advancex = f->cached_glyphs[glyph_index_tmp].advancex;
		}
		}

#ifdef ALFONT_LINUX //Fix for Linux Unicode System(be converted)
	for (character = ugetxc(&lpszW); character != 0; character = ugetxc(&lpszW), character = ugetxc(&lpszW)) {
#else
	for (character = ugetxc(&lpszW); character != 0; character = ugetxc(&lpszW)) {
#endif
		/* get the character out of the font */

#ifdef ALFONT_LINUX //Recover for Linux Unicode System Fixed
		if (f->type != 2) {
			lpszW--;
		}
#endif

		if (f->face->charmap)
			glyph_index = Get_Char_Index(f->face, character);
		else
			glyph_index = character;

		/* apply kerning */
		/*if (last_glyph_index) {
			FT_Vector v;
			Get_Kerning(f->face, last_glyph_index, glyph_index, ft_kerning_default, &v);
			total_length += v.x >> 6;
		}*/
		last_glyph_index = glyph_index;

		/* if out of existing glyph range -- skip it */
		if ((glyph_index < 0) || (glyph_index >= f->face->num_glyphs))
			continue;

		/* cache */
		_alfont_cache_glyph(f, glyph_index);
		if (f->fixed_width == TRUE)
			f->cached_glyphs[glyph_index].advancex = max_advancex;

		/* advance */
		if (f->cached_glyphs[glyph_index].advancex)
			total_length += f->cached_glyphs[glyph_index].advancex + f->ch_spacing;
	}

	if ((f->style == 1) || (f->style == 3)) {
		if (f->cached_glyphs[last_glyph_index].advancex)
			total_length += (f->outline_right + 1) + (f->cached_glyphs[last_glyph_index].advancex / 2);
	}

	if ((f->type == 1) || (f->type == 2)) {
		if (lpszW_pointer)
			free(lpszW_pointer);
	}

	if (str_pointer) {
		free(str_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	return total_length;
	}

int alfont_char_length(ALFONT_FONT * f, int character) {
	int curr_uformat = 0;
	int total_length = 0, last_glyph_index;
	int glyph_index;

	if (character == 0) {
		return 0;
	}

	//Font Code Convert


	if (f->type == 1) {
		setlocale(LC_CTYPE, f->language);
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
#endif
	}

	/* virtually draw char by char */
	last_glyph_index = 0;


	/* get the character out of the font */

	if (f->face->charmap)
		glyph_index = Get_Char_Index(f->face, character);
	else
		glyph_index = character;

	/* apply kerning */
	/*if (last_glyph_index) {
	  FT_Vector v;
	  Get_Kerning(f->face, last_glyph_index, glyph_index, ft_kerning_default, &v);
	  total_length += v.x >> 6;
	}*/
	last_glyph_index = glyph_index;

	/* if out of existing glyph range -- imagine empty char */
	if ((glyph_index < 0) || (glyph_index >= f->face->num_glyphs))
		return 0;

	if (f->fixed_width == TRUE) {
		_alfont_uncache_glyph_number(f, glyph_index);
	}

	/* cache */
	_alfont_cache_glyph(f, glyph_index);

	/* advance */
	if (f->cached_glyphs[glyph_index].advancex)
		total_length += f->cached_glyphs[glyph_index].advancex + f->ch_spacing;

	if ((f->style == 1) || (f->style == 3)) {
		if (f->cached_glyphs[last_glyph_index].advancex)
			total_length += (f->outline_right + 1) + (f->cached_glyphs[last_glyph_index].advancex / 2);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	return total_length;
}

int alfont_text_count(ALFONT_FONT * f, const char *str) {
	char *lpszW;
	char *lpszW_pointer = NULL; //used for freeing string
	char *str_pointer = NULL; //used for original string fixed by autofix
	char *str_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int curr_uformat = 0;
	int string_count = 0;
#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (str == NULL) {
		return 0;
	}

	nLen = strlen(str) + 1;
	str_pointer = (char *)malloc(nLen * sizeof(char));
	memset(str_pointer, 0, nLen);
	strcpy(str_pointer, str);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the str string
	//If find the cutted character, store it from the converted str string and remove it from the original str string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(str) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the str string
				if (f->precedingchar != 0) {
					free(str_pointer);
					fromlen = strlen(str) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					str_pointer = (char *)malloc(tolen * sizeof(char));
					memset(str_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(str_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(str_pointer, str);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = str;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				str_pointer_temp = str_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*str_pointer_temp != '\0') {
						f->precedingchar = *str_pointer_temp;
						str_pointer_temp++;
					}
					//remove the final character
					str_pointer_temp--;
					*str_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(str_pointer) * 5 + 1;
#else
			nLen = strlen(str_pointer) + 1;
#endif

			//add the previous character to the str string
			if (f->precedingchar != 0) {
				free(str_pointer);
				nLen = strlen(str) + 1 + 1;
				str_pointer = (char *)malloc(nLen * sizeof(char));
				memset(str_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				snprintf(precedingchar_pointer, 2 * sizeof(char), "%c", f->precedingchar);
				strcpy(str_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(str_pointer, str);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, str_pointer, nLen);

			str_pointer_temp = str_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*str_pointer_temp != '\0') {
					f->precedingchar = *str_pointer_temp;
					str_pointer_temp++;
				}
				//remove the final character
				str_pointer_temp--;
				*str_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	//Font Code Convert


	if (f->type == 1) {
#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			lpszW = (char *)str_pointer;
			string_count = strlen(lpszW);
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(str_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			lpszW_pointer = lpszW;
			sin = str_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)str_pointer;
			}
			string_count = strlen(lpszW);
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)str_pointer) + 1;
		lpszW = (char *)malloc(nLen * sizeof(char));
		memset(lpszW, 0, nLen);
		lpszW_pointer = lpszW;
		string_count = wcstombs(lpszW, (const wchar_t *)str_pointer, nLen);
#endif
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open("UTF-16LE", f->language)) == (iconv_t)-1) {
			lpszW = (char *)str_pointer;
			string_count = strlen(lpszW);
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(str_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			lpszW_pointer = lpszW;
			sin = str_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)str_pointer;
				string_count = strlen(lpszW);
			} else {
				set_uformat(U_UNICODE);
				string_count = ustrlen(lpszW);
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);

#ifdef ALFONT_LINUX
		nLen = strlen(str_pointer) * 5 + 1;
#else
		nLen = strlen(str_pointer) + 1;
#endif

		lpszW = (char *)malloc(nLen * sizeof(wchar_t));
		memset(lpszW, 0, nLen);
		lpszW_pointer = lpszW;
		string_count = mbstowcs((wchar_t *)lpszW, str_pointer, nLen);
#endif
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
		nLen = ustrlen(str_pointer) + 1;
		string_count = ustrlen(str_pointer);
#else
		string_count = strlen(str_pointer);
#endif
		lpszW = (char *)str_pointer;

	}

	if ((f->type == 1) || (f->type == 2)) {
		if (lpszW_pointer)
			free(lpszW_pointer);
	}

	if (str_pointer) {
		free(str_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	return string_count;
}

int alfont_ugetc(ALFONT_FONT * f, const char *s) {
	char *lpszW;
//	char *lpszW_pointer = NULL; //used for freeing string
	char *s_pointer = NULL; //used for original string fixed by autofix
	char *s_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int character;
	int curr_uformat = 0;
#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (s == NULL) {
		return 0;
	}

	nLen = strlen(s) + 1;
	s_pointer = (char *)malloc(nLen * sizeof(char));
	memset(s_pointer, 0, nLen);
	strcpy(s_pointer, s);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the s string
	//If find the cutted character, store it from the converted s string and remove it from the original s string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(s) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the s string
				if (f->precedingchar != 0) {
					free(s_pointer);
					fromlen = strlen(s) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					s_pointer = (char *)malloc(tolen * sizeof(char));
					memset(s_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(s_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(s_pointer, s);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = s;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				s_pointer_temp = s_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*s_pointer_temp != '\0') {
						f->precedingchar = *s_pointer_temp;
						s_pointer_temp++;
					}
					//remove the final character
					s_pointer_temp--;
					*s_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(s_pointer) * 5 + 1;
#else
			nLen = strlen(s_pointer) + 1;
#endif

			//add the previous character to the s string
			if (f->precedingchar != 0) {
				free(s_pointer);
				nLen = strlen(s) + 1 + 1;
				s_pointer = (char *)malloc(nLen * sizeof(char));
				memset(s_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				snprintf(precedingchar_pointer, 2 * sizeof(char), "%c", f->precedingchar);
				strcpy(s_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(s_pointer, s);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, s_pointer, nLen);

			s_pointer_temp = s_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*s_pointer_temp != '\0') {
					f->precedingchar = *s_pointer_temp;
					s_pointer_temp++;
				}
				//remove the final character
				s_pointer_temp--;
				*s_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	//Font Code Convert


	if (f->type == 1) {

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)s_pointer) + 1;
		lpszW = (char *)malloc(nLen * sizeof(char));
		memset(lpszW, 0, nLen);
		wcstombs(lpszW, (const wchar_t *)s_pointer, nLen);
#endif
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open("UTF-16LE", f->language)) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			} else {
				set_uformat(U_UNICODE);
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);

#ifdef ALFONT_LINUX
		nLen = strlen(s_pointer) * 5 + 1;
#else
		nLen = strlen(s_pointer) + 1;
#endif

		lpszW = (char *)malloc(nLen * sizeof(wchar_t));
		memset(lpszW, 0, nLen);
		mbstowcs((wchar_t *)lpszW, s_pointer, nLen);
#endif
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
		nLen = ustrlen(s_pointer) + 1;
#endif
		lpszW = (char *)s_pointer;
	}

	character = ugetc((const char *)lpszW);

	if ((f->type == 1) || (f->type == 2)) {
		if (lpszW)
			free(lpszW);
	}

	if (s_pointer) {
		free(s_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	return character;
}

int alfont_ugetx(ALFONT_FONT * f, char **s) {
	char *lpszW;
//	char *lpszW_pointer = NULL; //used for freeing string
	char *s_pointer = NULL; //used for original string fixed by autofix
	char *s_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int character;
	int curr_uformat = 0;

	char *lpszWS;
	char *lpszWA;
	int sLen; //length before advances the *s pointer to the next character
	int aLen; //length after advances the *s pointer to the next character
	int lIndex;

#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (*s == NULL) {
		return 0;
	}

	sLen = strlen(*s);

	nLen = strlen(*s) + 1;
	s_pointer = (char *)malloc(nLen * sizeof(char));
	memset(s_pointer, 0, nLen);
	strcpy(s_pointer, *s);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the *s string
	//If find the cutted character, store it from the converted *s string and remove it from the original *s string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(*s) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the *s string
				if (f->precedingchar != 0) {
					free(s_pointer);
					fromlen = strlen(*s) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					s_pointer = (char *)malloc(tolen * sizeof(char));
					memset(s_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(s_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(s_pointer, *s);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = *s;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				s_pointer_temp = s_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*s_pointer_temp != '\0') {
						f->precedingchar = *s_pointer_temp;
						s_pointer_temp++;
					}
					//remove the final character
					s_pointer_temp--;
					*s_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(s_pointer) * 5 + 1;
#else
			nLen = strlen(s_pointer) + 1;
#endif

			//add the previous character to the *s string
			if (f->precedingchar != 0) {
				free(s_pointer);
				nLen = strlen(*s) + 1 + 1;
				s_pointer = (char *)malloc(nLen * sizeof(char));
				memset(s_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				snprintf(precedingchar_pointer, 2 * sizeof(char), "%c", f->precedingchar);
				strcpy(s_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(s_pointer, *s);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, s_pointer, nLen);

			s_pointer_temp = s_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*s_pointer_temp != '\0') {
					f->precedingchar = *s_pointer_temp;
					s_pointer_temp++;
				}
				//remove the final character
				s_pointer_temp--;
				*s_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	//Font Code Convert


	if (f->type == 1) {

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)s_pointer) + 1;
		lpszW = (char *)malloc(nLen * sizeof(char));
		memset(lpszW, 0, nLen);
		wcstombs(lpszW, (const wchar_t *)s_pointer, nLen);
#endif
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open("UTF-16LE", f->language)) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			} else {
				set_uformat(U_UNICODE);
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);

#ifdef ALFONT_LINUX
		nLen = strlen(s_pointer) * 5 + 1;
#else
		nLen = strlen(s_pointer) + 1;
#endif

		lpszW = (char *)malloc(nLen * sizeof(wchar_t));
		memset(lpszW, 0, nLen);
		mbstowcs((wchar_t *)lpszW, s_pointer, nLen);
#endif
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
		nLen = ustrlen(s_pointer) + 1;
#endif
		lpszW = (char *)s_pointer;
	}

	lpszWS = lpszW;
	character = ugetc((const char *)lpszW);

	//advances the *s pointer to the next character

	if (f->type == 1) {
		ugetxc(s);
#ifdef ALFONT_LINUX
		ugetxc(s);
#endif
	} else if (f->type == 2) {
		ugetxc(&lpszWS);
#ifdef ALFONT_LINUX
		ugetxc(&lpszWS);
#endif
#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			aLen = strlen(lpszWS);
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(lpszWS) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszWA = (char *)malloc(tolen * sizeof(char));
			memset(lpszWA, 0, tolen);
			sin = lpszWS;
			sout = lpszWA;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				if (lpszWA)
					free(lpszWA);
				lpszWA = lpszWS
			}
			aLen = strlen(lpszWA);
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)lpszWS) + 1;
		lpszWA = (char *)malloc(nLen * sizeof(char));
		memset(lpszWA, 0, nLen);
		wcstombs(lpszWA, (const wchar_t *)lpszWS, nLen);
		aLen = strlen(lpszWA);
#endif

		for (lIndex = 0; lIndex < sLen - aLen; lIndex++) {
			(*s)++;
		}

		if (lpszWA) {
			free(lpszWA);
		}
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
#endif
		ugetxc(s);
#ifdef ALFONT_LINUX
		ugetxc(s);
#endif
	}

	if ((f->type == 1) || (f->type == 2)) {
		if (lpszW)
			free(lpszW);
	}

	if (s_pointer) {
		free(s_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	return character;
}

int alfont_ugetxc(ALFONT_FONT * f, const char **s) {
	char *lpszW;
//	char *lpszW_pointer = NULL; //used for freeing string
	char *s_pointer = NULL; //used for original string fixed by autofix
	char *s_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int character;
	int curr_uformat = 0;

	char *lpszWS;
	char *lpszWA;
	int sLen; //length before advances the *s pointer to the next character
	int aLen; //length after advances the *s pointer to the next character
	int lIndex;

#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (*s == NULL) {
		return 0;
	}

	sLen = strlen(*s);

	nLen = strlen(*s) + 1;
	s_pointer = (char *)malloc(nLen * sizeof(char));
	memset(s_pointer, 0, nLen);
	strcpy(s_pointer, *s);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the *s string
	//If find the cutted character, store it from the converted *s string and remove it from the original *s string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(*s) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the *s string
				if (f->precedingchar != 0) {
					free(s_pointer);
					fromlen = strlen(*s) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					s_pointer = (char *)malloc(tolen * sizeof(char));
					memset(s_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(s_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(s_pointer, *s);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = *s;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				s_pointer_temp = s_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*s_pointer_temp != '\0') {
						f->precedingchar = *s_pointer_temp;
						s_pointer_temp++;
					}
					//remove the final character
					s_pointer_temp--;
					*s_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(s_pointer) * 5 + 1;
#else
			nLen = strlen(s_pointer) + 1;
#endif

			//add the previous character to the *s string
			if (f->precedingchar != 0) {
				free(s_pointer);
				nLen = strlen(*s) + 1 + 1;
				s_pointer = (char *)malloc(nLen * sizeof(char));
				memset(s_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				snprintf(precedingchar_pointer, 2 * sizeof(char), "%c", f->precedingchar);
				strcpy(s_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(s_pointer, *s);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, s_pointer, nLen);

			s_pointer_temp = s_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*s_pointer_temp != '\0') {
					f->precedingchar = *s_pointer_temp;
					s_pointer_temp++;
				}
				//remove the final character
				s_pointer_temp--;
				*s_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	//Font Code Convert


	if (f->type == 1) {

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)s_pointer) + 1;
		lpszW = (char *)malloc(nLen * sizeof(char));
		memset(lpszW, 0, nLen);
		wcstombs(lpszW, (const wchar_t *)s_pointer, nLen);
#endif
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open("UTF-16LE", f->language)) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			} else {
				set_uformat(U_UNICODE);
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);

#ifdef ALFONT_LINUX
		nLen = strlen(s_pointer) * 5 + 1;
#else
		nLen = strlen(s_pointer) + 1;
#endif

		lpszW = (char *)malloc(nLen * sizeof(wchar_t));
		memset(lpszW, 0, nLen);
		mbstowcs((wchar_t *)lpszW, s_pointer, nLen);
#endif
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
		nLen = ustrlen(s_pointer) + 1;
#endif
		lpszW = (char *)s_pointer;
	}

	lpszWS = lpszW;
	character = ugetc((const char *)lpszW);

	//advances the *s pointer to the next character

	if (f->type == 1) {
		ugetxc(s);
#ifdef ALFONT_LINUX
		ugetxc(s);
#endif
	} else if (f->type == 2) {
		ugetxc(&lpszWS);
#ifdef ALFONT_LINUX
		ugetxc(&lpszWS);
#endif
#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			aLen = strlen(lpszWS);
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(lpszWS) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszWA = (char *)malloc(tolen * sizeof(char));
			memset(lpszWA, 0, tolen);
			sin = lpszWS;
			sout = lpszWA;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				if (lpszWA)
					free(lpszWA);
				lpszWA = lpszWS
			}
			aLen = strlen(lpszWA);
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)lpszWS) + 1;
		lpszWA = (char *)malloc(nLen * sizeof(char));
		memset(lpszWA, 0, nLen);
		wcstombs(lpszWA, (const wchar_t *)lpszWS, nLen);
		aLen = strlen(lpszWA);
#endif

		for (lIndex = 0; lIndex < sLen - aLen; lIndex++) {
			(*s)++;
		}

		if (lpszWA) {
			free(lpszWA);
		}
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
#endif
		ugetxc((const char **)s);
#ifdef ALFONT_LINUX
		ugetxc((const char **)s);
#endif
	}

	if ((f->type == 1) || (f->type == 2)) {
		if (lpszW)
			free(lpszW);
	}

	if (s_pointer) {
		free(s_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	return character;
}

// Following function alfont_get_string is removed from compilation because it
// is implemented with the use of non-standart malloc_usable_size function
// (defined as _msize). This may cause linking errors on some Linux systems or
// if using particular compilers.
#if 0

void alfont_get_string(ALFONT_FONT * f, const char *s, char **out) {
	char *lpszW;
	char *lpszW_pointer = NULL; //used for freeing string
	char *s_pointer = NULL; //used for original string fixed by autofix
	char *s_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int curr_uformat;

#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (s == NULL) {
		return;
	}

	nLen = strlen(s) + 1;
	s_pointer = (char *)malloc(nLen * sizeof(char));
	memset(s_pointer, 0, nLen);
	strcpy(s_pointer, s);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the s string
	//If find the cutted character, store it from the converted s string and remove it from the original s string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(s) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the s string
				if (f->precedingchar != 0) {
					free(s_pointer);
					fromlen = strlen(s) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					s_pointer = (char *)malloc(tolen * sizeof(char));
					memset(s_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(s_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(s_pointer, s);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = s;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				s_pointer_temp = s_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*s_pointer_temp != '\0') {
						f->precedingchar = *s_pointer_temp;
						s_pointer_temp++;
					}
					//remove the final character
					s_pointer_temp--;
					*s_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(s_pointer) * 5 + 1;
#else
			nLen = strlen(s_pointer) + 1;
#endif

			//add the previous character to the s string
			if (f->precedingchar != 0) {
				free(s_pointer);
				nLen = strlen(s) + 1 + 1;
				s_pointer = (char *)malloc(nLen * sizeof(char));
				memset(s_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				sprintf(precedingchar_pointer, "%c", f->precedingchar);
				strcpy(s_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(s_pointer, s);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, s_pointer, nLen);

			s_pointer_temp = s_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*s_pointer_temp != '\0') {
					f->precedingchar = *s_pointer_temp;
					s_pointer_temp++;
				}
				//remove the final character
				s_pointer_temp--;
				*s_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	//Font Code Convert


	if (f->type == 1) {

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open(f->language, "UTF-16LE")) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		nLen = MB_CUR_MAX * wcslen((const wchar_t *)s_pointer) + 1;
		lpszW = (char *)malloc(nLen * sizeof(char));
		memset(lpszW, 0, nLen);
		wcstombs(lpszW, (const wchar_t *)s_pointer, nLen);
#endif
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

#ifdef ALFONT_DOS
		if ((c_pt = iconv_open("UTF-16LE", f->language)) == (iconv_t)-1) {
			lpszW = (char *)s_pointer;
		} else {
			iconv(c_pt, NULL, NULL, NULL, NULL);
			fromlen = strlen(s_pointer) + 1;
			tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
			lpszW = (char *)malloc(tolen * sizeof(char));
			memset(lpszW, 0, tolen);
			sin = s_pointer;
			sout = lpszW;
			ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
			iconv_close(c_pt);
			if (ret == -1) {
				lpszW = (char *)s_pointer;
			} else {
				set_uformat(U_UNICODE);
			}
		}
#else
		setlocale(LC_CTYPE, f->language);
		set_uformat(U_UNICODE);

#ifdef ALFONT_LINUX
		nLen = strlen(s_pointer) * 5 + 1;
#else
		nLen = strlen(s_pointer) + 1;
#endif

		lpszW = (char *)malloc(nLen * sizeof(wchar_t));
		memset(lpszW, 0, nLen);
		mbstowcs((wchar_t *)lpszW, s_pointer, nLen);
#endif
	} else {
#ifdef ALFONT_LINUX
		set_uformat(U_UTF8);
		nLen = ustrlen(s_pointer) + 1;
#endif
		lpszW = (char *)s_pointer;
	}

	memset(*out, 0, _msize(*out));
	if (_msize(*out) > 0 && _msize(lpszW) >= _msize(*out)) {
		memcpy(*out, lpszW, _msize(*out));
	} else if (_msize(*out) > 0 && _msize(*out) > _msize(lpszW)) {
		memcpy(*out, lpszW, _msize(lpszW));
	}

	if ((f->type == 1) || (f->type == 2)) {
		if (lpszW)
			free(lpszW);
	}

	if (s_pointer) {
		free(s_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}
}

#endif // DISABLED

int alfont_need_uconvert(ALFONT_FONT * f, const char *str) {
	char *lpszW;
	char *str_pointer = NULL; //used for original string fixed by autofix
	char *str_pointer_temp = NULL; //temporary used for autofix string
	char *precedingchar_pointer = NULL; //used for precedingchar character
	int need_unicode_convert = TRUE;
	int nLen;
	int ret; //decide that if the ASCII Code convert to Unicode Code is all OK when used for autofix string or used for general convert.
	int curr_uformat = 0;
#ifdef ALFONT_DOS
	iconv_t c_pt;
	size_t fromlen, tolen;
	char *sin, *sout;
#endif

	if (str == NULL) {
		return FALSE;
	}

	nLen = strlen(str) + 1;
	str_pointer = (char *)malloc(nLen * sizeof(char));
	memset(str_pointer, 0, nLen);
	strcpy(str_pointer, str);

	//Auto Fix for cutted string
	//For ASCII convert to unicode
	//Add the previous character to the str string
	//If find the cutted character, store it from the converted str string and remove it from the original str string
	if (f->autofix == TRUE) {
		if (f->type == 2) {
			curr_uformat = get_uformat();

#ifdef ALFONT_DOS
			if ((c_pt = iconv_open("UTF-16LE", f->language)) != (iconv_t)-1) {

				fromlen = strlen(str) + 1;
				tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);

				//add the previous character to the str string
				if (f->precedingchar != 0) {
					free(str_pointer);
					fromlen = strlen(str) + 1 + 1;
					tolen = MB_CUR_MAX * fromlen * (sizeof(wchar_t) + 1);
					str_pointer = (char *)malloc(tolen * sizeof(char));
					memset(str_pointer, 0, tolen);
					precedingchar_pointer = (char *)malloc(2 * sizeof(char));
					memset(precedingchar_pointer, 0, 2);
					sprintf(precedingchar_pointer, "%c", f->precedingchar);
					strcpy(str_pointer, precedingchar_pointer);
					if (precedingchar_pointer) {
						free(precedingchar_pointer);
						precedingchar_pointer = NULL;
					}
					strcat(str_pointer, str);
					f->precedingchar = 0;
				}

				iconv(c_pt, NULL, NULL, NULL, NULL);
				lpszW = (char *)malloc(tolen * sizeof(char));
				memset(lpszW, 0, tolen);
				sin = str;
				sout = lpszW;
				ret = iconv(c_pt, &sin, &fromlen, &sout, &tolen);
				iconv_close(c_pt);

				str_pointer_temp = str_pointer;

				if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
					//store the last character to precedingchar character
					//get the final character
					set_uformat(curr_uformat);
					while (*str_pointer_temp != '\0') {
						f->precedingchar = *str_pointer_temp;
						str_pointer_temp++;
					}
					//remove the final character
					str_pointer_temp--;
					*str_pointer_temp = '\0';
				}
				if (lpszW) {
					free(lpszW);
					lpszW = NULL;
				}
			}
#else


#ifdef ALFONT_LINUX
			nLen = strlen(str_pointer) * 5 + 1;
#else
			nLen = strlen(str_pointer) + 1;
#endif

			//add the previous character to the str string
			if (f->precedingchar != 0) {
				free(str_pointer);
				nLen = strlen(str) + 1 + 1;
				str_pointer = (char *)malloc(nLen * sizeof(char));
				memset(str_pointer, 0, nLen);
				precedingchar_pointer = (char *)malloc(2 * sizeof(char));
				memset(precedingchar_pointer, 0, 2);
				snprintf(precedingchar_pointer, 2 * sizeof(char), "%c", f->precedingchar);
				strcpy(str_pointer, precedingchar_pointer);
				if (precedingchar_pointer) {
					free(precedingchar_pointer);
					precedingchar_pointer = NULL;
				}
				strcat(str_pointer, str);
				f->precedingchar = 0;
			}

			setlocale(LC_CTYPE, f->language);
			set_uformat(U_UNICODE);

			lpszW = (char *)malloc(nLen * sizeof(wchar_t));
			memset(lpszW, 0, nLen);
			ret = mbstowcs((wchar_t *)lpszW, str_pointer, nLen);

			str_pointer_temp = str_pointer;

			if (ret == -1) { //If the ret is -1, the final one will can be a shortcutted character.
				//store the last character to precedingchar character
				//get the final character
				set_uformat(curr_uformat);
				while (*str_pointer_temp != '\0') {
					f->precedingchar = *str_pointer_temp;
					str_pointer_temp++;
				}
				//remove the final character
				str_pointer_temp--;
				*str_pointer_temp = '\0';
			}
			if (lpszW) {
				free(lpszW);
				lpszW = NULL;
			}
#endif
			//recover to original codepage
			set_uformat(curr_uformat);
		}
	}


	if (f->type == 1) {
		need_unicode_convert = FALSE;
	} else if (f->type == 2) {
		curr_uformat = get_uformat();

		if (!need_uconvert(str_pointer, U_ASCII, U_UTF8)) {
			need_unicode_convert = FALSE;
		}
	} else {
		need_unicode_convert = FALSE;
	}

	if (str_pointer) {
		free(str_pointer);
	}

#ifndef ALFONT_DOS
	setlocale(LC_CTYPE, "");
#endif

	if (f->type == 2) {
		set_uformat(curr_uformat);
	}

	return need_unicode_convert;
}


void alfont_textout_centre_aa(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color) {
	alfont_textout_centre_aa_ex(bmp, f, s, x, y, color, alfont_textmode);
}


void alfont_textout_centre_aa_ex(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color, int backg) {
	x -= alfont_text_length(f, s) / 2;
	alfont_textout_aa_ex(bmp, f, s, x, y, color, backg);
}


void alfont_textout_centre(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color) {
	alfont_textout_centre_ex(bmp, f, s, x, y, color, alfont_textmode);
}


void alfont_textout_centre_ex(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color, int backg) {
	x -= alfont_text_length(f, s) / 2;
	alfont_textout_ex(bmp, f, s, x, y, color, backg);
}


void alfont_textout_right_aa(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color) {
	alfont_textout_right_aa_ex(bmp, f, s, x, y, color, alfont_textmode);
}


void alfont_textout_right_aa_ex(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color, int backg) {
	x -= alfont_text_length(f, s);
	alfont_textout_aa_ex(bmp, f, s, x, y, color, backg);
}


void alfont_textout_right(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color) {
	alfont_textout_right_ex(bmp, f, s, x, y, color, alfont_textmode);
}


void alfont_textout_right_ex(BITMAP * bmp, ALFONT_FONT * f, const char *s, int x, int y, int color, int backg) {
	x -= alfont_text_length(f, s);
	alfont_textout_ex(bmp, f, s, x, y, color, backg);
}


void alfont_textprintf(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout(bmp, f, buf, x, y, color);
}


void alfont_textprintf_ex(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, int backg, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_ex(bmp, f, buf, x, y, color, backg);
}


void alfont_textprintf_aa(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_aa(bmp, f, buf, x, y, color);
}


void alfont_textprintf_aa_ex(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, int backg, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_aa_ex(bmp, f, buf, x, y, color, backg);
}


void alfont_textprintf_centre(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_centre(bmp, f, buf, x, y, color);
}


void alfont_textprintf_centre_ex(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, int backg, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_centre_ex(bmp, f, buf, x, y, color, backg);
}


void alfont_textprintf_centre_aa(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_centre_aa(bmp, f, buf, x, y, color);
}


void alfont_textprintf_centre_aa_ex(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, int backg, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_centre_aa_ex(bmp, f, buf, x, y, color, backg);
}


void alfont_textprintf_right(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_right(bmp, f, buf, x, y, color);
}


void alfont_textprintf_right_ex(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, int backg, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_right_ex(bmp, f, buf, x, y, color, backg);
}


void alfont_textprintf_right_aa(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_right_aa(bmp, f, buf, x, y, color);
}


void alfont_textprintf_right_aa_ex(BITMAP * bmp, ALFONT_FONT * f, int x, int y, int color, int backg, const char *format, ...) {
	char buf[512];

	va_list ap;
	set_uformat(U_ASCII);
	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	alfont_textout_right_aa_ex(bmp, f, buf, x, y, color, backg);
}


int alfont_is_fixed_font(ALFONT_FONT * f) {
	if (f->num_fixed_sizes < 0)
		return FALSE;
	else
		return TRUE;
}


int alfont_is_scalable_font(ALFONT_FONT * f) {
	if (f->num_fixed_sizes < 0)
		return TRUE;
	else
		return FALSE;
}


const int *alfont_get_available_fixed_sizes(ALFONT_FONT * f) {
	/* we reget them just to make sure the contents where not changed */
	_alfont_reget_fixed_sizes(f);
	return f->fixed_sizes;
}


int alfont_get_nof_available_fixed_sizes(ALFONT_FONT * f) {
	return f->num_fixed_sizes;
}


int alfont_get_char_extra_spacing(ALFONT_FONT * f) {
	return f->ch_spacing;
}


void alfont_set_char_extra_spacing(ALFONT_FONT * f, int spacing) {
	if (spacing < 0)
		f->ch_spacing = 0;
	else if (spacing > 4096)
		f->ch_spacing = 4096;
	else
		f->ch_spacing = spacing;
}


void alfont_set_language(ALFONT_FONT * f, const char *language) {
	if (language == NULL) {
		f->language = NULL;
	} else {
		f->language = (char *)malloc((strlen(language) + 1) * sizeof(char));
		strcpy(f->language, language);
	}
}


char *alfont_get_language(ALFONT_FONT * f) {
	return f->language;
}


void alfont_set_convert(ALFONT_FONT * f, int type) {
	if (type < 0)
		f->type = 0;
	else if (type > 2)
		f->type = 2;
	else
		f->type = type;
}


int alfont_get_convert(ALFONT_FONT * f) {
	return f->type;
}


void alfont_set_font_outline_top(ALFONT_FONT * f, int w) {
	if (w < 0)
		f->outline_top = 0;
	else
		f->outline_top = w;
}


int alfont_get_font_outline_top(ALFONT_FONT * f) {
	return f->outline_top;
}


void alfont_set_font_outline_bottom(ALFONT_FONT * f, int w) {
	if (w < 0)
		f->outline_bottom = 0;
	else
		f->outline_bottom = w;
}


int alfont_get_font_outline_bottom(ALFONT_FONT * f) {
	return f->outline_bottom;
}


void alfont_set_font_outline_right(ALFONT_FONT * f, int w) {
	if (w < 0)
		f->outline_right = 0;
	else
		f->outline_right = w;
}


int alfont_get_font_outline_right(ALFONT_FONT * f) {
	return f->outline_right;
}


void alfont_set_font_outline_left(ALFONT_FONT * f, int w) {
	if (w < 0)
		f->outline_left = 0;
	else
		f->outline_left = w;
}


int alfont_get_font_outline_left(ALFONT_FONT * f) {
	return f->outline_left;
}


void alfont_set_font_outline_color(ALFONT_FONT * f, int c) {
	if (c < 0)
		f->outline_color = 0;
	else
		f->outline_color = c;
}


int alfont_get_font_outline_color(ALFONT_FONT * f) {
	return f->outline_color;
}


void alfont_set_font_outline_hollow(ALFONT_FONT * f, int hollow) {
	if (hollow == FALSE)
		f->outline_hollow = FALSE;
	else
		f->outline_hollow = TRUE;
}


int alfont_get_font_outline_hollow(ALFONT_FONT * f) {
	return f->outline_hollow;
}


void alfont_set_font_style(ALFONT_FONT * f, int style) {
	if (style < 0)
		f->style = 0;
	else if (style > 3)
		f->style = 3;
	else
		f->style = style;
}


int alfont_get_font_style(ALFONT_FONT * f) {
	return f->style;
}


void alfont_set_font_underline(ALFONT_FONT * f, int underline) {
	if (underline == FALSE)
		f->underline = FALSE;
	else
		f->underline = TRUE;
}


int alfont_get_font_underline(ALFONT_FONT * f) {
	return f->underline;
}


void alfont_set_font_underline_right(ALFONT_FONT * f, int underline_right) {
	if (underline_right == FALSE)
		f->underline_right = FALSE;
	else
		f->underline_right = TRUE;
}


int alfont_get_font_underline_right(ALFONT_FONT * f) {
	return f->underline_right;
}


void alfont_set_font_underline_left(ALFONT_FONT * f, int underline_left) {
	if (underline_left == FALSE)
		f->underline_left = FALSE;
	else
		f->underline_left = TRUE;
}


int alfont_get_font_underline_left(ALFONT_FONT * f) {
	return f->underline_left;
}


void alfont_set_font_background(ALFONT_FONT * f, int background) {
	if (background == FALSE)
		f->background = FALSE;
	else
		f->background = TRUE;
}


int alfont_get_font_background(ALFONT_FONT * f) {
	return f->background;
}


void alfont_set_font_transparency(ALFONT_FONT * f, int transparency) {
	f->transparency = transparency;
}


int alfont_get_font_transparency(ALFONT_FONT * f) {
	return f->transparency;
}


void alfont_set_autofix(ALFONT_FONT * f, int autofix) {
	if (autofix == FALSE)
		f->autofix = FALSE;
	else
		f->autofix = TRUE;
}


int alfont_get_autofix(ALFONT_FONT * f) {
	return f->autofix;
}


void alfont_set_precedingchar(ALFONT_FONT * f, int precedingchar) {
	f->precedingchar = precedingchar;
}


int alfont_get_precedingchar(ALFONT_FONT * f) {
	return f->precedingchar;
}

ALFONT_DLL_DECLSPEC void alfont_set_font_fixed_width(ALFONT_FONT * f, int fixed_width) {
	if (fixed_width == FALSE)
		f->fixed_width = FALSE;
	else
		f->fixed_width = TRUE;
	_alfont_uncache_glyphs(f);
}

} // namespace AGS3

#else

#include "common/textconsole.h"
#include "ags/lib/allegro/gfx.h"

namespace AGS3 {

struct ALFONT_FONT {
	int dummy;
};

const char *alfont_get_name(ALFONT_FONT *f) { return nullptr; }
int alfont_init(void) {	return 0; }
void alfont_exit(void) {}

ALFONT_FONT *alfont_load_font(const char *filepathname) {
	error("Game needs FreeType library, which was not included in this build");
}

ALFONT_FONT *alfont_load_font_from_mem(const char *data, int data_len) {
	return alfont_load_font(nullptr);
}

void alfont_destroy_font(ALFONT_FONT *f) {}
int alfont_set_font_size(ALFONT_FONT *f, int h) { return 0; }
int alfont_set_font_size_ex(ALFONT_FONT *f, int h, int flags) { return 0; }
int alfont_get_font_height(ALFONT_FONT *f) { return 0; }
int alfont_get_font_real_height(ALFONT_FONT *f) { return 0; }
void alfont_get_font_real_vextent(ALFONT_FONT *f, int *top, int *bottom) { *top = 0; *bottom = 0; }
int alfont_text_mode(int mode) { return 0; }
void alfont_textout_aa(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color) {}
void alfont_textout(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color) {}
void alfont_textout_aa_ex(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color, int bg) {}
void alfont_textout_ex(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color, int bg) {}
void alfont_textout_centre_aa(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color) {}
void alfont_textout_centre(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color) {}
void alfont_textout_centre_aa_ex(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color, int bg) {}
void alfont_textout_centre_ex(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color, int bg) {}
void alfont_textout_right_aa(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color);
void alfont_textout_right(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color) {}
void alfont_textout_right_aa_ex(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color, int bg) {}
void alfont_textout_right_ex(BITMAP *bmp, ALFONT_FONT *f, const char *s, int x, int y, int color, int bg) {}
void alfont_textprintf(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, const char *format, ...) {}
void alfont_textprintf_aa(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, const char *format, ...) {}
void alfont_textprintf_ex(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, int bg, const char *format, ...) {}
void alfont_textprintf_aa_ex(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, int bg, const char *format, ...) {}
void alfont_textprintf_centre(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, const char *format, ...) {}
void alfont_textprintf_centre_aa(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, const char *format, ...) {}
void alfont_textprintf_centre_ex(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, int bg, const char *format, ...) {}
void alfont_textprintf_centre_aa_ex(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, int bg, const char *format, ...) {}
void alfont_textprintf_right(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, const char *format, ...) {}
void alfont_textprintf_right_aa(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, const char *format, ...) {}
void alfont_textprintf_right_ex(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, int bg, const char *format, ...) {}
void alfont_textprintf_right_aa_ex(BITMAP *bmp, ALFONT_FONT *f, int x, int y, int color, int bg, const char *format, ...) {}
int alfont_text_height(ALFONT_FONT *f) { return 0; }
int alfont_text_length(ALFONT_FONT *f, const char *str) { return 0; }
int alfont_is_fixed_font(ALFONT_FONT *f) { return 0; }
int alfont_is_scalable_font(ALFONT_FONT *f) { return 0; }
const int *alfont_get_available_fixed_sizes(ALFONT_FONT *f) { return nullptr; }
int alfont_get_nof_available_fixed_sizes(ALFONT_FONT *f) { return 0; }
int alfont_get_char_extra_spacing(ALFONT_FONT *f) { return 0; }
void alfont_set_char_extra_spacing(ALFONT_FONT *f, int spacing) {}

} // namespace AGS3

#endif
