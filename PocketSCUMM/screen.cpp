/* (C) 2001  Vasyl Tsvirkunov */

#include <windows.h>
#include "gx.h"
#include "screen.h"

#define SMOOTH

#define MAX_CLR         0x100
#ifdef SMOOTH
static UBYTE palRed[MAX_CLR];
static UBYTE palGreen[MAX_CLR];
static UBYTE palBlue[MAX_CLR];
#endif
static unsigned short pal[MAX_CLR];


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
}

void GraphicsOff(void)
{
	GXCloseDisplay();
	active = 0;
}

int GraphicsOn(HWND hWndMain)
{
	GXOpenDisplay(hWndMain, GX_FULLSCREEN);
	
	gxdp = GXGetDisplayProperties();
	if((gxdp.ffFormat & (kfDirect555 | kfDirect565)) == 0 || gxdp.cxWidth < 240 || gxdp.cyHeight < 240)
	{
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

	active = 1;
	return 0;
}

void SetPalEntry(int ent, UBYTE r, UBYTE g, UBYTE b)
{
	if (ent >= MAX_CLR)
		return;
#ifdef SMOOTH
	palRed[ent] = r;
	palGreen[ent] = g;
	palBlue[ent] = b;
#endif
	if(gxdp.ffFormat & kfDirect565)
		pal[ent] = ((r&0xf8)<<(11-3))|((g&0xfc)<<(5-2))|((b&0xf8)>>3);
	else if(gxdp.ffFormat & kfDirect555)
		pal[ent] = ((r&0xf8)<<(10-3))|((g&0xf8)<<(5-2))|((b&0xf8)>>3);
}

void Cls()
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

int counter = 0;

void Blt(UBYTE * scr_ptr)
{
	static UBYTE *src;
	static UBYTE *dst;
	static UBYTE *scraddr;
	static UBYTE *scr_ptr_limit;
	static UBYTE *src_limit;
	static long pixelstep;
	static long linestep;
	static long skipmask;

#ifdef SMOOTH
	static bool b565 = (gxdp.ffFormat & kfDirect565);
#endif
	
	if(!active)
	{
		Sleep(100);
		return;
	}
	
	/* Update screen mode, also thread protection by doing this */
	if(useMode != currentScreenMode)
	{
		useMode = currentScreenMode;
		Cls();
	}
	
	scraddr = (UBYTE*)GXBeginDraw();
	if(scraddr)
	{
		scraddr += geom[useMode].startoffset;
		scr_ptr += geom[useMode].sourceoffset;
		scr_ptr_limit = scr_ptr + geom[useMode].lineLimit;
		pixelstep = geom[useMode].pixelstep;
		linestep = geom[useMode].linestep;
		src_limit = scr_ptr + geom[useMode].xLimit;
		skipmask = geom[useMode].xSkipMask;
		
		/* Internal pixel loops */
#ifdef SMOOTH
		if(skipmask == 3)
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

					if(b565)
						*(unsigned short*)dst = ((r&0xf8)<<(11-3))|((g&0xfc)<<(5-2))|((b&0xf8)>>3);
					else
						*(unsigned short*)dst = ((r&0xf8)<<(10-3))|((g&0xf8)<<(5-2))|((b&0xf8)>>3);
					dst += pixelstep;

					r = (palRed[*(src+1)] + palRed[*(src+2)])>>1;
					g = (palGreen[*(src+1)] + palGreen[*(src+2)])>>1;
					b = (palBlue[*(src+1)] + palBlue[*(src+2)])>>1;

					if(b565)
						*(unsigned short*)dst = ((r&0xf8)<<(11-3))|((g&0xfc)<<(5-2))|((b&0xf8)>>3);
					else
						*(unsigned short*)dst = ((r&0xf8)<<(10-3))|((g&0xf8)<<(5-2))|((b&0xf8)>>3);
					dst += pixelstep;

					r = (palRed[*(src+2)] + 3*palRed[*(src+3)])>>2;
					g = (palGreen[*(src+2)] + 3*palGreen[*(src+3)])>>2;
					b = (palBlue[*(src+2)] + 3*palBlue[*(src+3)])>>2;

					if(b565)
						*(unsigned short*)dst = ((r&0xf8)<<(11-3))|((g&0xfc)<<(5-2))|((b&0xf8)>>3);
					else
						*(unsigned short*)dst = ((r&0xf8)<<(10-3))|((g&0xf8)<<(5-2))|((b&0xf8)>>3);
					dst += pixelstep;

					src += 4;
				}
				scraddr += linestep;
				scr_ptr += 320;
				src_limit += 320;
			}
		}
		else
#endif
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
