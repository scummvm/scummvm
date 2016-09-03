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

#include "bbvs/sound.h"
#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"

namespace Bbvs {

Sound::Sound() : _stream(0) {
}

Sound::~Sound() {
	stop();
	delete _stream;
}

void Sound::load(const Common::String &filename) {
	Common::File *fd = new Common::File();
	if (!fd->open(filename)) {
		delete fd;
		error("SoundMan::loadSound() Could not load %s", filename.c_str());
	}
	_stream = Audio::makeAIFFStream(fd, DisposeAfterUse::YES);
	_filename = filename;
}

void Sound::play(bool loop) {
	debug(0, "Sound::play() [%s] loop:%d", _filename.c_str(), loop);
	stop();
	_stream->rewind();
	Audio::AudioStream *audioStream = Audio::makeLoopingAudioStream(_stream, loop ? 0 : 1);
	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_handle, audioStream,
		-1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

void Sound::stop() {
	g_system->getMixer()->stopHandle(_handle);
}

bool Sound::isPlaying() {
	return g_system->getMixer()->isSoundHandleActive(_handle);
}

SoundMan::~SoundMan() {
	stopAllSounds();
	unloadSounds();
}

void SoundMan::loadSound(const Common::String &filename) {
	Sound *sound = new Sound();
	sound->load(filename);
	_sounds.push_back(sound);
}

void SoundMan::playSound(uint index, bool loop) {
	_sounds[index]->play(loop);
}

void SoundMan::stopSound(uint index) {
	_sounds[index]->stop();
}

bool SoundMan::isSoundPlaying(uint index) {
	return _sounds[index]->isPlaying();
}

bool SoundMan::isAnySoundPlaying(uint *indices, uint count) {
	for (uint i = 0; i < count; ++i)
		if (isSoundPlaying(indices[i]))
			return true;
	return false;
}

void SoundMan::unloadSounds() {
	for (uint i = 0; i < _sounds.size(); ++i)
		delete _sounds[i];
	_sounds.clear();
}

void SoundMan::stopAllSounds() {
	for (uint i = 0; i < _sounds.size(); ++i)
		_sounds[i]->stop();
}

} // End of namespace Bbvs
