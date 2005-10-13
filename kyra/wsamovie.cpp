/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"


namespace Kyra {

WSAMovieV1 *KyraEngine::wsa_open(const char *filename, int offscreenDecode, uint8 *palBuf) {
	debug(9, "KyraEngine::wsa_open('%s', %d, 0x%X)", filename, offscreenDecode, palBuf);
	uint32 flags = 0;
	uint32 fileSize;
	uint8 *p = _res->fileData(filename, &fileSize);
	
	WSAMovieV1 *wsa = new WSAMovieV1;
	const uint8 *wsaData = p;
	wsa->numFrames = READ_LE_UINT16(wsaData); wsaData += 2;
	wsa->width = READ_LE_UINT16(wsaData); wsaData += 2;
	wsa->height = READ_LE_UINT16(wsaData); wsaData += 2;
	wsa->deltaBufferSize = READ_LE_UINT16(wsaData); wsaData += 2;
	wsa->offscreenBuffer = NULL;
	wsa->flags = 0;
	if (_features & GF_TALKIE) {
		flags = READ_LE_UINT16(wsaData); wsaData += 2;
	}
	
	uint32 offsPal = 0;
	if (flags & 1) {
		offsPal = 0x300;
		wsa->flags |= WF_HAS_PALETTE;
		if (palBuf) {
			memcpy(palBuf, wsaData + (wsa->numFrames + 2) * 4, 0x300);
		}
	}
	
	if (offscreenDecode) {
		wsa->flags |= WF_OFFSCREEN_DECODE;
		const int offscreenBufferSize = wsa->width * wsa->height;
		wsa->offscreenBuffer = (uint8 *)malloc(offscreenBufferSize);
		memset(wsa->offscreenBuffer, 0, offscreenBufferSize);
	}

	if (wsa->numFrames & 0x8000) {
		warning("Unhandled wsa flags 0x80");
		wsa->flags |= 0x80;
		wsa->numFrames &= 0x7FFF;
	}
	wsa->currentFrame = wsa->numFrames;

	wsa->deltaBuffer = (uint8 *)malloc(wsa->deltaBufferSize);
	memset(wsa->deltaBuffer, 0, wsa->deltaBufferSize);
	
	// read frame offsets
	wsa->frameOffsTable = (uint32 *)malloc((wsa->numFrames + 2) * 4);
	wsa->frameOffsTable[0] = 0;
	uint32 frameDataOffs = READ_LE_UINT32(wsaData); wsaData += 4;
	bool firstFrame = true;
	if (frameDataOffs == 0) {
		firstFrame = false;
		frameDataOffs = READ_LE_UINT32(wsaData);
		wsa->flags |= WF_NO_FIRST_FRAME;
	}
	for (int i = 1; i < wsa->numFrames + 2; ++i) {
		wsa->frameOffsTable[i] = READ_LE_UINT32(wsaData) - frameDataOffs;
		wsaData += 4;
	}
	
	// skip palette
	wsaData += offsPal;
	
	// read frame data
	const int frameDataSize = p + fileSize - wsaData;
	wsa->frameData = (uint8 *)malloc(frameDataSize);
	memcpy(wsa->frameData, wsaData, frameDataSize);
	
	// decode first frame
	if (firstFrame) {
		Screen::decodeFrame4(wsa->frameData, wsa->deltaBuffer, wsa->deltaBufferSize);
	}
	
	delete[] p;
	return wsa;
}

void KyraEngine::wsa_close(WSAMovieV1 *wsa) {
	debug(9, "KyraEngine::wsa_close(0x%X)", wsa);
	if (wsa) {
		free(wsa->deltaBuffer);
		free(wsa->offscreenBuffer);
		free(wsa->frameOffsTable);
		delete wsa;
	}
}

uint16 KyraEngine::wsa_getNumFrames(WSAMovieV1 *wsa) const {
	debug(9, "KyraEngine::wsa_getNumFrames(0x%X)", wsa);
	uint16 n = 0;
	if (wsa) {
		n = wsa->numFrames;
	}
	return n;
}

void KyraEngine::wsa_play(WSAMovieV1 *wsa, int frameNum, int x, int y, int pageNum) {
	debug(9, "KyraEngine::wsa_play(0x%X, %d, %d, %d, %d)", wsa, frameNum, x, y, pageNum);
	assert(frameNum <= wsa->numFrames);

	uint8 *dst;
	if (wsa->flags & WF_OFFSCREEN_DECODE) {
		dst = wsa->offscreenBuffer;
	} else {
		dst = _screen->getPagePtr(pageNum) + y * Screen::SCREEN_W + x;
	}
		
	if (wsa->currentFrame == wsa->numFrames) {
		if (!(wsa->flags & WF_NO_FIRST_FRAME)) {
			if (wsa->flags & WF_OFFSCREEN_DECODE) {
				Screen::decodeFrameDelta(dst, wsa->deltaBuffer);
			} else {
				Screen::decodeFrameDeltaPage(dst, wsa->deltaBuffer, wsa->width);
			}
		}
		wsa->currentFrame = 0;
	}

	// try to reduce the number of needed frame operations
	int diffCount = ABS(wsa->currentFrame - frameNum);
	int frameStep = 1;
	int frameCount;
	if (wsa->currentFrame < frameNum) {
		frameCount = wsa->numFrames - frameNum + wsa->currentFrame;
		if (diffCount > frameCount) {
			frameStep = -1;
		} else {
			frameCount = diffCount;
		}
	} else {
		frameCount = wsa->numFrames - wsa->currentFrame + frameNum;
		if (frameCount >= diffCount) {
			frameStep = -1;
			frameCount = diffCount;
		}
	}
	
	// process
	if (frameStep > 0) {
		uint16 cf = wsa->currentFrame;
		while (frameCount--) {
			cf += frameStep;
			wsa_processFrame(wsa, cf, dst);
			if (cf == wsa->numFrames) {
				cf = 0;
			}
		}
	} else {
		uint16 cf = wsa->currentFrame;
		while (frameCount--) {
			if (cf == 0) {
				cf = wsa->numFrames;
			}
			wsa_processFrame(wsa, cf, dst);
			cf += frameStep;
		}
	}
	
	// display
	wsa->currentFrame = frameNum;
	if (wsa->flags & WF_OFFSCREEN_DECODE) {
		_screen->copyBlockToPage(pageNum, x, y, wsa->width, wsa->height, wsa->offscreenBuffer);
	}
}

void KyraEngine::wsa_processFrame(WSAMovieV1 *wsa, int frameNum, uint8 *dst) {
	debug(9, "KyraEngine::wsa_processFrame(0x%X, %d, 0x%X)", wsa, frameNum, dst);
	assert(frameNum <= wsa->numFrames);
	const uint8 *src = wsa->frameData + wsa->frameOffsTable[frameNum];
	Screen::decodeFrame4(src, wsa->deltaBuffer, wsa->deltaBufferSize);
	if (wsa->flags & WF_OFFSCREEN_DECODE) {
		Screen::decodeFrameDelta(dst, wsa->deltaBuffer);
	} else {
		Screen::decodeFrameDeltaPage(dst, wsa->deltaBuffer, wsa->width);
	}
}
	
} // end of namespace Kyra
