/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:   
 
    Misc. graphics routines

 Notes: 

   Line drawing code utilizes Bresenham's run-length slice algorithm 
    described in "Michael Abrash's Graphics Programming Black Book", 
    Coriolis Group Books, 1997
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "yslib.h"

#include "reinherit.h"

/*
 * Begin module component
\*--------------------------------------------------------------------------*/
#include "gfx_mod.h"
#include "gfx.h"

namespace Saga {

int GFX_ClearSurface(char *buf, int w, int h, int p) {
	int y;

	for (y = 0; y < h; y++) {
		memset(buf, 0, w);
		buf += p;
	}

	return R_SUCCESS;
}

int GFX_ClearSurface16(char *buf, int w, int h, int p) {
	int y;
	w <<= 1;

	for (y = 0; y < h; y++) {
		memset(buf, 0, w);
		buf += p;
	}

	return R_SUCCESS;
}

int GFX_DrawPalette(R_SURFACE *dst_s) {
	int x;
	int y;

	int color = 0;

	R_RECT pal_rect;

	for (y = 0; y < 16; y++) {
		pal_rect.top = (y * 8) + 4;
		pal_rect.bottom = pal_rect.top + 8;

		for (x = 0; x < 16; x++) {
			pal_rect.left = (x * 8) + 4;
			pal_rect.right = pal_rect.left + 8;

			GFX_DrawRect(dst_s, &pal_rect, color);
			color++;
		}
	}

	return 0;
}

int GFX_SimpleBlit(R_SURFACE *dst_s, R_SURFACE *src_s) {
	byte *src_p;
	byte *dst_p;
	int y, w, p;

	assert((dst_s != NULL) && (src_s != NULL));
	assert(dst_s->buf_w == src_s->buf_w);
	assert(dst_s->buf_h == src_s->buf_h);

	src_p = src_s->buf;
	dst_p = dst_s->buf;

	w = src_s->buf_w * (dst_s->bpp / 8);
	p = src_s->buf_pitch;

	for (y = 0; y < src_s->buf_h; y++) {
		memcpy(dst_p, src_p, w);

		dst_p += p;
		src_p += p;
	}

	return R_SUCCESS;
}

int GFX_Scale2x(R_SURFACE *dst_s, R_SURFACE *src_s) {
	assert((dst_s != NULL) && (src_s != NULL));
	assert((dst_s->bpp == src_s->bpp));

	switch (dst_s->bpp) {
	case 8:
		return GFX_Scale2x8(dst_s, src_s);
		break;

	case 16:
		return GFX_Scale2x16(dst_s, src_s);
		break;

	default:
		break;

	}

	return R_FAILURE;
}

int GFX_Scale2x8(R_SURFACE *dst_s, R_SURFACE *src_s) {
	int y, x;

	int src_skip = src_s->buf_pitch - src_s->buf_w;
	int dst_skip = dst_s->buf_pitch - dst_s->buf_w;

	byte *src_ptr = src_s->buf;
	byte *dst_ptr = dst_s->buf;

	byte *src_row;
	byte *dst_row;

	assert(dst_s->buf_w == (src_s->buf_w * 2));
	assert(dst_s->buf_h == (src_s->buf_h * 2));

	for (y = 0; y < src_s->buf_h; y++) {
		src_row = src_ptr;
		dst_row = dst_ptr;

		for (x = 0; x < src_s->buf_w; x++) {
			*dst_ptr++ = *src_ptr;
			*dst_ptr++ = *src_ptr++;
		}

		dst_ptr += dst_skip;

		memcpy(dst_ptr, dst_row, dst_s->buf_w);

		dst_ptr += dst_s->buf_pitch;
		src_ptr += src_skip;
	}

	return R_SUCCESS;
}

int GFX_Scale2x16(R_SURFACE *dst_s, R_SURFACE *src_s) {
	int y, x;

	int src_skip;
	int dest_skip;

	byte *src_ptr = src_s->buf;
	byte *dest_ptr = dst_s->buf;

	short *src_row;
	short *dest_row;

	assert((dst_s != NULL) && (src_s != NULL));

	src_skip = (src_s->buf_pitch - src_s->buf_w) / sizeof(short);
	dest_skip = (dst_s->buf_pitch - dst_s->buf_w) / sizeof(short);

	for (y = 0; y < src_s->buf_h; y++) {
		src_row = (short *)src_ptr;
		dest_row = (short *)dest_ptr;

		for (x = 0; x < src_s->buf_w; x++) {
			*dest_row++ = *src_row;
			*dest_row++ = *src_row++;
		}

		src_ptr += src_s->buf_pitch;

		memcpy(dest_ptr + dst_s->buf_pitch, dest_ptr,
		    dst_s->buf_w << 1);

		dest_ptr += dst_s->buf_pitch;
		dest_ptr += dst_s->buf_pitch;
	}

	return R_SUCCESS;
}

/*--------------------------------------------------------------------------*\
 * Copies a rectangle from a raw 8 bit pixel buffer to the specified surface.
 * The buffer is of width 'src_w' and height 'src_h'. The rectangle to be 
 * copied is defined by 'src_rect'.  
 * The rectangle is copied to the destination surface at point 'dst_pt'.
 * - If dst_pt is NULL, the buffer is rectangle is copied to the destination 
 *    origin.
 * - If src_rect is NULL, the entire buffer is copied.
 * - The surface must match the logical dimensions of the buffer exactly.
 * - Returns R_FAILURE on error
\*--------------------------------------------------------------------------*/
int GFX_BufToSurface(R_SURFACE *ds, const byte *src, int src_w, int src_h, 
					 R_RECT *src_rect, R_POINT *dst_pt) {
	const byte *read_p;
	byte *write_p;

	int row;

	Common::Rect s;
	int d_x, d_y;

	Common::Rect clip;

	int dst_off_x, dst_off_y;
	int src_off_x, src_off_y;
	int src_draw_w, src_draw_h;

	/* Clamp source rectangle to source buffer */
	if (src_rect != NULL) {
		src_rect->clip(src_w - 1, src_h - 1);

		s = *src_rect;

		if ((s.left >= s.right) || (s.top >= s.bottom)) {
			/* Empty or negative region */
			return R_FAILURE;
		}
	} else {
		s.left = 0;
		s.top = 0;
		s.right = src_w - 1;
		s.bottom = src_h - 1;
	}

	/* Get destination origin and clip rectangle */
	if (dst_pt != NULL) {
		d_x = dst_pt->x;
		d_y = dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	clip = ds->clip_rect;

	if (clip.left == clip.right) {
		clip.left = 0;
		clip.right = ds->buf_w - 1;
	}

	if (clip.top == clip.bottom) {
		clip.top = 0;
		clip.bottom = ds->buf_h - 1;
	}

	/* Clip source rectangle to destination surface */
	dst_off_x = d_x;
	dst_off_y = d_y;
	src_off_x = s.left;
	src_off_y = s.top;
	src_draw_w = (s.right - s.left) + 1;
	src_draw_h = (s.bottom - s.top) + 1;

	/* Clip to left edge */

	if (d_x < clip.left) {
		if (d_x <= (-src_draw_w)) {
			/* dst rect completely off left edge */
			return R_SUCCESS;
		}

		src_off_x += (clip.left - d_x);
		src_draw_w -= (clip.left - d_x);

		dst_off_x = clip.left;
	}

	/* Clip to top edge */

	if (d_y < clip.top) {
		if (d_y >= (-src_draw_h)) {
			/* dst rect completely off top edge */
			return R_SUCCESS;
		}

		src_off_y += (clip.top - d_y);
		src_draw_h -= (clip.top - d_y);

		dst_off_y = clip.top;
	}

	/* Clip to right edge */

	if (d_x > clip.right) {
		/* dst rect completely off right edge */
		return R_SUCCESS;
	}

	if ((d_x + src_draw_w - 1) > clip.right) {
		src_draw_w -= (clip.right - (d_x + src_draw_w - 1));
	}

	/* Clip to bottom edge */

	if (d_x > clip.bottom) {
		/* dst rect completely off bottom edge */
		return R_SUCCESS;
	}

	if ((d_y + src_draw_h - 1) > clip.bottom) {
		src_draw_h -= (clip.bottom - (d_y + src_draw_h - 1));
	}

	/* Transfer buffer data to surface */
	read_p = (src + src_off_x) + (src_w * src_off_y);
	write_p = (ds->buf + dst_off_x) + (ds->buf_pitch * dst_off_y);

	for (row = 0; row < src_draw_h; row++) {
		memcpy(write_p, read_p, src_draw_w);

		write_p += ds->buf_pitch;
		read_p += src_w;
	}

	return R_SUCCESS;
}

int GFX_BufToBuffer(byte *dst_buf, int dst_w, int dst_h, const byte *src,
					int src_w, int src_h, R_RECT *src_rect, R_POINT *dst_pt) {
	const byte *read_p;
	byte *write_p;

	int row;

	Common::Rect s;
	int d_x, d_y;

	Common::Rect clip;

	int dst_off_x, dst_off_y;
	int src_off_x, src_off_y;
	int src_draw_w, src_draw_h;

	/* Clamp source rectangle to source buffer */
	if (src_rect != NULL) {
		src_rect->clip(src_w - 1, src_h - 1);

		s.left = src_rect->left;
		s.top = src_rect->top;
		s.right = src_rect->right;
		s.bottom = src_rect->bottom;

		if ((s.left >= s.right) || (s.top >= s.bottom)) {
			/* Empty or negative region */
			return R_FAILURE;
		}
	} else {
		s.left = 0;
		s.top = 0;
		s.right = src_w - 1;
		s.bottom = src_h - 1;
	}

	/* Get destination origin and clip rectangle */
	if (dst_pt != NULL) {
		d_x = dst_pt->x;
		d_y = dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	clip.left = 0;
	clip.top = 0;
	clip.right = dst_w - 1;
	clip.bottom = dst_h - 1;

	/* Clip source rectangle to destination surface */
	dst_off_x = d_x;
	dst_off_y = d_y;
	src_off_x = s.left;
	src_off_y = s.top;
	src_draw_w = (s.right - s.left) + 1;
	src_draw_h = (s.bottom - s.top) + 1;

	/* Clip to left edge */

	if (d_x < clip.left) {
		if (d_x <= (-src_draw_w)) {
			/* dst rect completely off left edge */
			return R_SUCCESS;
		}

		src_off_x += (clip.left - d_x);
		src_draw_w -= (clip.left - d_x);

		dst_off_x = clip.left;
	}

	/* Clip to top edge */

	if (d_y < clip.top) {
		if (d_y >= (-src_draw_h)) {
			/* dst rect completely off top edge */
			return R_SUCCESS;
		}

		src_off_y += (clip.top - d_y);
		src_draw_h -= (clip.top - d_y);

		dst_off_y = clip.top;
	}

	/* Clip to right edge */

	if (d_x > clip.right) {
		/* dst rect completely off right edge */
		return R_SUCCESS;
	}

	if ((d_x + src_draw_w - 1) > clip.right) {
		src_draw_w -= (clip.right - (d_x + src_draw_w - 1));
	}

	/* Clip to bottom edge */

	if (d_x > clip.bottom) {
		/* dst rect completely off bottom edge */
		return R_SUCCESS;
	}

	if ((d_y + src_draw_h - 1) > clip.bottom) {
		src_draw_h -= (clip.bottom - (d_y + src_draw_h - 1));
	}

	/* Transfer buffer data to surface */
	read_p = (src + src_off_x) + (src_w * src_off_y);
	write_p = (dst_buf + dst_off_x) + (dst_w * dst_off_y);

	for (row = 0; row < src_draw_h; row++) {
		memcpy(write_p, read_p, src_draw_w);

		write_p += dst_w;
		read_p += src_w;
	}

	return R_SUCCESS;
}

int GFX_DrawCursor(R_SURFACE *ds, R_POINT *p1) {
	static byte cursor_img[R_CURSOR_W * R_CURSOR_H] = {
		0,   0,   0,   255, 0,   0,   0,
		0,   0,   0,   255, 0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,
		255, 255, 0,   0,   0,   255, 255,
		0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   255, 0,   0,   0,
		0,   0,   0,   255, 0,   0,   0
	};

	R_CLIPINFO ci;

	byte *src_p, *dst_p;

	int x, y;
	int src_skip, dst_skip;

	R_POINT cur_pt;
	R_RECT cur_rect;

	/* Clamp point to surface */
	cur_pt.x = MAX(p1->x, (int16)0);
	cur_pt.y = MAX(p1->y, (int16)0);

	cur_pt.x = MIN(p1->x, (int16)(ds->buf_w - 1));
	cur_pt.y = MIN(p1->y, (int16)(ds->buf_h - 1));

	cur_pt.x -= R_CURSOR_ORIGIN_X;
	cur_pt.y -= R_CURSOR_ORIGIN_Y;

	/* Clip cursor to surface */

	cur_rect.left = 0;
	cur_rect.top = 0;
	cur_rect.right = R_CURSOR_W - 1;
	cur_rect.bottom = R_CURSOR_H - 1;

	ci.dst_rect = &ds->clip_rect;
	ci.src_rect = &cur_rect;
	ci.dst_pt = &cur_pt;

	GFX_GetClipInfo(&ci);

	src_p = cursor_img + ci.src_draw_x + (ci.src_draw_y * R_CURSOR_W);
	dst_p = ds->buf + ci.dst_draw_x + (ci.dst_draw_y * ds->buf_pitch);

	src_skip = R_CURSOR_W - ci.draw_w;
	dst_skip = ds->buf_pitch - ci.draw_w;

	for (y = 0; y < ci.draw_h; y++) {
		for (x = 0; x < ci.draw_w; x++) {
			if (*src_p != 0) {
				*dst_p = *src_p;
			}

			dst_p++;
			src_p++;
		}

		src_p += src_skip;
		dst_p += dst_skip;
	}

	return R_SUCCESS;

}

/*--------------------------------------------------------------------------*\
 * Fills a rectangle in the surface ds from point 'p1' to point 'p2' using
 * the specified color.
\*--------------------------------------------------------------------------*/
int GFX_DrawRect(R_SURFACE *ds, R_RECT *dst_rect, int color) {
	byte *write_p;

	int w;
	int h;
	int row;

	int left, top, right, bottom;

	if (dst_rect != NULL) {
		dst_rect->clip(ds->buf_w - 1, ds->buf_h - 1);

		left = dst_rect->left;
		top = dst_rect->top;
		right = dst_rect->right;
		bottom = dst_rect->bottom;

		if ((left >= right) || (top >= bottom)) {
			/* Empty or negative region */
			return R_FAILURE;
		}
	} else {
		left = 0;
		top = 0;
		right = ds->buf_w - 1;
		bottom = ds->buf_h - 1;
	}

	w = (right - left) + 1;
	h = (bottom - top) + 1;

	write_p = ds->buf + (ds->buf_pitch * top) + left;

	for (row = 0; row < h; row++) {
		memset(write_p, color, w);
		write_p += ds->buf_pitch;
	}

	return R_SUCCESS;
}

int GFX_DrawFrame(R_SURFACE *ds, R_POINT *p1, R_POINT *p2, int color) {
	int left, top, right, bottom;

	int min_x;
	int max_x;
	int min_y;
	int max_y;

	R_POINT n_p1;		/* 1 .. 2 */
	R_POINT n_p2;		/* .    . */
	R_POINT n_p3;		/* .    . */
	R_POINT n_p4;		/* 4 .. 3 */

	assert((ds != NULL) && (p1 != NULL) && (p2 != NULL));

	left = p1->x;
	top = p1->y;
	right = p2->x;
	bottom = p2->y;

	min_x = MIN(left, right);
	min_y = MIN(top, bottom);
	max_x = MAX(left, right);
	max_y = MAX(top, bottom);

	n_p1.x = min_x;
	n_p1.y = min_y;
	n_p2.x = max_x;
	n_p2.y = min_y;
	n_p3.x = max_x;
	n_p3.y = max_y;
	n_p4.x = min_x;
	n_p4.y = max_y;

	GFX_DrawLine(ds, &n_p1, &n_p2, color);
	GFX_DrawLine(ds, &n_p2, &n_p3, color);
	GFX_DrawLine(ds, &n_p3, &n_p4, color);
	GFX_DrawLine(ds, &n_p4, &n_p1, color);

	return R_SUCCESS;
}

int GFX_DrawPolyLine(R_SURFACE *ds, R_POINT *pts, int pt_ct, int draw_color) {
	R_POINT *first_pt = pts;
	int last_i = 1;
	int i;

	assert((ds != NULL) & (pts != NULL));

	if (pt_ct < 3) {
		return R_FAILURE;
	}

	for (i = 1; i < pt_ct; i++) {

		GFX_DrawLine(ds, &pts[i], &pts[i - 1], draw_color);
		last_i = i;
	}

	GFX_DrawLine(ds, &pts[last_i], first_pt, draw_color);

	return R_SUCCESS;
}

int GFX_GetClipInfo(R_CLIPINFO *clipinfo) {
	Common::Rect s;
	int d_x, d_y;

	Common::Rect clip;

	if (clipinfo == NULL) {
		return R_FAILURE;
	}

	if (clipinfo->dst_pt != NULL) {
		d_x = clipinfo->dst_pt->x;
		d_y = clipinfo->dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	s = *clipinfo->src_rect;

	clip = *clipinfo->dst_rect;

	/* Clip source rectangle to destination surface */
	clipinfo->dst_draw_x = d_x;
	clipinfo->dst_draw_y = d_y;
	clipinfo->src_draw_x = s.left;
	clipinfo->src_draw_y = s.top;
	clipinfo->draw_w = (s.right - s.left) + 1;
	clipinfo->draw_h = (s.bottom - s.top) + 1;

	clipinfo->nodraw = 0;

	/* Clip to left edge */

	if (d_x < clip.left) {
		if (d_x <= -(clipinfo->draw_w)) {
			/* dst rect completely off left edge */
			clipinfo->nodraw = 1;

			return R_SUCCESS;
		}

		clipinfo->src_draw_x += (clip.left - d_x);
		clipinfo->draw_w -= (clip.left - d_x);

		clipinfo->dst_draw_x = clip.left;
	}

	/* Clip to top edge */

	if (d_y < clip.top) {
		if (d_y <= -(clipinfo->draw_h)) {
			/* dst rect completely off top edge */
			clipinfo->nodraw = 1;

			return R_SUCCESS;
		}

		clipinfo->src_draw_y += (clip.top - d_y);
		clipinfo->draw_h -= (clip.top - d_y);

		clipinfo->dst_draw_y = clip.top;
	}

	/* Clip to right edge */

	if (d_x > clip.right) {
		/* dst rect completely off right edge */
		clipinfo->nodraw = 1;

		return R_SUCCESS;
	}

	if ((d_x + clipinfo->draw_w - 1) > clip.right) {
		clipinfo->draw_w += (clip.right - (d_x + clipinfo->draw_w - 1));
	}

	/* Clip to bottom edge */

	if (d_y > clip.bottom) {
		/* dst rect completely off bottom edge */
		clipinfo->nodraw = 1;

		return R_SUCCESS;
	}

	if ((d_y + clipinfo->draw_h - 1) > clip.bottom) {
		clipinfo->draw_h += (clip.bottom - (d_y + clipinfo->draw_h - 1));
	}

	return R_SUCCESS;
}

int GFX_ClipLine(R_SURFACE *ds, const R_POINT *src_p1, const R_POINT *src_p2, 
				 R_POINT *dst_p1, R_POINT *dst_p2) {
	const R_POINT *n_p1;
	const R_POINT *n_p2;

	Common::Rect clip;
	int left, top, right, bottom;
	int dx, dy;

	float m;
	int y_icpt_l, y_icpt_r;

	clip = ds->clip_rect;

	/* Normalize points by x */
	if (src_p1->x < src_p2->x) {
		n_p1 = src_p1;
		n_p2 = src_p2;
	} else {
		n_p1 = src_p2;
		n_p2 = src_p1;
	}

	dst_p1->x = n_p1->x;
	dst_p1->y = n_p1->y;

	dst_p2->x = n_p2->x;
	dst_p2->y = n_p2->y;

	left = n_p1->x;
	top = n_p1->y;

	right = n_p2->x;
	bottom = n_p2->y;

	dx = right - left;
	dy = bottom - top;

	if (left < 0) {
		if (right < 0) {
			/* Line completely off left edge */
			return -1;
		}

		/* Clip to left edge */
		m = ((float)bottom - top) / (right - left);
		y_icpt_l = (int)(top - (left * m) + 0.5f);

		dst_p1->x = 0;
		dst_p1->y = y_icpt_l;
	}

	if (bottom > clip.right) {
		if (left > clip.right) {
			/* Line completely off right edge */
			return -1;
		}

		/* Clip to right edge */
		m = ((float)top - bottom) / (right - left);
		y_icpt_r = (int)(top - ((clip.right - left) * m) + 0.5f);

		dst_p1->y = y_icpt_r;
		dst_p2->x = clip.right;
	}

	return 1;
}

/*--------------------------------------------------------------------------*\
 * Utilizes Bresenham's run-length slice algorithm described in
 *  "Michael Abrash's Graphics Programming Black Book", 
 *  Coriolis Group Books, 1997
 *
 * Performs no clipping
\*--------------------------------------------------------------------------*/
void GFX_DrawLine(R_SURFACE *ds, R_POINT *p1, R_POINT *p2, int color) {
	byte *write_p;

	int clip_result;

	int temp;

	int error_up, error_down;
	int error;

	int x_vector;
	int dx, dy;

	int min_run;
	int init_run;
	int run;
	int end_run;

	R_POINT clip_p1, clip_p2;
	int left, top, right, bottom;
	int i, k;

	clip_result = GFX_ClipLine(ds, p1, p2, &clip_p1, &clip_p2);
	if (clip_result < 0) {
		/* Line not visible */
		return;
	}

	left = clip_p1.x;
	top = clip_p1.y;

	right = clip_p2.x;
	bottom = clip_p2.y;

	if ((left < ds->clip_rect.left) || (right < ds->clip_rect.left) ||
	    (left > ds->clip_rect.right) || (right > ds->clip_rect.right)) {

		return;
	}

	if ((top < ds->clip_rect.top) || (bottom < ds->clip_rect.top) ||
	    (top > ds->clip_rect.bottom) || (bottom > ds->clip_rect.bottom)) {

		return;
	}

	if (top > bottom) {

		temp = top;
		top = bottom;
		bottom = temp;

		temp = left;
		left = right;
		right = temp;
	}

	write_p = ds->buf + (top * ds->buf_pitch) + left;

	dx = right - left;

	if (dx < 0) {
		x_vector = -1;
		dx = -dx;
	} else {
		x_vector = 1;
	}

	dy = bottom - top;

	if (dx == 0) {
		for (i = 0; i <= dy; i++) {
			*write_p = (byte) color;
			write_p += ds->buf_pitch;
		}
		return;
	}
	if (dy == 0) {
		for (i = 0; i <= dx; i++) {
			*write_p = (byte) color;
			write_p += x_vector;
		}
		return;
	}
	if (dx == dy) {
		for (i = 0; i <= dx; i++) {
			*write_p = (byte) color;
			write_p += x_vector + ds->buf_pitch;
		}
		return;
	}

	if (dx >= dy) {

		min_run = dx / dy;
		error_up = (dx % dy) * 2;
		error_down = dy * 2;
		error = (dx % dy) - (dy * 2);
		init_run = (min_run / 2) + 1;
		end_run = init_run;

		if ((error_up == 0) && (min_run & 0x01) == 0) {
			init_run--;
		}

		error += dy;

		/* Horiz. seg */
		for (k = 0; k < init_run; k++) {
			*write_p = (byte) color;
			write_p += x_vector;
		}
		write_p += ds->buf_pitch;
	/**********/

		for (i = 0; i < (dy - 1); i++) {
			run = min_run;
			if ((error += error_up) > 0) {

				run++;
				error -= error_down;
			}

			/* Horiz. seg */
			for (k = 0; k < run; k++) {
				*write_p = (byte) color;
				write_p += x_vector;
			}
			write_p += ds->buf_pitch;
	    /**********/
		}

		/* Horiz. seg */
		for (k = 0; k < end_run; k++) {
			*write_p = (byte) color;
			write_p += x_vector;
		}
		write_p += ds->buf_pitch;
	/**********/
		return;

	} else {

		min_run = dy / dx;

		error_up = (dy % dx) * 2;

		error_down = dx * 2;
		error = (dy % dx) - (dx * 2);

		init_run = (min_run / 2) + 1;
		end_run = init_run;

		if ((error_up == 0) && ((min_run & 0x01) == 0)) {
			init_run--;
		}

		if ((min_run & 0x01) != 0) {
			error += dx;
		}

		/* Vertical seg */
		for (k = 0; k < init_run; k++) {
			*write_p = (byte) color;
			write_p += ds->buf_pitch;
		}
		write_p += x_vector;
	/***********/

		for (i = 0; i < (dx - 1); i++) {
			run = min_run;
			if ((error += error_up) > 0) {
				run++;
				error -= error_down;
			}

			/* Vertical seg */
			for (k = 0; k < run; k++) {
				*write_p = (byte) color;
				write_p += ds->buf_pitch;
			}
			write_p += x_vector;
	    /***********/
		}

		/* Vertical seg */
		for (k = 0; k < end_run; k++) {
			*write_p = (byte) color;
			write_p += ds->buf_pitch;
		}
		write_p += x_vector;
	/***********/
		return;
	}

	return;
}

} // End of namespace Saga
