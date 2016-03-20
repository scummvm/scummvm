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
#include "titanic/simple_file.h"
#include "titanic/game_location.h"

namespace Titanic {

class CGameManager;

enum GameStateMode { GSMODE_0 = 0, GSMODE_1 = 1, GSMODE_2 = 2, GSMODE_3 = 3, GSMODE_4 = 4, GSMODE_5 = 5 };

class CGameStateList : public List<ListItem> {
public:
	CViewItem *_view;
	CMovieClip *_movieClip;
public:
	CGameStateList() : List<ListItem>(), _view(nullptr), _movieClip(nullptr) {}

	bool isViewChanging() const;
};

class CGameState {
public:
	CGameManager *_gameManager;
	CGameLocation _gameLocation;
	CGameStateList _list;
	int _field8;
	int _fieldC;
	GameStateMode _mode;
	int _field14;
	int _field18;
	int _field1C;
	int _field20;
	int _field24;
	uint _nodeChangeCtr;
	uint32 _nodeEnterTicks;
	Common::Point _mousePos;
	int _field38;
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
	void setMousePos(const Common::Point &pt);

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
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_STATE_H */
