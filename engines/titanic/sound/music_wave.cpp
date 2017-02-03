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

#include "titanic/sound/music_wave.h"
#include "titanic/sound/sound_manager.h"
#include "titanic/core/project_item.h"
#include "titanic/core/game_object.h"

namespace Titanic {

bool CMusicWave::_pianoToggle;
int CMusicWave::_pianoCtr;
int CMusicWave::_bassCtr;
byte *CMusicWave::_buffer;

void CMusicWave::init() {
	_pianoToggle = false;
	_pianoCtr = 0;
	_bassCtr = 0;
	_buffer = nullptr;
}

void CMusicWave::deinit() {
	delete[] _buffer;
	_buffer = nullptr;
}

CMusicWave::CMusicWave(CProjectItem *project, CSoundManager *soundManager, MusicWaveInstrument instrument) :
		_soundManager(soundManager), _instrument(instrument) {
	Common::fill(&_gameObjects[0], &_gameObjects[4], (CGameObject *)nullptr);
	_field20 = _field24 = 0;
	_field34 = -1;
	_field38 = 0;
	_field3C = 0;
	_field40 = 0;
	_field44 = 0;
	_field4C = 0;

	switch (instrument) {
	case MV_PIANO:
		_gameObjects[0] = static_cast<CGameObject *>(_project->findByName("Piano Man"));
		_gameObjects[1] = static_cast<CGameObject *>(_project->findByName("Piano Mouth"));
		_gameObjects[2] = static_cast<CGameObject *>(_project->findByName("Piano Left Arm"));
		_gameObjects[3] = static_cast<CGameObject *>(_project->findByName("Piano Right Arm"));
		_field20 = 0xCCCCCCCD;
		_field24 = 0x3FDCCCCC;
		break;

	case MV_BASS:
		_gameObjects[0] = static_cast<CGameObject *>(_project->findByName("Bass Player"));
		break;

	case MV_BELLS:
		_gameObjects[0] = static_cast<CGameObject *>(_project->findByName("Tubular Bells"));
		_field20 = 0x9999999A;
		_field24 = 0x3FD99999;
	
	case MV_SNAKE:
		_gameObjects[0] = static_cast<CGameObject *>(_project->findByName("Snake Hammer"));
		_gameObjects[1] = static_cast<CGameObject *>(_project->findByName("Snake Glass"));
		_gameObjects[2] = static_cast<CGameObject *>(_project->findByName("Snake Head"));
		_field20 = 0x5C28F5C3;
		_field24 = 0x3FC5C28F;
		break;
	}
}

void CMusicWave::setSize(uint count) {
	assert(_items.empty());
	_items.resize(count);
}

void CMusicWave::load(int index, const CString &filename, int v3) {
	assert(!_items[index]._waveFile);
	_items[index]._waveFile = createWaveFile(filename);
	_items[index]._value = v3;
}

CWaveFile *CMusicWave::createWaveFile(const CString &name) {
	if (name.empty())
		return nullptr;
	return _soundManager->loadSound(name);
}

void CMusicWave::start(int val) {
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
				_gameObjects[0]->movieSetAudioTiming(true);
				_gameObjects[0]->playMovie(0, 512, MOVIE_STOP_PREVIOUS);
				_field20 = 0x33333333;
				_field24 = 0x3FE33333;

			case 62:
				_gameObjects[0]->playMovie(828, 1023, MOVIE_STOP_PREVIOUS);
				_field20 = 0x33333333;
				_field24 = 0x3FD33333;
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
			int frameNum1 = (tempVal - frameNum) * 0.25;
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

void CMusicWave::stop() {
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

void CMusicWave::trigger() {
	if (_gameObjects[0]) {
		switch (_instrument) {
		case MV_PIANO:
			_gameObjects[0]->playMovie(0, 29, MOVIE_STOP_PREVIOUS);
			_gameObjects[2]->loadFrame(14);
			_gameObjects[3]->loadFrame(22);
			break;

		case MV_BELLS:
			_gameObjects[0]->loadFrame(0);
			_gameObjects[0]->movieSetAudioTiming(true);
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

void CMusicWave::reset() {
	_field34 = 0;
	_field38 = 0;
	_field3C = 0;
	_field40 = 0;
	_field44 = 0;
}

void CMusicWave::setState(int val) {
	_field34 = -1;
	_field38 = 0;
	_field3C = 0;
	_field40 = val;
	_field44 = 0;
}

int CMusicWave::setData(const byte *data, int count) {
	// TODO: Implement
	return 0;
}

} // End of namespace Titanic
