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

#include "audio/fmopl.h"
#include "common/memstream.h"
#include "mads/sound.h"
#include "mads/mads.h"
#include "mads/nebular/sound_nebular.h"

namespace Audio {
class Mixer;
}

namespace MADS {

SoundManager::SoundManager(MADSEngine *vm, Audio::Mixer *mixer) {
	_vm = vm;
	_mixer = mixer;
	_driver = nullptr;
	_pollSoundEnabled = false;
	_soundPollFlag = false;
	_newSoundsPaused = false;
	_masterVolume = 255;

	_preferRoland = false;

	_opl = OPL::Config::create();
	_opl->init();

	// Validate sound files
	switch (_vm->getGameID()) {
	case GType_RexNebular:
		Nebular::ASound::validate();
		break;
	default:
		break;
	}
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

	switch (_vm->getGameID()) {
	case GType_RexNebular:
		switch (sectionNumber) {
		case 1:
			_driver = new Nebular::ASound1(_mixer, _opl);
			break;
		case 2:
			_driver = new Nebular::ASound2(_mixer, _opl);
			break;
		case 3:
			_driver = new Nebular::ASound3(_mixer, _opl);
			break;
		case 4:
			_driver = new Nebular::ASound4(_mixer, _opl);
			break;
		case 5:
			_driver = new Nebular::ASound5(_mixer, _opl);
			break;
		case 6:
			_driver = new Nebular::ASound6(_mixer, _opl);
			break;
		case 7:
			_driver = new Nebular::ASound7(_mixer, _opl);
			break;
		case 8:
			_driver = new Nebular::ASound8(_mixer, _opl);
			break;
		case 9:
			_driver = new Nebular::ASound9(_mixer, _opl);
			break;
		default:
			_driver = nullptr;
			return;
		}
		break;

	default:
		warning("SoundManager: Unknown game");
		_driver = nullptr;
		return;
	}

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

void SoundManager::command(int commandId, int param) {
	if (_newSoundsPaused) {
		if (_queuedCommands.size() < 8)
			_queuedCommands.push(commandId);
	} else if (_driver) {
		// Note: I don't know any way to identify music commands versus sfx
		// commands, so if sfx is mute, then so is music
		if (_vm->_soundFlag)
			_driver->command(commandId, param);
	}
}

void SoundManager::stop() {
	if (_driver)
		_driver->stop();
}

void SoundManager::noise() {
	if (_driver)
		_driver->noise();
}

} // End of namespace MADS
