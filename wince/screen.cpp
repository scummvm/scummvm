#ifdef _WIN32_WCE

// TODO : toolbar for paletted devices

/* (C) 2001  Vasyl Tsvirkunov */
/* Added PocketATARI additional support code for paletted/mono devices */

#include <windows.h>
#include <Aygshell.h>
#include "gx.h"
#include "screen.h"
#include "resource.h"

#define COLORCONV565(r,g,b) \
(((r&0xf8)<<(11-3))|((g&0xfc)<<(5-2))|((b&0xf8)>>3))
#define COLORCONV555(r,g,b) \
(((r&0xf8)<<(10-3))|((g&0xf8)<<(5-2))|((b&0xf8)>>3))
#define COLORCONVMONO(r,g,b) ((((3*r>>3)+(g>>1)+(b>>3))>>colorscale)^invert)

#define MAX_CLR         0x100
static UBYTE palRed[MAX_CLR];
static UBYTE palGreen[MAX_CLR];
static UBYTE palBlue[MAX_CLR];
static unsigned short pal[MAX_CLR];
 /* First 10 and last 10 colors on palettized devices require special treatment */
static UBYTE staticTranslate[20];
static UBYTE invert = 0;
static int colorscale = 0;

extern UBYTE item_toolbar[];
extern UBYTE item_keyboard[];
extern UBYTE item_disk[];
extern UBYTE item_skip[];
extern UBYTE item_set_sound_on[];
extern UBYTE item_set_sound_off[];
extern UBYTE item_options[];

extern bool sound_activated;
bool toolbar_drawn;
bool draw_keyboard;

GXDisplayProperties gxdp;
int active;

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

int currentScreenMode = 0;
int useMode = 0;
int maxMode = 2;
int filter_available;
int smooth_filter;
int toolbar_available;

UBYTE *toolbar = NULL;

/* Using vectorized function to save on branches */
typedef void (*tCls)();
typedef void (*tBlt)(UBYTE*);

void mono_Cls();
void mono_Blt(UBYTE*);

void palette_Cls();
void palette_Blt(UBYTE*);

void hicolor_Cls();
void hicolor555_Blt(UBYTE*);
void hicolor565_Blt(UBYTE*);

void palette_update();

static tCls        pCls        = NULL;
static tBlt		   pBlt	       = NULL;

static int _geometry_w;
static int _geometry_h;  

HWND hWndMain;


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
		GXSuspend();
	}
}

void GraphicsResume()
{
	if(!active)
	{
		active = 1;
		GXResume();
	}

	palette_update();
}

void GraphicsOff(void)
{
	GXCloseDisplay();
	active = 0;
}

void SetScreenGeometry(int w, int h) {
	// Complain (loudly) if w > 320 and h > 240 ...
	if (w != 320 || h > 240) {
		MessageBox(NULL, TEXT("Unsupported screen geometry !"), TEXT("Error"), MB_OK);
		exit(1);
	}

	_geometry_w = w;
	_geometry_h = h;
	RestoreScreenGeometry();
}

void LimitScreenGeometry() {

	if (_geometry_h > 200) {
		geom[0].lineLimit = _geometry_w*200;
		geom[1].lineLimit = _geometry_w*200;
		geom[1].lineLimit = _geometry_w*200;
	}
}

void RestoreScreenGeometry() {
	geom[0].lineLimit = _geometry_w * _geometry_h;
	geom[1].lineLimit = _geometry_w * _geometry_h;
	geom[2].lineLimit = _geometry_w * _geometry_h;
}

int GraphicsOn(HWND hWndMain_param)
{
	hWndMain = hWndMain_param;
	GXOpenDisplay(hWndMain, GX_FULLSCREEN);
	
	gxdp = GXGetDisplayProperties();

	if(gxdp.ffFormat & kfDirect565)
	{
		pCls =    hicolor_Cls;
		pBlt =    hicolor565_Blt;
		filter_available = 1;
		smooth_filter = 1;
		toolbar_available = 1;
	}
	else if(gxdp.ffFormat & kfDirect555)
	{
		pCls =    hicolor_Cls;
		pBlt =    hicolor555_Blt;
		filter_available = 1;
		smooth_filter = 1;
		toolbar_available = 1;
	}
	else if((gxdp.ffFormat & kfDirect) && (gxdp.cBPP <= 8))
	{
		pCls =  mono_Cls;
		pBlt =  mono_Blt;
		
		if(gxdp.ffFormat & kfDirectInverted)
			invert = (1<<gxdp.cBPP)-1;
		colorscale = gxdp.cBPP < 8 ? 8-gxdp.cBPP : 0;

		if(gxdp.cBPP >= 4)
			filter_available = 1;

		toolbar_available = 0;
	}
	else if(gxdp.ffFormat & kfPalette)
	{
		pCls =    palette_Cls;
		pBlt =    palette_Blt;
		
		toolbar_available = 0;
	}


	if(!pCls || !pBlt || gxdp.cxWidth < 240 || gxdp.cyHeight < 
240)
	{
	// I don't believe there are devices that end up here
		GraphicsOff();
		return 1;
	}
	
	// portrait
	geom[0].width = gxdp.cxWidth; // 240
	geom[0].height = gxdp.cyHeight; // 320
	geom[0].startoffset = 0;
	geom[0].sourceoffset = 0;
	geom[0].linestep = gxdp.cbyPitch;
	geom[0].pixelstep = gxdp.cbxPitch;
	geom[0].xSkipMask = gxdp.cxWidth < 320 ? 0x00000003 : 0xffffffff;
	geom[0].xLimit = 320; // skip 1/4
	geom[0].lineLimit = 320*200;
	
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
	
	if(gxdp.cyHeight < 320)
		maxMode = 0; // portrait only!

	/*
	for(int i = 0; i < MAX_CLR; i++)
	{
		SetPalEntry(i,  (colortable[i] >> 16) & 0xff,
			(colortable[i] >> 8) & 0xff,
			(colortable[i]) & 0xff);
	}
	*/

	//palette_update();

	active = 1;
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

	}
}

void SetPalEntry(int ent, UBYTE r, UBYTE g, UBYTE b)
{
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

	scraddr = (UBYTE*)GXBeginDraw();
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
		GXEndDraw();
	}
}

void palette_Cls()
{
	int x, y;
	UBYTE* dst;
	UBYTE *scraddr;
	scraddr = (UBYTE*)GXBeginDraw();
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
		GXEndDraw();
	}
}

void hicolor_Cls()
{
	int x, y;
	UBYTE* dst;
	UBYTE *scraddr;
	scraddr = (UBYTE*)GXBeginDraw();
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
		GXEndDraw();
	}
}


void Cls()
{
	pCls();
}
int counter = 0;

void drawToolbarItem(UBYTE* item, int dest_x, int dest_y, int width_item, int height_item) {
	int x,y,z;
	UBYTE* dst;
	UBYTE *scraddr;

	scraddr = (UBYTE*)GXBeginDraw();
	if(scraddr)
	{
		scraddr += dest_y * geom[useMode].linestep;

		z = 0;

		for (y=dest_y; y<dest_y + height_item; y++) {
			dst = scraddr + geom[useMode].startoffset + dest_x * geom[useMode].pixelstep;
			for (x=dest_x; x<dest_x + width_item; x++) {
				if(gxdp.ffFormat & kfDirect565)
		*(unsigned short*)dst = 
			((item[z]&0xf8)<<(11-3))|((item[z+1]&0xfc)<<(5-2))|((item[z+2]&0xf8)>>3);
	else if(gxdp.ffFormat & kfDirect555)
		*(unsigned short*)dst = 
			((item[z]&0xf8)<<(10-3))|((item[z+1]&0xf8)<<(5-2))|((item[z+2]&0xf8)>>3);
				dst += geom[useMode].pixelstep;
				z += 3;
			}
			scraddr += geom[useMode].linestep;
		}
	}
	GXEndDraw();
}


void drawSoundItem(int x, int y) {
	if (sound_activated)
		drawToolbarItem(item_set_sound_off, x, y, 32, 32);
	else
		drawToolbarItem(item_set_sound_on, x, y, 32, 32);
}


void drawAllToolbar() {
	int x,y;

	if (draw_keyboard) {
		drawToolbarItem(item_keyboard, 0, 200, 320, 40);
	}
	else {
		drawToolbarItem(item_toolbar, 0, 200, 320, 40);
		x = 10;
		y = 204;
		drawToolbarItem(item_disk, x, y, 32, 32);
		x += 40;
		drawToolbarItem(item_skip, x, y, 32, 32);
		x += 40;
		drawSoundItem(x, y);
		x += 40;
		drawToolbarItem(item_options, x, y, 32, 32);
	}

	toolbar_drawn = true;
}

void redrawSoundItem() {
	drawSoundItem(10 + 40 + 40, 204);
}

bool isInBox(int x, int y, int x1, int y1, int x2, int y2) {
	return ((x >= x1 && y >= y1) && (x <= x2 && y <= y2));
}

ToolbarSelected getToolbarSelection (int x, int y) {
	int test_x, test_y;

	if (!currentScreenMode)
		return ToolbarNone;

	if (!(x >= 10 && y >= 204))
		return ToolbarNone;
	
	test_x = 10;
	test_y = 204;
	if (isInBox(x, y, test_x, test_y, test_x + 32, test_y + 32))
		return ToolbarSaveLoad;
	test_x += 40;
	if (isInBox(x, y, test_x, test_y, test_x + 32, test_y + 32))
		return ToolbarSkip;
	test_x += 40;
	if (isInBox(x, y, test_x, test_y, test_x + 32, test_y + 32))
		return ToolbarSound;
	test_x += 40;
	if (isInBox(x, y, test_x, test_y, test_x + 32, test_y + 32))
		return ToolbarExit;
	return ToolbarNone;
}
	
/* ************************** BLT IMPLEMENTATION **************************** */

void Blt(UBYTE * scr_ptr) 
{
	pBlt(scr_ptr);

	if (toolbar_available && currentScreenMode && !toolbar_drawn)
		drawAllToolbar();

}


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


void mono_Blt(UBYTE * scr_ptr)
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

	scraddr = (UBYTE*)GXBeginDraw();

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
			scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
			src_limit = scr_ptr + geom[useMode].xLimit;

			/* Internal pixel loops */
			if(skipmask == 3 && (smooth_filter) && gxdp.cBPP >= 4)
			{
				while(scr_ptr < scr_ptr_limit)
				{
					src = scr_ptr;
					dst = scraddr;
					while(src < src_limit)
					{
						UBYTE r, g, b;
						r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
						g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
						b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;

						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

						dst += pixelstep;

						r = (palRed[*(src+1)] + palRed[*(src+2)])>>1;
						g = (palGreen[*(src+1)] + palGreen[*(src+2)])>>1;
						b = (palBlue[*(src+1)] + palBlue[*(src+2)])>>1;

						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

						dst += pixelstep;

						r = (palRed[*(src+2)] + 3*palRed[*(src+3)])>>2;
						g = (palGreen[*(src+2)] + 3*palGreen[*(src+3)])>>2;
						b = (palBlue[*(src+2)] + 3*palBlue[*(src+3)])>>2;

						*dst = (*dst & ~bitmask) | (COLORCONVMONO(r,g,b)<<bitshift);

						dst += pixelstep;

						src += 4;
					}

					ADVANCE_PARTIAL(scraddr, linestep);

					scr_ptr += 320;
					src_limit += 320;
				}
			}
			else if(skipmask != 0xffffffff)
			{
				while(scr_ptr < scr_ptr_limit)
				{
					src = scr_ptr;
					dst = scraddr;
					while(src < src_limit)
					{
						if((long)src & skipmask)
						{
							*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							dst += pixelstep;
						}
						src ++;
					}

					ADVANCE_PARTIAL(scraddr, linestep);

					scr_ptr += 320;
					src_limit += 320;
				}
			}
			else
			{
				while(scr_ptr < scr_ptr_limit)
				{
					src = scr_ptr;
					dst = scraddr;
					while(src < src_limit)
					{
						*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
						dst += pixelstep;
						src ++;
					}

					ADVANCE_PARTIAL(scraddr, linestep);

					scr_ptr += 320;
					src_limit += 320;
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
			scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
			src_limit = scr_ptr + geom[useMode].xLimit;

			if(skipmask != 0xffffffff)
			{
				if(pixelstep > 0)
				{
					bitshift = 8-gxdp.cBPP;
					bitmask = ((1<<gxdp.cBPP)-1)<<bitshift;

					while(scr_ptr < scr_ptr_limit)
					{
						src = scr_ptr;
						dst = scraddr;
						dst -= (linestep-pixelstep);
						while(src < src_limit)
						{
							if((long)src & skipmask)
							{
								*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
								ADVANCE_REV_PARTIAL(dst, pixelstep);
							}
							src ++;
						}

						scraddr += linestep;

						scr_ptr += 320;
						src_limit += 320;
					}
				}
				else
				{
					bitshift = 0;
					bitmask = (1<<gxdp.cBPP)-1;

					while(scr_ptr < scr_ptr_limit)
					{
						src = scr_ptr;
						dst = scraddr;
						while(src < src_limit)
						{
							if((long)src & skipmask)
							{
								*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
								ADVANCE_PARTIAL(dst, pixelstep);
							}
							src ++;
						}

						scraddr += linestep;

						scr_ptr += 320;
						src_limit += 320;
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
						src = scr_ptr;
						dst = scraddr;
						dst -= (linestep-pixelstep);
						while(src < src_limit)
						{
							*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							ADVANCE_REV_PARTIAL(dst, pixelstep);
							src ++;
						}

						scraddr += linestep;

						scr_ptr += 320;
						src_limit += 320;
					}
				}
				else
				{
					bitshift = 0;
					bitmask = (1<<gxdp.cBPP)-1;

					while(scr_ptr < scr_ptr_limit)
					{
						src = scr_ptr;
						dst = scraddr;
						while(src < src_limit)
						{
							*dst = ((*dst)&~bitmask)|(pal[*src]<<bitshift);
							ADVANCE_PARTIAL(dst, pixelstep);
							src ++;
						}

						scraddr += linestep;

						scr_ptr += 320;
						src_limit += 320;
					}
				}
			}
		}
	}
	GXEndDraw();
}

void palette_Blt(UBYTE * scr_ptr)
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

	scraddr = (UBYTE*)GXBeginDraw();
	if(scraddr)
	{

		scraddr += geom[useMode].startoffset;
		scr_ptr += geom[useMode].sourceoffset;
		scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		src_limit = scr_ptr + geom[useMode].xLimit;

		/* Internal pixel loops */
		if(skipmask != 0xffffffff)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				src = scr_ptr;
				dst = scraddr;
				while(src < src_limit)
				{
					if((long)src & skipmask)
					{
						if (*src < 236)
							*dst = *src + 10;
						else
							*dst = staticTranslate[*src-236];
						dst += pixelstep;
					}
					src ++;
				}
				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}
		else
		{
			while(scr_ptr < scr_ptr_limit)
			{
				src = scr_ptr;
				dst = scraddr;
				while(src < src_limit)
				{
					if (*src < 236)
						*dst = *src + 10; 
					else
						*dst = staticTranslate[*src-236];
					dst += pixelstep;
					src ++;
				}
				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}

		GXEndDraw();
	}
}

void hicolor555_Blt(UBYTE * scr_ptr)
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

	scraddr = (UBYTE*)GXBeginDraw();
	if(scraddr)
	{

		scraddr += geom[useMode].startoffset;
		scr_ptr += geom[useMode].sourceoffset;
		scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		src_limit = scr_ptr + geom[useMode].xLimit;

		/* Internal pixel loops */
		if(skipmask == 3 && smooth_filter)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				src = scr_ptr;
				dst = scraddr;
				while(src < src_limit)
				{
					UBYTE r, g, b;
					r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
					g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
					b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;

					*(unsigned short*)dst = COLORCONV555(r,g,b);

					dst += pixelstep;

					r = (palRed[*(src+1)] + palRed[*(src+2)])>>1;
					g = (palGreen[*(src+1)] + palGreen[*(src+2)])>>1;
					b = (palBlue[*(src+1)] + palBlue[*(src+2)])>>1;

					*(unsigned short*)dst = COLORCONV555(r,g,b);

					dst += pixelstep;

					r = (palRed[*(src+2)] + 3*palRed[*(src+3)])>>2;
					g = (palGreen[*(src+2)] + 3*palGreen[*(src+3)])>>2;
					b = (palBlue[*(src+2)] + 3*palBlue[*(src+3)])>>2;

					*(unsigned short*)dst = COLORCONV555(r,g,b);

					dst += pixelstep;

					src += 4;
				}
				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}
		else if(skipmask != 0xffffffff)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				src = scr_ptr;
				dst = scraddr;
				while(src < src_limit)
				{
					if((long)src & skipmask)
					{
						*(unsigned short*)dst = pal[*src];
						dst += pixelstep;
					}
					src ++;
				}
				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}
		else
		{
			while(scr_ptr < scr_ptr_limit)
			{
				src = scr_ptr;
				dst = scraddr;
				while(src < src_limit)
				{
					*(unsigned short*)dst = pal[*src];
					dst += pixelstep;
					src ++;
				}

				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}

		GXEndDraw();
	}
}

void hicolor565_Blt(UBYTE * scr_ptr)
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

	scraddr = (UBYTE*)GXBeginDraw();
	if(scraddr)
	{

		scraddr += geom[useMode].startoffset;
		scr_ptr += geom[useMode].sourceoffset;
		scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		src_limit = scr_ptr + geom[useMode].xLimit;

		/* Internal pixel loops */
		if(skipmask == 3 && smooth_filter)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				src = scr_ptr;
				dst = scraddr;
				while(src < src_limit)
				{
					UBYTE r, g, b;
					r = (3*palRed[*(src+0)] + palRed[*(src+1)])>>2;
					g = (3*palGreen[*(src+0)] + palGreen[*(src+1)])>>2;
					b = (3*palBlue[*(src+0)] + palBlue[*(src+1)])>>2;

					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;

					r = (palRed[*(src+1)] + palRed[*(src+2)])>>1;
					g = (palGreen[*(src+1)] + palGreen[*(src+2)])>>1;
					b = (palBlue[*(src+1)] + palBlue[*(src+2)])>>1;

					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;

					r = (palRed[*(src+2)] + 3*palRed[*(src+3)])>>2;
					g = (palGreen[*(src+2)] + 3*palGreen[*(src+3)])>>2;
					b = (palBlue[*(src+2)] + 3*palBlue[*(src+3)])>>2;

					*(unsigned short*)dst = COLORCONV565(r,g,b);

					dst += pixelstep;

					src += 4;
				}
				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}
		else if(skipmask != 0xffffffff)
		{
			while(scr_ptr < scr_ptr_limit)
			{
				src = scr_ptr;
				dst = scraddr;
				while(src < src_limit)
				{
					if((long)src & skipmask)
					{
						*(unsigned short*)dst = pal[*src];
						dst += pixelstep;
					}
					src ++;
				}
				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}
		else
		{
			while(scr_ptr < scr_ptr_limit)
			{
				src = scr_ptr;
				dst = scraddr;
				while(src < src_limit)
				{
					*(unsigned short*)dst = pal[*src];
					dst += pixelstep;
					src ++;
				}

				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}

		GXEndDraw();
	}
}


void Translate(int* px, int* py)
{
	int x, y;
	
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

#endif
