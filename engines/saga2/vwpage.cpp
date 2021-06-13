/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "common/rect.h"
#include "graphics/surface.h"
#include "saga2/std.h"
#include "saga2/vdraw.h"
#include "saga2/vwpage.h"

namespace Saga2 {

static CDDWindow    *ddWindow = NULL;
static CDDWindow    *oldDDWindow = NULL;
static int          suspends = 0;

void WriteToLogFile(char *, ...);

/* ===================================================================== *
                    DirectDraw display page class
 * ===================================================================== */

#define vWDisplayPage   vDisplayPage

#ifndef KLUDGE
static bool displayEnabled(void) {
	return true;
}
#endif

/* ===================================================================== *
    Globals
 * ===================================================================== */

vWDisplayPage       displayPageList[2],
                    *displayPage,
                    protoPage;

int16               displayPageCount,
                    currentDisplayPage;

/* ===================================================================== *
    GTools assembly-language function prototypes
 * ===================================================================== */

//  Assembly-language function to copy pixel to SVGA (opaque)

#ifndef FTAASM_H
extern void _BltPixels(uint8 *srcPtr, uint32 srcMod,
                                 uint8 *dstPtr, uint32 dstMod,
                                 uint32 width, uint32 height);

//  Assembly-language function to copy pixel to SVGA (transparent)

extern void _BltPixelsT(uint8 *srcPtr, uint32 srcMod,
                                  uint8 *dstPtr, uint32 dstMod,
                                  uint32 width, uint32 height);

//  Assembly-language function to do rectangle fill (opaque)

extern void _FillRect(uint8 *dstPtr, uint32 dstMod,
                                uint32 width, uint32 height, uint32 color);

//  Assembly-language function to draw horizontal line (opaque)

extern void _HLine(uint8 *dstPtr, uint32 width, uint32 color);


#endif

/*
typedef struct _DDSURFACEDESC{
    DWORD       dwSize;         // size of the DDSURFACEDESC structure
    DWORD       dwFlags;        // determines what fields are valid
    DWORD       dwHeight;       // height of surface to be created
    DWORD       dwWidth;        // width of input surface
    LONG        lPitch;         // distance to start of next line (return value only)
    DWORD       dwBackBufferCount;  // number of back buffers requested
    DWORD       dwZBufferBitDepth;  // depth of Z buffer requested
    DWORD       dwAlphaBitDepth;    // depth of alpha buffer requested
LPVOID      lpSurface;      // pointer to the associated surface memory
    DDCOLORKEY      ddckCKDestOverlay;  // color key for destination overlay use
    DDCOLORKEY      ddckCKDestBlt;      // color key for destination blt use
    DDCOLORKEY      ddckCKSrcOverlay;   // color key for source overlay use
    DDCOLORKEY      ddckCKSrcBlt;       // color key for source blt use
    DDPIXELFORMAT   ddpfPixelFormat;    // pixel format description of the surface
    DDSCAPS     ddsCaps;        // direct draw surface capabilities
} DDSURFACEDESC,  FAR* LPDDSURFACEDESC;
*/

/* ===================================================================== *
                        Initialize graphics
 * ===================================================================== */

void initDDGraphics(gDisplayPort &mainPort, CDDWindow *displayWin) {
	ddWindow = displayWin;
	suspends = 0;
	displayPageCount = 1;
	currentDisplayPage = 0;

	displayPageList[0] = displayPageList[1] = protoPage;

	displayPage = &displayPageList[0];
	drawPage = (vDisplayPage *)displayPage;

	//  Set the size of the display page in pixels

//	gDisplaySize = Point16( mib->XResolution, mib->YResolution );
	gDisplaySize = Point16(640, 480);
	displayPageList[0].size = gDisplaySize;
	displayPageList[1].size = gDisplaySize;

	mainPort.setDisplayPage(drawPage);   // set up display page
}

/* ===================================================================== *
                        Clean up graphics
 * ===================================================================== */

void cleanupDDGraphics(void) {
	/*
	    restoreDisplay();
	*/
	ddWindow = NULL;
	suspends = 0;
}

void suspendDDGraphics(void) {
	if (0 == suspends) {
		oldDDWindow = ddWindow;
		ddWindow = NULL;
	}
	suspends++;
}

void resumeDDGraphics(void) {
	suspends--;
	if (0 == suspends) {
		ddWindow = oldDDWindow;
		oldDDWindow = NULL;
	}
}



typedef void (* BlittingFunc)(uint8 *srcPtr, uint32 srcMod,
                              uint8 *dstPtr, uint32 dstMod,
                              uint32 width, uint32 height);

void BltDDRect(Rect16 &r, uint8 *srcPtr, bool bTransparent, uint16 pixMod, bool BufferToVideo)
//
// General purpose DirectDraw blitter that takes in to account the limitation where there
// is a video memory "page break".  If you do a copy to video memory that spans memory
// banks, DirectDraw hangs the computer!  This routine is used for reads and writes using
// the BufferToVideo flag.
//
{
#if 0
	uint8   *dstPtr = NULL;
	uint8   *bltSrc, *bltDst;
	uint32  start;
	uint32  end;
	uint32  firstpage;
	uint32  lastpage;
	uint32  pagex, pagey;
	uint32  i;
	uint32  offy = r.y;
	bool show = false;

	if (!displayEnabled()) //ddWindow || !ddWindow->bIsActive )
		return;

	BlittingFunc    ourBlitter;

	if (bTransparent)
		ourBlitter = _BltPixelsT;
	else {
		ourBlitter = _BltPixels;
		show = true;
	}

	start = r.x + (r.y * ddWindow->lPitch); // starting rect offset
	end = (r.x + r.width - 1) + ((r.y + r.height - 1) * ddWindow->lPitch);

	firstpage = (start >> 16);
	lastpage = (end >> 16);

	if ((ddWindow->lPitch == 1024) || (firstpage == lastpage)) { // || (((r.x & 3) == 0) && BufferToVideo))
		//
		// No special handling required here (doesn't span pages)
		//
		dstPtr = (uint8 *)ddWindow->LockBackBuffer(NULL);
		if (dstPtr) {

			bltSrc = srcPtr;
			bltDst = dstPtr + (r.y * ddWindow->lPitch) + r.x;

			if (BufferToVideo)
				ourBlitter(bltSrc, pixMod,
				           bltDst, ddWindow->lPitch,
				           r.width, r.height);
			else
				ourBlitter(bltDst, ddWindow->lPitch,
				           bltSrc, pixMod,
				           r.width, r.height);

			ddWindow->UnlockBackBuffer(dstPtr);
			dstPtr = NULL;
		} else {
			gError::warn("Failed buffer lock");
		}
		return;
	}

	dstPtr = (uint8 *)ddWindow->LockBackBuffer(NULL);

	if (dstPtr) {


		for (i = (start + 65536) & (0xFFFF0000); i <= end; i += 65536) {
			pagex = i % ddWindow->lPitch;
			pagey = i / ddWindow->lPitch;

			if ((pagex > r.x) && (pagex < (r.x + r.width)) &&
			        (pagey >= r.y) && (pagey < (r.y + r.height))) {
				// Page break is in the middle of a rectangle row, so at least 2 blits are necessary
				//
				// First blit - top rectangle

				bltSrc = srcPtr + (offy - r.y) * pixMod;
				bltDst = dstPtr + (offy * ddWindow->lPitch) + r.x;
				if (pagey - offy) {
					if (BufferToVideo)
						ourBlitter(bltSrc, pixMod,
						           bltDst, ddWindow->lPitch,
						           r.width, (pagey - offy));
					else
						ourBlitter(bltDst, ddWindow->lPitch,
						           bltSrc, pixMod,
						           r.width, (pagey - offy));
				}


				// Second blit - broken line, left of boundary
				offy = pagey;


				bltSrc = srcPtr + (offy - r.y) * pixMod;
				bltDst = dstPtr + (offy * ddWindow->lPitch) + r.x;
				if (BufferToVideo)
					ourBlitter(bltSrc, pixMod,
					           bltDst, ddWindow->lPitch,
					           pagex - r.x, 1);
				else
					ourBlitter(bltDst, ddWindow->lPitch,
					           bltSrc, pixMod,
					           pagex - r.x, 1);

				//
				// Third blit - broken line, right of boundary
				//

				bltSrc = srcPtr + (offy - r.y) * pixMod + (pagex - r.x);
				bltDst = dstPtr + (offy * ddWindow->lPitch) + pagex;
				if (BufferToVideo)
					ourBlitter(bltSrc, pixMod,
					           bltDst, ddWindow->lPitch,
					           (r.x + r.width) - pagex, 1);
				else
					ourBlitter(bltDst, ddWindow->lPitch,
					           bltSrc, pixMod,
					           (r.x + r.width) - pagex, 1);
				offy++;
			}
		}

		bltSrc = srcPtr + (offy - r.y) * pixMod;
		bltDst = dstPtr + (offy * ddWindow->lPitch) + r.x;
		if (r.y + r.height - offy > 0) {
			if (BufferToVideo)
				ourBlitter(bltSrc, pixMod,
				           bltDst, ddWindow->lPitch,
				           r.width, (r.y + r.height) - offy);
			else
				ourBlitter(bltDst, ddWindow->lPitch,
				           bltSrc, pixMod,
				           r.width, (r.y + r.height) - offy);
		}
		ddWindow->UnlockBackBuffer(dstPtr);
		dstPtr = NULL;
	} else {
		gError::warn("Failed buffer lock");
	}
#endif
	warning("STUB: BltDDRect");
}


/* ===================================================================== *
                    Member functions for display page
 * ===================================================================== */

//  Fill a rectangle on the SVGA display. Note the rect must
//  be correct -- there is no clipping or error checking...
void vWDisplayPage::fillRect(Rect16 r, uint8 color) {
	Graphics::Surface *surf = g_system->lockScreen();

	_FillRect((byte *)surf->getBasePtr(r.x, r.y), surf->pitch, r.width, r.height, color);

	g_system->unlockScreen();
}

//  Fill a rectangle on the SVGA display. Note the rect must
//  be correct -- there is no clipping or error checking...
void vWDisplayPage::invertRect(Rect16 r, uint8 color) {
}

#define USE_BLTDDRECT
// -- we'll want to use this when we figure out why bltDDRect doesnt work here
#define USE_RECT

void vWDisplayPage::writePixels(Rect16 &r, uint8 *pixPtr, uint16 pixMod) {
	g_system->copyRectToScreen(pixPtr, pixMod, r.x, r.y, r.width, r.height);
}

void vWDisplayPage::writeTransPixels(Rect16 &r, uint8 *pixPtr, uint16 pixMod) {
	if (!displayEnabled()) //ddWindow || !ddWindow->bIsActive )
		return;

	Graphics::Surface *surf = g_system->lockScreen();

	_BltPixelsT(pixPtr, pixMod, (byte *)surf->getBasePtr(r.x, r.y), surf->pitch, r.width, r.height );

	g_system->unlockScreen();
}

void vWDisplayPage::readPixels(Rect16 &r, uint8 *pixPtr, uint16 pixMod) {
	BltDDRect(r, pixPtr, false, pixMod, false);
}

//  Function to quickly transfer pixels from an off-screen
//  buffer to a rectangle on the SVGA display;
void vWDisplayPage::writeColorPixels(Rect16 r, uint8 *pixPtr, uint16 pixMod, uint8 color) {
	warning("STUB: writeColorPixels");
	writePixels(r, pixPtr, pixMod);
}

//  Function to quickly transfer pixels from an off-screen
//  buffer to a rectangle on the SVGA display;
void vWDisplayPage::writeComplementPixels(Rect16 r, uint8 *pixPtr, uint16 pixMod, uint8 color) {
	warning("STUB: writeComplementPixels");
	writePixels(r, pixPtr, pixMod);
}

} // end of namespace Saga2
