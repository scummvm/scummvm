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
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

void CGameManagerList::postLoad(uint ticks, CProjectItem *project) {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->postLoad(ticks, project);
}

void CGameManagerList::preSave() {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->preSave();
}

void CGameManagerList::postSave() {
	for (iterator i = begin(); i != end(); ++i)
		(*i)->postSave();
}

void CGameManagerList::update(uint ticks) {
	warning("TODO: CGameManagerList::update");
}

/*------------------------------------------------------------------------*/

void CGameManagerListItem::postLoad(uint ticks, CProjectItem *project) {
	warning("TODO");
}

void CGameManagerListItem::preSave() {
	warning("TODO: CGameManagerListItem::preSave");
}

void CGameManagerListItem::postSave() {
	warning("TODO: CGameManagerListItem::postSave");
}

/*------------------------------------------------------------------------*/

CGameManager::CGameManager(CProjectItem *project, CGameView *gameView):
		_project(project), _gameView(gameView), _trueTalkManager(this),
		_inputHandler(this), _inputTranslator(&_inputHandler),		
		_gameState(this), _sound(this), _musicRoom(this),
		_field30(0), _soundMaker(nullptr), _field4C(0),
		_dragItem(nullptr), _field54(0), _lastDiskTicksCount(0), _tickCount2(0) {
	
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

void CGameManager::preLoad() {
	updateDiskTicksCount();
	_list.destroyContents();
	_soundMaker = nullptr;

	_trueTalkManager.preLoad();
	_sound.preLoad();
}

void CGameManager::postLoad(CProjectItem *project) {
	if (_gameView) {
		_gameView->postLoad();

		if (!_gameView->_surface) {
			CViewItem *view = getView();
			if (view)
				_gameView->setView(view);
		}
	}
	
	// Signal to anything interested that the game has been loaded
	CLoadSuccessMsg msg(_lastDiskTicksCount - _tickCount2);
	msg.execute(project, nullptr, MSGFLAG_SCAN);

	// Signal to any registered list items
	_list.postLoad(_lastDiskTicksCount, _project);

	// Signal the true talk manager and sound
	_trueTalkManager.postLoad();
	_sound.postLoad();
}

void CGameManager::preSave(CProjectItem *project) {
	// Generate a message that a save is being done
	updateDiskTicksCount();
	CPreSaveMsg msg(_lastDiskTicksCount);
	msg.execute(project, nullptr, MSGFLAG_SCAN);

	// Notify sub-objects of the save
	_list.preSave();
	_trueTalkManager.preSave();
	_sound.preSave();
}

void CGameManager::postSave() {
	_list.postSave();
	_trueTalkManager.postSave();
}

void CGameManager::initBounds() {
	_bounds = Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void CGameManager::fn2() {
	warning("TODO");
}

void CGameManager::playClip(CMovieClip *clip, CRoomItem *oldRoom, CRoomItem *newRoom) {
	warning("TODO: CGameManager::playClip");
}

void CGameManager::update() {
	handleMovies();
	frameMessage(getRoom());
	_list.update(g_vm->_events->getTicksCount());
	_trueTalkManager.update1();
	_trueTalkManager.update2();
	CScreenManager::_screenManagerPtr->_mouseCursor->update();

	CViewItem *view = getView();
	if (view) {
		// Expand the game manager's bounds to encompass all the view's items
		for (CTreeItem *item = view; item; item = item->scan(view)) {
			Common::Rect r = item->getBounds();
			if (!r.isEmpty())
				_bounds.extend(r);
		}

		// Also include the PET control in the bounds
		if (_project) {
			CPetControl *pet = _project->getPetControl();
			if (pet)
				_bounds.extend(pet->getBounds());
		}

		// And the text cursor
		CScreenManager *screenManager = CScreenManager::_screenManagerPtr;
		CTextCursor *textCursor = screenManager->_textCursor;
		if (textCursor->_active)
			_bounds.extend(textCursor->getBounds());
		
		// Set the surface bounds
		screenManager->setSurfaceBounds(0, _bounds);

		if (!_bounds.isEmpty()) {
			_gameView->proc4(_bounds);
			_bounds = Common::Rect();
		}

		_gameState.checkForViewChange();
	}
}

void CGameManager::handleMovies() {
	warning("TODO: CGameManager::handleMovies");
}

void CGameManager::updateDiskTicksCount() {
	_lastDiskTicksCount = g_vm->_events->getTicksCount();
}

void CGameManager::viewChange() {
	delete _videoSurface1;
	delete _videoSurface2;

	_videoSurface1 = nullptr;
	_videoSurface2 = CScreenManager::_screenManagerPtr->createSurface(600, 340);
	_trueTalkManager.viewChange();

	for (CTreeItem *treeItem = _project; treeItem; treeItem = treeItem->scan(_project))
		treeItem->viewChange();

	initBounds();
}

CRoomItem *CGameManager::getRoom() {
	return _gameState._gameLocation.getRoom();
}

void CGameManager::frameMessage(CRoomItem *room) {
	if (room) {
		// Signal the next frame
		CFrameMsg frameMsg(g_vm->_events->getTicksCount());
		frameMsg.execute(room, nullptr, MSGFLAG_SCAN);

		if (!_soundMaker) {
			// Check for a sound maker in the room
			_soundMaker = dynamic_cast<CBackgroundSoundMaker *>(
				_project->findByName("zBackgroundSoundMaker"));
		}

		// If there's a sound maker, dispatch the event to it as well
		if (_soundMaker)
			frameMsg.execute(_soundMaker);
	}
}

} // End of namespace Titanic
