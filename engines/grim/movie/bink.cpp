/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_getwd
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink

#include "common/endian.h"
#include "common/timer.h"
#include "common/file.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "graphics/surface.h"
#include "video/bink_decoder.h"

#include "engines/grim/movie/bink.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"

#ifdef USE_BINK

namespace Grim {

MoviePlayer *CreateBinkPlayer() {
	return new BinkPlayer();
}

void BinkPlayer::timerCallback(void *) {
	((BinkPlayer *)g_movie)->handleFrame();
}

BinkPlayer::BinkPlayer() : MoviePlayer() {
	g_movie = this;
	_binkDecoder = new Video::BinkDecoder();
	_surface = new Graphics::Surface();
	_externalSurface = new Graphics::Surface();
	_speed = 1000;
}

BinkPlayer::~BinkPlayer() {
	deinit();
}

void BinkPlayer::init() {
	_frame = 0;
	_movieTime = 0;
	_updateNeeded = false;
	_videoFinished = false;
	_width = 0;
	_height = 0;
	_x = 0;
	_y = 0;

	assert(!_externalBuffer);
	
	g_system->getTimerManager()->installTimerProc(&timerCallback, _speed, NULL);
}

void BinkPlayer::deinit() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);
	_binkDecoder->close();
	_surface->free();
	
	if (_externalBuffer) {
		delete[] _externalBuffer;
		_externalBuffer = NULL;
	}

	if (_stream) {
		_stream->finish();
		_stream = NULL;
		g_system->getMixer()->stopHandle(_soundHandle);
	}
	_videoPause = false;
}

void BinkPlayer::handleFrame() {
	if (_binkDecoder->endOfVideo())
		_videoFinished = true;

	if (_videoPause)
		return;

	if (_videoFinished) {
		g_grim->setMode(ENGINE_MODE_NORMAL);
		_videoPause = true;
		return;
	}

	if (_binkDecoder->getTimeToNextFrame() > 0)
		return;

	_surface->copyFrom(*_binkDecoder->decodeNextFrame());

	_width = _surface->w;
	_height = _surface->h;
	
	_internalBuffer = (byte *)_surface->pixels;

	// Avoid updating the _externalBuffer if it's flagged as updateNeeded
	// since the draw-loop might access it then. This way, any late frames
	// will be dropped, and the sound will continue, in synch.
	if (!_updateNeeded) {
		_externalSurface->copyFrom(*_surface);
		_externalBuffer = (byte *)_externalSurface->pixels; 
		_updateNeeded = true;
	}
		
	_movieTime = _binkDecoder->getElapsedTime();
	_frame = _binkDecoder->getCurFrame();

	return;
}

void BinkPlayer::stop() {
	deinit();
	g_grim->setMode(ENGINE_MODE_NORMAL);
}

bool BinkPlayer::play(const char *filename, bool looping, int x, int y) {
	deinit();
	_x = x;
	_y = y;
	_fname = filename;

	// The demo uses a weird .lab suffix instead of the normal .bik
	_fname += (g_grim->getGameFlags() & ADGF_DEMO) ? ".lab" : ".bik";

	if (!_binkDecoder->loadFile(_fname))
		return false;

	if (gDebugLevel == DEBUG_SMUSH)
		printf("Playing video '%s'.\n", filename);

	init();

	return true;
}

void BinkPlayer::saveState(SaveGame *state) {
}

void BinkPlayer::restoreState(SaveGame *state) {
}

} // end of namespace Grim

#endif // USE_BINK
