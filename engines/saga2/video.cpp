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

// Tiny Smacker player for the Miles Sound System

#include "saga2/std.h"
#include "saga2/video.h"

namespace Saga2 {

/*****************************************************************************/
/* globals                                                                   */

char tNameBuffer[260];

/*****************************************************************************/
/* ctor                                                                      */

VideoPlayer::VideoPlayer(
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
) {
	type = VideoNone;
#ifdef _WIN32
	hWnd = wnd;
#endif
	autoEnd = FALSE;
	toBuffer = FALSE;
	buffer = NULL;
	bufferWidth = bufferHeight = bufferLeft = bufferTop = 0;
	pathHook(vidPath, dgvPath, smkPath);
#if USE_SMK
#ifdef _WIN32
	smBuf = NULL;
#endif
	dig = audioInitHook();
	InitSMK(maxTimer, UseBuffer);
#endif
#if USE_MOV
	InitMOV(wnd, inst);
#endif
}


/*****************************************************************************/
/* dtor                                                                      */

VideoPlayer::~VideoPlayer() {
	setType(VideoNone);
#if USE_SMK
	TermSMK();
#endif
#if USE_MOV
	TermMOV();
#endif
	audioTermHook(dig);
}


/*****************************************************************************/
/* set up video buffer                                                       */

void VideoPlayer::setBuffer(uint32 newWidth, uint32 newHeight) {
	bufferWidth = newWidth;
	toBuffer = (newWidth  != 0 &&
	            newHeight != 0);
	if (toBuffer)
		bufferHeight = newHeight;
	else {
		bufferHeight = 0;
	}
	buffer = NULL;
}

void VideoPlayer::setBuffer(uint32 newWidth, uint32 newHeight, void *buf) {
	bufferWidth = newWidth;
	toBuffer = (newWidth != 0 && newHeight != 0);
	if (toBuffer) {
		bufferHeight = newHeight;
		buffer = buf;
	} else {
		bufferHeight = 0;
		buffer = NULL;
	}
}

/*****************************************************************************/
/* blitting call                                                             */

void VideoPlayer::bltBuffer(void) {
	bltBufferHook(buffer, bufferWidth, bufferHeight, bufferLeft, bufferTop, videoWidth, videoHeight);
}


/*****************************************************************************/
/* file type detection                                                       */

VideoInterface VideoPlayer::detectType(char *filespec) {
	size_t l = strlen(filespec);
	if (l < 5)
		return VideoNone;
//	if ( 0==stricmp(filespec+(l-strlen(VEXT_DGV)),VEXT_DGV) )
//		return VideoDGV;
#if USE_MOV
	if (0 == stricmp(filespec + (l - strlen(VEXT_MOV)), VEXT_MOV))
		return VideoMOV;
#endif
#if USE_SMK
	//if ( 0==stricmp(filespec+(l-strlen(VEXT_SMK)),VEXT_SMK) )
	return VideoSMK;
#else
	return VideoNone;
#endif
}


/*****************************************************************************/
/* type initialization to avoid multiple plays                               */

void VideoPlayer::setType(VideoInterface vi) {
	if (type && vi) EndPlay();
	type = vi;
}


/*****************************************************************************/
/* Play                                                                      */
#ifdef __WATCOMC__
#pragma off (unreferenced);
#endif

bool VideoPlayer::StartPlay(char *filespec,
                            uint16 xxx, uint16 yyy,
#if USE_SMK
                            uint32 extraBuf,
                            bool noSkip,
#endif
                            VideoInterface vi,
                            bool eraseWhenDone,
                            int16 from,
                            int16 to
                           ) {
	bool res;
	char fullPath[MAXVPATH];

	if (vi == VideoAutoDetect)
		vi = detectType(filespec);
	setType(vi);
	mouseHook(FALSE);
	if (toBuffer) {
		bufferLeft = xxx;
		bufferTop = yyy;
	}
	strcpy(tNameBuffer, filespec);
	switch (vi) {
//		case VideoDGV:
//			nameHook(fullPath,vidPath,dgvPath,filespec);
//			res=StartDGV(fullPath,xxx,yyy);
//			break;
#if USE_SMK
	case VideoSMK:
		nameHook(fullPath, vidPath, smkPath, filespec);
		res = StartSMK(fullPath, xxx, yyy, extraBuf, noSkip);
		break;
#endif
#if USE_MOV
	case VideoMOV:
		nameHook(fullPath, vidPath, smkPath, filespec);

		res = StartMOV(fullPath, xxx, yyy, eraseWhenDone, from, to);

		break;
#endif
	case VideoNone:
		res = FALSE;
		break;
	default:
		ASSERT(0);
		res = FALSE;
	}
	mouseHook(TRUE);
	playing = res;
	if (!res)
		setType(VideoNone);
	else
		abort = FALSE;
	return res;

}

#ifdef __WATCOMC__
#pragma on (unreferenced);
#endif

/*****************************************************************************/
/* Update & status                                                           */

bool VideoPlayer::CheckPlay(void) {
	bool res = FALSE;
	if (playing && type) {
		mouseHook(FALSE);
		autoEnd = TRUE;
		switch (type) {
//			case VideoDGV:      res=CheckDGV(); break;
#if USE_SMK
		case VideoSMK:
			res = CheckSMK();
			break;
#endif
#if USE_MOV
		case VideoMOV:
			res = CheckMOV();
			break;
#endif
		case VideoNone:
			break;
		default:
			ASSERT(0);
		}
		autoEnd = FALSE;
		mouseHook(TRUE);
	}
	return res;
}


/*****************************************************************************/
/* Stop                                                                      */

void VideoPlayer::EndPlay(void) {
	abort = (!autoEnd);
	playing = FALSE;
	mouseHook(FALSE);
	switch (type) {
	case VideoNone:
		break;
//		case VideoDGV:      EndDGV(); break;
#if USE_SMK
	case VideoSMK:
		EndSMK();
		break;
#endif
#if USE_MOV
	case VideoMOV:
		EndMOV();
		break;
#endif
	default:
		ASSERT(0);
	}
	mouseHook(TRUE);
	setType(VideoNone);
}

} // end of namespace Saga2
