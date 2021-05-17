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

#ifndef SAGA2_VIDEO_H
#define SAGA2_VIDEO_H

#include "saga2/videomem.h"

// ------------------------------------------------------------------
// Interface selection

#define USE_DGV 0
#define USE_SMK 1
#define USE_MOV 0


#include "saga2/errclass.h"

namespace Saga2 {

typedef int HDIGDRIVER;
typedef void *Smack;
#define SMACKAUTOEXTRA 1

/* ===================================================================== *
   Definitions
 * ===================================================================== */

// ------------------------------------------------------------------
// AIL default

#ifndef TICKSPERSECOND
#define TICKSPERSECOND          ( 728L/10L )
#endif

// ------------------------------------------------------------------
// Known video interfaces

enum VideoInterface {
	VideoAutoDetect = -1,              // Let the interface detect type
	VideoNone,                         // Invalid video type
	VideoDGV,                          // DGV video
	VideoSMK,                          // Smacker video
	VideoMOV,                          // Smacker video
};

// ------------------------------------------------------------------
// Extensions for video type autodetection

#define VEXT_DGV ".DGV"
#define VEXT_SMK ".SMK"
#define VEXT_MOV ".AVI"
// Note : don't change these without recompiling the Library

// ------------------------------------------------------------------
// sizes

#define MAXVPATH 260
// Note : don't change this without recompiling the Library


/* ===================================================================== *
   Class Definitions
 * ===================================================================== */

#ifdef __cplusplus

// ------------------------------------------------------------------
// General video playback interface object
// - yeah virtual functions would have been better

class VideoPlayer {
private:
	VideoInterface  type;              // The type of video currently playing
	bool            autoEnd;
	HDIGDRIVER      dig;
	bool            toBuffer;
	uint32          bufferWidth, bufferHeight;
	bool            StartFrame;        // time to show nother frame?
#ifdef _WIN32
	HWND        hWnd;
#endif

#if USE_DGV
	// DGV internal variables
	char        pal[256 * 3];      // palette?
	uint16      newcount;          // ???
	uint16      x, y, lx, ly, x1, yy1, x2, y2; // assorted screen positions
	Image       Im;                // ???
	bool        OldSoundOn;        // ???
	bool        dopal;             // need new palette?
	bool        norebuild;         // ???
	ChunkyMap   *DGVChunkyMap;     // pointer to DGV structure
	ChunkyMap   BackChunkyMap;     // The thickerer chocolate bar
#endif
#if USE_SMK
	// SMK internal variables
	uint8       lvleft, lvright;   // current volume levels
	Smack       *smk;              // pointer to smacker structure
	uint32      i;                 // current frame
#ifdef _WIN32
	SmackBuf    *smBuf;
#endif
#endif
#if USE_MOV
	// MOV internal variables
	HINSTANCE   hInst;
	HWND        mciWnd;
	uint16      tox, toy;
#endif


public:

	void            *buffer;
	bool            playing;           // whether or not a video is playing
	bool            abort;             // terminate immediately
	char            vidPath[MAXVPATH]; // base path for videos
	char            dgvPath[MAXVPATH]; // added path for dgv videos
	char            smkPath[MAXVPATH]; // added path for smk videos
	uint32          bufferLeft, bufferTop;
	uint32          videoWidth, videoHeight;
#if USE_DGV
	// DGV public varaiables
	bool        asBack;            // show DGV as background pic only
#endif
#if USE_SMK
	// SMK public vars
	uint8       vright;            // right volume
	uint8       vleft;             // wrong volume
	uint8       changePalette;     // palette changes enabled
	uint8       restoreChangePalette; // palette changes enabled
	bool        dumpStats;         // If enabled a .DMP file w/ stats is generated for each video
#endif
#if USE_MOV
	// MOV public
#endif

private:
	void setType(VideoInterface vi);
#if USE_DGV
	// DGV specific routines
	void InitDGV(void);
	bool StartDGV(char *filespec, uint16 xxx, uint16 yyy);
	bool CheckDGV(void);
	void EndDGV(void);
	void TermDGV(void);
#endif
#if USE_SMK
	// SMK specific routines
	void InitSMK(uint32 maxTimer, bool UseBuffer = TRUE);
	bool StartSMK(char *filespec, uint16 xxx, uint16 yyy, uint32 extraBuf = SMACKAUTOEXTRA, bool noSkip = FALSE);
	bool CheckSMK(void);
	void EndSMK(void);
	void TermSMK(void);
#endif
#if USE_MOV
	// MOV specific routines
	void InitMOV(HWND hWnd, HINSTANCE inst);
	bool StartMOV(char *filespec, uint16 xxx, uint16 yyy, bool eraseWhenDone, int16 from = 0, int16 to = 0);
	bool CheckMOV(void);
	void EndMOV(void);
	void TermMOV(void);
public:
	void BeginMOV(char *fileName, int x, int y, bool erase = TRUE, int16 from = 0, int16 to = 0);
	void ResumeMOV(int16 from = 0, int16 to = 0);
	void FinaleMOV(int16 from = 0, int16 to = 0);
#endif

public:
	VideoPlayer(
#if USE_SMK
	    uint32 maxTimer, bool UseBuffer
#endif
#ifdef _WIN32
#if USE_SMK
	    ,
#endif
	    HWND wnd
#if USE_MOV
	    ,
	    HINSTANCE inst
#endif
#endif
	);
	~VideoPlayer();

	// memory management mapping

	void *operator new (size_t s) {
		return videoAlloc(s, "VidPlayer");
	}
	void operator delete (void *m) {
		videoFree(m);
	}

	// detectType attempts to determine the video type based
	//   on the filename extension

	VideoInterface detectType(char *filespec);

	// StartPlay attempts to open and start playing the video
	//   file named in filespec at the given coordinates
	//   if no file type is given detectType() will be used
	//   returns true if the video was successfully started

	bool StartPlay(char *filespec,               // Filename of video
	               uint16 xxx, uint16 yyy,      // x,y of upper left corner
#if USE_SMK
	               uint32 extraBuf = SMACKAUTOEXTRA,
	               bool noSkip = FALSE,
#endif
	               VideoInterface vi = VideoAutoDetect,
	               bool eraseWhenDone = TRUE,
	               int16 from = 0, int16 to = 0
	              ); // video type

	// CheckPlay is a video maintainence call. It takes care of
	//   any updating the video interface must do to the screen, sound
	//   palettes etc. It can be safely called whether a video is
	//   playing or not. This call should be put in a main event loop
	//   or given itsown thread.
	//   It returns true if a video is playing

	bool CheckPlay(void);

	// EndPlay is a cleanup routine for the video interfaces. It is
	//   usually called automatically by CheckPlay at the end of
	//   a video, but can be called manually to abort a video

	void EndPlay(void);

	// setBuffer tells the player that videos should be played into
	//   a memory buffer of a given size. If 0 is passed for either
	//   parameter, the screen will be used. This currently only
	//   works for SMK files

	void setBuffer(uint32 newWidth, uint32 newHeight);

	// this version explicitly sets the buffer to use

	void setBuffer(uint32 newWidth, uint32 newHeight, void *buffer);

	// bltBuffer is called when a buffer is being used to blt that
	//   buffer to the screen.

	void bltBuffer(void);

#if USE_SMK
	Smack *getSmack(void) {
		return smk;
	}
#ifdef _WIN32
	LRESULT OnPaletteChanged(HWND win_handle, WORD wparam, LONG lparam);
	LRESULT OnQueryNewPalette(HWND win_handle, WORD wparam, LONG lparam);
#endif
#endif

};
#endif


/*****************************************************************************/
/* Global DGV stuff                                                          */
/*****************************************************************************/

#if USE_DGV
#ifdef __cplusplus
extern "C" {
#endif

extern uint16 VGAScreenWidth,
       VGAScreenHeight;
extern bool SoundOn;

#ifdef __cplusplus
}
#endif
#endif

/*****************************************************************************/
/* External Hooks                                                            */
/*   To override default behaviors you can create routines with these        */
/*   prototypes that perform the appropriate function                        */
/*****************************************************************************/

/* Mouse Hook                                                                */
/*   Use this routine to eliminate mouse droppings                           */
/*   put your local mouse pointer enable/disable code in it                  */

void mouseHook(
    const bool                         // TRUE=enable FALSE=disable
) ;

/* Path Hook                                                                 */
/*   Use this routine to initialize the base paths for video files           */
/*   See the name hook for more on how these are used                        */

void pathHook(
    char *vPath,           // base path for all video files, default=".\"
    char *dPath,           // dgv file path modifier         default=""
    char *sPath            // smk file path modifier         default=""
) ;

/* Name Hook                                                                 */
/*   Use this routine to change how filenames are built                      */
/*   By default the three sections are simply concatenated into the target   */
/*   This can be modified if desired                                         */
/*   This can be modified if desired                                         */


void nameHook(
    char *targ,            // copy a valid filename into here
    const char *bpath,     // vPath from pathHook
    const char *tpath,     // either dPath or sPath from pathHook (dgv or smk)
    const char *fname      // the filename that the we are about to try to open
);

/* Audio Hooks                                                               */
/*   Use these routine to initialize audio somewhere else in the code.       */

#ifdef __cplusplus
HDIGDRIVER &audioInitHook(void);
void audioTermHook(HDIGDRIVER d);
#endif

/* blt Hook                                                                  */
/*   Override this call to use an external blitting routine                  */

void bltBufferHook(
    const void  *buffer,
    const uint16  bufferWidth,
    const uint16  bufferHeight,
    const uint16  left,
    const uint16  top,
    const uint16  width,
    const uint16  height
);


/* palette Hook                                                              */
/*   Override this call to set the palette manually                          */

void setPaletteHook(
    void *paletteMinusFour
);

/* error Hook                                                                */
/*   This hook is called to deal with memory allocation errors               */

void videoErrorHook(
    const char *errMsg
);

} // end of namespace Saga2

#endif
