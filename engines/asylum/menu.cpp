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

#include "asylum/menu.h"
#include "asylum/resourcepack.h"
#include "asylum/graphics.h"

namespace Asylum {

/** This fixes the menu icons text x position on screen */
const int MenuIconFixedXpos[12] = { 28, 128, 225, 320, 410, 528, 16, 115, 237, 310, 508, 419 };

MainMenu::MainMenu(Screen *screen, Sound *sound, GameState *gameState) : 
	_screen(screen), _sound(sound), _gameState(gameState) {
	_mouseX             = 0;
	_mouseY             = 0;
	_leftClick          = false;
	_activeIcon         = -1;
	_previousActiveIcon = -1;
	_curIconFrame       = 0;
	_curMouseCursor     = 0;
	_cursorStep         = 1;
    _creditsBgFrame     = 0;
    _creditsTextScroll  = 0x1E0 - 30;
	_activeMenuScreen   = kMainMenu;
	_active				= false;

	_resPack = new ResourcePack(1);
	_bgResource = new GraphicResource(_resPack, 0);
	_eyeResource = new GraphicResource(_resPack, 1);
	_cursorResource = new GraphicResource(_resPack, 2);

	_iconResource = 0;
	_creditsResource = 0;
	_creditsFadeResource = 0;

    _text = new Text(_screen);
    _text->loadFont(_resPack, 16);	// 0x80010010, yellow font
	_textBlue = new Text(_screen);
	_textBlue->loadFont(_resPack, 22);	// 0x80010016, blue font
}

MainMenu::~MainMenu() {
	delete _creditsResource;
	delete _creditsFadeResource;
	delete _textBlue;
    delete _text;
	delete _iconResource;
	delete _eyeResource;
	delete _cursorResource;
	delete _bgResource;
	delete _resPack;
}

void MainMenu::openMenu() {
	_active = true;

	// Load the graphics palette
	_screen->setPalette(_resPack, 17);
	// Copy the bright background to the back buffer
	GraphicFrame *bg = _bgResource->getFrame(1);
	_screen->copyToBackBuffer((byte *)bg->surface.pixels, 0, 0, bg->surface.w, bg->surface.h);

	// Set mouse cursor
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();

	// Start playing music
	_sound->playMusic(_resPack, 39);
}

void MainMenu::closeMenu() {
	_active = false;
}

void MainMenu::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {
	case Common::EVENT_MOUSEMOVE:
		_mouseX = _ev->mouse.x;
		_mouseY = _ev->mouse.y;
		break;
	case Common::EVENT_LBUTTONUP:
		_leftClick = true;
		break;
	default:
		break;
	}

	if (doUpdate || _leftClick)
		update();
}

void MainMenu::update() {
	updateEyesAnimation();
	updateCursor();

	if (_activeMenuScreen == kMainMenu) {
		updateMainMenu();
	} else {
		updateSubMenu();
	}

	if (_leftClick) {
		_leftClick = false;
		
		if (_activeIcon != -1) {
			// Copy the dark background to the back buffer
			GraphicFrame *bg = _bgResource->getFrame(0);
			_screen->copyToBackBuffer((byte *)bg->surface.pixels, 0, 0, bg->surface.w, bg->surface.h);
			_activeMenuScreen = (MenuScreen) _activeIcon;

			// Set the cursor
			delete _cursorResource;
			_cursorResource = new GraphicResource(_resPack, 3);
			_screen->setCursor(_cursorResource, 0);
		}

		switch (_activeIcon) {
			case kNewGame:
				// Nothing here
				break;
			case kLoadGame:
				// TODO
				break;
			case kSaveGame:
				// TODO
				break;
			case kDeleteGame:
				// TODO
				break;
			case kViewCinematics:
				// TODO
				break;
			case kQuitGame: 
				// Nothing here
				break;
			case kTextOptions:
				// TODO
				break;
			case kAudioOptions:
				// TODO
				break;
			case kSettings:
				// TODO
				break;
			case kKeyboardConfig:
				// TODO
				break;
			case kShowCredits:
				// TODO if game finished then show resource image 33
				if (!_creditsResource)
					_creditsResource = new GraphicResource(_resPack, 24);
				if (!_creditsFadeResource)
					_creditsFadeResource = new GraphicResource(_resPack, 23);
                _creditsTextScroll = 0x1E0 - 30;
				// Set credits palette
				_screen->setPalette(_resPack, 26);
				break;
			case kReturnToGame:
				closeMenu();
				_gameState->enterScene();
				break;
		}
	}
}

void MainMenu::updateCursor() {
	_curMouseCursor += _cursorStep;
	if (_curMouseCursor == 0)
		_cursorStep = 1;
	if (_curMouseCursor == _cursorResource->getFrameCount() - 1)
		_cursorStep = -1;

	_screen->setCursor(_cursorResource, _curMouseCursor);
}

void MainMenu::updateEyesAnimation() {
	// Eyes animation
	// Get the appropriate eye resource depending on the mouse position
	int eyeFrameNum = kEyesFront;

	if (_mouseX <= 200) {
		if (_mouseY <= 160)
			eyeFrameNum = kEyesTopLeft;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeFrameNum = kEyesLeft;
		else
			eyeFrameNum = kEyesBottomLeft;
	} else if (_mouseX > 200 && _mouseX <= 400) {
		if (_mouseY <= 160)
			eyeFrameNum = kEyesTop;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeFrameNum = kEyesFront;
		else
			eyeFrameNum = kEyesBottom;
	} else if (_mouseX > 400) {
		if (_mouseY <= 160)
			eyeFrameNum = kEyesTopRight;
		else if (_mouseY > 160 && _mouseY <= 320)
			eyeFrameNum = kEyesRight;
		else
			eyeFrameNum = kEyesBottomRight;
	}
	// TODO: kEyesCrossed state

	GraphicFrame *eyeFrame = _eyeResource->getFrame(eyeFrameNum);
	_screen->copyRectToScreenWithTransparency((byte *)eyeFrame->surface.pixels, eyeFrame->x, eyeFrame->y, eyeFrame->surface.w, eyeFrame->surface.h);
}

void MainMenu::updateMainMenu() {
	int rowId = 0;

	if (_mouseY >= 20 && _mouseY <= 20 + 48) {
		rowId = 0; // Top row
	} else if (_mouseY >= 400 && _mouseY <= 400 + 48) {
		rowId = 1; // Bottom row
	} else {
		// No row selected
		_previousActiveIcon = _activeIcon = -1;
		_leftClick = false;
		return;
	}

	// Icon animation
	for (int i = 0; i <= 5; i++) {
		int curX = 40 + i * 100;
		if (_mouseX >= curX && _mouseX <= curX + 55) {
			int iconNum = i + 6 * rowId;
			_activeIcon = iconNum;

			// The last 2 icons are swapped
			if (iconNum == 11)
				iconNum = 10;
			else if (iconNum == 10)
				iconNum = 11;

			// Get the current icon animation
			if (!_iconResource || _activeIcon != _previousActiveIcon) {
				delete _iconResource;
				_iconResource = new GraphicResource(_resPack, iconNum + 4);
			}

			GraphicFrame *iconFrame = _iconResource->getFrame(MIN<int>(_iconResource->getFrameCount() - 1, _curIconFrame));
			_screen->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

			// Cycle icon frame
			_curIconFrame++;
			if (_curIconFrame >= _iconResource->getFrameCount())
				_curIconFrame = 0;

			// Show text
            _text->drawResTextCentered(MenuIconFixedXpos[iconNum], iconFrame->y + 50, _text->getResTextWidth(iconNum + 1309), iconNum + 1309);
			
			// Play creepy voice
			if (!_sound->isSfxActive() && _activeIcon != _previousActiveIcon) {
				_sound->playSfx(_resPack, iconNum + 44);
				_previousActiveIcon = _activeIcon;
			}

			break;
		}
	}
}

void MainMenu::updateSubMenu() {
	GraphicFrame *iconFrame = _iconResource->getFrame(MIN<int>(_iconResource->getFrameCount() - 1, _curIconFrame));
	_screen->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

	// Cycle icon frame
	_curIconFrame++;
	if (_curIconFrame >= _iconResource->getFrameCount())
		_curIconFrame = 0;

	switch (_activeIcon) {
		case kNewGame:
            updateSubMenuNewGame();
			break;
		case kLoadGame:
			// TODO
			break;
		case kSaveGame:
			// TODO
			break;
		case kDeleteGame:
			// TODO
			break;
		case kViewCinematics:
			// TODO
			break;
		case kQuitGame:
			updateSubMenuQuitGame();
			break;
		case kTextOptions:
			// TODO
			break;
		case kAudioOptions:
			// TODO
			break;
		case kSettings:
			// TODO
			break;
		case kKeyboardConfig:
			// TODO
			break;
		case kShowCredits:
			updateSubMenuShowCredits();
			break;
		case kReturnToGame:
			// TODO
			break;
	}
}

void MainMenu::updateSubMenuNewGame() {
    // begin new game
	_text->drawResTextCentered(10, 100, 620, 1321);	// 0x80000529u

	// Yes
	if (_mouseX >= 240 && _mouseX <= 280 && _mouseY >= 280 && _mouseY <= 300) {
		_textBlue->setTextPos(247, 273);
		_textBlue->drawResText(1322); // 0x8000052Au

		if (_leftClick) {
			_leftClick = false;

			// TODO handle new game event
		}
	} else {
		_text->setTextPos(247, 273);
		_text->drawResText(1322); // 0x8000052Au
	}

	// No
	if (_mouseX >= 360 && _mouseX <= 400 && _mouseY >= 280 && _mouseY <= 300) {
		_textBlue->setTextPos(369, 273);
		_textBlue->drawResText(1323); // 0x8000052Bu

		if (_leftClick)
			exitSubMenu();
	} else {
		_text->setTextPos(369, 273);
		_text->drawResText(1323); // 0x8000052Bu
	}
}

void MainMenu::updateSubMenuQuitGame() {
    // Quit confirmation
	_text->drawResTextCentered(10, 100, 620, 1408);	// 0x80000580u

	// Yes
	if (_mouseX >= 240 && _mouseX <= 280 && _mouseY >= 280 && _mouseY <= 300) {
		_textBlue->setTextPos(247, 273);
		_textBlue->drawResText(1409); // 0x80000581u

		if (_leftClick) {
			_leftClick = false;

			// User clicked on quit, so push a quit event
			Common::Event event;
			event.type = Common::EVENT_QUIT;
			g_system->getEventManager()->pushEvent(event);
		}
	} else {
		_text->setTextPos(247, 273);
		_text->drawResText(1409); // 0x80000581u
	}

	// No
	if (_mouseX >= 360 && _mouseX <= 400 && _mouseY >= 280 && _mouseY <= 300) {
		_textBlue->setTextPos(369, 273);
		_textBlue->drawResText(1410); // 0x80000582u

		if (_leftClick)
			exitSubMenu();
	} else {
		_text->setTextPos(369, 273);
		_text->drawResText(1410); // 0x80000582u
	}
}

void MainMenu::updateSubMenuShowCredits() {
    GraphicFrame *creditsFadeFrame = _creditsFadeResource->getFrame(0);
	_screen->copyRectToScreenWithTransparency((byte *)creditsFadeFrame->surface.pixels, creditsFadeFrame->x, creditsFadeFrame->y, creditsFadeFrame->surface.w, creditsFadeFrame->surface.h);

    GraphicFrame *creditsFrame = _creditsResource->getFrame(MIN<int>(_creditsResource->getFrameCount() - 1, _creditsBgFrame));
	_screen->copyRectToScreenWithTransparency((byte *)creditsFrame->surface.pixels, creditsFrame->x, creditsFrame->y, creditsFrame->surface.w, creditsFrame->surface.h);

	_creditsBgFrame++;
    if (_creditsBgFrame >= _creditsResource->getFrameCount())
		_creditsBgFrame = 0;

    int posY = _creditsTextScroll;
    int resId = 0;
    int step = 0;
    int minBound = 0;
    int maxBound = 0;
    do {
        if (posY + step >= 0) {
            if (posY + step > 450)
                break;

            minBound = posY + step + 24;
            if (minBound >= 0)
                if (minBound < 32)
                    posY = _creditsTextScroll;

            maxBound = posY + step;
            if (posY + step < 480)
                if (maxBound > 448)
                    posY = _creditsTextScroll;

            _text->setTextPos(320, step + posY);
            _text->drawResText(resId - 2147482201);
            posY = _creditsTextScroll;
        }
        step += 24;
        ++resId;
    } while (step < 0x21F0);

    _creditsTextScroll -= 2;

    // TODO: fade side text -> address 0041A400

	if (_leftClick) {
		// Restore palette
		_screen->setPalette(_resPack, 17);
        exitSubMenu();
	}
}

void MainMenu::exitSubMenu() {
	_leftClick = false;
	_activeMenuScreen = kMainMenu;

	// Copy the bright background to the back buffer
	GraphicFrame *bg = _bgResource->getFrame(1);
	_screen->copyToBackBuffer((byte *)bg->surface.pixels, 0, 0, bg->surface.w, bg->surface.h);

	// Set the cursor
	delete _cursorResource;
	_cursorResource = new GraphicResource(_resPack, 2);
	_screen->setCursor(_cursorResource, 0);
}

} // end of namespace Asylum
