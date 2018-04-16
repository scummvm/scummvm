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

#ifndef TITANIC_GAME_STATE_H
#define TITANIC_GAME_STATE_H

#include "titanic/core/list.h"
#include "titanic/core/link_item.h"
#include "titanic/support/simple_file.h"
#include "titanic/game_location.h"
#include "titanic/support/movie.h"

namespace Titanic {

class CGameManager;

enum GameStateMode {
	GSMODE_NONE = 0, GSMODE_INTERACTIVE = 1, GSMODE_CUTSCENE = 2,
	GSMODE_3 = 3, GSMODE_4 = 4, GSMODE_INSERT_CD = 5, GSMODE_PENDING_LOAD = 6
};

enum Season {
	SEASON_SUMMER = 0,
	SEASON_AUTUMN = 1,
	SEASON_WINTER = 2,
	SEASON_SPRING = 3
};

class CGameStateMovieList : public Common::List<CMovie *> {
public:
	CViewItem *_destView;
	CMovieClip *_movieClip;
public:
	CGameStateMovieList() : Common::List<CMovie *>(), _destView(nullptr), _movieClip(nullptr) {}

	/**
	 * Returns true if there are no movies in the list
	 */
	bool empty();
};

class CGameState {
public:
	CGameManager *_gameManager;
	CGameLocation _gameLocation;
	CGameStateMovieList _movieList;
	PassengerClass _passengerClass;
	PassengerClass _priorClass;
	GameStateMode _mode;
	Season _seasonNum;
	bool _petActive;
	bool _soundMakerAllowed;
	bool _quitGame;
	bool _parrotMet;
	uint _nodeChangeCtr;
	uint32 _nodeEnterTicks;
	Point _mousePos;
	int _parrotResponseIndex;
public:
	CGameState(CGameManager *gameManager);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file) const;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file);

	/**
	 * Sets a new mode
	 */
	void setMode(GameStateMode newMode);

	/**
	 * Sets the current mouse position
	 */
	void setMousePos(const Point &pt) { _mousePos = pt; }

	/**
	 * Gets the current mouse position
	 */
	Point getMousePos() const { return _mousePos; }

	/**
	 * Called by the PET when a new node is entered
	 */
	void enterNode();

	/**
	 * Enters a new view
	 */
	void enterView();

	/**
	 * Triggers a link item in a view
	 */
	void triggerLink(CLinkItem *link);

	/**
	 * Changes the current view
	 */
	void changeView(CViewItem *newView, CMovieClip *clip);

	/**
	 * Check for whether it's time to change the active view
	 */
	void checkForViewChange();

	/**
	 * Adds a movie to the movie list
	 */
	void addMovie(CMovie *movie);

	/**
	 * Change to the next season
	 */
	void changeSeason() {
		_seasonNum = (Season)(((int)_seasonNum + 1) & 3);
	}

	/**
	 * Sets whether the parrot has been met
	 */
	void setParrotMet(bool flag) { _parrotMet = flag; }

	/**
	 * Gets whether the parrot has been met
	 */
	bool getParrotMet() const { return _parrotMet; }

	/**
	 * Gets the counter for the number of times different nodes have
	 * been entered
	 */
	int getNodeChangedCtr() const { return _nodeChangeCtr; }

	/**
	 * Gets the node enter ticks amount
	 */
	uint32 getNodeEnterTicks() const { return _nodeEnterTicks; }

	/**
	 * Increments the index to use for parrot idle responses
	 */
	void incParrotResponse() { ++_parrotResponseIndex; }
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_STATE_H */
