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

#include "titanic/sound/music_handler.h"
#include "titanic/sound/sound_manager.h"
#include "titanic/core/project_item.h"

namespace Titanic {

CMusicHandler::CMusicHandler(CProjectItem *project, CSoundManager *soundManager) :
		_project(project), _soundManager(soundManager), _stopWaves(false),
		_soundHandle(-1), _waveFile(nullptr) {
	Common::fill(&_musicWaves[0], &_musicWaves[4], (CMusicWave *)nullptr);
}

CMusicHandler::~CMusicHandler() {
	stop();
}

CMusicWave *CMusicHandler::createMusicWave(int waveIndex, int count) {
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

bool CMusicHandler::isBusy() {
	// TODO
	return false;
}

void CMusicHandler::stop() {
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

bool CMusicHandler::checkSound(int index) const {
	// TODO
	return false;
}

} // End of namespace Titanic
