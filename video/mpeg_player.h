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
 */

#ifndef VIDEO_MPEG_PLAYER_H
#define VIDEO_MPEG_PLAYER_H

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
#else
#	include <inttypes.h>
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

#define SHIFT 1

#define BITDEPTH (1 << (8 - SHIFT))
#define ROUNDADD (1 << (SHIFT - 1))

#define BUFFER_SIZE 4096

namespace Common {
class File;
}

class OSystem;

namespace Video {

class BaseAnimationState {
protected:
	const int _movieWidth;
	const int _movieHeight;

	int _frameWidth;
	int _frameHeight;

	int _movieScale;

	OSystem *_sys;

	uint _frameNum;

#ifdef USE_MPEG2
	mpeg2dec_t *_mpegDecoder;
	const mpeg2_info_t *_mpegInfo;
#endif

	Common::File *_mpegFile;

	OverlayColor *_overlay;
	Graphics::PixelFormat _overlayFormat;
	int16 *_colorTab;
	OverlayColor *_rgbToPix;

public:
	BaseAnimationState(OSystem *sys, int width, int height);
	virtual ~BaseAnimationState();

	bool init(const char *name);
	bool decodeFrame();
	void handleScreenChanged();
	void updateScreen();

	void buildLookup();

	int getFrameWidth() { return _frameWidth; }
	int getFrameHeight() { return _frameHeight; }

protected:
	bool checkPaletteSwitch();
	virtual void drawYUV(int width, int height, byte *const *dat) = 0;

	void plotYUV(int width, int height, byte *const *dat);
	void plotYUV1x(int width, int height, byte *const *dat);
	void plotYUV2x(int width, int height, byte *const *dat);
	void plotYUV3x(int width, int height, byte *const *dat);
};

} // End of namespace Video

#endif
