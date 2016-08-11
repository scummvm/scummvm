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
#include "titanic/titanic.h"

namespace Titanic {

CMusicHandler *CMusicRoom::_musicHandler;

CMusicRoom::CMusicRoom(CGameManager *gameManager) :
		_gameManager(gameManager) {
	_sound = &_gameManager->_sound;
	_items.resize(4);
}

CMusicRoom::~CMusicRoom() {
	destroyMusicHandler();
}

CMusicHandler *CMusicRoom::createMusicHandler() {
	if (_musicHandler)
		destroyMusicHandler();

	_musicHandler = new CMusicHandler(_gameManager->_project, &_sound->_soundManager);
	return _musicHandler;
}

void CMusicRoom::destroyMusicHandler() {
	delete _musicHandler;
	_musicHandler = nullptr;
}

void CMusicRoom::startMusic(int musicId) {
	// TODO
}

void CMusicRoom::stopMusic() {
	if (_musicHandler)
		_musicHandler->stop();
}

} // End of namespace Titanic
