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

#include "titanic/game_state.h"
#include "titanic/titanic.h"
#include "titanic/game_manager.h"
#include "titanic/support/screen_manager.h"

namespace Titanic {

bool CGameStateMovieList::clear() {
	for (CGameStateMovieList::iterator i = begin(); i != end(); ) {
		CMovieListItem *movieItem = *i;

		if (movieItem->_item->isActive()) {
			++i;
		} else {
			i = erase(i);
			delete movieItem;
		}
	}

	return !empty();
}

/*------------------------------------------------------------------------*/

CGameState::CGameState(CGameManager *gameManager) :
		_gameManager(gameManager), _gameLocation(this),
		_passengerClass(0), _priorClass(0), _mode(GSMODE_NONE),
		_seasonNum(SEASON_SUMMER), _petActive(false), _field1C(false), _quitGame(false),
		_field24(0), _nodeChangeCtr(0), _nodeEnterTicks(0), _field38(0) {
}

void CGameState::save(SimpleFile *file) const {
	file->writeNumber(_petActive);
	file->writeNumber(_passengerClass);
	file->writeNumber(_priorClass);
	file->writeNumber(_seasonNum);
	file->writeNumber(_field24);
	file->writeNumber(_field38);
	_gameLocation.save(file);
	file->writeNumber(_field1C);
}

void CGameState::load(SimpleFile *file) {
	_petActive = file->readNumber() != 0;
	_passengerClass = file->readNumber();
	_priorClass = file->readNumber();
	_seasonNum = (Season)file->readNumber();
	_field24 = file->readNumber();
	_field38 = file->readNumber();
	_gameLocation.load(file);

	_field1C = file->readNumber();
	_nodeChangeCtr = 0;
	_nodeEnterTicks = 0;
}

void CGameState::setMode(GameStateMode newMode) {
	CScreenManager *sm = CScreenManager::_screenManagerPtr;

	if (newMode == GSMODE_CUTSCENE && newMode != _mode) {
		if (_gameManager)
			_gameManager->lockInputHandler();

		if (sm && sm->_mouseCursor)
			sm->_mouseCursor->hide();

	} else if (newMode != GSMODE_CUTSCENE && newMode != _mode) {
		if (sm && sm->_mouseCursor)
			sm->_mouseCursor->show();

		if (_gameManager)
			_gameManager->unlockInputHandler();
	}

	_mode = newMode;
}

void CGameState::enterNode() {
	++_nodeChangeCtr;
	_nodeEnterTicks = g_vm->_events->getTicksCount();
}

void CGameState::enterView() {
	CViewItem *oldView = _gameLocation.getView();
	CViewItem *newView = _movieList._view;
	oldView->preEnterView(newView);

	_gameManager->_gameView->setView(newView);
	CRoomItem *oldRoom = oldView->findNode()->findRoom();
	CRoomItem *newRoom = newView->findNode()->findRoom();
	_gameManager->playClip(_movieList._movieClip, oldRoom, newRoom);

	_gameManager->_sound.preEnterView(newView, newRoom != oldRoom);
	_gameManager->dec54();
	oldView->enterView(newView);

	_movieList._view = nullptr;
	_movieList._movieClip = nullptr;
}

void CGameState::triggerLink(CLinkItem *link) {
	changeView(link->getDestView(), link->getClip());
}

void CGameState::changeView(CViewItem *newView, CMovieClip *clip) {
	// Signal the old view that it's being left
	CViewItem *oldView = _gameLocation.getView();
	oldView->leaveView(newView);

	// If Shift key is pressed, skip showing the transition clip
	if (g_vm->_window->isSpecialPressed(MK_SHIFT))
		clip = nullptr;

	if (_mode == GSMODE_CUTSCENE) {
		_movieList._view = newView;
		_movieList._movieClip = clip;
	} else {
		oldView->preEnterView(newView);
		_gameManager->_gameView->setView(newView);
		CRoomItem *oldRoom = newView->findNode()->findRoom();
		CRoomItem *newRoom = newView->findNode()->findRoom();

		// If a transition clip is defined, play it
		if (clip)
			_gameManager->playClip(clip, oldRoom, newRoom);

		// Final view change handling
		_gameManager->_sound.preEnterView(newView, newRoom != oldRoom);
		oldView->enterView(newView);
	}
}

void CGameState::checkForViewChange() {
	if (_mode == GSMODE_CUTSCENE && _movieList.clear()) {
		setMode(GSMODE_INTERACTIVE);
		if (_movieList._view)
			enterView();
	}
}

void CGameState::addMovie(CMovie *movie) {
	_movieList.push_back(new CMovieListItem(movie));
	setMode(GSMODE_CUTSCENE);
}

} // End of namespace Titanic
