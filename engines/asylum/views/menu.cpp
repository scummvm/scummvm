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

#include "asylum/resources/actor.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/config.h"
#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/savegame.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"
#include "asylum/system/text.h"
#include "asylum/system/video.h"

#include "asylum/views/scene.h"

#include "asylum/respack.h"
#include "asylum/staticres.h"

#include "asylum/asylum.h"

namespace Asylum {

MainMenu::MainMenu(AsylumEngine *vm): _vm(vm) {
	_initGame = false;

	_activeScreen   = kMenuNone;
	_soundResourceId = kResourceNone;
	_musicResourceId = kResourceNone;
	_gameStarted = false;
	_currentIcon = kMenuNone;
	_dword_4464B8 = -1;
	_dword_455C74 = 0;
	_dword_455C78 = false;
	_dword_455C80 = false;
	_dword_455D4C = false;
	_dword_455D5C = false;
	_dword_455DD8 = false;
	_dword_455DE0 = false;
	_dword_456288 = 0;
	_dword_4562C0 = 0;
	_textScroll = 0;
	_creditsFrameIndex = 0;
	_needEyeCursorInit = false;

	memset(&_iconFrames, 0, sizeof(_iconFrames));
}

MainMenu::~MainMenu() {
	// Zero-out passed pointers
	_vm = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading and setup
//////////////////////////////////////////////////////////////////////////
void MainMenu::show() {
	getSharedData()->setFlag(kFlagSkipDrawScene, true);
	getScreen()->clear();

	// Set ourselves as the current event handler
	_vm->switchEventHandler(this);

	getSound()->stopAll();

	_activeScreen = kMenuShowCredits;
	_soundResourceId = kResourceNone;
	_musicResourceId = kResourceNone;
	_gameStarted = false;

	_textScroll = 480;
	_creditsFrameIndex = 0;

	setup();
}

void MainMenu::setup() {
	getScreen()->clear();
	// Original fills the screen with black

	getCursor()->hide();

	getSharedData()->setFlag(kFlag1, true);

	if (_vm->isGameFlagSet(kGameFlagFinishGame)) {
		getText()->loadFont(MAKE_RESOURCE(kResourcePackShared, 32));
		getScreen()->setPalette(MAKE_RESOURCE(kResourcePackShared, 31));
		getScreen()->setGammaLevel(MAKE_RESOURCE(kResourcePackShared, 31), 0);
		getScreen()->setupTransTables(4, MAKE_RESOURCE(kResourcePackShared, 34),
		                                 MAKE_RESOURCE(kResourcePackShared, 35),
		                                 MAKE_RESOURCE(kResourcePackShared, 36),
		                                 MAKE_RESOURCE(kResourcePackShared, 37));
		getScreen()->selectTransTable(1);

		_dword_455D4C = false;
		_dword_455D5C = false;

		getSound()->playSound(MAKE_RESOURCE(kResourcePackShared, 56), false, Config.voiceVolume);
	} else {
		getText()->loadFont(MAKE_RESOURCE(kResourcePackShared, 25));
		getScreen()->setPalette(MAKE_RESOURCE(kResourcePackShared, 26));
		getScreen()->setGammaLevel(MAKE_RESOURCE(kResourcePackShared, 26), 0);
		getScreen()->setupTransTables(4, MAKE_RESOURCE(kResourcePackShared, 27),
		                                 MAKE_RESOURCE(kResourcePackShared, 28),
		                                 MAKE_RESOURCE(kResourcePackShared, 29),
		                                 MAKE_RESOURCE(kResourcePackShared, 30));
		getScreen()->selectTransTable(1);

		getSound()->playMusic(kResourceNone, 0);
		getSound()->playMusic(MAKE_RESOURCE(kResourcePackShared, 38));
	}
}

void MainMenu::leave() {
	_activeScreen = kMenuNone;
	getCursor()->set(MAKE_RESOURCE(kResourcePackShared, 2), 0, 2);
	getText()->loadFont(kFontYellow);
}

void MainMenu::switchFont(bool condition) {
	getText()->loadFont((condition) ? kFontYellow : kFontBlue);
}

void MainMenu::closeCredits() {
	getScreen()->clear();
	// Original fills the screen with black

	getCursor()->show();
	getSharedData()->setFlag(kFlag1, false);

	getText()->loadFont(kFontYellow);
	getScreen()->setPalette(MAKE_RESOURCE(kResourcePackShared, 17));
	getScreen()->setGammaLevel(MAKE_RESOURCE(kResourcePackShared, 17), 0);
	getScreen()->setupTransTables(4, MAKE_RESOURCE(kResourcePackShared, 18),
	                                 MAKE_RESOURCE(kResourcePackShared, 19),
	                                 MAKE_RESOURCE(kResourcePackShared, 20),
	                                 MAKE_RESOURCE(kResourcePackShared, 21));
	getScreen()->selectTransTable(1);

	getSound()->playMusic(kResourceNone, 0);
	getSound()->playMusic(_musicResourceId);

	if (_vm->isGameFlagSet(kGameFlagFinishGame)) {
		if (!_dword_455D4C) {
			_dword_455D4C = true;
			getSound()->stop(MAKE_RESOURCE(kResourcePackShared, 56));
		}
	}

	leave();
}

void MainMenu::listMovies() {
	warning("[MainMenu::listMovies] Not implemented!");
}

MainMenu::MenuScreen MainMenu::findMousePosition() {
	for (uint i = 0; i < ARRAYSIZE(menuRects); i++)
		if (menuRects[i].contains(getCursor()->position()))
			return (MenuScreen)i;

	return kMenuNone;
}

//////////////////////////////////////////////////////////////////////////
// Event Handler
//////////////////////////////////////////////////////////////////////////
bool MainMenu::handleEvent(const AsylumEvent &evt) {
	switch ((uint32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		return init();
		break;

	case EVENT_ASYLUM_UPDATE:
		return update();
		break;

	case EVENT_ASYLUM_MUSIC:
		return music();
		break;

	case Common::EVENT_KEYDOWN:
		return key(evt);
		break;

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		return click(evt);
		break;
	}

	return false;
}

bool MainMenu::init() {
	// TODO: save dialog key codes into sntrm_k.txt (need to figure out why they use such thing) (address 00411CD0)

	if (_needEyeCursorInit) {
		getCursor()->set(MAKE_RESOURCE(kResourcePackShared, 3), 0, 2);
	} else {
		// Init the game if not already done
		if (!_initGame) {
			_initGame = true;

			// The original also
			//  - load the config (this is done when constructing the config object).
			//  - initialize game structures (this is done in classes constructors)

			getSaveLoad()->loadViewedMovies();

			_needEyeCursorInit = true;

			// Play start video
			getVideo()->playVideo(0);

			if (!getSaveLoad()->setup()) {
				_vm->restart();
				return true;
			}

			// The original preloads graphics

			getCursor()->show();
		}

		_dword_4562C0 = 0;
		_activeScreen = kMenuNone;
		_currentIcon = kMenuNone;
		_dword_455C74 = 0;

		getCursor()->hide();
		getCursor()->set(MAKE_RESOURCE(kResourcePackShared, 2), 0, 2);
	}

	if (_gameStarted)
		getScene()->getActor()->update_409230();

	getScreen()->clear();
	getText()->loadFont(kFontYellow);
	getScreen()->setPalette(MAKE_RESOURCE(kResourcePackShared, 17));
	getScreen()->setGammaLevel(MAKE_RESOURCE(kResourcePackShared, 17), 0);
	getScreen()->setupTransTables(4, MAKE_RESOURCE(kResourcePackShared, 18),
	                                 MAKE_RESOURCE(kResourcePackShared, 19),
	                                 MAKE_RESOURCE(kResourcePackShared, 20),
	                                 MAKE_RESOURCE(kResourcePackShared, 21));
	getScreen()->selectTransTable(1);

	// Update the screen
	g_system->updateScreen();

	getCursor()->show();
	return true;
}

bool MainMenu::update() {
	uint32 ticks = _vm->getTick();

	if (!getSharedData()->getFlag(kFlagRedraw)) {
		// TODO original fills a rectangle with black

		// Draw background
		getScreen()->draw(kBackground, (_activeScreen == kMenuNone) ? 1 : 0, 0, 0, 0);

		uint32 frameIndex = 0;

		// Get the eye frame index
		if (!getCursor()->isHidden()) {
			Common::Point cursor = getCursor()->position();

			if (cursor.x < 230 || cursor.x > 399 || cursor.y < 199 || cursor.y > 259)
				frameIndex = eyeFrameIndex[Actor::direction(Common::Point(320, 240), cursor)];
			else if (cursor.x >= 743 && cursor.x <= 743 && cursor.y >= 587 && cursor.y <= 602)
				frameIndex = 9;
		}

		if (_activeScreen == kMenuNone) {
			// Draw eye
			getScreen()->draw(kEye, frameIndex, 0, 0, 0);

			// Find mouse position
			MenuScreen icon = findMousePosition();
			if (icon != kMenuNone) {
				// Draw icon
				getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, icon + 4), _iconFrames[icon], 0, 0, 0);
				_iconFrames[icon] = (_iconFrames[icon] + 1) % GraphicResource::getFrameCount(_vm, MAKE_RESOURCE(kResourcePackShared, icon + 4));

				// Draw text
				getText()->drawCentered(menuRects[icon].left - 5, menuRects[icon].bottom + 5, menuRects[icon].width(), MAKE_RESOURCE(kResourcePackText, 1309 + icon));

				if (!_dword_455C74 || _currentIcon != icon) {
					_dword_455C74 = true;
					_currentIcon = icon;

					if (_soundResourceId
					 && getSound()->isPlaying(_soundResourceId)
					 && _soundResourceId != MAKE_RESOURCE(kResourcePackShared, icon + 44))
						getSound()->stopAll(_soundResourceId);

					if (_soundResourceId != MAKE_RESOURCE(kResourcePackShared, icon + 44) || !getSound()->isPlaying(_soundResourceId)) {
						_soundResourceId = MAKE_RESOURCE(kResourcePackShared, icon + 44);
						getSound()->playSound(_soundResourceId, false, Config.voiceVolume);
					}
				}
			} else {
				_dword_455C74 = 0;
			}
		} else {
			getScreen()->draw(kEye, frameIndex, 0, 0, 0, 3);

			// Draw icon
			getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, _activeScreen + 4), _iconFrames[_activeScreen], 0, 0, 0);
			_iconFrames[_activeScreen] = (_iconFrames[_activeScreen] + 1) % GraphicResource::getFrameCount(_vm, MAKE_RESOURCE(kResourcePackShared, _activeScreen + 4));
		}

		// Update current screen
		switch (_activeScreen) {
		default:
			break;

		case kMenuNewGame:
			updateNewGame();
			break;

		case kMenuLoadGame:
			updateLoadGame();
			break;

		case kMenuSaveGame:
			updateSaveGame();
			break;

		case kMenuDeleteGame:
			updateDeleteGame();
			break;

		case kMenuViewMovies:
			updateViewMovies();
			break;

		case kMenuQuitGame:
			updateQuitGame();
			break;

		case kMenuTextOptions:
			updateTextOptions();
			break;

		case kMenuAudioOptions:
			updateAudioOptions();
			break;

		case kMenuSettings:
			updateSettings();
			break;

		case kMenuKeyboardConfig:
			updateKeyboardConfig();
			break;

		case kMenuShowCredits:
			updateShowCredits();
			break;

		case kMenuReturnToGame:
			updateReturnToGame();
			break;
		}

		// Ask for redraw
		getSharedData()->setFlag(kFlagRedraw, true);
	}

	if (ticks > getSharedData()->getNextScreenUpdate()) {
		if (getSharedData()->getFlag(kFlagRedraw)) {
			getScreen()->copyBackBufferToScreen();

			getSharedData()->setFlag(kFlagRedraw, false);
			getSharedData()->setNextScreenUpdate(ticks + 55);
		}
	}

	return true;
}

bool MainMenu::music() {
	if (_activeScreen == kMenuShowCredits
	 && _vm->isGameFlagSet(kGameFlagFinishGame)
	 && !_dword_455D5C
	 && !_dword_455D4C) {
		_dword_455D5C = true;

		getSound()->playMusic(kResourceNone, 0);
		getSound()->playMusic(MAKE_RESOURCE(kResourcePackShared, 38));

		return true;
	}

	return false;
}

bool MainMenu::key(const AsylumEvent &evt) {
	switch (_activeScreen) {
	default:
		break;

	case kMenuSaveGame:
		keySaveGame();
		break;

	case kMenuKeyboardConfig:
		keyKeyboardConfig();
		break;

	case kMenuShowCredits:
		keyShowCredits();
		break;
	}

	return true;
}

bool MainMenu::click(const AsylumEvent &evt) {
	if (evt.type == Common::EVENT_RBUTTONDOWN
	 && _activeScreen == kMenuShowCredits) {
		 clickShowCredits();
		 return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Handle clicks on sub-screens
	if (_activeScreen != kMenuNone) {
		switch (_activeScreen) {
		default:
			break;

		case kMenuNewGame:
			clickNewGame();
			break;

		case kMenuLoadGame:
			clickLoadGame();
			break;

		case kMenuSaveGame:
			clickSaveGame();
			break;

		case kMenuDeleteGame:
			clickDeleteGame();
			break;

		case kMenuViewMovies:
			clickViewMovies();
			break;

		case kMenuQuitGame:
			clickQuitGame();
			break;

		case kMenuTextOptions:
			clickTextOptions();
			break;

		case kMenuAudioOptions:
			clickAudioOptions();
			break;

		case kMenuSettings:
			clickSettings();
			break;

		case kMenuKeyboardConfig:
			clickKeyboardConfig();
			break;

		case kMenuShowCredits:
			clickShowCredits();
			break;

		case kMenuReturnToGame:
			clickReturnToGame();
			break;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Handle clicks on the main menu
	_activeScreen = findMousePosition();
	if (_activeScreen == kMenuNone)
		return true;

	getCursor()->set(MAKE_RESOURCE(kResourcePackShared, 3), 0, 2);
	getText()->loadFont(kFontYellow);

	switch (_activeScreen) {
	default:
		break;

	case kMenuSaveGame:
		_dword_455DD8 = false;
		// Fallback to next case

	case kMenuLoadGame:
		_dword_455C80 = false;
		_dword_455C78 = false;
		_dword_456288 = 0;
		_textScroll = 0;
		getSaveLoad()->loadList();
		break;

	case kMenuDeleteGame:
		_dword_455C80 = false;
		_textScroll = 0;
		getSaveLoad()->loadList();
		break;

	case kMenuViewMovies:
		_needEyeCursorInit = false;
		_dword_455C78 = false;
		_dword_456288 = 0;
		_textScroll = 0;
		listMovies();
		break;

	case kMenuKeyboardConfig:
		_dword_4464B8 = -1;
		break;

	case kMenuReturnToGame:
		if (!_gameStarted)
			break;

		clickReturnToGame();
		break;

	case kMenuShowCredits:
		_textScroll = 480;
		_creditsFrameIndex = 0;
		setup();
		break;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// Update handlers
//////////////////////////////////////////////////////////////////////////
void MainMenu::updateNewGame() {
	Common::Point cursor = getCursor()->position();

	getText()->loadFont(kFontYellow);

	// Begin new game
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1321));

	// Yes
	switchFont(cursor.x < 247 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1322))) || cursor.y < 273 || cursor.y > (273 + 24));
	getText()->setPosition(247, 273);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1322));

	// No
	switchFont(cursor.x < 369 || cursor.x > (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1323))) || cursor.y < 273 || cursor.y > (273 + 24));
	getText()->setPosition(369, 273);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1323));
}

void MainMenu::updateLoadGame() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::updateLoadGame] Not implemented!");
}

void MainMenu::updateSaveGame() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::updateSaveGame] Not implemented!");
}

void MainMenu::updateDeleteGame() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::updateDeleteGame] Not implemented!");
}

void MainMenu::updateViewMovies() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::updateViewMovies] Not implemented!");
}

void MainMenu::updateQuitGame() {
	Common::Point cursor = getCursor()->position();

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1408));

	// Yes
	switchFont(cursor.x < 247 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1409))) || cursor.y < 273 || cursor.y > (273 + 24));
	getText()->setPosition(247, 273);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1409));

	// No
	switchFont(cursor.x < 369 || cursor.x > (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1410))) || cursor.y < 273 || cursor.y > (273 + 24));
	getText()->setPosition(369, 273);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1410));
}

void MainMenu::updateTextOptions() {
	Common::Point cursor = getCursor()->position();

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1411));

	getText()->draw(320, 150, MAKE_RESOURCE(kResourcePackText, 1412));

	switchFont(cursor.x < 350 || cursor.x > (350 + getText()->getWidth(Config.showMovieSubtitles ? MAKE_RESOURCE(kResourcePackText, 1414) : MAKE_RESOURCE(kResourcePackText, 1415))) || cursor.y < 150 || cursor.y > 174);
	getText()->setPosition(350, 150);
	getText()->draw(Config.showMovieSubtitles ? MAKE_RESOURCE(kResourcePackText, 1414) : MAKE_RESOURCE(kResourcePackText, 1415));

	getText()->loadFont(kFontYellow);
	getText()->draw(320, 179, MAKE_RESOURCE(kResourcePackText, 1413));

	switchFont(cursor.x < 350 || cursor.x > (350 + getText()->getWidth(Config.showEncounterSubtitles ? MAKE_RESOURCE(kResourcePackText, 1414) : MAKE_RESOURCE(kResourcePackText, 1415))) || cursor.y < 179 || cursor.y > 203);
	getText()->setPosition(350, 179);
	getText()->draw(Config.showEncounterSubtitles ? MAKE_RESOURCE(kResourcePackText, 1414) : MAKE_RESOURCE(kResourcePackText, 1415));

	switchFont(cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1416))) || cursor.y < 340 || cursor.y > (340 + 24));
	getText()->setPosition(300, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1416));
}

void MainMenu::updateAudioOptions() {
	Common::Point cursor = getCursor()->position();

	// Size of - and +
	int32 sizeMinus	= getText()->getWidth("-");
	int32 sizePlus  = getText()->getWidth("+");

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1420));

	int32 volumeIndex = 0;
	int32 volumeValue = cursor.x;
	do {
		getText()->loadFont(kFontYellow);
		getText()->draw(320, 29 * volumeIndex + 150, MAKE_RESOURCE(kResourcePackText, 1421 + volumeIndex));

		switchFont(cursor.x < 350 || cursor.x > (sizeMinus + 350) || cursor.y < (29 * volumeIndex + 150) || cursor.y > (29 * (volumeIndex + 6)));
		getText()->setPosition(350, 29 * volumeIndex + 150);
		getText()->draw("-");

		switchFont(cursor.x < (sizeMinus + 360) || cursor.x > (sizeMinus + sizePlus + 360) || cursor.y < (29 * volumeIndex + 150) || cursor.y > (29 * (volumeIndex + 6)));
		getText()->setPosition(sizeMinus + 360, 29 * volumeIndex + 150);
		getText()->draw("+");

		switch(volumeIndex) {
		default:
			break;

		case 0:
			volumeValue = 0;
			break;

		case 1:
			volumeValue = Config.musicVolume / 250 + 20;
			break;

		case 2:
			volumeValue = Config.ambientVolume / 250 + 20;
			break;

		case 3:
			volumeValue = Config.sfxVolume / 250 + 20;
			break;

		case 4:
			volumeValue = Config.voiceVolume / 250 + 20;
			break;

		case 5:
			volumeValue = Config.movieVolume / 250 + 20;
			break;
		}

		getText()->loadFont(kFontYellow);
		getText()->setPosition(sizePlus + sizeMinus + 365, 29 * volumeIndex + 150);
		if (volumeValue > 0) {
			for (int32 i = 0; i < volumeValue; i++)
				getText()->drawChar(']');

			if (volumeValue == 20)
				getText()->drawChar('*');
		} else if (volumeIndex) {
			getText()->draw(MAKE_RESOURCE(kResourcePackText, 1429));
		}

		++volumeIndex;
	} while (volumeIndex < 6);

	//////////////////////////////////////////////////////////////////////////
	//
	getText()->loadFont(kFontYellow);
	getText()->draw(320, 29 *volumeIndex + 150, MAKE_RESOURCE(kResourcePackText, 1427));

	switchFont(cursor.x < 350 || cursor.x > (350 + getText()->getWidth(Config.reverseStereo ? MAKE_RESOURCE(kResourcePackText, 1428) : MAKE_RESOURCE(kResourcePackText, 1429))) || cursor.y < (29 * volumeIndex + 150) || cursor.y > (29 * (volumeIndex + 6)));
	getText()->setPosition(350, 29 * volumeIndex + 150);
	getText()->draw(Config.reverseStereo ? MAKE_RESOURCE(kResourcePackText, 1428) : MAKE_RESOURCE(kResourcePackText, 1429));

	switchFont(cursor.x < 220 || cursor.x > (220 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1430))) || cursor.y < 360 || cursor.y > (360 + 24));
	getText()->setPosition(220, 360);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1430));

	switchFont((cursor.x < 360 || cursor.x > (360 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1431))) || cursor.y < 360 || cursor.y > (360 + 24)) && !_dword_455DE0);
	getText()->setPosition(360, 360);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1431));
}

void MainMenu::updateSettings() {
	Common::Point cursor = getCursor()->position();

	// Size of - and +
	int32 sizeMinus	= getText()->getWidth("-");
	int32 sizePlus  = getText()->getWidth("+");

	getText()->loadFont(kFontYellow);

	// Settings
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1432));

	//////////////////////////////////////////////////////////////////////////
	// Gamma correction
	getText()->draw(320, 150, MAKE_RESOURCE(kResourcePackText, 1433));

	switchFont(cursor.x < 350 || cursor.x > (sizeMinus + 350) || cursor.y < 150 || cursor.y > 174);
	getText()->setPosition(350, 150);
	getText()->draw("-");

	switchFont(cursor.x < (sizeMinus + 360) || cursor.x > (sizeMinus + sizePlus + 360) || cursor.y < 150 || cursor.y > 174);
	getText()->setPosition(sizeMinus + 360, 150);
	getText()->draw("+");

	getText()->setPosition(sizeMinus + sizePlus + 365, 150);
	getText()->loadFont(kFontYellow);
	if (Config.gammaLevel) {
		for (int32 i = 0; i < Config.gammaLevel; i++)
			getText()->drawChar(']');

		if (Config.gammaLevel == 8)
			getText()->drawChar('*');
	} else {
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1435));
	}

	//////////////////////////////////////////////////////////////////////////
	// Performance
	getText()->loadFont(kFontYellow);
	getText()->draw(320, 179, MAKE_RESOURCE(kResourcePackText, 1434));

	switchFont(cursor.x < 350 || cursor.x > (sizeMinus + 350) || cursor.y < 179 || cursor.y > 203);
	getText()->setPosition(350, 179);
	getText()->draw("-");

	switchFont(cursor.x < (sizeMinus + 360) || cursor.x > (sizeMinus + sizePlus + 360) || cursor.y < 179 || cursor.y > 203);
	getText()->setPosition(sizeMinus + 360, 179);
	getText()->draw("+");

	getText()->setPosition(sizeMinus + sizePlus + 365, 179);
	getText()->loadFont(kFontYellow);
	if (Config.performance == 5) {
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1436));
	} else {
		for (int32 i = 5; i > Config.performance; --i)
			getText()->drawChar(']');

		if (!Config.performance)
			getText()->draw('*');
	}

	//////////////////////////////////////////////////////////////////////////
	// Back to main menu
	switchFont(cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1437))) || cursor.y < 340 || cursor.y > (340 + 24));
	getText()->setPosition(300, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1437));
}

void MainMenu::updateKeyboardConfig() {
	Common::Point cursor = getCursor()->position();

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1438));

	char key = 0;
	int32 keyIndex = 0;

	do {
		getText()->loadFont(kFontYellow);
		if ((getScene() && getWorld()->chapter == 9) || keyIndex < 3) {
			getText()->draw(320, 29 * keyIndex + 150, MAKE_RESOURCE(kResourcePackText, 1439 + keyIndex));
		} else {
			getText()->draw(320, 29 * keyIndex + 150, MAKE_RESOURCE(kResourcePackText, 1445));
		}

		switch (keyIndex) {
		default:
			break;

		case 0:
			key = Config.keyShowVersion;
			break;

		case 1:
			key = Config.keyQuickLoad;
			break;

		case 2:
			key = Config.keyQuickSave;
			break;

		case 3:
			key = Config.keySwitchToSara;
			break;

		case 4:
			key = Config.keySwitchToGrimwall;
			break;

		case 5:
			key = Config.keySwitchToOlmec;
			break;
		}

		getText()->setPosition(350, 29 * keyIndex + 150);

		if (keyIndex == _dword_4464B8) {
			getText()->loadFont(kFontBlue);

			if (_dword_4562C0 < 6)
				getText()->drawChar('_');

			_dword_4562C0 = (_dword_4562C0 + 1) % 12;
		} else {
			switchFont(getCursor()->isHidden() || cursor.x < 350 || cursor.x > (350 + getText()->getWidth(key)) || cursor.y < (29 * keyIndex + 150) || cursor.y > (29 * (keyIndex + 6)));
			getText()->drawChar(key);
		}

		++keyIndex;
	} while (keyIndex < 6);

	switchFont(getCursor()->isHidden() || cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1446))) || cursor.y < 340 || cursor.y > (340 + 24));
	getText()->setPosition(340, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1446));
}

void MainMenu::updateReturnToGame() {
	Common::Point cursor = getCursor()->position();

	getText()->loadFont(kFontYellow);

	// No game loaded
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1810));

	// Main Menu
	switchFont(cursor.x < 285 || cursor.x > (285 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1811))) || cursor.y < 273 || cursor.y > (273 + 24));
	getText()->setPosition(285, 273);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1811));

}

void MainMenu::updateShowCredits() {
	if (_vm->isGameFlagSet(kGameFlagFinishGame)) {
		getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 33), 0, 0, 0, 0, false);
	} else {
		getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 23), 0, 0, 0, 0, false);
		getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 24), _creditsFrameIndex++ / 2, 0, 0, 0, false);

		_creditsFrameIndex %= 2 * GraphicResource::getFrameCount(_vm, MAKE_RESOURCE(kResourcePackShared, 24));
	}

	int32 step = 0;
	uint32 index = 0;
	do {
		if ((_textScroll + step - 24) >= 0) {
			if ((_textScroll + step) > 480)
				break;

			int32 minBound = _textScroll + step + 24;
			if (minBound >= 0)
				if (minBound < 32)
					getText()->setTransTableNum(3 - minBound / 8);

			int32 maxBound = _textScroll + step;
			if ((_textScroll + step) < 480)
				if (maxBound > 448)
					getText()->setTransTableNum(3 - (479 - maxBound) / 8);

			getText()->setPosition(320, step + _textScroll);
			getText()->draw(MAKE_RESOURCE(kResourcePackText, 1447 + index));
		}

		step += 24;
		++index;
	} while (step < 8688);

	if (_vm->isGameFlagSet(kGameFlagFinishGame)) {
		if (!_dword_455D4C && !getSound()->isPlaying(MAKE_RESOURCE(kResourcePackShared, 56))) {
			_dword_455D4C = true;
			getSound()->playMusic(kResourceNone, 0);
			getSound()->playMusic(MAKE_RESOURCE(kResourcePackShared, 40));
		}
	}

	_textScroll -= 2;
	if (_textScroll < -(8688 + 24))
		closeCredits();
}

//////////////////////////////////////////////////////////////////////////
// Click handlers
//////////////////////////////////////////////////////////////////////////
void MainMenu::clickNewGame() {
	Common::Point cursor = getCursor()->position();

	if (cursor.x < 247
	 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1322)))
	 || cursor.y < 273
	 || cursor.y > (273 + 24)) {
		if (cursor.x >= 369
		 && cursor.x <= (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1323)))
		 && cursor.y >= 273
		 && cursor.y <= (273 + 24))
			leave();
	} else {
		_vm->restart();
	}
}

void MainMenu::clickLoadGame() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::clickLoadGame] Not implemented!");
}

void MainMenu::clickSaveGame() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::clickSaveGame] Not implemented!");
}

void MainMenu::clickDeleteGame() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::clickDeleteGame] Not implemented!");
}

void MainMenu::clickViewMovies() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::clickViewMovies] Not implemented!");
}

void MainMenu::clickQuitGame() {
	Common::Point cursor = getCursor()->position();

	if (cursor.x < 247
	 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1409)))
	 || cursor.y < 273
	 || cursor.y > (273 + 24)) {
		if (cursor.x >= 369
		 && cursor.x <= (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1410)))
		 && cursor.y >= 273
		 && cursor.y <= (273 + 24)) {
			leave();
		}
	} else {
		getCursor()->hide();
		getScreen()->clear();
		_vm->quitGame();
	}
}

void MainMenu::clickTextOptions() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::clickTextOptions] Not implemented!");
}

void MainMenu::clickAudioOptions() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::clickAudioOptions] Not implemented!");
}

void MainMenu::clickSettings() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::clickSettings] Not implemented!");
}

void MainMenu::clickKeyboardConfig() {
	Common::Point cursor = getCursor()->position();

	error("[MainMenu::clickKeyboardConfig] Not implemented!");
}

void MainMenu::clickReturnToGame() {
	if (_gameStarted) {
		if (_musicResourceId != MAKE_RESOURCE(kResourcePackMusic, getWorld()->musicCurrentResourceIndex))
			getSound()->playMusic(kResourceNone, 0);

		getScreen()->clear();

		_vm->switchEventHandler(_vm->scene());
	} else {
		Common::Point cursor = getCursor()->position();

		if (cursor.x >= 285
		 && cursor.x <= (285 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1811)))
		 && cursor.y >= 273
		 && cursor.y <= (273 + 24))
			leave();
	}
}

void MainMenu::clickShowCredits() {
	closeCredits();
}

//////////////////////////////////////////////////////////////////////////
// Key handlers
//////////////////////////////////////////////////////////////////////////
void MainMenu::keySaveGame() {
	error("[MainMenu::keySaveGame] Not implemented!");
}

void MainMenu::keyKeyboardConfig() {
	error("[MainMenu::keyKeyboardConfig] Not implemented!");
}

void MainMenu::keyShowCredits() {
	closeCredits();
}

//////////////////////////////////////////////////////////////////////////
// TODO REMOVE
//////////////////////////////////////////////////////////////////////////

//void MainMenu::updateSubMenuCinematics() {
//	int32 currentCD = 1;	// FIXME: dummy value
//	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1352), currentCD);
//	getText()->setPosition(30, 340);
//	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1353));	// Prev Page
//
//	//if (cursor.x >= 280 && cursor.x <= 400 && cursor.y >= 340 && cursor.y <= 360) {
//	//	getText()->loadFont(kFontBlue);
//	//	if (_leftClick)
//	//		exitSubMenu();
//	//} else {
//	//	getText()->loadFont(kFontYellow);
//	//}
//
//	getText()->setPosition(280, 340);
//	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1355));	// Main Menu
//
//	getText()->loadFont(kFontYellow);
//
//	getText()->setPosition(500, 340);
//	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1354));	// Next Page
//}
//
//void MainMenu::updateSubMenuSettings() {
//	// action
//	//if (_leftClick) {
//	//	// back to main menu
//	//	if (cursor.x >= 300 && cursor.x <= 300 + sizeMainMenu && cursor.y >= 340 && cursor.y <= 340 + 24) {
//	//		// TODO: save new configurations
//	//		exitSubMenu();
//	//	}
//
//	//	// gamma level minus
//	//	if (cursor.x >= 350 && cursor.x <= sizeMinus + 350 && cursor.y >= 150 && cursor.y <= 174) {
//	//		if (Config.gammaLevel) {
//	//			Config.gammaLevel -= 1;
//	//			// TODO: setResGammaLevel(0x80010011, 0);
//	//		}
//	//	}
//	//	// gamma level plus
//	//	if (cursor.x >= sizeMinus + 360 && cursor.x <= sizeMinus + sizePlus + 360 && cursor.y >= 150 && cursor.y <= 174) {
//	//		if (Config.gammaLevel < 8) {
//	//			Config.gammaLevel += 1;
//	//			// TODO: setResGammaLevel(0x80010011, 0);
//	//		}
//	//	}
//
//	//	// performance minus
//	//	if (cursor.x >= 350 && cursor.x <= sizeMinus + 350 && cursor.y >= 179 && cursor.y <= 203) {
//	//		if (Config.performance) {
//	//			Config.performance -= 1;
//	//			// TODO: change quality settings
//	//		}
//	//	}
//	//	// performance plus
//	//	if (cursor.x >= sizeMinus + 360 && cursor.x <= sizeMinus + sizePlus + 360 && cursor.y >= 179 && cursor.y <= 203) {
//	//		if (Config.performance < 5) {
//	//			Config.performance += 1;
//	//			// TODO: change quality settings
//	//		}
//	//	}
//	//}
//}


} // end of namespace Asylum

