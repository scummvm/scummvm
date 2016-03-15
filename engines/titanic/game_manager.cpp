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

#include "titanic/titanic.h"
#include "titanic/game_manager.h"
#include "titanic/game_view.h"
#include "titanic/screen_manager.h"
#include "titanic/core/project_item.h"
#include "titanic/messages/messages.h"


namespace Titanic {

void CGameManagerList::postLoad(uint ticks, CProjectItem *project) {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->postLoad(ticks, project);
}

/*------------------------------------------------------------------------*/

void CGameManagerListItem::postLoad(uint ticks, CProjectItem *project) {
	warning("TODO");
}

/*------------------------------------------------------------------------*/

CGameManager::CGameManager(CProjectItem *project, CGameView *gameView):
		_project(project), _gameView(gameView), _trueTalkManager(this),
		_inputHandler(this), _inputTranslator(&_inputHandler),		
		_gameState(this), _sound(this), _musicRoom(this),
		_field30(0), _field34(0), _field4C(0), 
		_field50(0), _field54(0), _tickCount1(0), _tickCount2(0) {
	
	_videoSurface1 = nullptr;
	_videoSurface2 = CScreenManager::_screenManagerPtr->createSurface(600, 340);
	_project->setGameManager(this);
	g_vm->_filesManager.setGameManager(this);
}

void CGameManager::load(SimpleFile *file) {
	file->readNumber();

	_gameState.load(file);
	_list.load(file);
	_trueTalkManager.load(file);
	_sound.load(file);
}

void CGameManager::postLoad(CProjectItem *project) {
	if (_gameView) {
		_gameView->postLoad();

		if (!_gameView->_fieldC) {
			CViewItem *view = getView();
			if (view)
				_gameView->setView(view);
		}
	}
	
	// Signal to anything interested that the game has been loaded
	CLoadSuccessMsg msg(_tickCount1 - _tickCount2);
	msg.execute(project, nullptr, MSGFLAG_SCAN);

	// Signal to any registered list items
	_list.postLoad(_tickCount1, _project);

	// Signal the true talk manager and sound
	_trueTalkManager.postLoad();
	_sound.postLoad();
}

void CGameManager::initBounds() {
	_bounds = Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

} // End of namespace Titanic
