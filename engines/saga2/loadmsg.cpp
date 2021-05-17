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

#include "saga2/std.h"

#include "saga2/gdraw.h"
#include "saga2/vwpage.h"
#include "saga2/loadmsg.h"
#include "saga2/palette.h"
#include "saga2/display.h"

namespace Saga2 {

struct wPaletteEntry {
	uint8           r,
	                g,
	                b,
	                pad;
};

//  An entire palette of 256 colors

struct wPalette {
	wPaletteEntry   entry[ 256 ];
};


static gPalette         normalPalette;      //  Currently loaded palette


void w2gPalette(wPalette *w, gPalette *g) {
	for (int i = 0; i < 256; i++) {
		g->entry[ i ].r = w->entry[i].r >> 2;
		g->entry[ i ].g = w->entry[i].g >> 2;
		g->entry[ i ].b = w->entry[i].b >> 2;
	}
}


void useWPalette(wPalette *wp) {
	w2gPalette(wp, &normalPalette);
	setCurrentPalette(&normalPalette);
}



//void usePalette( void *p, size_t s);
//extern "C" void cdecl _BltPixels( uint8 *srcPtr, uint32 srcMod,
//									uint8 *dstPtr, uint32 dstMod,
//									uint32 width, uint32 height );

extern gDisplayPort         mainPort;               // default rendering port
extern uint8 normalPal[1024];
extern uint32 loadingWindowWidth;
extern uint32 loadingWindowHeight;
extern uint8 loadingWindowData[];
extern uint8 loadingWindowPalette[];

int16 showBitmapAtX = 0;
int16 showBitmapAtY = 0;

gPixelMap mess;

#ifdef _WIN32
extern CFTWindow *pWindow;
void displayEventLoop(void);
#endif

static bool inLoadMode = FALSE;

void initLoadMode(void) {
	inLoadMode = TRUE;
}

void updateLoadMode(void) {
	if (inLoadMode) {
#ifdef _WIN32
		resumeDDGraphics();
#endif
		mess.data = &loadingWindowData[0];
		mess.size.x = loadingWindowWidth;
		mess.size.y = loadingWindowHeight;
		mainPort.bltPixels(mess, 0, 0, showBitmapAtX, showBitmapAtY, loadingWindowWidth, loadingWindowHeight);
#ifdef _WIN32
		pWindow->Flip();
		//mainPort.bltPixels(  mess,0,0,showBitmapAtX,showBitmapAtY,loadingWindowWidth,loadingWindowHeight);
		//pWindow->Flip();
#endif
		useWPalette((wPalette *) loadingWindowPalette); //normalPal
#ifdef _WIN32
		suspendDDGraphics();
#endif
	}
}

void closeLoadMode(void) {
	inLoadMode = FALSE;
	//blackOut();
}

void loadingScreen(void) {
	initLoadMode();
	updateLoadMode();
}

} // end of namespace Saga2
