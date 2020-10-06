/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_MOVIE_PC_H__
#define ICB_MOVIE_PC_H__

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/bink_stub.h"
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

class SequenceManager {
private:
	HBINK m_binkObject;

	uint32 m_x;     // X offset in screen
	uint32 m_y;     // Y offset in screen
	uint32 m_flags; // Surface type
	uint32 m_fadeCounter;
	uint32 m_fadeRate;

	bool8 m_loop;
	bool8 m_haveFaded;
	bool8 m_haveClearedScreen;
	bool8 m_rater;
	bool8 m_mission1intro;

	uint32 m_endAtFrame;

public:
	SequenceManager();
	~SequenceManager();

	bool8 Register(const char *fileName, bool8 fade, bool8 loop, uint32 flags = 0);

	uint32 GetMovieHeight();
	uint32 GetMovieWidth();
	uint32 GetMovieFrames();

	bool8 Busy();

	uint32 DrawFrame(uint32 surface_id = working_buffer_id);
	int GetFrameNumber();

	void SetRate();
	void SetEndFrame(uint32 f) { m_endAtFrame = f; }

	void SetVolume(int32 vol);

	void Kill();

private:
	void FadeScreen(uint32 surface_id);
};

class MovieManager {
	Video::BinkDecoder *_binkDecoder;
	int _x;
	int _y;

public:
	MovieManager() : _x(0), _y(0) {
		_binkDecoder = new Video::BinkDecoder();
		_binkDecoder->setDefaultHighColorFormat(Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));
	};
	~MovieManager() { delete _binkDecoder; };

	bool registerMovie(const char *fileName, bool8 fade, bool8 loop);
	uint32 getMovieHeight() {
		if (!busy())
			return 0;
		return _binkDecoder->getHeight();
	};
	uint32 getMovieWidth() {
		if (!busy())
			return 0;
		return _binkDecoder->getWidth();
	};
	uint32 getMovieFrames() {
		if (!busy())
			return 0;
		return _binkDecoder->getFrameCount();
	};

	bool busy() { return _binkDecoder->isPlaying(); }

	uint32 drawFrame(uint32 surface_id = working_buffer_id);
	int getFrameNumber() {
		if (!busy())
			return 0;
		return _binkDecoder->getCurFrame();
	};

	void setRate() { warning("TODO: setRate"); }
	void setEndFrame(uint32 f) { warning("TODO: setEndFrame"); }
	void setVolume(int32 vol) { warning("TODO: setVolume"); }

	void kill() { _binkDecoder->close(); }

private:
	void fadeScreen(uint32 surface_id) {}
};

} // End of namespace ICB
#endif
