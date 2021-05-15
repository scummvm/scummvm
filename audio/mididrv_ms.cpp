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

#include "common/config-manager.h"

#include "audio/mididrv_ms.h"

const uint8 MidiDriver_Multisource::MAXIMUM_SOURCES;
const uint16 MidiDriver_Multisource::DEFAULT_SOURCE_NEUTRAL_VOLUME;
const uint16 MidiDriver_Multisource::FADING_DELAY;

MidiDriver_Multisource::MidiSource::MidiSource() :
	type(SOURCE_TYPE_UNDEFINED),
	volume(DEFAULT_SOURCE_NEUTRAL_VOLUME),
	neutralVolume(DEFAULT_SOURCE_NEUTRAL_VOLUME),
	fadeStartVolume(0),
	fadeEndVolume(0),
	fadePassedTime(0),
	fadeDuration(0) { }

MidiDriver_Multisource::MidiDriver_Multisource() :
		_userVolumeScaling(false),
		_userMusicVolume(192),
		_userSfxVolume(192),
		_userMute(false),
		_timerRate(0),
		_fadeDelay(0),
		_timer_param(0),
		_timer_proc(0) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		// Default source type: 0 = music, 1+ = SFX
		_sources[i].type = (i == 0 ? SOURCE_TYPE_MUSIC : SOURCE_TYPE_SFX);
	}
}

void MidiDriver_Multisource::send(uint32 b) {
	send(-1, b);
}

uint32 MidiDriver_Multisource::property(int prop, uint32 param) {
	switch (prop) {
	case PROP_USER_VOLUME_SCALING:
		if (param == 0xFFFF)
			return _userVolumeScaling ? 1 : 0;
		_userVolumeScaling = param > 0;
		break;
	default:
		return MidiDriver::property(prop, param);
	}
	return 0;
}

void MidiDriver_Multisource::startFade(uint16 duration, uint16 targetVolume) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		startFade(i, duration, targetVolume);
	}
}

void MidiDriver_Multisource::startFade(uint8 source, uint16 duration, uint16 targetVolume) {
	Common::StackLock lock(_fadingMutex);

	assert(source < MAXIMUM_SOURCES);

	// Reset the number of microseconds which have passed since the start of
	// the fade.
	_sources[source].fadePassedTime = 0;
	// Set start volume to current volume.
	_sources[source].fadeStartVolume = _sources[source].volume;
	_sources[source].fadeEndVolume = targetVolume;
	// Convert to microseconds and set the duration. A duration > 0 will cause
	// the fade to be processed by updateFading.
	_sources[source].fadeDuration = duration * 1000;
}

void MidiDriver_Multisource::abortFade(FadeAbortType abortType) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		abortFade(i, abortType);
	}
}

void MidiDriver_Multisource::abortFade(uint8 source, FadeAbortType abortType) {
	Common::StackLock lock(_fadingMutex);

	assert(source < MAXIMUM_SOURCES);

	if (!isFading(source)) {
		// Nothing to abort.
		return;
	}

	// Set the fade duration to 0. This will stop the fade from being processed
	// by updateFading.
	_sources[source].fadeDuration = 0;

	// Now set the intended end volume.
	uint16 newSourceVolume;
	switch (abortType) {
	case FADE_ABORT_TYPE_END_VOLUME:
		newSourceVolume = _sources[source].fadeEndVolume;
		break;
	case FADE_ABORT_TYPE_START_VOLUME:
		newSourceVolume = _sources[source].fadeStartVolume;
		break;
	case FADE_ABORT_TYPE_CURRENT_VOLUME:
	default:
		return;
	}
	setSourceVolume(source, newSourceVolume);
}

bool MidiDriver_Multisource::isFading() {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		if (isFading(i))
			return true;
	}
	return false;
}

bool MidiDriver_Multisource::isFading(uint8 source) {
	assert(source < MAXIMUM_SOURCES);

	return _sources[source].fadeDuration > 0;
}

void MidiDriver_Multisource::updateFading() {
	Common::StackLock lock(_fadingMutex);

	// Decrease the fade delay by the time that has passed since the last
	// fading update.
	_fadeDelay -= (_fadeDelay < _timerRate ? _fadeDelay : _timerRate);

	bool updatedVolume = false;
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {

		if (_sources[i].fadeDuration > 0) {
			// This source has an active fade.

			// Update the time that has passed since the start of the fade.
			_sources[i].fadePassedTime += _timerRate;

			if (_sources[i].fadePassedTime >= _sources[i].fadeDuration) {
				// The fade has finished.

				// Set the end volume.
				setSourceVolume(i, _sources[i].fadeEndVolume);
				updatedVolume = true;

				// Stop further processing of this fade.
				_sources[i].fadeDuration = 0;
			} else if (_fadeDelay == 0) {
				// The fade has not yet finished and the fade delay has run
				// down. Waiting for the fade delay prevents sending out volume
				// updates on every updateFading call, which can overflow
				// slower MIDI hardware.

				// Set the new volume value.
				setSourceVolume(i, ((_sources[i].fadePassedTime * (_sources[i].fadeEndVolume - _sources[i].fadeStartVolume)) /
					_sources[i].fadeDuration) + _sources[i].fadeStartVolume);
				updatedVolume = true;
			}
		}
	}

	if (updatedVolume)
		// Set the fade delay to delay the next volume update.
		_fadeDelay = FADING_DELAY;
}

void MidiDriver_Multisource::deinitSource(uint8 source) {
	abortFade(source, FADE_ABORT_TYPE_END_VOLUME);

	// Stop all active notes for this source.
	stopAllNotes(source, 0xFF);
}

void MidiDriver_Multisource::setSourceType(SourceType type) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		setSourceType(i, type);
	}
}

void MidiDriver_Multisource::setSourceType(uint8 source, SourceType type) {
	assert(source < MAXIMUM_SOURCES);

	_sources[source].type = type;

	// A changed source type can mean a different user volume level should be
	// used for this source. Calling applySourceVolume will apply the user
	// volume.
	applySourceVolume(source);
}

void MidiDriver_Multisource::setSourceVolume(uint16 volume) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		setSourceVolume(i, volume);
	}
}

void MidiDriver_Multisource::setSourceVolume(uint8 source, uint16 volume) {
	assert(source < MAXIMUM_SOURCES);

	_sources[source].volume = volume;

	// Set the volume for active notes and/or MIDI channels for this source.
	applySourceVolume(source);
}

void MidiDriver_Multisource::setSourceNeutralVolume(uint16 volume) {
	for (int i = 0; i < MAXIMUM_SOURCES; ++i) {
		setSourceNeutralVolume(i, volume);
	}
}

void MidiDriver_Multisource::setSourceNeutralVolume(uint8 source, uint16 volume) {
	assert(source < MAXIMUM_SOURCES);

	_sources[source].neutralVolume = volume;
}

void MidiDriver_Multisource::syncSoundSettings() {
	// Get user volume settings.
	_userMusicVolume = MIN(256, ConfMan.getInt("music_volume"));
	_userSfxVolume = MIN(256, ConfMan.getInt("sfx_volume"));
	_userMute = ConfMan.getBool("mute");

	// Calling applySourceVolume will apply the user volume.
	applySourceVolume(0xFF);
}

void MidiDriver_Multisource::onTimer() {
	updateFading();

	if (_timer_proc && _timer_param)
		_timer_proc(_timer_param);
}
