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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "allfiles.h"

#ifndef _MSC_VER        // Microsoft compiler?
#include <unistd.h>     // For unlink
#else
#include <io.h>
#include <stdio.h>
#endif

#include "newfatal.h"
#include "stringy.h"
#if 0
#include "vfw.h"
#endif
#include "sound.h"
#include "colours.h"
#include "fileset.h"

namespace Sludge {

struct timStream {
	PAVISTREAM      got;
	AVISTREAMINFO   info;
	LONG            chunkSize;
	LPBYTE          chunk;
};

static PAVIFILE pAviFile = NULL;
static PGETFRAME pgf;
static timStream audio, video;
static int videoFrameNum = 0;
static int vidBytesPerPixel = 0;
static int vidWidth, vidHeight;
static DWORD biSize;

static char *videoFile = NULL;

bool videoPlaying = false;

extern int winWidth, winHeight;
extern unsigned short int *screen;
extern HWND hMainWindow;

void initialiseMovieStuff() {
	char buffer[500];
	if (ExpandEnvironmentStrings("%temp%", buffer, 499) == 0) buffer[0] = NULL;
	videoFile = joinStrings(buffer, "\\test.avi");
//	warning (videoFile);
}

bool getStream(DWORD type, timStream &intoHere) {
	if (AVIFileGetStream(pAviFile, & intoHere.got, type, 0)) {
		intoHere.got = NULL;
		return true;
	} else if (AVIStreamInfo(intoHere.got, & intoHere.info, sizeof(AVISTREAMINFO))) {
		return fatal("Can't get stream info");
	} else if (AVIStreamReadFormat(intoHere.got, AVIStreamStart(intoHere.got), NULL, & intoHere.chunkSize)) {
		return fatal("Can't get stream chunk size");
	} else {
		// So far so good! Let's read a chunk of data (huh?)

		intoHere.chunk = new BYTE[intoHere.chunkSize];

		if (! intoHere.chunk) {
			return fatal("Out of memory");
		} else if (AVIStreamReadFormat(intoHere.got, AVIStreamStart(intoHere.got), intoHere.chunk, & intoHere.chunkSize)) {
			return fatal("Couldn't read stream format");
		}
	}
	return true;
}

void killStream(timStream &intoHere) {
	delete intoHere.chunk;
	intoHere.chunk = NULL;
}

/*
#define WAVHEADERSIZE 20
char wavHeader[WAVHEADERSIZE] = {'R', 'I', 'F', 'F', 0, 0, 0, 0,
                                 'W', 'A', 'V', 'E', 'f', 'm', 't',
                                 0x20, 0x10, 0, 0, 0};

void handleAudio () {
    LONG aSize;

    if(AVIStreamRead(audio.got, 0, AVISTREAMREAD_CONVENIENT, NULL, 0, & aSize, NULL)) return;

    int totalSize = aSize + audio.chunkSize + WAVHEADERSIZE + 4;

    LPBYTE pBuffer = new BYTE[totalSize];
    if (!pBuffer) return;

    memcpy (pBuffer, wavHeader, WAVHEADERSIZE);
    pBuffer[4] = (char) (aSize);
    pBuffer[5] = (char) (aSize >> 8);
    memcpy (pBuffer + WAVHEADERSIZE, audio.chunk, audio.chunkSize);
    memcpy (pBuffer + WAVHEADERSIZE + audio.chunkSize, "data", 4);

    if(! AVIStreamRead (audio.got, 0, AVISTREAMREAD_CONVENIENT, pBuffer + audio.chunkSize + WAVHEADERSIZE + 4, aSize, NULL, NULL)) {
        FILE * fp = fopen ("test.wav", "wb");
        if (fp) {
            fwrite (pBuffer, 1, totalSize, fp);
            fclose (fp);
        }
        int i = fakeCacheSoundForVideo ((char *) pBuffer, totalSize);
        if (i != -1) startSound (i, false);
    }

    delete pBuffer;
}
*/

void finishVideo() {
	videoPlaying = false;
	AVIStreamGetFrameClose(pgf);
	if (audio.got) AVIStreamRelease(audio.got);
	if (video.got) AVIStreamRelease(video.got);
	killStream(audio);
	killStream(video);
	AVIFileRelease(pAviFile);
	AVIFileExit();
#ifdef _MSC_VER
	_unlink(videoFile);
#else
	unlink(videoFile);
#endif
}

#define COPYSIZE 256

bool extractSlice(int fileNum, char *toName) {
	unsigned char buff[COPYSIZE];

	unsigned long fileLength = openFileFromNum(fileNum);
	if (! fileLength) return false; // Error already displayed
#if 0
	FILE *copyVid = fopen(toName, "wb");
	if (! copyVid) return fatal("Can't extract resource");

	while (fileLength >= COPYSIZE) {
		fread(buff, COPYSIZE, 1, bigDataFile);
		if (fwrite(buff, 1, COPYSIZE, copyVid) != COPYSIZE) return fatal("Out of disk space extracting resource");
		fileLength -= COPYSIZE;
	}
	if (fileLength) {
		fread(buff, fileLength, 1, bigDataFile);
		if (fwrite(buff, 1, fileLength, copyVid) != fileLength) return fatal("Out of disk space extracting resource");
	}

	fclose(copyVid);
#endif
	finishAccess();

	return true;
}

bool startVideo(int fileNum) {

	setResourceForFatal(fileNum);

	AVIFILEINFO info;

	if (videoPlaying) finishVideo();
	AVIFileInit();

	if (! extractSlice(fileNum, videoFile)) return false;
	if (AVIFileOpen(& pAviFile, videoFile, OF_READ, NULL))
		return fatal(ERROR_AVI_FILE_ERROR);

	AVIFileInfo(pAviFile, &info, sizeof(info));

	if (! getStream(streamtypeAUDIO, audio)) return false;
	if (! getStream(streamtypeVIDEO, video)) return false;

	if (! video.got) return fatal(ERROR_AVI_NO_STREAM);

//	if (audio.got) handleAudio ();

	pgf = AVIStreamGetFrameOpen(video.got, NULL);
	if (!pgf) return fatal(ERROR_AVI_ARGH);

	LPBITMAPINFO pInfo = (LPBITMAPINFO)(video.chunk);
	vidBytesPerPixel = pInfo -> bmiHeader.biBitCount / 8;
	biSize = pInfo -> bmiHeader.biSize;
	vidWidth = pInfo -> bmiHeader.biWidth;
	vidHeight = pInfo -> bmiHeader.biHeight;

	videoFrameNum = 0;
	videoPlaying = true;

	setResourceForFatal(-1);

	return true;
}

bool nextVideoFrame() {
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER) AVIStreamGetFrame(pgf, videoFrameNum);
	if (! lpbi) {
		finishVideo();
		return false;
	}

	BYTE *pData = (((BYTE *) lpbi) + lpbi->biSize);

	int xOff = (winWidth - vidWidth) >> 1;
	int yOff = (winHeight + vidHeight) >> 1;
	unsigned short int *startingPoint = screen + xOff + (yOff * winWidth);

	for (int y = 0; y < vidHeight; y ++) {
		startingPoint -= winWidth;
		unsigned short int *toHere = startingPoint;
		for (int x = 0; x < vidWidth; x ++) {
			switch (vidBytesPerPixel) {
			case 1:
				(* toHere) = makeGrey(*pData);
				break;

			case 3:
			case 4:
				(* toHere) = makeColour(*(pData + 2), *(pData + 1), *pData);
				break;

			default: {
				WORD Pixel16 = * ((WORD *) pData);
				(* toHere) = makeColour(
				                 (((UINT)(Pixel16) >> 10) & 0x1F) << 3,
				                 (((UINT)(Pixel16) >> 5) & 0x1F) << 3,
				                 (((UINT)(Pixel16) >> 0) & 0x1F) << 3);
			}
			break;
			}
			pData += vidBytesPerPixel;
			toHere ++;
		}
	}

	videoFrameNum ++;

	return true;
}

} // End of namespace Sludge
