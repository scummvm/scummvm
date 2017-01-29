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

#include "titanic/sound/music_room_handler.h"
#include "titanic/sound/sound_manager.h"
#include "titanic/core/project_item.h"

namespace Titanic {

CMusicRoomHandler::CMusicRoomHandler(CProjectItem *project, CSoundManager *soundManager) :
		_project(project), _soundManager(soundManager), _stopWaves(false),
		_soundHandle(-1), _waveFile(nullptr), _soundVolume(100),
		_field108(0) {
	Common::fill(&_musicWaves[0], &_musicWaves[4], (CMusicWave *)nullptr);
}

CMusicRoomHandler::~CMusicRoomHandler() {
	stop();
	for (int idx = 0; idx < 4; ++idx)
		delete _musicWaves[idx];
}

CMusicWave *CMusicRoomHandler::createMusicWave(int waveIndex, int count) {
	switch (waveIndex) {
	case 0:
		_musicWaves[waveIndex] = new CMusicWave(_project, _soundManager, 2);
		break;
	case 1:
		_musicWaves[waveIndex] = new CMusicWave(_project, _soundManager, 3);
		break;
	case 2:
		_musicWaves[waveIndex] = new CMusicWave(_project, _soundManager, 0);
		break;
	case 3:
		_musicWaves[waveIndex] = new CMusicWave(_project, _soundManager, 1);
		break;
	default:
		return nullptr;
	}

	_musicWaves[waveIndex]->setSize(count);
	return _musicWaves[waveIndex];
}

void CMusicRoomHandler::createWaveFile(int musicVolume) {
	_soundVolume = musicVolume;
//	_waveFile = _soundManager->loadMusic()
}

bool CMusicRoomHandler::poll() {
	// TODO
	return false;
}

void CMusicRoomHandler::stop() {
	if (_waveFile) {
		_soundManager->stopSound(_soundHandle);
		delete _waveFile;
		_waveFile = nullptr;
		_soundHandle = -1;
	}

	for (int idx = 0; idx < 4; ++idx) {
		if (_stopWaves && _musicWaves[idx])
			_musicWaves[idx]->stop();
	}
}

bool CMusicRoomHandler::checkInstrument(MusicControlArea area) const {
	// TODO
	return false;
}

void CMusicRoomHandler::setSpeedControl2(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array2[area]._speedControl = value;
}

void CMusicRoomHandler::setPitchControl2(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array2[area]._pitchControl = value * 3;
}

void CMusicRoomHandler::setInversionControl2(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array2[area]._inversionControl = value;
}

void CMusicRoomHandler::setDirectionControl2(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array2[area]._directionControl = value;
}

void CMusicRoomHandler::setPitchControl(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array1[area]._pitchControl = value;
}

void CMusicRoomHandler::setSpeedControl(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array1[area]._speedControl = value;
}

void CMusicRoomHandler::setDirectionControl(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array1[area]._directionControl = value;
}

void CMusicRoomHandler::setInversionControl(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array1[area]._inversionControl = value;
}

void CMusicRoomHandler::setMuteControl(MusicControlArea area, int value) {
	if (area >= 0 && area <= 3 && value >= -2 && value <= 2)
		_array1[area]._muteControl = value;
}

bool CMusicRoomHandler::isBusy() {
	// TODO: stuff
	return _field108 > 0;
}

} // End of namespace Titanic
