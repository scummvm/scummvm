/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TWINE_FLAMOVIES_H
#define TWINE_FLAMOVIES_H

#include "common/memstream.h"
#include "common/scummsys.h"
#include "common/file.h"
#include "graphics/palette.h"

namespace TwinE {

/** Original FLA screen width */
#define FLASCREEN_WIDTH 320
/** Original FLA screen height */
#define FLASCREEN_HEIGHT 200

/** FLA movie header structure */
struct FLAHeaderStruct {
	/** FLA version */
	int8 version[5] {0};
	/** Number of frames */
	int32 numOfFrames = 0;
	/** Frames per second */
	int16 speed = 0;
	/** Frame width */
	int16 xsize = 0;
	/** Frame height */
	int16 ysize = 0;
};

/** FLA movie frame structure */
struct FLAFrameDataStruct {
	int16 nbFrames = 0;
	int32 offsetNextFrame = 0;
};

class TwinEEngine;

class Movies {
private:
	TwinEEngine *_engine;

	Common::File _file;

	/** Auxiliar FLA fade out variable */
	int32 _fadeOut = 0;
	/** Auxiliar FLA fade out variable to count frames between the fade */
	int32 _fadeOutFrames = 0;
	bool _flagFirst = false;
	Graphics::Palette _paletteOrg{0};

	/** FLA movie file buffer */
	uint8 _flaBuffer[FLASCREEN_WIDTH * FLASCREEN_HEIGHT] {0};

	/** Number of samples in FLA movie */
	int32 _samplesInFla = 0;
	/** FLA movie header data */
	FLAHeaderStruct _flaHeaderData;
	/** FLA movie header data */
	FLAFrameDataStruct _frameData;

	void drawKeyFrame(Common::MemoryReadStream &stream, int32 width, int32 height);
	void drawDeltaFrame(Common::MemoryReadStream &stream, int32 width);
	/**
	 * Scale FLA movie 2 times
	 * According with the settins we can put the original aspect radio stretch
	 * to fullscreen or preserve it and use top and button black bars
	 */
	void scaleFla2x();
	void drawNextFrameFla();

	void prepareGIF(int index);
	void playGIFMovie(const char *flaName);

	bool playSmkMovie(const char *name, int index);

public:
	Movies(TwinEEngine *engine);

	/**
	 * Play FLA movies
	 * @param name FLA movie name
	 * @return @c true if finished. @c false if aborted.
	 */
	bool playMovie(const char *name);
};

} // namespace TwinE

#endif
