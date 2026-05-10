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
#include "common/file.h"
#include "common/memstream.h"
#include "mads/core/sound_manager.h"

namespace Audio {
class Mixer;
}

namespace MADS {

SoundManager::SoundManager(Audio::Mixer *mixer, bool &soundFlag) : _mixer(mixer), _soundFlag(soundFlag) {
	_opl = OPL::Config::create();
	_opl->init();
}

SoundManager::~SoundManager() {
	if (_driver) {
		_driver->stop();
		delete _driver;
	}

	delete _opl;
}

void SoundManager::init(int sectionNumber) {
	assert(sectionNumber > 0 && sectionNumber < 10);

	if (_driver != nullptr)
		delete _driver;

	// Load the correct driver for the section
	loadDriver(sectionNumber);

	// Set volume for newly loaded driver
	_driver->setVolume(_masterVolume);
}

void SoundManager::closeDriver() {
	if (_driver) {
		command(0);
		setEnabled(false);
		stop();

		removeDriver();
	}
}

void SoundManager::removeDriver() {
	delete _driver;
	_driver = nullptr;
}

void SoundManager::setEnabled(bool flag) {
	_pollSoundEnabled = flag;
	_soundPollFlag = false;
}

void SoundManager::pauseNewCommands() {
	_newSoundsPaused = true;
}

void SoundManager::startQueuedCommands() {
	_newSoundsPaused = false;

	while (!_queuedCommands.empty()) {
		int commandId = _queuedCommands.pop();
		command(commandId);
	}
}

void SoundManager::setVolume(int volume) {
	_masterVolume = volume;

	if (_driver)
		_driver->setVolume(volume);
}

int SoundManager::command(int commandId, int param) {
	if (_newSoundsPaused) {
		if (_queuedCommands.size() < 8)
			_queuedCommands.push(commandId);
		return _queuedCommands.size() - 1;
	} else if (_driver) {
		// Note: I don't know any way to identify music commands versus sfx
		// commands, so if sfx is mute, then so is music
		if (_soundFlag)
			_driver->command(commandId, param);
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

//====================================================================

SoundDriver::SoundDriver(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename,
		int dataOffset, int dataSize) : _mixer(mixer), _opl(opl) {
	// Open up the appropriate sound file
	Common::File soundFile;
	if (!soundFile.open(filename))
		error("Could not open file - %s", filename.toString().c_str());

	_soundData.resize(dataSize);
	soundFile.seek(dataOffset);
	soundFile.read(&_soundData[0], dataSize);
}

SoundDriver::~SoundDriver() {
	Common::StackLock slock(_driverMutex);
	_opl->stop();
}

} // namespace MADS
