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
#include "titanic/game_view.h"
#include "titanic/events.h"
#include "titanic/game_manager.h"
#include "titanic/support/screen_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

bool CGameStateMovieList::empty() {
	for (CGameStateMovieList::iterator i = begin(); i != end(); ) {
		CMovie *movie = *i;

		if (movie->isActive()) {
			++i;
		} else {
			i = erase(i);
		}
	}

	return Common::List<CMovie *>::empty();
}

/*------------------------------------------------------------------------*/

CGameState::CGameState(CGameManager *gameManager) :
		_gameManager(gameManager), _gameLocation(this), _passengerClass(NO_CLASS),
		_priorClass(NO_CLASS), _mode(GSMODE_NONE), _seasonNum(SEASON_SUMMER),
		_petActive(false), _soundMakerAllowed(false), _quitGame(false), _parrotMet(false),
		_nodeChangeCtr(0), _nodeEnterTicks(0), _parrotResponseIndex(0) {
}

void CGameState::save(SimpleFile *file) const {
	file->writeNumber(_petActive);
	file->writeNumber(_passengerClass);
	file->writeNumber(_priorClass);
	file->writeNumber(_seasonNum);
	file->writeNumber(_parrotMet);
	file->writeNumber(_parrotResponseIndex);
	_gameLocation.save(file);
	file->writeNumber(_soundMakerAllowed);
}

void CGameState::load(SimpleFile *file) {
	_petActive = file->readNumber() != 0;
	_passengerClass = (PassengerClass)file->readNumber();
	_priorClass = (PassengerClass)file->readNumber();
	_seasonNum = (Season)file->readNumber();
	_parrotMet = file->readNumber();
	_parrotResponseIndex = file->readNumber();
	_gameLocation.load(file);

	_soundMakerAllowed = file->readNumber();
	_nodeChangeCtr = 0;
	_nodeEnterTicks = 0;
}

void CGameState::setMode(GameStateMode newMode) {
	CScreenManager *sm = CScreenManager::_screenManagerPtr;

	if (newMode == GSMODE_CUTSCENE && _mode != GSMODE_CUTSCENE) {
		if (_gameManager)
			_gameManager->lockInputHandler();

		if (sm && sm->_mouseCursor)
			sm->_mouseCursor->incBusyCount();

	} else if (newMode != GSMODE_CUTSCENE && _mode == GSMODE_CUTSCENE) {
		if (sm && sm->_mouseCursor)
			sm->_mouseCursor->decBusyCount();

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
	CViewItem *newView = _movieList._destView;
	oldView->preEnterView(newView);

	_gameManager->_gameView->setView(newView);
	CRoomItem *oldRoom = oldView->findNode()->findRoom();
	CRoomItem *newRoom = newView->findNode()->findRoom();
	_gameManager->playClip(_movieList._movieClip, oldRoom, newRoom);

	_gameManager->_sound.preEnterView(newView, newRoom != oldRoom);
	_gameManager->decTransitions();
	oldView->enterView(newView);

	_movieList._destView = nullptr;
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
	if (g_vm->_events->isSpecialPressed(MK_SHIFT))
		clip = nullptr;

	if (_mode == GSMODE_CUTSCENE) {
		_movieList._destView = newView;
		_movieList._movieClip = clip;
		_gameManager->incTransitions();
	} else {
		oldView->preEnterView(newView);
		_gameManager->_gameView->setView(newView);
		CRoomItem *oldRoom = oldView->findNode()->findRoom();
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
	if (_mode == GSMODE_CUTSCENE && _movieList.empty()) {
		setMode(GSMODE_INTERACTIVE);
		if (_movieList._destView)
			enterView();
	}
}

void CGameState::addMovie(CMovie *movie) {
	_movieList.push_back(movie);
	setMode(GSMODE_CUTSCENE);
}

} // End of namespace Titanic
