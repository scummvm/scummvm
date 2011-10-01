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

BinkPlayer::BinkPlayer() : MoviePlayer() {
	g_movie = this;
	_videoDecoder = new Video::BinkDecoder();
	_speed = 1000;
}

BinkPlayer::~BinkPlayer() {
	deinit();
	delete _videoDecoder;
}

void BinkPlayer::handleFrame() {
	basicHandleFrame();
}

bool BinkPlayer::play(const char *filename, bool looping, int x, int y) {
	_fname = filename;
	// The demo uses a weird .lab suffix instead of the normal .bik
	_fname += (g_grim->getGameFlags() & ADGF_DEMO) ? ".lab" : ".bik";

	return MoviePlayer::play(_fname.c_str(), looping, x, y);
}

void BinkPlayer::saveState(SaveGame *state) {
}

void BinkPlayer::restoreState(SaveGame *state) {
}

} // end of namespace Grim

#endif // USE_BINK
