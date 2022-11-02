/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_MOVIE_PC_H__
#define ICB_MOVIE_PC_H__

#include "engines/icb/global_objects_pc.h"
#include "engines/icb/sound/direct_sound.h"
#include "engines/icb/sound.h"

#include "video/bink_decoder.h"

namespace ICB {

// Return codes for SequenceManager::DrawFrame()
#define JUSTFINE 0x0000
#define WAITING 0x0010
#define FINISHED 0x0020
#define NOMOVIE 0x0030
#define FADING 0x0040

// Our global bink handler for movies
class MovieManager;

extern MovieManager *g_theSequenceManager;

class MovieManager {
	Video::BinkDecoder *_binkDecoder;
	int32 _x;
	int32 _y;

	uint32 _fadeCounter;
	uint32 _fadeRate;

	bool8 _loop;
	bool _haveFaded;
	bool _haveClearedScreen;
	bool _rater;

public:
	MovieManager();
	~MovieManager();

	bool registerMovie(const char *fileName, bool8 fade, bool8 loop);

	uint32 getMovieHeight();
	uint32 getMovieWidth();
	uint32 getMovieFrames();

	bool busy();

	uint32 drawFrame(uint32 surface_id = working_buffer_id);
	int32 getFrameNumber();

	void setRate();

	void setVolume(int32 vol);

	void kill();

private:
	void fadeScreen(uint32 surface_id);
};

} // End of namespace ICB
#endif
