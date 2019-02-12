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

#include "engines/myst3/ambient.h"
#include "engines/myst3/database.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/state.h"

#include "audio/audiostream.h"
#include "audio/decoders/asf.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/wave.h"

#include "common/archive.h"
#include "common/config-manager.h"

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

void Sound::playEffectLooping(uint32 id, uint32 volume, uint16 heading, uint16 attenuation) {
	id = _vm->_state->valueOrVarValue(id);

	bool alreadyPlaying;
	SoundChannel *channel = getChannelForSound(id, kEffect, &alreadyPlaying);

	if (!alreadyPlaying) {
		channel->play(id, volume, heading, attenuation, true, kEffect);
	}
}

void Sound::playEffectFadeInOut(uint32 id, uint32 volume, uint16 heading, uint16 attenuation,
		uint32 fadeInDuration, uint32 playDuration, uint32 fadeOutDuration) {

	SoundChannel *channel = getChannelForSound(id, kEffect);
	channel->play(id, fadeInDuration == 0 ? volume : 0, heading, attenuation, true, kEffect);

	uint32 effectiveVolume = channel->adjustVolume(volume);
	if (channel->_playing) {
		channel->_fadeArrayPosition = 0;
		channel->_fadeDurations[0] = 0;
		channel->_fadeDurations[1] = 0;
		channel->_fadeDurations[2] = playDuration;
		channel->_fadeDurations[3] = fadeOutDuration;
		channel->_fadeVolumes[0] = 0;
		channel->_fadeVolumes[1] = effectiveVolume;
		channel->_fadeVolumes[2] = effectiveVolume;
		channel->_fadeVolumes[3] = 0;
		channel->fade(effectiveVolume, heading, attenuation, fadeInDuration);
		channel->_hasFadeArray = true;
	}
}

void Sound::stopEffect(uint32 id, uint32 fadeDuration) {
	bool found;
	SoundChannel *channel = getChannelForSound(id, kEffect, &found);

	if (found) {
		channel->fadeOut(fadeDuration);
	}
}

void Sound::stopMusic(uint32 fadeDelay) {
	for (uint i = 0; i < kNumChannels; i++) {
		SoundChannel *channel = _channels[i];
		if (channel->_type == kMusic && channel->_playing)
			channel->fadeOut(fadeDelay);
	}
}

void Sound::resetSoundVars() {
	uint32 minId = _vm->_db->getSoundIdMin();
	uint32 maxId = _vm->_db->getSoundIdMax();

	if (minId == 0 || maxId == 0) {
		return;
	}

	for (uint32 id = minId; id <= maxId; id++) {
		_vm->_state->setVar(id, 0);
	}
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
		if (_channels[i]->_id == id && (_channels[i]->_type == type || type == kAny ) && _channels[i]->_playing) {
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

void Sound::compute3DVolumes(int32 heading, uint angle, int32 *left, int32 *right) {
	// This table contains the left and right volume values for the cardinal directions
	static const struct {
		int32 angle;
		int32 left;
		int32 right;
	} volumes[] = {
		{ -180, 50,   50 },
		{  -90, 100,   0 },
		{    0, 100, 100 },
		{   90, 0,   100 },
		{  180, 50,   50 }
	};

#if 0
	// This is the equivalent volume table for the xbox version, with 4.0 surround
	static const struct {
		int32 angle;
		int32 frontLeft;
		int32 frontRight;
		int32 backLeft;
		int32 backRight;
	} surroundVolumes[] = {
		{ -180,   0,   0, 100, 100 },
		{ -135,   0,   0, 100,   0 },
		{  -90, 100,   0, 100,   0 },
		{  -40, 100,   0,   0,   0 },
		{    0, 100, 100,   0,   0 },
		{   40,   0, 100,   0,   0 },
		{   90,   0, 100,   0, 100 },
		{  135,   0,   0,   0, 100 },
		{  180,   0,   0, 100, 100 },
	};
#endif

	if (angle) {
		// Compute the distance to the sound source
		int32 headingDistance = heading - _vm->_state->getLookAtHeading();

		// Make sure to use the shortest direction
		while (ABS(headingDistance) > 180) {
			if (headingDistance > 0) {
				headingDistance -= 360;
			} else {
				headingDistance += 360;
			}
		}

		// Find the appropriate quadrant
		uint quadrant = 0;
		while (headingDistance < volumes[quadrant].angle || headingDistance > volumes[quadrant + 1].angle)
			quadrant++;

		float positionInQuadrant = (headingDistance - volumes[quadrant].angle)
				/ (float)(volumes[quadrant + 1].angle - volumes[quadrant].angle);

		// Compute the left and right volumes using linear interpolation from the cardinal directions
		*left = volumes[quadrant].left + (volumes[quadrant + 1].left - volumes[quadrant].left) * positionInQuadrant;
		*right = volumes[quadrant].right + (volumes[quadrant + 1].right - volumes[quadrant].right) * positionInQuadrant;

		// Add the base sound level
		*left += (100 - angle) * (100 - *left) / 100;
		*right += (100 - angle) * (100 - *right) / 100;
	} else {
		*left = 100;
		*right = 100;
	}
}

void Sound::computeVolumeBalance(int32 volume, int32 heading, uint attenuation, int32 *mixerVolume, int32 *balance) {
	int32 left, right;
	_vm->_sound->compute3DVolumes(heading, attenuation, &left, &right);

	*mixerVolume = MAX(left, right) * volume * Audio::Mixer::kMaxChannelVolume / 100 / 100;

	// Compute balance from the left and right volumes
	if (left == right) {
		*balance = 0;
	} else if (left > right) {
		*balance = -127 * (left - right) / left;
	} else {
		*balance = 127 * (right - left) / right;
	}
}

int32 Sound::playedFrames(uint32 id) {
	bool soundPlaying;
	SoundChannel *channel = getChannelForSound(id, kAny, &soundPlaying);

	if (!soundPlaying) {
		return -1;
	}

	return channel->playedFrames();
}

bool Sound::isPlaying(uint32 id) {
	bool soundPlaying;
	getChannelForSound(id, kAny, &soundPlaying);
	return soundPlaying;
}

void Sound::setupNextSound(SoundNextCommand command, int16 controlVar, int16 startSoundId, int16 soundCount,
		int32 soundMinDelay, int32 soundMaxDelay, int32 controlSoundId, int32 controlSoundMaxPosition) {

	bool playSeveralSounds = _vm->_state->getSoundNextMultipleSounds();

	_vm->_state->setSoundNextMultipleSounds(false);
	_vm->_state->setSoundNextIsChoosen(false);
	_vm->_state->setSoundNextId(0);
	_vm->_state->setSoundNextIsLast(false);

	uint32 controlLastTick = _vm->_state->getVar(controlVar);
	int32 playingSoundId = _vm->_state->getVar(controlVar + 1) >> 16;
	int32 soundDelay = _vm->_state->getVar(controlVar + 1) & 0xFFFF;

	if (!controlLastTick) {
		if (!playSeveralSounds) {
			for (int16 i = startSoundId; i < startSoundId + soundCount; i++) {
				int16 soundVarValue = _vm->_state->getVar(i);
				if (soundVarValue)
					return;
			}
		}

		soundDelay = _vm->_rnd->getRandomNumberRng(soundMinDelay, soundMaxDelay);

		_vm->_state->setVar(controlVar, 1);
		_vm->_state->setVar(controlVar + 1, soundDelay | (playingSoundId << 16));
		return;
	}

	uint currentTick = _vm->_state->getTickCount();
	if (currentTick == controlLastTick) {
		return;
	}

	if (currentTick < controlLastTick) {
		soundDelay = 0;
	} else if (currentTick > controlLastTick + 10) {
		soundDelay -= 10;
	} else {
		soundDelay -= currentTick - controlLastTick;
	}

	if (soundDelay < 0) {
		soundDelay = 0;
	}

	if (soundDelay) {
		_vm->_state->setVar(controlVar, currentTick);
		_vm->_state->setVar(controlVar + 1, soundDelay | (playingSoundId << 16));
		return;
	}

	bool shouldPlaySound;
	if (command == kRandom || command == kNext) {
		shouldPlaySound = true;
	} else {
		int32 controlSoundPosition = playedFrames(controlSoundId);

		shouldPlaySound = controlSoundPosition >= 0 && controlSoundPosition <= controlSoundMaxPosition;
	}

	if (!shouldPlaySound) {
		return;
	}

	switch (command) {
	case kRandom:
	case kRandomIfOtherStarting: {
		if (soundCount == 1) {
			playingSoundId = startSoundId;
		} else {
			int32 newSoundId;
			do {
				newSoundId = _vm->_rnd->getRandomNumberRng(startSoundId, startSoundId + soundCount - 1);
			} while (newSoundId == playingSoundId);
			playingSoundId = newSoundId;
		}
	}
	break;
	case kNext:
	case kNextIfOtherStarting: {
		if (!playingSoundId) {
			playingSoundId = startSoundId;
		} else {
			playingSoundId++;
		}

		if (playingSoundId == startSoundId + soundCount - 1) {
			_vm->_state->setSoundNextIsLast(true);
		}

		if (playingSoundId >= startSoundId + soundCount)
			playingSoundId = startSoundId;
	}
	break;
	}

	_vm->_state->setVar(controlVar, 0);
	_vm->_state->setVar(controlVar + 1, soundDelay | (playingSoundId << 16));

	_vm->_state->setVar(playingSoundId, 2);

	_vm->_state->setSoundNextIsChoosen(true);
	_vm->_state->setSoundNextId(playingSoundId);
}

SoundChannel::SoundChannel(Myst3Engine *vm) :
		_vm(vm),
		_playing(false),
		_fading(false),
		_id(0),
		_stream(0),
		_age(0),
		_ambientFadeOutDelay(0),
		_volume(0),
		_heading(0),
		_headingAngle(0),
		_fadeLastTick(0),
		_fadeDuration(0),
		_fadeTargetVolume(0),
		_fadeSourceVolume(0),
		_fadeTargetAttenuation(0),
		_fadeSourceAttenuation(0),
		_fadeTargetHeading(0),
		_fadeSourceHeading(0),
		_stopWhenSilent(true),
		_hasFadeArray(false),
		_fadeArrayPosition(0),
		_fadePosition(0),
		_type(kAny) {
}

SoundChannel::~SoundChannel() {
}

void SoundChannel::play(uint32 id, uint32 volume, uint16 heading, uint16 attenuation, bool loop, SoundType type) {
	stop();

	// Load the name of the sound from its id
	_name = _vm->_db->getSoundName(id);

	// Set the sound type
	if (_vm->_state->getVar(id) != 2) {
		_type = type;
	} else {
		_type = kMusic;
	}

	// Set the sound parameters
	_volume = adjustVolume(volume);
	_heading = heading;
	_headingAngle = attenuation;

	// Open the file to a stream
	Audio::RewindableAudioStream *plainStream = makeAudioStream(_name);

	if (!plainStream)
		return;

	// Get the sound's length
	Audio::SeekableAudioStream *seekableStream = dynamic_cast<Audio::SeekableAudioStream *>(plainStream);
	if (seekableStream) {
		_length = seekableStream->getLength();
	}

	_stream = Audio::makeLoopingAudioStream(plainStream, loop ? 0 : 1);

	// Play the sound
	g_system->getMixer()->playStream(mixerSoundType(), &_handle, _stream);
	setVolume3D(volume, heading, attenuation);

	// Update state
	_id = id;
	_age = 0;
	_playing = true;
	_stopWhenSilent = false;
	_vm->_state->setVar(id, 1);
}

Audio::Mixer::SoundType SoundChannel::mixerSoundType() {
	switch (_type) {
	case kCue:
	case kEffect:
		return Audio::Mixer::kSFXSoundType;
	case kAmbient:
	case kMusic:
		return  Audio::Mixer::kMusicSoundType;
	default:
		error("Impossible");
	}
}

uint32 SoundChannel::adjustVolume(uint32 volume) {
	if (_type == kMusic)
		return volume * 100 / 75;
	else
		return volume;
}

Audio::RewindableAudioStream *SoundChannel::makeAudioStream(const Common::String &name) const {
	Common::String folder = Common::String(name.c_str(), 4);
	Common::String filename = Common::String::format("M3Data/%s/%s", folder.c_str(), name.c_str());

	Common::SeekableReadStream *s = SearchMan.createReadStreamForMember(filename);

	bool isMP3 = false;
	bool isWMA = false;

	if (!s)
		s = SearchMan.createReadStreamForMember(filename + ".wav");

	if (!s) {
		s = SearchMan.createReadStreamForMember(filename + ".mp3");
		if (s) isMP3 = true;
	}

	if (!s) {
		s = SearchMan.createReadStreamForMember(filename + ".wma");
		if (s) isWMA = true;
	}

	if (!s)
		error("Unable to open sound file '%s'", filename.c_str());

	if (isMP3) {
#ifdef USE_MAD
		return Audio::makeMP3Stream(s, DisposeAfterUse::YES);
#else
		warning("Unable to play sound '%s', MP3 support is not compiled in.", filename.c_str());
		delete s;
		return NULL;
#endif
	} else if (isWMA) {
		return Audio::makeASFStream(s, DisposeAfterUse::YES);
	} else {
		return Audio::makeWAVStream(s, DisposeAfterUse::YES);
	}
}

void SoundChannel::update() {
	if (!_playing)
		return; // Nothing to update

	if (!_fading)
		setVolume3D(_volume, _heading, _headingAngle);
	else
		updateFading();

	_playing = g_system->getMixer()->isSoundHandleActive(_handle);

	if (!_playing || (_stopWhenSilent && !_volume)) {
		stop();
	}

	if (!_playing)
		return;
}

void SoundChannel::stop() {
	_playing = g_system->getMixer()->isSoundHandleActive(_handle);

	if (_playing) {
		g_system->getMixer()->stopHandle(_handle);
		_playing = false;
	}

	if (_id != 0) {
		_vm->_state->setVar(_id, 0);
		_id = 0;
	}

	_age = 99;
	_fading = false;
	_stopWhenSilent = true;
	_hasFadeArray = false;

	_stream = 0;
	_length = Audio::Timestamp();
}

void SoundChannel::setVolume3D(uint32 volume, uint16 heading, uint16 attenuation) {
	int32 mixerVolume, balance;
	_vm->_sound->computeVolumeBalance(volume, heading, attenuation, &mixerVolume, &balance);

	g_system->getMixer()->setChannelVolume(_handle, mixerVolume);
	g_system->getMixer()->setChannelBalance(_handle, balance);
}

void SoundChannel::fadeOut(uint32 fadeDelay) {
	if (fadeDelay == 0) {
		stop();
	} else {
		fade(0, -1, 0, fadeDelay);
	}
}

void SoundChannel::fade(uint32 targetVolume, int32 targetHeading, int32 targetAttenuation, uint32 fadeDelay) {
	_fading = true;
	_hasFadeArray = false;
	_fadeDuration = fadeDelay;
	_fadePosition = 0;
	_fadeLastTick = 0;

	_fadeSourceVolume = _volume;
	_fadeTargetVolume = targetVolume;
	if (!targetVolume)
		_stopWhenSilent = true;

	if (targetHeading < 0) {
		_fadeSourceHeading = _heading;
		_fadeTargetHeading = _heading;
		_fadeSourceAttenuation = _headingAngle;
		_fadeTargetAttenuation = _headingAngle;
	} else {
		_fadeSourceAttenuation = _headingAngle;
		_fadeTargetAttenuation = targetAttenuation;

		_fadeSourceHeading = _heading;
		_fadeTargetHeading = targetHeading;

		_fadeSourceHeading -= 360;

		while (ABS(targetHeading - _fadeSourceHeading) > 180) {
			_fadeSourceHeading += 360;
		}
	}
}

void SoundChannel::age(uint32 maxAge) {
	_age++;
	_age = CLIP<uint32>(_age, 0, maxAge);
}

void SoundChannel::updateFading() {
	uint tick = _vm->_state->getTickCount();
	if (tick == _fadeLastTick) {
		return; // We already updated fading this tick
	}

	_fadeLastTick = tick;
	_fadePosition++;

	if (_fadePosition <= _fadeDuration) {
		// Fading in progress, compute the new channel parameters
		_volume = _fadeSourceVolume + _fadePosition * (_fadeTargetVolume - _fadeSourceVolume) / _fadeDuration;
		_heading = _fadeSourceHeading + _fadePosition * (_fadeTargetHeading - _fadeSourceHeading) / _fadeDuration;
		_headingAngle = _fadeSourceAttenuation + _fadePosition * (_fadeTargetAttenuation - _fadeSourceAttenuation) / _fadeDuration;
	} else {
		if (!_hasFadeArray) {
			// The fading is complete
			_fading = false;
		} else {
			// This step of the fade array is complete, find the next one
			do {
				_fadeArrayPosition++;
			} while (_fadeArrayPosition < 4 && !_fadeDurations[_fadeArrayPosition]);

			if (_fadeArrayPosition < 4) {
				// Setup the new fading step
				_fadePosition = 0;
				_fadeDuration = _fadeDurations[_fadeArrayPosition];

				_fadeSourceVolume = _volume;
				_fadeTargetVolume = _fadeVolumes[_fadeArrayPosition];

				if (!_fadeTargetVolume) {
					_stopWhenSilent = true;
				}
			} else {
				// No more steps
				_hasFadeArray = false;
				_fading = false;
				_stopWhenSilent = true;
				_volume = 0;
			}
		}
	}
	setVolume3D(_volume, _heading, _headingAngle);
}

uint32 SoundChannel::playedFrames() {
	uint32 length = _length.msecs();
	if (!length) {
		warning("Unable to retrieve length for sound %d", _id);
		return 0;
	}

	uint32 elapsed = g_system->getMixer()->getSoundElapsedTime(_handle);

	// Don't count completed loops in
	while (elapsed > length) {
		elapsed -= length;
	}

	return elapsed * 30 / 1000;
}

} // End of namespace Myst3
