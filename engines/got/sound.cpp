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

#include "common/memstream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "got/sound.h"
#include "got/got.h"
#include "got/utils/file.h"

namespace Got {

static const byte SOUND_PRIORITY[] = { 1,2,3,3,3,1,4,4,4,5,4,3,1,2,2,5,1,3,1 };

void Sound::load() {
	File f("DIGSOUND");

	// Load index
	for (int i = 0; i < 16; ++i)
		_digiSounds[i].load(&f);

	// Allocate memory and load sound data
	_soundData = new byte[f.size() - 16 * 8];
	f.read(_soundData, f.size() - 16 * 8);
}

void Sound::play_sound(int index, bool priority_override) {
	if (index >= NUM_SOUNDS)
		return;

	// If a sound is playing, stop it unless there is a priority override
	if (sound_playing()) {
		if (!priority_override && _currentPriority < SOUND_PRIORITY[index])
			return;

		g_engine->_mixer->stopHandle(_soundHandle);
	}

	// Play the new sound
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(
		_soundData + _digiSounds[index].offset, _digiSounds[index].length);
	Audio::AudioStream *audioStream = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED);
	g_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType,
		&_soundHandle, audioStream);
}

bool Sound::sound_playing() const {
	return g_engine->_mixer->isSoundHandleActive(_soundHandle);
}

} // namespace Got
