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
 */

#include "common/events.h"
#include "common/textconsole.h"

#include "pegasus/pegasus.h"

namespace Pegasus {

enum {
	kInterfaceOverviewButton = 0,
	kStartButton = 1,
	kRestoreButton = 2,
	kDifficultyButton = 3,
	kCreditsButton = 4,
	kQuitButton = 5
};

enum {
	kDemoStartButton = 0,
	kDemoCreditsButton = 1,
	kDemoQuitButton = 2
};	

void PegasusEngine::runMainMenu() {
	_sound->playSound("Sounds/Main Menu.aiff", true);

	// Note down how long since the last click
	uint32 lastClickTime = _system->getMillis();

	int buttonSelected = 0;
	drawMenu(buttonSelected);

	while (!shouldQuit() && _system->getMillis() - lastClickTime < 60 * 1000) {
		Common::Event event;
	
		// Ignore events for now
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_UP:
					if (buttonSelected > 0) {
						buttonSelected--;
						drawMenu(buttonSelected);
					}
					break;
				case Common::KEYCODE_DOWN:
					if ((isDemo() && buttonSelected < 2) || (!isDemo() && buttonSelected < 5)) {
						buttonSelected++;
						drawMenu(buttonSelected);
					}
					break;
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_RIGHT:
					if (buttonSelected == kDifficultyButton) {
						_adventureMode = !_adventureMode;
						drawMenu(buttonSelected);
					}
					break;
				case Common::KEYCODE_RETURN:
					if (buttonSelected != kDifficultyButton) {
						drawMenuButtonSelected(buttonSelected);
						setGameMode(buttonSelected);

						if (_gameMode != kMainMenuMode) {
							_sound->stopSound();
							return;
						}

						drawMenu(buttonSelected);
					}
					break;
				default:
					break;
				}

				// Update our last press time too
				lastClickTime = _system->getMillis();
				break;
			default:
				break;
			}
		}
		
		//_system->updateScreen();
		_system->delayMillis(10);
	}

	if (shouldQuit())
		return;

	// Too slow! Go back and show the intro again.
	_sound->stopSound();
	_video->playMovie("Images/Opening_Closing/LilMovie.movie");
	_gameMode = kIntroMode;
}

void PegasusEngine::drawMenu(int buttonSelected) {
	if (isDemo()) {
		_gfx->drawPict("Images/Demo/DemoMenu.pict", 0, 0, false);
	} else {
		_gfx->drawPict("Images/Main Menu/MainMenu.mac", 0, 0, false);
		if (!_adventureMode)
			_gfx->drawPict("Images/Main Menu/BtnWlk.pict", 320, 340, false);
	}

	drawMenuButtonHighlighted(buttonSelected);
}

// FIXME: Most of these coordinates can use tweaking

static const int kMainMenuButtonX = 152;
static const char s_mainMenuButtonSuffix[] = { 'L', 'S', 'S', 'L', 'S', 'S' };
static const int s_mainMenuButtonY[] = { 202, 252, 292, 337, 382, 422 };
static const char s_demoMainMenuButtonSuffix[] = { 'S', 'S', 'L' }; // SSL!
static const int s_demoMainMenuButtonX[] = { 38, 38, 28 };
static const int s_demoMainMenuButtonY[] = { 332, 366, 408 };

void PegasusEngine::drawMenuButtonHighlighted(int buttonSelected) {
	if (isDemo())
		_gfx->drawPictTransparent(Common::String("Images/Demo/Select") + s_demoMainMenuButtonSuffix[buttonSelected] + ".pict", s_demoMainMenuButtonX[buttonSelected], s_demoMainMenuButtonY[buttonSelected], _gfx->getColor(0xff, 0xff, 0xff), true);
	else
		_gfx->drawPictTransparent(Common::String("Images/Main Menu/Select") + s_mainMenuButtonSuffix[buttonSelected] + ".pict", kMainMenuButtonX, s_mainMenuButtonY[buttonSelected], _gfx->getColor(0xf8, 0xf8, 0xf8), true);
}

static const char *s_mainMenuButtonSelSuffix[] = { "Overvi", "Start", "Restor", "", "Credit", "Quit" };
static const char *s_demoMainMenuButtonSel[] = { "Start", "Credits", "Quit" };
static const int s_mainMenuSelButtonX[] = { 198, 210, 210, 0, 210, 210 };
static const int s_demoMainMenuSelButtonX[] = { 43, 43, 34 };
static const int s_demoMainMenuSelButtonY[] = { 338, 373, 410 };

void PegasusEngine::drawMenuButtonSelected(int buttonSelected) {
	if (isDemo())
		_gfx->drawPict(Common::String("Images/Demo/") + s_demoMainMenuButtonSel[buttonSelected] + ".pict", s_demoMainMenuSelButtonX[buttonSelected], s_demoMainMenuSelButtonY[buttonSelected], false);
	else
		_gfx->drawPict(Common::String("Images/Main Menu/pb") + s_mainMenuButtonSelSuffix[buttonSelected] + ".pict", s_mainMenuSelButtonX[buttonSelected], s_mainMenuButtonY[buttonSelected] + 5, false);

	drawMenuButtonHighlighted(buttonSelected);
}

void PegasusEngine::setGameMode(int buttonSelected) {
	if (isDemo()) {
		switch (buttonSelected) {
		case kDemoStartButton:
			_gameMode = kMainGameMode;
			break;
		case kDemoCreditsButton:
			warning("No credits just yet");
			break;
		case kDemoQuitButton:
			_gameMode = kQuitMode;
			break;
		}
	} else {
		switch (buttonSelected) {
		case kInterfaceOverviewButton:
			runInterfaceOverview();
			break;
		case kStartButton:
			_gameMode = kMainGameMode;
			break;
		case kRestoreButton:
			showLoadDialog();
			break;
		case kCreditsButton:
			warning("No credits just yet");
			break;
		case kQuitButton:
			_gameMode = kQuitMode;
			break;
		}
	}
}

} // End of namespace Pegasus
