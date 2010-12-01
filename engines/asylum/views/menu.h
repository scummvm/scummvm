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

	enum EyesAnimation {
		kEyesFront       = 0,
		kEyesLeft        = 1,
		kEyesRight       = 2,
		kEyesTop         = 3,
		kEyesBottom      = 4,
		kEyesTopLeft     = 5,
		kEyesTopRight    = 6,
		kEyesBottomLeft  = 7,
		kEyesBottomRight = 8,
		kEyesCrossed     = 9
	};

	enum MenuScreen {
		kMenuNone            = -1,
		kMenuNewGame         = 0,
		kMenuLoadGame        = 1,
		kMenuSaveGame        = 2,
		kMenuDeleteGame      = 3,
		kMenuViewCinematics  = 4,
		kMenuQuitGame        = 5,
		kMenuTextOptions     = 6,
		kMenuAudioOptions    = 7,
		kMenuSettings        = 8,
		kMenuKeyboardConfig  = 9,
		kMenuShowCredits     = 10,
		kMenuReturnToGame    = 11,
		kMenuMain            = 12
	};

	enum Fonts {
		kFontBlue = MAKE_RESOURCE(kResourcePackShared, 22),
		kFontYellow = MAKE_RESOURCE(kResourcePackShared, 16)
	};

	int32  _activeIcon;
	int32  _previousActiveIcon;
	uint32 _curIconFrame;
	int32 _curMouseCursor;
	int32  _cursorStep;
	uint32 _creditsBgFrame;
	int32 _creditsTextScroll;
	bool   _leftClick;
	bool   _active;

	// Game initialization
	bool _initGame;

	// Data
	MenuScreen       _activeScreen;
	ResourceId       _soundResourceId;
	bool             _allowInteraction;
	int32            _dword_4464BC;
	int32            _dword_455C74;
	bool             _dword_455D4C;
	bool             _dword_455D5C;
	int32            _dword_4562C0;
	int32            _dword_4562C4;
	int32            _dword_45628C;
	bool             _needEyeCursorInit;

	GraphicResource *_bgResource;
	GraphicResource *_eyeResource;
	GraphicResource *_iconResource;
	GraphicResource *_creditsResource;
	GraphicResource *_creditsFadeResource;

	/**
	 * Setups menu screen
	 */
	void setup();

	/**
	 * Switch between fonts.
	 *
	 * @param condition if true, load kFontYellow, if false, load kFontBlue.
	 */
	void switchFont(bool condition);

	//////////////////////////////////////////////////////////////////////////
	// Message handling
	bool init();
	bool update();
	bool music();
	bool key(const AsylumEvent &evt);
	bool mouse(const AsylumEvent &evt);

	void updateEyesAnimation();
	void updateMainMenu();

	void updateSubMenu();
	void exitSubMenu();
	void updateSubMenuNewGame();
	void updateSubMenuCinematics();
	void updateSubMenuSettings();
	void updateSubMenuQuitGame();
	void updateSubMenuShowCredits();
	void updateSubMenuReturnToGame();
}; // end of class MainMenu

} // end of namespace Asylum

#endif
