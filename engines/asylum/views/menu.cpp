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

#include "asylum/views/menu.h"

#include "asylum/system/config.h"
#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/respack.h"
#include "asylum/staticres.h"

#include "asylum/asylum.h"

namespace Asylum {

MainMenu::MainMenu(AsylumEngine *vm): _vm(vm) {
	_leftClick          = false;
	_activeIcon         = -1;
	_previousActiveIcon = -1;
	_curIconFrame       = 0;
	_curMouseCursor     = 0;
	_cursorStep         = 1;
	_creditsBgFrame     = 0;
	_creditsTextScroll  = 0x1E0 - 30;
	_activeMenuScreen   = kMainMenu;
	_active             = false;

	Config.gammaLevel  = 2;
	Config.performance = 5;

	_bgResource  = new GraphicResource(_vm, MAKE_RESOURCE(kResourcePackShared, 0));
	_eyeResource = new GraphicResource(_vm, MAKE_RESOURCE(kResourcePackShared, 1));

	_ev = NULL;

	_cursor = new Cursor(_vm);

	_iconResource        = 0;
	_creditsResource     = 0;
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

	// Zero-out passed pointers
	_ev = NULL;
	_vm = NULL;
}

void MainMenu::loadFont(Fonts font) {
	switch (font) {
	default:
		error("[MainMenu::loadFont] Unknown font color specified!");

	case kFontBlue:
		getText()->loadFont(MAKE_RESOURCE(kResourcePackShared, 22));
		break;

	case kFontYellow:
		getText()->loadFont(MAKE_RESOURCE(kResourcePackShared, 16));
		break;
	}
}

void MainMenu::switchFont(bool condition) {
	loadFont((condition) ? kFontYellow : kFontBlue);
}

void MainMenu::openMenu() {
	_active = true;

	if (_vm->scene()) {
		_vm->scene()->deactivate();
	}

	loadFont(kFontYellow);

	// Load the graphics palette
	getScreen()->setPalette(MAKE_RESOURCE(kResourcePackShared, 17));
	getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 0), 1, 0, 0, 0);

	// Set mouse cursor
	_cursor->set(MAKE_RESOURCE(kResourcePackShared, 2));
	_cursor->show();

	// Stop all sounds
	_vm->sound()->stopMusic();
	_vm->sound()->stopAll();

	// Start playing music
	_vm->sound()->playMusic(MAKE_RESOURCE(kResourcePackShared, 39));

	_previousActiveIcon = _activeIcon = -1;
	_leftClick = false;
	_activeMenuScreen = kMainMenu;
}

void MainMenu::closeMenu() {
	_active = false;
    if (_vm->scene()) {
	    _vm->scene()->activate();
    }

	// Stop menu sounds and menu music
	_vm->sound()->stopAll();
	_vm->sound()->stopMusic();
}

void MainMenu::showOptions() {
	error("[MainMenu::showOptions] : not implemented!");
}

void MainMenu::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {
	case Common::EVENT_MOUSEMOVE:
		_cursor->move(_ev->mouse.x, _ev->mouse.y);
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
			getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 0), 0, 0, 0, 0);

			_activeMenuScreen = (MenuScreen) _activeIcon;

			// Set the cursor
			_cursor->set(MAKE_RESOURCE(kResourcePackShared, 3));
		}

		switch (_activeIcon) {
		default:
			break;

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
				_creditsResource = new GraphicResource(_vm, MAKE_RESOURCE(kResourcePackShared, 24));
			if (!_creditsFadeResource)
				_creditsFadeResource = new GraphicResource(_vm, MAKE_RESOURCE(kResourcePackShared, 23));
			_creditsTextScroll = 0x1E0 - 30;
			// Set credits palette
			getScreen()->setPalette(MAKE_RESOURCE(kResourcePackShared, 26));
			// Stop all sounds
			getSound()->stopMusic();
			// Start playing music
			getSound()->playMusic(MAKE_RESOURCE(kResourcePackShared, 38));
			break;
		case kReturnToGame:
			if (_vm->scene()) // FIXME: do this properly
				closeMenu();
			break;
		}
	}
}

void MainMenu::updateEyesAnimation() {
	// Eyes animation
	// Get the appropriate eye resource depending on the mouse position
	uint32 eyeFrameNum = kEyesFront;

	if (_cursor->position().x <= 200) {
		if (_cursor->position().y <= 160)
			eyeFrameNum = kEyesTopLeft;
		else if (_cursor->position().y > 160 && _cursor->position().y <= 320)
			eyeFrameNum = kEyesLeft;
		else
			eyeFrameNum = kEyesBottomLeft;
	} else if (_cursor->position().x > 200 && _cursor->position().x <= 400) {
		if (_cursor->position().y <= 160)
			eyeFrameNum = kEyesTop;
		else if (_cursor->position().y > 160 && _cursor->position().y <= 320)
			eyeFrameNum = kEyesFront;
		else
			eyeFrameNum = kEyesBottom;
	} else if (_cursor->position().x > 400) {
		if (_cursor->position().y <= 160)
			eyeFrameNum = kEyesTopRight;
		else if (_cursor->position().y > 160 && _cursor->position().y <= 320)
			eyeFrameNum = kEyesRight;
		else
			eyeFrameNum = kEyesBottomRight;
	}
	// TODO: kEyesCrossed state

	GraphicFrame *eyeFrame = _eyeResource->getFrame(eyeFrameNum);
	_vm->screen()->copyRectToScreenWithTransparency((byte *)eyeFrame->surface.pixels, eyeFrame->surface.w, eyeFrame->x, eyeFrame->y, eyeFrame->surface.w, eyeFrame->surface.h);
}

void MainMenu::updateMainMenu() {
	int32 rowId = 0;

	if (_cursor->position().y >= 20 && _cursor->position().y <= 20 + 48) {
		rowId = 0; // Top row
	} else if (_cursor->position().y >= 400 && _cursor->position().y <= 400 + 48) {
		rowId = 1; // Bottom row
	} else {
		// No row selected
		_previousActiveIcon = _activeIcon = -1;
		_leftClick = false;
		return;
	}

	loadFont(kFontYellow);

	// Icon animation
	for (int32 i = 0; i <= 5; i++) {
		int32 curX = 40 + i * 100;
		if (_cursor->position().x >= curX && _cursor->position().x <= curX + 55) {
			int32 iconNum = i + 6 * rowId;
			_activeIcon = iconNum;

			// The last 2 icons are swapped
			if (iconNum == 11)
				iconNum = 10;
			else if (iconNum == 10)
				iconNum = 11;

			// Get the current icon animation
			if (!_iconResource || _activeIcon != _previousActiveIcon) {
				delete _iconResource;
				_iconResource = new GraphicResource(_vm, MAKE_RESOURCE(kResourcePackShared, iconNum + 4));
			}

			GraphicFrame *iconFrame = _iconResource->getFrame(MIN<uint>(_iconResource->count() - 1, _curIconFrame));
			_vm->screen()->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->surface.w, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

			// Cycle icon frame
			_curIconFrame++;
			if (_curIconFrame >= _iconResource->count())
				_curIconFrame = 0;

			// Show text
			_vm->text()->drawCentered(MenuIconFixedXpos[iconNum],
					iconFrame->y + 50,
					getText()->getWidth(MAKE_RESOURCE(kResourcePackText, iconNum + 1309)),
					MAKE_RESOURCE(kResourcePackText, iconNum + 1309));

			// Play creepy voice
			if (_activeIcon != _previousActiveIcon) {
				getSound()->playSound(MAKE_RESOURCE(kResourcePackShared, iconNum + 44), false, Config.sfxVolume);
				_previousActiveIcon = _activeIcon;
			}

			break;
		}
	}
}

void MainMenu::updateSubMenu() {
	GraphicFrame *iconFrame = _iconResource->getFrame(MIN<uint>(_iconResource->count() - 1, _curIconFrame));
	_vm->screen()->copyRectToScreenWithTransparency((byte *)iconFrame->surface.pixels, iconFrame->surface.w, iconFrame->x, iconFrame->y, iconFrame->surface.w, iconFrame->surface.h);

	// Cycle icon frame
	_curIconFrame++;
	if (_curIconFrame >= _iconResource->count())
		_curIconFrame = 0;

	switch (_activeIcon) {
	default:
		break;

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
		updateSubMenuReturnToGame();
		break;
	}
}

void MainMenu::updateSubMenuNewGame() {
	loadFont(kFontYellow);

	// begin new game
	_vm->text()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 529));

	// Yes
	switchFont(_cursor->position().x < 247 || _cursor->position().x > 247 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 1322)) || _cursor->position().y < 273 || _cursor->position().y > 273 + 24);
	_vm->text()->setPosition(247, 273);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1322));

	// No
	switchFont(_cursor->position().x < 369 || _cursor->position().x > 369 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 1323)) || _cursor->position().y < 273 ||	_cursor->position().y > 273 + 24);
	_vm->text()->setPosition(369, 273);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1323));

	// action
	if (_leftClick) {
		// Yes
		if (_cursor->position().x >= 247 && _cursor->position().x <= 247 + 24 && _cursor->position().y >= 273 && _cursor->position().y <= 273 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 1322))) {
			_leftClick = false;
            closeMenu();
            _vm->startGame(kResourcePackTowerCells, AsylumEngine::kStartGamePlayIntro);
		}
		// No
		if (_cursor->position().x >= 369 && _cursor->position().x <= 369 + 24 && _cursor->position().y >= 273 && _cursor->position().y <= 273 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 1323)))
			exitSubMenu();
	}
}

void MainMenu::updateSubMenuCinematics() {
	int32 currentCD = 1;	// FIXME: dummy value
	_vm->text()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1352), currentCD);
	_vm->text()->setPosition(30, 340);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1353));	// Prev Page

	if (_cursor->position().x >= 280 && _cursor->position().x <= 400 && _cursor->position().y >= 340 && _cursor->position().y <= 360) {
		loadFont(kFontBlue);
		if (_leftClick)
			exitSubMenu();
	} else {
		loadFont(kFontYellow);
	}

	_vm->text()->setPosition(280, 340);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1355));	// Main Menu

	loadFont(kFontYellow);

	_vm->text()->setPosition(500, 340);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1354));	// Next Page
}

void MainMenu::updateSubMenuSettings() {
	int32 sizeMinus	= _vm->text()->getWidth("-");
	int32 sizePlus		= _vm->text()->getWidth("+");
	int32 sizeMainMenu = _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 1437));

	loadFont(kFontYellow);
	// Settings
	_vm->text()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1432));

	// gamma correction
	_vm->text()->drawAlignedRight(320, 150, MAKE_RESOURCE(kResourcePackText, 1433));
	switchFont(_cursor->position().x < 350 || _cursor->position().x > sizeMinus + 350 || _cursor->position().y < 150 || _cursor->position().y > 174);
	_vm->text()->setPosition(350, 150);
	_vm->text()->draw("-");

	switchFont(_cursor->position().x < sizeMinus + 360 || _cursor->position().x > sizeMinus + sizePlus + 360 || _cursor->position().y < 150 || _cursor->position().y > 174);
	_vm->text()->setPosition(sizeMinus + 360, 150);
	_vm->text()->draw("+");

	_vm->text()->setPosition(sizeMinus + sizePlus + 365, 150);
	loadFont(kFontYellow);
	if (Config.gammaLevel) {
		for (int32 i = 0; i < Config.gammaLevel; i++) {
			_vm->text()->draw("]");
		}
		if (Config.gammaLevel == 8)
			_vm->text()->draw("*");
	} else
		_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1435));

	// performance
	loadFont(kFontYellow);
	_vm->text()->drawAlignedRight(320, 179, MAKE_RESOURCE(kResourcePackText, 1434));
	switchFont(_cursor->position().x < 350 || _cursor->position().x > sizeMinus + 350 || _cursor->position().y < 179 || _cursor->position().y > 203);
	_vm->text()->setPosition(350, 179);
	_vm->text()->draw("-");

	switchFont(_cursor->position().x < sizeMinus + 360 || _cursor->position().x > sizeMinus + sizePlus + 360 || _cursor->position().y < 179 || _cursor->position().y > 203);
	_vm->text()->setPosition(sizeMinus + 360, 179);
	_vm->text()->draw("+");

	_vm->text()->setPosition(sizeMinus + sizePlus + 365, 179);
	loadFont(kFontYellow);
	if (Config.performance == 5) {
		_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1436));
	} else {
		for (int32 i = 5; i > Config.performance; --i) {
			_vm->text()->draw("]");
		}
		if (!Config.performance)
			_vm->text()->draw("*");
	}

	// back to main menu
	switchFont(_cursor->position().x < 300 || _cursor->position().x > 300 + sizeMainMenu || _cursor->position().y < 340 || _cursor->position().y > 340 + 24);
	_vm->text()->setPosition(300, 340);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1437));

	// action
	if (_leftClick) {
		// back to main menu
		if (_cursor->position().x >= 300 && _cursor->position().x <= 300 + sizeMainMenu && _cursor->position().y >= 340 && _cursor->position().y <= 340 + 24) {
			// TODO: save new configurations
			exitSubMenu();
		}

		// gamma level minus
		if (_cursor->position().x >= 350 && _cursor->position().x <= sizeMinus + 350 && _cursor->position().y >= 150 && _cursor->position().y <= 174) {
			if (Config.gammaLevel) {
				Config.gammaLevel -= 1;
				// TODO: setResGammaLevel(0x80010011, 0);
			}
		}
		// gamma level plus
		if (_cursor->position().x >= sizeMinus + 360 && _cursor->position().x <= sizeMinus + sizePlus + 360 && _cursor->position().y >= 150 && _cursor->position().y <= 174) {
			if (Config.gammaLevel < 8) {
				Config.gammaLevel += 1;
				// TODO: setResGammaLevel(0x80010011, 0);
			}
		}

		// performance minus
		if (_cursor->position().x >= 350 && _cursor->position().x <= sizeMinus + 350 && _cursor->position().y >= 179 && _cursor->position().y <= 203) {
			if (Config.performance) {
				Config.performance -= 1;
				// TODO: change quality settings
			}
		}
		// performance plus
		if (_cursor->position().x >= sizeMinus + 360 && _cursor->position().x <= sizeMinus + sizePlus + 360 && _cursor->position().y >= 179 && _cursor->position().y <= 203) {
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
	_vm->text()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1408));

	// Yes
	switchFont(_cursor->position().x < 247 || _cursor->position().x > 247 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 581)) || _cursor->position().y < 273 || _cursor->position().y > 273 + 24);
	_vm->text()->setPosition(247, 273);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1409));

	// No
	switchFont(_cursor->position().x < 369 || _cursor->position().x > 369 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 582)) || _cursor->position().y < 273 || _cursor->position().y > 273 + 24);
	_vm->text()->setPosition(369, 273);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 1410));

	// action
	if (_leftClick) {
		// Yes
		if (_cursor->position().x >= 247 && _cursor->position().x <= 247 + 24 && _cursor->position().y >= 273 && _cursor->position().y <= 273 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 1409))) {
			_leftClick = false;

			// User clicked on quit, so push a quit event
			Common::Event event;
			event.type = Common::EVENT_QUIT;
			g_system->getEventManager()->pushEvent(event);
		}
		// No
		if (_cursor->position().x >= 369 && _cursor->position().x <= 369 + 24 && _cursor->position().y >= 273 && _cursor->position().y <= 273 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 1410)))
			exitSubMenu();
	}
}

void MainMenu::updateSubMenuShowCredits() {
	int32 posY	 = _creditsTextScroll;
	ResourceId resourceId = kResourceNone;
	int32 step	 = 0;
	int32 minBound = 0;
	int32 maxBound = 0;

	GraphicFrame *creditsFadeFrame = _creditsFadeResource->getFrame(0);
	_vm->screen()->copyRectToScreenWithTransparency((byte *)creditsFadeFrame->surface.pixels, creditsFadeFrame->surface.w, creditsFadeFrame->x, creditsFadeFrame->y, creditsFadeFrame->surface.w, creditsFadeFrame->surface.h);

	GraphicFrame *creditsFrame = _creditsResource->getFrame(MIN<uint>(_creditsResource->count() - 1, _creditsBgFrame));
	_vm->screen()->copyRectToScreenWithTransparency((byte *)creditsFrame->surface.pixels, creditsFrame->surface.w, creditsFrame->x, creditsFrame->y, creditsFrame->surface.w, creditsFrame->surface.h);

	_creditsBgFrame++;
	if (_creditsBgFrame >= _creditsResource->count())
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

			_vm->text()->setPosition(320, step + posY);
			_vm->text()->draw((ResourceId)(resourceId - 2147482201));
			posY = _creditsTextScroll;
		}
		step += 24;
		resourceId = (ResourceId)(resourceId + 1);
	} while (step < 0x21F0);

	_creditsTextScroll -= 2;

	// TODO: some palette stuffs

	// TODO: if gameFlag=901 (finished game) and already play music 56, start music 40

	if (_leftClick) {
		// Restore palette
		_vm->screen()->setPalette(MAKE_RESOURCE(kResourcePackShared, 17));
		// Stop all sounds
		_vm->sound()->stopMusic();
		// Start playing music
		_vm->sound()->playMusic(MAKE_RESOURCE(kResourcePackShared, 39));
		exitSubMenu();
	}
}

void MainMenu::exitSubMenu() {
	_leftClick = false;
	_activeMenuScreen = kMainMenu;

	// Copy the bright background to the back buffer
	getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 0), 1, 0, 0, 0);

	// Set the cursor
	_cursor->set(MAKE_RESOURCE(kResourcePackShared, 2));
}

void MainMenu::updateSubMenuReturnToGame() {
	loadFont(kFontYellow);

	// no game loaded
	_vm->text()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 712));

	// Main menu
	switchFont(_cursor->position().x < 285 || _cursor->position().x > 285 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 713)) || _cursor->position().y < 273 || _cursor->position().y > 273 + 24);
	_vm->text()->setPosition(285, 273);
	_vm->text()->draw(MAKE_RESOURCE(kResourcePackText, 713));

	// action
	if (_leftClick) {
		// Main menu
		if (_cursor->position().x >= 285 && _cursor->position().x <= 285 + _vm->text()->getWidth(MAKE_RESOURCE(kResourcePackText, 713)) && _cursor->position().y >= 273 && _cursor->position().y <= 273 + 24)
			exitSubMenu();
	}
}

} // end of namespace Asylum

