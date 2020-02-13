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

#include "titanic/core/project_item.h"
#include "titanic/events.h"
#include "common/array.h"
#include "common/scummsys.h"

namespace Common {
struct Point;
}

namespace Titanic {

class CGameManager;
class CGameView;
class CScreenManager;
class Image;
class TitanicEngine;

class CMainGameWindow : public CEventTarget {
private:
	TitanicEngine *_vm;
	int _pendingLoadSlot;
	uint32 _priorLeftDownTime;
	uint32 _priorMiddleDownTime;
private:
	/**
	 * Returns true if a savegame was selected to be loaded
	 * from the ScummVM launcher
	 */
	bool isLoadingFromLauncher() const;

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

	void leftButtonDoubleClick(const Point &mousePos) override;
	void middleButtonDoubleClick(const Point &mousePos) override;

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
	~CMainGameWindow() override;

	/**
	* Called to handle any regular updates the game requires
	*/
	void onIdle() override;

	void mouseMove(const Point &mousePos) override;
	void leftButtonDown(const Point &mousePos) override;
	void leftButtonUp(const Point &mousePos) override;
	void middleButtonDown(const Point &mousePos) override;
	void middleButtonUp(const Point &mousePos) override;
	void mouseWheel(const Point &mousePos, bool wheelUp) override;
	void keyDown(Common::KeyState keyState) override;

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
