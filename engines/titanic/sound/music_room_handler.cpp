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
#include "titanic/titanic.h"

namespace Titanic {

CMusicRoomHandler::CMusicRoomHandler(CProjectItem *project, CSoundManager *soundManager) :
		_project(project), _soundManager(soundManager), _active(false),
		_soundHandle(-1), _waveFile(nullptr), _volume(100) {
	Common::fill(&_musicWaves[0], &_musicWaves[4], (CMusicWave *)nullptr);
	_field108 = 0;
	_field118 = 0;
	_startTicks = _soundStartTicks = 0;
}

CMusicRoomHandler::~CMusicRoomHandler() {
	stop();
	for (int idx = 0; idx < 4; ++idx)
		delete _musicWaves[idx];
}

CMusicWave *CMusicRoomHandler::createMusicWave(MusicInstrument instrument, int count) {
	switch (instrument) {
	case BELLS:
		_musicWaves[BELLS] = new CMusicWave(_project, _soundManager, MV_BELLS);
		break;
	case SNAKE:
		_musicWaves[SNAKE] = new CMusicWave(_project, _soundManager, MV_SNAKE);
		break;
	case PIANO:
		_musicWaves[PIANO] = new CMusicWave(_project, _soundManager, MV_PIANO);
		break;
	case BASS:
		_musicWaves[BASS] = new CMusicWave(_project, _soundManager, MV_BASS);
		break;
	default:
		return nullptr;
	}

	_musicWaves[instrument]->setSize(count);
	return _musicWaves[instrument];
}

void CMusicRoomHandler::createWaveFile(int musicVolume) {
	_volume = musicVolume;
	// TODO
//	_waveFile = _soundManager->loadMusic()
}

void CMusicRoomHandler::stop() {
	if (_waveFile) {
		_soundManager->stopSound(_soundHandle);
		delete _waveFile;
		_waveFile = nullptr;
		_soundHandle = -1;
	}

	for (int idx = 0; idx < 4; ++idx) {
		_musicWaves[idx]->reset();
		if (_active && _musicWaves[idx])
			_musicWaves[idx]->stop();
	}

	_field108 = 0;
	_field118 = 0;
	_startTicks = _soundStartTicks = 0;
}

bool CMusicRoomHandler::checkInstrument(MusicInstrument instrument) const {
	return (_array1[instrument]._speedControl + _array2[instrument]._speedControl) == 0
		&& (_array1[instrument]._pitchControl + _array2[instrument]._pitchControl) == 0
		&& _array1[instrument]._directionControl == _array2[instrument]._directionControl
		&& _array1[instrument]._inversionControl == _array2[instrument]._inversionControl
		&& _array1[instrument]._muteControl == _array2[instrument]._muteControl;
}

void CMusicRoomHandler::setSpeedControl2(MusicInstrument instrument, int value) {
	if (instrument >= BELLS && instrument <= BASS && value >= -2 && value <= 2)
		_array2[instrument]._speedControl = value;
}

void CMusicRoomHandler::setPitchControl2(MusicInstrument instrument, int value) {
	if (instrument >= BELLS && instrument <= BASS && value >= -2 && value <= 2)
		_array2[instrument]._pitchControl = value * 3;
}

void CMusicRoomHandler::setInversionControl2(MusicInstrument instrument, bool value) {
	if (instrument >= BELLS && instrument <= BASS)
		_array2[instrument]._inversionControl = value;
}

void CMusicRoomHandler::setDirectionControl2(MusicInstrument instrument, bool value) {
	if (instrument >= BELLS && instrument <= BASS)
		_array2[instrument]._directionControl = value;
}

void CMusicRoomHandler::setPitchControl(MusicInstrument instrument, int value) {
	if (instrument >= BELLS && instrument <= BASS && value >= -2 && value <= 2)
		_array1[instrument]._pitchControl = value;
}

void CMusicRoomHandler::setSpeedControl(MusicInstrument instrument, int value) {
	if (instrument >= BELLS && instrument <= BASS && value >= -2 && value <= 2)
		_array1[instrument]._speedControl = value;
}

void CMusicRoomHandler::setDirectionControl(MusicInstrument instrument, bool value) {
	if (instrument >= BELLS && instrument <= BASS)
		_array1[instrument]._directionControl = value;
}

void CMusicRoomHandler::setInversionControl(MusicInstrument instrument, bool value) {
	if (instrument >= BELLS && instrument <= BASS)
		_array1[instrument]._inversionControl = value;
}

void CMusicRoomHandler::setMuteControl(MusicInstrument instrument, bool value) {
	if (instrument >= BELLS && instrument <= BASS)
		_array1[instrument]._muteControl = value;
}

void CMusicRoomHandler::trigger() {
	if (_active) {
		for (int idx = 0; idx < 4; ++idx)
			_musicWaves[idx]->trigger();
	}
}

bool CMusicRoomHandler::update() {
	uint currentTicks = g_vm->_events->getTicksCount();

	if (!_startTicks) {
		trigger();
		_startTicks = currentTicks;
	} else if (!_soundStartTicks && currentTicks >= (_startTicks + 3000)) {
		if (_waveFile) {
			CProximity prox;
			prox._channelVolume = _volume;
			_soundHandle = _soundManager->playSound(*_waveFile, prox);
		}

		_soundStartTicks = currentTicks;
	}

	updateAudio();
	fn1();

	return _field108 > 0;
}

void CMusicRoomHandler::updateAudio() {
	// TODO
}

void CMusicRoomHandler::fn1() {
	if (_active && _soundStartTicks) {
		for (int idx = 0; idx < 4; ++idx) {
			MusicRoomInstrument &ins1 = _array1[idx];
			MusicRoomInstrument &ins2 = _array2[idx];

			// Is this about checking playback position?
			if (_array6[idx] < 0 || ins1._muteControl || _array6[idx] >= _array3[idx]->_field4) {
				_array6[idx] = -1;
				continue;
			}

			// TODO
		}
	}
}

} // End of namespace Titanic
