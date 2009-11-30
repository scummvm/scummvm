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

#include "asylum/menu.h"
#include "asylum/respack.h"
#include "asylum/graphics.h"
#include "asylum/config.h"

namespace Asylum {

MainMenu::MainMenu(AsylumEngine *vm): _vm(vm) {
	_leftClick			= false;
	_activeIcon			= -1;
	_previousActiveIcon = -1;
	_curIconFrame		= 0;
	_curMouseCursor		= 0;
	_cursorStep			= 1;
	_creditsBgFrame		= 0;
	_creditsTextScroll	= 0x1E0 - 30;
	_activeMenuScreen	= kMainMenu;
	_active				= false;

	Config.gammaLevel  = 2;
	Config.performance = 5;

	_resPack		= new ResourcePack(1);
	_bgResource		= new GraphicResource(_resPack, 0);
	_eyeResource	= new GraphicResource(_resPack, 1);

	_cursor = new Cursor(_resPack);

	_iconResource		 = 0;
	_creditsResource	 = 0;
	_creditsFadeResource = 0;

	loadFont(kFontYellow);
}

MainMenu::~MainMenu() {
	delete _creditsResource;
	delete _creditsFadeResource;
	delete _iconResource;
	delete _eyeResource;
	delete _cursor;
	delete _bgResource;
	delete _resPack;
}

void MainMenu::loadFont(Fonts font) {
	switch (font) {
	case kFontBlue:
		_vm->text()->loadFont(_resPack, 0x80010016);
		break;
	case kFontYellow:
		_vm->text()->loadFont(_resPack, 0x80010010);
		break;
	default:
		error("Unknown Font Colour Specified");
	}
}

void MainMenu::switchFont(bool condition) {
	if (condition)
		loadFont(kFontYellow);
	else
		loadFont(kFontBlue);
}

void MainMenu::openMenu() {
	_active = true;
	_vm->scene()->deactivate();

	loadFont(kFontYellow);

	// Load the graphics palette
	_vm->screen()->setPalette(_resPack, 17);
	// Copy the bright background to the back buffer
	GraphicFrame *bg = _bgResource->getFrame(1);
	_vm->screen()->copyToBackBuffer((byte *)bg->surface.pixels,
	                                bg->surface.w,
	                                0,
	                                0,
	                                bg->surface.w,
	                                bg->surface.h);

	// Set mouse cursor
	_cursor->load(2);
	_cursor->show();

	// Stop all sounds
	_vm->sound()->stopMusic();
	_vm->sound()->stopSound();

	// Start playing music
	_vm->sound()->playMusic(_resPack, 39);

	_previousActiveIcon = _activeIcon = -1;
	_leftClick = false;
	_activeMenuScreen = kMainMenu;
}

void MainMenu::closeMenu() {
	_active = false;
	_vm->scene()->activate();

	// Stop menu sounds and menu music
	_vm->sound()->stopSound();
	_vm->sound()->stopMusic();
}

void MainMenu::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {
	case Common::EVENT_MOUSEMOVE:
		_cursor->setCoords(_ev->mouse.x, _ev->mouse.y);
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
	_cursor->animate();

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
			_vm->screen()->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);
			_activeMenuScreen = (MenuScreen) _activeIcon;

			// Set the cursor
			_cursor->load(3);
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
			_vm->screen()->setPalette(_resPack, 26);
			// Stop all sounds
			_vm->sound()->stopMusic();
			// Start playing music
			_vm->sound()->playMusic(_resPack, 38);
			break;
		case kReturnToGame:
			closeMenu();
			_vm->scene()->enterScene();
			break;
		}
	}
}

void MainMenu::updateEyesAnimation() {
	// Eyes animation
	// Get the appropriate eye resource depending on the mouse position
	int eyeFrameNum = kEyesFront;

	if (_cursor->x() <= 200) {
		if (_cursor->y() <= 160)
			eyeFrameNum = kEyesTopLeft;
		else if (_cursor->y() > 160 && _cursor->y() <= 320)
			eyeFrameNum = kEyesLeft;
		else
			eyeFrameNum = kEyesBottomLeft;
	} else if (_cursor->x() > 200 && _cursor->x() <= 400) {
		if (_cursor->y() <= 160)
			eyeFrameNum = kEyesTop;
		else if (_cursor->y() > 160 && _cursor->y() <= 320)
			eyeFrameNum = kEyesFront;
		else
			eyeFrameNum = kEyesBottom;
	} else if (_cursor->x() > 400) {
		if (_cursor->y() <= 160)
			eyeFrameNum = kEyesTopRight;
		else if (_cursor->y() > 160 && _cursor->y() <= 320)
			eyeFrameNum = kEyesRight;
		else
			eyeFrameNum = kEyesBottomRight;
	}
	// TODO: kEyesCrossed state

	GraphicFrame *eyeFrame = _eyeResource->getFrame(eyeFrameNum);
	_vm->screen()->copyRectToScreenWithTransparency((byte *)eyeFrame->surface.pixels, eyeFrame->surface.w, eyeFrame->x, eyeFrame->y, eyeFrame->surface.w, eyeFrame->surface.h);
}

void MainMenu::updateMainMenu() {
	int rowId = 0;

	if (_cursor->y() >= 20 && _cursor->y() <= 20 + 48) {
		rowId = 0; // Top row
	} else if (_cursor->y() >= 400 && _cursor->y() <= 400 + 48) {
		rowId = 1; // Bottom row
	} else {
		// No row selected
		_previousActiveIcon = _activeIcon = -1;
		_leftClick = false;
		return;
	}

	loadFont(kFontYellow);

	// Icon animation
	for (uint32 i = 0; i <= 5; i++) {
		uint32 curX = 40 + i * 100;
		if (_cursor->x() >= curX && _cursor->x() <= curX + 55) {
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
			_vm->screen()->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->surface.w, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

			// Cycle icon frame
			_curIconFrame++;
			if (_curIconFrame >= _iconResource->getFrameCount())
				_curIconFrame = 0;

			// Show text
			_vm->text()->drawResTextCentered(MenuIconFixedXpos[iconNum],
					iconFrame->y + 50,
					_vm->text()->getResTextWidth(iconNum + 1309),
					iconNum + 1309);

			// Play creepy voice
			if (_activeIcon != _previousActiveIcon) {
				_vm->sound()->playSound(_resPack, iconNum + 44, Config.sfxVolume);
				_previousActiveIcon = _activeIcon;
			}

			break;
		}
	}
}

void MainMenu::updateSubMenu() {
	GraphicFrame *iconFrame = _iconResource->getFrame(MIN<int>(_iconResource->getFrameCount() - 1, _curIconFrame));
	_vm->screen()->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->surface.w, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

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
		updateSubMenuCinematics();
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
	loadFont(kFontYellow);

	// begin new game
	_vm->text()->drawResTextCentered(10, 100, 620, 0x80000529);

	// Yes
	switchFont(_cursor->x() < 247 || _cursor->x() > 247 + _vm->text()->getResTextWidth(0x8000052A) || _cursor->y() < 273 || _cursor->y() > 273 + 24);
	_vm->text()->setTextPos(247, 273);
	_vm->text()->drawResText(0x8000052A);

	// No
	switchFont(_cursor->x() < 369 || _cursor->x() > 369 + _vm->text()->getResTextWidth(0x8000052B) || _cursor->y() < 273 ||	_cursor->y() > 273 + 24);
	_vm->text()->setTextPos(369, 273);
	_vm->text()->drawResText(0x8000052B);

	// action
	if (_leftClick) {
		// Yes
		if (_cursor->x() >= 247 && _cursor->x() <= 247 + 24 && _cursor->y() >= 273 && _cursor->y() <= 273 + _vm->text()->getResTextWidth(0x8000052A)) {
			_leftClick = false;
			// TODO handle new game event
		}
		// No
		if (_cursor->x() >= 369 && _cursor->x() <= 369 + 24 && _cursor->y() >= 273 && _cursor->y() <= 273 + _vm->text()->getResTextWidth(0x8000052B))
			exitSubMenu();
	}
}

void MainMenu::updateSubMenuCinematics() {
	int currentCD = 1;	// FIXME: dummy value
	_vm->text()->drawResTextWithValueCentered(10, 100, 620, 0x80000548, currentCD);
	_vm->text()->setTextPos(30, 340);
	_vm->text()->drawResText(0x80000549);	// Prev Page

	if (_cursor->x() >= 280 && _cursor->x() <= 400 && _cursor->y() >= 340 && _cursor->y() <= 360) {
		loadFont(kFontBlue);
		if (_leftClick)
			exitSubMenu();
	} else {
		loadFont(kFontYellow);
	}

	_vm->text()->setTextPos(280, 340);
	_vm->text()->drawResText(0x8000054B);	// Main Menu

	loadFont(kFontYellow);

	_vm->text()->setTextPos(500, 340);
	_vm->text()->drawResText(0x8000054A);	// Next Page
}

void MainMenu::updateSubMenuSettings() {
	uint32 sizeMinus	= _vm->text()->getTextWidth("-");
	uint32 sizePlus		= _vm->text()->getTextWidth("+");
	uint32 sizeMainMenu = _vm->text()->getResTextWidth(0x8000059D);

	loadFont(kFontYellow);
	// Settings
	_vm->text()->drawResTextCentered(10, 100, 620, 0x80000598);

	// gamma correction
	_vm->text()->drawResTextAlignRight(320, 150, 0x80000599);
	switchFont(_cursor->x() < 350 || _cursor->x() > sizeMinus + 350 || _cursor->y() < 150 || _cursor->y() > 174);
	_vm->text()->setTextPos(350, 150);
	_vm->text()->drawText("-");

	switchFont(_cursor->x() < sizeMinus + 360 || _cursor->x() > sizeMinus + sizePlus + 360 || _cursor->y() < 150 || _cursor->y() > 174);
	_vm->text()->setTextPos(sizeMinus + 360, 150);
	_vm->text()->drawText("+");

	_vm->text()->setTextPos(sizeMinus + sizePlus + 365, 150);
	loadFont(kFontYellow);
	if (Config.gammaLevel) {
		for (int32 i = 0; i < Config.gammaLevel; i++) {
			_vm->text()->drawText("]");
		}
		if (Config.gammaLevel == 8)
			_vm->text()->drawText("*");
	} else
		_vm->text()->drawResText(0x8000059B);

	// performance
	loadFont(kFontYellow);
	_vm->text()->drawResTextAlignRight(320, 179, 0x8000059A);
	switchFont(_cursor->x() < 350 || _cursor->x() > sizeMinus + 350 || _cursor->y() < 179 || _cursor->y() > 203);
	_vm->text()->setTextPos(350, 179);
	_vm->text()->drawText("-");

	switchFont(_cursor->x() < sizeMinus + 360 || _cursor->x() > sizeMinus + sizePlus + 360 || _cursor->y() < 179 || _cursor->y() > 203);
	_vm->text()->setTextPos(sizeMinus + 360, 179);
	_vm->text()->drawText("+");

	_vm->text()->setTextPos(sizeMinus + sizePlus + 365, 179);
	loadFont(kFontYellow);
	if (Config.performance == 5) {
		_vm->text()->drawResText(0x8000059C);
	} else {
		for (int32 i = 5; i > Config.performance; --i) {
			_vm->text()->drawText("]");
		}
		if (!Config.performance)
			_vm->text()->drawText("*");
	}

	// back to main menu
	switchFont(_cursor->x() < 300 || _cursor->x() > 300 + sizeMainMenu || _cursor->y() < 340 || _cursor->y() > 340 + 24);
	_vm->text()->setTextPos(300, 340);
	_vm->text()->drawResText(0x8000059D);

	// action
	if (_leftClick) {
		// back to main menu
		if (_cursor->x() >= 300 && _cursor->x() <= 300 + sizeMainMenu && _cursor->y() >= 340 && _cursor->y() <= 340 + 24) {
			// TODO: save new configurations
			exitSubMenu();
		}

		// gamma level minus
		if (_cursor->x() >= 350 && _cursor->x() <= sizeMinus + 350 && _cursor->y() >= 150 && _cursor->y() <= 174) {
			if (Config.gammaLevel) {
				Config.gammaLevel -= 1;
				// TODO: setResGammaLevel(0x80010011, 0);
			}
		}
		// gamma level plus
		if (_cursor->x() >= sizeMinus + 360 && _cursor->x() <= sizeMinus + sizePlus + 360 && _cursor->y() >= 150 && _cursor->y() <= 174) {
			if (Config.gammaLevel < 8) {
				Config.gammaLevel += 1;
				// TODO: setResGammaLevel(0x80010011, 0);
			}
		}

		// performance minus
		if (_cursor->x() >= 350 && _cursor->x() <= sizeMinus + 350 && _cursor->y() >= 179 && _cursor->y() <= 203) {
			if (Config.performance) {
				Config.performance -= 1;
				// TODO: change quality settings
			}
		}
		// performance plus
		if (_cursor->x() >= sizeMinus + 360 && _cursor->x() <= sizeMinus + sizePlus + 360 && _cursor->y() >= 179 && _cursor->y() <= 203) {
			if (Config.performance < 5) {
				Config.performance += 1;
				// TODO: change quality settings
			}
		}
	}
}

void MainMenu::updateSubMenuQuitGame() {
	loadFont(kFontYellow);

	// begin new game
	_vm->text()->drawResTextCentered(10, 100, 620, 0x80000580);

	// Yes
	switchFont(_cursor->x() < 247 || _cursor->x() > 247 + _vm->text()->getResTextWidth(0x80000581) || _cursor->y() < 273 || _cursor->y() > 273 + 24);
	_vm->text()->setTextPos(247, 273);
	_vm->text()->drawResText(0x80000581);

	// No
	switchFont(_cursor->x() < 369 || _cursor->x() > 369 + _vm->text()->getResTextWidth(0x80000582) || _cursor->y() < 273 || _cursor->y() > 273 + 24);
	_vm->text()->setTextPos(369, 273);
	_vm->text()->drawResText(0x80000582);

	// action
	if (_leftClick) {
		// Yes
		if (_cursor->x() >= 247 && _cursor->x() <= 247 + 24 && _cursor->y() >= 273 && _cursor->y() <= 273 + _vm->text()->getResTextWidth(0x80000581)) {
			_leftClick = false;

			// User clicked on quit, so push a quit event
			Common::Event event;
			event.type = Common::EVENT_QUIT;
			g_system->getEventManager()->pushEvent(event);
		}
		// No
		if (_cursor->x() >= 369 && _cursor->x() <= 369 + 24 && _cursor->y() >= 273 && _cursor->y() <= 273 + _vm->text()->getResTextWidth(0x80000582))
			exitSubMenu();
	}
}

void MainMenu::updateSubMenuShowCredits() {
	int posY	 = _creditsTextScroll;
	int resId	 = 0;
	int step	 = 0;
	int minBound = 0;
	int maxBound = 0;

	GraphicFrame *creditsFadeFrame = _creditsFadeResource->getFrame(0);
	_vm->screen()->copyRectToScreenWithTransparency((byte *)creditsFadeFrame->surface.pixels, creditsFadeFrame->surface.w, creditsFadeFrame->x, creditsFadeFrame->y, creditsFadeFrame->surface.w, creditsFadeFrame->surface.h);

	GraphicFrame *creditsFrame = _creditsResource->getFrame(MIN<int>(_creditsResource->getFrameCount() - 1, _creditsBgFrame));
	_vm->screen()->copyRectToScreenWithTransparency((byte *)creditsFrame->surface.pixels, creditsFrame->surface.w, creditsFrame->x, creditsFrame->y, creditsFrame->surface.w, creditsFrame->surface.h);

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

			_vm->text()->setTextPos(320, step + posY);
			_vm->text()->drawResText(resId - 2147482201);
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
		_vm->screen()->setPalette(_resPack, 17);
		// Stop all sounds
		_vm->sound()->stopMusic();
		// Start playing music
		_vm->sound()->playMusic(_resPack, 39);
		exitSubMenu();
	}
}

void MainMenu::exitSubMenu() {
	_leftClick = false;
	_activeMenuScreen = kMainMenu;

	// Copy the bright background to the back buffer
	GraphicFrame *bg = _bgResource->getFrame(1);
	_vm->screen()->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);

	// Set the cursor
	_cursor->load(2);
}

} // end of namespace Asylum
