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

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "common/system.h"

#include "chewy/resource.h"
#include "chewy/sound.h"

namespace Chewy {

Sound::Sound() {
	_speechRes = new Resource("speech.tvp");
	_soundRes = new Resource("details.tap");
}

Sound::~Sound() {
	delete _soundRes;
	delete _speechRes;
}

void Sound::playSound(int num, bool loop) {
	Chunk *chunk = _soundRes->getChunk(num);
	byte *data = _soundRes->getChunkData(num);

	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
		Audio::makeRawStream(data,
		chunk->size, 22050, Audio::FLAG_UNSIGNED,
		DisposeAfterUse::NO),
		loop ? 0 : 1);

	g_engine->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream);
}

void Sound::playMusic(int num, bool loop) {
	uint32 musicNum = _soundRes->getChunkCount() - 1 - num;
	Chunk *chunk = _soundRes->getChunk(musicNum);
	byte *data = _soundRes->getChunkData(musicNum);

	// TODO: TMF music files are similar to MOD files. With the following
	// incorrect implementation, the PCM parts of these files can be played
	warning("The current music playing implementation is wrong");

	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
		Audio::makeRawStream(data,
		chunk->size, 22050, Audio::FLAG_UNSIGNED,
		DisposeAfterUse::NO),
		loop ? 0 : 1);

	g_engine->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, stream);
}

void Sound::playSpeech(int num) {
	Chunk *chunk = _speechRes->getChunk(num);
	byte *data = _speechRes->getChunkData(num);

	Audio::AudioStream *stream = Audio::makeLoopingAudioStream(
		Audio::makeRawStream(data,
		chunk->size, 22050, Audio::FLAG_UNSIGNED,
		DisposeAfterUse::NO),
		1);

	g_engine->_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream);
}

} // End of namespace Chewy
