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

#include "common/debug.h"
#include "common/system.h"

#include "audio/audiostream.h"

#include "mohawk/riven_sound.h"
#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/resource.h"
#include "mohawk/sound.h"

namespace Mohawk {

RivenSoundManager::RivenSoundManager(MohawkEngine_Riven *vm) :
		_vm(vm),
		_effect(nullptr),
		_mainAmbientSoundId(-1),
		_effectPlayOnDraw(false),
		_nextFadeUpdate(0) {

}

RivenSoundManager::~RivenSoundManager() {
	stopSound();
	stopAllSLST(false);
}

Audio::RewindableAudioStream *RivenSoundManager::makeAudioStream(uint16 id) {
	return makeMohawkWaveStream(_vm->getResource(ID_TWAV, id));
}

void RivenSoundManager::playSound(uint16 id, uint16 volume, bool playOnDraw) {
	debug (0, "Playing sound %d", id);

	stopSound();

	Audio::RewindableAudioStream *rewindStream = makeAudioStream(id);
	if (!rewindStream) {
		warning("Unable to play sound with id %d", id);
		return;
	}

	_effect = new RivenSound(_vm, rewindStream, Audio::Mixer::kSFXSoundType);
	_effect->setVolume(volume);

	_effectPlayOnDraw = playOnDraw;
	if (!playOnDraw) {
		_effect->play();
	}
}

void RivenSoundManager::playCardSound(const Common::String &name, uint16 volume, bool playOnDraw) {
	Common::String fullName = Common::String::format("%d_%s_1", _vm->getCard()->getId(), name.c_str());
	uint16 id =_vm->findResourceID(ID_TWAV, fullName);
	playSound(id, volume, playOnDraw);
}

void RivenSoundManager::playSLST(const SLSTRecord &slstRecord) {
	if (slstRecord.soundIds.empty()) {
		return;
	}

	if (slstRecord.soundIds[0] == _mainAmbientSoundId) {
		if (slstRecord.soundIds.size() > _ambientSounds.sounds.size()) {
			addAmbientSounds(slstRecord);
		}
		setAmbientLooping(slstRecord.loop);
		setTargetVolumes(slstRecord);
		_ambientSounds.suspend = slstRecord.suspend;
		if (slstRecord.suspend) {
			freePreviousAmbientSounds();
			pauseAmbientSounds();
			applyTargetVolumes();
		} else {
			playAmbientSounds();
		}
	} else {
		_mainAmbientSoundId = slstRecord.soundIds[0];
		freePreviousAmbientSounds();
		moveAmbientSoundsToPreviousSounds();
		addAmbientSounds(slstRecord);
		setAmbientLooping(slstRecord.loop);
		setTargetVolumes(slstRecord);
		_ambientSounds.suspend = slstRecord.suspend;
		if (slstRecord.suspend) {
			freePreviousAmbientSounds();
			applyTargetVolumes();
		} else {
			startFadingAmbientSounds(slstRecord.fadeFlags);
		}
	}
}

void RivenSoundManager::stopAllSLST(bool fade) {
	_mainAmbientSoundId = -1;
	freePreviousAmbientSounds();
	moveAmbientSoundsToPreviousSounds();
	startFadingAmbientSounds(fade ? kFadeOutPreviousSounds : 0);
}

void RivenSoundManager::stopSound() {
	if (_effect) {
		delete _effect;
	}
	_effect = nullptr;
	_effectPlayOnDraw = false;
}

void RivenSoundManager::addAmbientSounds(const SLSTRecord &record) {
	if (record.soundIds.size() > _ambientSounds.sounds.size()) {
		uint oldSize = _ambientSounds.sounds.size();

		// Resize the list to the new size
		_ambientSounds.sounds.resize(record.soundIds.size());

		// Add new elements to the list
		for (uint i = oldSize; i < _ambientSounds.sounds.size(); i++) {
			Audio::RewindableAudioStream *stream = makeAudioStream(record.soundIds[i]);

			RivenSound *sound = new RivenSound(_vm, stream, Audio::Mixer::kMusicSoundType);
			sound->setVolume(record.volumes[i]);
			sound->setBalance(record.balances[i]);

			_ambientSounds.sounds[i].sound = sound;
			_ambientSounds.sounds[i].targetVolume = record.volumes[i];
			_ambientSounds.sounds[i].targetBalance = record.balances[i];
		}
	}
}

void RivenSoundManager::setTargetVolumes(const SLSTRecord &record) {
	for (uint i = 0; i < MIN(_ambientSounds.sounds.size(), record.volumes.size()); i++) {
		_ambientSounds.sounds[i].targetVolume = record.volumes[i] * record.globalVolume / 256;
		_ambientSounds.sounds[i].targetBalance = record.balances[i];
	}
	_ambientSounds.fading = true;
}

void RivenSoundManager::freePreviousAmbientSounds() {
	for (uint i = 0; i < _previousAmbientSounds.sounds.size(); i++) {
		delete _previousAmbientSounds.sounds[i].sound;
	}
	_previousAmbientSounds = AmbientSoundList();
}

void RivenSoundManager::moveAmbientSoundsToPreviousSounds() {
	_previousAmbientSounds = _ambientSounds;
	_ambientSounds = AmbientSoundList();
}

void RivenSoundManager::applyTargetVolumes() {
	for (uint i = 0; i < _ambientSounds.sounds.size(); i++) {
		AmbientSound &ambientSound = _ambientSounds.sounds[i];
		RivenSound *sound = ambientSound.sound;
		sound->setVolume(ambientSound.targetVolume);
		sound->setBalance(ambientSound.targetBalance);
	}
	_ambientSounds.fading = false;
}

void RivenSoundManager::startFadingAmbientSounds(uint16 flags) {
	for (uint i = 0; i < _ambientSounds.sounds.size(); i++) {
		AmbientSound &ambientSound = _ambientSounds.sounds[i];
		uint16 volume;
		if (flags & kFadeInNewSounds) {
			volume = 0;
		} else {
			volume = ambientSound.targetVolume;
		}
		ambientSound.sound->setVolume(volume);
	}
	_ambientSounds.fading = true;
	playAmbientSounds();

	if (!_previousAmbientSounds.sounds.empty()) {
		if (flags) {
			_previousAmbientSounds.fading = true;
		} else {
			freePreviousAmbientSounds();
		}

		for (uint i = 0; i < _previousAmbientSounds.sounds.size(); i++) {
			AmbientSound &ambientSound = _previousAmbientSounds.sounds[i];
			if (flags & kFadeOutPreviousSounds) {
				ambientSound.targetVolume = 0;
			} else {
				ambientSound.sound->setVolume(ambientSound.targetVolume);
			}
		}
	}
}

void RivenSoundManager::playAmbientSounds() {
	for (uint i = 0; i < _ambientSounds.sounds.size(); i++) {
		_ambientSounds.sounds[i].sound->play();
	}
}

void RivenSoundManager::setAmbientLooping(bool loop) {
	for (uint i = 0; i < _ambientSounds.sounds.size(); i++) {
		_ambientSounds.sounds[i].sound->setLooping(loop);
	}
}

void RivenSoundManager::triggerDrawSound() {
	if (_effectPlayOnDraw && _effect) {
		_effect->play();
	}
	_effectPlayOnDraw = false;
}

void RivenSoundManager::pauseAmbientSounds() {
	for (uint i = 0; i < _ambientSounds.sounds.size(); i++) {
		_ambientSounds.sounds[i].sound->pause();
	}
}

void RivenSoundManager::updateSLST() {
	uint32 time = _vm->_system->getMillis();
	int32 delta = CLIP<int32>(time - _nextFadeUpdate, -50, 50);
	if (_nextFadeUpdate == 0 || delta > 0) {
		_nextFadeUpdate = time + 50 - delta;

		if (_ambientSounds.fading) {
			fadeAmbientSoundList(_ambientSounds);
		}

		if (_previousAmbientSounds.fading) {
			fadeAmbientSoundList(_previousAmbientSounds);
		}

		if (!_previousAmbientSounds.sounds.empty() && !_ambientSounds.fading && !_previousAmbientSounds.fading) {
			freePreviousAmbientSounds();
		}
	}
}

void RivenSoundManager::fadeAmbientSoundList(AmbientSoundList &list) {
	list.fading = false;

	for (uint i = 0; i < list.sounds.size(); i++) {
		AmbientSound &ambientSound = list.sounds[i];
		list.fading |= fadeVolume(ambientSound);
		list.fading |= fadeBalance(ambientSound);
	}
}

bool RivenSoundManager::fadeVolume(AmbientSound &ambientSound) {
	uint16 volume = ambientSound.sound->getVolume();
	float delta = (ambientSound.targetVolume - volume) / 30.0f;

	if (ABS<float>(delta) < 0.01f) {
		ambientSound.sound->setVolume(ambientSound.targetVolume);
		return false;
	} else {
		// Make sure the increment is not zero once converted to an integer
		if (delta > 0 && delta < 1) {
			delta = 1;
		} else if (delta < 0 && delta > -1) {
			delta = -1;
		}

		ambientSound.sound->setVolume(volume + delta);
		return true;
	}
}

bool RivenSoundManager::fadeBalance(RivenSoundManager::AmbientSound &ambientSound) {
	int16 balance = ambientSound.sound->getBalance();
	float delta = (ambientSound.targetBalance - balance) / 10.0f;

	if (ABS<float>(delta) < 0.01) {
		ambientSound.sound->setBalance(ambientSound.targetBalance);
		return false;
	} else {
		// Make sure the increment is not zero once converted to an integer
		if (delta > 0 && delta < 1) {
			delta = 1;
		} else if (delta < 0 && delta > -1) {
			delta = -1;
		}

		ambientSound.sound->setBalance(balance + delta);
		return true;
	}
}

bool RivenSoundManager::isEffectPlaying() const {
	return _effect != nullptr && _effect->isPlaying();
}

RivenSound::RivenSound(MohawkEngine_Riven *vm, Audio::RewindableAudioStream *rewindStream, Audio::Mixer::SoundType mixerType) :
		_vm(vm),
		_volume(Audio::Mixer::kMaxChannelVolume),
		_balance(0),
		_looping(false),
		_stream(rewindStream),
		_mixerType(mixerType) {

}

bool RivenSound::isPlaying() const {
	return _vm->_mixer->isSoundHandleActive(_handle);
}

void RivenSound::pause() {
	_vm->_mixer->pauseHandle(_handle, true);
}

void RivenSound::setVolume(uint16 volume) {
	_volume = volume;
	if (isPlaying()) {
		byte mixerVolume = convertVolume(volume);
		_vm->_mixer->setChannelVolume(_handle, mixerVolume);
	}
}

void RivenSound::setBalance(int16 balance) {
	_balance = balance;
	if (isPlaying()) {
		int8 mixerBalance = convertBalance(balance);
		_vm->_mixer->setChannelBalance(_handle, mixerBalance);
	}
}

void RivenSound::setLooping(bool loop) {
	if (isPlaying() && _looping != loop) {
		warning("Changing loop state while a sound is playing is not implemented.");
	}
	_looping = loop;
}

void RivenSound::play() {
	if (isPlaying()) {
		// If the sound is already playing, make sure it is not paused
		_vm->_mixer->pauseHandle(_handle, false);
		return;
	}

	if (!_stream) {
		warning("Trying to play a sound without a stream");
		return;
	}

	Audio::AudioStream *playStream;
	if (_looping) {
		playStream = new Audio::LoopingAudioStream(_stream, 0);
	} else {
		playStream = _stream;
	}

	int8 mixerBalance = convertBalance(_balance);
	byte mixerVolume = convertVolume(_volume);
	_vm->_mixer->playStream(_mixerType, &_handle, playStream, -1, mixerVolume, mixerBalance);
	_stream = nullptr;
}

byte RivenSound::convertVolume(uint16 volume) {
	// The volume is a fixed point value in the Mohawk part of the original engine.
	// It's not clear what happens when it is higher than one.
	return (volume > 255) ? 255 : volume;
}

int8 RivenSound::convertBalance(int16 balance) {
	return (int8)(balance >> 8);
}

RivenSound::~RivenSound() {
	_vm->_mixer->stopHandle(_handle);
	delete _stream;
}

int16 RivenSound::getBalance() const {
	return _balance;
}

uint16 RivenSound::getVolume() const {
	return _volume;
}

RivenSoundManager::AmbientSound::AmbientSound() :
		sound(nullptr),
		targetVolume(0),
		targetBalance(0) {

}

RivenSoundManager::AmbientSoundList::AmbientSoundList() :
		fading(false),
		suspend(false) {
}

} // End of namespace Mohawk
