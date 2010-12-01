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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_MENU_H
#define ASYLUM_MENU_H

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;
class Cursor;
class GraphicResource;
class ResourcePack;
class Scene;
class Text;

class MainMenu : public EventHandler {
public:
	MainMenu(AsylumEngine *vm);
	~MainMenu();

	/**
	 * Shows the menu
	 */
	void show();

	/**
	 * Handle event.
	 *
	 * @param evt The event.
	 *
	 * @return true if it succeeds, false if it fails.
	 */
	bool handleEvent(const AsylumEvent &evt);

private:
	AsylumEngine *_vm;

	enum MenuScreen {
		kMenuNone            = -1,
		kMenuNewGame         = 0,
		kMenuLoadGame        = 1,
		kMenuSaveGame        = 2,
		kMenuDeleteGame      = 3,
		kMenuViewMovies  = 4,
		kMenuQuitGame        = 5,
		kMenuTextOptions     = 6,
		kMenuAudioOptions    = 7,
		kMenuSettings        = 8,
		kMenuKeyboardConfig  = 9,
		kMenuReturnToGame    = 10,
		kMenuShowCredits     = 11
	};

	enum MenuResource {
		kBackground = MAKE_RESOURCE(kResourcePackShared, 0),
		kEye        = MAKE_RESOURCE(kResourcePackShared, 1),
		kFontBlue   = MAKE_RESOURCE(kResourcePackShared, 22),
		kFontYellow = MAKE_RESOURCE(kResourcePackShared, 16)
	};

	// Game initialization
	bool             _initGame;

	// Data
	MenuScreen       _activeScreen;
	ResourceId       _soundResourceId;
	ResourceId       _musicResourceId;
	bool             _gameStarted;
	MenuScreen       _currentIcon;
	int32            _dword_4464B8;
	int32            _dword_455C74;
	bool             _dword_455C78;
	bool             _dword_455C80;
	bool             _dword_455D4C;
	bool             _dword_455D5C;
	bool             _dword_455DD8;
	int32            _dword_456288;
	int32            _dword_4562C0;
	int32            _creditsTextScroll;
	int32            _creditsFrameIndex;
	bool             _needEyeCursorInit;
	uint32           _iconFrames[12];

	/**
	 * Setups menu screen
	 */
	void setup();

	/**
	 * Leaves an opened menu
	 */
	void leave();

	/**
	 * Switch between fonts.
	 *
	 * @param condition if true, load kFontYellow, if false, load kFontBlue.
	 */
	void switchFont(bool condition);

	/**
	 * Close the credits.
	 */
	void closeCredits();

	/**
	 * List viewed movies
	 */
	void listMovies();

	/**
	 * Find if the mouse if on an icon
	 *
	 * @return The icon identifier
	 */
	MenuScreen findMousePosition();

	//////////////////////////////////////////////////////////////////////////
	// Message handling
	bool init();
	bool update();
	bool music();
	bool key(const AsylumEvent &evt);
	bool click(const AsylumEvent &evt);

	// Update handlers
	void updateNewGame();
	void updateLoadGame();
	void updateSaveGame();
	void updateDeleteGame();
	void updateViewMovies();
	void updateQuitGame();
	void updateTextOptions();
	void updateAudioOptions();
	void updateSettings();
	void updateKeyboardConfig();
	void updateReturnToGame();
	void updateShowCredits();

	// Click handlers
	void clickNewGame();
	void clickLoadGame();
	void clickSaveGame();
	void clickDeleteGame();
	void clickViewMovies();
	void clickQuitGame();
	void clickTextOptions();
	void clickAudioOptions();
	void clickSettings();
	void clickKeyboardConfig();
	void clickReturnToGame();
	void clickShowCredits();

	// Key handlers
	void keySaveGame();
	void keyKeyboardConfig();
	void keyShowCredits();

}; // end of class MainMenu

} // end of namespace Asylum

#endif
