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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRAPHICS_VIDEO_MPEG_PLAYER_H
#define GRAPHICS_VIDEO_MPEG_PLAYER_H

#include "common/scummsys.h"
#include "graphics/pixelformat.h"

// Uncomment this if you are using libmpeg2 0.3.1.
// #define USE_MPEG2_0_3_1

#ifdef USE_MPEG2

#if defined(__PLAYSTATION2__)
	typedef uint8 uint8_t;
	typedef uint16 uint16_t;
	typedef uint32 uint32_t;
#elif defined(_WIN32_WCE)
	typedef signed char int8_t;
	typedef signed short int16_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
#elif defined(_MSC_VER)
	typedef signed char int8_t;
	typedef signed short int16_t;
	typedef unsigned char uint8_t;
	typedef unsigned short uint16_t;
	#if !defined(SDL_COMPILEDVERSION) || (SDL_COMPILEDVERSION < 1210)
	typedef signed long int32_t;
	typedef unsigned long uint32_t;
	#endif
#elif !defined(PALMOS_MODE)
#	include <inttypes.h>
#else
#	include <stdint.h>
#endif

extern "C" {
	#include <mpeg2dec/mpeg2.h>
}

#ifdef USE_MPEG2_0_3_1
typedef int mpeg2_state_t;
typedef sequence_t mpeg2_sequence_t;
#define STATE_BUFFER -1
#endif

#endif

#ifdef BACKEND_8BIT
#define SQR(x) ((x) * (x))
#define SHIFT 3
#else
#define SHIFT 1
#endif

#define BITDEPTH (1 << (8 - SHIFT))
#define ROUNDADD (1 << (SHIFT - 1))

#define BUFFER_SIZE 4096

namespace Common {
	class File;
}

class OSystem;

namespace Graphics {

class BaseAnimationState {
protected:
	const int _movieWidth;
	const int _movieHeight;

	int _frameWidth;
	int _frameHeight;

#ifndef BACKEND_8BIT
	int _movieScale;
#endif

	OSystem *_sys;

	uint _frameNum;

#ifdef USE_MPEG2
	mpeg2dec_t *_mpegDecoder;
	const mpeg2_info_t *_mpegInfo;
#endif

	Common::File *_mpegFile;

#ifdef BACKEND_8BIT
	int _palNum;
	int _maxPalNum;

	byte _yuvLookup[2][(BITDEPTH+1) * (BITDEPTH+1) * (BITDEPTH+1)];
	byte *_lut;
	byte *_lut2;
	int _lutCalcNum;

	int _curPal;
	int _cr;
	int _pos;

	struct {
		uint cnt;
		uint end;
		byte pal[4 * 256];
	} _palettes[50];
#else
	OverlayColor *_overlay;
	Graphics::PixelFormat _overlayFormat;
	int16 *_colorTab;
	OverlayColor *_rgbToPix;
#endif

public:
	BaseAnimationState(OSystem *sys, int width, int height);
	virtual ~BaseAnimationState();

	bool init(const char *name);
	bool decodeFrame();
	void handleScreenChanged();
	void updateScreen();

#ifndef BACKEND_8BIT
	void buildLookup();
#endif

	int getFrameWidth() { return _frameWidth; }
	int getFrameHeight() { return _frameHeight; }

protected:
	bool checkPaletteSwitch();
	virtual void drawYUV(int width, int height, byte *const *dat) = 0;

#ifdef BACKEND_8BIT
	void buildLookup(int p, int lines);
	virtual void setPalette(byte *pal) = 0;
#else
	void plotYUV(int width, int height, byte *const *dat);
	void plotYUV1x(int width, int height, byte *const *dat);
	void plotYUV2x(int width, int height, byte *const *dat);
	void plotYUV3x(int width, int height, byte *const *dat);
#endif
};

} // End of namespace Graphics

#endif
