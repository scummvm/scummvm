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

#ifndef TWINE_FLAMOVIES_H
#define TWINE_FLAMOVIES_H

#include "common/memstream.h"
#include "common/scummsys.h"
#include "common/file.h"

namespace TwinE {

/** Original FLA screen width */
#define FLASCREEN_WIDTH 320
/** Original FLA screen height */
#define FLASCREEN_HEIGHT 200

/** FLA movie header structure */
struct FLAHeaderStruct {
	/** FLA version */
	int8 version[6] {0};
	/** Number of frames */
	int32 numOfFrames = 0;
	/** Frames per second */
	int8 speed = 0;
	/** Unknown var1 */
	int8 var1 = 0;
	/** Frame width */
	int16 xsize = 0;
	/** Frame height */
	int16 ysize = 0;
};

/** FLA movie frame structure */
struct FLAFrameDataStruct {
	/** Current frame size */
	int8 videoSize = 0;
	/** Dummy variable */
	int8 dummy = 0;
	/** Unknown frameVar0 */
	int32 frameVar0 = 0;
};

class TwinEEngine;

class FlaMovies {
private:
	TwinEEngine *_engine;

	Common::File file;

	/** Auxiliar FLA fade out variable */
	int32 _fadeOut = 0;
	/** Auxiliar FLA fade out variable to count frames between the fade */
	int32 fadeOutFrames = 0;

	/** FLA movie file buffer */
	uint8 flaBuffer[FLASCREEN_WIDTH * FLASCREEN_HEIGHT] {0};

	/** Number of samples in FLA movie */
	int32 samplesInFla = 0;
	/** FLA movie header data */
	FLAHeaderStruct flaHeaderData;
	/** FLA movie header data */
	FLAFrameDataStruct frameData;

	void drawKeyFrame(Common::MemoryReadStream &stream, int32 width, int32 height);
	void drawDeltaFrame(Common::MemoryReadStream &stream, int32 width);
	/**
	 * Scale FLA movie 2 times
	 * According with the settins we can put the original aspect radio stretch
	 * to fullscreen or preserve it and use top and button black bars
	 */
	void scaleFla2x();
	void processFrame();

	void prepareGIF(int index);
	void playGIFMovie(const char *flaName);

public:
	FlaMovies(TwinEEngine *engine);

	/**
	 * Play FLA movies
	 * @param flaName FLA movie name
	 */
	void playFlaMovie(const char *flaName);
};

} // namespace TwinE

#endif
