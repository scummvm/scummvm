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

int GFX_ClearSurface(char *buf, int w, int h, int p)
{

	int y;

	for (y = 0; y < h; y++) {
		memset(buf, 0, w);
		buf += p;
	}

	return R_SUCCESS;
}

int GFX_ClearSurface16(char *buf, int w, int h, int p)
{

	int y;
	w <<= 1;

	for (y = 0; y < h; y++) {
		memset(buf, 0, w);
		buf += p;
	}

	return R_SUCCESS;
}

int GFX_DrawPalette(R_SURFACE * dst_s)
{
	int x;
	int y;

	int color = 0;

	R_RECT pal_rect;

	for (y = 0; y < 16; y++) {

		pal_rect.y1 = (y * 8) + 4;
		pal_rect.y2 = pal_rect.y1 + 8;

		for (x = 0; x < 16; x++) {

			pal_rect.x1 = (x * 8) + 4;
			pal_rect.x2 = pal_rect.x1 + 8;

			GFX_DrawRect(dst_s, &pal_rect, color);
			color++;
		}
	}

	return 0;
}

int GFX_SimpleBlit(R_SURFACE * dst_s, R_SURFACE * src_s)
{
	uchar *src_p;
	uchar *dst_p;
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

int GFX_Scale2x(R_SURFACE * dst_s, R_SURFACE * src_s)
{

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

int GFX_Scale2x8(R_SURFACE * dst_s, R_SURFACE * src_s)
{

	int y, x;

	int src_skip = src_s->buf_pitch - src_s->buf_w;
	int dst_skip = dst_s->buf_pitch - dst_s->buf_w;

	uchar *src_ptr = src_s->buf;
	uchar *dst_ptr = dst_s->buf;

	uchar *src_row;
	uchar *dst_row;

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

int GFX_Scale2x16(R_SURFACE * dst_s, R_SURFACE * src_s)
{
	int y, x;

	int src_skip;
	int dest_skip;

	uchar *src_ptr = src_s->buf;
	uchar *dest_ptr = dst_s->buf;

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

int
GFX_BufToSurface(R_SURFACE * ds,
    const uchar * src,
    int src_w, int src_h, R_RECT * src_rect, R_POINT * dst_pt)
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
{

	const uchar *read_p;
	uchar *write_p;

	int row;

	int s_x1, s_y1, s_x2, s_y2;
	int d_x, d_y;

	int clip_x1, clip_y1, clip_x2, clip_y2;

	int dst_off_x, dst_off_y;
	int src_off_x, src_off_y;
	int src_draw_w, src_draw_h;

	/* Clamp source rectangle to source buffer
	 * \*------------------------------------------------------------- */
	if (src_rect != NULL) {

		R_CLAMP_RECT(src_rect, 0, (src_w - 1), 0, (src_h - 1));

		s_x1 = src_rect->x1;
		s_y1 = src_rect->y1;
		s_x2 = src_rect->x2;
		s_y2 = src_rect->y2;

		if ((s_x1 >= s_x2) || (s_y1 >= s_y2)) {
			/* Empty or negative region */
			return R_FAILURE;
		}
	} else {
		s_x1 = 0;
		s_y1 = 0;
		s_x2 = src_w - 1;
		s_y2 = src_h - 1;
	}

	/* Get destination origin and clip rectangle
	 * \*------------------------------------------------------------- */
	if (dst_pt != NULL) {
		d_x = dst_pt->x;
		d_y = dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	clip_x1 = ds->clip_rect.x1;
	clip_y1 = ds->clip_rect.y1;
	clip_x2 = ds->clip_rect.x2;
	clip_y2 = ds->clip_rect.y2;

	if (clip_x1 == clip_x2) {
		clip_x1 = 0;
		clip_x2 = ds->buf_w - 1;
	}

	if (clip_y1 == clip_y2) {
		clip_y1 = 0;
		clip_y2 = ds->buf_h - 1;
	}

	/* Clip source rectangle to destination surface
	 * \*------------------------------------------------------------- */
	dst_off_x = d_x;
	dst_off_y = d_y;
	src_off_x = s_x1;
	src_off_y = s_y1;
	src_draw_w = (s_x2 - s_x1) + 1;
	src_draw_h = (s_y2 - s_y1) + 1;

	/* Clip to left edge */

	if (d_x < clip_x1) {
		if (d_x <= (-src_draw_w)) {
			/* dst rect completely off left edge */
			return R_SUCCESS;
		}

		src_off_x += (clip_x1 - d_x);
		src_draw_w -= (clip_x1 - d_x);

		dst_off_x = clip_x1;
	}

	/* Clip to top edge */

	if (d_y < clip_y1) {
		if (d_y >= (-src_draw_h)) {
			/* dst rect completely off top edge */
			return R_SUCCESS;
		}

		src_off_y += (clip_y1 - d_y);
		src_draw_h -= (clip_y1 - d_y);

		dst_off_y = clip_y1;
	}

	/* Clip to right edge */

	if (d_x > clip_x2) {
		/* dst rect completely off right edge */
		return R_SUCCESS;
	}

	if ((d_x + src_draw_w - 1) > clip_x2) {
		src_draw_w -= (clip_x2 - (d_x + src_draw_w - 1));
	}

	/* Clip to bottom edge */

	if (d_x > clip_y2) {
		/* dst rect completely off bottom edge */
		return R_SUCCESS;
	}

	if ((d_y + src_draw_h - 1) > clip_y2) {
		src_draw_h -= (clip_y2 - (d_y + src_draw_h - 1));
	}

	/* Transfer buffer data to surface
	 * \*------------------------------------------------------------- */
	read_p = (src + src_off_x) + (src_w * src_off_y);
	write_p = (ds->buf + dst_off_x) + (ds->buf_pitch * dst_off_y);

	for (row = 0; row < src_draw_h; row++) {

		memcpy(write_p, read_p, src_draw_w);

		write_p += ds->buf_pitch;
		read_p += src_w;
	}

	return R_SUCCESS;
}

int
GFX_BufToBuffer(uchar * dst_buf,
    int dst_w,
    int dst_h,
    const uchar * src,
    int src_w, int src_h, R_RECT * src_rect, R_POINT * dst_pt)
/*--------------------------------------------------------------------------*\
\*--------------------------------------------------------------------------*/
{

	const uchar *read_p;
	uchar *write_p;

	int row;

	int s_x1, s_y1, s_x2, s_y2;
	int d_x, d_y;

	int clip_x1, clip_y1, clip_x2, clip_y2;

	int dst_off_x, dst_off_y;
	int src_off_x, src_off_y;
	int src_draw_w, src_draw_h;

	/* Clamp source rectangle to source buffer
	 * \*------------------------------------------------------------- */
	if (src_rect != NULL) {

		R_CLAMP_RECT(src_rect, 0, (src_w - 1), 0, (src_h - 1));

		s_x1 = src_rect->x1;
		s_y1 = src_rect->y1;
		s_x2 = src_rect->x2;
		s_y2 = src_rect->y2;

		if ((s_x1 >= s_x2) || (s_y1 >= s_y2)) {
			/* Empty or negative region */
			return R_FAILURE;
		}
	} else {
		s_x1 = 0;
		s_y1 = 0;
		s_x2 = src_w - 1;
		s_y2 = src_h - 1;
	}

	/* Get destination origin and clip rectangle
	 * \*------------------------------------------------------------- */
	if (dst_pt != NULL) {
		d_x = dst_pt->x;
		d_y = dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	clip_x1 = 0;
	clip_y1 = 0;
	clip_x2 = dst_w - 1;
	clip_y2 = dst_h - 1;

	/* Clip source rectangle to destination surface
	 * \*------------------------------------------------------------- */
	dst_off_x = d_x;
	dst_off_y = d_y;
	src_off_x = s_x1;
	src_off_y = s_y1;
	src_draw_w = (s_x2 - s_x1) + 1;
	src_draw_h = (s_y2 - s_y1) + 1;

	/* Clip to left edge */

	if (d_x < clip_x1) {
		if (d_x <= (-src_draw_w)) {
			/* dst rect completely off left edge */
			return R_SUCCESS;
		}

		src_off_x += (clip_x1 - d_x);
		src_draw_w -= (clip_x1 - d_x);

		dst_off_x = clip_x1;
	}

	/* Clip to top edge */

	if (d_y < clip_y1) {
		if (d_y >= (-src_draw_h)) {
			/* dst rect completely off top edge */
			return R_SUCCESS;
		}

		src_off_y += (clip_y1 - d_y);
		src_draw_h -= (clip_y1 - d_y);

		dst_off_y = clip_y1;
	}

	/* Clip to right edge */

	if (d_x > clip_x2) {
		/* dst rect completely off right edge */
		return R_SUCCESS;
	}

	if ((d_x + src_draw_w - 1) > clip_x2) {
		src_draw_w -= (clip_x2 - (d_x + src_draw_w - 1));
	}

	/* Clip to bottom edge */

	if (d_x > clip_y2) {
		/* dst rect completely off bottom edge */
		return R_SUCCESS;
	}

	if ((d_y + src_draw_h - 1) > clip_y2) {
		src_draw_h -= (clip_y2 - (d_y + src_draw_h - 1));
	}

	/* Transfer buffer data to surface
	 * \*------------------------------------------------------------- */
	read_p = (src + src_off_x) + (src_w * src_off_y);
	write_p = (dst_buf + dst_off_x) + (dst_w * dst_off_y);

	for (row = 0; row < src_draw_h; row++) {

		memcpy(write_p, read_p, src_draw_w);

		write_p += dst_w;
		read_p += src_w;
	}

	return R_SUCCESS;
}

int GFX_DrawCursor(R_SURFACE * ds, R_POINT * p1)
{

	static uchar cursor_img[R_CURSOR_W * R_CURSOR_H] = {

		0, 0, 0, 255, 0, 0, 0,
		0, 0, 0, 255, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		255, 255, 0, 0, 0, 255, 255,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 255, 0, 0, 0,
		0, 0, 0, 255, 0, 0, 0
	};

	R_CLIPINFO ci;

	uchar *src_p, *dst_p;

	int x, y;
	int src_skip, dst_skip;

	R_POINT cur_pt;
	R_RECT cur_rect;

	/* Clamp point to surface */
	cur_pt.x = YS_MAX(p1->x, 0);
	cur_pt.y = YS_MAX(p1->y, 0);

	cur_pt.x = YS_MIN(p1->x, ds->buf_w - 1);
	cur_pt.y = YS_MIN(p1->y, ds->buf_h - 1);

	cur_pt.x -= R_CURSOR_ORIGIN_X;
	cur_pt.y -= R_CURSOR_ORIGIN_Y;

	/* Clip cursor to surface */

	cur_rect.x1 = 0;
	cur_rect.y1 = 0;
	cur_rect.x2 = R_CURSOR_W - 1;
	cur_rect.y2 = R_CURSOR_H - 1;

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

int GFX_DrawRect(R_SURFACE * ds, R_RECT * dst_rect, int color)
/*--------------------------------------------------------------------------*\
 * Fills a rectangle in the surface ds from point 'p1' to point 'p2' using
 * the specified color.
\*--------------------------------------------------------------------------*/
{
	uchar *write_p;

	int w;
	int h;
	int row;

	int x1, y1, x2, y2;

	if (dst_rect != NULL) {

		R_CLAMP_RECT(dst_rect, 0, (ds->buf_w - 1), 0, (ds->buf_h - 1));

		x1 = dst_rect->x1;
		y1 = dst_rect->y1;
		x2 = dst_rect->x2;
		y2 = dst_rect->y2;

		if ((x1 >= x2) || (y1 >= y2)) {
			/* Empty or negative region */
			return R_FAILURE;
		}
	} else {
		x1 = 0;
		y1 = 0;
		x2 = ds->buf_w - 1;
		y2 = ds->buf_h - 1;
	}

	w = (x2 - x1) + 1;
	h = (y2 - y1) + 1;

	write_p = ds->buf + (ds->buf_pitch * y1) + x1;

	for (row = 0; row < h; row++) {
		memset(write_p, color, w);
		write_p += ds->buf_pitch;
	}

	return R_SUCCESS;
}

int GFX_DrawFrame(R_SURFACE * ds, R_POINT * p1, R_POINT * p2, int color)
{
	int x1, y1, x2, y2;

	int min_x;
	int max_x;
	int min_y;
	int max_y;

	R_POINT n_p1;		/* 1 .. 2 */
	R_POINT n_p2;		/* .    . */
	R_POINT n_p3;		/* .    . */
	R_POINT n_p4;		/* 4 .. 3 */

	assert((ds != NULL) && (p1 != NULL) && (p2 != NULL));

	x1 = p1->x;
	y1 = p1->y;
	x2 = p2->x;
	y2 = p2->y;

	min_x = YS_MIN(x1, x2);
	min_y = YS_MIN(y1, y2);
	max_x = YS_MAX(x1, x2);
	max_y = YS_MAX(y1, y2);

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

int GFX_DrawPolyLine(R_SURFACE * ds, R_POINT * pts, int pt_ct, int draw_color)
{

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

int GFX_GetClipInfo(R_CLIPINFO * clipinfo)
{

	int s_x1, s_y1, s_x2, s_y2;
	int d_x, d_y;

	int clip_x1, clip_y1, clip_x2, clip_y2;

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

	s_x1 = clipinfo->src_rect->x1;
	s_y1 = clipinfo->src_rect->y1;
	s_x2 = clipinfo->src_rect->x2;
	s_y2 = clipinfo->src_rect->y2;

	clip_x1 = clipinfo->dst_rect->x1;
	clip_y1 = clipinfo->dst_rect->y1;
	clip_x2 = clipinfo->dst_rect->x2;
	clip_y2 = clipinfo->dst_rect->y2;

	/* Clip source rectangle to destination surface
	 * \*------------------------------------------------------------- */
	clipinfo->dst_draw_x = d_x;
	clipinfo->dst_draw_y = d_y;
	clipinfo->src_draw_x = s_x1;
	clipinfo->src_draw_y = s_y1;
	clipinfo->draw_w = (s_x2 - s_x1) + 1;
	clipinfo->draw_h = (s_y2 - s_y1) + 1;

	clipinfo->nodraw = 0;

	/* Clip to left edge */

	if (d_x < clip_x1) {
		if (d_x <= -(clipinfo->draw_w)) {
			/* dst rect completely off left edge */
			clipinfo->nodraw = 1;

			return R_SUCCESS;
		}

		clipinfo->src_draw_x += (clip_x1 - d_x);
		clipinfo->draw_w -= (clip_x1 - d_x);

		clipinfo->dst_draw_x = clip_x1;
	}

	/* Clip to top edge */

	if (d_y < clip_y1) {
		if (d_y <= -(clipinfo->draw_h)) {
			/* dst rect completely off top edge */
			clipinfo->nodraw = 1;

			return R_SUCCESS;
		}

		clipinfo->src_draw_y += (clip_y1 - d_y);
		clipinfo->draw_h -= (clip_y1 - d_y);

		clipinfo->dst_draw_y = clip_y1;
	}

	/* Clip to right edge */

	if (d_x > clip_x2) {
		/* dst rect completely off right edge */
		clipinfo->nodraw = 1;

		return R_SUCCESS;
	}

	if ((d_x + clipinfo->draw_w - 1) > clip_x2) {
		clipinfo->draw_w += (clip_x2 - (d_x + clipinfo->draw_w - 1));
	}

	/* Clip to bottom edge */

	if (d_y > clip_y2) {
		/* dst rect completely off bottom edge */
		clipinfo->nodraw = 1;

		return R_SUCCESS;
	}

	if ((d_y + clipinfo->draw_h - 1) > clip_y2) {
		clipinfo->draw_h += (clip_y2 - (d_y + clipinfo->draw_h - 1));
	}

	return R_SUCCESS;
}

int
GFX_ClipLine(R_SURFACE * ds,
    const R_POINT * src_p1,
    const R_POINT * src_p2, R_POINT * dst_p1, R_POINT * dst_p2)
{

	const R_POINT *n_p1;
	const R_POINT *n_p2;

	int clip_x1, clip_y1, clip_x2, clip_y2;
	int x1, y1, x2, y2;
	int dx, dy;

	float m;
	int y_icpt_l, y_icpt_r;

	clip_x1 = ds->clip_rect.x1;
	clip_y1 = ds->clip_rect.y1;
	clip_x2 = ds->clip_rect.x2;
	clip_y2 = ds->clip_rect.y2;

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

	x1 = n_p1->x;
	y1 = n_p1->y;

	x2 = n_p2->x;
	y2 = n_p2->y;

	dx = x2 - x1;
	dy = y2 - y1;

	if (x1 < 0) {

		if (x2 < 0) {
			/* Line completely off left edge */
			return -1;
		}

		/* Clip to left edge */
		m = ((float)y2 - y1) / (x2 - x1);
		y_icpt_l = (int)(y1 - (x1 * m) + 0.5f);

		dst_p1->x = 0;
		dst_p1->y = y_icpt_l;
	}

	if (y2 > clip_x2) {

		if (x1 > clip_x2) {
			/* Line completely off right edge */
			return -1;
		}

		/* Clip to right edge */
		m = ((float)y1 - y2) / (x2 - x1);
		y_icpt_r = (int)(y1 - ((clip_x2 - x1) * m) + 0.5f);

		dst_p1->y = y_icpt_r;
		dst_p2->x = clip_x2;
	}

	return 1;
}

void GFX_DrawLine(R_SURFACE * ds, R_POINT * p1, R_POINT * p2, int color)
/*--------------------------------------------------------------------------*\
 * Utilizes Bresenham's run-length slice algorithm described in
 *  "Michael Abrash's Graphics Programming Black Book", 
 *  Coriolis Group Books, 1997
 *
 * Performs no clipping
\*--------------------------------------------------------------------------*/
{

	uchar *write_p;

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
	int x1, y1, x2, y2;
	int i, k;

	clip_result = GFX_ClipLine(ds, p1, p2, &clip_p1, &clip_p2);
	if (clip_result < 0) {
		/* Line not visible */
		return;
	}

	x1 = clip_p1.x;
	y1 = clip_p1.y;

	x2 = clip_p2.x;
	y2 = clip_p2.y;

	if ((x1 < ds->clip_rect.x1) || (x2 < ds->clip_rect.x1) ||
	    (x1 > ds->clip_rect.x2) || (x2 > ds->clip_rect.x2)) {

		return;
	}

	if ((y1 < ds->clip_rect.y1) || (y2 < ds->clip_rect.y1) ||
	    (y1 > ds->clip_rect.y2) || (y2 > ds->clip_rect.y2)) {

		return;
	}

	if (y1 > y2) {

		temp = y1;
		y1 = y2;
		y2 = temp;

		temp = x1;
		x1 = x2;
		x2 = temp;
	}

	write_p = ds->buf + (y1 * ds->buf_pitch) + x1;

	dx = x2 - x1;

	if (dx < 0) {
		x_vector = -1;
		dx = -dx;
	} else {
		x_vector = 1;
	}

	dy = y2 - y1;

	if (dx == 0) {
		for (i = 0; i <= dy; i++) {
			*write_p = (uchar) color;
			write_p += ds->buf_pitch;
		}
		return;
	}
	if (dy == 0) {
		for (i = 0; i <= dx; i++) {
			*write_p = (uchar) color;
			write_p += x_vector;
		}
		return;
	}
	if (dx == dy) {
		for (i = 0; i <= dx; i++) {
			*write_p = (uchar) color;
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
			*write_p = (uchar) color;
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
				*write_p = (uchar) color;
				write_p += x_vector;
			}
			write_p += ds->buf_pitch;
	    /**********/
		}

		/* Horiz. seg */
		for (k = 0; k < end_run; k++) {
			*write_p = (uchar) color;
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
			*write_p = (uchar) color;
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
				*write_p = (uchar) color;
				write_p += ds->buf_pitch;
			}
			write_p += x_vector;
	    /***********/
		}

		/* Vertical seg */
		for (k = 0; k < end_run; k++) {
			*write_p = (uchar) color;
			write_p += ds->buf_pitch;
		}
		write_p += x_vector;
	/***********/
		return;
	}

	return;
}

} // End of namespace Saga
