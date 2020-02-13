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

#include "titanic/sound/music_room_instrument.h"
#include "titanic/sound/sound_manager.h"
#include "titanic/core/project_item.h"
#include "titanic/core/game_object.h"

namespace Titanic {

bool CMusicRoomInstrument::_pianoToggle;
int CMusicRoomInstrument::_pianoCtr;
int CMusicRoomInstrument::_bassCtr;
byte *CMusicRoomInstrument::_buffer;
double *CMusicRoomInstrument::_array;
int CMusicRoomInstrument::_arrayIndex;

void CMusicRoomInstrument::init() {
	_pianoToggle = false;
	_pianoCtr = 0;
	_bassCtr = 0;
	_buffer = nullptr;
	_array = nullptr;
	_arrayIndex = 0;
}

void CMusicRoomInstrument::deinit() {
	delete[] _buffer;
	delete[] _array;
	_buffer = nullptr;
}

CMusicRoomInstrument::CMusicRoomInstrument(CProjectItem *project, CSoundManager *soundManager, MusicWaveInstrument instrument) :
		_project(project), _soundManager(soundManager), _instrument(instrument) {
	Common::fill(&_gameObjects[0], &_gameObjects[4], (CGameObject *)nullptr);
	_insStartTime = 0.0;
	_waveIndex = -1;
	_readPos = 0;
	_readIncrement = 0;
	_size = 0;
	_count = 0;
	_field4C = 0;

	switch (instrument) {
	case MV_PIANO:
		_gameObjects[0] = static_cast<CGameObject *>(_project->findByName("Piano Man"));
		_gameObjects[1] = static_cast<CGameObject *>(_project->findByName("Piano Mouth"));
		_gameObjects[2] = static_cast<CGameObject *>(_project->findByName("Piano Left Arm"));
		_gameObjects[3] = static_cast<CGameObject *>(_project->findByName("Piano Right Arm"));
		_insStartTime = 0.45;
		break;

	case MV_BASS:
		_gameObjects[0] = static_cast<CGameObject *>(_project->findByName("Bass Player"));
		break;

	case MV_BELLS:
		_gameObjects[0] = static_cast<CGameObject *>(_project->findByName("Tubular Bells"));
		_insStartTime = 0.4;
		break;

	case MV_SNAKE:
		_gameObjects[0] = static_cast<CGameObject *>(_project->findByName("Snake_Hammer"));
		_gameObjects[1] = static_cast<CGameObject *>(_project->findByName("Snake_Glass"));
		_gameObjects[2] = static_cast<CGameObject *>(_project->findByName("Snake_Head"));
		_insStartTime = 0.17;
		break;

	default:
		break;
	}
}

void CMusicRoomInstrument::setFilesCount(uint count) {
	assert(_items.empty());
	_items.resize(count);
}

void CMusicRoomInstrument::load(int index, const CString &filename, int v3) {
	assert(!_items[index]._waveFile);
	_items[index]._waveFile = createWaveFile(filename);
	_items[index]._value = v3;
}

CWaveFile *CMusicRoomInstrument::createWaveFile(const CString &name) {
	if (name.empty())
		return nullptr;
	return _soundManager->loadSound(name);
}

void CMusicRoomInstrument::start() {
	if (_gameObjects[0]) {
		switch (_instrument) {
		case MV_PIANO:
			_gameObjects[0]->playMovie(0, 29, MOVIE_STOP_PREVIOUS);
			_gameObjects[2]->loadFrame(14);
			_gameObjects[3]->loadFrame(22);
			break;

		case MV_BELLS:
			_gameObjects[0]->loadFrame(0);
			_gameObjects[0]->movieSetPlaying(true);
			break;

		case MV_SNAKE:
			_field4C = 22;
			_gameObjects[1]->playMovie(0, 22, 0);
			_gameObjects[2]->playMovie(0, 35, MOVIE_STOP_PREVIOUS);
			_gameObjects[0]->playMovie(0, 1, MOVIE_STOP_PREVIOUS);
			_gameObjects[0]->playMovie(0, 1, 0);
			_gameObjects[0]->playMovie(0, 1, 0);
			_gameObjects[0]->playMovie(0, 1, 0);
			_gameObjects[0]->playMovie(0, 1, 0);
			break;

		default:
			break;
		}
	}
}

void CMusicRoomInstrument::stop() {
	if (_gameObjects[0]) {
		switch (_instrument) {
		case MV_PIANO:
			_gameObjects[1]->setVisible(false);
			_gameObjects[2]->setVisible(false);
			_gameObjects[3]->setVisible(false);
			_gameObjects[0]->playMovie(29, 58, MOVIE_STOP_PREVIOUS);
			break;

		case MV_BELLS:
			_gameObjects[0]->stopMovie();
			break;

		default:
			break;
		}
	}
}

void CMusicRoomInstrument::update(int val) {
	if (_gameObjects[0]) {
		switch (_instrument) {
		case MV_PIANO:
			_gameObjects[1]->setVisible(true);
			_gameObjects[2]->setVisible(true);
			_gameObjects[3]->setVisible(true);
			_gameObjects[_pianoToggle ? 3 : 2]->playMovie(MOVIE_STOP_PREVIOUS);
			_pianoToggle = !_pianoToggle;

			switch (_pianoCtr) {
			case 0:
				_gameObjects[1]->playMovie(0, 4, MOVIE_STOP_PREVIOUS);
				break;
			case 1:
				_gameObjects[1]->playMovie(4, 8, MOVIE_STOP_PREVIOUS);
				break;
			case 2:
				_gameObjects[1]->playMovie(8, 12, MOVIE_STOP_PREVIOUS);
				break;
			case 3:
				_gameObjects[1]->playMovie(12, 16, MOVIE_STOP_PREVIOUS);
				break;
			default:
				break;
			}

			_pianoCtr = (_pianoCtr + 1) % 4;
			break;

		case MV_BASS:
			switch (_bassCtr) {
			case 0:
				_gameObjects[0]->playMovie(0, 7, MOVIE_STOP_PREVIOUS);
				break;
			case 1:
				_gameObjects[0]->playMovie(7, 14, MOVIE_STOP_PREVIOUS);
				break;
			case 2:
				_gameObjects[0]->playMovie(15, 24, MOVIE_STOP_PREVIOUS);
				break;
			case 3:
				_gameObjects[0]->playMovie(25, 33, MOVIE_STOP_PREVIOUS);
				break;
			default:
				break;
			}

			// WORKAROUND: Original didn't change the selected bass animation
			_bassCtr = (_bassCtr + 1) % 4;
			break;

		case MV_BELLS:
			switch (val) {
			case 60:
				_gameObjects[0]->playMovie(0, 512, MOVIE_STOP_PREVIOUS);
				_gameObjects[0]->movieSetPlaying(true);
				_insStartTime = 0.6;
				break;

			case 62:
				_gameObjects[0]->playMovie(828, 1023, MOVIE_STOP_PREVIOUS);
				_insStartTime = 0.3;
				break;

			case 63:
				_gameObjects[0]->playMovie(1024, 1085, MOVIE_STOP_PREVIOUS);
				break;

			default:
				break;
			}
			break;

		case MV_SNAKE: {
			_gameObjects[0]->playMovie(0, 7, MOVIE_STOP_PREVIOUS);

			double tempVal = 46.0 - ((double)(val - 14) * 1.43);
			int frameNum = _field4C;
			int frameNum1 = (int)((tempVal - frameNum) * 0.25);
			_gameObjects[1]->playMovie(frameNum1, frameNum1, MOVIE_STOP_PREVIOUS);

			frameNum += frameNum1;
			_gameObjects[1]->playMovie(frameNum, frameNum, 0);

			frameNum += frameNum1;
			_gameObjects[1]->playMovie(frameNum, frameNum, 0);

			_gameObjects[2]->playMovie(45, 49, MOVIE_STOP_PREVIOUS);
			break;
		}

		default:
			break;
		}
	}
}

void CMusicRoomInstrument::clear() {
	_waveIndex = 0;
	_readPos = 0;
	_readIncrement = 0;
	_size = 0;
	_count = 0;
}

void CMusicRoomInstrument::reset(uint total) {
	_waveIndex = -1;
	_readPos = 0;
	_readIncrement = 0;
	_size = total;
	_count = 0;
}

int CMusicRoomInstrument::read(int16 *ptr, uint size) {
	if (!_size)
		return 0;

	if (size >= _size)
		size = _size;

	if (_waveIndex != -1) {
		// Lock the specified wave file for access
		const int16 *data = _items[_waveIndex]._waveFile->lock();
		assert(data);
		const int16 *src = data;

		// Loop through merging data from the wave file into the dest buffer
		for (uint idx = 0; idx < (size / sizeof(int16)); ++idx, _readPos += _readIncrement) {
			uint srcPos = _readPos >> 8;
			if (srcPos >= _count)
				break;

			int16 val = READ_LE_UINT16(src + srcPos);
			*ptr++ += val;
		}

		// Unlock the wave file
		_items[_waveIndex]._waveFile->unlock(data);
	}

	_size -= size;
	return size;
}

void CMusicRoomInstrument::chooseWaveFile(int index, int size) {
	if (!_array)
		setupArray(-36, 36);

	int minDiff = ABS(_items[0]._value - index);
	int waveIndex = 0;

	for (uint idx = 1; idx < _items.size(); ++idx) {
		int diff = ABS(_items[idx]._value - index);
		if (diff < minDiff) {
			minDiff = diff;
			waveIndex = idx;
		}
	}

	const CInstrumentWaveFile &wf = _items[waveIndex];
	int arrIndex = _arrayIndex - wf._value + index;
	uint waveSize = wf._waveFile->size();

	_waveIndex = waveIndex;
	_readPos = 0;
	_readIncrement = (int)(_array[arrIndex] * 256);
	_size = size;
	_count = waveSize / 2;
}

void CMusicRoomInstrument::setupArray(int minVal, int maxVal) {
	// Delete any prior array and recreate it
	delete[] _array;

	int arrSize = maxVal - minVal + 1;
	_array = new double[arrSize];
	_arrayIndex = ABS(minVal);

	// Setup array contents
	_array[_arrayIndex] = 1.0;

	double val = 1.0594634;
	for (int idx = 1; idx <= maxVal; ++idx) {
		_array[_arrayIndex + idx] = val;
		val *= 1.0594634;
	}

	val = 0.94387404038686;
	for (int idx = -1; idx >= minVal; --idx) {
		_array[_arrayIndex + idx] = val;
		val *= 0.94387404038686;
	}
}

} // End of namespace Titanic
