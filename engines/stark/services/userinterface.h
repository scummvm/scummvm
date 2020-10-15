/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_SERVICES_USER_INTERFACE_H
#define STARK_SERVICES_USER_INTERFACE_H

#include "engines/stark/ui/screen.h"

#include "engines/stark/services/gamemessage.h"

#include "engines/engine.h"

#include "common/keyboard.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "common/stack.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Graphics {
struct Surface;
}

namespace Stark {

namespace Gfx {
class Driver;
}

class DialogBox;
class DiaryIndexScreen;
class GameScreen;
class MainMenuScreen;
class SettingsMenuScreen;
class SaveMenuScreen;
class LoadMenuScreen;
class FMVMenuScreen;
class DiaryPagesScreen;
class DialogScreen;
class Cursor;
class FMVScreen;

enum {
	kThumbnailWidth = 160,
	kThumbnailHeight = 92,
	kThumbnailSize = kThumbnailWidth * kThumbnailHeight * 4
};

/**
 * Facade object for interacting with the user interface from the rest of the engine
 */
class UserInterface {
public:
	explicit UserInterface(Gfx::Driver *gfx);
	virtual ~UserInterface();

	void init();

	/** Called once per game loop. */
	void onGameLoop();

	void render();
	void handleMouseMove(const Common::Point &pos);
	void handleMouseUp();
	void handleClick();
	void handleRightClick();
	void handleDoubleClick();
	void handleEscape();
	void notifyShouldExit() { _exitGame = true; }
	void inventoryOpen(bool open);
	bool shouldExit() { return _exitGame; }

	/** Start playing a FMV */
	void requestFMVPlayback(const Common::String &name);

	/** FMV playback has just ended */
	void onFMVStopped();

	/**
	 * Abort the currently playing FMV, if any
	 *
	 * @return true if a FMV was skipped
	 */
	bool skipFMV();

	/** Set the currently displayed screen */
	void changeScreen(Screen::Name screenName);

	/** Back to the previous displayed screen */
	void backPrevScreen();

	/** Apply the scheduled screen change if any */
	void doQueuedScreenChange();

	/** Back to the main menu screen and rest resources */
	void requestQuitToMainMenu() { _quitToMainMenu = true; }

	/** Restore the screen travelling history to the initial state*/
	void restoreScreenHistory();

	/** Is the game screen currently displayed? */
	bool isInGameScreen() const;

	/** Is the save & load menu screen currently displayed? */
	bool isInSaveLoadMenuScreen() const;

	/** Is the diary index screen currently displayed? */
	bool isInDiaryIndexScreen() const;

	/** Is the inventory panel being displayed? */
	bool isInventoryOpen() const;

	/** Can the player interact with the game world? */
	bool isInteractive() const;

	/** Allow or forbid interaction with the game world */
	void setInteractive(bool interactive);

	/** A new item has been added to the player's inventory */
	void notifyInventoryItemEnabled(uint16 itemIndex);

	/** A new entry has been added to the player's diary */
	void notifyDiaryEntryEnabled();

	/** Access the selected inventory item */
	int16 getSelectedInventoryItem() const;
	void selectInventoryItem(int16 itemIndex);

	/** Clears all the pointers to data that may be location dependent */
	void clearLocationDependentState();

	/** Open the in game options menu */
	void optionsOpen();

	/** Signal a denied interaction that occurred during a non interactive period */
	void markInteractionDenied();

	/** Was a player interaction with the world denied during this non interactive period? */
	bool wasInteractionDenied() const;

	/** The screen resolution just changed, rebuild resolution dependent data */
	void onScreenChanged();

	/** Grab a screenshot of the game screen and store it in the class context as a thumbnail */
	void saveGameScreenThumbnail();

	/** Clear the currently stored game screen thumbnail, if any */
	void freeGameScreenThumbnail();

	/** Get the currently stored game screen thumbnail, returns nullptr if there is not thumbnail stored */
	const Graphics::Surface *getGameWindowThumbnail() const;

	/**
	 * Display a confirmation dialog
	 *
	 * Close the dialog when the cancel button is pressed,
	 * call a callback when the confirm button is pressed.
	 */
	template<class T>
	void confirm(const Common::String &message, T *instance, void (T::*confirmCallBack)());
	template<class T>
	void confirm(GameMessage::TextKey key, T *instance, void (T::*confirmCallBack)());
	void confirm(const Common::String &message, Common::Functor0<void> *confirmCallBack);
	void confirm(GameMessage::TextKey key, Common::Functor0<void> *confirmCallBack);

	/** Directly open or close a screen */
	void toggleScreen(Screen::Name screenName);

	/** Toggle subtitles on and off */
	bool hasToggleSubtitleRequest() { return _shouldToggleSubtitle; }
	void performToggleSubtitle();

	/** Perform an action after a keypress */
	void handleKeyPress(const Common::KeyState &keyState);

private:
	Screen *getScreenByName(Screen::Name screenName) const;

	void cycleInventory(bool forward);

	GameScreen *_gameScreen;
	FMVScreen *_fmvScreen;
	DiaryIndexScreen *_diaryIndexScreen;
	MainMenuScreen *_mainMenuScreen;
	SettingsMenuScreen *_settingsMenuScreen;
	SaveMenuScreen *_saveMenuScreen;
	LoadMenuScreen *_loadMenuScreen;
	FMVMenuScreen *_fmvMenuScreen;
	DiaryPagesScreen *_diaryPagesScreen;
	DialogScreen *_dialogScreen;
	Screen *_currentScreen;
	Common::Stack<Screen::Name> _prevScreenNameStack;

	DialogBox *_modalDialog;
	Cursor *_cursor;

	Gfx::Driver *_gfx;
	bool _exitGame;
	bool _quitToMainMenu;
	PauseToken _gamePauseToken;

	bool _interactive;
	bool _interactionAttemptDenied;

	bool _shouldToggleSubtitle;

	// TODO: Generalize to all screen changes
	bool _shouldGoBackToPreviousScreen;
	Common::String _shouldPlayFmv;

	Graphics::Surface *_gameWindowThumbnail;
};

template<class T>
void UserInterface::confirm(GameMessage::TextKey key, T *instance, void (T::*confirmCallBack)()) {
	confirm(key, new Common::Functor0Mem<void, T>(instance, confirmCallBack));
}

template<class T>
void UserInterface::confirm(const Common::String &message, T *instance, void (T::*confirmCallBack)()) {
	confirm(message, new Common::Functor0Mem<void, T>(instance, confirmCallBack));
}

} // End of namespace Stark

#endif // STARK_SERVICES_USER_INTERFACE_H
