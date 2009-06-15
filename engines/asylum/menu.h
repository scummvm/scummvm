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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_MENU_H_
#define ASYLUM_MENU_H_

#include "common/events.h"

#include "asylum/asylum.h"
#include "asylum/resourcepack.h"
#include "asylum/scene.h"
#include "asylum/graphics.h"
#include "asylum/text.h"

namespace Asylum {

class Scene;
class Screen;
class Sound;
class Text;

class MainMenu {
public:
	MainMenu(Screen *screen, Sound *sound, Scene *state);
	~MainMenu();

	void handleEvent(Common::Event *event, bool doUpdate);
	bool isActive() { return _active; }
	void openMenu();
	void closeMenu();

private:

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

	Screen *_screen;
	Sound *_sound;
	Scene *_Scene;
	Common::Event *_ev;

	int _mouseX;
	int _mouseY;
	int _activeIcon;
	int _previousActiveIcon;
	int _curIconFrame;
	int _curMouseCursor;
	int _cursorStep;
    int _creditsBgFrame;
    int _creditsTextScroll;
	bool _leftClick;
	bool _active;
	MenuScreen _activeMenuScreen;
	ResourcePack *_resPack;
	GraphicResource *_bgResource;
	GraphicResource *_cursorResource;
	GraphicResource *_eyeResource;
	GraphicResource *_iconResource;
    GraphicResource *_creditsResource;
    GraphicResource *_creditsFadeResource;
    Text *_text;
	Text *_textBlue;

	void updateCursor();
	void updateEyesAnimation();
	void updateMainMenu();

	void updateSubMenu();
	void exitSubMenu();
    void updateSubMenuNewGame();
    void updateSubMenuQuitGame();
    void updateSubMenuShowCredits();

	void update();
}; // end of class MainMenu

} // end of namespace Asylum

#endif
