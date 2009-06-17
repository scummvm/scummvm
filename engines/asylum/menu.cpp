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
#include "asylum/respack.h"
#include "asylum/graphics.h"

namespace Asylum {

/** This fixes the menu icons text x position on screen */
const int MenuIconFixedXpos[12] = { 28, 128, 225, 320, 410, 528, 16, 115, 237, 310, 508, 419 };

MainMenu::MainMenu(Screen *screen, Sound *sound, Scene *scene) :
	_screen(screen), _sound(sound), _scene(scene) {
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
    _confGammaLevel     = 2;
    _confGameQuality    = 5;

	_resPack        = new ResourcePack(1);
	_bgResource     = new GraphicResource(_resPack, 0);
	_eyeResource    = new GraphicResource(_resPack, 1);
	_cursorResource = new GraphicResource(_resPack, 2);

	_iconResource        = 0;
	_creditsResource     = 0;
	_creditsFadeResource = 0;

    _text = new Text(_screen);
    _text->loadFont(_resPack, 16);	// 0x80010010, yellow font
}

MainMenu::~MainMenu() {
	delete _creditsResource;
	delete _creditsFadeResource;
    delete _text;
	delete _iconResource;
	delete _eyeResource;
	delete _cursorResource;
	delete _bgResource;
	delete _resPack;
}

void MainMenu::openMenu() {
	_active = true;

    // yellow font
    _text->loadFont(_resPack, 0x80010010);

	// Load the graphics palette
	_screen->setPalette(_resPack, 17);
	// Copy the bright background to the back buffer
	GraphicFrame *bg = _bgResource->getFrame(1);
	_screen->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);

	// Set mouse cursor
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();

	// Stop all sounds
	_sound->stopMusic();
	_sound->stopSfx();

	// Start playing music
	_sound->playMusic(_resPack, 39);

	_previousActiveIcon = _activeIcon = -1;
	_leftClick = false;
	_activeMenuScreen = kMainMenu;
	_mouseX = _mouseY = 0;
}

void MainMenu::closeMenu() {
	_active = false;

	// Stop menu sounds and menu music
	_sound->stopSfx();
	_sound->stopMusic();
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
			_screen->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);
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
				// TODO if game finished (gameFlag=901) then show resource image 33 and music 56 and than music 40
				if (!_creditsResource)
					_creditsResource = new GraphicResource(_resPack, 24);
				if (!_creditsFadeResource)
					_creditsFadeResource = new GraphicResource(_resPack, 23);
                _creditsTextScroll = 0x1E0 - 30;
				// Set credits palette
				_screen->setPalette(_resPack, 26);
                // Stop all sounds
	            _sound->stopMusic();
	            // Start playing music
	            _sound->playMusic(_resPack, 38);
				break;
			case kReturnToGame:
				closeMenu();
				_scene->enterScene();
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
	_screen->copyRectToScreenWithTransparency((byte *)eyeFrame->surface.pixels, eyeFrame->surface.w, eyeFrame->x, eyeFrame->y, eyeFrame->surface.w, eyeFrame->surface.h);
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

    // yellow font
    _text->loadFont(_resPack, 0x80010010);

	// Icon animation
	for (uint32 i = 0; i <= 5; i++) {
		uint32 curX = 40 + i * 100;
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
			_screen->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->surface.w, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

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
	_screen->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->surface.w, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

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
			updateSubMenuSettings();
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
    // yellow font
    _text->loadFont(_resPack, 0x80010010);

    // begin new game
	_text->drawResTextCentered(10, 100, 620, 0x80000529);

	// Yes
    if (_mouseX < 247 || _mouseX > 247 + _text->getResTextWidth(0x8000052A) || _mouseY < 273 || _mouseY > 273 + 24 )
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(247, 273);
	_text->drawResText(0x8000052A);

    // No
    if (_mouseX < 369 || _mouseX > 369 + _text->getResTextWidth(0x8000052B) || _mouseY < 273 || _mouseY > 273 + 24 )
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(369, 273);
	_text->drawResText(0x8000052B);

    // action
    if (_leftClick) {
        // Yes
        if (_mouseX >= 247 && _mouseX <= 247 + 24 && _mouseY >= 273 && _mouseY <= 273 + _text->getResTextWidth(0x8000052A)) {
            _leftClick = false;
	        // TODO handle new game event
        }
		// No 
        if (_mouseX >= 369 && _mouseX <= 369 + 24 && _mouseY >= 273 && _mouseY <= 273 + _text->getResTextWidth(0x8000052B))
            exitSubMenu();
    }
}

void MainMenu::updateSubMenuSettings() {
    uint32 sizeMinus    = _text->getTextWidth("-");
    uint32 sizePlus     = _text->getTextWidth("+");
    uint32 sizeMainMenu = _text->getResTextWidth(0x8000059D);

    // yellow font
    _text->loadFont(_resPack, 0x80010010);
    // Settings
	_text->drawResTextCentered(10, 100, 620, 0x80000598);

    // gamma correction
    _text->drawResTextAlignRight(320, 150, 0x80000599);
    if (_mouseX < 350 || _mouseX > sizeMinus + 350 || _mouseY < 150 || _mouseY > 174)
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else        
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(350, 150);
    _text->drawText("-");

    if (_mouseX < sizeMinus + 360 || _mouseX > sizeMinus + sizePlus + 360 || _mouseY < 150 || _mouseY > 174)
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else        
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(sizeMinus + 360, 150);
    _text->drawText("+");

    _text->setTextPos(sizeMinus + sizePlus + 365, 150);
    _text->loadFont(_resPack, 0x80010010);
    if(_confGammaLevel) {
        for (uint32 i = 0; i < _confGammaLevel; i++ ) {
          _text->drawText("]");
        }
        if (_confGammaLevel == 8)
            _text->drawText("*");
    } else
        _text->drawResText(0x8000059B);

    // performance
    _text->loadFont(_resPack, 0x80010010);
    _text->drawResTextAlignRight(320, 179, 0x8000059A);
    if (_mouseX < 350 || _mouseX > sizeMinus + 350 || _mouseY < 179 || _mouseY > 203)
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else        
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(350, 179);
    _text->drawText("-");

    if (_mouseX < sizeMinus + 360 || _mouseX > sizeMinus + sizePlus + 360 || _mouseY < 179 || _mouseY > 203)
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else        
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(sizeMinus + 360, 179);
    _text->drawText("+");

    _text->setTextPos(sizeMinus + sizePlus + 365, 179);
    _text->loadFont(_resPack, 0x80010010);
    if(_confGameQuality == 5) {
        _text->drawResText(0x8000059C);
    } else {
        for (uint32 i = 5; i > _confGameQuality; --i ) {
          _text->drawText("]");
        }
        if (!_confGameQuality)
            _text->drawText("*");
    }

    // back to main menu
    if (_mouseX < 300 || _mouseX > 300 + sizeMainMenu || _mouseY < 340 || _mouseY > 340 + 24)
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else        
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(300, 340);
    _text->drawResText(0x8000059D);

    // action
    if (_leftClick) {
        // back to main menu
        if (_mouseX >= 300 && _mouseX <= 300 + sizeMainMenu && _mouseY >= 340 && _mouseY <= 340 + 24) {
            // TODO: save new configurations
            exitSubMenu();
        }

        // gamma level minus
        if (_mouseX >= 350 && _mouseX <= sizeMinus + 350 && _mouseY >= 150 && _mouseY <= 174) {
            if(_confGammaLevel) {
                _confGammaLevel -= 1;
                // TODO: setResGammaLevel(0x80010011, 0);
            }
        }
        // gamma level plus
        if (_mouseX >= sizeMinus + 360 && _mouseX <= sizeMinus + sizePlus + 360 && _mouseY >= 150 && _mouseY <= 174) {
            if(_confGammaLevel < 8) {
                _confGammaLevel += 1;
                // TODO: setResGammaLevel(0x80010011, 0);
            }
        }

        // performance minus
        if (_mouseX >= 350 && _mouseX <= sizeMinus + 350 && _mouseY >= 179 && _mouseY <= 203) {
            if(_confGameQuality) {
                _confGameQuality -= 1;
                // TODO: change quality settings
            }
        }
        // performance plus
        if (_mouseX >= sizeMinus + 360 && _mouseX <= sizeMinus + sizePlus + 360 && _mouseY >= 179 && _mouseY <= 203) {
            if(_confGameQuality < 5) {
                _confGameQuality += 1;
                // TODO: change quality settings
            }
        }
    }
}

void MainMenu::updateSubMenuQuitGame() {
    // yellow font
    _text->loadFont(_resPack, 0x80010010);

    // begin new game
	_text->drawResTextCentered(10, 100, 620, 0x80000580);

	// Yes
    if (_mouseX < 247 || _mouseX > 247 + _text->getResTextWidth(0x80000581) || _mouseY < 273 || _mouseY > 273 + 24 )
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(247, 273);
	_text->drawResText(0x80000581);

    // No
    if (_mouseX < 369 || _mouseX > 369 + _text->getResTextWidth(0x80000582) || _mouseY < 273 || _mouseY > 273 + 24 )
        _text->loadFont(_resPack, 0x80010010); // yellow font
    else
        _text->loadFont(_resPack, 0x80010016); // blue font
    _text->setTextPos(369, 273);
	_text->drawResText(0x80000582);

    // action
    if (_leftClick) {
        // Yes
        if (_mouseX >= 247 && _mouseX <= 247 + 24 && _mouseY >= 273 && _mouseY <= 273 + _text->getResTextWidth(0x80000581)) {
		    _leftClick = false;

			// User clicked on quit, so push a quit event
			Common::Event event;
			event.type = Common::EVENT_QUIT;
			g_system->getEventManager()->pushEvent(event);
        }
        // No
        if (_mouseX >= 369 && _mouseX <= 369 + 24 && _mouseY >= 273 && _mouseY <= 273 + _text->getResTextWidth(0x80000582))
            exitSubMenu();
    }
}

void MainMenu::updateSubMenuShowCredits() {
    int posY     = _creditsTextScroll;
    int resId    = 0;
    int step     = 0;
    int minBound = 0;
    int maxBound = 0;

    GraphicFrame *creditsFadeFrame = _creditsFadeResource->getFrame(0);
	_screen->copyRectToScreenWithTransparency((byte *)creditsFadeFrame->surface.pixels, creditsFadeFrame->surface.w, creditsFadeFrame->x, creditsFadeFrame->y, creditsFadeFrame->surface.w, creditsFadeFrame->surface.h);

    GraphicFrame *creditsFrame = _creditsResource->getFrame(MIN<int>(_creditsResource->getFrameCount() - 1, _creditsBgFrame));
	_screen->copyRectToScreenWithTransparency((byte *)creditsFrame->surface.pixels, creditsFrame->surface.w, creditsFrame->x, creditsFrame->y, creditsFrame->surface.w, creditsFrame->surface.h);

	_creditsBgFrame++;
    if (_creditsBgFrame >= _creditsResource->getFrameCount())
		_creditsBgFrame = 0;

    do {
        if (posY + step >= 0) {
            if (posY + step > 450)
                break;

            minBound = posY + step + 24;
            if (minBound >= 0)
                if (minBound < 32) {
                    // TODO fade side text
                    posY = _creditsTextScroll;
                }

            maxBound = posY + step;
            if (posY + step < 480)
                if (maxBound > 448) {
                    // TODO fade side text
                    posY = _creditsTextScroll;
                }

            _text->setTextPos(320, step + posY);
            _text->drawResText(resId - 2147482201);
            posY = _creditsTextScroll;
        }
        step += 24;
        ++resId;
    } while (step < 0x21F0);

    _creditsTextScroll -= 2;

    // TODO: some palette stuffs
    
    // TODO: if gameFlag=901 (finished game) and already play music 56, start music 40

	if (_leftClick) {
		// Restore palette
		_screen->setPalette(_resPack, 17);
        // Stop all sounds
	    _sound->stopMusic();
	    // Start playing music
	    _sound->playMusic(_resPack, 39);
        exitSubMenu();
	}
}

void MainMenu::exitSubMenu() {
	_leftClick = false;
	_activeMenuScreen = kMainMenu;

	// Copy the bright background to the back buffer
	GraphicFrame *bg = _bgResource->getFrame(1);
	_screen->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);

	// Set the cursor
	delete _cursorResource;
	_cursorResource = new GraphicResource(_resPack, 2);
	_screen->setCursor(_cursorResource, 0);
}

} // end of namespace Asylum
