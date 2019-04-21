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
#include "titanic/events.h"
#include "titanic/core/project_item.h"
#include "titanic/titanic.h"

namespace Titanic {

CMusicRoomHandler::CMusicRoomHandler(CProjectItem *project, CSoundManager *soundManager) :
		_project(project), _soundManager(soundManager), _active(false),
		_soundHandle(-1), _waveFile(nullptr), _volume(100) {
	_instrumentsActive = 0;
	_isPlaying = false;
	_startTicks = _soundStartTicks = 0;
	Common::fill(&_instruments[0], &_instruments[4], (CMusicRoomInstrument *)nullptr);
	for (int idx = 0; idx < 4; ++idx)
		_songs[idx] = new CMusicSong(idx);
	Common::fill(&_startPos[0], &_startPos[4], 0);
	Common::fill(&_animExpiryTime[0], &_animExpiryTime[4], 0.0);
	Common::fill(&_position[0], &_position[4], 0);

	_audioBuffer = new CAudioBuffer(88200);
}

CMusicRoomHandler::~CMusicRoomHandler() {
	stop();
	for (int idx = 0; idx < 4; ++idx)
		delete _songs[idx];

	delete _audioBuffer;
}

CMusicRoomInstrument *CMusicRoomHandler::createInstrument(MusicInstrument instrument, int count) {
	switch (instrument) {
	case BELLS:
		_instruments[BELLS] = new CMusicRoomInstrument(_project, _soundManager, MV_BELLS);
		break;
	case SNAKE:
		_instruments[SNAKE] = new CMusicRoomInstrument(_project, _soundManager, MV_SNAKE);
		break;
	case PIANO:
		_instruments[PIANO] = new CMusicRoomInstrument(_project, _soundManager, MV_PIANO);
		break;
	case BASS:
		_instruments[BASS] = new CMusicRoomInstrument(_project, _soundManager, MV_BASS);
		break;
	default:
		return nullptr;
	}

	_instruments[instrument]->setFilesCount(count);
	return _instruments[instrument];
}

void CMusicRoomHandler::setup(int volume) {
	_volume = volume;
	_audioBuffer->reset();

	for (int idx = 0; idx < 4; ++idx) {
		MusicRoomInstrument &ins1 = _array1[idx];
		MusicRoomInstrument &ins2 = _array2[idx];

		if (ins1._directionControl == ins2._directionControl) {
			_startPos[idx] = 0;
		} else {
			_startPos[idx] = _songs[idx]->size() - 1;
		}

		_position[idx] = _startPos[idx];
		_animExpiryTime[idx] = 0.0;
	}

	_instrumentsActive = 4;
	_isPlaying = true;
	update();

	_waveFile = _soundManager->loadMusic(_audioBuffer, DisposeAfterUse::NO);
	update();
}

void CMusicRoomHandler::stop() {
	if (_waveFile) {
		_soundManager->stopSound(_soundHandle);
		delete _waveFile;
		_waveFile = nullptr;
		_soundHandle = -1;
	}

	for (int idx = 0; idx < 4; ++idx) {
		_instruments[idx]->clear();
		if (_active && _instruments[idx])
			_instruments[idx]->stop();
	}

	_instrumentsActive = 0;
	_isPlaying = false;
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
		_array1[instrument]._pitchControl = value * 3;
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

void CMusicRoomHandler::start() {
	if (_active) {
		for (int idx = 0; idx < 4; ++idx)
			_instruments[idx]->start();
	}
}

bool CMusicRoomHandler::update() {
	uint currentTicks =  g_system->getMillis();

	if (!_startTicks) {
		start();
		_startTicks = currentTicks;
	} else if (!_soundStartTicks && currentTicks >= (_startTicks + 3000)) {
		if (_waveFile) {
			CProximity prox;
			prox._channelVolume = _volume;
			_soundHandle = _soundManager->playSound(*_waveFile, prox);
		}

		_soundStartTicks = currentTicks;
	}

	if (_instrumentsActive > 0) {
		updateAudio();
		updateInstruments();
	}

	return !_audioBuffer->isFinished();
}

void CMusicRoomHandler::updateAudio() {
	int size = _audioBuffer->freeSize();
	int count;
	int16 *ptr;

	if (size > 0) {
		// Create a temporary buffer for merging the instruments into
		int16 *audioData = new int16[size];
		Common::fill(audioData, audioData + size, 0);

		for (MusicInstrument instrument = BELLS; instrument <= BASS;
				instrument = (MusicInstrument)((int)instrument + 1)) {
			CMusicRoomInstrument *musicWave = _instruments[instrument];

			// Iterate through each of the four instruments and do an additive
			// read that will merge their data onto the output buffer
			for (count = size, ptr = audioData; count > 0; ) {
				int amount = musicWave->read(ptr, count * 2);
				if (amount > 0) {
					count -= amount / sizeof(uint16);
					ptr += amount / sizeof(uint16);
				} else if (!pollInstrument(instrument)) {
					--_instrumentsActive;
					break;
				}
			}
		}

		_audioBuffer->push(audioData, size);
		delete[] audioData;
	}

	if (_instrumentsActive == 0)
		// Reaching end of music
		_audioBuffer->finalize();
}

void CMusicRoomHandler::updateInstruments() {
	uint currentTicks = g_system->getMillis();

	if (_active && _soundStartTicks) {
		for (MusicInstrument instrument = BELLS; instrument <= BASS;
				instrument = (MusicInstrument)((int)instrument + 1)) {
			MusicRoomInstrument &ins1 = _array1[instrument];
			MusicRoomInstrument &ins2 = _array2[instrument];
			CMusicRoomInstrument *ins = _instruments[instrument];

			// Is this about checking playback position?
			if (_position[instrument] < 0 || ins1._muteControl || _position[instrument] >= _songs[instrument]->size()) {
				_position[instrument] = -1;
				continue;
			}

			double time = (double)(currentTicks - _soundStartTicks) / 1000.0 - 0.6;
			double threshold = _animExpiryTime[instrument] - ins->_insStartTime;

			if (time >= threshold) {
				_animExpiryTime[instrument] += getAnimDuration(instrument, _position[instrument]);

				const CValuePair &vp = (*_songs[instrument])[_position[instrument]];
				if (vp._data != 0x7FFFFFFF) {
					int amount = getPitch(instrument, _position[instrument]);
					_instruments[instrument]->update(amount);
				}

				if (ins1._directionControl == ins2._directionControl) {
					_position[instrument]++;
				} else {
					_position[instrument]--;
				}
			}
		}
	}
}

bool CMusicRoomHandler::pollInstrument(MusicInstrument instrument) {
	int &arrIndex = _startPos[instrument];
	if (arrIndex < 0) {
		_instruments[instrument]->clear();
		return false;
	}

	const CMusicSong &song = *_songs[instrument];
	if (arrIndex >= song.size()) {
		arrIndex = -1;
		_instruments[instrument]->clear();
		return false;
	}

	const CValuePair &vp = song[arrIndex];
	uint duration = static_cast<int>(getAnimDuration(instrument, arrIndex) * 44100.0) & ~1;

	if (vp._data == 0x7FFFFFFF || _array1[instrument]._muteControl)
		_instruments[instrument]->reset(duration);
	else
		_instruments[instrument]->chooseWaveFile(getPitch(instrument, arrIndex), duration);

	if (_array1[instrument]._directionControl == _array2[instrument]._directionControl) {
		++arrIndex;
	} else {
		--arrIndex;
	}

	return true;
}

double CMusicRoomHandler::getAnimDuration(MusicInstrument instrument, int arrIndex) {
	const CValuePair &vp = (*_songs[instrument])[arrIndex];

	switch (_array1[instrument]._speedControl + _array2[instrument]._speedControl + 3) {
	case 0:
		return (double)vp._length * 1.5 * 0.0625 * 0.46875;
	case 1:
		return (double)vp._length * 1.33 * 0.0625 * 0.46875;
	case 2:
		return (double)vp._length * 1.25 * 0.0625 * 0.46875;
	case 4:
		return (double)vp._length * 0.75 * 0.0625 * 0.46875;
	case 5:
		return (double)vp._length * 0.67 * 0.0625 * 0.46875;
	case 6:
		return (double)vp._length * 0.5 * 0.0625 * 0.46875;
	default:
		return (double)vp._length * 1.0 * 0.0625 * 0.46875;
	}
}

int CMusicRoomHandler::getPitch(MusicInstrument instrument, int arrIndex) {
	const CMusicSong &song = *_songs[instrument];
	const CValuePair &vp = song[arrIndex];
	int val = vp._data;
	const MusicRoomInstrument &ins1 = _array1[instrument];
	const MusicRoomInstrument &ins2 = _array2[instrument];

	if (ins1._inversionControl != ins2._inversionControl) {
		val = song._minVal * 2 + song._range - val;
	}

	val += ins1._pitchControl + ins2._pitchControl;
	return val;
}

} // End of namespace Titanic
