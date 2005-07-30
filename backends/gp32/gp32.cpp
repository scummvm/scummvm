/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2004 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

/*
 *
 * Main Source for ScummVM for the GP32
 *
 */

//      TODO:
//      Clean up GP leftovers and strip backend to only bits the GP32 needs.
//      Finish restructure.
//      Sort all remaining GCC 3.4 warnings.
//      setTimerCallback: function call doesnt use * ?

//#define REAL_MAIN

#ifdef GP32_GDB
#include <gdb-stub.h>
#endif				/*GP32_GDB */

#include "backends/gp32/setup.h"

#include "backends/gp32/gp32.h"

#include "backends/gp32/gfx_splash.h"
//#include "backends/gp32/resources/gfx_splash_alt.h"

#define gpRGB16(r,g,b)		(((((r)>>3)&0x1F) << 11) | ((((g)>>3)&0x1F) << 6) | (((b)>>3)&0x1F)<<1)
#define RGB_TO_16(r,g,b)	(((((r)>>3)&0x1F) << 11) | ((((g)>>3)&0x1F) << 6) | (((b)>>3)&0x1F)<<1)

int nflip, keydata;		// Flip Index
GP_HPALETTE PAL;		//palette
GPDRAWSURFACE LCDbuffer[BUFFERCOUNT + 1];	//buffers

void GpSetPaletteEntry(u8 i, u8 r, u8 g, u8 b);

//      FIXME: No global init!

float gammatab[256], gammatab2[256];	// fixme: one table
const float scrGamma[] = { 1.0, 1.001, 1.002, 1.003, 1.004, 1.005 };
char gindex = 3;

int mx = 1, my = 1;
int scrofsy = 239;
char currentsurf;

//      FIXME!!
//      crashes if here and not buildgammatab() not called as very first line! check
//      check if sav file handling overwrites something!
//      float gammatab[256];

FILE *fstderr, *fstdout, *fstdin;

/****************************************************************
    GP32 Input mappings - Returns Button Pressed.
****************************************************************/
int gpTrapKey(void)
{
	int value = 0;

#define rKEY_A          0x4000
#define rKEY_B          0x2000
#define rKEY_L          0x1000
#define rKEY_R          0x8000
#define rKEY_UP         0x0800
#define rKEY_DOWN       0x0200
#define rKEY_LEFT       0x0100
#define rKEY_RIGHT      0x0400
#define rKEY_START      0x0040
#define rKEY_SELECT     0x0080
#define rPBDAT          (*(volatile unsigned *)0x1560000c)
#define rPEDAT          (*(volatile unsigned *)0x15600030)

	unsigned long gpb = rPBDAT;	// 0x156
	unsigned long gpe = rPEDAT;

	if ((gpb & rKEY_LEFT) == 0)
		value |= GPC_VK_LEFT;
	if ((gpb & rKEY_RIGHT) == 0)
		value |= GPC_VK_RIGHT;
	if ((gpb & rKEY_UP) == 0)
		value |= GPC_VK_UP;
	if ((gpb & rKEY_DOWN) == 0)
		value |= GPC_VK_DOWN;
	if ((gpb & rKEY_A) == 0)
		value |= GPC_VK_FA;
	if ((gpb & rKEY_B) == 0)
		value |= GPC_VK_FB;
	if ((gpb & rKEY_L) == 0)
		value |= GPC_VK_FL;
	if ((gpb & rKEY_R) == 0)
		value |= GPC_VK_FR;
	if ((gpe & rKEY_SELECT) == 0)
		value |= GPC_VK_SELECT;
	if ((gpe & rKEY_START) == 0)
		value |= GPC_VK_START;

	return value;
}

/****************************************************************
    GP32 ScummVM OSystem Implementation.
****************************************************************/

//OSystem *OSystem_GP32::create()
//{
//      //OSystem_GP32 *syst = new OSystem_GP32();
//      //return syst;
//      return new OSystem_GP32();
//}

OSystem *OSystem_GP32_create()
{
//
//      OSystem_GP32 *syst = new OSystem_GP32();
////
////    //syst->_mode = gfx_mode;
////    //syst->_full_screen = full_screen;
//
//      // allocate palette storage
//      syst->_currentPalette = (gpColor*)calloc(sizeof(gpColor), 256);
//
//      // allocate the dirty rect storage
//      syst->_mouseBackup = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING * 2);
//      return syst;
//      return new OSystem_GP32();

	//return OSystem_GP32::create();
	return new OSystem_GP32();
}

OSystem_GP32::OSystem_GP32() :
//#ifdef USE_OSD
//      _osdSurface(0), _osdAlpha(SDL_ALPHA_TRANSPARENT), _osdFadeStartTime(0),
//#endif
	_hwscreen(0), _screen(0), _screenWidth(0), _screenHeight(0),
	_tmpscreen(0), _overlayVisible(false),
//      _cdrom(0),
//      _scaler_proc(0),
    _modeChanged(false), _dirty_checksums(0),
	_mouseVisible(false), _mouseDrawn(false), _mouseData(0),
	_mouseHotspotX(0), _mouseHotspotY(0),
	_currentShakePos(0), _newShakePos(0),
	_paletteDirtyStart(0), _paletteDirtyEnd(0), _graphicsMutex(0) {

	// allocate palette storage
	_currentPalette = (gpColor *) calloc(sizeof(gpColor), 256);

	// allocate the dirty rect storage
	_mouseBackup =
	    (byte *) malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING * 2);

	// reset mouse state
	memset(&km, 0, sizeof(km));

	_scaleFactor = 1;
	_scaler_proc = Normal1x;

	_mode = GFX_NORMAL;
	_full_screen = true;
	_adjustAspectRatio = false;
	_mode_flags = 0;

	//init_intern();
}

OSystem_GP32::~OSystem_GP32() {
	if (_dirty_checksums)
		free(_dirty_checksums);
	free(_currentPalette);
	free(_mouseBackup);
	deleteMutex(_graphicsMutex);
	quit();
}

//      Set colors of the palette
void OSystem_GP32::setPalette(const byte * colors, uint start, uint num) {
	const byte *b = colors;
	uint i;
	gpColor *base = _currentPalette + start;
	for (i = 0; i < num; i++) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		b += 4;
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;
}

//      Set the size of the video bitmap.
//      Typically, 320x200 (x240 for FMTowns)

//      FIXME: Is there a clean way to get the Game_ID in the backend, I still like the virual keymap feature below.
//#include "base/gameDetector.h"
//#include "scumm/scumm.h"
//extern ScummEngine *g_scumm;

const char shortkey0[] = "";
const char shortkey1[] = "gpuolscty";	// give, pick up, use, open, look at, push, close, talk to, pull
const char shortkey2[] = "qwerasdfzxcv";	//push, open, walk to, use, pull, close, pick up, turn on, give, look, what is, turn off
const char shortkey3[] = "wlptuo";	// samnmax
const char *shortkey;
int skindex = 0;

void OSystem_GP32::initSize(uint w, uint h, int overlayScale) {
	/*switch (menu[MENU_SCREENPOS].index) {
	 * case 0 : scrofsy = 239 - (240 - h)/2; break;
	 * case 1 : scrofsy = 239; break;
	 * }
	 */
	//switch (g_scumm->_gameId) { //fixme: add all
	//      case GID_TENTACLE : case GID_MONKEY2 : case GID_INDY4 : shortkey=shortkey1; break;
	//      case GID_INDY3 : case GID_ZAK256 : case GID_MONKEY : shortkey=shortkey2; break;
	//      case GID_SAMNMAX : shortkey=shortkey3; break;
	//      default : shortkey=shortkey0; break;
	//}

	shortkey = shortkey0;

	//      Avoid redundant res changes
	if ((int)w == _screenWidth && (int)h == _screenHeight)
		return;

	_screenWidth = w;
	_screenHeight = h;
	CKSUM_NUM = (_screenWidth * _screenHeight / (8 * 8));
	if (_dirty_checksums)
		free(_dirty_checksums);
	_dirty_checksums = (uint32 *) calloc(CKSUM_NUM * 2, sizeof(uint32));

	_mouseData = NULL;
	unload_gfx_mode();
	load_gfx_mode();
	return;
}

void OSystem_GP32::add_dirty_rect(int x, int y, int w, int h) {
	if (_forceFull)
		return;

	if (_num_dirty_rects == NUM_DIRTY_RECT)
		_forceFull = true;
	else {
		gpRect *r = &_dirty_rect_list[_num_dirty_rects++];

		//      Extend the dirty region by 1 pixel for scalers
		//      that "smear" the screen, e.g. 2xSAI
		if (_mode_flags & DF_UPDATE_EXPAND_1_PIXEL) {
			x--;
			y--;
			w += 2;
			h += 2;
		}
		// clip
		if (x < 0) {
			w += x;
			x = 0;
		}
		if (y < 0) {
			h += y;
			y = 0;
		}
		if (w > _screenWidth - x) {
			w = _screenWidth - x;
		}
		if (h > _screenHeight - y) {
			h = _screenHeight - y;
		}

		r->x = x;
		r->y = y;
		r->w = w;
		r->h = h;
	}
}

void OSystem_GP32::mk_checksums(const byte *buf) {
	uint32 *sums = _dirty_checksums;
	uint x, y;
	const uint last_x = (uint) _screenWidth / 8;
	const uint last_y = (uint) _screenHeight / 8;

	const uint BASE = 65521;	/* largest prime smaller than 65536 */

	/* the 8x8 blocks in buf are enumerated starting in the top left corner and
	 * reading each line at a time from left to right */
	for (y = 0; y != last_y; y++, buf += _screenWidth * (8 - 1))
		for (x = 0; x != last_x; x++, buf += 8) {
			// Adler32 checksum algorithm (from RFC1950, used by gzip and zlib).
			// This computes the Adler32 checksum of a 8x8 pixel block. Note
			// that we can do the modulo operation (which is the slowest part)
			// of the algorithm) at the end, instead of doing each iteration,
			// since we only have 64 iterations in total - and thus s1 and
			// s2 can't overflow anyway.
			uint32 s1 = 1;
			uint32 s2 = 0;
			const byte *ptr = buf;
			for (int subY = 0; subY < 8; subY++) {
				for (int subX = 0; subX < 8; subX++) {
					s1 += ptr[subX];
					s2 += s1;
				}
				ptr += _screenWidth;
			}

			s1 %= BASE;
			s2 %= BASE;

			/* output the checksum for this block */
			*sums++ = (s2 << 16) + s1;
		}
}

void OSystem_GP32::add_dirty_rgn_auto(const byte *buf) {
	assert(((uint32) buf & 3) == 0);

	/* generate a table of the checksums */
	mk_checksums(buf);

	if (!cksum_valid) {
		_forceFull = true;
		cksum_valid = true;
	}

	/* go through the checksum list, compare it with the previous checksums,
	 * and add all dirty rectangles to a list. try to combine small rectangles
	 * into bigger ones in a simple way */
	if (!_forceFull) {
		int x, y, w;
		uint32 *ck = _dirty_checksums;

		for (y = 0; y != _screenHeight / 8; y++) {
			for (x = 0; x != _screenWidth / 8; x++, ck++) {
				if (ck[0] != ck[CKSUM_NUM]) {
					/* found a dirty 8x8 block, now go as far to the right as possible,
					 * and at the same time, unmark the dirty status by setting old to new. */
					w = 0;
					do {
						ck[w + CKSUM_NUM] = ck[w];
						w++;
					} while (x + w != _screenWidth / 8
					    && ck[w] != ck[w + CKSUM_NUM]);

					add_dirty_rect(x * 8, y * 8, w * 8, 8);

					if (_forceFull)
						goto get_out;
				}
			}
		}
	} else {
	      get_out:;
		/* Copy old checksums to new */
		memcpy(_dirty_checksums + CKSUM_NUM, _dirty_checksums,
		    CKSUM_NUM * sizeof(uint32));
	}
}

// Draw a bitmap to screen.
// The screen will not be updated to reflect the new bitmap
void OSystem_GP32::copyRectToScreen(const byte *buf, int pitch, int x, int y,
    int w, int h) {
	if (_screen == NULL)
		return;

	if (pitch == _screenWidth && x == 0 && y == 0 && w == _screenWidth
	    && h == _screenHeight && _mode_flags & DF_WANT_RECT_OPTIM) {
		/* Special, optimized case for full screen updates.
		 * It tries to determine what areas were actually changed,
		 * and just updates those, on the actual display. */
		add_dirty_rgn_auto(buf);
	} else {
		/* Clip the coordinates */
		if (x < 0) {
			w += x;
			buf -= x;
			x = 0;
		}
		if (y < 0) {
			h += y;
			buf -= y * pitch;
			y = 0;
		}
		if (w > _screenWidth - x) {
			w = _screenWidth - x;
		}
		if (h > _screenHeight - y) {
			h = _screenHeight - y;
		}

		if (w <= 0 || h <= 0)
			return;

		cksum_valid = false;
		add_dirty_rect(x, y, w, h);
	}

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouseDrawn)
		undraw_mouse();

	byte *dst = (byte *) _screen->pixels + y * _screenWidth + x;
	do {
		memcpy(dst, buf, w);
		dst += _screenWidth;
		buf += pitch;
	} while (--h);
}

gpSurface *gpCreateRGBSurface(Uint32 flags, int width, int height, int depth,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {

	gpSurface *surf = (gpSurface *) malloc(sizeof(gpSurface));
	surf->format = (gpPixelFormat *) malloc(sizeof(gpPixelFormat));

	if ((flags & gpHWSurface) == gpHWSurface) {
		error(">HW surface (w=%d, h=%d)", width, height);
	} else if ((flags & gpSWSurface) == gpSWSurface) {
		int size = width * height * (depth / 8);
		printf(">SW surface (w=%d, h=%d, size=%d, depth=%d)", width,
		    height, size, depth);
		surf->pixels = malloc(size);
	} else {
		error(">unknown surface", width, height);
		return NULL;
	}
	surf->w = width;
	surf->h = height;
	surf->pitch = width * (depth / 8);
	surf->format->BitsPerPixel = depth;
	surf->format->BytesPerPixel = depth / 8;
	return surf;
}

gpSurface *gpSetVideoMode(int width, int height, int bpp, Uint32 flags) {
	return gpCreateRGBSurface(flags, width, height, bpp, 0, 0, 0, 0);
}

void gpFreeSurface(gpSurface *surface) {
	// implement
}

gpSurface *gpCreateRGBSurfaceFrom(void *pixels,
    int width, int height, int depth, int pitch,
    Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {

// FIXME dont reuse code

	gpSurface *surf = (gpSurface *) malloc(sizeof(gpSurface));
	surf->format = (gpPixelFormat *) malloc(sizeof(gpPixelFormat));

	surf->w = width;
	surf->h = height;
	surf->pitch = pitch;
	surf->pixels = pixels;
	surf->format->BitsPerPixel = depth;
	surf->format->BytesPerPixel = depth / 8;
	return surf;
}

int gpFillRect(gpSurface *dst, gpRect *dstrect, Uint32 color) {
	// FIXME: implement
	return 0;
}

int mcshake = 0;

void gpUpdateRects(gpSurface *screen, int numrects, gpRect *rects) {
	// FIXME dont duplicate code :)
	// CHECK: shake causes crash? mcshake can get negative?

	if (screen->format->BitsPerPixel == 8)
		while (numrects--) {
			//if (mcshake && rects->h == LCD_HEIGHT) { //fixme?
			if (mcshake && rects->h == /*_screenHeight*/ 200) {	//fixme?
				//printf("shaking %d", mcshake);
				rects->h -= mcshake;
				GpRectFill(NULL, &LCDbuffer[GAME_SURFACE], 0, rects->h + scrofsy, 320, mcshake, 0);	//black border
			}

			u8 *s =
			    (u8 *) ((u8 *) screen->pixels + (rects->y +
				mcshake) * 320 + rects->x);
			u8 *d =
			    (u8 *) ((u8 *) LCDbuffer[GAME_SURFACE].ptbuffer +
			    rects->x * 240 + scrofsy - rects->y);
			u8 *s2 = s, *d2 = d;

			for (int x = rects->w; x; x--) {
				for (int y = rects->h; y; y--) {
					*d-- = *s;
					s += 320;	// FIXME? screen->pitch;
				}
				d2 += 240;
				d = d2;
				s2++;
				s = s2;
			}
			rects++;
	} else if (screen->format->BitsPerPixel == 16)
		while (numrects--) {
			u16 *s =
			    (u16 *) ((u16 *) screen->pixels + rects->y * 320 +
			    rects->x);
			u16 *d =
			    (u16 *) ((u16 *) LCDbuffer[GAME_SURFACE].ptbuffer +
			    rects->x * 240 + scrofsy - rects->y);
			u16 *s2 = s, *d2 = d;

			for (int x = rects->w; x; x--) {
				for (int y = rects->h; y; y--) {
					*d-- = *s;
					s += 320;	// FIXME? screen->pitch;
				}
				d2 += 240;
				d = d2;
				s2++;
				s = s2;
			}
			rects++;
	} else
		error("blitting surface with wrong depth (%d)",
		    screen->format->BitsPerPixel);
	// eh? works also when rects++ is here??
}

//#define gpBlitSurface gpUpperBlit
int gpBlitSurface(gpSurface *screen, gpRect *rects, gpSurface *dst, gpRect *dstrect) {
	// FIXME? role??
	//gpUpdateRects(screen, 1, rects); //ph0x! _hwscreen
	return 0;
}

int gpSetColors(gpSurface *surface, gpColor *colors, int firstcolor, int ncolors) {
	float rr, gg, bb;
	gpColor colors2[256];

	if (currentsurf == DEBUG_SURFACE)
		return 1;

	for (int i = firstcolor; i < firstcolor + ncolors; i++) {
		rr = colors[i].r * gammatab[colors[i].r];
		gg = colors[i].g * gammatab[colors[i].g];
		bb = colors[i].b * gammatab[colors[i].b];

		if (rr > 255)
			rr = 255;
		if (gg > 255)
			gg = 255;
		if (bb > 255)
			bb = 255;

		colors2[i].r = (u8) rr;
		colors2[i].g = (u8) gg;
		colors2[i].b = (u8) bb;
	}

	GpPaletteEntryChangeEx(firstcolor, ncolors, (GP_LOGPALENTRY *) colors2,
	    0);
	return 1;
}

// Moves the screen content around by the given amount of pixels
// but only the top height pixel rows, the rest stays untouched
//void OSystem_GP32::move_screen(int dx, int dy, int height)
//{
//      if ((dx == 0) && (dy == 0))
//              return;
//
//      if (dx == 0) {
//              // vertical movement
//              if (dy > 0) {
//                      // move down
//                      // copy from bottom to top
//                      for (int y = height - 1; y >= dy; y--)
//                              copyRectToScreen((byte *)_screen->pixels + _screenWidth * (y - dy), _screenWidth, 0, y, _screenWidth, 1);
//              } else {
//                      // move up
//                      // copy from top to bottom
//                      for (int y = 0; y < height + dx; y++)
//                              copyRectToScreen((byte *)_screen->pixels + _screenWidth * (y - dy), _screenWidth, 0, y, _screenWidth, 1);
//              }
//      } else if (dy == 0) {
//              // horizontal movement
//              if (dx > 0) {
//                      // move right
//                      // copy from right to left
//                      for (int x = _screenWidth - 1; x >= dx; x--)
//                              copyRectToScreen((byte *)_screen->pixels + x - dx, _screenWidth, x, 0, 1, height);
//              } else {
//                      // move left
//                      // copy from left to right
//                      for (int x = 0; x < _screenWidth; x++)
//                              copyRectToScreen((byte *)_screen->pixels + x - dx, _screenWidth, x, 0, 1, height);
//              }
//      } else {
//              // free movement
//              // not necessary for now
//      }
//}

int16 OSystem_GP32::get_height() {
	return _screenHeight;
}

int16 OSystem_GP32::get_width() {
	return _screenWidth;
}

//void OSystem_GP32::warpMouse(int, int)
//{
//}

void OSystem_GP32::warpMouse(int x, int y) {
//              set_mouse_pos(x, y);
}

void OSystem_GP32::load_gfx_mode() {

	GpRectFill(NULL, &LCDbuffer[GAME_SURFACE], 0, 0, 320, 240, 0);	//black border

	_forceFull = true;
	_mode_flags = DF_WANT_RECT_OPTIM | DF_UPDATE_EXPAND_1_PIXEL;

	_tmpscreen = NULL;
	TMP_SCREEN_WIDTH = (_screenWidth + 3);

	switch (_mode) {

	case GFX_NORMAL:
		//???????
		//normal_mode:;
		_scaleFactor = 1;
		_scaler_proc = Normal1x;

		break;
	default:
		error("Unknown graphics mode");
		_scaleFactor = 1;
		_scaler_proc = NULL;
	}

	//
	// Create the surface that contains the 8 bit game data
	//
	_screen =
	    gpCreateRGBSurface(gpSWSurface, _screenWidth, _screenHeight, 8, 0,
	    0, 0, 0);
	if (_screen == NULL)
		error("_screen failed");

	//
	// Create the surface that contains the scaled graphics in 16 bit mode
	//
	_hwscreen =
	    gpSetVideoMode(_screenWidth * _scaleFactor,
	    _screenHeight * _scaleFactor, 16,
	    _full_screen ? (gpFullScreen | gpSWSurface) : gpSWSurface);
	if (_hwscreen == NULL)
		error("_hwscreen failed");

	//
	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	//

/*
	// Distinguish 555 and 565 mode
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);
*/
	//InitScalers(555); // ph0x fixme?

	//ph0x fixme - tmpscreen needed?

	// Need some extra bytes around when using 2xSaI
	uint16 *tmp_screen =
	    (uint16 *) calloc(TMP_SCREEN_WIDTH * (_screenHeight + 3),
	    sizeof(uint16));
	_tmpscreen =
	    gpCreateRGBSurfaceFrom(tmp_screen, TMP_SCREEN_WIDTH,
	    _screenHeight + 3, 16, TMP_SCREEN_WIDTH * 2,
	    _hwscreen->format->Rmask, _hwscreen->format->Gmask,
	    _hwscreen->format->Bmask, _hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("_tmpscreen failed");

	// keyboard cursor control, some other better place for it?
	km.x_max = _screenWidth * _scaleFactor - 1;
	km.y_max = _screenHeight * _scaleFactor - 1;
	//km.delay_time = 25;
	km.delay_time = 15;
	km.last_time = 0;

}

void OSystem_GP32::unload_gfx_mode() {
	if (_screen) {
		gpFreeSurface(_screen);
		_screen = NULL;
	}

	if (_hwscreen) {
		gpFreeSurface(_hwscreen);
		_hwscreen = NULL;
	}

	if (_tmpscreen) {
		free((uint16 *) _tmpscreen->pixels);
		gpFreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}
}

#include "common/util.h"

void OSystem_GP32::draw_mouse() {
	if (!_overlayVisible) {

		if (_mouseDrawn || !_mouseVisible)
			return;

		int x = _mouseCurState.x - _mouseHotspotX;
		int y = _mouseCurState.y - _mouseHotspotY;
		int w = _mouseCurState.w;
		int h = _mouseCurState.h;
		byte color;
		byte *src = _mouseData;	// Image representing the mouse
		byte *bak = _mouseBackup;	// Surface used to backup the area obscured by the mouse
		byte *dst;	// Surface we are drawing into

		// clip the mouse rect, and addjust the src pointer accordingly
		if (x < 0) {
			w += x;
			src -= x;
			x = 0;
		}
		if (y < 0) {
			h += y;
			src -= y * _mouseCurState.w;
			y = 0;
		}
		if (w > _screenWidth - x)
			w = _screenWidth - x;
		if (h > _screenHeight - y)
			h = _screenHeight - y;

		// Quick check to see if anything has to be drawn at all
		if (w <= 0 || h <= 0)
			return;

		// Store the bounding box so that undraw mouse can restore the area the
		// mouse currently covers to its original content.
		_mouseOldState.x = x;
		_mouseOldState.y = y;
		_mouseOldState.w = w;
		_mouseOldState.h = h;

		// Draw the mouse cursor; backup the covered area in "bak"

		///if (gpLockSurface(_screen) == -1)
		///     error("gpLockSurface failed: %s.\n", gpGetError());

		//  as dirty
		add_dirty_rect(x, y, w, h);

		dst = (byte *) _screen->pixels + y * _screenWidth + x;
		while (h > 0) {
			int width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeycolor)	// Transparent, don't draw
					*dst = color;
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += MAX_MOUSE_W - w;
			dst += _screenWidth - w;
			h--;
		}

		///gpUnlockSurface(_screen);

		// Finally, set the flag to indicate the mouse has been drawn
		_mouseDrawn = true;
	}

	if (_mouseDrawn || !_mouseVisible)
		return;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	//byte *src = _mouseData;               // Image representing the mouse
	//uint16 *src = _mouseData;             // Image representing the mouse
	const byte *src = _mouseData;	// Image representing the mouse
	//byte *bak = _mouseBackup;     // Surface used to backup the area obscured by the mouse
	//byte *dst;                                    // Surface we are drawing into

	//uint16 *bak = (uint16*)_mouseBackup;  // Surface used to backup the area obscured by the mouse
	//uint16 *dst;                                  // Surface we are drawing into

	// clip the mouse rect, and addjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouseCurState.w;
		y = 0;
	}
	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	if (w > _screenWidth - x)
		w = _screenWidth - x;
	if (h > _screenHeight - y)
		h = _screenHeight - y;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouseOldState.x = x;
	_mouseOldState.y = y;
	_mouseOldState.w = w;
	_mouseOldState.h = h;

	// Draw the mouse cursor; backup the covered area in "bak"

	///if (gpLockSurface(_tmpscreen) == -1)
	///     error("gpLockSurface failed: %s.\n", gpGetError());

	//  as dirty
	add_dirty_rect(x, y, w, h);

	uint16 *bak = (uint16 *) _mouseBackup;	// Surface used to backup the area obscured by the mouse
	uint16 *dst;		// Surface we are drawing into

	dst =
	    (uint16 *) _tmpscreen->pixels + (y + 1) * TMP_SCREEN_WIDTH + (x +
	    1);
	while (h > 0) {
		int width = w;
		while (width > 0) {
			*bak++ = *dst;
			color = *src++;
			if (color != 0xFF)	// 0xFF = transparent, don't draw
				*dst =
				    RGB_TO_16(_currentPalette[color].r,
				    _currentPalette[color].g,
				    _currentPalette[color].b);
			dst++;
			width--;
		}
		src += _mouseCurState.w - w;
		bak += MAX_MOUSE_W - w;
		dst += TMP_SCREEN_WIDTH - w;
		h--;
	}

	///gpUnlockSurface(_tmpscreen);

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true;
}

void OSystem_GP32::undraw_mouse() {
	//return; //fixme!
	if (!_overlayVisible) {

		if (!_mouseDrawn)
			return;
		_mouseDrawn = false;

		byte *dst, *bak = _mouseBackup;
		const int old_mouse_x = _mouseOldState.x;
		const int old_mouse_y = _mouseOldState.y;
		const int old_mouse_w = _mouseOldState.w;
		const int old_mouse_h = _mouseOldState.h;
		int x, y;

		// No need to do clipping here, since draw_mouse() did that already

		dst =
		    (byte *) _screen->pixels + old_mouse_y * _screenWidth +
		    old_mouse_x;
		for (y = 0; y < old_mouse_h;
		    ++y, bak += MAX_MOUSE_W, dst += _screenWidth) {
			for (x = 0; x < old_mouse_w; ++x) {
				dst[x] = bak[x];
			}
		}

		add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w,
		    old_mouse_h);
	}

	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

	uint16 *dst, *bak = (uint16 *) _mouseBackup;
	const int old_mouse_x = _mouseOldState.x;
	const int old_mouse_y = _mouseOldState.y;
	const int old_mouse_w = _mouseOldState.w;
	const int old_mouse_h = _mouseOldState.h;
	int x, y;

	// No need to do clipping here, since draw_mouse() did that already

	dst =
	    (uint16 *) _tmpscreen->pixels + (old_mouse_y +
	    1) * TMP_SCREEN_WIDTH + (old_mouse_x + 1);
	for (y = 0; y < old_mouse_h;
	    ++y, bak += MAX_MOUSE_W, dst += TMP_SCREEN_WIDTH) {
		for (x = 0; x < old_mouse_w; ++x) {
			dst[x] = bak[x];
		}
	}

	add_dirty_rect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);
}

char *gpGetError(void) {
	// FIXME: implement
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
//
// GP32 Screen Update Stuff - Mostly 'borrowed' from GP but using gpSDK
//
/////////////////////////////////////////////////////////////////////////////

//// Update the dirty areas of the screen
void OSystem_GP32::updateScreen() {
	//Common::StackLock lock(_graphicsMutex);       // Lock the mutex until this function ends
	internUpdateScreen();
}

//      assert(_hwscreen != NULL);
//
//      // If the shake position changed, fill the dirty area with blackness
//      if (_currentShakePos != _newShakePos) {
//              gpRect blackrect = {0, 0, _screenWidth*_scaleFactor, _newShakePos*_scaleFactor};
//
//              if (_adjustAspectRatio)
//                      blackrect.h = real2Aspect(blackrect.h - 1) + 1;
//
//              gpFillRect(_hwscreen, &blackrect, 0);
//
//              _currentShakePos = _newShakePos;
//
//              _forceFull = true;
//      }
//
//      // Make sure the mouse is drawn, if it should be drawn.
//      draw_mouse();
//
//      // Check whether the palette was changed in the meantime and update the
//      // screen surface accordingly.
//      if (_paletteDirtyEnd != 0) {
//              gpSetColors(_screen, _currentPalette + _paletteDirtyStart,
//                      _paletteDirtyStart,
//                      _paletteDirtyEnd - _paletteDirtyStart);
//
//              _paletteDirtyEnd = 0;
//
//              _forceFull = true;
//      }
//
//      // Force a full redraw if requested
//      if (_forceFull) {
//              _num_dirty_rects = 1;
//
//              _dirty_rect_list[0].x = 0;
//              _dirty_rect_list[0].y = 0;
//              _dirty_rect_list[0].w = _screenWidth;
//              _dirty_rect_list[0].h = _screenHeight;
//      }
//
//      // Only draw anything if necessary
//      if (_num_dirty_rects > 0) {
//
//              gpRect *r;
//              uint32 srcPitch, dstPitch;
//              gpRect *last_rect = _dirty_rect_list + _num_dirty_rects;
//
//              // Convert appropriate parts of the 8bpp image into 16bpp
//              if (!_overlayVisible) {
//                      gpRect dst;
//                      for(r = _dirty_rect_list; r != last_rect; ++r) {
//                              dst = *r;
//                              dst.x++;        // FIXME? Shift rect by one since 2xSai needs to acces the data around
//                              dst.y++;        // FIXME? any pixel to scale it, and we want to avoid mem access crashes.
//
//                              if (gpBlitSurface(_screen, r, _hwscreen, &dst) != 0) //ph0x! _tmpscreen
//                                      error("gpBlitSurface failed: %s", gpGetError());
//                      }
//              }
//
//// ph0x! (no scaling) cannot skip intro if commented?
//
//              srcPitch = _tmpscreen->pitch;
//              dstPitch = _hwscreen->pitch;
//              for(r = _dirty_rect_list; r != last_rect; ++r) {
//                      register int dst_y = r->y + _currentShakePos;
//                      register int dst_h = 0;
//                      if (dst_y < _screenHeight) {
//                              dst_h = r->h;
//                              if (dst_h > _screenHeight - dst_y)
//                                      dst_h = _screenHeight - dst_y;
//
//                              dst_y *= _scaleFactor;
//
//                              if (_overlayVisible) //ph0x fixme?
//                              _scaler_proc((byte*)_tmpscreen->pixels + (r->x*2+2) + (r->y+1)*srcPitch, srcPitch,
//                                      (byte*)_hwscreen->pixels + r->x*2*_scaleFactor + dst_y*dstPitch, dstPitch, r->w, dst_h);
//                      }
//                      r->x *= _scaleFactor;
//                      r->y = dst_y;
//                      r->w *= _scaleFactor;
//                      r->h = dst_h * _scaleFactor;
//              }
//
//              // Readjust the dirty rect list in case we are doing a full update.
//              // This is necessary if shaking is active.
//              if (_forceFull) {
//                      _dirty_rect_list[0].y = 0;
//                      _dirty_rect_list[0].h = _screenHeight * _scaleFactor;
//              }
//
//              // Finally, blit all our changes to the screen
//
//              // FIXME (dont use condition)
//              if (_overlayVisible)
//                      gpUpdateRects(_hwscreen, _num_dirty_rects, _dirty_rect_list); //ph0x! _hwscreen
//              else
//                      gpUpdateRects(_screen, _num_dirty_rects, _dirty_rect_list);
//      }
//
//      _num_dirty_rects = 0;
//      _forceFull = false;
//}

// Either show or hide the mouse cursor
bool OSystem_GP32::showMouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;

	bool last = _mouseVisible;
	_mouseVisible = visible;

	if (visible)
		draw_mouse();
	else
		undraw_mouse();

	return last;
}

// Set the position of the mouse cursor
void OSystem_GP32::set_mouse_pos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		_mouseCurState.x = x;
		_mouseCurState.y = y;

		mx = x;		//ph0x fixme
		my = y;		//ph0x fixme
		undraw_mouse();
	}
}

// Set the bitmap that's used when drawing the cursor.
void OSystem_GP32::setMouseCursor(const byte *buf, uint w, uint h,
    int hotspot_x, int hotspot_y, byte keycolor, int cursorTargetScale) {

//      assert(w <= MAX_MOUSE_W);
//      assert(h <= MAX_MOUSE_H);

	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseKeycolor = keycolor;

//??????????
	_mouseData = (byte *) buf;

	undraw_mouse();

	if (_mouseData)
		free(_mouseData);

	_mouseData = (byte *) malloc(w * h);

	memcpy(_mouseData, buf, w * h);
}

// Shaking is used in SCUMM. Set current shake position.
void OSystem_GP32::setShakePos(int shake_pos) {
	_newShakePos = shake_pos;
	mcshake = shake_pos;
}

// Get the number of milliseconds since the program was started.
uint32 OSystem_GP32::getMillis() {
	return GpTickCountGet();
}

// Delay for a specified amount of milliseconds
void OSystem_GP32::delayMillis(uint msecs) {
	int n = GpTickCountGet();
	while ((GpTickCountGet() - n) < msecs);
}

// Get the next event.
// Returns true if an event was retrieved.

const signed char abc[] = "0123456789abcdefghijklmnopqrstuvwxyz";
signed int abcindex = -1;

void switchsurf(int surf);
void buildgammatab(int val);

/////////////////////////////////////////////////////////////////////////////
//
// GP32 Event Handlers.
//
/////////////////////////////////////////////////////////////////////////////

bool OSystem_GP32::pollEvent(Event & event) {

#define EVENT_COUNT	2	// >=1
	//#define MOUSE_MIPS    2 // bg updates wrong if >1 ??
#define MOUSE_MIPS 1		// bg updates wrong if >1 ??

	static int lastkey, eventcount = EVENT_COUNT, lastevent = 0;
	static int simulate;
	static bool backspace = true;
	static uint32 t;
	int key;

	key = gpTrapKey();

	if (simulate)
		simulate--;
	switch (simulate) {
	case 5:
		lastevent = event.type = EVENT_KEYDOWN;
		event.kbd.keycode = event.kbd.ascii = 8;
		return true;
		break;
	case 3:
		lastevent = event.type = EVENT_KEYDOWN;
		event.kbd.keycode = event.kbd.ascii = abc[abcindex];
		return true;
		break;
	case 4:
	case 2:
		lastevent = event.type = EVENT_KEYUP;
		//event.kbd.keycode = event.kbd.ascii =
		return true;
		break;
	case 1:
		lastkey = key = 0;
		lastevent = 0;
		event.type = (EventType) 0;
		break;
	}

	if (lastevent == EVENT_KEYDOWN) {
		lastevent = event.type = EVENT_KEYUP;
		//event.kbd.keycode = event.kbd.ascii;
		return true;
	}

	if (key == GPC_VK_NONE) {
		lastevent = lastkey = 0;
		return false;
	}

	if (key == lastkey) {
		eventcount--;
		if (eventcount)
			return false;
	}

	eventcount = EVENT_COUNT;
	event.type = EVENT_KEYDOWN;

	if (key & GPC_VK_FL) {	// L
		if (_overlayVisible)
			return false;

		if (key & GPC_VK_UP) {
			if (key == lastkey)
				return false;

			if (gindex < ARRAYSIZE(scrGamma) - 1)
				gindex++;
			buildgammatab(gindex);
			_paletteDirtyStart = 0;
			_paletteDirtyEnd = 255;	//fixme?
			lastevent = event.type;
			lastkey = key;
			return true;
		} else
		 if (key & GPC_VK_DOWN) {
			if (key == lastkey)
				return false;

			if (gindex > 0)
				gindex--;
			buildgammatab(gindex);
			_paletteDirtyStart = 0;
			_paletteDirtyEnd = 255;	//fixme?
			lastevent = event.type;
			lastkey = key;
			return true;
		}

		if (key == lastkey)
			return false;
		if (skindex > 0)
			skindex--;
		event.kbd.keycode = event.kbd.ascii = shortkey[skindex];
		lastevent = event.type;
		lastkey = key;
		return true;
	}

	lastkey = key;

	if (key & GPC_VK_FR) {	// R
		if (key & GPC_VK_UP) {
			if (getMillis() < t)
				return false;
			//do key=GpKeyGet(); while (key & GPC_VK_UP);
			//fixme -2/-1
			t = getMillis() + 200;
			if (abcindex == -1)
				abcindex = 0;
			else {
				if (abcindex < sizeof(abc) - 2)
					abcindex++;
				else
					abcindex = 0;
			}
			if (backspace)
				simulate = 6;
			else {
				backspace = true;
				simulate = 4;
			}
			return false;
		}

		if (key & GPC_VK_DOWN) {
			if (getMillis() < t)
				return false;
			//do key=GpKeyGet(); while (key & GPC_VK_DOWN);
			//fixme -2/-1
			t = getMillis() + 200;
			if (abcindex == -1)
				abcindex = abcindex = sizeof(abc) - 2;
			else {
				if (abcindex > 0)
					abcindex--;
				else
					abcindex = sizeof(abc) - 2;
			}
			if (backspace)
				simulate = 6;
			else {
				backspace = true;
				simulate = 4;
			}
			return false;
		}

		if (key & GPC_VK_LEFT) {
			abcindex = -1;
			event.kbd.keycode = event.kbd.ascii = 8;
			lastevent = event.type;
			do
				key = gpTrapKey();
			while (key & GPC_VK_LEFT);
			return true;
		} else if (key & GPC_VK_RIGHT) {
			abcindex = -1;
			backspace = false;
			return false;
		}
		if (!_overlayVisible) {
			if (lastevent == EVENT_KEYUP)
				return false;
			if (shortkey[skindex + 1])
				skindex++;
			event.kbd.keycode = event.kbd.ascii =
			    shortkey[skindex];
			lastevent = event.type;
			return true;
		}
	} else if (key & GPC_VK_START) {	// START = menu/enter
		if (_overlayVisible)
			event.kbd.keycode = event.kbd.ascii = 13;
		else {
			event.kbd.keycode = event.kbd.ascii = 319;
			//buildgammatab(ARRAYSIZE(scrGamma)-1); // moved to colortoRBG
		}
		lastevent = event.type;
		return true;
	}

	if (key & GPC_VK_SELECT) {	// SELECT == escape/skip
		if (_overlayVisible) {
			do
				key = gpTrapKey();
			while (key != GPC_VK_NONE);	// prevent 2xESC
			buildgammatab(gindex);
			_paletteDirtyStart = 0;
			_paletteDirtyEnd = 255;	//fixme?
		}
		event.kbd.keycode = event.kbd.ascii = 27;
		lastevent = event.type;
		return true;
	}

	if (key & GPC_VK_FA) {
		if (lastevent == EVENT_LBUTTONUP)
			return false;

		if (lastevent == EVENT_LBUTTONDOWN) {
			lastevent = EVENT_LBUTTONUP;
			event.type = EVENT_LBUTTONUP;
		} else {
			lastevent = EVENT_LBUTTONDOWN;
			event.type = EVENT_LBUTTONDOWN;
		}
		return true;
	}

	if (key & GPC_VK_FB) {
		if (lastevent == EVENT_RBUTTONUP)
			return false;

		if (lastevent == EVENT_RBUTTONDOWN) {
			lastevent = EVENT_RBUTTONUP;
			event.type = EVENT_RBUTTONUP;
		} else {
			lastevent = EVENT_RBUTTONDOWN;
			event.type = EVENT_RBUTTONDOWN;
		}
		return true;
	}

	if (key & GPC_VK_LEFT) {
		mx -= MOUSE_MIPS;
		if (mx < 1)
			mx = 1;	// wrong if 0?
	}

	if (key & GPC_VK_RIGHT) {
		mx += MOUSE_MIPS;
		if (mx > 319)
			mx = 319;
	}

	if (key & GPC_VK_UP) {
		my -= MOUSE_MIPS;
		if (my < 1)
			my = 1;	// wrong if 0?
	}

	if (key & GPC_VK_DOWN) {
		my += MOUSE_MIPS;
		if (my > _screenHeight - 1)
			my = _screenHeight - 1;
	}

	event.type = EVENT_MOUSEMOVE;
	km.x = event.mouse.x = mx;
	km.y = event.mouse.y = my;
	event.mouse.x /= _scaleFactor;
	event.mouse.y /= _scaleFactor;
	set_mouse_pos(event.mouse.x, event.mouse.y);
}

/////////////////////////////////////////////////////////////////////////////
//
// GP32 Graphics Stuff -
//
/////////////////////////////////////////////////////////////////////////////

int16 OSystem_GP32::RBGToColor(uint8 r, uint8 g, uint8 b) {
	float rr, gg, bb;

	rr = r * gammatab2[r];
	gg = g * gammatab2[g];
	bb = b * gammatab2[b];

	if (rr > 255)
		rr = 255;
	if (gg > 255)
		gg = 255;
	if (bb > 255)
		bb = 255;

	r = (u8) rr;
	g = (u8) gg;
	b = (u8) bb;

	//return ((((r>>3)&0x1F) << 11) | (((g>>2)&0x3F) << 5) | ((b>>3)&0x1F)); //ph0x
	return (((((r) >> 3) & 0x1F) << 11) | ((((g) >> 3) & 0x1F) << 6) |
	    (((b) >> 3) & 0x1F) << 1);
}

void OSystem_GP32::colorToRBG(int16 color, uint8 &r, uint8 &g, uint8 &b) {
	float rr, gg, bb;
	r = ((((color) >> 11) & 0x1F) << 3);	//(((color>>11)&0x1F) << 3);
	g = ((((color) >> 6) & 0x1F) << 3);	//(((color>>5)&0x3F) << 2);
	b = ((((color) >> 1) & 0x1F) << 3);	//((color&0x1F) << 3);

	rr = r * gammatab2[r];
	gg = g * gammatab2[g];
	bb = b * gammatab2[b];

	if (rr > 255)
		rr = 255;
	if (gg > 255)
		gg = 255;
	if (bb > 255)
		bb = 255;

	r = (u8) rr;
	g = (u8) gg;
	b = (u8) bb;
}

void switchsurf(int surf) {
	GPLCDINFO lcd;
	GpLcdInfoGet(&lcd);

	if (surf == DEBUG_SURFACE) {
		if (lcd.lcd_global.U8_lcd.bpp == 16)
			GpGraphicModeSet(8, NULL);

		currentsurf = DEBUG_SURFACE;
		GpSurfaceFlip(&LCDbuffer[(int)currentsurf]);

		//GpSetPaletteEntry ( 0, 0,0,0 );
		//GpSetPaletteEntry ( 1, 0,0,0 );
		//GpSetPaletteEntry ( 2, 255,255,255 );

	} else if (surf == GAME_SURFACE) {
		//if (lcd.lcd_global.U8_lcd.bpp == 8) GpGraphicModeSet(16, NULL);

		currentsurf = GAME_SURFACE;
		GpSurfaceFlip(&LCDbuffer[(int)currentsurf]);
		//GpSetPaletteEntry ( 2, 0,0,0 );
		//GpSetPaletteEntry ( 1, 0,107,84 );
		//GpSetPaletteEntry ( 0, 255,255,255 );
	} else
		error("Switching to false stuface");
}

/////////////////////////////////////////////////////////////////////////////
//
// GP32 Sound Stuff -
//
/////////////////////////////////////////////////////////////////////////////

void OSystem_GP32::clearSoundCallback() {
//  _sound_proc = NULL;
//  _sound_proc_param = NULL;
}

typedef void SoundProc(void *param, byte *buf, int len);

typedef struct GPSOUNDBUF {
	PCM_SR freq;		/* Taken from gpmm.h */
	PCM_BIT format;		/* Taken from gpmm.h */
	unsigned int samples;	/* Buffer length (in samples) */
	void *userdata;		/* Userdata which gets passed to the callback function */
	SoundProc *callback;
	unsigned int pollfreq;	/* Frequency of the timer interrupt which polls the playing position
				 * recommended value: 2*(playingfreq in Hz/GPSOUNDBUF.samples) */
	unsigned int samplesize;	/* Size of one sample (8bit mono->1, 16bit stereo->4) - don't touch this */
} GPSOUNDBUF;

GPSOUNDBUF gpsndbuf;		// for scumm

/* Global variables */
unsigned int frame = 0;
unsigned int *soundPos = 0;
volatile int idx_buf;
unsigned int shiftVal = 0;
void *buffer;
GPSOUNDBUF soundBuf;

/* This routine gets called by the timer interrupt and
 * polls the current playing position within the buffer.
 */

//void *blah; // holds "this" for mixer.cpp

void soundtimer(void) {
	unsigned int t =
	    (((unsigned int)(*soundPos) - (unsigned int)buffer) >> shiftVal) >=
	    soundBuf.samples ? 1 : 0;
	if (t != frame) {
		unsigned int offs =
		    ((frame == 1) ? (soundBuf.samples << shiftVal) : 0);
		soundBuf.callback(soundBuf.userdata /*blah */ , (u8 *) ((unsigned int)buffer + offs), soundBuf.samples << shiftVal);	//FIXME (*callback)(param) ?
		frame = t;
	}
}

int GpSoundBufStart(GPSOUNDBUF *sb) {
	frame = 0;

	/* Copy the structure */
	memcpy(&soundBuf, sb, sizeof(GPSOUNDBUF));

	/* Calculate size of a single sample in bytes
	 * and a corresponding shift value
	 */
	shiftVal = 0;
	switch (soundBuf.freq) {
	case PCM_S11:
		break;
	case PCM_S22:
		break;
	case PCM_S44:
		shiftVal++;
		break;
	case PCM_M11:
		break;
	case PCM_M22:
		break;
	case PCM_M44:
		shiftVal++;
		break;
	}
	if (soundBuf.format == PCM_16BIT)
		shiftVal++;
	soundBuf.samplesize = 1 << shiftVal;

	/* Allocate memory for the playing buffer */
	buffer = malloc(soundBuf.samplesize * soundBuf.samples * 2);
	memset(buffer, 0, soundBuf.samplesize * soundBuf.samples * 2);

	/* Set timer interrupt #0 */
	if (GpTimerOptSet(0, soundBuf.pollfreq, 0,
		soundtimer) == GPOS_ERR_ALREADY_USED)
		error("timer slot used");
	GpTimerSet(0);
	/* Start playing */
	GpPcmPlay((unsigned short *)buffer,
	    soundBuf.samples * soundBuf.samplesize * 2, 1);
	GpPcmLock((unsigned short *)buffer, (int *)&idx_buf,
	    (unsigned int *)&soundPos);

	return 0;
}

void GpSoundBufStop(void) {
	GpPcmStop();
	GpPcmRemove((unsigned short *)buffer);
	GpTimerKill(0);
	free(buffer);
}

int OSystem_GP32::getOutputSampleRate() const {
	return SAMPLES_PER_SEC;
}

// Set the function to be invoked whenever samples need to be generated
// Buffer Length and Poll Frequency changed. DJWillis
bool OSystem_GP32::setSoundCallback(SoundProc proc, void *param) {
	gpsndbuf.freq = PCM_S22;	// Taken from gpmm.h
	gpsndbuf.format = PCM_16BIT;	// Taken from gpmm.h
	gpsndbuf.samples = 2048;	//128; //fixme?    // Buffer length (in samples)
	//FIXME? crashes if not commented?!
	//gpsndbuf.userdata=g_scumm; //param; //fixme?          // Userdata which gets passed to the callback function
	gpsndbuf.callback = proc;	//mycallback;         // Callback function (just like in SDL)
	//2*((float)22025/(float)s.samples);
	gpsndbuf.pollfreq = 8 * (SAMPLES_PER_SEC / gpsndbuf.samples);	//fixme
	// Frequency of the timer interrupt which polls the playing position
	// recommended value: 2*(playingfreq in Hz/GPSOUNDBUF.samples)
	//s.samplesize;  // Size of one sample (8bit mono->1, 16bit stereo->4) - don't touch this

	GpPcmInit(PCM_S22, PCM_16BIT);
	GpSoundBufStart(&gpsndbuf);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//
// GP32 Graphics Stuff
//
/////////////////////////////////////////////////////////////////////////////

void OSystem_GP32::get_screen_image(byte *buf) {
	/* make sure the mouse is gone */
	undraw_mouse();

	///if (gpLockSurface(_screen) == -1)
	///     error("gpLockSurface failed: %s.\n", gpGetError());

	memcpy(buf, _screen->pixels, _screenWidth * _screenHeight);

	///gpUnlockSurface(_screen);
}

void OSystem_GP32::hotswap_gfx_mode() {
	/* We allocate a screen sized bitmap which contains a "backup"
	 * of the screen data during the change. Then we draw that to
	 * the new screen right after it's setup.
	 */

	byte *bak_mem = (byte *) malloc(_screenWidth * _screenHeight);

	get_screen_image(bak_mem);

	unload_gfx_mode();
	load_gfx_mode();

	// reset palette
	gpSetColors(_screen, _currentPalette, 0, 256);

	// blit image
	copyRectToScreen(bak_mem, _screenWidth, 0, 0, _screenWidth,
	    _screenHeight);
	free(bak_mem);

	updateScreen();
}

// Get or set a property
//uint32 OSystem_GP32::property(int param, Property *value)
//{
//      switch(param) {
//
//      case PROP_GET_FULLSCREEN:
//              return _full_screen;
//}

void OSystem_GP32::setWindowCaption(const char *caption) {
	//gGameName = caption; // Would like to return game here like DC port. - DJWillis
}

// CDROM Code - All returns false as the GP32 has no CDROM ;-)

bool OSystem_GP32::openCD(int drive) {
	return false;
}

bool OSystem_GP32::pollCD() {
	return false;
}

void OSystem_GP32::playCD(int track, int num_loops, int start_frame, int duration) {
}

void OSystem_GP32::stopCD() {
}

void OSystem_GP32::updateCD() {
}

// End CDROM Code.

// Add a new callback timer

// ph0x FIXME: make members
int _timerinterval;
int (*_timercallback)(int);

void voidcallback() {
	//printf("timer running");
	_timercallback(_timerinterval);	//FIXME ?? (*_timercallback)(_timerinterval);
}

void OSystem_GP32::setTimerCallback(TimerProc callback, int timer) {
	int timerno = 1;	//0 used by sound proc

	if (!callback) {
		GpTimerKill(timerno);
		return;
	}

	if (GpTimerOptSet(timerno, timer, 0,
		voidcallback) == GPOS_ERR_ALREADY_USED)
		error("timer slot used");

	_timerinterval = timer;
	_timercallback = callback;
	GpTimerSet(timerno);
}

// Mutex handling - DJWillis Hack
OSystem::MutexRef OSystem_GP32::createMutex(void) {
	return NULL;
}

void OSystem_GP32::lockMutex(MutexRef) {
}

void OSystem_GP32::unlockMutex(MutexRef) {
}

void OSystem_GP32::deleteMutex(MutexRef) {
}

// Quit
void OSystem_GP32::quit() {
	printf("Quitting...");
	exit(0);
}

// Overlay
void OSystem_GP32::showOverlay() {
	// hide the mouse
	undraw_mouse();

	u8 *s = (u8 *) _screen->pixels;
	u16 *d = (u16 *) _tmpscreen->pixels;
	u8 c;
	// convert to 16 bit
	for (int y = 0; y < _screenHeight; y++) {
		for (int x = 0; x < 320; x++) {
			c = *s;
			*d++ =
			    (u16) gpRGB16(_currentPalette[c].r,
			    _currentPalette[c].g, _currentPalette[c].b);
			s++;
		}
		d += 3;		// tmpscreen width is screen+3
	}
	GpGraphicModeSet(16, NULL);	//ph0x
	// Test code.
	//???????????
	//GpRectFill(NULL,&LCDbuffer[GAME_SURFACE], 0, 0, 320, 240, 0); //black border
	_overlayVisible = true;
	clearOverlay();
}

void OSystem_GP32::hideOverlay() {
	// hide the mouse
	undraw_mouse();

	GpGraphicModeSet(8, NULL);	//ph0x
	GpRectFill(NULL, &LCDbuffer[GAME_SURFACE], 0, 0, 320, 240, 0);	//black border

	_overlayVisible = false;
	_forceFull = true;
}

void OSystem_GP32::clearOverlay() {
	if (!_overlayVisible)
		return;

	// hide the mouse
	undraw_mouse();

	// Clear the overlay by making the game screen "look through" everywhere.
	gpRect src, dst;
	src.x = src.y = 0;
	dst.x = dst.y = 1;
	src.w = dst.w = _screenWidth;
	src.h = dst.h = _screenHeight;
	if (gpBlitSurface(_screen, &src, _tmpscreen, &dst) != 0)	//FIXME
		error("gpBlitSurface failed: %s", gpGetError());
	_forceFull = true;
}

void OSystem_GP32::grabOverlay(int16 *buf, int pitch) {
	if (!_overlayVisible)
		return;

	if (_tmpscreen == NULL)
		return;

	// hide the mouse
	undraw_mouse();

	///if (gpLockSurface(_tmpscreen) == -1)
	///     error("gpLockSurface failed: %s.\n", gpGetError());

	int16 *src = (int16 *) _tmpscreen->pixels + TMP_SCREEN_WIDTH + 1;
	int h = _screenHeight;
	do {
		memcpy(buf, src, _screenWidth * 2);
		src += TMP_SCREEN_WIDTH;
		buf += pitch;
	} while (--h);

	///gpUnlockSurface(_tmpscreen);
}

void OSystem_GP32::copyRectToOverlay(const int16 * buf, int pitch, int x,
    int y, int w, int h) {
	if (!_overlayVisible)
		return;

	if (_tmpscreen == NULL)
		return;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}
	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}
	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}
	if (w <= 0 || h <= 0)
		return;

	// Mark the modified region as dirty
	cksum_valid = false;
	add_dirty_rect(x, y, w, h);

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	undraw_mouse();

	///if (gpLockSurface(_tmpscreen) == -1)
	///     error("gpLockSurface failed: %s.\n", gpGetError());

	int16 *dst =
	    (int16 *) _tmpscreen->pixels + (y + 1) * TMP_SCREEN_WIDTH + (x +
	    1);
	do {
		memcpy(dst, buf, w * 2);
		dst += TMP_SCREEN_WIDTH;
		buf += pitch;
	} while (--h);

	///gpUnlockSurface(_tmpscreen);
}

void OSystem_GP32::internUpdateScreen() {
	assert(_hwscreen != NULL);

	// If the shake position changed, fill the dirty area with blackness

	if (_currentShakePos != _newShakePos) {

		gpRect blackrect =
		    { 0, 0, _screenWidth * _scaleFactor,
			    _newShakePos * _scaleFactor };

		//if (_adjustAspectRatio)
		//      blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		gpFillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}
	// Make sure the mouse is drawn, if it should be drawn.
	draw_mouse();

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_paletteDirtyEnd != 0) {

		gpSetColors(_screen, _currentPalette + _paletteDirtyStart,
		    _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceFull = true;
	}
//#ifdef USE_OSD
//      // OSD visible (i.e. non-transparent)?
//      if (_osdAlpha != gpALPHA_TRANSPARENT) {
//              // Updated alpha value
//              const int diff = gpGetTicks() - _osdFadeStartTime;
//              if (diff > 0) {
//                      if (diff >= kOSDFadeOutDuration) {
//                              // Back to full transparency
//                              _osdAlpha = gpALPHA_TRANSPARENT;
//                      } else {
//                              // Do a linear fade out...
//                              const int startAlpha = gpALPHA_TRANSPARENT + kOSDInitialAlpha * (gpALPHA_OPAQUE - gpALPHA_TRANSPARENT) / 100;
//                              _osdAlpha = startAlpha + diff * (gpALPHA_TRANSPARENT - startAlpha) / kOSDFadeOutDuration;
//                      }
//                      gpSetAlpha(_osdSurface, gpRLEACCEL | gpSRCCOLORKEY | gpSRCALPHA, _osdAlpha);
//                      _forceFull = true;
//              }
//      }
//#endif

	// Force a full redraw if requested
	if (_forceFull) {
		_num_dirty_rects = 1;

		_dirty_rect_list[0].x = 0;
		_dirty_rect_list[0].y = 0;
		_dirty_rect_list[0].w = _screenWidth;
		_dirty_rect_list[0].h = _screenHeight;
	}
	// Only draw anything if necessary
	if (_num_dirty_rects > 0) {

		gpRect *r;
		gpRect dst;
		uint32 srcPitch, dstPitch;
		gpRect *last_rect = _dirty_rect_list + _num_dirty_rects;

		if (_scaler_proc == Normal1x && !_adjustAspectRatio) {

			gpSurface *target =
			    _overlayVisible ? _tmpscreen : _screen;
			for (r = _dirty_rect_list; r != last_rect; ++r) {
				dst = *r;

				if (_overlayVisible) {
					// FIXME: I don't understand why this is necessary...
					dst.x--;
					dst.y--;
				}
				dst.y += _currentShakePos;
				if (gpBlitSurface(target, r, _hwscreen,
					&dst) != 0)
					error("gpBlitSurface failed: %s",
					    gpGetError());
			}
		} else {
			if (!_overlayVisible) {

				for (r = _dirty_rect_list; r != last_rect; ++r) {
					dst = *r;
					dst.x++;	// Shift rect by one since 2xSai needs to acces the data around
					dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.
					if (gpBlitSurface(_screen, r,
						_tmpscreen, &dst) != 0)
						error
						    ("gpBlitSurface failed: %s",
						    gpGetError());
				}
			}
			//gpLockSurface(_tmpscreen);
			//gpLockSurface(_hwscreen);

			srcPitch = _tmpscreen->pitch;
			dstPitch = _hwscreen->pitch;

			for (r = _dirty_rect_list; r != last_rect; ++r) {

				register int dst_y = r->y + _currentShakePos;
				register int dst_h = 0;
				register int orig_dst_y = 0;

				if (dst_y < _screenHeight) {
					dst_h = r->h;
					if (dst_h > _screenHeight - dst_y)
						dst_h = _screenHeight - dst_y;

					dst_y *= _scaleFactor;

					if (_adjustAspectRatio) {
						orig_dst_y = dst_y;
						dst_y = real2Aspect(dst_y);

					}

					_scaler_proc((byte *) _tmpscreen->
					    pixels + (r->x * 2 + 2) + (r->y +
						1) * srcPitch, srcPitch,
					    (byte *) _hwscreen->pixels +
					    r->x * 2 * _scaleFactor +
					    dst_y * dstPitch, dstPitch, r->w,
					    dst_h);
				}

				r->x *= _scaleFactor;
				r->y = dst_y;
				r->w *= _scaleFactor;
				r->h = dst_h * _scaleFactor;

				if (_adjustAspectRatio
				    && orig_dst_y / _scaleFactor <
				    _screenHeight)
					r->h =
					    stretch200To240((uint8 *)
					    _hwscreen->pixels, dstPitch, r->w,
					    r->h, r->x, r->y, orig_dst_y);
			}
			//gpUnlockSurface(_tmpscreen);
			//gpUnlockSurface(_hwscreen);

		}

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirty_rect_list[0].y = 0;
			_dirty_rect_list[0].h = 240;

		}
#ifdef USE_OSD
		if (_osdAlpha != gpALPHA_TRANSPARENT) {
			gpBlitSurface(_osdSurface, 0, _hwscreen, 0);
		}
#endif

		// Finally, blit all our changes to the screen
		gpUpdateRects(_hwscreen, _num_dirty_rects, _dirty_rect_list);

	}

	_num_dirty_rects = 0;
	_forceFull = false;
}

//      assert(_hwscreen != NULL);
//
//      // If the shake position changed, fill the dirty area with blackness
//      if (_currentShakePos != _newShakePos) {
//              gpRect blackrect = {0, 0, _screenWidth*_scaleFactor, _newShakePos*_scaleFactor};
//              gpFillRect(_hwscreen, &blackrect, 0);
//
//              _currentShakePos = _newShakePos;
//
//              _forceFull = true;
//      }
//
//      // Make sure the mouse is drawn, if it should be drawn.
//      draw_mouse(); //ph0x
//
//      // Check whether the palette was changed in the meantime and update the
//      // screen surface accordingly.
//      if (_paletteDirtyEnd != 0) {
//              gpSetColors(_screen, _currentPalette + _paletteDirtyStart,
//                      _paletteDirtyStart,
//                      _paletteDirtyEnd - _paletteDirtyStart);
//
//              _paletteDirtyEnd = 0;
//
//              _forceFull = true;
//      }
//
//      // Force a full redraw if requested
//      if (_forceFull) {
//              _num_dirty_rects = 1;
//
//              _dirty_rect_list[0].x = 0;
//              _dirty_rect_list[0].y = 0;
//              _dirty_rect_list[0].w = _screenWidth;
//              _dirty_rect_list[0].h = _screenHeight;
//      }
//
//      // Only draw anything if necessary
//      if (_num_dirty_rects > 0) {
//
//              gpRect *r;
//              uint32 srcPitch, dstPitch;
//              gpRect *last_rect = _dirty_rect_list + _num_dirty_rects;
//
//              // Convert appropriate parts of the 8bpp image into 16bpp
//              if (!_overlayVisible) {
//                      gpRect dst;
//                      for(r = _dirty_rect_list; r != last_rect; ++r) {
//                              dst = *r;
//                              dst.x++;        // FIXME? Shift rect by one since 2xSai needs to acces the data around
//                              dst.y++;        // FIXME? any pixel to scale it, and we want to avoid mem access crashes.
//
//                              if (gpBlitSurface(_screen, r, _hwscreen, &dst) != 0) //ph0x! gp_tmpscreen
//                                      error("gpBlitSurface failed: %s", gpGetError());
//                      }
//              }
//
//              ///gp_LockSurface(gp_tmpscreen);
//              ///gp_LockSurface(gp_hwscreen);
//
//// ph0x! (no scaling) cannot skip intro if commented?
//
//              srcPitch = _tmpscreen->pitch;
//              dstPitch = _hwscreen->pitch;
//              for(r = _dirty_rect_list; r != last_rect; ++r) {
//                      register int dst_y = r->y + _currentShakePos;
//                      register int dst_h = 0;
//                      if (dst_y < _screenHeight) {
//                              dst_h = r->h;
//                              if (dst_h > _screenHeight - dst_y)
//                                      dst_h = _screenHeight - dst_y;
//
//                              dst_y *= _scaleFactor;
//
//                              if (_overlayVisible) //ph0x fixme?
//                              _scaler_proc((byte*)_tmpscreen->pixels + (r->x*2+2) + (r->y+1)*srcPitch, srcPitch,
//                                      (byte*)_hwscreen->pixels + r->x*2*_scaleFactor + dst_y*dstPitch, dstPitch, r->w, dst_h);
//                      }
//                      r->x *= _scaleFactor;
//                      r->y = dst_y;
//                      r->w *= _scaleFactor;
//                      r->h = dst_h * _scaleFactor;
//              }
//
//              ///gp_UnlockSurface(gp_tmpscreen);
//              ///gp_UnlockSurface(gp_hwscreen);
//
//              // Readjust the dirty rect list in case we are doing a full update.
//              // This is necessary if shaking is active.
//              if (_forceFull) {
//                      _dirty_rect_list[0].y = 0;
//                      _dirty_rect_list[0].h = _screenHeight * _scaleFactor;
//              }
//
//              // Finally, blit all our changes to the screen
//
//              // FIXME (dont use condition)
//              if (_overlayVisible)
//                      gpUpdateRects(_hwscreen, _num_dirty_rects, _dirty_rect_list); //ph0x! gp_hwscreen
//              else
//                      gpUpdateRects(_screen, _num_dirty_rects, _dirty_rect_list);
//      }
//
//      _num_dirty_rects = 0;
//      _forceFull = false;
//}

void OSystem_GP32::setFeatureState(Feature f, bool enable) {

	switch (f) {
	case kFeatureFullscreenMode:
		setFullscreenMode(enable);
		break;
	case kFeatureAspectRatioCorrection:
		if (_screenHeight == 200 && _adjustAspectRatio != enable) {
			Common::StackLock lock(_graphicsMutex);

			//assert(_hwscreen != 0);
			_adjustAspectRatio ^= true;
			hotswap_gfx_mode();

#ifdef USE_OSD
			char buffer[128];
			if (_adjustAspectRatio)
				sprintf(buffer,
				    "Enabled aspect ratio correction\n%d x %d -> %d x %d",
				    _screenWidth, _screenHeight, _hwscreen->w,
				    _hwscreen->h);
			else
				sprintf(buffer,
				    "Disabled aspect ratio correction\n%d x %d -> %d x %d",
				    _screenWidth, _screenHeight, _hwscreen->w,
				    _hwscreen->h);
			displayMessageOnOSD(buffer);
#endif

			// Blit everything to the screen
			internUpdateScreen();

			// Make sure that an EVENT_SCREEN_CHANGED gets sent later
			_modeChanged = true;
		}
		break;
	case kFeatureAutoComputeDirtyRects:
		if (enable)
			_mode_flags |= DF_WANT_RECT_OPTIM;
		else
			_mode_flags &= ~DF_WANT_RECT_OPTIM;
		break;
	default:
		break;
	}
}

bool OSystem_GP32::hasFeature(Feature f) {
	return false;
	(f == kFeatureFullscreenMode) || (f == kFeatureAspectRatioCorrection);
	// ||
//              (f == kFeatureAutoComputeDirtyRects);
}

bool OSystem_GP32::getFeatureState(Feature f) {

	switch (f) {
	case kFeatureFullscreenMode:
		return _full_screen;
	case kFeatureAspectRatioCorrection:
		return _adjustAspectRatio;
	case kFeatureAutoComputeDirtyRects:
		return _mode_flags & DF_WANT_RECT_OPTIM;
	default:
		return false;
	}
}

void OSystem_GP32::setFullscreenMode(bool enable) {
	Common::StackLock lock(_graphicsMutex);

	if (_full_screen != enable) {
		assert(_hwscreen != 0);
		_full_screen ^= true;
		undraw_mouse();

		//if (!gpWM_ToggleFullScreen(_hwscreen)) {
		// if ToggleFullScreen fails, achieve the same effect with hotswap gfx mode
		//      hotswap_gfx_mode();
		//}

#ifdef USE_OSD
		if (_full_screen)
			displayMessageOnOSD("Fullscreen mode");
		else
			displayMessageOnOSD("Windowed mode");
#endif

		// Blit everything to the screen
		internUpdateScreen();

		// Make sure that an EVENT_SCREEN_CHANGED gets sent later
		_modeChanged = true;
	}
}

static const OSystem::GraphicsMode supportedGraphicsModes[] = {
	{"1x", "320x240 16bpp", GFX_NORMAL},
	{0, 0, 0}
};

const OSystem::GraphicsMode *OSystem_GP32::getSupportedGraphicsModes() const {
	return supportedGraphicsModes;
}

int OSystem_GP32::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

bool OSystem_GP32::setGraphicsMode(int mode) {

	Common::StackLock lock(_graphicsMutex);

	int newScaleFactor = 1;
	ScalerProc *newScalerProc;

	switch (mode) {
	case GFX_NORMAL:
		newScaleFactor = 1;
		newScalerProc = Normal1x;
		break;
		//case GFX_DOUBLESIZE:
		//      newScaleFactor = 2;
		//      newScalerProc = Normal2x;
		//      break;
		//case GFX_TRIPLESIZE:
		//      newScaleFactor = 3;
		//      newScalerProc = Normal3x;
		//      break;
		//case GFX_2XSAI:
		//      newScaleFactor = 2;
		//      newScalerProc = _2xSaI;
		//      break;
		//case GFX_SUPER2XSAI:
		//      newScaleFactor = 2;
		//      newScalerProc = Super2xSaI;
		//      break;
		//case GFX_SUPEREAGLE:
		//      newScaleFactor = 2;
		//      newScalerProc = SuperEagle;
		//      break;
		//case GFX_ADVMAME2X:
		//      newScaleFactor = 2;
		//      newScalerProc = AdvMame2x;
		//      break;
		//case GFX_ADVMAME3X:
		//      newScaleFactor = 3;
		//      newScalerProc = AdvMame3x;
		//      break;
		//case GFX_HQ2X:
		//      newScaleFactor = 2;
		//      newScalerProc = HQ2x;
		//      break;
		//case GFX_HQ3X:
		//      newScaleFactor = 3;
		//      newScalerProc = HQ3x;
		//      break;
		//case GFX_TV2X:
		//      newScaleFactor = 2;
		//      newScalerProc = TV2x;
		//      break;
		//case GFX_DOTMATRIX:
		//      newScaleFactor = 2;
		//      newScalerProc = DotMatrix;
		//      break;

	default:
		warning("unknown gfx mode %d", mode);
		return false;
	}

	_mode = mode;
	_scaler_proc = newScalerProc;
	if (newScaleFactor != _scaleFactor) {
		_scaleFactor = newScaleFactor;
		hotswap_gfx_mode();
	}

	if (!_screen)
		return true;

#ifdef USE_OSD
	if (_osdSurface) {
		const char *newScalerName = 0;
		const GraphicsMode *g = s_supportedGraphicsModes;
		while (g->name) {
			if (g->id == mode) {
				newScalerName = g->description;
				break;
			}
			g++;
		}
		if (newScalerName) {
			char buffer[128];
			sprintf(buffer,
			    "Active graphics filter: %s\n%d x %d -> %d x %d",
			    newScalerName, _screenWidth, _screenHeight,
			    _hwscreen->w, _hwscreen->h);
			displayMessageOnOSD(buffer);
		}
	}
#endif

	// Blit everything to the screen
	_forceFull = true;
	internUpdateScreen();

	// Make sure that an EVENT_SCREEN_CHANGED gets sent later
	_modeChanged = true;

	return true;
}

int OSystem_GP32::getGraphicsMode() const {
	return _mode;
}

////OSystem *OSystem_GP32::create(int gfx_mode, bool full_screen)
//OSystem *OSystem_GP32::create()
//{
//      //OSystem_GP32 *syst = new OSystem_GP32();
//
//      //syst->_mode = gfx_mode;
//      //syst->_full_screen = full_screen;
//      //
//      //// allocate palette storage
//      //syst->_currentPalette = (gpColor*)calloc(sizeof(gpColor), 256);
//
//      //// allocate the dirty rect storage
//      //syst->_mouseBackup = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H * MAX_SCALING * 2);
//      /*return syst;*/
//      return new OSystem_GP32();
//}

//////////////////////////////////////////////////
// GP32 stuff
//////////////////////////////////////////////////

extern "C" int write(int fd, void *p, size_t n);
int write(int fd, void *p, size_t n) {	//ph0x hack!
	return 0;
}

// Converts 8bit rgb values to a GP32 palette value
void GpSetPaletteEntry(u8 i, u8 r, u8 g, u8 b) {
	GP_PALETTEENTRY entry = gpRGB16(r, g, b);
	GpPaletteEntryChange(i, 1, &entry, 0);
}

int gpprintf(const char *fmt, ...) {
	static bool busy;
	static int y;
	char s[1024];		// ?
	va_list marker;

	if (busy)
		return 0;
	busy = true;
	va_start(marker, fmt);
	vsnprintf(s, 1024, fmt, marker);
	va_end(marker);

#ifdef GPDEBUG
	//dprintf("mem: %d  ", gm_availablesize());
	dprintf(s);
	if (s[strlen(s) - 1] != '\n')
		dprintf("\n");
	//if (s[0]!='>') return r;
#endif

	fprintf(stdout, s);

	// print to lcd
	GpTextOut(NULL, &LCDbuffer[DEBUG_SURFACE], 0, y, s, 1);
	y += (ENGFONT_H - FONT_LINEGAP);
	if (y >
	    (240 / (ENGFONT_H - FONT_LINEGAP)) * (ENGFONT_H - FONT_LINEGAP)) {
		y = 0;
		GpRectFill(NULL, &LCDbuffer[DEBUG_SURFACE], 0, 0, 320, 240, 2);
	}
	busy = false;
	return 0;
}

int gpfprintf(FILE *stream, const char *fmt, ...) {
	char s[256];
	va_list marker;

	va_start(marker, fmt);
	vsnprintf(s, 256, fmt, marker);
	va_end(marker);

	return fwrite(s, 1, strlen(s), stream);
}

typedef struct {
	FILE f;
	ulong size;
	ulong p;		//cache position
} xfile;

#define XFILE(f) (*(xfile*)f)
#define FCACHE_SIZE 8*1024	// speed up writes

FILE *gpfopen(const char *filename, const char *mode) {
	//FIXME: allocation, mode, malloc -> new
	ulong m;
	FILE *f;
	ERR_CODE err;
	char s[256];

	if (!strchr(filename, '.')) {
		sprintf(s, "%s.", filename);
		filename = s;
	}
	//printf(">open %s as %s", filename, mode);

	// FIXME add binary/text support
	if (tolower(mode[0]) == 'r') {
		f = (FILE *) malloc(sizeof(xfile));
		m = OPEN_R;
		GpFileGetSize(filename, &XFILE(f).size);
		err = GpFileOpen(filename, m, f);
	} else if (tolower(mode[0]) == 'w') {
		//printf("open if as W");
		f = (FILE *) malloc(sizeof(xfile) + FCACHE_SIZE);
		XFILE(f).size = 0;	// FIXME? new file has no size?
		XFILE(f).p = 0;
		m = OPEN_W;
		err = GpFileCreate(filename, ALWAYS_CREATE, f);
	} else
		error("wrong file mode");

	if (!f)
		error("%s: cannot crate F_HANDLE", __FUNCTION__);
	if (err) {
		//printf("IOerr %d", err);
		return NULL;
	} else
		return f;
}

int gpfclose(FILE *f) {
	if (!f) {
		//warning("closing null file");
		return 1;
	}

	if (*(u32 *)((char *)f - sizeof(u32)) == 0x4321) {
		debug(0, "Double closing", __FUNCTION__);
		return 1;
	}			// return 1 ??

	if (XFILE(f).p) {
		GpFileWrite(*f, (char *)f + sizeof(xfile), XFILE(f).p);	// flush cache
		XFILE(f).p = 0;
	}

	ERR_CODE err = GpFileClose(*f);
	free(f);

	return err;
}

int gpfseek(FILE *stream, long offset, int whence) {
	ulong dummy;

	switch (whence) {
	case SEEK_SET:
		whence = FROM_BEGIN;
		break;
	case SEEK_CUR:
		whence = FROM_CURRENT;
		break;
	case SEEK_END:
		whence = FROM_END;
		break;
	}
	return GpFileSeek(*stream, whence, offset, (long *)&dummy);
}

long gpftell(FILE *stream)	{ // fixme? use standard func
	ulong pos = 0;
	//ERR_CODE err = GpFileSeek(*stream, FROM_CURRENT, 0, (long*)&pos);
	return pos;
}

size_t gpfread(void *ptr, size_t size, size_t n, FILE *stream) {
	ulong readcount = 0;
	//ERR_CODE err = GpFileRead(*stream, ptr, size*n, &readcount); //fixme? size*n
	return readcount / size;	//FIXME?
}

size_t gpfwrite(const void *ptr, size_t size, size_t n, FILE *f) {
	int len = size * n;

	if (!f) {
		//warning("writing to null file");
		return 0;
	}

	if (XFILE(f).p + len < FCACHE_SIZE) {
		memcpy((char *)f + sizeof(xfile) + XFILE(f).p, ptr, len);
		XFILE(f).p += len;
	} else {
		if (XFILE(f).p) {
			GpFileWrite(*f, (char *)f + sizeof(xfile), XFILE(f).p);	// flush cache
			XFILE(f).p = 0;
		}

		ERR_CODE err = GpFileWrite(*f, ptr, len);
		if (!err)
			return n;
		else
			return -err;
	}
	return 0;
}

void gpclearerr(FILE *stream) {
	//warning("fixme: %s", __FUNCTION__);
}

int gpfeof(FILE *f)	{	//fixme!
	return ftell(f) >= XFILE(f).size;
}

char *gpfgets(char *s, int n, FILE *f) {
	int i = 0;

	while (!feof(f) && i < n) {
		fread(&s[i], 1, 1, f);
		if (s[i] == '\n') {
			s[i + 1] = 0;
			return s;
		}
		i++;
	}
	if (feof(f))
		return NULL;
	else
		return s;
}

char gpfgetc(FILE *f) {
	char c[1];

	fread(&c[0], 1, 1, f);
	return c[0];
}

int gpfflush(FILE * stream) {
	return 0;
}

/*
 * GP32 Memory managment.
 */

void *gpmalloc(size_t size) {
	u32 np;
	u32 *up;

	np = (u32) gm_malloc(size + sizeof(u32));

	if (np) {
		up = (u32 *) np;
		*up = 0x1234;
		return (void *)(np + sizeof(u32));
	}

	return NULL;
}

void *gpcalloc(size_t nitems, size_t size) {
	void *p = gpmalloc(nitems * size);	//gpcalloc doesnt clear?

	memset(p, 0, nitems * size);
	if (*(u8 *) p)
		warning("%s: calloc doesn't clear!", __FUNCTION__);	//fixme: was error
	//printf("callocing");
	return p;
}

void gpfree(void *block) {
	u32 np;
	u32 *up;

	if (!block) {
		debug(0, "freeing null pointer");
		return;
	}

	np = ((u32) block) - sizeof(u32);
	up = (u32 *) np;
	if (*up == 0x4321)
		error("%s: double deallocation!", __FUNCTION__);

	if (*up != 0x1234)
		error("%s: corrupt block!", __FUNCTION__);
	*up = 0x4321;

	gm_free(up);
}

/*
char *gpstrdup(const char *s) {
	char *p=(char*)malloc(strlen(s)+1);
	memcpy(p, s, strlen(s)+1);
	return p;
}
*/

char *gpstrdup(const char *strSource) {
	char *strBuffer;
	strBuffer = (char *)malloc(strlen(strSource) + 1);
	if (strBuffer)
		strcpy(strBuffer, strSource);
	return strBuffer;
}

time_t gptime(time_t *timer) {
	time_t t = GpTickCountGet() / 1000;
	if (timer)
		*timer = t;
	return t;
}

void gpdeinit() {
	fclose(fstdin);
	fclose(fstdout);
	fclose(fstderr);
}

void gpexit(int code) {
	switchsurf(DEBUG_SURFACE);

	if (!code) {
		printf("----------------------------------------");
		printf("     Your GP32 is now restarting...     ");
		printf("----------------------------------------");

		gpdeinit();

		// FIXME: use function :)
		int n = GpTickCountGet();
		while ((GpTickCountGet() - n) < 3000);

		GpAppExit();
	} else {
		printf("Exit Code %d", code);
		while (1);
	}
}

/****************************************************************
    Setup CPU Speed - Calls to CPUSPEED.S
****************************************************************/
void gpCPUSpeed(int freq) {
	// To extend use: cpu_speed(CLK_SPEED, DIV_FACTOR, CLK_MODE);
	if (freq == 166)
		cpu_speed(165000000, 0x2f001, 3);	// 40 Bus?
	if (freq == 156)
		cpu_speed(156000000, 0x2c001, 3);	// 36 Bus
	if (freq == 133)
		cpu_speed(133500000, (81 << 12) | (2 << 4) | 1, 2);	// 66 Bus?
	if (freq == 132)
		cpu_speed(132000000, 0x3a011, 3);	// 33 Bus
	if (freq == 120)
		cpu_speed(120000000, 0x24001, 2);
	if (freq == 100)
		cpu_speed(102000000, (43 << 12) | (1 << 4) | 1, 2);
	if (freq == 66)
		cpu_speed(67500000, (37 << 12) | (0 << 4) | 2, 2);
	if (freq == 40)
		cpu_speed(40000000, 0x48013, 1);	// Default
	if (freq == 33)
		cpu_speed(33750000, (37 << 12) | (0 << 4) | 3, 2);	// Ultra slow
}

/****************************************************************
    Pre-ScummVM Configuration Menu.
****************************************************************/
int ConfigMenu() {
	//#ifndef GP32_GDB
	//      gpCPUSpeed(40);
	//#endif /*GP32_GDB*/

	GpSetPaletteEntry(2, 0, 0, 0);
	GpSetPaletteEntry(1, 0, 0, 0);
	GpSetPaletteEntry(0, 255, 255, 255);

	int i, key, fg, bg, choice = 0, y = ENGFONT_H * 7;
	int n = ARRAYSIZE(menu);

	GpTextOut(NULL, &LCDbuffer[(int)currentsurf], 0, y,
	    "Configuration Menu", 1);
	y += ENGFONT_H;
	GpTextOut(NULL, &LCDbuffer[(int)currentsurf], 0, y,
	    "----------------------------------------", 1);
	y += ENGFONT_H;

	do {
		for (i = 0; i < n; i++) {
			if (i == choice) {
				fg = 2;
				bg = 1;
			} else {
				fg = 1;
				bg = 2;
			}
			GpRectFill(NULL, &LCDbuffer[(int)currentsurf], 0,
			    y + i * ENGFONT_H, 320, ENGFONT_H, bg);
			char s[256];
			sprintf(s, "%s [%s]", menu[i].option,
			    menu[i].submenu[menu[i].index]);
			GpTextOut(NULL, &LCDbuffer[(int)currentsurf], 0,
			    y + i * ENGFONT_H, s, fg);
		}

		do
			key = gpTrapKey();
		while (key == GPC_VK_NONE);

		if (key & GPC_VK_DOWN) {
			if (choice < n - 1)
				choice++;
		}
		if (key & GPC_VK_UP) {
			if (choice > 0)
				choice--;
		}
		if (key & GPC_VK_LEFT) {
			if (menu[choice].index > 0)
				menu[choice].index--;
		}
		if (key & GPC_VK_RIGHT) {
			if (menu[choice].submenu[menu[choice].index + 1])
				menu[choice].index++;
		}
		//if (key & GPC_VK_START || key  & GPC_VK_FA) return 1;
		if (key & GPC_VK_FA)
			return 1;
		//if (key & GPC_VK_SELECT) return 0;

		do
			key = gpTrapKey();
		while (key != GPC_VK_NONE);

	} while (1);
}

/****************************************************************
    Delay (very simple delay)
****************************************************************/
void Delay(unsigned int ms) {
	unsigned int delay_by;
	delay_by = GpTickCountGet();
	while (GpTickCountGet() - delay_by < ms);
}

/****************************************************************
    Triple buffering code
****************************************************************/
void FlipScreen() {
	if (nflip == 0) {
		GpSurfaceFlip(&LCDbuffer[0]);
		nflip = 1;
	} else if (nflip == 1) {
		GpSurfaceFlip(&LCDbuffer[1]);
		nflip = 2;
	} else if (nflip == 2) {
		GpSurfaceFlip(&LCDbuffer[2]);
		nflip = 0;
	}
}

/****************************************************************
    Clear all the screen buffers
****************************************************************/
void ClearScreen() {
	int i;
	for (i = 0; i <= BUFFERCOUNT; i++) {
		GpRectFill(NULL, &LCDbuffer[i], 0, 0, LCDbuffer[i].buf_w,
		    LCDbuffer[i].buf_h, 0x00);
	}
}

/****************************************************************
    Fade to black
****************************************************************/
void FadeToBlack(int delay_time) {
	//Fade to black
	int x;
	for (x = 0; x < 30; x++) {
		//Fade it further
		GpLcdFade(-1, NULL);

		//refresh screen
		GpSurfaceFlip(&LCDbuffer[nflip]);

		//wait a littel bit
		Delay(delay_time);
	}

	//Now Clear all the buffers
	ClearScreen();

	//Turn the fading off
	GpLcdNoFade(NULL);

	//Now flip to end it all and leave it black
	FlipScreen();
}

/****************************************************************
    Fade to White
****************************************************************/
void FadeToWhite(int delay_time) {
	//Fade to black
	int x;
	for (x = 0; x < 30; x++) {
		//Fade it further
		GpLcdFade(1, NULL);

		//refresh screen
		GpSurfaceFlip(&LCDbuffer[nflip]);

		//wait a littel bit
		Delay(delay_time);
	}

	//Now Clear all the buffers
	ClearScreen();

	//Turn the fading off
	GpLcdNoFade(NULL);

	//Now flip to end it all and leave it black
	FlipScreen();
}

/****************************************************************
    Initialise the File System
****************************************************************/
void InitFileSystem() {
	//Initialises GP32 file system
	GpFatInit();
	GpRelativePathSet("gp:\\gpmm");

	// Create folders on SMC if there not there
	// For storing games, saves, config and scummvm.ini.
	GpDirCreate("gp:\\data", NOT_IF_EXIST);
	GpDirCreate("gp:\\data\\scummvm", NOT_IF_EXIST);
	GpDirCreate("gp:\\data\\scummvm\\games", NOT_IF_EXIST);
	GpDirCreate("gp:\\data\\scummvm\\config", NOT_IF_EXIST);
}

/****************************************************************
    Splash Screen - show splash screen
****************************************************************/

void InitSplashPal() {
	static GP_HPALETTE h_splash_pal = NULL;
	if (h_splash_pal)
		GpPaletteDelete(h_splash_pal);
	h_splash_pal = GpPaletteCreate(gfx_splash_palnb, gfx_splash_Pal);
	GpPaletteDelete(GpPaletteSelect(h_splash_pal));
	GpPaletteRealize();
}

int SplashScreen() {
	int key;

	ClearScreen();
	InitSplashPal();

	// Show the screen (load into all buffers - ready for screen transition)
	GpBitBlt(NULL, &LCDbuffer[0], 0, 0, gfx_splash_width,
	    gfx_splash_height, (unsigned char *)gfx_splash, 0, 0,
	    gfx_splash_width, gfx_splash_height);
	GpBitBlt(NULL, &LCDbuffer[1], 0, 0, gfx_splash_width,
	    gfx_splash_height, (unsigned char *)gfx_splash, 0, 0,
	    gfx_splash_width, gfx_splash_height);
	GpBitBlt(NULL, &LCDbuffer[2], 0, 0, gfx_splash_width,
	    gfx_splash_height, (unsigned char *)gfx_splash, 0, 0,
	    gfx_splash_width, gfx_splash_height);

	// Refresh screen (show the logo)
	FlipScreen();

	// Initialise the File System
	// Done during SpashScreen to hide folder create (if needed) from users.
	InitFileSystem();

	//TODO: Put branchs for Start and Select and act accordingly.

	do {
		do
			key = gpTrapKey();
		while (key == GPC_VK_NONE);
		if (key & GPC_VK_START) {
			FadeToBlack(20);
			return 0;
		}
		if (key & GPC_VK_SELECT) {
			FadeToWhite(20);
			ConfigMenu();
			return 0;
		}
		do
			key = gpTrapKey();
		while (key != GPC_VK_NONE);
	} while (1);

	//Fade the screen into GP32 setup or ScummVM.
	//FadeToWhite(100);
}

/****************************************************************
    Read and write the GP32 config file to the SMC
****************************************************************/
void ConfigRead() {
	FILE *f;

	f = fopen("gp:\\data\\scummvm\\config\\config.dat", "r");

	if (f) {
		for (unsigned int i = 0; i < ARRAYSIZE(menu); i++)
			fread(&menu[i].index, 1, sizeof(menu[i].index), f);
		fclose(f);
	}
}

void ConfigWrite() {
	FILE *f;

	f = fopen("gp:\\data\\scummvm\\config\\config.dat", "w");

	if (f) {
		for (unsigned int i = 0; i < ARRAYSIZE(menu); i++)
			fwrite(&menu[i].index, 1, sizeof(menu[i].index), f);
		fclose(f);
	}
}

/****************************************************************
    Prepare GP32
****************************************************************/
void InitLCD() {
	// Initialize graphics
	GpGraphicModeSet(COLOUR_8BIT_MODE, NULL);

	// Set the current buffer
	nflip = 0;

	short i;
	for (i = 0; i <= BUFFERCOUNT; i++) {
		GpLcdSurfaceGet(&LCDbuffer[i], i);
	}
}

void Init() {
	// Setup the LCD.
	InitLCD();

	// Load the Splash Screen and give the option of config or ScummVM.
	// also sets up file system.
	SplashScreen();

	//GpSetPaletteEntry ( 2, 0,0,0 );
	//GpSetPaletteEntry ( 1, 0,0,0 );
	//GpSetPaletteEntry ( 0, 255,255,255 );

	//// fixme - use get function
	////currentsurf=DEBUG_SURFACE;
	////GpSurfaceSet(&LCDbuffer[(int)currentsurf]);
	//GpSurfaceSet(&LCDbuffer[nflip]);
	//GpLcdEnable();

	//stderr = fstdout = fopen("gp:\\data\\scummvm\\config\\debug.out", "w");
	//stdin = NULL; //fixme?
	////fstdin = fopen("stdin", "w");
	////fstderr = fopen("stderr", "w");

	//printf("          ScummVM for the GP32");
	//printf("----------------------------------------");
	//printf("PRIVATE BUILD - DO NOT PASS ON!");
	//printf("ScummVM (c) 2001-4 The ScummVM Team");
	//printf("GP32 Backend (c) 2004 by DJWillis");
	//printf("Compiled %s, %s", __DATE__, __TIME__);
	//printf("----------------------------------------");
	//printf("       Press 'A' to Start ScummVM");
	//printf("----------------------------------------");

	///*
	//ERR_CODE err;
	//
	//unsigned long bad;
	//err = GpFormat("gp:", FORMAT_RESCUE, &bad);
	//char s[256];
	//GpRelativePathGet(s);
	//*/
}

//void *gpmemset (void *s, int c, size_t n) {
//      for (int i=n-1; i>=0; i--)
//              ((char*)s)[i]=(char)c;
//}
//
//void *gpmemcpy (void *dest, const void *src, size_t n) {
//      for (int i=n-1; i>=0; i--)
//              ((char*)dest)[i]=((char*)src)[i];
//}

void buildgammatab(int val) {
	float g = 1;
	for (int i = 0; i < 256; i++) {
		gammatab[255 - i] = g;
		g *= scrGamma[val];
	}
}

void buildgammatab2(int val) {
	float g = 1;
	for (int i = 0; i < 256; i++) {
		gammatab2[255 - i] = g;
		g *= scrGamma[val];
	}
}

int stricmp(const char *string1, const char *string2) {
	char src[4096];
	char dest[4096];
	int i;

	for (i = 0; i < strlen(string1); i++)
		if (string1[i] >= 'A' && string1[i] <= 'Z')
			src[i] = string1[i] + 32;
		else
			src[i] = string1[i];
	src[i] = 0;

	for (i = 0; i < strlen(string2); i++)
		if (string2[i] >= 'A' && string2[i] <= 'Z')
			dest[i] = string2[i] + 32;
		else
			dest[i] = string2[i];
	dest[i] = 0;

	return strcmp(src, dest);
}

int strnicmp(const char *string1, const char *string2, int len) {
	char src[4096];
	char dest[4096];
	int i;

	for (i = 0; i < strlen(string1) && i < len; i++)
		if (string1[i] >= 'A' && string1[i] <= 'Z')
			src[i] = string1[i] + 32;
		else
			src[i] = string1[i];
	src[i] = 0;

	for (i = 0; i < strlen(string2) && i < len; i++)
		if (string2[i] >= 'A' && string2[i] <= 'Z')
			dest[i] = string2[i] + 32;
		else
			dest[i] = string2[i];
	dest[i] = 0;

	return strncmp(src, dest, len);
}

extern "C" void GpMain(void *arg);
extern "C" int scummvm_main(int argc, char *argv[]);

void GpMain(void *arg) {

#ifdef GP32_GDB
	OpenUSB();
	InstallISR();
#endif				/*GP32_GDB */

	// Wank up the GP32 good and propper ;-)
	//      asm volatile(" \n"
	//" mov         r0, #0x01 \n"
	//" ldr         r1, [r0] \n"

	//" \n"
	//:
	//:
	//:"r0", "r1");

	// FIXME: causes crash?! (if not at first line of gpmain())
	buildgammatab(gindex);
	buildgammatab2(ARRAYSIZE(scrGamma) - 1);

	Init();

	// ConfigRead();

	//if ()
	//{
	//      ConfigWrite();
	//}

	//ConfigMenu();
	//ConfigWrite();

	// fixme - use get function

	//currentsurf=GAME_SURFACE;
	//GpSurfaceFlip(&LCDbuffer[(int)currentsurf]);

#ifndef GP32_GDB
	int CPUSpeed =
	    atoi((const char *)menu[MENU_CPUSPEED].submenu[menu[MENU_CPUSPEED].
		index]);
#endif				/*GP32_GDB */

	//static char *argv[] = { "scummvm", NULL, NULL, NULL };
//      char *argv[] = { "scummvm", (char*)menu[MENU_MUSICDRV].submenu[menu[MENU_MUSICDRV].index]};
//      static int argc = 4;

	// Game Testing...
	//int argc = 2;
	//int argc = 4; char *argv[] = { "scummvm", "-enull", "-pgp:\\gpmm\\scummvm\\sky\\", "sky" };
	int argc = 4;
	char *argv[] = { "scummvm", "-enull", "", "" };

	while (1) {
		// Only set the CPU speed if the GDB Stub is NOT needed.
		// No point calling ClearScreen(); as we want to see any odd stuff.
#ifndef GP32_GDB
		gpCPUSpeed(CPUSpeed);
		ClearScreen();
#endif				/*GP32_GDB */

		//FadeToWhite(200);
		//exit(scummvm_main(argc, argv));
		////////////////cast_argv = f(const_cast<double&>(d));
		//////////////char* argv_;
		//////////////  argv_ = const_cast<*char*>(argv);
		scummvm_main(argc, argv);
	}
}
