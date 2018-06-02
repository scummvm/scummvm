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

#include "common/stream.h"
#include "common/substream.h"

namespace Pink {

/*TODO
  from disasm foreground 100 %, background 80 %
  dont know how to properly do it
  may be use ConfMan
*/

class Sound {
public:
	Sound(Audio::Mixer *mixer, Common::SafeSeekableSubReadStream *stream);
	~Sound();

	void play(Audio::Mixer::SoundType type, int volume, bool isLoop);

	bool isPlaying();

	void pause(bool paused);

	uint32 getCurrentSample();
	void setBalance(int8 balance);

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	Common::SafeSeekableSubReadStream *_fileStream;
};

} // End of namespace Pink

#endif
