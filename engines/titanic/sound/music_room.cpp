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

#include "common/textconsole.h"
#include "titanic/sound/music_room.h"
#include "titanic/sound/sound.h"
#include "titanic/game_manager.h"

namespace Titanic {

CMusicRoomHandler *CMusicRoom::_musicHandler;

CMusicRoom::CMusicRoom(CGameManager *gameManager) :
		_gameManager(gameManager) {
	_sound = &_gameManager->_sound;
}

CMusicRoom::~CMusicRoom() {
	destroyMusicHandler();
}

CMusicRoomHandler *CMusicRoom::createMusicHandler() {
	if (_musicHandler)
		destroyMusicHandler();

	_musicHandler = new CMusicRoomHandler(_gameManager->_project, &_sound->_soundManager);
	return _musicHandler;
}

void CMusicRoom::destroyMusicHandler() {
	delete _musicHandler;
	_musicHandler = nullptr;
}

void CMusicRoom::setupMusic(int volume) {
	if (_musicHandler) {
		// Set up the control values that form the correct settings
		_musicHandler->setSpeedControl2(BELLS, 0);
		_musicHandler->setSpeedControl2(SNAKE, 1);
		_musicHandler->setSpeedControl2(PIANO, -1);
		_musicHandler->setSpeedControl2(BASS, -2);

		_musicHandler->setPitchControl2(BELLS, 1);
		_musicHandler->setPitchControl2(SNAKE, 2);
		_musicHandler->setPitchControl2(PIANO, 0);
		_musicHandler->setPitchControl2(BASS, 1);

		_musicHandler->setInversionControl2(BELLS, true);
		_musicHandler->setInversionControl2(SNAKE, false);
		_musicHandler->setInversionControl2(PIANO, true);
		_musicHandler->setInversionControl2(BASS, false);

		_musicHandler->setDirectionControl2(BELLS, false);
		_musicHandler->setDirectionControl2(SNAKE, false);
		_musicHandler->setDirectionControl2(PIANO, true);
		_musicHandler->setDirectionControl2(BASS, true);

		// Set up the current control values
		for (MusicInstrument idx = BELLS; idx <= BASS;
				idx = (MusicInstrument)((int)idx + 1)) {
			MusicRoomInstrument &instr = _instruments[idx];
			_musicHandler->setSpeedControl(idx, instr._speedControl);
			_musicHandler->setPitchControl(idx, instr._pitchControl);
			_musicHandler->setDirectionControl(idx, instr._directionControl);
			_musicHandler->setInversionControl(idx, instr._inversionControl);
			_musicHandler->setMuteControl(idx, instr._muteControl);
		}

		// Set up the music handler
		_musicHandler->setup(volume);
	}
}

void CMusicRoom::stopMusic() {
	if (_musicHandler)
		_musicHandler->stop();
}

} // End of namespace Titanic
