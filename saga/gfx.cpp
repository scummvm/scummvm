/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

// Misc. graphics routines

// Line drawing code utilizes Bresenham's run-length slice algorithm 
// described in "Michael Abrash's Graphics Programming Black Book", 
// Coriolis Group Books, 1997

#include "saga/saga.h"
#include "saga/gfx.h"
#include "saga/interface.h"

#include "common/system.h"


namespace Saga {

Gfx::Gfx(OSystem *system, int width, int height, GameDetector &detector) : _system(system) {
	SURFACE back_buf;

	_system->beginGFXTransaction();
		_vm->initCommonGFX(detector);
		_system->initSize(width, height);
	_system->endGFXTransaction();

	debug(0, "Init screen %dx%d", width, height);
	// Convert surface data to R surface data
	back_buf.pixels = calloc(1, width * height);
	back_buf.w = width;
	back_buf.h = height;
	back_buf.pitch = width;
	back_buf.bytesPerPixel = 1;

	back_buf.clip_rect.left = 0;
	back_buf.clip_rect.top = 0;
	back_buf.clip_rect.right = width;
	back_buf.clip_rect.bottom = height;

	// Set module data
	_back_buf = back_buf;
	_init = 1;

	// For now, always show the mouse cursor.
	setCursor();
	_system->showMouse(true);
}

/*
~Gfx() {
  free(GfxModule.r_back_buf->pixels);
}
 */

int drawPalette(SURFACE *dst_s) {
	int x;
	int y;
	int color = 0;

	Rect pal_rect;

	for (y = 0; y < 16; y++) {
		pal_rect.top = (y * 8) + 4;
		pal_rect.bottom = pal_rect.top + 8;

		for (x = 0; x < 16; x++) {
			pal_rect.left = (x * 8) + 4;
			pal_rect.right = pal_rect.left + 8;

			drawRect(dst_s, pal_rect, color);
			color++;
		}
	}

	return 0;
}

// TODO: I've fixed at least one clipping bug here, but I have a feeling there
//       are several more. 

// * Copies a rectangle from a raw 8 bit pixel buffer to the specified surface.
// The buffer is of width 'src_w' and height 'src_h'. The rectangle to be 
// copied is defined by 'src_rect'.  
// The rectangle is copied to the destination surface at point 'dst_pt'.
// - If dst_pt is NULL, the buffer is rectangle is copied to the destination 
//    origin.
// - If src_rect is NULL, the entire buffer is copied./
// - The surface must match the logical dimensions of the buffer exactly.
// - Returns FAILURE on error
int bufToSurface(SURFACE *ds, const byte *src, int src_w, int src_h, 
					 Rect *src_rect, Point *dst_pt) {
	const byte *read_p;
	byte *write_p;

	int row;

	Common::Rect s;
	int d_x, d_y;

	Common::Rect clip;

	int dst_off_x, dst_off_y;
	int src_off_x, src_off_y;
	int src_draw_w, src_draw_h;

	// Clamp source rectangle to source buffer
	if (src_rect != NULL) {
		src_rect->clip(src_w, src_h);

		s = *src_rect;
	} else {
		s.left = 0;
		s.top = 0;
		s.right = src_w;
		s.bottom = src_h;
	}

	if (s.width() <= 0 || s.height() <= 0) {
		// Empty or negative region
		return FAILURE;
	}

	// Get destination origin and clip rectangle
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
		clip.right = ds->w;
	}

	if (clip.top == clip.bottom) {
		clip.top = 0;
		clip.bottom = ds->h;
	}

	// Clip source rectangle to destination surface
	dst_off_x = d_x;
	dst_off_y = d_y;
	src_off_x = s.left;
	src_off_y = s.top;
	src_draw_w = s.width();
	src_draw_h = s.height();

	// Clip to left edge

	if (d_x < clip.left) {
		if (d_x <= (-src_draw_w)) {
			// dst rect completely off left edge
			return SUCCESS;
		}

		src_off_x += (clip.left - d_x);
		src_draw_w -= (clip.left - d_x);

		dst_off_x = clip.left;
	}

	// Clip to top edge

	if (d_y < clip.top) {
		if (d_y >= (-src_draw_h)) {
			// dst rect completely off top edge
			return SUCCESS;
		}

		src_off_y += (clip.top - d_y);
		src_draw_h -= (clip.top - d_y);

		dst_off_y = clip.top;
	}

	// Clip to right edge

	if (d_x >= clip.right) {
		// dst rect completely off right edge
		return SUCCESS;
	}

	if ((d_x + src_draw_w) > clip.right) {
		src_draw_w = clip.right - d_x;
	}

	// Clip to bottom edge

	if (d_y > clip.bottom) {
		// dst rect completely off bottom edge
		return SUCCESS;
	}

	if ((d_y + src_draw_h) > clip.bottom) {
		src_draw_h = clip.bottom - d_y;
	}

	// Transfer buffer data to surface
	read_p = (src + src_off_x) + (src_w * src_off_y);
	write_p = ((byte *)ds->pixels + dst_off_x) + (ds->pitch * dst_off_y);

	for (row = 0; row < src_draw_h; row++) {
		memcpy(write_p, read_p, src_draw_w);

		write_p += ds->pitch;
		read_p += src_w;
	}

	return SUCCESS;
}

int bufToBuffer(byte *dst_buf, int dst_w, int dst_h, const byte *src,
					int src_w, int src_h, Rect *src_rect, Point *dst_pt) {
	const byte *read_p;
	byte *write_p;
	int row;

	Common::Rect s;
	int d_x, d_y;
	Common::Rect clip;

	int dst_off_x, dst_off_y;
	int src_off_x, src_off_y;
	int src_draw_w, src_draw_h;

	// Clamp source rectangle to source buffer
	if (src_rect != NULL) {
		src_rect->clip(src_w, src_h);

		s.left = src_rect->left;
		s.top = src_rect->top;
		s.right = src_rect->right;
		s.bottom = src_rect->bottom;
	} else {
		s.left = 0;
		s.top = 0;
		s.right = src_w;
		s.bottom = src_h;
	}

	if (s.width() <= 0 || s.height() <= 0) {
		// Empty or negative region
		return FAILURE;
	}

	// Get destination origin and clip rectangle
	if (dst_pt != NULL) {
		d_x = dst_pt->x;
		d_y = dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	clip.left = 0;
	clip.top = 0;
	clip.right = dst_w;
	clip.bottom = dst_h;

	// Clip source rectangle to destination surface
	dst_off_x = d_x;
	dst_off_y = d_y;
	src_off_x = s.left;
	src_off_y = s.top;
	src_draw_w = s.width();
	src_draw_h = s.height();

	// Clip to left edge

	if (d_x < clip.left) {
		if (d_x <= (-src_draw_w)) {
			// dst rect completely off left edge
			return SUCCESS;
		}

		src_off_x += (clip.left - d_x);
		src_draw_w -= (clip.left - d_x);

		dst_off_x = clip.left;
	}

	// Clip to top edge

	if (d_y < clip.top) {
		if (d_y >= (-src_draw_h)) {
			// dst rect completely off top edge
			return SUCCESS;
		}

		src_off_y += (clip.top - d_y);
		src_draw_h -= (clip.top - d_y);

		dst_off_y = clip.top;
	}

	// Clip to right edge

	if (d_x >= clip.right) {
		// dst rect completely off right edge
		return SUCCESS;
	}

	if ((d_x + src_draw_w) > clip.right) {
		src_draw_w = clip.right - d_x;
	}

	// Clip to bottom edge

	if (d_y >= clip.bottom) {
		// dst rect completely off bottom edge
		return SUCCESS;
	}

	if ((d_y + src_draw_h) > clip.bottom) {
		src_draw_h = clip.bottom - d_y;
	}

	// Transfer buffer data to surface
	read_p = (src + src_off_x) + (src_w * src_off_y);
	write_p = (dst_buf + dst_off_x) + (dst_w * dst_off_y);

	for (row = 0; row < src_draw_h; row++) {
		memcpy(write_p, read_p, src_draw_w);

		write_p += dst_w;
		read_p += src_w;
	}

	return SUCCESS;
}

// Fills a rectangle in the surface ds from point 'p1' to point 'p2' using
// the specified color.
int drawRect(SURFACE *ds, Rect &dst_rect, int color) {
	dst_rect.clip(ds->w, ds->h);

	if (!dst_rect.isValidRect()) {
		// Empty or negative region
		return FAILURE;
	}
	
	ds->fillRect(dst_rect, color);

	return SUCCESS;
}

int drawFrame(SURFACE *ds, const Point *p1, const Point *p2, int color) {
	int min_x;
	int max_x;
	int min_y;
	int max_y;

	assert((ds != NULL) && (p1 != NULL) && (p2 != NULL));
	
	min_x = MIN(p1->x, p2->x);
	max_x = MAX(p1->x, p2->x);
	min_y = MIN(p1->y, p2->y);
	max_y = MAX(p1->y, p2->y);

	ds->frameRect(Common::Rect(min_x, min_y, max_x+1, max_y+1), color);

	return SUCCESS;
}

int drawPolyLine(SURFACE *ds, const Point *pts, int pt_ct, int draw_color) {
	assert((ds != NULL) & (pts != NULL));

	if (pt_ct < 3) {
		return FAILURE;
	}

	for (int i = 1; i < pt_ct; i++)
		ds->drawLine(pts[i].x, pts[i].y, pts[i - 1].x, pts[i - 1].y, draw_color);

	ds->drawLine(pts[pt_ct - 1].x, pts[pt_ct - 1].y, pts[0].x, pts[0].y, draw_color);

	return SUCCESS;
}

int getClipInfo(CLIPINFO *clipinfo) {
	Common::Rect s;
	int d_x, d_y;

	Common::Rect clip;

	if (clipinfo == NULL) {
		return FAILURE;
	}

	if (clipinfo->dst_pt != NULL) {
		d_x = clipinfo->dst_pt->x;
		d_y = clipinfo->dst_pt->y;
	} else {
		d_x = 0;
		d_y = 0;
	}

	// Get the clip rect.

	clip.left = clipinfo->dst_rect->left;
	clip.right = clipinfo->dst_rect->right;
	clip.top = clipinfo->dst_rect->top;
	clip.bottom = clipinfo->dst_rect->bottom;

	// Adjust the rect to draw to its screen coordinates

	s.left = d_x + clipinfo->src_rect->left;
	s.right = d_x + clipinfo->src_rect->right;
	s.top = d_y + clipinfo->src_rect->top;
	s.bottom = d_y + clipinfo->src_rect->bottom;

	s.clip(clip);

	if (s.width() <= 0 || s.height() <= 0) {
		clipinfo->nodraw = 1;
		return SUCCESS;
	}

	clipinfo->nodraw = 0;
	clipinfo->src_draw_x = s.left - clipinfo->src_rect->left - d_x;
	clipinfo->src_draw_y = s.top - clipinfo->src_rect->top - d_y;
	clipinfo->dst_draw_x = s.left;
	clipinfo->dst_draw_y = s.top;
	clipinfo->draw_w = s.width();
	clipinfo->draw_h = s.height();

	return SUCCESS;
}

SURFACE *Gfx::getBackBuffer() {
	return &_back_buf;
}

int Gfx::setPalette(PALENTRY *pal) {
	int i;
	byte *ppal;

	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		ppal[0] = pal[i].red;
		ppal[1] = pal[i].green;
		ppal[2] = pal[i].blue;
		ppal[3] = 0;
	}

	_system->setPalette(_cur_pal, 0, PAL_ENTRIES);

	return SUCCESS;
}

int Gfx::getCurrentPal(PALENTRY *src_pal) {
	int i;
	byte *ppal;

	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		src_pal[i].red = ppal[0];
		src_pal[i].green = ppal[1];
		src_pal[i].blue = ppal[2];
	}

	return SUCCESS;
}

int Gfx::palToBlack(PALENTRY *src_pal, double percent) {
	int i;
	//int fade_max = 255;
	int new_entry;
	byte *ppal;

	double fpercent;

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry 
	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		new_entry = (int)(src_pal[i].red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	_system->setPalette(_cur_pal, 0, PAL_ENTRIES);

	return SUCCESS;
}

int Gfx::blackToPal(PALENTRY *src_pal, double percent) {
	int new_entry;
	double fpercent;
	int color_delta;
	int best_wdelta = 0;
	int best_windex = 0;
	int best_bindex = 0;
	int best_bdelta = 1000;
	byte *ppal;
	int i;

	if (percent > 1.0) {
		percent = 1.0;
	}

	// Exponential fade
	fpercent = percent * percent;

	fpercent = 1.0 - fpercent;

	// Use the correct percentage change per frame for each palette entry
	for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
		new_entry = (int)(src_pal[i].red - src_pal[i].red * fpercent);

		if (new_entry < 0) {
			ppal[0] = 0;
		} else {
			ppal[0] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].green - src_pal[i].green * fpercent);

		if (new_entry < 0) {
			ppal[1] = 0;
		} else {
			ppal[1] = (byte) new_entry;
		}

		new_entry = (int)(src_pal[i].blue - src_pal[i].blue * fpercent);

		if (new_entry < 0) {
			ppal[2] = 0;
		} else {
			ppal[2] = (byte) new_entry;
		}
		ppal[3] = 0;
	}

	// Find the best white and black color indices again
	if (percent >= 1.0) {
		for (i = 0, ppal = _cur_pal; i < PAL_ENTRIES; i++, ppal += 4) {
			color_delta = ppal[0];
			color_delta += ppal[1];
			color_delta += ppal[2];

			if (color_delta < best_bdelta) {
				best_bindex = i;
				best_bdelta = color_delta;
			}

			if (color_delta > best_wdelta) {
				best_windex = i;
				best_wdelta = color_delta;
			}
		}
	}

	_system->setPalette(_cur_pal, 0, PAL_ENTRIES);

	return SUCCESS;
}

void Gfx::showCursor(bool state) {
	updateCursor();
	g_system->showMouse(state);
}

void Gfx::setCursor() {
	// Set up the mouse cursor
	const byte A = kITEColorLightGrey;
	const byte B = kITEColorWhite;

	const byte cursor_img[CURSOR_W * CURSOR_H] = {
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		A, A, A, B, A, A, A,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
		0, 0, 0, A, 0, 0, 0,
	};

	_system->setMouseCursor(cursor_img, CURSOR_W, CURSOR_H, 3, 3, 0);
}

bool hitTestPoly(const Point *points, unsigned int npoints, const Point& test_point) {
	int yflag0;
	int yflag1;
	bool inside_flag = false;
	unsigned int pt;

	const Point *vtx0 = &points[npoints - 1];
	const Point *vtx1 = &points[0];

	yflag0 = (vtx0->y >= test_point.y);
	for (pt = 0; pt < npoints; pt++, vtx1++) {
		yflag1 = (vtx1->y >= test_point.y);
		if (yflag0 != yflag1) {
			if (((vtx1->y - test_point.y) * (vtx0->x - vtx1->x) >=
				(vtx1->x - test_point.x) * (vtx0->y - vtx1->y)) == yflag1) {
				inside_flag = !inside_flag;
			}
		}
		yflag0 = yflag1;
		vtx0 = vtx1;
	}

	return inside_flag;
}

} // End of namespace Saga
