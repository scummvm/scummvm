/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/ambient.h"
#include "engines/myst3/database.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/state.h"

#include "audio/audiostream.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/wave.h"

namespace Myst3 {

Sound::Sound(Myst3Engine *vm) :
	_vm(vm) {
	for (uint i = 0; i < kNumChannels; i++)
		_channels[i] = new SoundChannel(_vm);
}

Sound::~Sound() {
	for (uint i = 0; i < kNumChannels; i++)
		delete _channels[i];
}

void Sound::playEffect(uint32 id, uint32 volume, uint16 heading, uint16 attenuation) {
	id = _vm->_state->valueOrVarValue(id);

	SoundChannel *channel = getChannelForSound(id, kEffect);
	channel->play(id, volume, heading, attenuation, false, kEffect);
}

void Sound::playCue(uint32 id, uint32 volume, uint16 heading, uint16 attenuation) {
	SoundChannel *channel = _channels[13];
	channel->play(id, volume, heading, attenuation, false, kCue);
}

void Sound::stopCue(uint32 fadeDelay) {
	SoundChannel *channel = _channels[13];
	channel->fadeOut(fadeDelay);
}

SoundChannel *Sound::getChannelForSound(uint32 id, SoundType type, bool *found) {
	// Channel number 13 is reserved for cue sounds

	// if the sound is already playing, return that channel
	for (uint i = 0; i < kNumChannels - 1; i++)
		if (_channels[i]->_id == id && _channels[i]->_type == type && _channels[i]->_playing) {
			if (found) *found = true;
			return _channels[i];
		}

	// else return the channel with the oldest sound
	SoundChannel *oldest = _channels[0];
	for (uint i = 0; i < kNumChannels - 1; i++) {
		if (_channels[i]->_age > oldest->_age) {
			oldest = _channels[i];
		}
	}

	if (found) *found = false;

	return oldest;
}

void Sound::update() {
	for (uint i = 0; i < kNumChannels; i++)
		_channels[i]->update();

	_vm->runBackgroundSoundScriptsFromNode(_vm->_state->getLocationNode());
	_vm->_ambient->updateCue();
}

void Sound::age() {
	for (uint i = 0; i < kNumChannels; i++)
		_channels[i]->age(99);
}

void Sound::fadeOutOldSounds(uint32 fadeDelay) {
	for (uint i = 0; i < kNumChannels; i++) {
		if (_channels[i]->_playing && _channels[i]->_type == kAmbient && _channels[i]->_age == 1) {
			uint32 delay = _channels[i]->_ambientFadeOutDelay;
			if (_vm->_state->getAmbientOverrideFadeOutDelay() || delay == 0)
				delay = fadeDelay;
			_channels[i]->fadeOut(delay);

		}
	}
	_vm->_state->setAmbientOverrideFadeOutDelay(false);
}

SoundChannel::SoundChannel(Myst3Engine *vm) :
	_vm(vm),
	_playing(false),
	_id(0),
	_stream(0),
	_age(0),
	_ambientFadeOutDelay(0) {
}

SoundChannel::~SoundChannel() {
}

void SoundChannel::play(uint32 id, uint32 volume, uint16 heading, uint16 attenuation, bool loop, SoundType type) {
	stop();

	// Load the name of the sound from its id
	_name = _vm->_db->getSoundName(id);
	_volume = volume;

	// Open the file to a stream
	_stream = Audio::makeLoopingAudioStream(makeAudioStream(_name), loop ? 0 : 1);

	if (!_stream)
		return;

	// Play the sound
	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_handle, _stream);
	setVolume3D(volume, heading, attenuation);

	// Update state
	_id = id;
	_type = type;
	_age = 0;
	_playing = true;
	_vm->_state->setVar(id, 1);
}

Audio::RewindableAudioStream *SoundChannel::makeAudioStream(const Common::String &name) const {
	Common::String folder = Common::String(name.c_str(), 4);
	Common::String filename = Common::String::format("M3Data/%s/%s", folder.c_str(), name.c_str());

	Common::SeekableReadStream *s = SearchMan.createReadStreamForMember(filename);

	bool isMP3 = false;

	if (!s)
		s = SearchMan.createReadStreamForMember(filename + ".wav");

	if (!s) {
		s = SearchMan.createReadStreamForMember(filename + ".mp3");
		isMP3 = true;
	}

	if (!s)
		error("Unable to open sound file '%s'", filename.c_str());

	if (isMP3) {
#ifdef USE_MAD
		return Audio::makeMP3Stream(s, DisposeAfterUse::YES);
#else
		warning("Unable to play sound '%s', MP3 support is not compiled in.", filename.c_str());
		delete s;
		return 0;
#endif
	} else {
		return Audio::makeWAVStream(s, DisposeAfterUse::YES);
	}
}

void SoundChannel::update() {
	if (!_playing)
		return; // Nothing to update

	_playing = g_system->getMixer()->isSoundHandleActive(_handle);

	if (!_playing) {
		_vm->_state->setVar(_id, 0);
		_stream = 0;
	}
}

void SoundChannel::stop() {
	if (!_playing)
		return; // Nothing to do

	_playing = g_system->getMixer()->isSoundHandleActive(_handle);

	if (_playing) {
		g_system->getMixer()->stopHandle(_handle);
		_playing = false;
	}

	_vm->_state->setVar(_id, 0);
	_id = 0;
	_age = 99;

	_stream = 0;
}

void SoundChannel::setVolume3D(uint32 volume, uint16 heading, uint16 attenuation) {
	uint16 mixerVolume = _volume * Audio::Mixer::kMaxChannelVolume / 100;

	// TODO: 3D sound
	g_system->getMixer()->setChannelVolume(_handle, mixerVolume);
}

void SoundChannel::fadeOut(uint32 fadeDelay) {
	if (fadeDelay == 0) {
		stop();
	} else {
		fade(0, -1, 0, fadeDelay);
	}
}

void SoundChannel::fade(uint32 targetVolume, int32 targetHeading, int32 targetAttenuation, uint32 fadeDelay) {
	//TODO: Implement
	if (targetVolume == 0) {
		stop();
	} else {
		setVolume3D(targetVolume, targetHeading, targetAttenuation);
	}
}

void SoundChannel::age(uint32 maxAge) {
	_age++;
	_age = CLIP<uint32>(_age, 0, maxAge);
}

} /* namespace Myst3 */
