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
#include "titanic/core/project_item.h"
#include "titanic/events.h"
#include "titanic/game_view.h"
#include "titanic/messages/messages.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/sound/background_sound_maker.h"
#include "titanic/support/files_manager.h"
#include "titanic/support/screen_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

CGameManager::CGameManager(CProjectItem *project, CGameView *gameView, Audio::Mixer *mixer):
		_project(project), _gameView(gameView), _trueTalkManager(this),
		_inputHandler(this), _inputTranslator(&_inputHandler),
		_gameState(this), _sound(this, mixer), _musicRoom(this),
		_treeItem(nullptr), _soundMaker(nullptr), _movieRoom(nullptr),
		_dragItem(nullptr), _transitionCtr(0), _lastDiskTicksCount(0), _tickCount2(0) {

	CTimeEventInfo::_nextId = 0;
	_movie = nullptr;
	_movieSurface = CScreenManager::_screenManagerPtr->createSurface(600, 340);
	_project->setGameManager(this);
	g_vm->_filesManager->setGameManager(this);
}

CGameManager::~CGameManager() {
	delete _movie;
	delete _movieSurface;
	destroyTreeItem();

	_project->resetGameManager();
}

void CGameManager::destroyTreeItem() {
	if (_treeItem) {
		_treeItem->destroyAll();
		_treeItem = nullptr;
	}
}

void CGameManager::save(SimpleFile *file) {
	file->writeNumber(_lastDiskTicksCount);
	_gameState.save(file);
	_timers.save(file, 0);
	_trueTalkManager.save(file);
	_sound.save(file);
}

void CGameManager::load(SimpleFile *file) {
	file->readNumber();

	_gameState.load(file);
	_timers.load(file);
	_trueTalkManager.load(file);
	_sound.load(file);
}

void CGameManager::preLoad() {
	updateDiskTicksCount();
	_timers.destroyContents();
	_soundMaker = nullptr;

	_sound.preLoad();
	_trueTalkManager.preLoad();
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

	// Signal to any registered timers
	_timers.postLoad(_lastDiskTicksCount, _project);

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
	_timers.preSave(_lastDiskTicksCount);
	_trueTalkManager.preSave();
	_sound.preSave();
}

void CGameManager::postSave() {
	_timers.postSave();
	_trueTalkManager.postSave();
	_sound.postSave();
}

void CGameManager::roomTransition(CRoomItem *oldRoom, CRoomItem *newRoom) {
	delete _movie;
	_movie = nullptr;

	CResourceKey movieKey;
	if (newRoom == oldRoom) {
		movieKey = oldRoom->getTransitionMovieKey();
		_movieRoom = oldRoom;
	} else {
		movieKey = oldRoom->getExitMovieKey();
		_movieRoom = nullptr;
	}

	CString filename = movieKey.getFilename();
	if (g_vm->_filesManager->fileExists(filename)) {
		_movieSurface->freeSurface();
		_movie = g_vm->_movieManager.createMovie(filename, _movieSurface);
	}
}

void CGameManager::playClip(CMovieClip *clip, CRoomItem *oldRoom, CRoomItem *newRoom) {
	if (oldRoom != newRoom || newRoom != _movieRoom || !_movie)
		roomTransition(oldRoom, newRoom);

	if (clip && clip->_startFrame != clip->_endFrame && _movie) {
		// Clip details specifying a sub-section of movie to play
		Rect tempRect(20, 10, SCREEN_WIDTH - 20, 350);
		CMouseCursor &mouseCursor = *CScreenManager::_screenManagerPtr->_mouseCursor;

		lockInputHandler();
		mouseCursor.incHideCounter();
		_movie->playCutscene(tempRect, clip->_startFrame, clip->_endFrame);
		mouseCursor.decHideCounter();
		unlockInputHandler();
	}
}

void CGameManager::update() {
	updateMovies();
	frameMessage(getRoom());
	_timers.update(g_vm->_events->getTicksCount());
	_trueTalkManager.removeCompleted();

	CScreenManager::_screenManagerPtr->_mouseCursor->update();

	CViewItem *view = getView();
	if (view) {
		// Expand the game manager's bounds to encompass any modified
		// areas of any of the view's items
		for (CTreeItem *item = view; item; item = item->scan(view)) {
			Rect r = item->getBounds();
			if (!r.isEmpty())
				_bounds.combine(r);
		}

		// Also include any modified area of the PET control
		if (_project) {
			CPetControl *pet = _project->getPetControl();

			if (pet)
				_bounds.combine(pet->getBounds());
		}

		// And the text cursor
		CScreenManager *screenManager = CScreenManager::_screenManagerPtr;
		CTextCursor *textCursor = screenManager->_textCursor;
		if (textCursor && textCursor->_active)
			_bounds.combine(textCursor->getCursorBounds());

		// Set the screen's modified area bounds
		screenManager->setSurfaceBounds(SURFACE_PRIMARY, _bounds);

		// Handle redrawing the view if there is any changed area
		if (!_bounds.isEmpty()) {
			_gameView->draw(_bounds);
			_bounds = Rect();
		}

		_gameState.checkForViewChange();
	}
}

void CGameManager::updateMovies() {
	// Initial iteration to mark all the movies as not yet handled
	for (CMovieList::iterator i = CMovie::_playingMovies->begin();
			i != CMovie::_playingMovies->end(); ++i)
		(*i)->_handled = false;

	bool repeatFlag;
	do {
		repeatFlag = false;

		// Scan for a movie to process
		for (CMovieList::iterator i = CMovie::_playingMovies->begin();
				i != CMovie::_playingMovies->end(); ++i) {
			CMovie *movie = *i;
			if (movie->_handled)
				continue;

			// Flag the movie to have been handled
			movie->_handled = true;

			CMovieEventList eventsList;
			if (!movie->handleEvents(eventsList))
				movie->removeFromPlayingMovies();

			while (!eventsList.empty()) {
				CMovieEvent *movieEvent = eventsList.front();

				switch (movieEvent->_type) {
				case MET_MOVIE_END: {
					CMovieEndMsg endMsg(movieEvent->_startFrame, movieEvent->_endFrame);
					endMsg.execute(movieEvent->_gameObject);
					break;
				}

				case MET_FRAME: {
					CMovieFrameMsg frameMsg(movieEvent->_initialFrame, 0);
					frameMsg.execute(movieEvent->_gameObject);
					break;
				}

				default:
					break;
				}

				eventsList.remove(movieEvent);
				delete movieEvent;
			}

			repeatFlag = true;
			break;
		}
	} while (repeatFlag);
}

void CGameManager::updateDiskTicksCount() {
	_lastDiskTicksCount = g_vm->_events->getTicksCount();
}

void CGameManager::roomChange() {
	delete _movie;
	delete _movieSurface;

	_movie = nullptr;
	_movieSurface = CScreenManager::_screenManagerPtr->createSurface(600, 340);
	_trueTalkManager.clear();

	for (CTreeItem *treeItem = _project; treeItem; treeItem = treeItem->scan(_project))
		treeItem->freeSurface();

	markAllDirty();
}

void CGameManager::frameMessage(CRoomItem *room) {
	if (room) {
		// Signal the next frame
		CFrameMsg frameMsg(g_vm->_events->getTicksCount());
		frameMsg.execute(room, nullptr, MSGFLAG_SCAN);

		if (_gameState._soundMakerAllowed && !_soundMaker) {
			// Check for a sound maker in the room
			_soundMaker = dynamic_cast<CBackgroundSoundMaker *>(
				_project->findByName("zBackgroundSoundMaker"));
		}

		// If there's a sound maker, dispatch the event to it as well
		if (_soundMaker)
			frameMsg.execute(_soundMaker);
	}
}

void CGameManager::addDirtyRect(const Rect &r) {
	if (_bounds.isEmpty())
		_bounds = r;
	else
		_bounds.combine(r);
}

void CGameManager::markAllDirty() {
	_bounds = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

CScreenManager *CGameManager::setScreenManager() const {
	return CScreenManager::setCurrent();
}

CString CGameManager::getFullViewName() {
	CViewItem *view = getView();
	CNodeItem *node = view->findNode();
	CRoomItem *room = node->findRoom();

	return CString::format("%s.%s.%s", room->getName().c_str(),
		node->getName().c_str(), view->getName().c_str());
}

} // End of namespace Titanic
