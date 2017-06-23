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

#ifndef BBVS_SOUND_H
#define BBVS_SOUND_H

#include "audio/mixer.h"
#include "common/array.h"

namespace Audio {
class RewindableAudioStream;
}

namespace Bbvs {

class Sound {
public:
	Sound();
	~Sound();
	void load(const Common::String &filename);
	void play(bool loop);
	void stop();
	bool isPlaying();
protected:
	Audio::RewindableAudioStream *_stream;
	Audio::SoundHandle _handle;
	// Keep the filename for debugging purposes
	Common::String _filename;
};

class SoundMan {
public:
	~SoundMan();
	void loadSound(const Common::String &fileName);
	void playSound(uint index, bool loop = false);
	void stopSound(uint index);
	bool isSoundPlaying(uint index);
	bool isAnySoundPlaying(uint *indices, uint count);
	void unloadSounds();
	void stopAllSounds();
protected:
	Common::Array<Sound*> _sounds;
};

} // End of namespace Bbvs

#endif // BBVS_SOUND_H
