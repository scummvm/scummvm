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

#include "asylum/views/scene.h"
#include "asylum/views/video.h"

#include "asylum/respack.h"
#include "asylum/staticres.h"

#include "asylum/asylum.h"

namespace Asylum {

Menu::Menu(AsylumEngine *vm): _vm(vm) {
	_initGame = false;

	_activeScreen   = kMenuNone;
	_soundResourceId = kResourceNone;
	_musicResourceId = kResourceNone;
	_gameStarted = false;
	_currentIcon = kMenuNone;
	_selectedShortcutIndex = -1;
	_dword_455C74 = 0;
	_dword_455C78 = false;
	_dword_455C80 = false;
	_dword_455D4C = false;
	_dword_455D5C = false;
	_dword_455DD8 = false;
	_testSoundsPlaying = false;
	_dword_456288 = 0;
	_caretBlink = 0;
	_startIndex = 0;
	_creditsFrameIndex = 0;
	_showMovie = false;
	memset(&_iconFrames, 0, sizeof(_iconFrames));

	// Movies
	_movieCount = 0;
	_movieIndex = 0;
	memset(&_movieList, 0 , sizeof(_movieList));

	// Savegames
	_prefixWidth = 0;
}

Menu::~Menu() {
	// Zero-out passed pointers
	_vm = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Loading and setup
//////////////////////////////////////////////////////////////////////////
void Menu::show() {
	getSharedData()->setFlag(kFlagSkipDrawScene, true);
	getScreen()->clear();

	// Set ourselves as the current event handler
	_vm->switchEventHandler(this);

	getSound()->stopAll();

	_activeScreen = kMenuShowCredits;
	_soundResourceId = kResourceNone;
	_musicResourceId = kResourceNone;
	_gameStarted = false;

	_startIndex = 480;
	_creditsFrameIndex = 0;

	setup();
}

void Menu::setup() {
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

void Menu::leave() {
	_activeScreen = kMenuNone;
	getCursor()->set(MAKE_RESOURCE(kResourcePackShared, 2));
	getText()->loadFont(kFontYellow);
}

void Menu::switchFont(bool condition) {
	getText()->loadFont((condition) ? kFontYellow : kFontBlue);
}

void Menu::closeCredits() {
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

Menu::MenuScreen Menu::findMousePosition() {
	for (uint i = 0; i < ARRAYSIZE(menuRects); i++)
		if (menuRects[i].contains(getCursor()->position()))
			return (MenuScreen)i;

	return kMenuNone;
}

void Menu::playTestSounds() {
	_testSoundsPlaying = true;
	getSound()->playSound(kAmbientSound, true, Config.ambientVolume);
	getSound()->playSound(kSfxSound, true, Config.sfxVolume);
	getSound()->playSound(kVoiceSound, true, Config.voiceVolume);
}

void Menu::stopTestSounds() {
	_testSoundsPlaying = false;
	getSound()->stop(kAmbientSound);
	getSound()->stop(kSfxSound);
	getSound()->stop(kVoiceSound);
}

void Menu::adjustMasterVolume(int32 delta) {
	int32 *volume = NULL;
	int32 volumeIndex = 1;

	do {
		switch (volumeIndex) {
		default:
			break;

		case 1:
			volume = &Config.musicVolume;
			break;

		case 2:
			volume = &Config.ambientVolume;
			break;

		case 3:
			volume = &Config.sfxVolume;
			break;

		case 4:
			volume = &Config.voiceVolume;
			break;

		case 5:
			volume = &Config.movieVolume;
			break;
		}

		// Adjust and normalize volume
		if (delta >= 0) {
			if (*volume < 0) {
				if (*volume == -9999)
					*volume = -5000;

				*volume += 250;

				if (*volume > 0)
					*volume = 0;
			}
		} else {
			if (*volume > -5000) {
				*volume -= 250;

				if (*volume <= -5000)
					*volume = -9999;
			}
		}

		++volumeIndex;
	} while (volumeIndex < 6);
}

void Menu::adjustTestVolume() {
	getSound()->setMusicVolume(Config.musicVolume);
	if ((Config.movieVolume / 250 + 20) <= 0)
		getSound()->playMusic(_musicResourceId);

	if (getSound()->isPlaying(kAmbientSound))
		getSound()->setVolume(kAmbientSound, Config.ambientVolume);
	else if (_testSoundsPlaying)
		getSound()->playSound(kAmbientSound, true, Config.ambientVolume);

	if (getSound()->isPlaying(kSfxSound))
		getSound()->setVolume(kSfxSound, Config.sfxVolume);
	else if (_testSoundsPlaying)
		getSound()->playSound(kSfxSound, true, Config.sfxVolume);

	if (getSound()->isPlaying(kVoiceSound))
		getSound()->setVolume(kVoiceSound, Config.voiceVolume);
	else if (_testSoundsPlaying)
		getSound()->playSound(kVoiceSound, true, Config.voiceVolume);
}

void Menu::setupMusic() {
	getSound()->stopAll();

	uint32 index = getScene() ? getWorld()->musicCurrentResourceIndex : kMusicStopped;

	if (index == kMusicStopped) {
		_soundResourceId = kResourceNone;
		_musicResourceId = MAKE_RESOURCE(kResourcePackShared, 39);

		getSound()->playMusic(_musicResourceId);
	} else {
		_soundResourceId = kResourceNone;
		_musicResourceId = MAKE_RESOURCE(kResourcePackMusic, index);
	}
}

void Menu::adjustPerformance() {
	// Original reinitialize sound to 11kHz for performance == 0, 22kHz otherwise
	getSound()->stopAll();
	getSound()->playMusic(kResourceNone, 0);
	setupMusic();

	uint32 index = getScene() ? getWorld()->musicCurrentResourceIndex : kMusicStopped;
	if (index != kMusicStopped)
		getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, index));
}

Common::String Menu::getChapterName() {
	return Common::String::format("%s%2d", getText()->get(MAKE_RESOURCE(kResourcePackText, 1334)), chapterIndexes[getWorld()->chapter]);
}

//////////////////////////////////////////////////////////////////////////
// Event Handler
//////////////////////////////////////////////////////////////////////////
bool Menu::handleEvent(const AsylumEvent &evt) {
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

bool Menu::init() {
	// TODO: save dialog key codes into sntrm_k.txt (need to figure out why they use such thing) (address 00411CD0)

	if (_showMovie) {
		_showMovie = false;
		getCursor()->set(MAKE_RESOURCE(kResourcePackShared, 3));
	} else {
		// Init the game if not already done
		if (!_initGame) {
			_initGame = true;

			// The original also
			//  - load the config (this is done when constructing the config singleton).
			//  - initialize game structures (this is done in classes constructors)
			getSaveLoad()->loadMoviesViewed();

			_showMovie = true;

			// Play start video
			getVideo()->play(0, this);

			// If no savegame is present, start the game directly
			if (!getSaveLoad()->hasSavegames()) {
				_vm->restart();
				return true;
			}

			// The original preloads graphics
			getCursor()->show();
		}

		_caretBlink = 0;
		_activeScreen = kMenuNone;
		_currentIcon = kMenuNone;
		_dword_455C74 = 0;

		setupMusic();

		getCursor()->hide();
		getCursor()->set(MAKE_RESOURCE(kResourcePackShared, 2));
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

bool Menu::update() {
	uint32 ticks = _vm->getTick();

	if (!getSharedData()->getFlag(kFlagRedraw)) {
		// The original clears the area where the eyes are
		//getScreen()->fillRect(260, 229, 119, 16, 0);

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

	if (ticks > getSharedData()->nextScreenUpdate) {
		if (getSharedData()->getFlag(kFlagRedraw)) {
			getScreen()->copyBackBufferToScreen();

			getSharedData()->setFlag(kFlagRedraw, false);
			getSharedData()->nextScreenUpdate = ticks + 55;
		}
	}

	return true;
}

bool Menu::music() {
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

bool Menu::key(const AsylumEvent &evt) {
	switch (_activeScreen) {
	default:
		break;

	case kMenuSaveGame:
		keySaveGame(evt);
		break;

	case kMenuKeyboardConfig:
		keyKeyboardConfig(evt);
		break;

	case kMenuShowCredits:
		keyShowCredits();
		break;
	}

	return true;
}

bool Menu::click(const AsylumEvent &evt) {
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

	getCursor()->set(MAKE_RESOURCE(kResourcePackShared, 3));
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
		_startIndex = 0;
		getSaveLoad()->loadList();
		break;

	case kMenuDeleteGame:
		_dword_455C80 = false;
		_startIndex = 0;
		getSaveLoad()->loadList();
		break;

	case kMenuViewMovies:
		_showMovie = false;
		_dword_455C78 = false;
		_dword_456288 = 0;
		_startIndex = 0;
		_movieCount = getSaveLoad()->getMoviesViewed((int32 *)&_movieList);
		break;

	case kMenuKeyboardConfig:
		_selectedShortcutIndex = -1;
		break;

	case kMenuReturnToGame:
		if (!_gameStarted)
			break;

		clickReturnToGame();
		break;

	case kMenuShowCredits:
		_startIndex = 480;
		_creditsFrameIndex = 0;
		setup();
		break;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// Update handlers
//////////////////////////////////////////////////////////////////////////
void Menu::updateNewGame() {
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

void Menu::updateLoadGame() {
	Common::Point cursor = getCursor()->position();

	char text[100];

	if (_dword_455C80) {
		getText()->loadFont(kFontYellow);
		getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1329));

		sprintf((char *)&text, "%s ?", getSaveLoad()->getName()->c_str());
		getText()->drawCentered(10, 134, 620, (char *)&text);

		if (cursor.x < 247 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1330)))
		 || cursor.y < 273 || cursor.y > (273 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(247, 273);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1330));

		if (cursor.x < 369 || cursor.x > (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1331)))
		 || cursor.y < 273 || cursor.y > (273 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(369, 273);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1331));
		return;
	}

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1325));

	if (_dword_455C78) {
		getText()->drawCentered(10, 190, 620, MAKE_RESOURCE(kResourcePackText, 1332));
		getText()->drawCentered(10, 190 + 29, 620, MAKE_RESOURCE(kResourcePackText, 1333));
		getText()->drawCentered(10, 190 + 53, 620, getSaveLoad()->getName()->c_str());

		++_dword_456288;

		if (_dword_456288 == 60) {
			_dword_456288 = 0;
			_dword_455C78 = false;
			getCursor()->show();
		} else if (_dword_456288 == 1) {
			getCursor()->hide();
		}
	} else {
		//////////////////////////////////////////////////////////////////////////
		// First column
		uint32 index = 0;
		for (int32 y = 150; y < 324; y += 29) {
			if (index + _startIndex >= 25)
				break;

			sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

			if (cursor.x < 30 || cursor.x > (30 + getText()->getWidth((char *)&text))
			 || cursor.y < y  || cursor.y > (y + 24))
				getText()->loadFont(kFontYellow);
			else
				getText()->loadFont(kFontBlue);

			getText()->setPosition(30, y);
			getText()->draw((char *)&text);

			++index;
		}

		//////////////////////////////////////////////////////////////////////////
		// Second column
		for (int32 y = 150; y < 324; y += 29) {
			if (index + _startIndex >= 25)
				break;

			sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

			if (cursor.x < 350 || cursor.x > (350 + getText()->getWidth((char *)&text))
				|| cursor.y < y   || cursor.y > (y + 24))
				getText()->loadFont(kFontYellow);
			else
				getText()->loadFont(kFontBlue);

			getText()->setPosition(350, y);
			getText()->draw((char *)&text);

			++index;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Previous page
	if (cursor.x < 30  || cursor.x > (30 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1326)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(30, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1326));

	//////////////////////////////////////////////////////////////////////////
	// Main menu
	if (cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1328)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(300, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1328));

	//////////////////////////////////////////////////////////////////////////
	// Next page
	if (cursor.x < 550 || cursor.x > (550 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1327)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(550, 347);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1327));
}

void Menu::updateSaveGame() {
	Common::Point cursor = getCursor()->position();

	char text[100];

	if (_dword_455C80) {
		getText()->loadFont(kFontYellow);
		getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1339));

		sprintf((char *)&text, "%s ?", getSaveLoad()->getName()->c_str());
		getText()->drawCentered(10, 134, 620, (char *)&text);

		if (cursor.x < 247 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1340)))
		 || cursor.y < 273 || cursor.y > (273 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(247, 273);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1340));

		if (cursor.x < 369 || cursor.x > (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1341)))
		 || cursor.y < 273 || cursor.y > (273 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(369, 273);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1341));
		return;
	}

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1335));

	if (_dword_455C78) {
		getText()->drawCentered(10, 220, 620, MAKE_RESOURCE(kResourcePackText, 1343));
		getText()->drawCentered(10, 220 + 29, 620, getSaveLoad()->getName()->c_str());

		++_dword_456288;

		if (_dword_456288 == 30) {
			_dword_456288 = 0;
			_dword_455C78 = false;
			getCursor()->show();
		}
	} else {
		//////////////////////////////////////////////////////////////////////////
		// First column
		uint32 index = 0;
		for (int32 y = 150; y < 324; y += 29) {
			if (index + _startIndex >= 25)
				break;

			sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

			if (!_dword_455DD8) {
				if (cursor.x < 30 || cursor.x > (30 + getText()->getWidth((char *)&text))
				 || cursor.y < y  || cursor.y > (y + 24))
					getText()->loadFont(kFontYellow);
				else
					getText()->loadFont(kFontBlue);
			} else {
				if (getSaveLoad()->getIndex() != index + _startIndex)
					getText()->loadFont(kFontYellow);
				else
					getText()->loadFont(kFontBlue);
			}

			getText()->setPosition(30, y);
			getText()->draw((char *)&text);

			// Draw underscore
			if (_dword_455DD8) {
				if (getSaveLoad()->getIndex() == index + _startIndex) {
					if (_caretBlink < 6)
						getText()->drawChar('_');

					_caretBlink = (_caretBlink + 1) % 12;
				}
			}

			++index;
		}

		//////////////////////////////////////////////////////////////////////////
		// Second column
		for (int32 y = 150; y < 324; y += 29) {
			if (index + _startIndex >= 25)
				break;

			sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

			if (!_dword_455DD8) {
				if (cursor.x < 350 || cursor.x > (350 + getText()->getWidth((char *)&text))
				 || cursor.y < y   || cursor.y > (y + 24))
					getText()->loadFont(kFontYellow);
				else
					getText()->loadFont(kFontBlue);
			} else {
				if (getSaveLoad()->getIndex() != index + _startIndex)
					getText()->loadFont(kFontYellow);
				else
					getText()->loadFont(kFontBlue);
			}

			getText()->setPosition(350, y);
			getText()->draw((char *)&text);

			// Draw underscore
			if (_dword_455DD8) {
				if (getSaveLoad()->getIndex() == index + _startIndex) {
					if (_caretBlink < 6)
						getText()->drawChar('_');

					_caretBlink = (_caretBlink + 1) % 12;
				}
			}

			++index;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Previous page
	if (getCursor()->isHidden()
	 || cursor.x < 30  || cursor.x > (30 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1336)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(30, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1336));

	//////////////////////////////////////////////////////////////////////////
	// Main menu
	if (getCursor()->isHidden()
	 || cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1338)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(300, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1338));

	//////////////////////////////////////////////////////////////////////////
	// Next page
	if (getCursor()->isHidden()
	 || cursor.x < 550 || cursor.x > (550 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1337)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(550, 347);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1337));
}

void Menu::updateDeleteGame() {
	Common::Point cursor = getCursor()->position();

	char text[100];

	if (_dword_455C80) {
		getText()->loadFont(kFontYellow);
		getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1349));

		sprintf((char *)&text, "%s ?", getSaveLoad()->getName()->c_str());
		getText()->drawCentered(10, 134, 620, (char *)&text);

		if (cursor.x < 247 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1350)))
		 || cursor.y < 273 || cursor.y > (273 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(247, 273);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1350));

		if (cursor.x < 369 || cursor.x > (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1351)))
		 || cursor.y < 273 || cursor.y > (273 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(369, 273);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1351));
		return;
	}

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1345));

	//////////////////////////////////////////////////////////////////////////
	// First column
	uint32 index = 0;
	for (int32 y = 150; y < 324; y += 29) {
		if (index + _startIndex >= 25)
			break;

		sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

		if (cursor.x < 30 || cursor.x > (30 + getText()->getWidth((char *)&text))
		 || cursor.y < y  || cursor.y > (y + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(30, y);
		getText()->draw((char *)&text);

		++index;
	}

	//////////////////////////////////////////////////////////////////////////
	// Second column
	for (int32 y = 150; y < 324; y += 29) {
		if (index + _startIndex >= 25)
			break;

		sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

		if (cursor.x < 350 || cursor.x > (350 + getText()->getWidth((char *)&text))
		 || cursor.y < y   || cursor.y > (y + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(350, y);
		getText()->draw((char *)&text);

		++index;
	}

	//////////////////////////////////////////////////////////////////////////
	// Previous page
	if (cursor.x < 30  || cursor.x > (30 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1346)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(30, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1346));

	//////////////////////////////////////////////////////////////////////////
	// Main menu
	if (cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1348)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(300, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1348));

	//////////////////////////////////////////////////////////////////////////
	// Next page
	if (cursor.x < 550 || cursor.x > (550 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1347)))
	 || cursor.y < 340 || cursor.y > (340 + 24))
		getText()->loadFont(kFontYellow);
	else
		getText()->loadFont(kFontBlue);

	getText()->setPosition(550, 347);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1347));
}

void Menu::updateViewMovies() {
	Common::Point cursor = getCursor()->position();

	char text[100];
	char text2[100];

	if (!_dword_455C78) {
		getText()->loadFont(kFontYellow);
		sprintf((char *)&text2, getText()->get(MAKE_RESOURCE(kResourcePackText, 1352)), getSharedData()->cdNumber);
		getText()->drawCentered(10, 100, 620, (char *)&text2);

		//////////////////////////////////////////////////////////////////////////
		// First column
		int32 index = _startIndex;
		for (int32 y = 150; y < 324; y += 29) {
			if (index >= ARRAYSIZE(_movieList))
				break;

			if (_movieList[index] != -1 ) {
				sprintf((char *)&text, "%d. %s ", index + 1, getText()->get(MAKE_RESOURCE(kResourcePackText, 1359 + _movieList[index])));
				sprintf((char *)&text2, getText()->get(MAKE_RESOURCE(kResourcePackText, 1356)), moviesCd[_movieList[index]]);
				strcat((char *)&text, (char *)&text2);

				if (getCursor()->isHidden()
				 || cursor.x < 30 || cursor.x > (30 + getText()->getWidth((char *)&text))
				 || cursor.y < y || cursor.y > (y + 24))
					getText()->loadFont(kFontYellow);
				else
					getText()->loadFont(kFontBlue);

				getText()->setPosition(30, y);
				getText()->draw((char *)&text);
			}

			++index;
		}

		//////////////////////////////////////////////////////////////////////////
		// Second column
		for (int32 y = 150; y < 324; y += 29) {
			if (index >= ARRAYSIZE(_movieList))
				break;

			if (_movieList[index] != -1 ) {
				sprintf((char *)&text, "%d. %s ", index + 1, getText()->get(MAKE_RESOURCE(kResourcePackText, 1359 + _movieList[index])));
				sprintf((char *)&text2, getText()->get(MAKE_RESOURCE(kResourcePackText, 1356)), moviesCd[_movieList[index]]);
				strcat((char *)&text, (char *)&text2);

				if (getCursor()->isHidden()
					|| cursor.x < 350 || cursor.x > (350 + getText()->getWidth((char *)&text))
					|| cursor.y < y || cursor.y > (y + 24))
					getText()->loadFont(kFontYellow);
				else
					getText()->loadFont(kFontBlue);

				getText()->setPosition(350, y);
				getText()->draw((char *)&text);
			}

			index++;
		}

		//////////////////////////////////////////////////////////////////////////
		// Previous page
		if (getCursor()->isHidden()
			|| cursor.x < 30 || cursor.x > (30 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1353)))
			|| cursor.y < 340 || cursor.y > (340 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(30, 340);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1353));

		//////////////////////////////////////////////////////////////////////////
		// Main Menu
		if (getCursor()->isHidden()
		 || cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1355)))
		 || cursor.y < 340 || cursor.y > (340 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(300, 340);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1355));

		//////////////////////////////////////////////////////////////////////////
		// Next Page
		if (getCursor()->isHidden()
		 || cursor.x < 550 || cursor.x > (550 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1354)))
		 || cursor.y < 340 || cursor.y > (340 + 24))
			getText()->loadFont(kFontYellow);
		else
			getText()->loadFont(kFontBlue);

		getText()->setPosition(550, 340);
		getText()->draw(MAKE_RESOURCE(kResourcePackText, 1354));

		//////////////////////////////////////////////////////////////////////////
		// Play video if needed
		if (_showMovie) {
			getSound()->playMusic(0, 0);

			getVideo()->play(_movieIndex, this);

			getSound()->playMusic(_musicResourceId);
		}

		return;
	}

	getText()->loadFont(kFontYellow);
	sprintf((char *)&text2, getText()->get(MAKE_RESOURCE(kResourcePackText, 1357)), getSharedData()->cdNumber);
	getText()->drawCentered(10, 100, 620, text2);

	strcpy((char *)&text, getText()->get(MAKE_RESOURCE(kResourcePackText, 1359 + _movieIndex)));
	sprintf((char *)&text2, getText()->get(MAKE_RESOURCE(kResourcePackText, 1356)), moviesCd[_movieIndex]);
	strcat((char *)&text, (char *)&text2);
	getText()->drawCentered(10, 134, 620, text);

	getText()->drawCentered(10, 168, 620, getText()->get(MAKE_RESOURCE(kResourcePackText, 1358)));

	++_dword_456288;
	if (_dword_456288 == 90) {
		_dword_456288 = 0;
		_dword_455C78 = false;

		getCursor()->show();
	}
}

void Menu::updateQuitGame() {
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

void Menu::updateTextOptions() {
	Common::Point cursor = getCursor()->position();

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1411));

	getText()->draw(320, 150, MAKE_RESOURCE(kResourcePackText, 1412));

	switchFont(cursor.x < 350 || cursor.x > (350 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, Config.showMovieSubtitles ? 1414 : 1415))) || cursor.y < 150 || cursor.y > 174);
	getText()->setPosition(350, 150);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, Config.showMovieSubtitles ? 1414 : 1415));

	getText()->loadFont(kFontYellow);
	getText()->draw(320, 179, MAKE_RESOURCE(kResourcePackText, 1413));

	switchFont(cursor.x < 350 || cursor.x > (350 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, Config.showEncounterSubtitles ? 1414 : 1415))) || cursor.y < 179 || cursor.y > 203);
	getText()->setPosition(350, 179);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, Config.showEncounterSubtitles ? 1414 : 1415));

	switchFont(cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1416))) || cursor.y < 340 || cursor.y > (340 + 24));
	getText()->setPosition(300, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1416));
}

void Menu::updateAudioOptions() {
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

	switchFont((cursor.x < 360 || cursor.x > (360 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1431))) || cursor.y < 360 || cursor.y > (360 + 24)) && !_testSoundsPlaying);
	getText()->setPosition(360, 360);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1431));
}

void Menu::updateSettings() {
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
		for (int32 i = 5; i > Config.performance; --i) // This has ] augmenting when pressing - which is a bit convoluted (perf == speed == more ])
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

void Menu::updateKeyboardConfig() {
	Common::Point cursor = getCursor()->position();

	getText()->loadFont(kFontYellow);
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1438));

	char keyCode = 0;
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
			keyCode = Config.keyShowVersion;
			break;

		case 1:
			keyCode = Config.keyQuickLoad;
			break;

		case 2:
			keyCode = Config.keyQuickSave;
			break;

		case 3:
			keyCode = Config.keySwitchToSara;
			break;

		case 4:
			keyCode = Config.keySwitchToGrimwall;
			break;

		case 5:
			keyCode = Config.keySwitchToOlmec;
			break;
		}

		getText()->setPosition(350, 29 * keyIndex + 150);

		if (keyIndex == _selectedShortcutIndex) {
			getText()->loadFont(kFontBlue);

			if (_caretBlink < 6)
				getText()->drawChar('_');

			_caretBlink = (_caretBlink + 1) % 12;
		} else {
			switchFont(getCursor()->isHidden() || cursor.x < 350 || cursor.x > (350 + getText()->getWidth(keyCode)) || cursor.y < (29 * keyIndex + 150) || cursor.y > (29 * (keyIndex + 6)));
			getText()->drawChar(keyCode);
		}

		++keyIndex;
	} while (keyIndex < 6);

	switchFont(getCursor()->isHidden() || cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1446))) || cursor.y < 340 || cursor.y > (340 + 24));
	getText()->setPosition(340, 340);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1446));
}

void Menu::updateReturnToGame() {
	Common::Point cursor = getCursor()->position();

	getText()->loadFont(kFontYellow);

	// No game loaded
	getText()->drawCentered(10, 100, 620, MAKE_RESOURCE(kResourcePackText, 1810));

	// Main Menu
	switchFont(cursor.x < 285 || cursor.x > (285 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1811))) || cursor.y < 273 || cursor.y > (273 + 24));
	getText()->setPosition(285, 273);
	getText()->draw(MAKE_RESOURCE(kResourcePackText, 1811));

}

void Menu::updateShowCredits() {
	if (_vm->isGameFlagSet(kGameFlagFinishGame)) {
		getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 33));
	} else {
		getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 23));
		getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 24), _creditsFrameIndex++ / 2, 0, 0, 0, false);

		_creditsFrameIndex %= 2 * GraphicResource::getFrameCount(_vm, MAKE_RESOURCE(kResourcePackShared, 24));
	}

	int32 step = 0;
	uint32 index = 0;
	do {
		if ((_startIndex + step - 24) >= 0) {
			if ((_startIndex + step) > 480)
				break;

			int32 minBound = _startIndex + step + 24;
			if (minBound >= 0)
				if (minBound < 32)
					getText()->setTransTableNum(3 - minBound / 8);

			int32 maxBound = _startIndex + step;
			if ((_startIndex + step) < 480)
				if (maxBound > 448)
					getText()->setTransTableNum(3 - (479 - maxBound) / 8);

			getText()->setPosition(320, step + _startIndex);
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

	_startIndex -= 2;
	if (_startIndex < -(8688 + 24))
		closeCredits();
}

//////////////////////////////////////////////////////////////////////////
// Click handlers
//////////////////////////////////////////////////////////////////////////
void Menu::clickNewGame() {
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

void Menu::clickLoadGame() {
	Common::Point cursor = getCursor()->position();

	if (_dword_455C80) {
		if (cursor.x < 247 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1330)))
		 || cursor.y < 273 || cursor.y > (273 + 24)) {
			if (cursor.x >= 369 && cursor.x <= (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1331)))
			 && cursor.y >= 273 && cursor.y <= (273 + 24))
				_dword_455C80 = false;
		} else {
			_vm->startGame(getSaveLoad()->getScenePack(), AsylumEngine::kStartGameLoad);
		}
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Previous page
	if (cursor.x >= 30  && cursor.x <= (30 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1326)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		if (_startIndex) {
			_startIndex -= 12;
			if (_startIndex < 0)
				_startIndex = 0;
		}

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Main Menu
	if (cursor.x >= 300 && cursor.x <= (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1328)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		leave();
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Next page
	if (cursor.x >= 550 && cursor.x <= (550 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1327)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		if (_startIndex + 12 < 25) {
			_startIndex += 12;
			if (_startIndex >= 25)
				_startIndex = 24;
		}

		return;
	}

	char text[100];

	//////////////////////////////////////////////////////////////////////////
	// Columns
	uint32 index = 0;
	for (int32 y = 150; y < 324; y += 29) {
		if (cursor.x >= 350) {
			sprintf((char *)&text, "%d. %s ", index + _startIndex + 7, getSaveLoad()->getName(index + _startIndex + 6).c_str());

			if (cursor.x <= (350 + getText()->getWidth((char *)&text))
			 && cursor.y >= y
			 && cursor.y <= (y + 24)) {
				if (index + _startIndex + 6 < 25) {
					if (getSaveLoad()->hasSavegame(index + _startIndex + 6)) {
						_dword_455C80 = true;
						getSaveLoad()->setIndex(index + _startIndex + 6);
					}
				}
			}
		} else if (cursor.x >= 30) {
			sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

			if (cursor.x <= (30 + getText()->getWidth((char *)&text))
			 && cursor.y >= y
			 && cursor.y <= (y + 24)) {
				if (index + _startIndex < 25) {
					if (getSaveLoad()->hasSavegame(index + _startIndex)) {
						_dword_455C80 = true;
						getSaveLoad()->setIndex(index + _startIndex);
					}
				}
			}
		}

		++index;
	}
}

void Menu::clickSaveGame() {
	Common::Point cursor = getCursor()->position();

	if (_dword_455C80) {
		if (cursor.x < 247 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1340)))
		 || cursor.y < 273 || cursor.y > (247 + 24)) {
			if (cursor.x >= 369 && cursor.x <= (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1341)))
			 && cursor.y >= 273 && cursor.y <= (273 + 24))
				_dword_455C80 = 0;
		} else {
			_dword_455C80 = false;
			_dword_455DD8 = true;

			_previousName = *getSaveLoad()->getName();
			_prefixWidth = getText()->getWidth(Common::String::format("%d. %c", getSaveLoad()->getIndex() + 1, '_').c_str());
			getCursor()->hide();
		}

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Previous page
	if (cursor.x >= 30  && cursor.x <= (30 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1336)))
	&& cursor.y >= 340 && cursor.y <= (340 + 24)) {
		if (_startIndex) {
			_startIndex -= 12;
			if (_startIndex < 0)
				_startIndex = 0;
		}

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Main menu
	if (cursor.x >= 300 && cursor.x <= (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1338)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		leave();
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Next page
	if (cursor.x >= 550 && cursor.x <= (550 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1337)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		if (_startIndex + 12 < 25) {
			_startIndex += 12;
			if (_startIndex >= 25)
				_startIndex = 24;
		}
		return;
	}

	char text[200];

	//////////////////////////////////////////////////////////////////////////
	// Columns
	uint32 index = 0;
	for (int32 y = 150; y < 324; y += 29) {
		if (cursor.x >= 350) {
			sprintf((char *)&text, "%d. %s ", index + _startIndex + 7, getSaveLoad()->getName(index + _startIndex + 6).c_str());

			if (cursor.x <= (30 + getText()->getWidth((char *)&text))
			 && cursor.y >= y
			 && cursor.y <= (y + 24)
			 && getWorld()->chapter != kChapterNone) {
				if (index + _startIndex < 25) {
					getSaveLoad()->setIndex(index + _startIndex + 6);
					if (getSaveLoad()->hasSavegame(index + _startIndex + 6)) {
						_dword_455C80 = true;
					} else {
						_dword_455DD8 = true;
						_previousName = *getSaveLoad()->getName();
						*getSaveLoad()->getName() = getChapterName();
						_prefixWidth = getText()->getWidth(Common::String::format("%d. %c", getSaveLoad()->getIndex() + 1, '_').c_str());
						getCursor()->hide();
					}
				}
			}
		} else if (cursor.x >= 30) {
			sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

			if (cursor.x <= (350 + getText()->getWidth((char *)&text))
			 && cursor.y >= y
			 && cursor.y <= (y + 24)
			 && getWorld()->chapter != kChapterNone) {
				if (index + _startIndex < 25) {
					getSaveLoad()->setIndex(index + _startIndex);
					if (getSaveLoad()->hasSavegame(index + _startIndex)) {
						_dword_455C80 = true;
					} else {
						_dword_455DD8 = true;
						_previousName = *getSaveLoad()->getName();
						*getSaveLoad()->getName() = getChapterName();
						_prefixWidth = getText()->getWidth(Common::String::format("%d. %c", getSaveLoad()->getIndex() + 1, '_').c_str());
						getCursor()->hide();
					}
				}
			}
		}

		++index;
	}
}

void Menu::clickDeleteGame() {
	Common::Point cursor = getCursor()->position();

	if (_dword_455C80) {
		if (cursor.x < 247 || cursor.x > (247 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1350)))
		 || cursor.y < 273 || cursor.y > (247 + 24)) {
			if (cursor.x >= 369 && cursor.x <= (369 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1351)))
			 && cursor.y >= 273 && cursor.y <= (273 + 24))
				_dword_455C80 = 0;
		} else {
			getSaveLoad()->remove();
		}

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Previous page
	if (cursor.x >= 30  && cursor.x <= (30 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1346)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		if (_startIndex) {
			_startIndex -= 12;
			if (_startIndex < 0)
				_startIndex = 0;
		}

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Main menu
	if (cursor.x >= 300 && cursor.x <= (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1348)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		leave();
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Next page
	if (cursor.x >= 550 && cursor.x <= (550 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1347)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		if (_startIndex + 12 < 25) {
			_startIndex += 12;
			if (_startIndex >= 25)
				_startIndex = 24;
		}

		return;
	}

	char text[200];

	//////////////////////////////////////////////////////////////////////////
	// Columns
	uint32 index = 0;
	for (int32 y = 150; y < 324; y += 29) {
		if (cursor.x >= 350) {
			sprintf((char *)&text, "%d. %s ", index + _startIndex + 7, getSaveLoad()->getName(index + _startIndex + 6).c_str());

			if (cursor.x <= (30 + getText()->getWidth((char *)&text))
			 && cursor.y >= y
			 && cursor.y <= (y + 24)) {
				if (index + _startIndex < 25) {
					if (getSaveLoad()->hasSavegame(index + _startIndex + 6)) {
						_dword_455C80 = true;
						getSaveLoad()->setIndex(index + _startIndex + 6);
					}
				}
			}
		} else if (cursor.x >= 30) {
			sprintf((char *)&text, "%d. %s ", index + _startIndex + 1, getSaveLoad()->getName(index + _startIndex).c_str());

			if (cursor.x <= (350 + getText()->getWidth((char *)&text))
				&& cursor.y >= y
				&& cursor.y <= (y + 24)) {
					if (index + _startIndex < 25) {
						if (getSaveLoad()->hasSavegame(index + _startIndex)) {
							_dword_455C80 = true;
							getSaveLoad()->setIndex(index + _startIndex);
						}
					}
			}
		}

		++index;
	}
}

void Menu::clickViewMovies() {
	Common::Point cursor = getCursor()->position();

	if (_dword_455C78)
		return;

	//////////////////////////////////////////////////////////////////////////
	// Previous page
	if (cursor.x >= 30  && cursor.x <= (30 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1353)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		if (_startIndex) {
			_startIndex -= 12;
			if (_startIndex < 0)
				_startIndex = 0;
		}

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Main Menu
	if (cursor.x >= 300 && cursor.x <= (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1355)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		leave();
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Next page
	if (cursor.x >= 550 && cursor.x <= (550 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1354)))
	 && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		if (_startIndex + 12 < (int32)_movieCount) {
			_startIndex += 12;
			if (_startIndex >= (int32)_movieCount)
				_startIndex = _movieCount - 1;
		}

		return;
	}

	char text[100];
	char text2[100];

	//////////////////////////////////////////////////////////////////////////
	// Columns
	uint32 index = 0;
	for (int32 y = 150; y < 324; y += 29) {
		if (cursor.x >= 350) {
			if (_movieList[index + _startIndex + 6] == -1)
				break;

			sprintf((char *)&text, "%d. %s ", index + 1, getText()->get(MAKE_RESOURCE(kResourcePackText, 1359 + _movieList[index])));
			sprintf((char *)&text2, getText()->get(MAKE_RESOURCE(kResourcePackText, 1356)), moviesCd[_movieList[index]]);
			strcat((char *)&text, (char *)&text2);

			if (cursor.x <= (350 + getText()->getWidth((char *)&text))
			 && cursor.y >= y
			 && cursor.y <= (y + 24)) {
				if (index + _startIndex  + 6 <= _movieCount) {
					// The original checks for the proper cd, but we can skip that since we have all data on disk
					_movieIndex = _movieList[index + _startIndex  + 6];

					//if (moviesCd[_movieIndex] != getSharedData()->cdNumber) {
					//	_dword_455C78 = true;
					//	getCursor()->hide();
					//} else {
						_showMovie = true;
					//}
				}
			}
		} else if (cursor.x >= 30) {
			if (_movieList[index + _startIndex] == -1)
				break;

			sprintf((char *)&text, "%d. %s ", index + 1, getText()->get(MAKE_RESOURCE(kResourcePackText, 1359 + _movieList[index])));
			sprintf((char *)&text2, getText()->get(MAKE_RESOURCE(kResourcePackText, 1356)), moviesCd[_movieList[index]]);
			strcat((char *)&text, (char *)&text2);

			if (cursor.x <= (30 + getText()->getWidth((char *)&text))
			 && cursor.y >= y
			 && cursor.y <= (y + 24)) {
				if (index + _startIndex < _movieCount) {
					// The original checks for the proper cd, but we can skip that since we have all data on disk
					_movieIndex = _movieList[index + _startIndex];

					//if (moviesCd[_movieIndex] != getSharedData()->cdNumber) {
					//	_dword_455C78 = true;
					//	getCursor()->hide();
					//} else {
						_showMovie = true;
					//}
				}
			}
		}

		++index;
	}
}

void Menu::clickQuitGame() {
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

void Menu::clickTextOptions() {
	Common::Point cursor = getCursor()->position();

	if (cursor.x < 350 || cursor.x > (350 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, Config.showMovieSubtitles ? 1414 : 1415))) || cursor.y < 150 || cursor.y > 174) {
		if (cursor.x < 350 || cursor.x > (350 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, Config.showEncounterSubtitles ? 1414 : 1415))) || cursor.y < 179 || cursor.y > 203) {
			if (cursor.x >= 300 && cursor.x <= (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1416))) && cursor.y >= 340 && cursor.y <= (340 + 24)) {
				Config.write();
				leave();
			}
		} else {
			Config.showEncounterSubtitles = !Config.showEncounterSubtitles;
		}
	} else {
		Config.showMovieSubtitles = !Config.showMovieSubtitles;
	}
}

void Menu::clickAudioOptions() {
	Common::Point cursor = getCursor()->position();

	// Size of - and +
	int32 sizeMinus	= getText()->getWidth("-");
	int32 sizePlus  = getText()->getWidth("+");

	if (cursor.x >= 220 && cursor.x <= (220 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1430))) && cursor.y >= 360 && cursor.y <= (360 + 24)) {
		stopTestSounds();
		Config.write();
		_vm->syncSoundSettings();
		leave();
		return;
	}

	if (cursor.x < 360 || cursor.x > (360 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1431))) || cursor.y < 360 || cursor.y > (360 + 24)) {
		int32 volumeIndex = 0;
		int32 defaultVolume = 0;
		int32 *volume = &defaultVolume;
		bool found = false;

		for (;;) {
			if (found)
				return;

			switch (volumeIndex) {
			default:
				break;

			case 0:
				if (cursor.x >= 350 && cursor.x <= (350 + sizeMinus) && cursor.y >= 150 && cursor.y <= 174) {
					adjustMasterVolume(-1);
					adjustTestVolume();
					found = true;
					break;
				}

				if (cursor.x >= (360 + sizeMinus) && cursor.x <= (360 + sizeMinus + sizePlus) && cursor.y >= 150 && cursor.y <= 174) {
					adjustMasterVolume(1);
					adjustTestVolume();
					found = true;
					break;
				}
				break;

			case 1:
				volume = &Config.musicVolume;
				break;

			case 2:
				volume = &Config.ambientVolume;
				break;

			case 3:
				volume = &Config.sfxVolume;
				break;

			case 4:
				volume = &Config.voiceVolume;
				break;

			case 5:
				volume = &Config.movieVolume;
				break;
			}

			if (!found) {
				if (cursor.x < 350 || cursor.x > (350 + sizeMinus) || cursor.y < (29 * volumeIndex + 150) || cursor.y > (29 * (volumeIndex + 6))) {
					if (cursor.x >= (sizeMinus + 360)) {
						if (cursor.x <= (sizeMinus + sizePlus + 360)) {

							if (cursor.y >= (29 * volumeIndex + 150) && cursor.y <= (29 * (volumeIndex + 6))) {
								// Normalize volume
								if (*volume < 0) {
									if (*volume == -9999)
										*volume = -5000;

									*volume += 250;

									if (*volume > 0)
										*volume = 0;

									adjustTestVolume();
								}
								found = true;
							}
						}
					}
				} else {
					if (*volume > -5000) {
						*volume -= 250;

						if (*volume <= -5000)
							*volume = -9999;

						adjustTestVolume();
					}
					found = true;
				}
			}

			++volumeIndex;

			if (volumeIndex >= 6) {
				if (!found) {
					if (cursor.x >= 350 && cursor.x <= (350 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, Config.reverseStereo ? 1428 : 1429)))
					 && cursor.y >= (29 * volumeIndex + 150) && cursor.y <= (29 * (volumeIndex + 6))) {
						Config.reverseStereo = !Config.reverseStereo;
						_vm->updateReverseStereo();
					}
				}

				return;
			}
		}
	}

	if (_testSoundsPlaying)
		stopTestSounds();
	else
		playTestSounds();
}

void Menu::clickSettings() {
	Common::Point cursor = getCursor()->position();

	// Size of - and +
	int32 sizeMinus	= getText()->getWidth("-");
	int32 sizePlus  = getText()->getWidth("+");

	//////////////////////////////////////////////////////////////////////////
	// Back to main menu
	if (cursor.x >= 300 && cursor.x <= (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1437))) && cursor.y >= 340 && cursor.y <= (340 + 24)) {
		Config.write();
		leave();
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Performance minus
	if (cursor.x >= 350 && cursor.x <= (sizeMinus + 350) && cursor.y >= 179 && cursor.y <= 203) {
		if (!Config.performance)
			return;

		Config.performance -= 1;
		adjustPerformance();

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Performance plus
	if (cursor.x >= sizeMinus + 360 && cursor.x <= (sizeMinus + sizePlus + 360) && cursor.y >= 179 && cursor.y <= 203) {
		if (Config.performance >= 5)
			return;

		Config.performance += 1;
		adjustPerformance();

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Gamma level minus
	if (cursor.x >= 350 && cursor.x <= (sizeMinus + 350) && cursor.y >= 150 && cursor.y <= 174) {
		if (!Config.gammaLevel)
			return;

		Config.gammaLevel -= 1;
		getScreen()->setGammaLevel(MAKE_RESOURCE(kResourcePackShared, 17), 0);

		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// Gamma level plus
	if (cursor.x >= (sizeMinus + 360) && cursor.x <= (sizeMinus + sizePlus + 360) && cursor.y >= 150 && cursor.y <= 174) {
		if (Config.gammaLevel >= 8)
			return;


		Config.gammaLevel += 1;
		getScreen()->setGammaLevel(MAKE_RESOURCE(kResourcePackShared, 17), 0);

		return;
	}
}

void Menu::clickKeyboardConfig() {
	Common::Point cursor = getCursor()->position();

	if (cursor.x < 300 || cursor.x > (300 + getText()->getWidth(MAKE_RESOURCE(kResourcePackText, 1446))) || cursor.y < 340 || cursor.y > (340 + 24)) {
		int32 keyIndex = 0;
		char keyCode = 0;

		do {
			switch (keyIndex) {
			default:
				break;

			case 0:
				keyCode = Config.keyShowVersion;
				break;

			case 1:
				keyCode = Config.keyQuickLoad;
				break;

			case 2:
				keyCode = Config.keyQuickSave;
				break;

			case 3:
				keyCode = Config.keySwitchToSara;
				break;

			case 4:
				keyCode = Config.keySwitchToGrimwall;
				break;

			case 5:
				keyCode = Config.keySwitchToOlmec;
				break;
			}

			if (cursor.x >= 350 && cursor.x <= (350 + getText()->getWidth(keyCode)) && cursor.y >= (29 * keyIndex + 150) && cursor.y <= (29 * (keyIndex + 6))) {
				_selectedShortcutIndex = keyIndex;
				getCursor()->hide();
			}

			++keyIndex;
		} while (keyIndex < 6);
	} else {
		Config.write();
		leave();
	}
}

void Menu::clickReturnToGame() {
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

void Menu::clickShowCredits() {
	closeCredits();
}

//////////////////////////////////////////////////////////////////////////
// Key handlers
//////////////////////////////////////////////////////////////////////////
void Menu::keySaveGame(const AsylumEvent &evt) {
	if (!_dword_455DD8)
		return;

	switch (evt.kbd.keycode) {
	default:
		if (evt.kbd.ascii > 255 || !isalnum(evt.kbd.ascii))
			break;

		if (getSaveLoad()->getName()->size() < 44) {
			int32 width = getText()->getWidth(getSaveLoad()->getName()->c_str());

			bool test = false;
			if ((getSaveLoad()->getIndex() % 12) >= 6)
				test = (width + _prefixWidth + 350 < 630);
			else
				test = (width + _prefixWidth + 30 < 340);

			if (test)
				*getSaveLoad()->getName() += (char)evt.kbd.ascii;
		}
		break;

	case Common::KEYCODE_RETURN:
		_dword_455DD8 = false;
		getSaveLoad()->save();
		break;

	case Common::KEYCODE_ESCAPE:
		_dword_455C80 = false;
		_dword_455DD8 = false;
		*getSaveLoad()->getName() = _previousName;
		getCursor()->show();
		break;

	case Common::KEYCODE_BACKSPACE:
	case Common::KEYCODE_DELETE:
		if (getSaveLoad()->getName()->size())
			getSaveLoad()->getName()->deleteLastChar();
		break;

	case Common::KEYCODE_KP_PERIOD:
		*getSaveLoad()->getName() = "";
		break;
	}
}

void Menu::keyKeyboardConfig(const AsylumEvent &evt) {
	if (_selectedShortcutIndex == -1)
		return;

	if (evt.kbd.keycode == Common::KEYCODE_ESCAPE || evt.kbd.keycode == Common::KEYCODE_RETURN || evt.kbd.keycode == Common::KEYCODE_KP_ENTER) {
		_selectedShortcutIndex = -1;
		getCursor()->show();
		return;
	}

	char *keyCode = NULL;
	switch(_selectedShortcutIndex) {
	default:
		break;

	case 0:
		keyCode = &Config.keyShowVersion;
		break;

	case 1:
		keyCode = &Config.keyQuickLoad;
		break;

	case 2:
		keyCode = &Config.keyQuickSave;
		break;

	case 3:
		keyCode = &Config.keySwitchToSara;
		break;

	case 4:
		keyCode = &Config.keySwitchToGrimwall;
		break;

	case 5:
		keyCode = &Config.keySwitchToOlmec;
		break;
	}

	// Check for alphanumeric character
	if (evt.kbd.ascii > 255 || !isalnum(evt.kbd.ascii))
		return;

	if (!Config.isKeyAssigned(evt.kbd.ascii) || *keyCode == evt.kbd.ascii) {
		*keyCode = evt.kbd.ascii;
		_selectedShortcutIndex = -1;
		getCursor()->show();
	}
}

void Menu::keyShowCredits() {
	closeCredits();
}

} // end of namespace Asylum

