/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifdef _WIN32_WCE

/* Original GFX code by Vasyl Tsvirkunov */

#include "wince.h"

//#include <windows.h>
//#include <Aygshell.h>
//#include "gx.h"
//#include "screen.h"
//#include "resource.h"
//#include "dynamic_imports.h"

#include <tlhelp32.h>

#define COLORCONV565(r,g,b) \
(((r&0xf8)<<(11-3))|((g&0xfc)<<(5-2))|((b&0xf8)>>3))
#define COLORCONV555(r,g,b) \
(((r&0xf8)<<(10-3))|((g&0xf8)<<(5-2))|((b&0xf8)>>3))
#define COLORCONVMONO(r,g,b) ((((3*r>>3)+(g>>1)+(b>>3))>>colorscale)^invert)
#define RED_FROM_565(x)		((((x)>>11)&0x1F) << 3)
#define GREEN_FROM_565(x)	((((x)>>5)&0x3F) << 2)
#define BLUE_FROM_565(x)	(((x)&0x1F) << 3)

#define MAX_CLR         0x100
static UBYTE palRed[MAX_CLR];
static UBYTE palGreen[MAX_CLR];
static UBYTE palBlue[MAX_CLR];
static unsigned short pal[MAX_CLR];
 /* First 10 and last 10 colors on palettized devices require special treatment */
static UBYTE staticTranslate[20];
static UBYTE invert = 0;
static int colorscale = 0;
unsigned char color_match[500];  // used for paletted issues	

extern const char *getBuildDate();

extern UBYTE item_toolbar[];
extern UBYTE item_toolbar_colors[];
extern UBYTE item_toolbarPortrait[];
extern UBYTE item_toolbarPortrait_colors[];
extern UBYTE item_disk[];
extern UBYTE item_disk_colors[];
extern UBYTE item_skip[];
extern UBYTE item_skip_colors[];
extern UBYTE item_soundOff[];
extern UBYTE item_soundOff_colors[];
extern UBYTE item_soundOn[];
extern UBYTE item_soundOn_colors[];
extern UBYTE item_monkeyPortrait[];
extern UBYTE item_monkeyPortrait_colors[];
extern UBYTE item_monkeyLandscape[];
extern UBYTE item_monkeyLandscape_colors[];
extern UBYTE item_keyboard[];
extern UBYTE item_keyboard_colors[];
extern UBYTE item_keyboardPortrait[];
extern UBYTE item_keyboardPortrait_colors[];
extern UBYTE item_loading[];
extern UBYTE item_loading_colors[];
extern UBYTE item_startup[];
extern UBYTE item_startup_colors[];
extern int item_startup_colors_size;
extern UBYTE item_bomb[];
extern UBYTE item_bomb_colors[];


extern bool sound_activated;
extern bool hide_toolbar;
extern bool is_simon;
extern bool smartphone;
extern bool high_res;
extern NewGui *g_gui;
bool toolbar_drawn;
bool draw_keyboard;
bool wide_screen;
bool extra_wide_screen;

GXDisplayProperties gxdp;
int active;

UBYTE decomp[320 * 240];
UBYTE comment_zone[8 * 220];
UBYTE highlighted_zone[8 * 220];
int _highlighted_index = -1;


bool _gfx_mode_switch;
int _game_selection_X_offset;
int _game_selection_Y_offset;

float _screen_factor;

struct tScreenGeometry
{
	long width;
	long height;
	long startoffset;
	long sourceoffset;
	long linestep;
	long pixelstep;
	long xSkipMask;
	long xLimit;
	long lineLimit;
};


tScreenGeometry geom[3];
tScreenGeometry portrait_geometry;

int currentScreenMode = 0;
int useMode = 0;
int maxMode = 2;
int filter_available;
int smooth_filter;
int toolbar_available;

UBYTE *toolbar = NULL;

#ifndef NEW_GAPI_CODE

UBYTE *noGAPI_video_buffer = NULL;
HDC noGAPI_compat;

#else


UBYTE *noGAPI_buffer;
HBITMAP noGAPI_bitmap;
BITMAPINFOHEADER noGAPI_bitmap_header;
HDC noGAPI_compat;

#endif

char noGAPI = 0;

/* Using vectorized function to save on branches */
typedef void (*tCls)();
typedef void (*tBlt)(UBYTE*);
typedef void (*tBlt_part)(UBYTE*,int, int, int, int, UBYTE*, int);
typedef void (*tSet_565)(INT16 *buffer, int pitch, int x, int y, int width, int height);

void mono_Cls();
void mono_Blt(UBYTE*);
void mono_Blt_part(UBYTE*, int, int, int, int, UBYTE*, int);
void mono_Set_565(INT16*, int, int, int, int, int);


void palette_Cls();
void palette_Blt(UBYTE*);
void palette_Blt_part(UBYTE*, int, int, int, int, UBYTE*, int);
void palette_Set_565(INT16*, int, int, int, int, int);


void hicolor_Cls();
void hicolor555_Blt(UBYTE*);
void hicolor555_Blt_part(UBYTE*, int, int, int, int, UBYTE*, int);
void hicolor555_Set_565(INT16*, int, int, int, int, int);

void hicolor565_Blt(UBYTE*);
void hicolor565_Blt_part(UBYTE*, int, int, int, int, UBYTE*, int);
//void hicolor565_Get_565(INT16*, int, int, int, int, int);
void hicolor565_Set_565(INT16*, int, int, int, int, int);

void noGAPI_Cls();
void noGAPI_Blt(UBYTE*);
void noGAPI_Blt_part(UBYTE*, int, int, int, int, UBYTE*, int);
void noGAPI_Set_565(INT16*, int, int, int, int, int);

//void NULL_Get_565(INT16*, int, int, int, int, int);
void NULL_Set_565(INT16*, int, int, int, int, int);

void palette_update();

void printString(const char *, int, int, int, int = -1, int = 220);
int drawString(const char *, int, int, int, int = -1);
static byte textfont[] = {0,0,99,1,226,8,4,8,6,8,6,0,0,0,0,0,0,0,0,0,0,0,8,2,1,8,0,0,0,0,0,0,0,0,0,0,0,0,4,3,7,8,7,7,8,4,5,5,8,7,4,7,3,8,7,7,7,7,8,7,7,7,7,7,3,4,7,5,7,7,8,7,7,7,7,7,7,7,7,5,7,7,
7,8,7,7,7,7,7,7,7,7,7,8,7,7,7,5,8,5,8,8,7,7,7,6,7,7,7,7,7,5,6,7,5,8,7,7,7,7,7,7,7,7,7,8,7,7,7,5,3,5,0,8,7,7,7,7,7,7,0,6,7,7,7,5,5,5,7,0,6,8,8,7,7,7,7,7,0,7,7,0,0,
0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,0,1,3,6,12,
24,62,3,0,128,192,96,48,24,124,192,0,0,3,62,24,12,6,3,1,0,192,124,24,48,96,192,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,237,74,72,0,0,0,0,0,128,128,128,0,0,0,0,0,0,0,0,0,0,0,0,0,60,66,153,161,161,153,66,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,96,96,96,96,0,0,96,0,102,102,102,0,0,0,0,0,102,102,255,102,255,102,102,0,24,62,96,60,6,124,24,0,98,102,12,24,48,102,70,0,60,102,60,56,103,102,63,0,96,48,16,0,0,0,0,0,24,48,96,96,96,48,24,0,96,48,24,24,24,48,96,0,
0,102,60,255,60,102,0,0,0,24,24,126,24,24,0,0,0,0,0,0,0,48,48,96,0,0,0,126,0,0,0,0,0,0,0,0,0,96,96,0,0,3,6,12,24,48,96,0,60,102,102,102,102,102,60,0,24,24,56,24,24,24,126,0,60,102,6,12,48,96,126,0,60,102,6,28,6,102,60,0,6,
14,30,102,127,6,6,0,126,96,124,6,6,102,60,0,60,102,96,124,102,102,60,0,126,102,12,24,24,24,24,0,60,102,102,60,102,102,60,0,60,102,102,62,6,102,60,0,0,0,96,0,0,96,0,0,0,0,48,0,0,48,48,96,14,24,48,96,48,24,14,0,0,0,120,0,120,0,0,0,112,24,
12,6,12,24,112,0,60,102,6,12,24,0,24,0,0,0,0,255,255,0,0,0,24,60,102,126,102,102,102,0,124,102,102,124,102,102,124,0,60,102,96,96,96,102,60,0,120,108,102,102,102,108,120,0,126,96,96,120,96,96,126,0,126,96,96,120,96,96,96,0,60,102,96,110,102,102,60,0,102,102,102,
126,102,102,102,0,120,48,48,48,48,48,120,0,30,12,12,12,12,108,56,0,102,108,120,112,120,108,102,0,96,96,96,96,96,96,126,0,99,119,127,107,99,99,99,0,102,118,126,126,110,102,102,0,60,102,102,102,102,102,60,0,124,102,102,124,96,96,96,0,60,102,102,102,102,60,14,0,124,102,102,124,
120,108,102,0,60,102,96,60,6,102,60,0,126,24,24,24,24,24,24,0,102,102,102,102,102,102,60,0,102,102,102,102,102,60,24,0,99,99,99,107,127,119,99,0,102,102,60,24,60,102,102,0,102,102,102,60,24,24,24,0,126,6,12,24,48,96,126,0,120,96,96,96,96,96,120,0,3,6,12,24,48,
96,192,0,120,24,24,24,24,24,120,0,0,0,0,0,0,219,219,0,0,0,0,0,0,0,0,255,102,102,102,0,0,0,0,0,0,0,60,6,62,102,62,0,0,96,96,124,102,102,124,0,0,0,60,96,96,96,60,0,0,6,6,62,102,102,62,0,0,0,60,102,126,96,60,0,0,14,24,62,24,24,
24,0,0,0,62,102,102,62,6,124,0,96,96,124,102,102,102,0,0,48,0,112,48,48,120,0,0,12,0,12,12,12,12,120,0,96,96,108,120,108,102,0,0,112,48,48,48,48,120,0,0,0,102,127,127,107,99,0,0,0,124,102,102,102,102,0,0,0,60,102,102,102,60,0,0,0,124,102,102,124,96,
96,0,0,62,102,102,62,6,6,0,0,124,102,96,96,96,0,0,0,62,96,60,6,124,0,0,24,126,24,24,24,14,0,0,0,102,102,102,102,62,0,0,0,102,102,102,60,24,0,0,0,99,107,127,62,54,0,0,0,102,60,24,60,102,0,0,0,102,102,102,62,12,120,0,0,126,12,24,48,126,0,
24,48,48,96,48,48,24,0,96,96,96,0,96,96,96,0,96,48,48,24,48,48,96,0,0,0,0,0,0,0,0,0,8,12,14,255,255,14,12,8,60,102,96,96,102,60,24,56,102,0,102,102,102,102,62,0,12,24,60,102,126,96,60,0,24,36,60,6,62,102,62,0,102,0,60,6,62,102,62,0,48,
24,60,6,62,102,62,0,0,0,0,0,0,0,0,0,0,60,96,96,96,60,24,56,24,36,60,102,126,96,60,0,102,0,60,102,126,96,60,0,48,24,60,102,126,96,60,0,0,216,0,112,48,48,120,0,48,72,0,112,48,48,120,0,96,48,0,112,48,48,120,0,102,24,60,102,126,102,102,0,0,0,
0,0,0,0,0,0,24,48,124,96,120,96,124,0,0,0,108,26,126,216,110,0,30,40,40,126,72,136,142,0,24,36,60,102,102,102,60,0,102,0,60,102,102,102,60,0,48,24,60,102,102,102,60,0,24,36,0,102,102,102,62,0,48,24,102,102,102,102,62,0,0,0,0,0,0,0,0,0,102,60,102,
102,102,102,60,0,102,0,102,102,102,102,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,24,60,6,62,102,62,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,28,54,54,124,102,102,124,64,0,0,0};

static tCls        pCls        = NULL;
static tBlt		   pBlt	       = NULL;
static tBlt_part   pBlt_part   = NULL;
static tSet_565    pSet_565	   = NULL;

static int _geometry_w;
static int _geometry_h;  
static int _saved_geometry_h;

HWND hWndMain;

typedef enum {
	DEVICE_HPC = 0,
	DEVICE_PPC,
	DEVICE_SMARTPHONE
} pdaDevice;


typedef enum {
	DEVICE_GAPI = 0,
	DEVICE_VIDEO,
	DEVICE_GDI
} gfxDevice;

typedef enum {
	VIDEO_565 = 0,
	VIDEO_555,
	VIDEO_PALETTED,
	VIDEO_MONO,
	VIDEO_DONT_CARE
} gfxOption;


unsigned char* pda_device_name[] = {
	(unsigned char*)"HPC",
	(unsigned char*)"PPC",
	(unsigned char*)"Smartphone"
};

unsigned char* gfx_device_name[] = {
	(unsigned char*)"GAPI",
	(unsigned char*)"Direct Video",
	(unsigned char*)"GDI"
};

unsigned char* gfx_device_options_name[] = {
	(unsigned char*)"565",
	(unsigned char*)"555",
	(unsigned char*)"paletted",
	(unsigned char*)"mono",
	(unsigned char*)"",
};

pdaDevice _pda_device;
gfxDevice _gfx_device;
gfxOption _gfx_option;


unsigned char *image_expand(unsigned char *src) {
	int i = 0;
	int j;
	int expanded = 0;

	for (;;) {
		if (!src[i]) {
			if (!src[i + 1])
				break;
			for (j=0; j<src[i + 1]; j++)
				decomp[expanded++] = src[i+2];
			i += 3;
		}
		else 
			decomp[expanded++] = src[i++];
	}

	return decomp;
}


void SetScreenMode(int mode)
{
	currentScreenMode = mode;
	if(currentScreenMode > maxMode)
		currentScreenMode = 0;
}

int GetScreenMode()
{
	return currentScreenMode;
}

void GraphicsSuspend()
{
	if(active)
	{
		active = 0;
		dynamicGXSuspend();
	}
}

void GraphicsResume()
{
	if(!active)
	{
		active = 1;
		dynamicGXResume();
	}

	palette_update();
}

void GraphicsOff(void)
{
	dynamicGXCloseDisplay();
	active = 0;
}

void SetScreenGeometry(int w, int h) {
	// Complain (loudly) if w > 320 and h > 240 ...
	/*
	if (w != 320 || h > 240) {
		char tempo[100];
		sprintf(tempo, "Unsupported screen geometry %dx%d", w, h);
		drawError(tempo);
		exit(1);
	}
	*/

	_geometry_w = w;
	_geometry_h = h;
	_saved_geometry_h = h;
	RestoreScreenGeometry();
}

void LimitScreenGeometry() {
	int limit;

	if (high_res) {
		if (wide_screen)
			limit = 440;
		else
			limit = 400;
	}
	else
		limit = 200;

	if (_geometry_h > limit) {
		geom[0].lineLimit = _geometry_w*limit;
		geom[1].lineLimit = _geometry_w*limit;
		geom[2].lineLimit = _geometry_w*limit;
		_geometry_h = limit;
	}
}

void RestoreScreenGeometry() {
	_geometry_h = _saved_geometry_h;
	geom[0].lineLimit = _geometry_w * _geometry_h;
	geom[1].lineLimit = _geometry_w * _geometry_h;
	geom[2].lineLimit = _geometry_w * _geometry_h;
}

int GraphicsOn(HWND hWndMain_param, bool gfx_mode_switch)
{
	hWndMain = hWndMain_param;

	_gfx_device = DEVICE_GAPI;

	if (!noGAPI && dynamicGXOpenDisplay(hWndMain, GX_FULLSCREEN) == GAPI_SIMU)
		_gfx_device = DEVICE_VIDEO;

	_gfx_mode_switch = gfx_mode_switch;

	if (_gfx_mode_switch) {
		_game_selection_X_offset = GAME_SELECTION_X_OFFSET;
		_game_selection_Y_offset = GAME_SELECTION_Y_OFFSET;
	}
	else {
		_game_selection_X_offset = 0;
		_game_selection_Y_offset = 0;
	}
	
	memset(&gxdp, 0, sizeof(gxdp));

	if (!noGAPI)
		gxdp = dynamicGXGetDisplayProperties();
	else {
			gxdp.cxWidth = GetSystemMetrics(SM_CXSCREEN);
			gxdp.cyHeight = GetSystemMetrics(SM_CYSCREEN);
			gxdp.ffFormat = 0xA8; 
	}

	// Possible Aero problem

	if (gxdp.cbxPitch == 61440 && 
		gxdp.cbyPitch == -2 && 
		gxdp.ffFormat == 0x18
	   ) {
		gxdp.cbxPitch = 640;
		gxdp.cbyPitch = -2;
		gxdp.ffFormat = kfDirect | kfDirect565;
	}

	if(noGAPI) 
	{
		HDC hdc;
		int width = (!high_res ? 320 : 640);
		int height = (!high_res ? 240 : 480);

		pCls = noGAPI_Cls;
		pBlt = noGAPI_Blt;
		pBlt_part = noGAPI_Blt_part;
		pSet_565 = noGAPI_Set_565;

		toolbar_available = 1;

		hdc = GetDC(hWndMain);
		noGAPI_compat = CreateCompatibleDC(hdc);
		ReleaseDC(hWndMain, hdc);


#ifndef NEW_GAPI_CODE
		// Init GDI
		noGAPI_video_buffer = (UBYTE*)malloc(320 * 240 * 2);
#else
	
		memset(&noGAPI_bitmap_header, 0, sizeof(BITMAPINFOHEADER));
		noGAPI_bitmap_header.biSize = sizeof(BITMAPINFOHEADER);

		noGAPI_bitmap_header.biWidth = GetSystemMetrics(SM_CXSCREEN);
		noGAPI_bitmap_header.biHeight = -GetSystemMetrics(SM_CYSCREEN); /* origin = top */
		
		noGAPI_bitmap_header.biPlanes = 1;
		noGAPI_bitmap_header.biBitCount = 24;
		noGAPI_bitmap_header.biCompression = BI_RGB; /* paletted fixme Jornada 820 ? */ 


		noGAPI_bitmap = CreateDIBSection(noGAPI_compat, (BITMAPINFO*)&noGAPI_bitmap_header, DIB_RGB_COLORS, (void**)&noGAPI_buffer, NULL, 0);
		if (!noGAPI_bitmap)
			exit(1);

		SelectObject(noGAPI_compat, noGAPI_bitmap);
#endif

		_gfx_device = DEVICE_GDI;
		_gfx_option = VIDEO_DONT_CARE;
	}
	else if(gxdp.ffFormat & kfDirect565)
	{
		pCls =    hicolor_Cls;
		pBlt =    hicolor565_Blt;
		pBlt_part = hicolor565_Blt_part;
		pSet_565 = hicolor565_Set_565;
		filter_available = 1;
		smooth_filter = 1;
		toolbar_available = 1;

		_gfx_option = VIDEO_565;
	}
	else if(gxdp.ffFormat & kfDirect555)
	{
		pCls =    hicolor_Cls;
		pBlt =    hicolor555_Blt;
		pBlt_part = hicolor555_Blt_part;
		pSet_565 = hicolor555_Set_565;
		filter_available = 1;
		smooth_filter = 1;
		toolbar_available = 1;

		_gfx_option = VIDEO_555;	
	}
	else if((gxdp.ffFormat & kfDirect) && (gxdp.cBPP <= 8))
	{
		pCls =  mono_Cls;
		pBlt =  mono_Blt;
		pBlt_part = mono_Blt_part;
		pSet_565 = mono_Set_565;
		
		if(gxdp.ffFormat & kfDirectInverted)
			invert = (1<<gxdp.cBPP)-1;
		colorscale = gxdp.cBPP < 8 ? 8-gxdp.cBPP : 0;

		if(gxdp.cBPP >= 4)
			filter_available = 1;

		toolbar_available = 1;

		_gfx_option = VIDEO_MONO;
	}
	else if(gxdp.ffFormat & kfPalette)
	{
		pCls =    palette_Cls;
		pBlt =    palette_Blt;
		pBlt_part = palette_Blt_part;
		pSet_565 = palette_Set_565;
		
		toolbar_available = 1;

		_gfx_option = VIDEO_PALETTED;
	}


	if(!pCls || !pBlt || (!smartphone && (gxdp.cxWidth < 240 || gxdp.cyHeight < 240))
	  )
	{
	// I don't believe there are devices that end up here
		GraphicsOff();
		return 1;
	}

	if (smartphone)
		toolbar_available = 0;


	// compute geometries
	
	// portrait
	portrait_geometry.width = gxdp.cxWidth; // 240
	portrait_geometry.height = gxdp.cyHeight; // 320
	portrait_geometry.startoffset = 0;
	portrait_geometry.sourceoffset = 0;
	portrait_geometry.linestep = gxdp.cbyPitch;
	portrait_geometry.pixelstep = gxdp.cbxPitch;
	portrait_geometry.xSkipMask = gxdp.cxWidth < 320 ? 0x00000003 : 0xffffffff;
	portrait_geometry.xLimit = 320; // skip 1/4
	portrait_geometry.lineLimit = 320*200;
	geom[0].width = gxdp.cxWidth;
	geom[0].height = gxdp.cyHeight;
	geom[0].startoffset = 0;
	geom[0].sourceoffset = 0;
	geom[0].linestep = gxdp.cbyPitch;
	geom[0].pixelstep = gxdp.cbxPitch;
	geom[0].xSkipMask = 0xffffffff;
	geom[0].xLimit = 240;
	geom[0].lineLimit = 320*240;

	_screen_factor = 1/2;
	
	// This will be overridden for the Smartphone implementation

	// left handed landscape
	geom[1].width = gxdp.cyHeight; // 320
	geom[1].height = gxdp.cxWidth; // 240
	geom[1].startoffset = gxdp.cbyPitch*(gxdp.cyHeight-1);
	geom[1].sourceoffset = 0;
	geom[1].linestep = gxdp.cbxPitch;
	geom[1].pixelstep = -gxdp.cbyPitch;
	geom[1].xSkipMask = 0xffffffff; 
	geom[1].xLimit = 320; // no skip
	geom[1].lineLimit = 320*200;	
	
	// right handed landscape
	geom[2].width = gxdp.cyHeight; // 320
	geom[2].height = gxdp.cxWidth; // 240
	geom[2].startoffset = gxdp.cbxPitch*(gxdp.cxWidth-1);
	geom[2].sourceoffset = 0;
	geom[2].linestep = -gxdp.cbxPitch;
	geom[2].pixelstep = gxdp.cbyPitch;
	geom[2].xSkipMask = 0xffffffff;
	geom[2].xLimit = 320; // no skip
	geom[2].lineLimit = 320*200;
	
	
	if(gxdp.cyHeight < 320 && !smartphone)
		maxMode = 0; // portrait only!
	
	active = 1;

	wide_screen = GetSystemMetrics(SM_CXSCREEN) >= 320;
	extra_wide_screen = GetSystemMetrics(SM_CYSCREEN) >= 480;

	if (wide_screen)
		_pda_device = DEVICE_HPC;
	else {
		if (smartphone)
			_pda_device = DEVICE_SMARTPHONE;
		else
			_pda_device = DEVICE_PPC;
	}

	return 0;
}

/* Find the best color match in the palette (limited to 'limit' entries) */ 
   UBYTE best_match(UBYTE r, UBYTE g, UBYTE b, int limit) 
   { 
           UBYTE best = 0; 
           int distance = 768; 
           int i, d; 
           for(i=0; i<limit; i++) 
           { 
           /* Manhattan distance for now. Not the best but rather fast */ 
                   d = abs(r-palRed[i])+abs(g-palGreen[i])+abs(b-palBlue[i]); 
                   if(d < distance) 
                   { 
                           distance = d; 
                           best = i; 
                   } 
           } 
    
           return (UBYTE)best; 
   } 


void palette_update()
{
	if(gxdp.ffFormat & kfPalette)
	{
		LOGPALETTE* ple = 
(LOGPALETTE*)malloc(sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*255);
		ple->palVersion = 0x300;
		ple->palNumEntries = 256;
		for(int i=0; i<236; i++) // first 10 and last ten belong to the system!
		{
			ple->palPalEntry[i+10].peBlue =  palBlue[i];
			ple->palPalEntry[i+10].peGreen = palGreen[i];
			ple->palPalEntry[i+10].peRed =   palRed[i];
			ple->palPalEntry[i+10].peFlags = PC_RESERVED;
		}
		HDC hDC = GetDC(hWndMain);
		GetSystemPaletteEntries(hDC, 0, 10, &(ple->palPalEntry[0]));
		GetSystemPaletteEntries(hDC, 246, 10, &(ple->palPalEntry[246]));
		HPALETTE hpal =	CreatePalette(ple);
		SelectPalette(hDC, hpal, FALSE);
		RealizePalette(hDC);
		DeleteObject((HGDIOBJ)hpal);
		ReleaseDC(hWndMain, hDC);
		free((void*)ple);

		for(i=0; i<20; i++) 
          staticTranslate[i] = best_match(palRed[i+236], palGreen[i+236], palBlue[i+236], 236)+10; 

		memset(color_match, 255, sizeof(color_match));

	}
}

void SetPalEntry(int ent, UBYTE r, UBYTE g, UBYTE b)
{
	int i;

	if (ent >= MAX_CLR)
		return;

	palRed[ent] = r;
	palGreen[ent] = g;
	palBlue[ent] = b;

	if(gxdp.ffFormat & kfDirect565)
		pal[ent] = COLORCONV565(r,g,b);
	else if(gxdp.ffFormat & kfDirect555)
		pal[ent] = COLORCONV555(r,g,b);
	else if(gxdp.ffFormat & kfDirect)
		pal[ent] = COLORCONVMONO(r,g,b);
	
}

/* *************** CLS IMPLEMENTATIONS ****************** */

void mono_Cls()
{
	int x, y;
	UBYTE* dst;
	UBYTE *scraddr;
	int linestep, pixelstep;
	UBYTE fillcolor;

	fillcolor = (gxdp.ffFormat & kfDirectInverted) ? 0xff : 0x00;

	pixelstep = geom[0].pixelstep;
	if(pixelstep == 0)
		return;
	linestep = (pixelstep > 0) ? -1 : 1;

	scraddr = (UBYTE*)dynamicGXBeginDraw();
	if(scraddr)
	{
		for(y=0; y<geom[0].height*gxdp.cBPP/8; y++)
		{
			dst = scraddr+geom[0].startoffset;
			for(x=0; x<geom[0].width; x++)
			{
				*dst = fillcolor;
				dst += pixelstep;
			}
			scraddr += linestep;
		}
		dynamicGXEndDraw();
	}
}

void palette_Cls()
{
	int x, y;
	UBYTE* dst;
	UBYTE *scraddr;
	scraddr = (UBYTE*)dynamicGXBeginDraw();
	if(scraddr)
	{
		for(y=0; y<geom[useMode].height; y++)
		{
			dst = scraddr+geom[useMode].startoffset;
			for(x=0; x<geom[useMode].width; x++)
			{
				*dst = 0;
				dst += geom[useMode].pixelstep;
			}
			scraddr += geom[useMode].linestep;
		}
		dynamicGXEndDraw();
	}
}

void hicolor_Cls()
{
	int x, y;
	UBYTE* dst;
	UBYTE *scraddr;
	scraddr = (UBYTE*)dynamicGXBeginDraw();
	if(scraddr)
	{
		for(y=0; y<geom[useMode].height; y++)
		{
			dst = scraddr+geom[useMode].startoffset;
			for(x=0; x<geom[useMode].width; x++)
			{
				*(unsigned short*)dst = 0;
				dst += geom[useMode].pixelstep;
			}
			scraddr += geom[useMode].linestep;
		}
		dynamicGXEndDraw();
	}
}


void Cls()
{
	pCls();
}
int counter = 0;

void drawSoundItem(int x, int y) {
	if (!sound_activated)
		pBlt_part(image_expand(item_soundOn), x, y, 32, 32, item_soundOn_colors, 0);
	else
		pBlt_part(image_expand(item_soundOff), x, y, 32, 32, item_soundOff_colors, 0);
}

void drawError(char *error) {
	FILE *file_error;

	file_error=fopen("scummvm_error.txt", "w");
	if (file_error) {
		fprintf(file_error, "%s\n", error);
		fclose(file_error);
	}
	int current_y = 80;
	Cls();
	pBlt_part(image_expand(item_toolbar), 0, 0, 320, 40, item_toolbar_colors, 0);
	pBlt_part(image_expand(item_bomb), 0, 0, 64, 67, item_bomb_colors, 0);
	memset(decomp, 0, sizeof(decomp));
	current_y = drawString("SCUMMVM ERROR !", 10, current_y, 2, 1);
	current_y += 10;
	current_y = drawString(error, 10, current_y, 8, 1);
	current_y += 10;
	current_y = drawString("Exiting in 10 seconds", 10, current_y, 8, 1);
	Sleep(10 * 1000);
}

void drawWait() {
	pBlt_part(image_expand(item_toolbar), 0, 0, 320, 40, item_toolbar_colors, 0);
	pBlt_part(image_expand(item_loading), 28, 10, 100, 25, item_loading_colors, 0);
}

void setGameSelectionPalette() {
	int i;

	if (_gfx_option != VIDEO_PALETTED)
		return;

	for (i=0; i<item_startup_colors_size; i++)
		SetPalEntry(i, item_startup_colors[3 * i], item_startup_colors[(3 * i) + 1],
						item_startup_colors[(3 * i) + 2]);

	palette_update();
}

void drawBlankGameSelection() {
	//int i;
	image_expand(item_startup);
	/* Store empty comment */
	memcpy(comment_zone, decomp + (206 * 220), 8 * 220); 
	pBlt_part(decomp, _game_selection_X_offset, _game_selection_Y_offset, 220, 250, item_startup_colors, 0);
}

void drawVideoDevice() {
	char info[100];

	sprintf(info, "Video device : %s %s - CVS %s PPC %s", gfx_device_name[_gfx_device], gfx_device_options_name[_gfx_option], gScummVMVersion, getBuildDate());
	drawString(info, 10, 0, 2, 1);
	/*
	printString(video_device, 10, 270, 2, 0);
	pBlt_part(decomp + (270 * 220), 0, 5, 220, 8, item_startup_colors, 1);
	*/

}

void drawCommentString(char *comment) {
	/* Erase old comment */	
	memcpy(decomp + (206 * 220), comment_zone, 8 * 220);
	/* Draw new comment */
	printString(comment, 24, 206, 2);
	pBlt_part(decomp + (206 * 220), _game_selection_X_offset, _game_selection_Y_offset + 206, 220, 8, item_startup_colors, 0);
}

void drawStandardString(char *game, int index) {
	printString(game, 24, 70 + (15 * index), 2);
	//pBlt_part(decomp, GAME_SELECTION_X_OFFSET + 24, GAME_SELECTION_Y_OFFSET + 70 + (12 * index), 220, 8, item_startup_colors);
	pBlt_part(decomp + ((70 + (15 * index)) * 220), _game_selection_X_offset, _game_selection_Y_offset + 70 + (15 * index), 220, 8, item_startup_colors, 0);	
}

void drawHighlightedString(char *game, int index) {
	/* Replace former highlighted string */
	if (_highlighted_index != -1) {
		memcpy(decomp + ((70 + (15 * _highlighted_index)) * 220), highlighted_zone, 8 * 220);
		pBlt_part(decomp + ((70 + (15 * _highlighted_index)) * 220), _game_selection_X_offset, _game_selection_Y_offset + 70 + (15 * _highlighted_index), 220, 8, item_startup_colors, 0);
	}
	/* Save non highlighted string */
	_highlighted_index = index;
	memcpy(highlighted_zone, decomp + ((70 + (15 * index)) * 220), 8 * 220);
	/* Draw new highlighted string */
	printString(game, 24, 70 + (15 * index), 2, 3);
	pBlt_part(decomp + ((70 + (15 * index)) * 220), _game_selection_X_offset, _game_selection_Y_offset + 70 + (15 * index), 220, 8, item_startup_colors, 0);
}

void resetLastHighlighted() {
	_highlighted_index = -1;
}

void reducePortraitGeometry() {
	if (_gfx_mode_switch)
		memcpy(&geom[0], &portrait_geometry, sizeof(tScreenGeometry));
}

void drawAllToolbar() {
	int x,y;
	int start_offset;

	if (high_res && wide_screen)
		start_offset = 440;
	else
		start_offset = 200;

	if (currentScreenMode || wide_screen) {

		if (draw_keyboard) {
			pBlt_part(image_expand(item_keyboard), 0, start_offset, 320, 40, item_keyboard_colors, 0);
		}
		else {
			pBlt_part(image_expand(item_toolbar), 0, start_offset, 320, 40, item_toolbar_colors, 0);
			x = 10;
			y = start_offset + 4;
			if (!is_simon)
				pBlt_part(image_expand(item_disk), x, y, 32, 32, item_disk_colors, 0);
			x += 40;
			pBlt_part(image_expand(item_skip), x, y, 32, 32, item_skip_colors, 0);
			x += 40;
			drawSoundItem(x, y);
			if (_gfx_mode_switch && !high_res) {
				x += 40;
				pBlt_part(image_expand(item_monkeyPortrait), x, y, 32, 32, 
						item_monkeyPortrait_colors, 0);
			}
		}
	}
	else {
			if (draw_keyboard) {
				pBlt_part(image_expand(item_keyboardPortrait), 0, 240, 320, 80,
							item_keyboardPortrait_colors, 0);
			}
			else {
			pBlt_part(image_expand(item_toolbarPortrait), 0, 240, 320, 80, 
						item_toolbarPortrait_colors, 0);
			/*drawToolbarItem(item_toolbarPortrait_colors, item_toolbarPortrait,
							0, 240, 240, 80);*/
			x = 10;
			y = 240;
			pBlt_part(image_expand(item_disk), x, y, 32, 32, item_disk_colors, 0);
			x += 40;
			pBlt_part(image_expand(item_skip), x, y, 32, 32, item_skip_colors, 0);
			x += 40;
			drawSoundItem(x, y);
			if (_gfx_mode_switch && !high_res) {
				x += 40;
				pBlt_part(image_expand(item_monkeyLandscape), x, y, 32, 32,
							item_monkeyLandscape_colors, 0);			
			}
		}
	}

	toolbar_drawn = true;
}

void redrawSoundItem() {
	drawSoundItem(10 + 40 + 40, (currentScreenMode || wide_screen ? 204 : 240));
}

bool isInBox(int x, int y, int x1, int y1, int x2, int y2) {
	return ((x >= x1 && y >= y1) && (x <= x2 && y <= y2));
}

ToolbarSelected getToolbarSelection (int x, int y) {
	int test_x, test_y;
	int offset = 1;

	/*
	if (!currentScreenMode)
		return ToolbarNone;
	*/

	if (!high_res && !(x >= 10 && y >= 204))
		return ToolbarNone;

	if (high_res && !(x >= 10 && y >= 444))
		return ToolbarNone;
	
	if (!high_res)
		test_y = (currentScreenMode || wide_screen ? 204 : 240);
	else
		test_y = 444;

	if (high_res && !wide_screen) 
			offset = 2;

	test_x = (10 * offset);

	if (isInBox(x, y, test_x, test_y, test_x + (32 * offset), test_y + (32 * offset)))
		return ToolbarSaveLoad;
	test_x += (40 * offset);
	if (isInBox(x, y, test_x, test_y, test_x + (32 * offset), test_y + (32 * offset)))
		return ToolbarSkip;
	test_x += (40 * offset);
	if (isInBox(x, y, test_x, test_y, test_x + (32 * offset), test_y + (32 * offset)))
		return ToolbarSound;
	if (_gfx_mode_switch && !high_res) {
		test_x += (40 * offset);
		if (isInBox(x, y, test_x, test_y, test_x + (32 * offset), test_y + (32 * offset)))
			return ToolbarMode;
	}
	return ToolbarNone;
}
	
/* ************************** BLT IMPLEMENTATION **************************** */

void Blt(UBYTE * scr_ptr) 
{
	pBlt(scr_ptr);

	//if (toolbar_available && currentScreenMode && !toolbar_drawn)
	if (toolbar_available && !toolbar_drawn && !hide_toolbar)
		drawAllToolbar();

}

void Blt_part(UBYTE * src_ptr, int x, int y, int width, int height, int pitch, bool check) {

	if (toolbar_available && !toolbar_drawn && !hide_toolbar)
		drawAllToolbar();

	if (y > _geometry_h)
		return;

	if (y + height > _geometry_h)
		height = _geometry_h - y;

	pBlt_part(src_ptr, x, y, width, height, NULL, pitch);

	//if (check && (y > _geometry_h || (y + height) > _geometry_h)) {
	//	toolbar_drawn = false;
	//}

}

void checkToolbar() {
	if (toolbar_available && !toolbar_drawn && !hide_toolbar)
		drawAllToolbar();
}

/* *************************** MONO DISPLAY ********************************* */

#define ADVANCE_PARTIAL(address, step) \
	bitshift += gxdp.cBPP;             \
	if(bitshift >= 8)                  \
	{                                  \
		bitshift = 0;                  \
		bitmask = (1<<gxdp.cBPP)-1;    \
		address += step;               \
	}                                  \
	else                               \
		bitmask <<= gxdp.cBPP;

#define ADVANCE_REV_PARTIAL(address, step)        \
	bitshift -= gxdp.cBPP;                        \
	if(bitshift < 0)                              \
	{                                             \
		bitshift = 8-gxdp.cBPP;                   \
		bitmask = ((1<<gxdp.cBPP)-1)<<bitshift;   \
		address += step;                          \
	}                                             \
	else                                          \
		bitmask >>= gxdp.cBPP;


void mono_Set_565(INT16 *buffer, int pitch, int x, int y, int width, int height) {

	static UBYTE *scraddr;
	static UBYTE *dst;
	static long pixelstep;
	static long linestep;
	static UBYTE bitmask;
	static int   bitshift;
	static long skipmask;

	scraddr = (UBYTE*)dynamicGXBeginDraw();
	pixelstep = geom[useMode].pixelstep;
	linestep = (pixelstep > 0) ? -1 : 1;
	skipmask = geom[useMode].xSkipMask;
	bitshift = 0;
	bitmask = (1<<gxdp.cBPP)-1;

	if(scraddr)
	{
		int lines = 0;
		int current = 0;

		scraddr += geom[useMode].startoffset;
		scraddr += y * linestep;
	
		while(lines != height)
			{
				int i;
				current = 0;

				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)					
					dst += pixelstep;

				/* Turtle warning !!! */

				for (i=0; i<width; i++) {
					if (skipmask == 0xffffffff || (long)i & skipmask) {
						*dst = ((*dst)&~bitmask)|(COLORCONVMONO(
										RED_FROM_565(*(buffer + i)), 
										GREEN_FROM_565(*(buffer + i)),
										BLUE_FROM_565(*(buffer + i))) << bitshift);

						dst += pixelstep;
					}
				}

				ADVANCE_PARTIAL(scraddr, linestep);

				buffer += pitch;
				lines++;
			}
	}

	dynamicGXEndDraw();
}


void mono_Blt(UBYTE *src_ptr) {
	mono_Blt_part(src_ptr, 0, 0, _geometry_w, _geometry_h, NULL, 0);
}


void mono_Blt_part(UBYTE * scr_ptr, int x, int y, int width, int height,
				    UBYTE * own_palette, int pitch)
{
// Mono blit routines contain good deal of voodoo
	static UBYTE *src;
	static UBYTE *dst;
	static UBYTE *scraddr;
	static UBYTE *scr_ptr_limit;
	static UBYTE *src_limit;
	static long pixelstep;
	static long linestep;
	static long skipmask;

// Special code is used to deal with packed pixels in monochrome mode
	static UBYTE bitmask;
	static int   bitshift;

	if(!active)
	{
		Sleep(100);
		return;
	}

	/* Update screen mode, also thread protection by doing this */
	if(useMode != currentScreenMode)
	{
		useMode = currentScreenMode;
		pCls();
	}

	pixelstep = geom[useMode].pixelstep;
	linestep = geom[useMode].linestep;
	skipmask = geom[useMode].xSkipMask;

	scraddr = (UBYTE*)dynamicGXBeginDraw();

	if(pixelstep)
	{
	// this will work on mono iPAQ and @migo, don't know about any others
		linestep = (pixelstep > 0) ? -1 : 1;

		bitshift = 0;
		bitmask = (1<<gxdp.cBPP)-1;

		if(scraddr)
		{
			scraddr += geom[useMode].startoffset;
			scr_ptr += geom[useMode].sourceoffset;
			//scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
			//src_limit = scr_ptr + geom[useMode].xLimit;

			/* Update offsets to the current line */
			scraddr += y * linestep;
			scr_ptr_limit = scr_ptr + (pitch ? pitch : width) * height;
			src_limit = scr_ptr + width;

			/*
			 if (scr_ptr_limit > scr_ptr + geom[useMode].lineLimit)
                scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
			*/

			/* CMI rendering */
            if (high_res && !own_palette && !extra_wide_screen) {
				while(scr_ptr < scr_ptr_limit)
				{
					int i;

					src = scr_ptr;
					dst = scraddr;

					/* skip non updated pixels for this line */
					for (i=0; i < x; i++)
						dst += pixelstep;

					while(src < src_limit)
					{
	
						*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
						dst += pixelstep;
						src +=2;
					}

					ADVANCE_PARTIAL(scraddr, linestep);

					scr_ptr += (pitch ? 2 * pitch : 2 * width);
					src_limit += (pitch ? 2 * pitch : 2 * width);
				}
			}
			else
			/* Internal pixel loops */
			if(skipmask == 3 && (smooth_filter) && gxdp.cBPP >= 4)
			{
				while(scr_ptr < scr_ptr_limit)
				{
					int i;

					src = scr_ptr;
					dst = scraddr;

					/* skip non updated pixels for this line */
					for (i=0; i < x; i++)
						dst += pixelstep;

					while(src < src_limit)
					{
						UBYTE r, g, b;

						if (!own_palette) {
							r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
							g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
							b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;
						} else {
							r = (3 * own_palette[3 * *(src + 0)] + 
									own_palette[3 * *(src + 1)]) >> 2;
							g = (3 * own_palette[3 * *(src + 0) + 1] +
									own_palette[3 * *(src + 1) + 1]) >> 2;
							b = (3 * own_palette[3 * *(src + 0) + 2] +
									own_palette[3 * *(src + 1) + 2]) >> 2;
						}
						
						
						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

						dst += pixelstep;

						if (!own_palette) {
							r = (palRed[*(src+1)] + palRed[*(src+2)])>>1;
							g = (palGreen[*(src+1)] + palGreen[*(src+2)])>>1;
							b = (palBlue[*(src+1)] + palBlue[*(src+2)])>>1;
						}
						else {
							r = (own_palette[3 * *(src + 1)] + 
									own_palette[3 * *(src + 2)]) >> 2;
							g = (own_palette[3 * *(src + 1) + 1] +
									own_palette[3 * *(src + 2) + 1]) >> 2;
							b = (own_palette[3 * *(src + 1) + 2] +
									own_palette[3 * *(src + 2) + 2]) >> 2;
						}

						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

						dst += pixelstep;

						if (!own_palette) {
							r = (palRed[*(src+2)] + 3*palRed[*(src+3)])>>2;
							g = (palGreen[*(src+2)] + 3*palGreen[*(src+3)])>>2;
							b = (palBlue[*(src+2)] + 3*palBlue[*(src+3)])>>2;
						}
						else {
							r = (own_palette[3 * *(src + 2)] + 
									3 * own_palette[3 * *(src + 3)]) >> 2;
							g = (own_palette[3 * *(src + 2) + 1] +
									3 * own_palette[3 * *(src + 3) + 1]) >> 2;
							b = (own_palette[3 * *(src + 2) + 2] +
									3 * own_palette[3 * *(src + 3) + 2]) >> 2;

						}

						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

						dst += pixelstep;

						src += 4;
					}

					ADVANCE_PARTIAL(scraddr, linestep);

					scr_ptr += (pitch ? pitch : width);
					src_limit += (pitch ? pitch : width);
				}
			}
			else if(skipmask != 0xffffffff)
			{
				while(scr_ptr < scr_ptr_limit)
				{
					int i;

					src = scr_ptr;
					dst = scraddr;

					/* skip non updated pixels for this line */
					for (i=0; i < x; i++)
						dst += pixelstep;

					while(src < src_limit)
					{
						if((long)src & skipmask)
						{
							if (!own_palette)
								*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							else
								*dst = ((*dst)&~bitmask)|(COLORCONVMONO(
									own_palette[3 * *src], 
									own_palette[(3 * *src) + 1],
									own_palette[(3 * *src) + 2]) << bitshift);
							dst += pixelstep;
						}
						src ++;
					}

					ADVANCE_PARTIAL(scraddr, linestep);

					scr_ptr += (pitch ? pitch : width);
					src_limit += (pitch ? pitch : width);
				}
			}
			else
			{
				while(scr_ptr < scr_ptr_limit)
				{
					int i;

					src = scr_ptr;
					dst = scraddr;

					/* skip non updated pixels for this line */
					for (i=0; i < x; i++)
						dst += pixelstep;

					while(src < src_limit)
					{
							if (!own_palette)
								*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							else
								*dst = ((*dst)&~bitmask)|(COLORCONVMONO(
									own_palette[3 * *src], 
									own_palette[(3 * *src) + 1],
									own_palette[(3 * *src) + 2]) << bitshift);
						dst += pixelstep;
						src ++;
					}

					ADVANCE_PARTIAL(scraddr, linestep);

					scr_ptr += (pitch ? pitch : width);
					src_limit += (pitch ? pitch : width);
				}
			}
		}
	}
	else
	{
	// Filtering is not implemented in this mode. Not needed by current devices anyway
		pixelstep = (linestep > 0) ? 1 : -1;

		if(scraddr)
		{

			scraddr += geom[useMode].startoffset;
			scr_ptr += geom[useMode].sourceoffset;
			//scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
			//src_limit = scr_ptr + geom[useMode].xLimit;

			/* Update offsets to the current line */
			scraddr += y * linestep;
			scr_ptr_limit = scr_ptr + (pitch ? pitch : width) * height;
			src_limit = scr_ptr + width;

            if (scr_ptr_limit > scr_ptr + geom[useMode].lineLimit)
                scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;

			/* CMI rendering */
            if (high_res && !own_palette && !extra_wide_screen) {
				if(pixelstep > 0)
				{
					bitshift = 8-gxdp.cBPP;
					bitmask = ((1<<gxdp.cBPP)-1)<<bitshift;

					while(scr_ptr < scr_ptr_limit)
					{
						int i;

						src = scr_ptr;
						dst = scraddr;
						dst -= (linestep-pixelstep);

						/* skip non updated pixels for this line */
						for (i=0; i < x; i++)
							ADVANCE_REV_PARTIAL(dst, pixelstep);

						while(src < src_limit)
						{
							*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							ADVANCE_REV_PARTIAL(dst, pixelstep);
							src += 2;
						}

						scraddr += linestep;

						scr_ptr += (pitch ? 2 * pitch : 2 * width);
						src_limit += (pitch ? 2 * pitch : 2 * width);
					}
				}
				else {
					bitshift = 0;
					bitmask = (1<<gxdp.cBPP)-1;

					while(scr_ptr < scr_ptr_limit)
					{
						int i;

						src = scr_ptr;
						dst = scraddr;

						/* skip non updated pixels for this line */
						for (i=0; i < x; i++)
							ADVANCE_PARTIAL(dst, pixelstep);

						while(src < src_limit)
						{
							*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							ADVANCE_PARTIAL(dst, pixelstep);
							src += 2;
						}

						scraddr += linestep;

						scr_ptr += (pitch ? 2 * pitch : 2 * width);
						src_limit += (pitch ? 2 * pitch : 2 * width);
					}				
				}
			}
			else
			
			if(skipmask != 0xffffffff)
			{
				if(pixelstep > 0)
				{
					bitshift = 8-gxdp.cBPP;
					bitmask = ((1<<gxdp.cBPP)-1)<<bitshift;

					while(scr_ptr < scr_ptr_limit)
					{
						int i;

						src = scr_ptr;
						dst = scraddr;
						dst -= (linestep-pixelstep);

						/* skip non updated pixels for this line */
						for (i=0; i < x; i++)
							ADVANCE_REV_PARTIAL(dst, pixelstep);

						while(src < src_limit)
						{
							if((long)src & skipmask)
							{
								if (!own_palette)
									*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
								else
									*dst = ((*dst)&~bitmask)|(COLORCONVMONO(
										own_palette[3 * *src], 
										own_palette[(3 * *src) + 1],
										own_palette[(3 * *src) + 2]) << bitshift);
								ADVANCE_REV_PARTIAL(dst, pixelstep);
							}
							src ++;
						}

						scraddr += linestep;

						scr_ptr += (pitch ? pitch : width);
						src_limit += (pitch ? pitch : width);
					}
				}
				else
				{
					bitshift = 0;
					bitmask = (1<<gxdp.cBPP)-1;

					while(scr_ptr < scr_ptr_limit)
					{
						int i;

						src = scr_ptr;
						dst = scraddr;

						/* skip non updated pixels for this line */
						for (i=0; i < x; i++)
							ADVANCE_PARTIAL(dst, pixelstep);

						while(src < src_limit)
						{
							if((long)src & skipmask)
							{
								if (!own_palette)
									*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
								else
									*dst = ((*dst)&~bitmask)|(COLORCONVMONO(
										own_palette[3 * *src], 
										own_palette[(3 * *src) + 1],
										own_palette[(3 * *src) + 2]) << bitshift);
								ADVANCE_PARTIAL(dst, pixelstep);
							}
							src ++;
						}

						scraddr += linestep;

						scr_ptr += (pitch ? pitch : width);
						src_limit += (pitch ? pitch : width);
					}
				}
			}
			else
			{
				if(pixelstep > 0)
				{
					bitshift = 8-gxdp.cBPP;
					bitmask = ((1<<gxdp.cBPP)-1)<<bitshift;

					while(scr_ptr < scr_ptr_limit)
					{
						int i;

						src = scr_ptr;
						dst = scraddr;
						dst -= (linestep-pixelstep);

						/* skip non updated pixels for this line */
						for (i=0; i < x; i++)
							ADVANCE_REV_PARTIAL(dst, pixelstep);

						while(src < src_limit)
						{
							if (!own_palette)
								*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							else
								*dst = ((*dst)&~bitmask)|(COLORCONVMONO(
										own_palette[3 * *src], 
										own_palette[(3 * *src) + 1],
										own_palette[(3 * *src) + 2]) << bitshift);
							ADVANCE_REV_PARTIAL(dst, pixelstep);
							src ++;
						}

						scraddr += linestep;

						scr_ptr += (pitch ? pitch : width);
						src_limit += (pitch ? pitch : width);
					}
				}
				else
				{
					bitshift = 0;
					bitmask = (1<<gxdp.cBPP)-1;

					while(scr_ptr < scr_ptr_limit)
					{
						int i;

						src = scr_ptr;
						dst = scraddr;

						/* skip non updated pixels for this line */
						for (i=0; i < x; i++)
							ADVANCE_PARTIAL(dst, pixelstep);


						while(src < src_limit)
						{
							if (!own_palette)
								*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							else
								*dst = ((*dst)&~bitmask)|(COLORCONVMONO(
										own_palette[3 * *src], 
										own_palette[(3 * *src) + 1],
										own_palette[(3 * *src) + 2]) << bitshift);
							ADVANCE_PARTIAL(dst, pixelstep);
							src ++;
						}

						scraddr += linestep;

						scr_ptr += (pitch ? pitch : width);
						src_limit += (pitch ? pitch : width);
					}
				}
			}
		}
	}
	dynamicGXEndDraw();
}

/* *************************** PALETTED DISPLAY ********************************* */

void palette_Set_565(INT16 *buffer_param, int pitch, int x, int y, int width, int height) {

	static UBYTE *scraddr;
	static UBYTE *dst;
	static long pixelstep;
	static long linestep;
	static long skipmask;
	static unsigned char *color_match_2;
	static UINT16 *buffer;

	buffer = (UINT16*)buffer_param;

	scraddr = (UBYTE*)dynamicGXBeginDraw();

	pixelstep = geom[useMode].pixelstep;
	linestep = geom[useMode].linestep;
	skipmask = geom[useMode].xSkipMask;

	color_match_2 = (unsigned char*)malloc(0xffff);
	if (color_match_2) 
		memset(color_match_2, 0xff, 0xffff);


	if(scraddr)
	{
		int lines = 0;
		int current = 0;

		scraddr += geom[useMode].startoffset;
		scraddr += y * linestep;
	
		while(lines != height)
			{
				int i;
				current = 0;

				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)					
					dst += pixelstep;

				/* Turtle warning !!! */

				for (i=0; i<width; i++) {
					if (skipmask == 0xffffffff || (long)i & skipmask) {
						if (color_match_2) {
							if (color_match_2[*(buffer + i)] == 255) 
								color_match_2[*(buffer + i)] = best_match(
											RED_FROM_565(*(buffer + i)),
										    GREEN_FROM_565(*(buffer + i)), 
										    BLUE_FROM_565(*(buffer + i)), 236) + 10;
							*dst = color_match_2[*(buffer + i)];
						}
						else {
							*dst = best_match(
										RED_FROM_565(*(buffer + i)),
										GREEN_FROM_565(*(buffer + i)), 
										BLUE_FROM_565(*(buffer + i)), 236) + 10;
						}
						dst += pixelstep;
					}
				}

				buffer += pitch;
				scraddr += linestep;
				lines++;
			}
	}

	if (color_match_2)
		free(color_match_2);
	dynamicGXEndDraw();
}


void palette_Blt(UBYTE *src_ptr) {
	palette_Blt_part(src_ptr, 0, 0, _geometry_w, _geometry_h, NULL, 0);
}

void palette_Blt_part(UBYTE * scr_ptr,int x, int y, int width, int height,
				    UBYTE * own_palette, int pitch)
{
	static UBYTE *src;
	static UBYTE *dst;
	static UBYTE *scraddr;
	static UBYTE *scr_ptr_limit;
	static UBYTE *src_limit;
	static long pixelstep;
	static long linestep;
	static long skipmask;

// Special code is used to deal with packed pixels in monochrome mode
	static UBYTE bitmask;
	static int   bitshift;

	if(!active)
	{
		Sleep(100);
		return;
	}

	/* Update screen mode, also thread protection by doing this */
	if(useMode != currentScreenMode)
	{
		useMode = currentScreenMode;
		pCls();
	}

	pixelstep = geom[useMode].pixelstep;
	linestep = geom[useMode].linestep;
	skipmask = geom[useMode].xSkipMask;

	scraddr = (UBYTE*)dynamicGXBeginDraw();
	if(scraddr)
	{

		scraddr += geom[useMode].startoffset;
		scr_ptr += geom[useMode].sourceoffset;
		//scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		//src_limit = scr_ptr + geom[useMode].xLimit;

		/* Update offsets to the current line */
        scraddr += y * linestep;
		scr_ptr_limit = scr_ptr + (pitch ? pitch : width) * height;
        src_limit = scr_ptr + width;

		/*
		if (scr_ptr_limit > scr_ptr + geom[useMode].lineLimit)
            scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		*/

		/* CMI rendering */
        if (high_res && !own_palette && !extra_wide_screen) {
			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;

				while(src < src_limit)
				{
					if (*src < 236)
						*dst = *src + 10;
					else
						*dst = staticTranslate[*src-236];
					
					dst += pixelstep;

					src += 2;
				}
				scraddr += linestep;
				scr_ptr += (pitch ? 2 * pitch : 2 * width);
				src_limit += (pitch ? 2 * pitch : 2 * width);
			}
		}
		else
		/* Internal pixel loops */
		if(skipmask != 0xffffffff)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;

				while(src < src_limit)
				{
					if((long)src & skipmask)
					{
						if (!own_palette) {
							if (*src < 236)
								*dst = *src + 10;
							else
								*dst = staticTranslate[*src-236];
						} else {
							// Turtle warning !!!
							if (color_match[*src] == 255)
								color_match[*src] = 
									best_match(own_palette[(3 * *src)],
										own_palette[(3 * *src) + 1], 
										own_palette[(3 * *src) + 2], 236) + 10;
							*dst = color_match[*src];
						}
						dst += pixelstep;
					}
					src ++;
				}
				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);
			}
		}
		else
		{
			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;


				while(src < src_limit)
				{
					if (!own_palette) {
						if (*src < 236)
							*dst = *src + 10; 
						else
							*dst = staticTranslate[*src-236];
					}
					else {
							// Turtle warning !!!
							if (color_match[*src] == 255)
								color_match[*src] = 
									best_match(own_palette[(3 * *src)],
										own_palette[(3 * *src) + 1], 
										own_palette[(3 * *src) + 2], 236) + 10;
							*dst = color_match[*src];
					}
					dst += pixelstep;
					src ++;
				}
				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);
			}
		}

		dynamicGXEndDraw();
	}
}

/* ********************************* 555 DISPLAY ********************************* */

void hicolor555_Set_565(INT16 *buffer, int pitch, int x, int y, int width, int height) {

	static UBYTE *scraddr;
	static UBYTE *dst;
	static long pixelstep;
	static long linestep;
	static long skipmask;
	
	scraddr = (UBYTE*)dynamicGXBeginDraw();

	pixelstep = geom[useMode].pixelstep;
	linestep = geom[useMode].linestep;
	skipmask = geom[useMode].xSkipMask;

	if(scraddr)
	{
		int lines = 0;
		int current = 0;

		scraddr += geom[useMode].startoffset;
		scraddr += y * linestep;
	
		while(lines != height)
			{
				int i;
				current = 0;

				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)					
					dst += pixelstep;

				/* Turtle warning !!! */

				for (i=0; i<width; i++) {
					if (skipmask == 0xffffffff || (long)i & skipmask) {

						*(unsigned short*)dst = COLORCONV555(
							RED_FROM_565(*(buffer + i)), GREEN_FROM_565(*(buffer + i)), BLUE_FROM_565(*(buffer + i))
						);
						dst += pixelstep;
					}
				}

				buffer += pitch;
				scraddr += linestep;
				lines++;
			}
	}

	dynamicGXEndDraw();
}


void hicolor555_Blt(UBYTE *src_ptr) {
	hicolor555_Blt_part(src_ptr, 0, 0, _geometry_w, _geometry_h, NULL, 0);
}


void hicolor555_Blt_part(UBYTE * scr_ptr,int x, int y, int width, int height,
				    UBYTE * own_palette, int pitch)
{
	static UBYTE *src;
	static UBYTE *dst;
	static UBYTE *scraddr;
	static UBYTE *scr_ptr_limit;
	static UBYTE *src_limit;
	static long pixelstep;
	static long linestep;
	static long skipmask;

	if(!active)
	{
		Sleep(100);
		return;
	}

	/* Update screen mode, also thread protection by doing this */
	if(useMode != currentScreenMode)
	{
		useMode = currentScreenMode;
		pCls();
	}

	pixelstep = geom[useMode].pixelstep;
	linestep = geom[useMode].linestep;
	skipmask = geom[useMode].xSkipMask;

	scraddr = (UBYTE*)dynamicGXBeginDraw();
	if(scraddr)
	{

		scraddr += geom[useMode].startoffset;
		scr_ptr += geom[useMode].sourceoffset;
		//scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		//src_limit = scr_ptr + geom[useMode].xLimit;

		/* Update offsets to the current line */
        scraddr += y * linestep;
        scr_ptr_limit = scr_ptr + (pitch ? pitch : width) * height;

		/*
		if (scr_ptr_limit > scr_ptr + geom[useMode].lineLimit)
            scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		*/

        src_limit = scr_ptr + width;

		/* CMI rendering */
		if (high_res && !own_palette && !extra_wide_screen) {

			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;
				
				while(src < src_limit)
				{
					UBYTE r, g, b;
					
					r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
					g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
					b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;
					
					*(unsigned short*)dst = COLORCONV555(r,g,b);

					dst += pixelstep;

					src += 2;
				}

				scraddr += linestep;
				scr_ptr += (pitch ? 2 * pitch : 2 * width);
				src_limit += (pitch ? 2 * pitch : 2 * width);
			}
		}
		else
		/* Internal pixel loops */
		if(skipmask == 3 && smooth_filter)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;

				while(src < src_limit)
				{
					UBYTE r, g, b;
					if (!own_palette) {
						r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
						g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
						b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;
					} else {
						r = (3 * own_palette[3 * *(src + 0)] + 
							     own_palette[3 * *(src + 1)]) >> 2;
						g = (3 * own_palette[3 * *(src + 0) + 1] +
								 own_palette[3 * *(src + 1) + 1]) >> 2;
						b = (3 * own_palette[3 * *(src + 0) + 2] +
							     own_palette[3 * *(src + 1) + 2]) >> 2;
					}

					*(unsigned short*)dst = COLORCONV555(r,g,b);

					dst += pixelstep;

					if (!own_palette) {
						r = (palRed[*(src+1)] + palRed[*(src+2)])>>1;
						g = (palGreen[*(src+1)] + palGreen[*(src+2)])>>1;
						b = (palBlue[*(src+1)] + palBlue[*(src+2)])>>1;
					}
					else {
						r = (own_palette[3 * *(src + 1)] + 
							     own_palette[3 * *(src + 2)]) >> 2;
						g = (own_palette[3 * *(src + 1) + 1] +
								 own_palette[3 * *(src + 2) + 1]) >> 2;
						b = (own_palette[3 * *(src + 1) + 2] +
							     own_palette[3 * *(src + 2) + 2]) >> 2;
					}

					*(unsigned short*)dst = COLORCONV555(r,g,b);

					dst += pixelstep;

					if (!own_palette) {
						r = (palRed[*(src+2)] + 3*palRed[*(src+3)])>>2;
						g = (palGreen[*(src+2)] + 3*palGreen[*(src+3)])>>2;
						b = (palBlue[*(src+2)] + 3*palBlue[*(src+3)])>>2;
					}
					else {
						r = (own_palette[3 * *(src + 2)] + 
							     3 * own_palette[3 * *(src + 3)]) >> 2;
						g = (own_palette[3 * *(src + 2) + 1] +
								 3 * own_palette[3 * *(src + 3) + 1]) >> 2;
						b = (own_palette[3 * *(src + 2) + 2] +
							     3 * own_palette[3 * *(src + 3) + 2]) >> 2;

					}

					*(unsigned short*)dst = COLORCONV555(r,g,b);

					dst += pixelstep;

					src += 4;
				}
				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);
			}
		}
		else if(skipmask != 0xffffffff)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;


				while(src < src_limit)
				{
					if((long)src & skipmask)
					{
						if (!own_palette)
							*(unsigned short*)dst = pal[*src];
						else
							*(unsigned short*)dst = 
								COLORCONV555(own_palette[3 * *src],
									own_palette[(3 * *src) + 1], 
									own_palette[(3 * *src) + 2]);
						dst += pixelstep;
					}
					src ++;
				}
				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);
			}
		}
		else
		{
			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;


				while(src < src_limit)
				{
					if (!own_palette)
							*(unsigned short*)dst = pal[*src];
						else
							*(unsigned short*)dst = 
								COLORCONV555(own_palette[3 * *src],
									own_palette[(3 * *src) + 1], 
									own_palette[(3 * *src) + 2]);
						dst += pixelstep;
						src ++;
				}

				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);
			}
		}

		dynamicGXEndDraw();
	}
}

/* ********************************* 565 DISPLAY ********************************* */


void hicolor565_Blt(UBYTE *src_ptr) {
	hicolor565_Blt_part(src_ptr, 0, 0, _geometry_w, _geometry_h, NULL, 0);
}


void hicolor565_Set_565(INT16 *buffer, int pitch, int x, int y, int width, int height) {

	static UBYTE *scraddr;
	static UBYTE *dst;
	static long pixelstep;
	static long linestep;
	static long skipmask;

	scraddr = (UBYTE*)dynamicGXBeginDraw();

	pixelstep = geom[useMode].pixelstep;
	linestep = geom[useMode].linestep;
	skipmask = geom[useMode].xSkipMask;

	if(scraddr)
	{
		int lines = 0;
		int current = 0;

		scraddr += geom[useMode].startoffset;
		scraddr += y * linestep;

		if (smartphone) {
		while(lines != height)
			{
				int i;
				current = 0;

				dst = scraddr;

				/* skip non updated pixels for this line */

				dst += x * pixelstep;

				for (i=0; i<width; i+=3) {

					register UBYTE r,g,b;

					register currentRed = RED_FROM_565(buffer[i]);
					register currentGreen = GREEN_FROM_565(buffer[i]);
					register currentBlue = BLUE_FROM_565(buffer[i]);
					register currentRed1 = RED_FROM_565(buffer[i+1]);
					register currentGreen1 = GREEN_FROM_565(buffer[i+1]);
					register currentBlue1 = BLUE_FROM_565(buffer[i+1]);
					register currentRed2 = RED_FROM_565(buffer[i+2]);
					register currentGreen2 = GREEN_FROM_565(buffer[i+2]);
					register currentBlue2 = BLUE_FROM_565(buffer[i+2]);


					r = (3 * currentRed + currentRed1)>>2;
					g = (3 * currentGreen + currentGreen1)>>2;
					b = (3 * currentBlue + currentBlue1)>>2;

					*(unsigned short*)dst = COLORCONV565(r, g, b);
					dst += pixelstep;

					r = (currentRed1 + currentRed2)>>1;
					g = (currentGreen1 + currentGreen2)>>1;
					b = (currentBlue1 + currentBlue2)>>1;

					*(unsigned short*)dst = COLORCONV565(r, g, b);
					dst += pixelstep;
				}
				
				buffer += pitch;
				scraddr += linestep;
				lines++;
				if (lines != 0 && !(lines % 7)) {
					lines++;
					buffer += pitch;
				}
			}
		
		}
		else {

		while(lines != height)
			{
				int i;
				current = 0;

				dst = scraddr;


				/* skip non updated pixels for this line */

				dst += x * pixelstep;

				for (i=0; i<width; i++) {
					if (skipmask == 0xffffffff || (long)i & skipmask) {
						*(unsigned short*)dst = buffer[i];
						dst += pixelstep;
					}
				}
				//memcpy(dst, buffer, width * 2);

				buffer += pitch;
				scraddr += linestep;
				lines++;
			}
	}

	}

	dynamicGXEndDraw();
}

int getColor565 (int color) {
	return COLORCONV565(palRed[color], palGreen[color], palBlue[color]);
}

void hicolor565_Blt_part(UBYTE * scr_ptr, int x, int y, int width, int height,
						 UBYTE * own_palette, int pitch)
{
	static UBYTE *src;
	static UBYTE *dst;
	static UBYTE *scraddr;
	static UBYTE *scr_ptr_limit;
	static UBYTE *src_limit;
	static long pixelstep;
	static long linestep;
	static long skipmask;

	if(!active)
	{
		Sleep(100);
		return;
	}

	/* Update screen mode, also thread protection by doing this */
	if(useMode != currentScreenMode)
	{
		useMode = currentScreenMode;
		pCls();
	}

	pixelstep = geom[useMode].pixelstep;
	linestep = geom[useMode].linestep;
	skipmask = geom[useMode].xSkipMask;

	scraddr = (UBYTE*)dynamicGXBeginDraw();

	if(scraddr)
	{

		scraddr += geom[useMode].startoffset;
		scr_ptr += geom[useMode].sourceoffset;
		//scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		//src_limit = scr_ptr + geom[useMode].xLimit;

		/* Update offsets to the current line */
		scraddr += y * linestep;
		scr_ptr_limit = scr_ptr + (pitch ? pitch : width) * height;

		/*
		if (scr_ptr_limit > scr_ptr + geom[useMode].lineLimit)
			scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		*/

		src_limit = scr_ptr + width;

		/* CMI rendering */
		if (high_res && !own_palette && !extra_wide_screen) {

			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;
				
				while(src < src_limit)
				{
					UBYTE r, g, b;
					
					r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
					g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
					b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;
					
					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;

					src += 2;
				}

				scraddr += linestep;
				scr_ptr += (pitch ? 2 * pitch : 2 * width);
				src_limit += (pitch ? 2 * pitch : 2 * width);
			}
				
		}
		else
		/* Special Smartphone implementation */
		/* Landscape mode, 2/3 X, 7/8 Y      */
		if (smartphone && own_palette) {
			int line = 0;
			int toskip = 0;

			while(scr_ptr < scr_ptr_limit)
			{
				//int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				//for (i=0; i < x; i++) 
				dst += x * pixelstep;


				while(src < src_limit)
				{
					UBYTE r, g, b;
					register first = 3 * *(src + 0);
					register second = 3 * *(src + 1);	
					register third = 3 * *(src + 2);

					r = (3 * own_palette[first] + 
							own_palette[second]) >> 2;
					g = (3 * own_palette[first + 1] +
							 own_palette[second + 1]) >> 2;
					b = (3 * own_palette[first + 2] +
						     own_palette[second + 2]) >> 2;

					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;
					
					r = (own_palette[second] + 
						     own_palette[third]) >> 1;
					g = (own_palette[second + 1] +
							 own_palette[third + 1]) >> 1;
					b = (own_palette[second + 2] +
						     own_palette[third + 2]) >> 1;

					*(unsigned short*)dst = COLORCONV565(r,g,b);
	
					dst += pixelstep;

					src += 3;
				}

				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);

				
				line++;
				if (line == 7) {
					scr_ptr += (pitch ? pitch : width);
					src_limit += (pitch ? pitch : width);
					line = 0;
				}
			}
		}
		else
		if (smartphone && !own_palette) {
			int line = 0;
			int toskip = 0;

			while(scr_ptr < scr_ptr_limit)
			{
				//int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				//for (i=0; i < x; i++) 
				dst += x * pixelstep;


				while(src < src_limit)
				{
					UBYTE r, g, b;
					register first = *(src + 0);
					register second = *(src + 1);	
					register third = *(src + 2);

					r = (3*palRed[first] + palRed[second])>>2;
					g = (3*palGreen[first] + palGreen[second])>>2;
					b = (3*palBlue[first] + palBlue[second])>>2;
					
					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;	

					r = (palRed[second] + palRed[third])>>1;
					g = (palGreen[second] + palGreen[third])>>1;
					b = (palBlue[second] + palBlue[third])>>1;
					
					*(unsigned short*)dst = COLORCONV565(r,g,b);
	
					dst += pixelstep;

					src += 3;
				}

				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);

				
				line++;
				if (line == 7) {
					scr_ptr += (pitch ? pitch : width);
					src_limit += (pitch ? pitch : width);
					line = 0;
				}
			}
		}
		else
		/* Internal pixel loops */
		if(skipmask == 3 && smooth_filter)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;
				
				while(src < src_limit)
				{
					UBYTE r, g, b;
					if (!own_palette) {
						r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
						g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
						b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;
					} else {
						r = (3 * own_palette[3 * *(src + 0)] + 
							     own_palette[3 * *(src + 1)]) >> 2;
						g = (3 * own_palette[3 * *(src + 0) + 1] +
								 own_palette[3 * *(src + 1) + 1]) >> 2;
						b = (3 * own_palette[3 * *(src + 0) + 2] +
							     own_palette[3 * *(src + 1) + 2]) >> 2;
					}

					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;

					if (!own_palette) {
						r = (palRed[*(src+1)] + palRed[*(src+2)])>>1;
						g = (palGreen[*(src+1)] + palGreen[*(src+2)])>>1;
						b = (palBlue[*(src+1)] + palBlue[*(src+2)])>>1;
					}
					else {
						r = (own_palette[3 * *(src + 1)] + 
							     own_palette[3 * *(src + 2)]) >> 1;
						g = (own_palette[3 * *(src + 1) + 1] +
								 own_palette[3 * *(src + 2) + 1]) >> 1;
						b = (own_palette[3 * *(src + 1) + 2] +
							     own_palette[3 * *(src + 2) + 2]) >> 1;
					}

					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;

					if (!own_palette) {
						r = (palRed[*(src+2)] + 3*palRed[*(src+3)])>>2;
						g = (palGreen[*(src+2)] + 3*palGreen[*(src+3)])>>2;
						b = (palBlue[*(src+2)] + 3*palBlue[*(src+3)])>>2;
					}
					else {
						r = (own_palette[3 * *(src + 2)] + 
							     3 * own_palette[3 * *(src + 3)]) >> 2;
						g = (own_palette[3 * *(src + 2) + 1] +
								 3 * own_palette[3 * *(src + 3) + 1]) >> 2;
						b = (own_palette[3 * *(src + 2) + 2] +
							     3 * own_palette[3 * *(src + 3) + 2]) >> 2;

					}

					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;

					src += 4;
				}

				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);
			}
		}
		else if(skipmask != 0xffffffff)
		{

			int line = 0;
			int toskip = 0;

			while(scr_ptr < scr_ptr_limit)
			{
				int i;				

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;

				while(src < src_limit)
				{	
					
					if((long)src & skipmask)
					{
						if (!own_palette)
							*(unsigned short*)dst = pal[*src];
						else
							*(unsigned short*)dst = 
								COLORCONV565(own_palette[3 * *src],
									own_palette[(3 * *src) + 1], 
									own_palette[(3 * *src) + 2]);
						dst += pixelstep;
					}
					
					src ++;
				}			


				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);
		}
		}
		else
		{
			while(scr_ptr < scr_ptr_limit)
			{
				int i;

				src = scr_ptr;
				dst = scraddr;

				/* skip non updated pixels for this line */
				for (i=0; i < x; i++)
					dst += pixelstep;

				while(src < src_limit)
				{
					if (!own_palette)
						*(unsigned short*)dst = pal[*src];
					else
						*(unsigned short*)dst = 
							COLORCONV565(own_palette[3 * *src],
								own_palette[(3 * *src) + 1], 
								own_palette[(3 * *src) + 2]);
					dst += pixelstep;
					src ++;
				}

				scraddr += linestep;
				scr_ptr += (pitch ? pitch : width);
				src_limit += (pitch ? pitch : width);
			}
		}

		dynamicGXEndDraw();
	}
}

/* ********************************* NO GAPI DISPLAY ********************************* */

#ifndef NEW_GAPI_CODE

void noGAPI_Cls() {
	HBITMAP old;
	RECT rc;
	HDC hdc = GetDC(hWndMain);
	HBITMAP hb;

	GetWindowRect(hWndMain, &rc);
	memset(noGAPI_video_buffer, 0x00, sizeof(noGAPI_video_buffer));
	if (currentScreenMode || wide_screen)
		hb = CreateBitmap(320, 240, 1, 16, noGAPI_video_buffer);
	else
		hb = CreateBitmap(240, 320, 1, 16, noGAPI_video_buffer);
	old = (HBITMAP)SelectObject(noGAPI_compat, hb);
	if (currentScreenMode || wide_screen)
		BitBlt(hdc, 0, 0, 320, 240, noGAPI_compat, 0, 0, SRCCOPY);
	else
		BitBlt(hdc, 0, 0, 240, 320, noGAPI_compat, 0, 0, SRCCOPY);
	SelectObject(noGAPI_compat, old);
	ReleaseDC(hWndMain, hdc);
	DeleteObject(hb);
}

void noGAPI_Blt(UBYTE *src_ptr) {
	noGAPI_Blt_part(src_ptr, 0, 0, _geometry_w, _geometry_h, NULL, 0);
}

void noGAPI_Set_565(INT16 *buffer, int pitch, int x, int y, int width, int height) {
	HBITMAP old;
	RECT rc;
	HDC hdc = GetDC(hWndMain);
	HBITMAP hb;
	UBYTE *work_buffer;
	int i;
	int j;
	//long skipmask;

	//skipmask = geom[useMode].xSkipMask;


	GetWindowRect(hWndMain, &rc);

	work_buffer = noGAPI_video_buffer;
	unsigned short *work_buffer_2 = (unsigned short*)work_buffer;
	if (currentScreenMode && !wide_screen) {
	
		for (i=0; i<width; i++) {
			for (j=0; j<height; j++) {
				work_buffer_2[i * height + j] = 
					buffer[(pitch ? pitch : width) * j + (width - i)];
			}			
		}
	}
	else {
		for (i=0; i<height; i++) {
			for (j=0; j<width; j++) {
				*(unsigned short*)work_buffer = buffer[(pitch ? pitch : width) * i + j];
				work_buffer += 2;
			}
		}
	}

	if (currentScreenMode && !wide_screen)
		hb = CreateBitmap(height, width, 1, 16, noGAPI_video_buffer);
	else
		hb = CreateBitmap(width, height, 1, 16, noGAPI_video_buffer);
	old = (HBITMAP)SelectObject(noGAPI_compat, hb);
	if (currentScreenMode && !wide_screen)
		BitBlt(hdc, y , 320 - (x + width), height, width, noGAPI_compat, 0, 0, SRCCOPY);
	else
		BitBlt(hdc, x, y, width, height, noGAPI_compat, 0, 0, SRCCOPY);
	SelectObject(noGAPI_compat, old);
	ReleaseDC(hWndMain, hdc);
	DeleteObject(hb);
}

void noGAPI_Blt_part(UBYTE * scr_ptr, int x, int y, int width, int height,
					 UBYTE * own_palette, int pitch) {
	HBITMAP old;
	RECT rc;
	HDC hdc = GetDC(hWndMain);
	HBITMAP hb;
	UBYTE *work_buffer;
	int i;
	int j;
	//long skipmask;

	//skipmask = geom[useMode].xSkipMask;


	GetWindowRect(hWndMain, &rc);

	work_buffer = noGAPI_video_buffer;
	if (currentScreenMode && !wide_screen) {
		unsigned short *work_buffer_2 = (unsigned short*)work_buffer;
		for (i=0; i<width; i++)
			for (j=0; j<height; j++) 
				if (!own_palette)
					work_buffer_2[i * height + j] = 
					pal[scr_ptr[(pitch ? pitch : width) * j + (width - i)]];
				else
					work_buffer_2[i * height + j] =
						COLORCONV565(own_palette[3 * scr_ptr[(pitch ? pitch : width) * j + (width - i)]],
									own_palette[(3 * scr_ptr[(pitch ? pitch : width) * j + (width - i)]) + 1], 
									own_palette[(3 * scr_ptr[(pitch ? pitch : width) * j + (width - i)]) + 2]);
	}
	else {
	for (i=0; i<height; i++) {
		for (j=0; j<width; j++) {
				if (!own_palette)
					*(unsigned short*)work_buffer = 
						pal[scr_ptr[(pitch ? pitch : width) * i + j]];	
				else
					*(unsigned short*)work_buffer =
								COLORCONV565(own_palette[3 * scr_ptr[(pitch ? pitch : width) * i + j]],
									own_palette[(3 * scr_ptr[(pitch ? pitch : width) * i + j]) + 1], 
									own_palette[(3 * scr_ptr[(pitch ? pitch : width) * i + j]) + 2]);

				work_buffer += 2;
			}
	}
	}

	if (currentScreenMode && !wide_screen)
		hb = CreateBitmap(height, width, 1, 16, noGAPI_video_buffer);
	else
		hb = CreateBitmap(width, height, 1, 16, noGAPI_video_buffer);
	old = (HBITMAP)SelectObject(noGAPI_compat, hb);
	if (currentScreenMode && !wide_screen)
		BitBlt(hdc, y , 320 - (x + width), height, width, noGAPI_compat, 0, 0, SRCCOPY);
	else
		BitBlt(hdc, x, y, width, height, noGAPI_compat, 0, 0, SRCCOPY);
	SelectObject(noGAPI_compat, old);
	ReleaseDC(hWndMain, hdc);
	DeleteObject(hb);
}

#else

void noGAPI_Cls() {
	HBITMAP old;
	RECT rc;
	HDC hdc = GetDC(hWndMain);
	HBITMAP hb;

	memset(noGAPI_buffer, 0x00, GetSystemMetrics(SM_CXSCREEN) * GetSystemMetrics(SM_CYSCREEN) * 3);
	BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), noGAPI_compat, 0, 0, SRCCOPY);
	ReleaseDC(hWndMain, hdc);
}

void noGAPI_Blt(UBYTE *src_ptr) {
	noGAPI_Blt_part(src_ptr, 0, 0, _geometry_w, _geometry_h, NULL, 0);
}

void noGAPI_Set_565(INT16 *buffer, int pitch, int x, int y, int width, int height) {
	HBITMAP old;
	RECT rc;
	HDC hdc = GetDC(hWndMain);
	HBITMAP hb;
	UBYTE *workBuffer = noGAPI_buffer;
	INT16 *srcBuffer = buffer;
	UBYTE *tempBuffer;
	INT16 *tempSrcBuffer;
	int i;
	int j;

	int startOffset;
	int lineStep;
	int pixelStep;

	if (currentScreenMode && !wide_screen) {
		startOffset = (GetSystemMetrics(SM_CXSCREEN) * (GetSystemMetrics(SM_CYSCREEN) - 1)) * 3;
		lineStep = 3;
		pixelStep = -(GetSystemMetrics(SM_CXSCREEN) * 3);
	}
	else {
		startOffset = 0;
		lineStep = GetSystemMetrics(SM_CXSCREEN) * 3;
		pixelStep = 3;
	}

	workBuffer += startOffset;
	workBuffer += y * lineStep;

	for (i=0; i<height; i++) {

		tempBuffer = workBuffer;
		tempSrcBuffer = srcBuffer;

		tempBuffer += x * pixelStep;

		 for (j=0; j<width; j++) {
				*tempBuffer = BLUE_FROM_565(*tempSrcBuffer);
				*(tempBuffer + 1) = GREEN_FROM_565(*tempSrcBuffer);
				*(tempBuffer + 2) = RED_FROM_565(*tempSrcBuffer);
				tempBuffer += pixelStep;
				tempSrcBuffer++;
		}

		workBuffer += lineStep;
		srcBuffer += (pitch ? pitch : width);
	}
	

	if (currentScreenMode && !wide_screen)
		BitBlt(hdc, y , 320 - (x + width), height, width, noGAPI_compat, y, 320 - (x + width), SRCCOPY);
	else
		BitBlt(hdc, x, y, width, height, noGAPI_compat, x, y, SRCCOPY);

	ReleaseDC(hWndMain, hdc);		
}

void noGAPI_Blt_part(UBYTE * scr_ptr, int x, int y, int width, int height,
					 UBYTE * own_palette, int pitch) {
	HDC hdc = GetDC(hWndMain);
	HBITMAP hb;
	UBYTE *workBuffer = noGAPI_buffer;
	UBYTE *srcBuffer = scr_ptr;
	UBYTE *tempBuffer;
	UBYTE *tempSrcBuffer;
	int i;
	int j;
	//long skipmask;

	int startOffset;
	int lineStep;
	int pixelStep;

	if (currentScreenMode && !wide_screen) {
		startOffset = (GetSystemMetrics(SM_CXSCREEN) * (GetSystemMetrics(SM_CYSCREEN) - 1)) * 3;
		lineStep = 3;
		pixelStep = -(GetSystemMetrics(SM_CXSCREEN) * 3);
	}
	else {
		startOffset = 0;
		lineStep = GetSystemMetrics(SM_CXSCREEN) * 3;
		pixelStep = 3;
	}

	if (high_res && !extra_wide_screen) {
		height /= 2;
		width /= 2;
	}

	workBuffer += startOffset;
	workBuffer += y * lineStep;

	for (i=0; i<height; i++) {

		tempBuffer = workBuffer;
		tempSrcBuffer = srcBuffer;

		tempBuffer += x * pixelStep;

		 for (j=0; j<width; j++) {
			if (!own_palette) {
				if (high_res && !extra_wide_screen) {
					*tempBuffer = (3*palBlue[*tempSrcBuffer] + palBlue[*(tempSrcBuffer + 1)]) >> 2;
					*(tempBuffer + 1) = (3 * palGreen[*tempSrcBuffer] + palGreen[(*tempSrcBuffer + 1)]) >> 2;
					*(tempBuffer + 2) = (3 * palRed[*tempSrcBuffer] + palRed[*(tempSrcBuffer + 1)]) >> 2;
					tempBuffer += pixelStep;
					tempSrcBuffer += 2;
				}
				else {
					*tempBuffer = palBlue[*tempSrcBuffer];
					*(tempBuffer + 1) = palGreen[*tempSrcBuffer];
					*(tempBuffer + 2) = palRed[*tempSrcBuffer];
					tempBuffer += pixelStep;
					tempSrcBuffer++;
				}
				
			}
			else {
				*tempBuffer = own_palette[3 * *tempSrcBuffer + 2];
				*(tempBuffer + 1) = own_palette[3 * *tempSrcBuffer + 1];
				*(tempBuffer + 2) = own_palette[3 * *tempSrcBuffer];
				tempBuffer += pixelStep;
				tempSrcBuffer++;
			}
		}

		workBuffer += lineStep;
		if (high_res && !extra_wide_screen)
			srcBuffer += (pitch ? 2 * pitch : 2 * width);
		else
			srcBuffer += (pitch ? pitch : width);
	}

	if (currentScreenMode && !wide_screen)
		BitBlt(hdc, y , 320 - (x + width), height, width, noGAPI_compat, y, 320 - (x + width), SRCCOPY);
	else
		BitBlt(hdc, x, y, width, height, noGAPI_compat, x, y, SRCCOPY);

	ReleaseDC(hWndMain, hdc);
}

#endif

/* ************************** STYLUS TRANSLATION ************************* */


void Translate(int* px, int* py)
{
	int x, y;
	
	if ((wide_screen && !high_res) || (wide_screen && extra_wide_screen && high_res))
		return;

	if (wide_screen && high_res) {
		if (!g_gui->isActive()) {
			*px = x * 2;
			*py = y * 2;
		}
		else {
			*px = x;
			*py = y;
		}
		return;
	}
	
	if (high_res) {
		x = 320 - *py;
		y = *px;
		if (!g_gui->isActive()) {
			*px = x * 2;
			*py = y * 2;
		}
		else {
			*px = x;
			*py = y;
		}
		return;
	}

	switch(currentScreenMode)
	{
	case 0: /* portrait */
		*px = *px*4/3;
		break;
	case 1: /* landscape left */
		x = 320 - *py;
		y = *px;
		*px = x;
		*py = y;
		break;
	case 2: /* landscape right */
		x = *py;
		y = 240 - *px;
		*px = x;
		*py = y;
		break;
	}
}

/* ************************** LAUNCHER FONT STUFF ************************* */

void printChar(const char str, int xx, int yy, int textcolor, int highlight, int width)
{
	unsigned int buffer = 0, mask = 0, x, y;
	byte *tmp;
	byte *ptr;

	tmp = &textfont[0];
	tmp += 224 + (str + 1) * 8;

	ptr = decomp + (yy * width) + xx;

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			unsigned char color;
			if ((mask >>= 1) == 0) {
				buffer = *tmp++;
				mask = 0x80;
			}
			color = ((buffer & mask) != 0);
			if (color)
				ptr[x] = textcolor;
			else if (highlight > 0)
				ptr[x] = highlight;
		}
		if (highlight > 0) {
			int i;
			for (i=9; i<width; i++)
				ptr[x] = highlight;
		}
		ptr += width;
	}

}
void printString(const char *str, int x, int y, int textcolor, int highlight, int width)
{
	for (uint letter = 0; letter < strlen(str); letter++)
		printChar(str[letter], x + (letter * 8), y, textcolor, highlight, width);
}

#define MAX_CHARS_PER_LINE 29

int drawString(const char *str, int x, int y, int textcolor, int highlight) {
	int current_y;
	unsigned int current_pos = 0;
	char substring[MAX_CHARS_PER_LINE + 1];

	current_y = y;

	while(current_pos < strlen(str)) {
		memset(substring, 0, sizeof(substring));
		if (strlen(str + current_pos) > MAX_CHARS_PER_LINE) 
			memcpy(substring, str + current_pos, MAX_CHARS_PER_LINE);
		else
			strcpy(substring, str + current_pos);
		printString(substring, x, current_y, textcolor, highlight, 320);
		pBlt_part(decomp + (current_y * 320), 0, current_y, 320, 8, item_startup_colors, 0);
		current_pos += MAX_CHARS_PER_LINE;
		current_y += 10;
	}

	return current_y;
}

/* ************************** DIRECT BLT IMPLEMENTATION ************************* */


void Get_565(UBYTE *src, INT16 *buffer, int pitch, int x, int y, int width, int height) {
	int i,j;
	UBYTE *tempo = (UBYTE*)buffer;

	// Dumb conversion to 565

	if (!(high_res && !wide_screen)) {
		for (i=0; i<height; i++) {
			for (j=0; j<width; j++) {
				*buffer++ = COLORCONV565(palRed[*src], palGreen[*src], palBlue[*src]);
				src++;
			}
		}
	}
	else {
		UBYTE *current;

		for (i=0; i<height; i++) {
			current = src;
			for (j=0; j<width; j++) {
				*buffer++ = COLORCONV565(palRed[*current], palGreen[*current], palBlue[*current]);
				current += 2;
			}
			src += 2 * 640;
		}
	}
}

void Set_565(INT16 *buffer, int pitch, int x, int y, int width, int height) {
		pSet_565(buffer, pitch, x, y, width, height);
}

void NULL_Set_565(INT16 *buffer, int pitch, int x, int y, int width, int height) {
}


#endif
