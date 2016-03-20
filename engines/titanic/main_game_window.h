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
#include "titanic/image.h"
#include "titanic/core/project_item.h"

namespace Titanic {

class TitanicEngine;

class CMainGameWindow {
private:
	TitanicEngine *_vm;

	/**
	 * Checks for the presence of any savegames and, if present,
	 * lets the user pick one to resume
	 */
	int loadGame();

	/**
	 * Creates the game "project" and determine a game save slot
	 * to use
	 */
	int selectSavegame();

	/**
	 * Used for drawing the PET fullscreen? maybe
	 */
	void drawPet(CScreenManager *screenManager);

	void draw1();

	void draw2(CScreenManager *screenManager);
public:
	CGameView *_gameView;
	CGameManager *_gameManager;
	CProjectItem *_project;
	bool _inputAllowed;
	Image *_image;
	void *_cursor;
public:
	CMainGameWindow(TitanicEngine *vm);

	/**
	 * Creates the window
	 */
	bool Create();

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
};

} // End of namespace Titanic

#endif /* TITANIC_MAIN_GAME_WINDOW_H */
