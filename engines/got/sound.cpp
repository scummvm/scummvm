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

void Sound::music_play(const char *name, bool override) {
	if (name != _currentMusic || override) {
		g_engine->_mixer->stopHandle(_musicHandle);
		_currentMusic = name;

#ifdef TODO
		// FIXME: Completely wrong. Don't know music format yet
		// Open it up for access
		File file(name);

		Common::SeekableReadStream *f = file.readStream(file.size());
		Audio::AudioStream *audioStream = Audio::makeRawStream(
			f, 11025, 0, DisposeAfterUse::YES);
		g_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType,
			&_musicHandle, audioStream);
#else
		warning("TODO: play_music %s", name);
#endif
	}
}

void Sound::music_pause() {
	g_engine->_mixer->pauseHandle(_musicHandle, true);
}

void Sound::music_resume() {
	g_engine->_mixer->pauseHandle(_musicHandle, false);
}

bool Sound::music_is_on() const {
	return g_engine->_mixer->isSoundHandleActive(_musicHandle);
}

const char *Sound::getMusicName(int num) const {
	const char *name = nullptr;

	switch (_G(area)) {
	case 1:
		switch (num) {
		case 0:
			name = "SONG1";
			break;
		case 1:
			name = "SONG2";
			break;
		case 2:
			name = "SONG3";
			break;
		case 3:
			name = "SONG4";
			break;
		case 4:
			name = "WINSONG";
			break;
		case 5:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	case 2:
		switch (num) {
		case 0:
			name = "SONG21";
			break;
		case 1:
			name = "SONG22";
			break;
		case 2:
			name = "SONG23";
			break;
		case 3:
			name = "SONG24";
			break;
		case 4:
			name = "SONG35";
			break;
		case 5:
			name = "SONG25";
			break;
		case 6:
			name = "WINSONG";
			break;
		case 7:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	case 3:
		switch (num) {
		case 0:
			name = "SONG31";
			break;
		case 1:
			name = "SONG32";
			break;
		case 2:
			name = "SONG33";
			break;
		case 3:
			name = "SONG34";
			break;
		case 4:
			name = "SONG35";
			break;
		case 5:
			name = "SONG36";
			break;
		case 6:
			name = "WINSONG";
			break;
		case 7:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	if (!name)
		error("Invalid music");

	return name;
}

} // namespace Got
