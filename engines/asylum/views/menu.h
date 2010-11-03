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

#include "common/events.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/views/scene.h"
#include "asylum/system/graphics.h"
#include "asylum/system/text.h"
#include "asylum/system/cursor.h"

namespace Asylum {

class Text;
class Cursor;

class MainMenu {
public:
	MainMenu(AsylumEngine *vm);
	~MainMenu();

	void handleEvent(Common::Event *event, bool doUpdate);
	bool isActive() {
		return _active;
	}

	void openMenu();
	void closeMenu();

	void showOptions();

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
		kNewGame         = 0,
		kLoadGame        = 1,
		kSaveGame        = 2,
		kDeleteGame      = 3,
		kViewCinematics  = 4,
		kQuitGame        = 5,
		kTextOptions     = 6,
		kAudioOptions    = 7,
		kSettings        = 8,
		kKeyboardConfig  = 9,
		kShowCredits     = 10,
		kReturnToGame    = 11,
		kMainMenu        = 12
	};

	enum Fonts {
		kFontBlue,
		kFontYellow
	};

	Common::Event *_ev;

	Cursor *_cursor;
	int32  _activeIcon;
	int32  _previousActiveIcon;
	uint32 _curIconFrame;
	int32 _curMouseCursor;
	int32  _cursorStep;
	uint32 _creditsBgFrame;
	int32 _creditsTextScroll;
	bool   _leftClick;
	bool   _active;

	MenuScreen      _activeMenuScreen;
	ResourcePack    *_resPack;
	GraphicResource *_bgResource;
	GraphicResource *_eyeResource;
	GraphicResource *_iconResource;
	GraphicResource *_creditsResource;
	GraphicResource *_creditsFadeResource;

	void loadFont(Fonts font);
	/**
	 * Determine which font colour to use. If the condition is true,
	 * load kFontYellow, if false, load kFontBlue
	 */
	void switchFont(bool condition);

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

	void update();

}; // end of class MainMenu

} // end of namespace Asylum

#endif
