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

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"

#include "saga2/video.h"
#include "saga2/config.h"

#include "saga2/panel.h"

#include "saga2/fta.h"

#include "saga2/itevideo.h"
#include "saga2/palette.h"
#include "saga2/messager.h"

namespace Saga2 {

uint32 performanceBufferSize = 512 * 1024;

#define VIDEO_EXT ".SMK"

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern gMouseState      mouseState;
extern gMousePointer    pointer;    // the actual pointer
extern configuration    globalConfig;
extern bool             cliWriteStatusF;
extern configuration    globalConfig;

#ifdef _WIN32
extern HWND             hWndMain;
extern HINSTANCE        mainInst;
#endif

//-----------------------------------------------------------------------
// External prototypes

HDIGDRIVER &digitalAudioDriver(void);

/* ===================================================================== *
   Locals
 * ===================================================================== */

static VideoPlayer *vp = NULL;

static gPort        mouseVidSavePort;           // for tweaking mouse backsave
static gPort        videoSavePort;          // for tweaking mouse backsave
static gPixelMap    tileVidDrawMap;

/* ===================================================================== *
   Code
 * ===================================================================== */

//-----------------------------------------------------------------------
// Name building hook

bool nameCheck(char name[], char ext[]) {
	size_t l = strlen(name);
	if (l < 5 || 0 != scumm_stricmp(name + (l - strlen(ext)), ext))
		strcat(name, ext);
	return TRUE; //fileExists(name);
}


//-----------------------------------------------------------------------
// Routine to play a video

void startVideo(char *fileName, int x, int y, bool erase, int16, int16) {
	char file[260];
	strncpy(file, fileName, 260);
	nameCheck(file, VIDEO_EXT);

	vp->StartPlay(file, x, y,
#if USE_SMK
	              performanceBufferSize, TRUE,
#endif
	              VideoSMK, erase);
}

//-----------------------------------------------------------------------
// General video maintainence & status check

bool checkVideo(void) {
	return vp->CheckPlay();
}

//-----------------------------------------------------------------------
// Routine to abort a video

bool abortVideo(void) {
	bool old = checkVideo();
	endVideo();
	return old;
}

//-----------------------------------------------------------------------
// Routine to end a video

void endVideo() {
	if (vp)
		vp->EndPlay();
}

//-----------------------------------------------------------------------
// Obsolete

void beginVideo(char *, int, int, bool, int16, int16) {
}

void resumeVideo(int16, int16) {
}

void finaleVideo(int16, int16) {
}


//-----------------------------------------------------------------------
// Config calls

void setVideoVolume(uint8 v) {
#if USE_SMK
	vp->vright = v;
	vp->vleft = v;
#endif
}

bool OurBlits = TRUE;

//-----------------------------------------------------------------------
// name hook

void pathHook(char *vPath, char *dPath, char *sPath) {
	char *ev;
	strcpy(vPath, ".\\");
	ev = globalConfig.videoFilePath;
	if (strlen(ev)) {
		strcpy(vPath, ev);
	}
	if (vPath[strlen(vPath) - 1] != '\\')
		strcat(vPath, "\\");
	strcpy(dPath, "");
	strcpy(sPath, "");
}

//-----------------------------------------------------------------------
// General video initialization

void initVideo(void) {
	vp = new VideoPlayer(
#if USE_SMK
	    TICKSPERSECOND, 0
#endif
#ifdef _WIN32
#if USE_SMK
	    ,
#endif
	    hWndMain
#if USE_MOV
	    ,
	    mainInst
#endif
#endif
	);
	if (vp == NULL)
		error("Cannot create video player");
#if USE_SMK && defined(_WIN32)
	if (OurBlits) {
		vp->setBuffer(640, 480);
	}
#endif
#if DEBUG
#if USE_SMK
	if (cliWriteStatusF)
		vp->dumpStats = 1;
#endif
#endif

	setVideoVolume(globalConfig.soundVolume);
}


void cleanupVideo(void) {
	endVideo();
	if (vp)
		delete vp;
	vp = NULL;
}


//-----------------------------------------------------------------------
// Video hook for blitting the buffer

#ifdef __WATCOMC__
#pragma off (unreferenced);
#endif

void bltBufferHook(const void   *buffer,
                   const uint16  bufferWidth,
                   const uint16  bufferHeight,
                   const uint16  left,
                   const uint16  top,
                   const uint16  width,
                   const uint16  height) {
#ifndef _WIN32
	Rect16 r(left, top, width, height);
	tileVidDrawMap.data = (uint8 *) buffer;
	tileVidDrawMap.size = Extent16(640, 480);
	videoSavePort.setMap(&tileVidDrawMap);
	//drawVideoMousePointer(r,(char *) buffer, bufferWidth);
#endif
}

#ifdef __WATCOMC__
#pragma on (unreferenced);
#endif

//-----------------------------------------------------------------------
// This routine blits the video buffer foreward but the mouse pointer
//   is temporarily blitted into it

void drawVideoMousePointer(
    Rect16 showZone,
    char *buffer,
    int bufferWidth
) {
#ifndef _WIN32
	gPixelMap       *currentPtr,
	                *saveMap;
	Point16         offset;
	Rect16          saveExtent,
	                blitExtent;

	//  Get the image of the pointer and the hotspot offset
	currentPtr = pointer.getImage(offset);

	//  If pointer exists, and is in a visible state
	if (currentPtr && pointer.isShown()) {
		//  Get address of pointer's backsave rect
		saveMap = pointer.getSaveMap(saveExtent);

		//  If the pointer overlaps the tile scrolling area
		if (saveExtent.overlap(showZone)) {
			//  get the intersecting area
			blitExtent = intersect(saveExtent, showZone);

			mouseVidSavePort.setMap(saveMap);
			mouseVidSavePort.setMode(drawModeReplace);

			//  Blit the tile data into the backsave buffer
			mouseVidSavePort.bltPixels(
			    tileVidDrawMap,
			    blitExtent.x - showZone.x,
			    blitExtent.y - showZone.y,
			    blitExtent.x - saveExtent.x,
			    blitExtent.y - saveExtent.y,
			    blitExtent.width,
			    blitExtent.height);

			//  Blit the mouse pointer onto the video buffer
			TBlit(&tileVidDrawMap,
			      currentPtr,
			      mouseState.pos.x + offset.x  - showZone.x,
			      mouseState.pos.y + offset.y  - showZone.y);


			// blit the video buffer to the screen
			drawPage->writePixels(showZone, (uint8 *)buffer, bufferWidth);

			// restore mouse area
			//videoSavePort.bltPixels(ngpm,0,0,x,y,w,h);
			videoSavePort.setMode(drawModeReplace);
			videoSavePort.bltPixels(
			    *saveMap,
			    blitExtent.x - saveExtent.x,
			    blitExtent.y - saveExtent.y,
			    blitExtent.x - showZone.x,
			    blitExtent.y - showZone.y,
			    blitExtent.width,
			    blitExtent.height);

		} else {
			// blit the video buffer to the screen
			drawPage->writePixels(showZone,(uint8 *)buffer, bufferWidth);
		}
	} else {
		// blit the video buffer to the screen
		drawPage->writePixels(showZone, (uint8 *)buffer, bufferWidth);
	}
#endif
}



//-----------------------------------------------------------------------
// Video hook for crude mouse enabling / disabling

void mouseHook(const bool) {
}


void videoErrorHook(
    const char *errMsg
) {
	error("%s", errMsg);
}



//-----------------------------------------------------------------------
// Video hook for generating filenames
#define F_OK 0

#define FileForm "%s"

inline bool file_exists(const char f[]) {
#if 0
	return (!access(f, F_OK));
#endif
	warning("STUB: file_exists");
	return false;
}

void nameHook(char *targ, const char *bpath, const char *, const char *fname) {
	char filename[260] = "";
	VERIFY(targ);

	if (strcmp(bpath, ".\\")) {   //if env var set
		strcpy(filename, bpath);
		if (filename[strlen(filename) - 1] != '\\')
			strcat(filename, "\\");
		strcat(filename, fname);
		//if file not there
		if (file_exists(filename)) {
			sprintf(targ, FileForm, filename);
			return;
		}
	}
	sprintf(targ, FileForm, fname);
	return;
}

//-----------------------------------------------------------------------
// Video hook to get the AIL digital audio driver

HDIGDRIVER &audioInitHook(void) {
	return digitalAudioDriver();
}

//-----------------------------------------------------------------------
// Video hook to free the AIL digital audio driver

void audioTermHook(HDIGDRIVER) {}



/* palette Hook                                                              */
/*   Override this call to set the palette manually                          */

extern  UByteHandle         newPalette,
        our64Palette;

void setPaletteHook(
    void *paletteMinusFour
) {
#ifndef _WIN32
	gPalette *gp = (gPalette *)(void *)(((uint8 *) paletteMinusFour));
	_LoadPalette((uint8 *)paletteMinusFour, 0, 256);
#endif
}


bool InVideo(void) {
#if USE_SMK && defined(_WIN32)
	if (NULL != vp->getSmack())
		return TRUE;
#endif
	return FALSE;
}

#ifdef _WIN32

LRESULT VideoOnPaletteChanged(HWND win_handle, WORD wparam, LONG lparam) {
	return vp->OnPaletteChanged(win_handle, wparam, lparam);
}

LRESULT VideoOnQueryNewPalette(HWND win_handle, WORD wparam, LONG lparam) {
	return vp->OnQueryNewPalette(win_handle, wparam, lparam);
}

#endif

} // end of namespace Saga2
