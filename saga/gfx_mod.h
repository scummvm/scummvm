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
 
    Graphics manipulation routines - module header file

 Notes: 
*/

#ifndef SAGA_GFX_MOD_H_
#define SAGA_GFX_MOD_H_

namespace Saga {

struct R_CLIPINFO {

	/* input members */
	const R_RECT *src_rect;
	const R_RECT *dst_rect;
	const R_POINT *dst_pt;

	/* output members */
	int nodraw;
	int src_draw_x;
	int src_draw_y;
	int dst_draw_x;
	int dst_draw_y;
	int draw_w;
	int draw_h;

};

int GFX_SimpleBlit(R_SURFACE * dst_s, R_SURFACE * src_s);

int GFX_Scale2x(R_SURFACE * dst_s, R_SURFACE * src_s);

int GFX_Scale2x8(R_SURFACE * dst_s, R_SURFACE * src_s);

int GFX_Scale2x16(R_SURFACE * dst_s, R_SURFACE * src_s);

int GFX_SLScale2x16(char *dest_buf,
    int dest_w,
    int dest_p, int dst_h, char *src_buf, int src_w, int src_p, int src_h);

int GFX_ClearSurface(char *buf, int w, int h, int p);
int GFX_ClearSurface16(char *buf, int w, int h, int p);

int GFX_DrawPalette(R_SURFACE * dst_s);

int GFX_BufToSurface(R_SURFACE * ds,
    const uchar * src,
    int src_w, int src_h, R_RECT * src_rect, R_POINT * dst_pt);

int
GFX_BufToBuffer(uchar * dst_buf,
    int dst_w,
    int dst_h,
    const uchar * src,
    int src_w, int src_h, R_RECT * src_rect, R_POINT * dst_pt);

int GFX_DrawCursor(R_SURFACE * ds, R_POINT * p1);
int GFX_DrawRect(R_SURFACE * ds, R_RECT * dst_rect, int color);
int GFX_DrawFrame(R_SURFACE * ds, R_POINT * p1, R_POINT * p2, int color);
int GFX_DrawPolyLine(R_SURFACE * ds, R_POINT * pts, int pt_ct, int draw_color);

int GFX_GetClipInfo(R_CLIPINFO * clipinfo);

int
GFX_ClipLine(R_SURFACE * ds,
    const R_POINT * src_p1,
    const R_POINT * src_p2, R_POINT * dst_p1, R_POINT * dst_p2);

void GFX_DrawLine(R_SURFACE * ds, R_POINT * p1, R_POINT * p2, int color);

} // End of namespace Saga

#endif				/* R_GFX_MOD_H_ */
/* end "r_gfx_mod.h" */
