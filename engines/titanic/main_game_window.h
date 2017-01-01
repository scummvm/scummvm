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

#ifndef TITANIC_MAIN_GAME_WINDOW_H
#define TITANIC_MAIN_GAME_WINDOW_H

#include "common/scummsys.h"
#include "common/array.h"
#include "titanic/game_manager.h"
#include "titanic/game_view.h"
#include "titanic/support/image.h"
#include "titanic/core/project_item.h"
#include "titanic/events.h"

namespace Titanic {

class TitanicEngine;

class CMainGameWindow : public CEventTarget {
private:
	TitanicEngine *_vm;
	int _pendingLoadSlot;
	uint32 _priorLeftDownTime;
	uint32 _priorMiddleDownTime;
	uint32 _priorRightDownTime;
private:
	/**
	 * Checks for the presence of any savegames and, if present,
	 * lets the user pick one to resume
	 */
	int getSavegameSlot();

	/**
	 * Creates the game "project" and determine a game save slot
	 * to use
	 */
	int selectSavegame();

	/**
	 * Used for drawing the PET fullscreen? maybe
	 */
	void drawPet(CScreenManager *screenManager);

	/**
	 * Draws the background for the view
	 */
	void drawView();

	/**
	 * Draws all the items within the view
	 */
	void drawViewContents(CScreenManager *screenManager);

	void leftButtonDoubleClick(const Point &mousePos);
	void middleButtonDoubleClick(const Point &mousePos);
	void rightButtonDoubleClick(const Point &mousePos);

	/**
	 * Returns true if the player can control the mouse
	 */
	bool isMouseControlEnabled() const;
public:
	CGameView *_gameView;
	CGameManager *_gameManager;
	CProjectItem *_project;
	bool _inputAllowed;
	Image *_image;
	void *_cursor;
public:
	CMainGameWindow(TitanicEngine *vm);
	virtual ~CMainGameWindow();

	/**
	* Called to handle any regular updates the game requires
	*/
	void onIdle();

	virtual void mouseMove(const Point &mousePos);
	virtual void leftButtonDown(const Point &mousePos);
	virtual void leftButtonUp(const Point &mousePos);
	virtual void middleButtonDown(const Point &mousePos);
	virtual void middleButtonUp(const Point &mousePos);
	virtual void rightButtonDown(const Point &mousePos);
	virtual void rightButtonUp(const Point &mousePos);
	virtual void mouseWheel(const Point &mousePos, bool wheelUp);
	virtual void keyDown(Common::KeyState keyState);

	/**
	 * Called when the application starts
	 */
	void applicationStarting();

	/**
	 * Sets the view to be shown
	 */
	void setActiveView(CViewItem *viewItem);

	/**
	 * Main draw method for the window
	 */
	void draw();

	/**
	 * Called by the event handler when a mouse event has been generated
	 */
	void mouseChanged();

	/**
	 * Schedules a savegame to be loaded
	 */
	void loadGame(int slotId);
};

} // End of namespace Titanic

#endif /* TITANIC_MAIN_GAME_WINDOW_H */
