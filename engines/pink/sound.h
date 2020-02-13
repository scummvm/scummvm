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

#ifndef PINK_SOUND_H
#define PINK_SOUND_H

#include "audio/mixer.h"
#include "audio/timestamp.h"

#include "common/system.h"

#include "pink/constants.h"

namespace Common {
	class SafeSeekableSubReadStream;
}

namespace Pink {

class Sound {
public:
	~Sound() { stop(); }

	void play(Common::SafeSeekableSubReadStream *stream, Audio::Mixer::SoundType type, byte volume = 100, int8 balance = 0, bool isLoop = false);

	bool isPlaying() const { return g_system->getMixer()->isSoundHandleActive(_handle); }

	void stop() { g_system->getMixer()->stopHandle(_handle); }

	void pause(bool paused) { g_system->getMixer()->pauseHandle(_handle, paused); }

	uint64 getCurrentSample() const { return (uint64)g_system->getMixer()->getElapsedTime(_handle).msecs() * kSampleRate / 1000; }

private:
	Audio::SoundHandle _handle;
};

} // End of namespace Pink

#endif
