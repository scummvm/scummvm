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
#include "audio/decoders/wave.h"

#include "common/substream.h"
#include "common/system.h"
#include "common/debug.h"

#include "petka/petka.h"
#include "petka/sound.h"

namespace Petka {

Sound::Sound(Common::SeekableReadStream *stream, Audio::Mixer::SoundType type)
	: _type(type), _stream(stream->readStream(stream->size())) {
	delete stream;
}

Sound::~Sound() {
	stop();
}

void Sound::play(bool isLoop) {
	if (!_stream)
		return;
	stop();

	Audio::AudioStream *audioStream;
	Audio::SeekableAudioStream *wavStream = Audio::makeWAVStream(_stream.get(), DisposeAfterUse::NO);
	if (isLoop)
		audioStream = Audio::makeLoopingAudioStream(wavStream, 0, 0, 0);
	else
		audioStream = wavStream;

	g_system->getMixer()->playStream(_type, &_handle , audioStream);
}

bool Sound::isPlaying() {
	return g_system->getMixer()->isSoundHandleActive(_handle);
}

void Sound::stop() {
	g_system->getMixer()->stopHandle(_handle);
	_stream->seek(0, SEEK_SET);
}

void Sound::setBalance(uint16 x, uint16 width) {
	// original scales from -12.5 db to 12.5 db
	g_system->getMixer()->setChannelBalance(_handle, (int8)(255 * (2 * x - width) / (2 * width)));
}

Audio::Mixer::SoundType Sound::type() {
	return _type;
}

void Sound::pause(bool p) {
	g_system->getMixer()->pauseHandle(_handle, p);
}


Sound *SoundMgr::addSound(const Common::String &name, Audio::Mixer::SoundType type) {
	Sound *sound = findSound(name);
	if (sound)
		return sound;
	Common::SeekableReadStream *s = _vm.openFile(name, false);
	if (s) {
		debug("SoundMgr: added sound %s", name.c_str());
		sound = new Sound(s, type);
		_sounds.getVal(name).reset(sound);
	}
	return sound;
}

Sound *SoundMgr::findSound(const Common::String &name) const {
	SoundsMap::iterator it = _sounds.find(name);
	return it != _sounds.end() ? it->_value.get() : nullptr;
}

void SoundMgr::removeSound(const Common::String &name) {
	debug("SoundMgr::removeSound %s", name.c_str());
 	_sounds.erase(name);
}

void SoundMgr::removeAll() {
	debug("SoundMgr::removeAll");
	_sounds.clear(false);
}

void SoundMgr::removeSoundsWithType(Audio::Mixer::SoundType type) {
	SoundsMap::iterator it;
	for (it = _sounds.begin(); it != _sounds.end(); ++it) {
		Sound *s = it->_value.get();
		if (s->type() == type) {
			_sounds.erase(it); // it is safe to inc iterator after erasing in our hashmap impl
		}
	}
}

} // End of namespace Petka
