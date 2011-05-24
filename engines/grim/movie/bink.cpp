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

#include "engines/grim/movie/bink.h"

#include "engines/grim/grim.h"
#include "engines/grim/colormap.h"

#define MWIDTH 640
#define MHEIGHT 400

namespace Grim {

MoviePlayer *CreateBinkPlayer() {
	return new BinkPlayer();
}

void BinkPlayer::timerCallback(void *) {
	((BinkPlayer *)g_movie)->handleFrame();
}

BinkPlayer::BinkPlayer() : MoviePlayer() {
	g_movie = this;
	_speed = 50;
}

BinkPlayer::~BinkPlayer() {
	deinit();
}

void BinkPlayer::init() {
	_frame = 0;
	_movieTime = 0;
	_updateNeeded = false;
	_width = MWIDTH;
	_height = MHEIGHT;

	assert(!_externalBuffer);

	_externalBuffer = new byte[_width * _height * 2];

	warning("Trying to play %s",_fname.c_str());
	//_videoBase->init(_fname.c_str());
	g_system->getTimerManager()->installTimerProc(&timerCallback, _speed, NULL);
}

void BinkPlayer::deinit() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);

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
	if (_videoPause)
		return;

	if (_videoFinished) {
		_videoPause = true;
		return;
	}

	return;
}

void BinkPlayer::stop() {
	deinit();
	g_grim->setMode(ENGINE_MODE_NORMAL);
}

bool BinkPlayer::play(const char *filename, bool looping, int x, int y) {
	deinit();
	_fname = filename;

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
