/***************************************************************************
 gfx_support.c Copyright (C) 2000 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* Graphics support functions for drivers and replacements for driver functions
** for use with the graphical state manager
*/

#include "sci/include/gfx_system.h"
#include "sci/include/gfx_tools.h"

#ifdef HAVE_ALPHA_EV6_SUPPORT
int axp_have_mvi = 0;
#endif

int gfx_crossblit_alpha_threshold = 128;

#define DRAWLINE_FUNC _gfx_draw_line_buffer_1
#define PIXELWIDTH 1
#include "gfx_line.cpp"
#undef PIXELWIDTH
#undef DRAWLINE_FUNC

#define DRAWLINE_FUNC _gfx_draw_line_buffer_2
#define PIXELWIDTH 2
#include "gfx_line.cpp"
#undef PIXELWIDTH
#undef DRAWLINE_FUNC

#define DRAWLINE_FUNC _gfx_draw_line_buffer_3
#define PIXELWIDTH 3
#include "gfx_line.cpp"
#undef PIXELWIDTH
#undef DRAWLINE_FUNC

#define DRAWLINE_FUNC _gfx_draw_line_buffer_4
#define PIXELWIDTH 4
#include "gfx_line.cpp"
#undef PIXELWIDTH
#undef DRAWLINE_FUNC

inline void
gfx_draw_line_buffer(byte *buffer, int linewidth, int pixelwidth, point_t start, point_t end, unsigned int color)
{
	switch (pixelwidth) {

	case 1:
		_gfx_draw_line_buffer_1(buffer, linewidth, start, end, color);
		return;

	case 2:
		_gfx_draw_line_buffer_2(buffer, linewidth, start, end, color);
		return;

	case 3:
		_gfx_draw_line_buffer_3(buffer, linewidth, start, end, color);
		return;

	case 4:
		_gfx_draw_line_buffer_4(buffer, linewidth, start, end, color);
		return;

	default:
		GFXERROR("pixelwidth=%d not supported!\n", pixelwidth);
		return;

	}
}




void
gfx_draw_line_pixmap_i(gfx_pixmap_t *pxm, point_t start, point_t end, int color)
{
	gfx_draw_line_buffer(pxm->index_data, pxm->index_xl, 1, start, end, color);
}




void
gfx_draw_box_buffer(byte *buffer, int linewidth, rect_t zone, int color)
{
  byte *dest = buffer + zone.x + (linewidth * zone.y);
  int i;

  if (zone.xl <= 0 || zone.yl <= 0)
    return;

  for (i = 0; i < zone.yl; i++) {
    memset(dest, color, zone.xl);
    dest += linewidth;
  }
}


void
gfx_draw_box_pixmap_i(gfx_pixmap_t *pxm, rect_t box, int color)
{
  gfx_clip_box_basic(&box, pxm->index_xl - 1, pxm->index_yl - 1);

  gfx_draw_box_buffer(pxm->index_data, pxm->index_xl, box, color);
}


/* Import various crossblit functions */
#undef USE_PRIORITY
#undef FUNCTION_NAME
#undef BYTESPP

# define FUNCTION_NAME _gfx_crossblit_8
# define BYTESPP 1
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_16
# define BYTESPP 2
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_24
# define BYTESPP 3
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_32
# define BYTESPP 4
# include "gfx_crossblit.cpp"

#define USE_PRIORITY

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_8_P
# define BYTESPP 1
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_16_P
# define BYTESPP 2
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_24_P
# define BYTESPP 3
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_32_P
# define BYTESPP 4
# include "gfx_crossblit.cpp"

#undef USE_PRIORITY
#undef FUNCTION_NAME
#undef BYTESPP

/* Reverse alpha versions */
#undef USE_PRIORITY
#undef FUNCTION_NAME
#undef BYTESPP
#undef REVERSE_ALPHA

#define REVERSE_ALPHA
# define FUNCTION_NAME _gfx_crossblit_8_RA
# define BYTESPP 1
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_16_RA
# define BYTESPP 2
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_24_RA
# define BYTESPP 3
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_32_RA
# define BYTESPP 4
# include "gfx_crossblit.cpp"

#define USE_PRIORITY

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_8_P_RA
# define BYTESPP 1
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_16_P_RA
# define BYTESPP 2
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_24_P_RA
# define BYTESPP 3
# include "gfx_crossblit.cpp"

# undef FUNCTION_NAME
# undef BYTESPP
# define FUNCTION_NAME _gfx_crossblit_32_P_RA
# define BYTESPP 4
# include "gfx_crossblit.cpp"

#undef USE_PRIORITY
#undef FUNCTION_NAME
#undef BYTESPP
#undef REVERSE_ALPHA

static void (*crossblit_fns[5])(byte *, byte *, int, int, int, int, byte *, int, int, unsigned int, unsigned int) =
{ NULL,
  _gfx_crossblit_8,
  _gfx_crossblit_16,
  _gfx_crossblit_24,
  _gfx_crossblit_32 };

static void (*crossblit_fns_P[5])(byte *, byte *, int, int, int, int, byte *, int, int, unsigned int, unsigned int, byte *, int, int, int) =
{ NULL,
  _gfx_crossblit_8_P,
  _gfx_crossblit_16_P,
  _gfx_crossblit_24_P,
  _gfx_crossblit_32_P };

static void (*crossblit_fns_RA[5])(byte *, byte *, int, int, int, int, byte *, int, int, unsigned int, unsigned int) =
{ NULL,
  _gfx_crossblit_8_RA,
  _gfx_crossblit_16_RA,
  _gfx_crossblit_24_RA,
  _gfx_crossblit_32_RA };

static void (*crossblit_fns_P_RA[5])(byte *, byte *, int, int, int, int, byte *, int, int, unsigned int, unsigned int, byte *, int, int, int) =
{ NULL,
  _gfx_crossblit_8_P_RA,
  _gfx_crossblit_16_P_RA,
  _gfx_crossblit_24_P_RA,
  _gfx_crossblit_32_P_RA };
  

void
_gfx_crossblit_simple(byte *dest, byte *src, int dest_line_width, int src_line_width, 
		      int xl, int yl, int bpp)
{
	int line_width = xl * bpp;
	int i;

	for (i = 0; i < yl; i++) {
		memcpy(dest, src, line_width);
		dest += dest_line_width;
		src += src_line_width;
	}
}

int
gfx_crossblit_pixmap(gfx_mode_t *mode, gfx_pixmap_t *pxm, int priority,
		     rect_t src_coords,
		     rect_t dest_coords, byte *dest, int dest_line_width,
		     byte *priority_dest, int priority_line_width,
		     int priority_skip, int flags)
{
	int maxx = 320 * mode->xfact;
	int maxy = 200 * mode->yfact;
	byte *src = pxm->data;
	byte *alpha = pxm->alpha_map? pxm->alpha_map : pxm->data;
	byte *priority_pos = priority_dest;
	unsigned int alpha_mask, alpha_min;
	int bpp = mode->bytespp;
	int bytes_per_alpha_pixel = pxm->alpha_map? 1 : bpp;
	int bytes_per_alpha_line =  bytes_per_alpha_pixel * pxm->xl;
	int xl = pxm->xl, yl = pxm->yl;
	int xoffset = (dest_coords.x < 0)? - dest_coords.x : 0;
	int yoffset = (dest_coords.y < 0)? - dest_coords.y : 0;
	int revalpha = mode->flags & GFX_MODE_FLAG_REVERSE_ALPHA;

	if (src_coords.x + src_coords.xl > xl)
		src_coords.xl = xl - src_coords.x;

	if (src_coords.y + src_coords.yl > yl)
		src_coords.yl = yl - src_coords.y;

/** --???-- **/
	if (src_coords.y > yl)
		return GFX_OK;
	if (src_coords.x > xl)
		return GFX_OK;
/** --???-- **/

	if (dest_coords.x + xl >= maxx)
		xl = maxx - dest_coords.x;
	if (dest_coords.y + yl >= maxy)
		yl = maxy - dest_coords.y;

	xl -= xoffset;
	yl -= yoffset;

	if (!pxm->data)
		return GFX_ERROR;

	if (xl <= 0 || yl <= 0)
		return GFX_OK;

	/* Set destination offsets */

	/* Set x offsets */
        if (!(flags & GFX_CROSSBLIT_FLAG_DATA_IS_HOMED))
	        dest += dest_coords.x * bpp;
	priority_pos += dest_coords.x * priority_skip;

	/* Set y offsets */
        if (!(flags & GFX_CROSSBLIT_FLAG_DATA_IS_HOMED))
	        dest += dest_coords.y * dest_line_width;
	priority_pos += dest_coords.y * priority_line_width;

	/* Set source offsets */
	if (xoffset += src_coords.x) {
		dest_coords.x = 0;
                src += xoffset * bpp;
		alpha += xoffset * bytes_per_alpha_pixel;
	}


	if (yoffset += src_coords.y) {
		dest_coords.y = 0;
                src += yoffset * bpp * pxm->xl;
		alpha += yoffset * bytes_per_alpha_line;
	}

	/* Adjust length for clip box */
	if (xl > src_coords.xl)
		xl = src_coords.xl;
	if (yl > src_coords.yl)
		yl = src_coords.yl;

	/* now calculate alpha */
	if (pxm->alpha_map)
		alpha_mask = 0xff;
	else {
		int shift_nr = 0;

		alpha_mask = mode->alpha_mask;
		if (!alpha_mask && pxm->alpha_map) {
			GFXERROR("Invalid alpha mode: both pxm->alpha_map and alpha_mask are white!\n");
			return GFX_ERROR;
		}

		if (alpha_mask) {
			while (!(alpha_mask & 0xff)) {
				alpha_mask >>= 8;
				shift_nr++;
			}
			alpha_mask &= 0xff;
		}

#ifdef WORDS_BIGENDIAN
		alpha += (mode->bytespp) - (shift_nr + 1);
#else
		alpha += shift_nr;
#endif
	}

#ifdef HAVE_ALPHA_EV6_SUPPORT
	if (mode->alpha_mask && axp_have_mvi && bpp == 4) {
		if (priority == GFX_NO_PRIORITY)
			alpha_mvi_crossblit_32(dest, src, dest_line_width, pxm->xl * bpp,
					       xl, yl, NULL, 0, 0, mode->alpha_mask, 24 - mode->alpha_shift);
		else
			alpha_mvi_crossblit_32_P(dest, src, dest_line_width, pxm->xl * bpp,
						 xl, yl, NULL, 0, 0, mode->alpha_mask, 24 - mode->alpha_shift,
						 priority_pos, priority_line_width, priority_skip, priority);
	} else {
#endif

		if (alpha_mask & 0xff)
			alpha_min = ((alpha_mask * gfx_crossblit_alpha_threshold) >> 8) & alpha_mask;
		else
			alpha_min = ((alpha_mask >> 8) * gfx_crossblit_alpha_threshold) & alpha_mask;

		if (revalpha)
			alpha_min = 255 - alpha_min; /* Since we use it for the reverse effect */
		
		if (!alpha_mask)
			_gfx_crossblit_simple(dest, src, dest_line_width, pxm->xl * bpp, 
					      xl, yl, bpp);
		else

		if (priority == GFX_NO_PRIORITY) {
			if (bpp > 0 && bpp < 5)
				((revalpha) ? crossblit_fns_RA : crossblit_fns)[bpp](dest, src, dest_line_width, pxm->xl * bpp, 
										     xl, yl, alpha, bytes_per_alpha_line, bytes_per_alpha_pixel,
										     alpha_mask, alpha_min);
			else {
				GFXERROR("Invalid mode->bytespp: %d\n", mode->bytespp);
				return GFX_ERROR;
			}
		} else { /* priority */
			if (bpp > 0 && bpp < 5)
				((revalpha) ? crossblit_fns_P_RA : crossblit_fns_P)[bpp](dest, src, dest_line_width, pxm->xl * bpp, 
											 xl, yl, alpha, bytes_per_alpha_line, bytes_per_alpha_pixel,
											 alpha_mask, alpha_min, priority_pos,
											 priority_line_width, priority_skip, priority);
			else {
				GFXERROR("Invalid mode->bytespp: %d\n", mode->bytespp);
				return GFX_ERROR;
			}
		}
#ifdef HAVE_ALPHA_EV6_SUPPORT
	}
#endif

	return GFX_OK;
}




