/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/p4_generic_pc.h"

namespace ICB {

// This type is used by General_quad_draw_24_32().
typedef struct { int32 nX, nY; } Span;

// This function is used only inside this file.
static void RawSpriteDraw(uint8 *pSurfaceBitmap, uint32 nPitch, uint32 nSurfaceWidth, uint32 nSurfaceHeight, _pxSprite *pSprite, uint8 *pPalette, int32 nX, int32 nY,
                          uint32 *nTransparencyRef, uint8 nOpacity);

void Draw_horizontal_line(int32 xx, int32 yy, uint32 len, _rgb *pen, uint8 *ad, uint32 pitch) {
	// viewport coordinates
	// 640*480 screen
	// 24 or 32but colour

	uint32 j;

	// check top and bottom
	if (yy < 0)
		return;
	if (yy > 479)
		return;

	// line totaly off left
	if ((int32)(xx + len) < 0)
		return;

	// line totally off right
	if (xx > 639)
		return;

	// clip left
	if (xx < 0) {
		len -= (0 - xx); // 0 - -5 == 5 so len=len-5
		xx = 0;
	}

	// clip right
	if ((xx + len) > 639) {
		len -= ((xx + len) - 639);
	}

	// move to y
	ad += (yy * pitch);

	//	move to x
	ad += (xx * 4);

	for (j = 0; j < len; j++) {
		*(ad++) = pen->blue;
		*(ad++) = pen->green;
		*(ad++) = pen->red;
		ad++;
	}
}

void Draw_vertical_line(int32 xx, int32 yy, uint32 len, _rgb *pen, uint8 *ad, uint32 pitch) {
	// viewport coordinates
	// 640*480 screen
	// 24 or 32but colour

	uint32 j;

	// check left/right
	if (xx < 0)
		return;
	if (xx > 639)
		return;

	// line totaly off top
	if ((int32)(yy + len) < 0)
		return;

	// line totally off bottom
	if (yy > 479)
		return;

	// clip top
	if (yy < 0) {
		len -= (0 - yy); // 0 - -5 == 5 so len=len-5
		yy = 0;
	}

	// clip bottom
	if ((yy + len) > 479) {
		len -= ((yy + len) - 479);
	}

	// move to y
	ad += (yy * pitch);

	// ove to x
	ad += (xx * 4);

	for (j = 0; j < len; j++) {
		*(ad) = pen->blue;
		*(ad + 1) = pen->green;
		*(ad + 2) = pen->red;
		ad += pitch;
	}
}

void Fill_rect(int32 x, int32 y, int32 x2, int32 y2, uint32 pen, int32 /*z*/) {
	// put a filled rectangle on the screen
	// backbuffer must be unlocked

	LRECT blank;

	// completely off screen
	if ((x2 <= 0) || (x >= 640) || (y >= 480) || (y2 <= 0))
		return;

	// clip
	if (x < 0)
		x = 0;

	if (x2 > 640)
		x2 = 640;

	if (y < 0)
		y = 0;

	if (y2 > 480)
		y2 = 480;

	// setup LRECT for bottom line of the screen
	blank.left = x;
	blank.right = x2;
	blank.top = y;
	blank.bottom = y2;

	surface_manager->Blit_fillfx(working_buffer_id, &blank, pen);
}

void General_draw_line_24_32(int16 x0, int16 y0, int16 x1, int16 y1, _rgb *colour, uint8 *myScreenBuffer, uint32 pitch, int32 surface_width, int32 surface_height) {
	// Uses Bressnham's incremental algorithm!
	// we pass a colour
	// we pass the surface base
	// assume 640*480

	int dx, dy;
	int dxmod, dymod;
	int ince, incne;
	int d;
	int x, y;
	int addTo;
	uint32 BYTEWIDE;
	uint32 vram = 4;

	//#define  RENDERWIDE  640
	//#define  RENDERDEEP  480
	BYTEWIDE = pitch;

	// Make sure we're going from left to right
	if (x1 < x0) {
		x = x1;
		x1 = x0;
		x0 = (int16)x;
		y = y1;
		y1 = y0;
		y0 = (int16)y;
	}
	dx = x1 - x0;
	dy = y1 - y0;

	if (dx < 0)
		dxmod = -dx;
	else
		dxmod = dx;

	if (dy < 0)
		dymod = -dy;
	else
		dymod = dy;

	if (dxmod >= dymod) {
		if (dy > 0) {
			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < surface_width) && (y >= 0) && (y < surface_height)) {
				myScreenBuffer[y * BYTEWIDE + (x * vram)] = colour->blue;
				myScreenBuffer[y * BYTEWIDE + (x * vram) + 1] = colour->green;
				myScreenBuffer[y * BYTEWIDE + (x * vram) + 2] = colour->red;
			}

			while (x < x1) {
				if (d <= 0) {
					d += ince;
					x += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < surface_width) && (y >= 0) && (y < surface_height)) {
					myScreenBuffer[y * BYTEWIDE + (x * vram)] = colour->blue;
					myScreenBuffer[y * BYTEWIDE + (x * vram) + 1] = colour->green;
					myScreenBuffer[y * BYTEWIDE + (x * vram) + 2] = colour->red;
				}
			}
		} else {
			addTo = y0;
			y0 = 0;
			y1 = (int16)(y1 - addTo);
			y1 = (int16)-y1;
			dy = y1 - y0;

			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < surface_width) && (addTo - y >= 0) && (addTo - y < surface_height)) {
				// myScreenBuffer[(addTo - y) * surface_width + x] = colour;

				myScreenBuffer[(addTo - y) * BYTEWIDE + (x * vram)] = colour->blue;
				myScreenBuffer[(addTo - y) * BYTEWIDE + ((x * vram) + 1)] = colour->green;
				myScreenBuffer[(addTo - y) * BYTEWIDE + ((x * vram) + 2)] = colour->red;
			}

			while (x < x1) {
				if (d <= 0) {
					d += ince;
					x += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < surface_width) && (addTo - y >= 0) && (addTo - y < surface_height)) {
					// myScreenBuffer[(addTo - y) * surface_width + x] = colour;

					myScreenBuffer[(addTo - y) * BYTEWIDE + (x * vram)] = colour->blue;
					myScreenBuffer[(addTo - y) * BYTEWIDE + ((x * vram) + 1)] = colour->green;
					myScreenBuffer[(addTo - y) * BYTEWIDE + ((x * vram) + 2)] = colour->red;
				}
			}
		}
	} else {
		// OK, y is now going to be the single increment.
		//  Ensure the line is going top to bottom
		if (y1 < y0) {
			x = x1;
			x1 = x0;
			x0 = (int16)x;
			y = y1;
			y1 = y0;
			y0 = (int16)y;
		}
		dx = x1 - x0;
		dy = y1 - y0;

		if (dx > 0) {
			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			if ((x >= 0) && (x < surface_width) && (y >= 0) && (y < surface_height)) {
				myScreenBuffer[y * BYTEWIDE + (x * vram)] = colour->blue;
				myScreenBuffer[y * BYTEWIDE + (x * vram) + 1] = colour->green;
				myScreenBuffer[y * BYTEWIDE + (x * vram) + 2] = colour->red;
			}
			while (y < y1) {
				if (d <= 0) {
					d += ince;
					y += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				if ((x >= 0) && (x < surface_width) && (y >= 0) && (y < surface_height)) {
					myScreenBuffer[y * BYTEWIDE + (x * vram)] = colour->blue;
					myScreenBuffer[y * BYTEWIDE + (x * vram) + 1] = colour->green;
					myScreenBuffer[y * BYTEWIDE + (x * vram) + 2] = colour->red;
				}
			}
		} else {
			addTo = x0;
			x0 = 0;
			x1 = (int16)(x1 - addTo);
			x1 = (int16)-x1;
			dx = x1 - x0;

			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;
			if ((addTo - x >= 0) && (addTo - x < surface_width) && (y >= 0) && (y < surface_height)) {
				// myScreenBuffer[y * surface_width + addTo - x] = colour;

				myScreenBuffer[y * BYTEWIDE + ((addTo - x) * vram)] = colour->blue;
				myScreenBuffer[y * BYTEWIDE + (((addTo - x) * vram) + 1)] = colour->green;
				myScreenBuffer[y * BYTEWIDE + (((addTo - x) * vram) + 2)] = colour->red;
			}

			while (y < y1) {
				if (d <= 0) {
					d += ince;
					y += 1;
				} else {
					d += incne;
					x += 1;
					y += 1;
				}
				if ((addTo - x >= 0) && (addTo - x < surface_width) && (y >= 0) && (y < surface_height)) {
					// myScreenBuffer[y * surface_width + addTo - x] = colour;

					myScreenBuffer[y * BYTEWIDE + ((addTo - x) * vram)] = colour->blue;
					myScreenBuffer[y * BYTEWIDE + (((addTo - x) * vram) + 1)] = colour->green;
					myScreenBuffer[y * BYTEWIDE + (((addTo - x) * vram) + 2)] = colour->red;
				}
			}
		}
	}
}

void AdditiveGouraudLine(int16 x0, int16 y0, _rgb c0, int16 x1, int16 y1, _rgb c1, uint32 surface_id) {
	// Uses Bressnham's incremental algorithm!
	// We have a start colour and an end colour
	int dx, dy;
	int dxmod, dymod;
	int ince, incne;
	int d;
	int x, y;
	int addTo;

	uint8 *surface = (uint8 *)surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);
	int surface_width = surface_manager->Get_width(surface_id);
	int surface_height = surface_manager->Get_height(surface_id);

	// Make sure we're going from left to right
	if (x1 < x0) {
		x = x1;
		x1 = x0;
		x0 = (int16)x;
		y = y1;
		y1 = y0;
		y0 = (int16)y;

		_rgb tC = c1;
		c1 = c0;
		c0 = tC;
	}
	dx = x1 - x0;
	dy = y1 - y0;

	if (dx < 0)
		dxmod = -dx;
	else
		dxmod = dx;

	if (dy < 0)
		dymod = -dy;
	else
		dymod = dy;

	if (dxmod >= dymod) {
		if (dy > 0) {
			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			uint8 r = c0.red;
			uint8 g = c0.green;
			uint8 b = c0.blue;
			int dr = (dx) ? (c1.red - c0.red) / dx : 0;
			int dg = (dx) ? (c1.green - c0.green) / dx : 0;
			int db = (dx) ? (c1.blue - c0.blue) / dx : 0;
			uint32 offset = y * pitch + (x << 2);
			if ((x >= 0) && (x < surface_width) && (y >= 0) && (y < surface_height)) {
				surface[offset] = ((surface[offset] + b) > 255) ? (uint8)255 : (uint8)(surface[offset] + b);
				surface[offset + 1] = ((surface[offset] + g) > 255) ? (uint8)255 : (uint8)(surface[offset + 1] + g);
				surface[offset + 2] = ((surface[offset] + r) > 255) ? (uint8)255 : (uint8)(surface[offset + 2] + r);
			}

			while (x < x1) {
				r = (uint8)(r + dr);
				g = (uint8)(g + dg);
				b = (uint8)(b + db);
				offset += 4;
				x++;

				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					y++;
					offset += pitch;
				}
				if ((x >= 0) && (x < surface_width) && (y >= 0) && (y < surface_height)) {
					surface[offset] = ((surface[offset] + b) > 255) ? (uint8)255 : (uint8)(surface[offset] + b);
					surface[offset + 1] = ((surface[offset] + g) > 255) ? (uint8)255 : (uint8)(surface[offset + 1] + g);
					surface[offset + 2] = ((surface[offset] + r) > 255) ? (uint8)255 : (uint8)(surface[offset + 2] + r);
				}
			}
		} else {
			addTo = y0;
			y0 = 0;
			y1 = (int16)(y1 - addTo);
			y1 = (int16)-y1;
			dy = y1 - y0;

			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			uint8 r = c0.red;
			uint8 g = c0.green;
			uint8 b = c0.blue;
			int dr = (dx) ? (c1.red - c0.red) / dx : 0;
			int dg = (dx) ? (c1.green - c0.green) / dx : 0;
			int db = (dx) ? (c1.blue - c0.blue) / dx : 0;

			uint32 offset = (addTo - y) * pitch + (x << 2);
			if ((x >= 0) && (x < surface_width) && (addTo - y >= 0) && (addTo - y < surface_height)) {
				surface[offset] = ((surface[offset] + b) > 255) ? (uint8)255 : (uint8)(surface[offset] + b);
				surface[offset + 1] = ((surface[offset] + g) > 255) ? (uint8)255 : (uint8)(surface[offset + 1] + g);
				surface[offset + 2] = ((surface[offset] + r) > 255) ? (uint8)255 : (uint8)(surface[offset + 2] + r);
			}

			while (x < x1) {
				r = (uint8)(r + dr);
				g = (uint8)(g + dg);
				b = (uint8)(b + db);

				x++;
				offset += 4;
				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					y++;
					offset -= pitch;
				}
				if ((x >= 0) && (x < surface_width) && (addTo - y >= 0) && (addTo - y < surface_height)) {
					surface[offset] = ((surface[offset] + b) > 255) ? (uint8)255 : (uint8)(surface[offset] + b);
					surface[offset + 1] = ((surface[offset] + g) > 255) ? (uint8)255 : (uint8)(surface[offset + 1] + g);
					surface[offset + 2] = ((surface[offset] + r) > 255) ? (uint8)255 : (uint8)(surface[offset + 2] + r);
				}
			}
		}
	} else {
		// OK, y is now going to be the single increment.
		//  Ensure the line is going top to bottom
		if (y1 < y0) {
			x = x1;
			x1 = x0;
			x0 = (int16)x;
			y = y1;
			y1 = y0;
			y0 = (int16)y;

			_rgb tC = c1;
			c1 = c0;
			c0 = tC;
		}
		dx = x1 - x0;
		dy = y1 - y0;

		if (dx > 0) {
			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;

			uint8 r = c0.red;
			uint8 g = c0.green;
			uint8 b = c0.blue;
			int dr = (dy) ? (c1.red - c0.red) / dy : 0;
			int dg = (dy) ? (c1.green - c0.green) / dy : 0;
			int db = (dy) ? (c1.blue - c0.blue) / dy : 0;

			uint32 offset = y * pitch + (x << 2);

			if ((x >= 0) && (x < surface_width) && (y >= 0) && (y < surface_height)) {
				surface[offset] = ((surface[offset] + b) > 255) ? (uint8)255 : (uint8)(surface[offset] + b);
				surface[offset + 1] = ((surface[offset] + g) > 255) ? (uint8)255 : (uint8)(surface[offset + 1] + g);
				surface[offset + 2] = ((surface[offset] + r) > 255) ? (uint8)255 : (uint8)(surface[offset + 2] + r);
			}
			while (y < y1) {
				r = (uint8)(r + dr);
				g = (uint8)(g + dg);
				b = (uint8)(b + db);

				offset += pitch;
				y++;
				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					x++;
					offset += 4;
				}
				if ((x >= 0) && (x < surface_width) && (y >= 0) && (y < surface_height)) {
					surface[offset] = ((surface[offset] + b) > 255) ? (uint8)255 : (uint8)(surface[offset] + b);
					surface[offset + 1] = ((surface[offset] + g) > 255) ? (uint8)255 : (uint8)(surface[offset + 1] + g);
					surface[offset + 2] = ((surface[offset] + r) > 255) ? (uint8)255 : (uint8)(surface[offset + 2] + r);
				}
			}
		} else {
			addTo = x0;
			x0 = 0;
			x1 = (int16)(x1 - addTo);
			x1 = (int16)-x1;
			dx = x1 - x0;

			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;

			uint8 r = c0.red;
			uint8 g = c0.green;
			uint8 b = c0.blue;
			int dr = (dy) ? (c1.red - c0.red) / dy : 0;
			int dg = (dy) ? (c1.green - c0.green) / dy : 0;
			int db = (dy) ? (c1.blue - c0.blue) / dy : 0;

			uint32 offset = y * pitch + ((addTo - x) << 2);

			if ((addTo - x >= 0) && (addTo - x < surface_width) && (y >= 0) && (y < surface_height)) {
				surface[offset] = ((surface[offset] + b) > 255) ? (uint8)255 : (uint8)(surface[offset] + b);
				surface[offset + 1] = ((surface[offset] + g) > 255) ? (uint8)255 : (uint8)(surface[offset + 1] + g);
				surface[offset + 2] = ((surface[offset] + r) > 255) ? (uint8)255 : (uint8)(surface[offset + 2] + r);
			}

			while (y < y1) {
				r = (uint8)(r + dr);
				g = (uint8)(g + dg);
				b = (uint8)(b + db);

				offset += pitch;
				y++;
				if (d <= 0) {
					d += ince;

				} else {
					d += incne;
					x += 1;
					offset -= 4;
				}
				if ((addTo - x >= 0) && (addTo - x < surface_width) && (y >= 0) && (y < surface_height)) {
					surface[offset] = ((surface[offset] + b) > 255) ? (uint8)255 : (uint8)(surface[offset] + b);
					surface[offset + 1] = ((surface[offset] + g) > 255) ? (uint8)255 : (uint8)(surface[offset + 1] + g);
					surface[offset + 2] = ((surface[offset] + r) > 255) ? (uint8)255 : (uint8)(surface[offset + 2] + r);
				}
			}
		}
	}

	surface_manager->Unlock_surface(surface_id);
}

void BlendedLine(int32 x0, int32 y0, int32 x1, int32 y1, _rgb c, uint32 surface_id) {
	uint8 *surface = (uint8 *)surface_manager->Lock_surface(surface_id);
	uint32 pitch = surface_manager->Get_pitch(surface_id);
	int surface_width = surface_manager->Get_width(surface_id);
	int surface_height = surface_manager->Get_height(surface_id);

	BlendedLine(x0, y0, x1, y1, c, surface_width, surface_height, pitch, surface);

	surface_manager->Unlock_surface(surface_id);
}

void BlendedLine(int32 x0, int32 y0, int32 x1, int32 y1, _rgb c, int surface_width, int surface_height, uint32 pitch, uint8 *surface) {
	// Uses Bressnham's incremental algorithm!
	// We have a start colour and an end colour
	int dx, dy;
	int dxmod, dymod;
	int ince, incne;
	int d;
	int x, y;
	int addTo;

	// Make sure we're going from left to right
	if (x1 < x0) {
		x = x1;
		x1 = x0;
		x0 = (int16)x;
		y = y1;
		y1 = y0;
		y0 = (int16)y;
	}
	dx = x1 - x0;
	dy = y1 - y0;

	if (dx < 0)
		dxmod = -dx;
	else
		dxmod = dx;

	if (dy < 0)
		dymod = -dy;
	else
		dymod = dy;

	if (dxmod >= dymod) {
		if (dy > 0) {
			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			uint8 r = c.red;
			uint8 g = c.green;
			uint8 b = c.blue;
			uint32 offset = y * pitch + (x << 2);

			while (x < 0 || y < 0) {
				offset += 4;
				x++;

				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					y++;
					offset += pitch;
				}
			}

			if (x1 >= surface_width)
				x1 = surface_width - 1;

			while (x < x1) {
				offset += 4;
				x++;

				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					y++;
					offset += pitch;
				}
				if (y < surface_height) {
					surface[offset] = (uint8)((surface[offset] + b) >> 1);
					surface[offset + 1] = (uint8)((surface[offset + 1] + g) >> 1);
					surface[offset + 2] = (uint8)((surface[offset + 2] + r) >> 1);
				}
			}
		} else {
			addTo = y0;
			y0 = 0;
			y1 = (int16)(y1 - addTo);
			y1 = (int16)-y1;
			dy = y1 - y0;

			d = 2 * dy - dx;
			ince = 2 * dy;
			incne = 2 * (dy - dx);
			x = x0;
			y = y0;
			uint8 r = c.red;
			uint8 g = c.green;
			uint8 b = c.blue;

			uint32 offset = (addTo - y) * pitch + (x << 2);

			while ((x < 0) || ((addTo - y) >= surface_height)) {
				x++;
				offset += 4;
				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					y++;
					offset -= pitch;
				}
			}

			if (x1 >= surface_width)
				x1 = surface_width - 1;

			while (x < x1) {
				x++;
				offset += 4;
				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					y++;
					offset -= pitch;
				}
				if ((addTo - y) >= 0) {
					surface[offset] = (uint8)((surface[offset] + b) >> 1);
					surface[offset + 1] = (uint8)((surface[offset + 1] + g) >> 1);
					surface[offset + 2] = (uint8)((surface[offset + 2] + r) >> 1);
				}
			}
		}
	} else {
		// OK, y is now going to be the single increment.
		//  Ensure the line is going top to bottom
		if (y1 < y0) {
			x = x1;
			x1 = x0;
			x0 = (int16)x;
			y = y1;
			y1 = y0;
			y0 = (int16)y;
		}
		dx = x1 - x0;
		dy = y1 - y0;

		if (dx > 0) {
			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;

			uint8 r = c.red;
			uint8 g = c.green;
			uint8 b = c.blue;
			uint32 offset = y * pitch + (x << 2);

			while ((y < 0) || (x < 0)) {
				offset += pitch;
				++y;
				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					++x;
					offset += 4;
				}
			}

			if (y1 >= surface_height)
				y1 = surface_height - 1;

			while (y < y1) {
				offset += pitch;
				++y;
				if (d <= 0) {
					d += ince;
				} else {
					d += incne;
					++x;
					offset += 4;
				}
				if (x < surface_width) {
					surface[offset] = (uint8)((surface[offset] + b) >> 1);
					surface[offset + 1] = (uint8)((surface[offset + 1] + g) >> 1);
					surface[offset + 2] = (uint8)((surface[offset + 2] + r) >> 1);
				}
			}
		} else {
			addTo = x0;
			x0 = 0;
			x1 = (int16)(x1 - addTo);
			x1 = (int16)-x1;
			dx = x1 - x0;

			d = 2 * dx - dy;
			ince = 2 * dx;
			incne = 2 * (dx - dy);
			x = x0;
			y = y0;

			uint8 r = c.red;
			uint8 g = c.green;
			uint8 b = c.blue;
			uint32 offset = y * pitch + ((addTo - x) << 2);

			while ((y < 0) || (addTo - x >= surface_width)) {
				offset += pitch;
				++y;
				if (d <= 0) {
					d += ince;

				} else {
					d += incne;
					++x;
					offset -= 4;
				}
			}

			if (y1 >= surface_height)
				y1 = surface_height - 1;

			while (y < y1) {
				offset += pitch;
				y++;
				if (d <= 0) {
					d += ince;

				} else {
					d += incne;
					x += 1;
					offset -= 4;
				}
				if ((addTo - x) >= 0) {
					surface[offset] = (uint8)((surface[offset] + b) >> 1);
					surface[offset + 1] = (uint8)((surface[offset + 1] + g) >> 1);
					surface[offset + 2] = (uint8)((surface[offset + 2] + r) >> 1);
				}
			}
		}
	}
}

void SpriteFrameDraw(uint8 *pSurfaceBitmap,     // IN:  Pointer to the surface's drawing area.
                     uint32 nPitch,             // IN:  Pitch for the surface.
                     uint32 nSurfaceWidth,      // IN:  Width of the surface.
                     uint32 nSurfaceHeight,     // IN:  Height of the surface.
                     _pxBitmap *pBitmap,        // IN:  Pointer to the bitmap to render.
                     uint32 nFrameNumber,       // IN:  Frame to draw.
                     uint32 *pnTransparencyRef, // IN:  Colour to use for transparency.
                     uint8 nOpacity             // IN:  Opacity 0-255 to draw sprite into surface.
                     ) {
	uint8 *pPalette;
	_pxSprite *pSprite;

	// Get to the sprite frame.
	pSprite = pBitmap->Fetch_item_by_number(nFrameNumber);

	// Get the palette pointer.
	pPalette = pBitmap->Fetch_palette_pointer();

	// Now do the actual drawing.
	RawSpriteDraw(pSurfaceBitmap, nPitch, nSurfaceWidth, nSurfaceHeight, pSprite, pPalette, pSprite->x, pSprite->y, pnTransparencyRef, nOpacity);
}

void SpriteXYFrameDraw(uint8 *pSurfaceBitmap,     // IN:  Pointer to the surface's drawing area.
                       uint32 nPitch,             // IN:  Pitch for the surface.
                       uint32 nSurfaceWidth,      // IN:  Width of the surface.
                       uint32 nSurfaceHeight,     // IN:  Height of the surface.
                       _pxBitmap *pBitmap,        // IN:  Pointer to the bitmap to render.
                       int32 nX,                  // IN:  X-position to draw sprite at (relative to the surface).
                       int32 nY,                  // IN:  Y-position to draw sprite at (relative to the surface).
                       uint32 nFrameNumber,       // IN:  Frame to draw.
                       bool8 bCenter,             // IN:  If true, centre the sprite.
                       uint32 *pnTransparencyRef, // IN:  Colour to use for transparency.
                       uint8 nOpacity             // IN:  Opacity 0-255 to draw sprite into surface.
                       ) {
	uint8 *pPalette;
	_pxSprite *pSprite;

	// Get to the sprite frame.
	pSprite = pBitmap->Fetch_item_by_number(nFrameNumber);

	// Work out a new plotting position if it is to be centred.
	if (bCenter) {
		nX -= (pSprite->width >> 1);
		nY -= (pSprite->height >> 1);
	}

	// Get the palette pointer.
	pPalette = pBitmap->Fetch_palette_pointer();

	// Now do the actual drawing.
	RawSpriteDraw(pSurfaceBitmap, nPitch, nSurfaceWidth, nSurfaceHeight, pSprite, pPalette, nX, nY, pnTransparencyRef, nOpacity);
}

void RawSpriteDraw(uint8 *pSurfaceBitmap,    // IN:  Pointer to the surface's drawing area.
                   uint32 nPitch,            // IN:  Pitch for the surface.
                   uint32 nSurfaceWidth,     // IN:  Width of surface in pixels.
                   uint32 nSurfaceHeight,    // IN:  Height of surface in pixels.
                   _pxSprite *pSprite,       // IN:  Pointer to one frame of a sprite.
                   uint8 *pPalette,          // IN:  Pointer to the palette.
                   int32 nX,                 // IN:  X-position to draw sprite at (relative to the surface).
                   int32 nY,                 // IN:  Y-position to draw sprite at (relative to the surface).
                   uint32 *pnTransparentRef, // IN:  Colour to use for transparency.
                   uint8 nOpacity            // IN:  If true, pixels are blended into the surface.
                   ) {
	uint32 x, y;
	uint32 *pSurfaceRowStart32;
	uint8 *pSurfaceRowStart8;
	uint8 *pSpriteData;
	uint8 *pPaletteEntry;
	int32 nSpriteTop, nSpriteLeft;
	int32 nSurfaceTop, nSurfaceLeft;
	uint32 nNumRowsToRender, nNumColumnsToRender;
	uint32 nSurfaceSkipToNextRow, nSpriteSkipToNextRow;
	uint32 nSpriteRowStart;
	uint8 nExistingB, nExistingG, nExistingR;
	uint8 nNewB, nNewG, nNewR;

	// This used to be variable; now it's fixed.
	const uint32 nBytesPerPixel = 4;

	// If the start position of the sprite is below the bottom row of the surface or its end position
	// is before the first row of the surface, there can be nothing to display.
	if ((nY >= (int32)nSurfaceHeight) || ((nY + (int32)pSprite->height) <= 0))
		return;

	// Now do the same check in the horizontal direction.
	if ((nX >= (int32)nSurfaceWidth) || ((nX + (int32)pSprite->width) <= 0))
		return;

	// First work out what proportion of the height of the sprite we have to copy allowing
	// for clipping it where it overlaps the edges of the surface.
	nNumRowsToRender = pSprite->height;

	if (nY < 0) {
		nSurfaceTop = 0;
		nSpriteTop = abs(nY);
		nNumRowsToRender -= nSpriteTop;
	} else {
		nSurfaceTop = nY;
		nSpriteTop = 0;
	}

	if ((nY + pSprite->height) > nSurfaceHeight)
		nNumRowsToRender -= ((nY + pSprite->height) - nSurfaceHeight);

	// Now do the same in the horizontal direction.
	nNumColumnsToRender = pSprite->width;

	if (nX < 0) {
		nSurfaceLeft = 0;
		nSpriteLeft = abs(nX);
		nNumColumnsToRender -= nSpriteLeft;
	} else {
		nSurfaceLeft = nX;
		nSpriteLeft = 0;
	}

	if ((nX + pSprite->width) > nSurfaceWidth)
		nNumColumnsToRender -= ((nX + pSprite->width) - nSurfaceWidth);

	// Get to the sprite data.
	pSpriteData = pSprite->data;

	// Work out the byte in the surface to start drawing at.
	pSurfaceRowStart32 = (uint32 *)(pSurfaceBitmap + (nSurfaceTop * nPitch) + (nBytesPerPixel * nSurfaceLeft));
	nSurfaceSkipToNextRow = (nPitch - (nBytesPerPixel * nNumColumnsToRender)) >> 2;

	// Work out the pixel to start drawing and how much we need to jump when we get to the end of a row.
	nSpriteRowStart = nSpriteTop * pSprite->width + nSpriteLeft;
	nSpriteSkipToNextRow = pSprite->width - nNumColumnsToRender;

	// Check if transparency support is required.
	if (pnTransparentRef) {
		// Check if pixel blending is required.
		if (nOpacity != 255) {
			for (y = 0; y < nNumRowsToRender; ++y) {
				// Now loop for one row of the image.
				for (x = 0; x < nNumColumnsToRender; ++x) {
					// Get the palette index for this pixel of the sprite.
					pPaletteEntry = pPalette + (pSpriteData[nSpriteRowStart++] << 2);

					// Don't print anything if the pixel is transparent.
					if (*((uint32 *)pPaletteEntry) != *pnTransparentRef) {
						// We are doing pixel merging.
						pSurfaceRowStart8 = (uint8 *)pSurfaceRowStart32;

						// Get the existing RGB in the surface.
						nExistingB = *pSurfaceRowStart8;
						nExistingG = *(pSurfaceRowStart8 + 1);
						nExistingR = *(pSurfaceRowStart8 + 2);

						// Get the new RGB.
						nNewB = *pPaletteEntry;
						nNewG = *(pPaletteEntry + 1);
						nNewR = *(pPaletteEntry + 2);

						// Merge the pixels.
						nNewB = (uint8)((((nNewB - nExistingB) * nOpacity) >> 8) + nExistingB);
						nNewG = (uint8)((((nNewG - nExistingG) * nOpacity) >> 8) + nExistingG);
						nNewR = (uint8)((((nNewR - nExistingR) * nOpacity) >> 8) + nExistingR);

						// Write the new colour back to the surface.
						*pSurfaceRowStart8 = (uint8)nNewB;
						*(pSurfaceRowStart8 + 1) = (uint8)nNewG;
						*(pSurfaceRowStart8 + 2) = (uint8)nNewR;
					}

					++pSurfaceRowStart32;
				}

				// We have completed a row; need to jump to the start of the next one.
				pSurfaceRowStart32 += nSurfaceSkipToNextRow;
				nSpriteRowStart += nSpriteSkipToNextRow;
			}
		} else {
			for (y = 0; y < nNumRowsToRender; ++y) {
				// Now loop for one row of the image.
				for (x = 0; x < nNumColumnsToRender; ++x) {
					// Get the palette index for this pixel of the sprite.
					pPaletteEntry = pPalette + (pSpriteData[nSpriteRowStart++] << 2);

					// Don't print anything if the pixel is transparent.
					if (*((uint32 *)pPaletteEntry) != *pnTransparentRef)
						*pSurfaceRowStart32 = *((uint32 *)pPaletteEntry);

					++pSurfaceRowStart32;
				}

				// We have completed a row; need to jump to the start of the next one.
				pSurfaceRowStart32 += nSurfaceSkipToNextRow;
				nSpriteRowStart += nSpriteSkipToNextRow;
			}
		}
	} else {
		// No transparency support required.  Check if pixel blending is required.
		if (nOpacity != 255) {
			// Pixel blending required.
			for (y = 0; y < nNumRowsToRender; ++y) {
				// Now loop for one row of the image.
				for (x = 0; x < nNumColumnsToRender; ++x) {
					// Get the palette index for this pixel of the sprite.
					pPaletteEntry = pPalette + (pSpriteData[nSpriteRowStart++] << 2);

					// We are doing pixel merging.
					pSurfaceRowStart8 = (uint8 *)pSurfaceRowStart32;

					// Get the existing RGB in the surface.
					nExistingB = *pSurfaceRowStart8;
					nExistingG = *(pSurfaceRowStart8 + 1);
					nExistingR = *(pSurfaceRowStart8 + 2);

					// Get the new RGB.
					nNewB = *pPaletteEntry;
					nNewG = *(pPaletteEntry + 1);
					nNewR = *(pPaletteEntry + 2);

					// Merge the pixels.
					nNewB = (uint8)((((nNewB - nExistingB) * nOpacity) >> 8) + nExistingB);
					nNewG = (uint8)((((nNewG - nExistingG) * nOpacity) >> 8) + nExistingG);
					nNewR = (uint8)((((nNewR - nExistingR) * nOpacity) >> 8) + nExistingR);

					// Write the new colour back to the surface.
					*pSurfaceRowStart8 = (uint8)nNewB;
					*(pSurfaceRowStart8 + 1) = (uint8)nNewG;
					*(pSurfaceRowStart8 + 2) = (uint8)nNewR;

					++pSurfaceRowStart32;
				}

				// We have completed a row; need to jump to the start of the next one.
				pSurfaceRowStart32 += nSurfaceSkipToNextRow;
				nSpriteRowStart += nSpriteSkipToNextRow;
			}
		} else {
			// No pixel blending required - just a straight opaque copy.
			for (y = 0; y < nNumRowsToRender; ++y) {
				// Now loop for one row of the image.
				for (x = 0; x < nNumColumnsToRender; ++x) {
					// Get the palette index for this pixel of the sprite.
					pPaletteEntry = pPalette + (pSpriteData[nSpriteRowStart++] << 2);

					// 32-bit opaque copy is a straight assignment.
					*pSurfaceRowStart32++ = *((uint32 *)pPaletteEntry);
				}

				// We have completed a row; need to jump to the start of the next one.
				pSurfaceRowStart32 += nSurfaceSkipToNextRow;
				nSpriteRowStart += nSpriteSkipToNextRow;
			}
		}
	}
}

LRECT ConvertPxBitmapRectToRECT(const _PxBitmapRect &sBitmapRect) {
	LRECT sRect;

	sRect.left = sBitmapRect.nX;
	sRect.right = (sBitmapRect.nX + sBitmapRect.nWidth) - 1;
	sRect.top = sBitmapRect.nY;
	sRect.bottom = (sBitmapRect.nY + sBitmapRect.nHeight) - 1;

	return sRect;
}

void General_poly_draw_24_32(_point *pVerts, int32 nNumVerts, _rgb sColour, bool8 bFill, uint8 *pSurface, int32 nPitch, int32 /*nSurfaceWidth*/, int32 /*nSurfaceHeight*/) {
	int32 i, j;
	int32 x, y;
	int32 nTopVert, nBottomVert, nLeftVert, nRightVert;
	int32 nNextVert;
	int32 nTopY, nBottomY, nSlope, nSpanTopY, nSpanBottomY;
	int32 nX;
	int32 nCount;
	PXreal fTopY, fBottomY, fSlope, fHeight, fWidth, fPrestep;
	Span pSpans[SCREEN_DEPTH];
	Span *pSpan;
	int32 nBytesPerPixel;
	uint8 *pSurfaceAddress;

	nTopVert = 0;
	nBottomVert = 0;
	nX = 0;

	// Some initialisation.
	nBytesPerPixel = 4;
	fTopY = 999999.0f;
	fBottomY = -999999.0f;

	// Find the top and bottom vertices.
	for (i = 0; i < (int32)nNumVerts; ++i) {
		if (pVerts[i].z < fTopY) {
			fTopY = pVerts[i].z;
			nTopVert = i;
		}

		if (pVerts[i].z > fBottomY) {
			fBottomY = pVerts[i].z;
			nBottomVert = i;
		}
	}

	nTopY = (int32)ceil(fTopY);
	nBottomY = (int32)ceil(fBottomY);

	// Polygon has to have a height to be drawn.
	if (nBottomY == nTopY)
		return;

	// Scan out the left edge.
	pSpan = pSpans;
	nLeftVert = nTopVert;

	do {
		nNextVert = nLeftVert - 1;

		if (nNextVert < 0)
			nNextVert = nNumVerts - 1;

		nSpanTopY = (int32)ceil(pVerts[nLeftVert].z);
		nSpanBottomY = (int32)ceil(pVerts[nNextVert].z);

		if (nSpanTopY < nSpanBottomY) {
			fHeight = pVerts[nNextVert].z - pVerts[nLeftVert].z;
			fWidth = pVerts[nNextVert].x - pVerts[nLeftVert].x;
			fSlope = fWidth / fHeight;
			fPrestep = nSpanTopY - pVerts[nLeftVert].z;
			nX = (int32)((pVerts[nLeftVert].x + (fSlope * fPrestep)) * 65536.0f) + ((1 << 16) - 1);
			nSlope = (int32)(fSlope * 65536.0f);

			for (j = nSpanTopY; j < nSpanBottomY; ++j) {
				pSpan->nX = nX >> 16;
				nX += nSlope;
				++pSpan;
			}
		}

		--nLeftVert;
		if (nLeftVert < 0)
			nLeftVert = nNumVerts - 1;

	} while (nLeftVert != nBottomVert);

	// Scan out the right edge
	pSpan = pSpans;
	nRightVert = nTopVert;

	do {
		nNextVert = (nRightVert + 1) % nNumVerts;

		nSpanTopY = (int32)ceil(pVerts[nRightVert].z);
		nSpanBottomY = (int32)ceil(pVerts[nNextVert].z);

		if (nSpanTopY < nSpanBottomY) {
			fHeight = pVerts[nNextVert].z - pVerts[nRightVert].z;
			fWidth = pVerts[nNextVert].x - pVerts[nRightVert].x;
			fSlope = fWidth / fHeight;
			fPrestep = nSpanTopY - pVerts[nRightVert].z;
			nX = (int32)((pVerts[nRightVert].x + (fSlope * fPrestep)) * 65536.0f) + ((1 << 16) - 1);
			nSlope = (int32)(fSlope * 65536.0f);

			for (j = nSpanTopY; j < nSpanBottomY; ++j) {
				pSpan->nY = nX >> 16;
				nX += nSlope;
				++pSpan;
			}
		}

		nRightVert = (nRightVert + 1) % nNumVerts;

	} while (nRightVert != nBottomVert);

	// Draw the spans
	pSpan = pSpans;

	// How we draw depends on the bitmap.
	// Drawing in 32-bit colour depth.  Fill or just outline?
	if (bFill) {
		// Fill the polygon (32-bit colour).
		for (x = nTopY; x < nBottomY; ++x) {
			// Work out the width of the line at this point in the height.
			nCount = pSpan->nY - pSpan->nX;

			if (nCount > 0) {
				// Work out the start address of the row in our target surface.
				pSurfaceAddress = pSurface + nPitch * x;

				// Loop along the row.
				for (y = 0; y < nCount; ++y) {
					// Look up the actual colour.
					*pSurfaceAddress++ = sColour.red;
					*pSurfaceAddress++ = sColour.green;
					*pSurfaceAddress++ = sColour.blue;
					*pSurfaceAddress = 0x00;
				}
			}

			// Move on to the next span we've stored.
			++pSpan;
		}
	} else {
		// Just draw outline (32-bit colour).
		for (x = nTopY; x < nBottomY; ++x) {
			// Work out the width of the line at this point in the height.
			nCount = pSpan->nY - pSpan->nX;

			if (nCount > 0) {
				// Draw left edge.
				pSurfaceAddress = pSurface + nPitch * x;
				*pSurfaceAddress++ = sColour.red;
				*pSurfaceAddress++ = sColour.green;
				*pSurfaceAddress++ = sColour.blue;
				*pSurfaceAddress = 0x00;

				// Draw right edge.
				pSurfaceAddress = (pSurface + nPitch * x) + ((nCount - 1) * nBytesPerPixel);
				*pSurfaceAddress++ = sColour.red;
				*pSurfaceAddress++ = sColour.green;
				*pSurfaceAddress++ = sColour.blue;
				*pSurfaceAddress = 0x00;
			}

			// Move on to the next span we've stored.
			++pSpan;
		}
	}
}

} // End of namespace ICB
