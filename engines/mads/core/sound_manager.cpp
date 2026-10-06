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

#include "audio/fmopl.h"
#include "audio/mididrv.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "mads/core/sound_manager.h"

namespace Audio {
class Mixer;
}

namespace MADS {

const uint32 SoundManager::UPDATE_DELTA = 1000000 / 60;

SoundManager::SoundManager(Audio::Mixer *mixer, bool &soundFlag,
		bool supportsGeneralMidi) : _mixer(mixer), _soundFlag(soundFlag) {
	_updateDeltaRemainder = 0;
	_driverCallbackDelta = 0;
	_midiDriver = nullptr;

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_PCSPK | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32);
	MusicType musicType = MidiDriver::getMusicType(dev);
	if ((musicType == MT_GM || musicType == MT_GS) && ConfMan.getBool("native_mt32"))
		musicType = MT_MT32;
	switch (musicType) {
	case MT_MT32:
		_driverType = SOUND_MT32;
		break;
	case MT_GM:
	case MT_GS:
		_driverType = supportsGeneralMidi ? SOUND_GM : SOUND_ADLIB;
		break;
	case MT_PCSPK:
		_driverType = SOUND_PCSPEAKER;
		break;
	default:
		_driverType = SOUND_ADLIB;
		break;
	}
}

SoundManager::~SoundManager() {
	if (_driver != nullptr) {
		_driver->stop();
	}
	if (_midiDriver != nullptr) {
		_midiDriver->setTimerCallback(nullptr, nullptr);
		_midiDriver->close();
	}

	if (_driver != nullptr) {
		delete _driver;
		_driver = nullptr;
	}
	if (_midiDriver != nullptr) {
		delete _midiDriver;
		_midiDriver = nullptr;
	}
}

void SoundManager::init(int sectionNumber) {
	assert(sectionNumber > 0 && sectionNumber < 10);

	// Load the correct driver for the section
	removeDriver();
	loadDriver(sectionNumber);
	if (!_driver) {
		warning("No MADS sound driver is available for section %d",
			sectionNumber);
		return;
	}

	// Set volume for newly loaded driver
	_driver->setVolume(_masterVolume);
}

bool SoundManager::isDriverActive() {
	return _driver && _soundFlag && _driver->command(8, 0) != 0;
}

void SoundManager::closeDriver() {
	if (_driver) {
		command(0);
		stop();

		removeDriver();
	}
}

void SoundManager::removeDriver() {
	delete _driver;
	_driver = nullptr;
}

void SoundManager::pauseNewCommands() {
	_newSoundsPaused = true;
}

void SoundManager::startQueuedCommands() {
	_newSoundsPaused = false;

	while (!_queuedCommands.empty()) {
		const QueuedCommand queuedCommand = _queuedCommands.pop();
		command(queuedCommand._commandId, queuedCommand._param);
	}
}

void SoundManager::setVolume(int volume) {
	_masterVolume = volume;

	if (_driver)
		_driver->setVolume(volume);
}

int SoundManager::command(int commandId, int param) {
	if (_newSoundsPaused) {
		if (_queuedCommands.size() < 8) {
			QueuedCommand queuedCommand = { commandId, param };
			_queuedCommands.push(queuedCommand);
		}
		return _queuedCommands.size() - 1;
	} else if (_driver) {
		// Note: I don't know any way to identify music commands versus sfx
		// commands, so if sfx is mute, then so is music
		if (_soundFlag)
			return _driver->command(commandId, param);
	}

	return 0;
}

void SoundManager::stop() {
	if (_driver)
		_driver->stop();
}

void SoundManager::noise() {
	if (_driver)
		_driver->noise();
}

void SoundManager::onTimer() {
	// The frequency of the callbacks is dependent on the underlying driver
	// implementation and might not be 60Hz. Adjust to make sure poll() is called
	// with the correct frequency.
	/*
	_updateDeltaRemainder += _driverCallbackDelta;
	while (_updateDeltaRemainder >= UPDATE_DELTA) {
		if (_driver)
			_driver->poll();
		_updateDeltaRemainder -= UPDATE_DELTA;
	}
	*/

	uint32 serviceTicks = _hostTimer.advance(_driverCallbackDelta, 1000000);
	while (serviceTicks--) {
		// RSOUND export 4 is a return stub in every audited overlay.
		if (_driver && _hostTimer.pollDue())
			_driver->poll();
	}
}

void SoundManager::timerCallback(void *data) {
	SoundManager *soundManager = (SoundManager *)data;
	soundManager->onTimer();
}

//====================================================================

SoundDriver::SoundDriver(Audio::Mixer *mixer, const Common::Path &filename,
		int dataOffset, int dataSize) : _mixer(mixer) {
	// Open up the appropriate sound file
	Common::File soundFile;
	if (!soundFile.open(filename))
		error("Could not open file - %s", filename.toString().c_str());

	_soundData.resize(dataSize);
	soundFile.seek(dataOffset);
	soundFile.read(&_soundData[0], dataSize);
}

} // namespace MADS
