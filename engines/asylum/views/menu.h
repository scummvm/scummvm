/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_VIEWS_MENU_H
#define ASYLUM_VIEWS_MENU_H

#include "graphics/surface.h"

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;
class Cursor;
class GraphicResource;
class ResourcePack;
class Scene;
class Text;

class Menu : public EventHandler {
public:
	Menu(AsylumEngine *vm);
	virtual ~Menu() {};

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

	/**
	 * Sets the game as started.
	 */
	void setGameStarted() { _gameStarted = true; }

	// Savegames
	void setDword455C78(bool state) { _dword_455C78 = state; }
	void setDword455C80(bool state) { _dword_455C80 = state; }
	void setLoadingDuringStartup()  { _loadingDuringStartup = true; }

	bool isEditingSavegameName() { return _activeScreen == kMenuSaveGame && _isEditingSavegameName; };
	bool isConfiguringKeyboard() { return _activeScreen == kMenuKeyboardConfig && _selectedShortcutIndex != -1; }

private:
	AsylumEngine *_vm;

	enum MenuScreen {
		kMenuNone            = -1,
		kMenuNewGame         = 0,
		kMenuLoadGame        = 1,
		kMenuSaveGame        = 2,
		kMenuDeleteGame      = 3,
		kMenuViewMovies      = 4,
		kMenuQuitGame        = 5,
		kMenuTextOptions     = 6,
		kMenuAudioOptions    = 7,
		kMenuSettings        = 8,
		kMenuKeyboardConfig  = 9,
		kMenuReturnToGame    = 10,
		kMenuShowCredits     = 11
	};

	enum MenuResource {
		kMenuResourceNone = 0,
		kBackground   = MAKE_RESOURCE(kResourcePackShared, 0),
		kEye          = MAKE_RESOURCE(kResourcePackShared, 1),
		kFontBlue     = MAKE_RESOURCE(kResourcePackShared, 22),
		kFontYellow   = MAKE_RESOURCE(kResourcePackShared, 16),
		kSfxSound     = MAKE_RESOURCE(kResourcePackShared, 41),
		kAmbientSound = MAKE_RESOURCE(kResourcePackShared, 42),
		kVoiceSound   = MAKE_RESOURCE(kResourcePackShared, 43)
	};

	// Game initialization
	bool             _initGame;

	// Data
	MenuScreen       _activeScreen;
	ResourceId       _soundResourceId;
	ResourceId       _musicResourceId;
	bool             _gameStarted;
	MenuScreen       _currentIcon;
	int32            _selectedShortcutIndex;
	int32            _dword_455C74;
	bool             _dword_455C78;
	bool             _dword_455C80;
	bool             _dword_455D4C;
	bool             _dword_455D5C;
	bool             _isEditingSavegameName;
	bool             _testSoundsPlaying;
	int32            _dword_456288;
	int32            _caretBlink;
	int32            _startIndex;
	int32            _creditsFrameIndex;
	int32            _creditsNumSteps;
	bool             _showMovie;
	uint32           _iconFrames[12];

	// Movies
	int32  _movieList[196];
	uint32 _movieCount;
	uint32 _movieIndex;

	// Savegames
	Common::String _previousName;
	int32 _prefixWidth;
	bool _loadingDuringStartup;

	// Thumbnails
	int _thumbnailIndex;
	Graphics::Surface _thumbnailSurface;

	/**
	 * Setups menu screen
	 */
	void setup();

	/**
	 * Leaves an opened menu
	 */
	void leave();

	/**
	 * Switch between fonts.
	 *
	 * @param condition if true, load kFontYellow, if false, load kFontBlue.
	 */
	void switchFont(bool condition);

	/**
	 * Close the credits.
	 */
	void closeCredits();

	/**
	 * Sets up the music.
	 */
	void setupMusic();

	/**
	 * Find if the mouse if on an icon
	 *
	 * @return The icon identifier
	 */
	MenuScreen findMousePosition();

	/**
	 * Play test sounds
	 */
	void playTestSounds();

	/**
	 * Stop test sounds.
	 */
	void stopTestSounds();

	/**
	 * Adjust volume.
	 *
	 * @param delta The delta.
	 */
	void adjustMasterVolume(int32 delta) const;

	/**
	 * Adjust test sounds volume.
	 */
	void adjustTestVolume();

	/**
	 * Adjust performance.
	 */
	void adjustPerformance();

	/**
	 * Gets the chapter name.
	 *
	 * @return The chapter name.
	 */
	Common::String getChapterName();

	//////////////////////////////////////////////////////////////////////////
	// Message handling
	bool init();
	bool update();
	bool music();
	bool key(const AsylumEvent &evt);
	bool click(const AsylumEvent &evt);

	// Thumbnails
	bool hasThumbnail(int index);
	void readThumbnail();
	void showThumbnail();

	// Update handlers
	void updateNewGame();
	void updateLoadGame();
	void updateSaveGame();
	void updateDeleteGame();
	void updateViewMovies();
	void updateQuitGame();
	void updateTextOptions();
	void updateAudioOptions();
	void updateSettings();
	void updateKeyboardConfig();
	void updateReturnToGame();
	void updateShowCredits();

	// Click handlers
	void clickNewGame();
	void clickLoadGame();
	void clickSaveGame();
	void clickDeleteGame();
	void clickViewMovies();
	void clickQuitGame();
	void clickTextOptions();
	void clickAudioOptions();
	void clickSettings();
	void clickKeyboardConfig();
	void clickReturnToGame();
	void clickShowCredits();

	// Key handlers
	void keySaveGame(const AsylumEvent &evt);
	void keyKeyboardConfig(const AsylumEvent &evt);
	void keyShowCredits();

}; // end of class MainMenu

} // end of namespace Asylum

#endif // ASYLUM_VIEWS_MENU_H
