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

#ifndef TITANIC_GAME_VIEW_H
#define TITANIC_GAME_VIEW_H

#include "common/scummsys.h"

namespace Titanic {

class CMainGameWindow;
class CGameManager;
class CResourceKey;
class CViewItem;
class CVideoSurface;
class Rect;

class CGameView {
protected:
	CGameManager *_gameManager;
public:
	CVideoSurface *_surface;
public:
	CGameView();
	virtual ~CGameView();

	/**
	 * Set the game manager
	 */
	void setGameManager(CGameManager *gameManager);

	/**
	 * Called after loading a game has finished
	 */
	void postLoad();

	virtual void deleteView(int roomNumber, int nodeNumber, int viewNumber);

	/**
	 * Set the currently active view
	 */
	virtual void setView(CViewItem *item) = 0;

	virtual void draw(const Rect &bounds) = 0;

	/**
	 * Creates a surface from a specified resource
	 */
	void createSurface(const CResourceKey &key);

	/**
	 * Draws the background of a view
	 */
	void drawView();
};

class CSTGameView: public CGameView {
private:
	CMainGameWindow *_gameWindow;
public:
	CSTGameView(CMainGameWindow *gameWindow);

	/**
	 * Set the currently active view
	 */
	void setView(CViewItem *item) override;

	/**
	 * Handles drawing the view
	 */
	void draw(const Rect &bounds) override;
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_VIEW_H */
