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
	_field108 = 0;
	_field118 = 0;
	_startTicks = _soundStartTicks = 0;
	Common::fill(&_musicWaves[0], &_musicWaves[4], (CMusicWave *)nullptr);
	for (int idx = 0; idx < 4; ++idx)
		_musicObjs[idx] = new CMusicObject(idx);
	Common::fill(&_startPos[0], &_startPos[4], 0);
	Common::fill(&_array5[0], &_array5[4], 0.0);
	Common::fill(&_position[0], &_position[4], 0);

	_audioBuffer = new CAudioBuffer(176400);
}

CMusicRoomHandler::~CMusicRoomHandler() {
	stop();
	for (int idx = 0; idx < 4; ++idx)
		delete _musicObjs[idx];

	delete _audioBuffer;
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

	_musicWaves[instrument]->setFilesCount(count);
	return _musicWaves[instrument];
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
			_startPos[idx] = _musicObjs[idx]->size() - 1;
		}

		_position[idx] = _startPos[idx];
		_array5[idx] = 0.0;
	}

	_field108 = 4;
	_field118 = 1;
	update();

	_waveFile = _soundManager->loadMusic(_audioBuffer, DisposeAfterUse::NO);
	_audioBuffer->advanceRead(_audioBuffer->getBytesToRead());
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
	_audioBuffer->enterCriticalSection();
	int size = _audioBuffer->getWriteBytesLeft();
	int count;
	uint16 *ptr;

	if (size > 0) {
		// Null out the destination write area
		uint16 *audioPtr = _audioBuffer->getWritePtr();
		Common::fill(audioPtr, audioPtr + size / sizeof(uint16), 0);

		for (int instrIdx = 0; instrIdx < 4; ++instrIdx) {
			CMusicWave *musicWave = _musicWaves[instrIdx];

			// Iterate through each of the four instruments and do an additive
			// read that will merge their data onto the output buffer
			for (count = size, ptr = audioPtr; count > 0; ) {
				int amount = musicWave->read(ptr, count);
				if (amount > 0) {
					count -= amount;
					ptr += amount / sizeof(uint16);
				} else if (!fn2(instrIdx)) {
					--_field108;
					break;
				}
			}
		}
		
		_audioBuffer->advanceWrite(size);
	}

	_audioBuffer->leaveCriticalSection();
}

void CMusicRoomHandler::fn1() {
	if (_active && _soundStartTicks) {
		for (int idx = 0; idx < 4; ++idx) {
			MusicRoomInstrument &ins1 = _array1[idx];
			MusicRoomInstrument &ins2 = _array2[idx];
			CMusicWave *musicWave = _musicWaves[idx];

			// Is this about checking playback position?
			if (_position[idx] < 0 || ins1._muteControl || _position[idx] >= _musicObjs[idx]->size()) {
				_position[idx] = -1;
				continue;
			}

			uint ticks = g_vm->_events->getTicksCount() - _soundStartTicks;
			double val = (double)ticks * 0.001 - 0.6;

			if (val >= musicWave->_floatVal) {
				_array5[idx] += fn3(idx, _position[idx]);

				const CValuePair &vp = (*_musicObjs[idx])[_position[idx]];
				if (vp._field0 != 0x7FFFFFFF) {
					int amount = getPitch(idx, _position[idx]);
					_musicWaves[idx]->start(amount);
				}

				if (ins1._directionControl == ins2._directionControl) {
					_position[idx]++;
				} else {
					_position[idx]--;
				}
			}
		}
	}
}

bool CMusicRoomHandler::fn2(int index) {
	int &arrIndex = _startPos[index];
	if (arrIndex < 0) {
		_musicWaves[index]->reset();
		return false;
	}

	const CMusicObject &mObj = *_musicObjs[index];
	if (arrIndex >= mObj.size()) {
		arrIndex = -1;
		_musicWaves[index]->reset();
		return false;
	}

	const CValuePair &vp = mObj[arrIndex];
	int size = static_cast<int>(fn3(index, arrIndex) * 44100.0) & ~1;

	if (vp._field0 == 0x7FFFFFFF || _array1[index]._muteControl)
		_musicWaves[index]->setSize(size);
	else
		_musicWaves[index]->chooseInstrument(getPitch(index, arrIndex), size);

	if (_array1[index]._directionControl == _array2[index]._directionControl) {
		++arrIndex;
	} else {
		--arrIndex;
	}

	return true;
}

double CMusicRoomHandler::fn3(int index, int arrIndex) {
	const CValuePair &vp = (*_musicObjs[index])[arrIndex];

	switch (_array1[index]._speedControl + _array2[index]._speedControl + 3) {
	case 0:
		return (double)vp._field4 * 1.5 * 0.0625 * 0.46875;
	case 1:
		return (double)vp._field4 * 1.33 * 0.0625 * 0.46875;
	case 2:
		return (double)vp._field4 * 1.25 * 0.0625 * 0.46875;
	case 4:
		return (double)vp._field4 * 0.75 * 0.0625 * 0.46875;
	case 5:
		return (double)vp._field4 * 0.67 * 0.0625 * 0.46875;
	case 6:
		return (double)vp._field4 * 0.5 * 0.0625 * 0.46875;
	default:
		return (double)vp._field4 * 1.0 * 0.0625 * 0.46875;
	}
}

int CMusicRoomHandler::getPitch(int index, int arrIndex) {
	const CMusicObject &mObj = *_musicObjs[index];
	const CValuePair &vp = mObj[arrIndex];
	int val = vp._field0;
	const MusicRoomInstrument &ins1 = _array1[index];
	const MusicRoomInstrument &ins2 = _array2[index];

	if (ins1._inversionControl != ins2._inversionControl) {
		val -= mObj._minVal * 2 + mObj._range;
	}

	val += ins1._pitchControl + ins2._pitchControl;
	return val;
}

} // End of namespace Titanic
