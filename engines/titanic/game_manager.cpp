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

#include "titanic/game_manager.h"
#include "titanic/screen_manager.h"
#include "titanic/core/project_item.h"
#include "titanic/messages/messages.h"

namespace Titanic {

CGameManager::CGameManager(CProjectItem *project, CGameView *gameView):
		_project(project), _gameView(gameView), _trueTalkManager(this),
		_inputHandler(this), _inputTranslator(&_inputHandler),		
		_gameState(this), _sound(this), _musicRoom(this),
		_field30(0), _field34(0), _field48(0),
		_field4C(0), _field50(0), _field54(0), _tickCount(0) {
	_videoSurface = CScreenManager::_screenManagerPtr->createSurface(600, 340);
	_project->setGameManager(this);
}

void CGameManager::load(SimpleFile *file) {
	file->readNumber();

	_gameState.load(file);
	_list.load(file);
	_trueTalkManager.load(file);
	_sound.load(file);

}

void CGameManager::gameLoaded() {
	// TODO

	//CLoadSuccessMsg msg(0);

}

} // End of namespace Titanic z
