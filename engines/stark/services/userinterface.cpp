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

#include "common/events.h"
#include "common/stream.h"
#include "common/system.h"

#include "engines/engine.h"

#include "engines/stark/services/userinterface.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/services/diary.h"
#include "engines/stark/services/gameinterface.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/settings.h"

#include "engines/stark/ui/cursor.h"
#include "engines/stark/ui/dialogbox.h"
#include "engines/stark/ui/menu/diaryindex.h"
#include "engines/stark/ui/menu/mainmenu.h"
#include "engines/stark/ui/menu/settingsmenu.h"
#include "engines/stark/ui/menu/saveloadmenu.h"
#include "engines/stark/ui/menu/fmvmenu.h"
#include "engines/stark/ui/menu/diarypages.h"
#include "engines/stark/ui/menu/dialogmenu.h"
#include "engines/stark/ui/world/inventorywindow.h"
#include "engines/stark/ui/world/fmvscreen.h"
#include "engines/stark/ui/world/gamescreen.h"
#include "engines/stark/ui/world/gamewindow.h"
#include "engines/stark/ui/world/dialogpanel.h"

#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/item.h"

#include "gui/message.h"

namespace Stark {

UserInterface::UserInterface(Gfx::Driver *gfx) :
		_gfx(gfx),
		_cursor(nullptr),
		_diaryIndexScreen(nullptr),
		_mainMenuScreen(nullptr),
		_settingsMenuScreen(nullptr),
		_saveMenuScreen(nullptr),
		_loadMenuScreen(nullptr),
		_fmvMenuScreen(nullptr),
		_diaryPagesScreen(nullptr),
		_dialogScreen(nullptr),
		_exitGame(false),
		_quitToMainMenu(false),
		_shouldToggleSubtitle(false),
		_shouldGoBackToPreviousScreen(false),
		_fmvScreen(nullptr),
		_gameScreen(nullptr),
		_interactive(true),
		_interactionAttemptDenied(false),
		_currentScreen(nullptr),
		_gameWindowThumbnail(nullptr),
		_modalDialog(nullptr) {
}

UserInterface::~UserInterface() {
	freeGameScreenThumbnail();

	delete _modalDialog;
	delete _gameScreen;
	delete _fmvScreen;
	delete _diaryIndexScreen;
	delete _cursor;
	delete _mainMenuScreen;
	delete _settingsMenuScreen;
	delete _saveMenuScreen;
	delete _loadMenuScreen;
	delete _fmvMenuScreen;
	delete _diaryPagesScreen;
	delete _dialogScreen;
}

void UserInterface::init() {
	_cursor = new Cursor(_gfx);

	_mainMenuScreen = new MainMenuScreen(_gfx, _cursor);
	_gameScreen = new GameScreen(_gfx, _cursor);
	_diaryIndexScreen = new DiaryIndexScreen(_gfx, _cursor);
	_settingsMenuScreen = new SettingsMenuScreen(_gfx, _cursor);
	_saveMenuScreen = new SaveMenuScreen(_gfx, _cursor);
	_loadMenuScreen = new LoadMenuScreen(_gfx, _cursor);
	_fmvMenuScreen = new FMVMenuScreen(_gfx, _cursor);
	_diaryPagesScreen = new DiaryPagesScreen(_gfx, _cursor);
	_dialogScreen = new DialogScreen(_gfx, _cursor);
	_fmvScreen = new FMVScreen(_gfx, _cursor);
	_modalDialog = new DialogBox(_gfx, _cursor);

	_prevScreenNameStack.push(Screen::kScreenMainMenu);
	_currentScreen = _fmvScreen;

	// Play the FunCom logo video
	_fmvScreen->play("1402.bbb");
}

void UserInterface::onGameLoop() {
	StarkStaticProvider->onGameLoop();

	if (_modalDialog->isVisible()) {
		_modalDialog->handleGameLoop();
		_modalDialog->handleMouseMove();
	} else {
		_currentScreen->handleGameLoop();

		// Check for UI mouse overs
		// TODO: Call mouse move only if the mouse position actually changed
		//  probably some code will need to be moved to gameloop handling to
		//  account for the case where hotspots move and the mouse cursor needs
		//  to be updated regardless of the mouse actually moved.
		_currentScreen->handleMouseMove();
	}
}

void UserInterface::handleMouseMove(const Common::Point &pos) {
	_cursor->setMousePosition(pos);
}

void UserInterface::handleMouseUp() {
	// Only the settings menu needs to handle this event
	// TODO: Clean this up
	_settingsMenuScreen->handleMouseUp();
}

void UserInterface::handleClick() {
	if (_modalDialog->isVisible()) {
		_modalDialog->handleClick();
	} else {
		_currentScreen->handleClick();
	}
}

void UserInterface::handleRightClick() {
	if (_modalDialog->isVisible()) {
		_modalDialog->handleRightClick();
	} else {
		_currentScreen->handleRightClick();
	}
}

void UserInterface::handleDoubleClick() {
	if (_modalDialog->isVisible()) {
		_modalDialog->handleDoubleClick();
	} else {
		_currentScreen->handleDoubleClick();
	}
}

void UserInterface::handleEscape() {
	if (StarkGameInterface->skipCurrentSpeeches()) {
		return;
	}

	if (skipFMV()) {
		return;
	}

	Screen::Name curScreenName = _currentScreen->getName();
	if (curScreenName != Screen::kScreenGame && curScreenName != Screen::kScreenMainMenu) {
		backPrevScreen();
	} else if (StarkSettings->getBoolSetting(Settings::kTimeSkip)) {
		StarkGlobal->setFastForward();
	}
}

void UserInterface::inventoryOpen(bool open) {
	// Make the inventory update its contents.
	if (open) {
		_gameScreen->getInventoryWindow()->open();
	} else {
		_gameScreen->getInventoryWindow()->close();
	}
}

int16 UserInterface::getSelectedInventoryItem() const {
	if (_gameScreen) {
		return _gameScreen->getInventoryWindow()->getSelectedInventoryItem();
	} else {
		return -1;
	}
}

void UserInterface::selectInventoryItem(int16 itemIndex) {
	_gameScreen->getInventoryWindow()->setSelectedInventoryItem(itemIndex);
}

void UserInterface::requestFMVPlayback(const Common::String &name) {
	_shouldPlayFmv = name;
}

void UserInterface::onFMVStopped() {
	_shouldGoBackToPreviousScreen = true;
}

void UserInterface::changeScreen(Screen::Name screenName) {
	if (screenName == _currentScreen->getName()) {
		return;
	}

	_prevScreenNameStack.push(_currentScreen->getName());
	_currentScreen->close();
	_currentScreen = getScreenByName(screenName);
	_currentScreen->open();
}

void UserInterface::backPrevScreen() {
	// No need to check the stack since at least there will be a MainMenuScreen in it
	// and MainMenuScreen will not request to go back
	changeScreen(_prevScreenNameStack.pop());

	// No need to push for going back
	_prevScreenNameStack.pop();
}

void UserInterface::restoreScreenHistory() {
	_shouldGoBackToPreviousScreen = false;
	_prevScreenNameStack.clear();
	_prevScreenNameStack.push(Screen::kScreenMainMenu);
}

Screen *UserInterface::getScreenByName(Screen::Name screenName) const {
	switch (screenName) {
		case Screen::kScreenFMV:
			return _fmvScreen;
		case Screen::kScreenDiaryIndex:
			return _diaryIndexScreen;
		case Screen::kScreenGame:
			return _gameScreen;
		case Screen::kScreenMainMenu:
			return _mainMenuScreen;
		case Screen::kScreenSettingsMenu:
			return _settingsMenuScreen;
		case Screen::kScreenSaveMenu:
			return _saveMenuScreen;
		case Screen::kScreenLoadMenu:
			return _loadMenuScreen;
		case Screen::kScreenFMVMenu:
			return _fmvMenuScreen;
		case Screen::kScreenDiaryPages:
			return _diaryPagesScreen;
		case Screen::kScreenDialog:
			return _dialogScreen;
		default:
			error("Unhandled screen name '%d'", screenName);
	}
}

bool UserInterface::isInGameScreen() const {
	return _currentScreen && (_currentScreen->getName() == Screen::kScreenGame);
}

bool UserInterface::isInSaveLoadMenuScreen() const {
	Screen::Name name = _currentScreen->getName();
	return name == Screen::kScreenSaveMenu || name == Screen::kScreenLoadMenu;
}

bool UserInterface::isInDiaryIndexScreen() const {
	return _currentScreen->getName() == Screen::kScreenDiaryIndex;
}

bool UserInterface::isInventoryOpen() const {
	return _gameScreen->getInventoryWindow()->isVisible();
}

bool UserInterface::skipFMV() {
	if (_currentScreen->getName() == Screen::kScreenFMV) {
		_fmvScreen->stop();
		return true;
	}
	return false;
}

void UserInterface::render() {
	_currentScreen->render();

	if (_modalDialog->isVisible()) {
		_modalDialog->render();
	}

	// The cursor depends on the UI being done.
	if (_currentScreen->getName() != Screen::kScreenFMV) {
		_cursor->render();
	}
}

bool UserInterface::isInteractive() const {
	return _interactive;
}

void UserInterface::setInteractive(bool interactive) {
	if (interactive && !_interactive) {
		StarkGlobal->setNormalSpeed();
	} else if (!interactive && _interactive) {
		_interactionAttemptDenied = false;
	}

	_interactive = interactive;
}

void UserInterface::markInteractionDenied() {
	if (!_interactive) {
		_interactionAttemptDenied = true;
	}
}

bool UserInterface::wasInteractionDenied() const {
	return !_interactive && _interactionAttemptDenied;
}

void UserInterface::clearLocationDependentState() {
	_gameScreen->reset();
}

void UserInterface::optionsOpen() {
	changeScreen(Screen::kScreenDiaryIndex);
}

void UserInterface::saveGameScreenThumbnail() {
	freeGameScreenThumbnail();

	if (StarkGlobal->getLevel() && StarkGlobal->getCurrent()) {
		// Re-render the screen to exclude the cursor
		StarkGfx->clearScreen();
		_gameScreen->render();
	}

	Graphics::Surface *big = _gameScreen->getGameWindow()->getScreenshot();
	assert(big->format.bytesPerPixel == 4);

	_gameWindowThumbnail = new Graphics::Surface();
	_gameWindowThumbnail->create(kThumbnailWidth, kThumbnailHeight, big->format);

	uint32 *dst = (uint32 *)_gameWindowThumbnail->getPixels();
	for (uint i = 0; i < _gameWindowThumbnail->h; i++) {
		for (uint j = 0; j < _gameWindowThumbnail->w; j++) {
			uint32 srcX = big->w * j / _gameWindowThumbnail->w;
			uint32 srcY = big->h * i / _gameWindowThumbnail->h;
			uint32 *src = (uint32 *)big->getBasePtr(srcX, srcY);

			// Copy RGBA pixel
			*dst++ = *src;
		}
	}

	big->free();
	delete big;
}

void UserInterface::freeGameScreenThumbnail() {
	if (_gameWindowThumbnail) {
		_gameWindowThumbnail->free();
		delete _gameWindowThumbnail;
		_gameWindowThumbnail = nullptr;
	}
}

const Graphics::Surface *UserInterface::getGameWindowThumbnail() const {
	return _gameWindowThumbnail;
}

void UserInterface::onScreenChanged() {
	_gameScreen->onScreenChanged();

	if (_modalDialog->isVisible()) {
		_modalDialog->onScreenChanged();
	}

	if (!isInGameScreen()) {
		_currentScreen->onScreenChanged();
	}
}

void UserInterface::notifyInventoryItemEnabled(uint16 itemIndex) {
	_gameScreen->notifyInventoryItemEnabled(itemIndex);
}

void UserInterface::notifyDiaryEntryEnabled() {
	_gameScreen->notifyDiaryEntryEnabled();
}

void UserInterface::toggleScreen(Screen::Name screenName) {
	Screen::Name currentName = _currentScreen->getName();

	if (currentName == screenName
			|| (currentName == Screen::kScreenSaveMenu && screenName == Screen::kScreenLoadMenu)
			|| (currentName == Screen::kScreenLoadMenu && screenName == Screen::kScreenSaveMenu)) {
		backPrevScreen();
	} else if (currentName == Screen::kScreenGame 
			|| currentName == Screen::kScreenDiaryIndex
			|| (currentName == Screen::kScreenMainMenu && screenName == Screen::kScreenLoadMenu)
			|| (currentName == Screen::kScreenMainMenu && screenName == Screen::kScreenSettingsMenu)) {
		changeScreen(screenName);
	}
}

void UserInterface::performToggleSubtitle() {
	StarkSettings->flipSetting(Settings::kSubtitle);
	_shouldToggleSubtitle = false;
}

void UserInterface::cycleInventory(bool forward) {
	int16 curItem = getSelectedInventoryItem();
	int16 nextItem = StarkGlobal->getInventory()->getNeighborInventoryItem(curItem, forward);
	selectInventoryItem(nextItem);
}

void UserInterface::doQueuedScreenChange() {
	if (_quitToMainMenu) {
		clearLocationDependentState();
		changeScreen(Screen::kScreenGame);
		StarkResourceProvider->shutdown();
		changeScreen(Screen::kScreenMainMenu);
		_prevScreenNameStack.clear();
		_quitToMainMenu = false;
	}

	if (_shouldGoBackToPreviousScreen) {
		backPrevScreen();
		_shouldGoBackToPreviousScreen = false;
	}

	if (!_shouldPlayFmv.empty()) {
		changeScreen(Screen::kScreenFMV);
		_fmvScreen->play(_shouldPlayFmv);
		_shouldPlayFmv.clear();
	}
}

void UserInterface::handleKeyPress(const Common::KeyState &keyState) {
	if (_modalDialog->isVisible()) {
		_modalDialog->onKeyPress(keyState);
		return;
	}

	// TODO: Delegate keypress handling to the screens

	if (keyState.keycode == Common::KEYCODE_ESCAPE) {
		handleEscape();
	} else if ((keyState.keycode == Common::KEYCODE_RETURN
				|| keyState.keycode == Common::KEYCODE_KP_ENTER)) {
		if (isInGameScreen()) {
			_gameScreen->getDialogPanel()->selectFocusedOption();
		}
	} else if (keyState.keycode == Common::KEYCODE_F1) {
		toggleScreen(Screen::kScreenDiaryIndex);
	} else if (keyState.keycode == Common::KEYCODE_F2) {
		if (isInSaveLoadMenuScreen() || g_engine->canSaveGameStateCurrently()) {
			toggleScreen(Screen::kScreenSaveMenu);
		}
	} else if (keyState.keycode == Common::KEYCODE_F3) {
		toggleScreen(Screen::kScreenLoadMenu);
	} else if (keyState.keycode == Common::KEYCODE_F4) {
		toggleScreen(Screen::kScreenDialog);
	} else if (keyState.keycode == Common::KEYCODE_F5) {
		if (StarkDiary->isEnabled()) {
			toggleScreen(Screen::kScreenDiaryPages);
		}
	} else if (keyState.keycode == Common::KEYCODE_F6) {
		toggleScreen(Screen::kScreenFMVMenu);
	} else if (keyState.keycode == Common::KEYCODE_F7) {
		toggleScreen(Screen::kScreenSettingsMenu);
	} else if (keyState.keycode == Common::KEYCODE_F8) {
		g_system->saveScreenshot();
	} else if (keyState.keycode == Common::KEYCODE_F9) {
		if (isInGameScreen()) {
			_shouldToggleSubtitle = !_shouldToggleSubtitle;
		}
	} else if (keyState.keycode == Common::KEYCODE_F10) {
		if (isInGameScreen() || isInDiaryIndexScreen()) {
			confirm(GameMessage::kQuitGamePrompt, this, &UserInterface::requestQuitToMainMenu);
		}
	} else if (keyState.keycode == Common::KEYCODE_a) {
		if (isInGameScreen() && isInteractive()) {
			cycleInventory(false);
		}
	} else if (keyState.keycode == Common::KEYCODE_s) {
		if (isInGameScreen() && isInteractive()) {
			cycleInventory(true);
		}
	} else if (keyState.keycode == Common::KEYCODE_i) {
		if (isInGameScreen() && isInteractive()) {
			inventoryOpen(!isInventoryOpen());
		}
	} else if (keyState.keycode == Common::KEYCODE_x
				&& !keyState.hasFlags(Common::KBD_ALT)) {
		if (isInGameScreen() && isInteractive()) {
			_gameScreen->getGameWindow()->toggleExitDisplay();
		}
	} else if ((keyState.keycode == Common::KEYCODE_x
				|| keyState.keycode == Common::KEYCODE_q)
				&& keyState.hasFlags(Common::KBD_ALT)) {
		confirm(GameMessage::kQuitPrompt, this, &UserInterface::notifyShouldExit);
	} else if (keyState.keycode == Common::KEYCODE_p) {
		if (isInGameScreen()) {
			if (g_engine->isPaused()) {
				_gamePauseToken.clear();
			} else {
				_gamePauseToken = g_engine->pauseEngine();
				debug("The game is paused");
			}
		}
	} else if (keyState.keycode == Common::KEYCODE_PAGEUP) {
		if (isInGameScreen()) {
			if (isInventoryOpen()) {
				_gameScreen->getInventoryWindow()->scrollUp();
			} else {
				_gameScreen->getDialogPanel()->scrollUp();
			}
		}
	} else if (keyState.keycode == Common::KEYCODE_UP) {
		if (isInGameScreen()) {
			if (isInventoryOpen()) {
				_gameScreen->getInventoryWindow()->scrollUp();
			} else {
				_gameScreen->getDialogPanel()->focusPrevOption();
			}
		}
	} else if (keyState.keycode == Common::KEYCODE_PAGEDOWN) {
		if (isInGameScreen()) {
			if (isInventoryOpen()) {
				_gameScreen->getInventoryWindow()->scrollDown();
			} else {
				_gameScreen->getDialogPanel()->scrollDown();
			}
		}
	} else if (keyState.keycode == Common::KEYCODE_DOWN) {
		if (isInGameScreen()) {
			if (isInventoryOpen()) {
				_gameScreen->getInventoryWindow()->scrollDown();
			} else {
				_gameScreen->getDialogPanel()->focusNextOption();
			}
		}
	} else if (keyState.keycode >= Common::KEYCODE_1 && keyState.keycode <= Common::KEYCODE_9) {
		if (isInGameScreen()) {
			uint index = keyState.keycode - Common::KEYCODE_1;
			_gameScreen->getDialogPanel()->selectOption(index);
		}
	}
}

void UserInterface::confirm(const Common::String &message, Common::Functor0<void> *confirmCallBack) {
	Common::String textYes = StarkGameMessage->getTextByKey(GameMessage::kYes);
	Common::String textNo = StarkGameMessage->getTextByKey(GameMessage::kNo);

	_modalDialog->open(message, confirmCallBack, textYes, textNo);
}

void UserInterface::confirm(GameMessage::TextKey key, Common::Functor0<void> *confirmCallBack) {
	Common::String message = StarkGameMessage->getTextByKey(key);

	confirm(message, confirmCallBack);
}

} // End of namespace Stark
