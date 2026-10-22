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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "harvester/menu/menu.h"
#include "harvester/menu/menu_render.h"

#include "common/algorithm.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/scene/room_support.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const char *const kGameOverBitmapPath = "1:/GRAPHIC/OTHER/GAMEOVER.BM";
static const char *const kGameOverPalettePath = "1:/GRAPHIC/PAL/GAMEOVER.PAL";
static const char *const kGameOverMusicPath = "SOUND/MUSIC/ANXIETY.CMP";
static const char *const kBlankMenuSlot = " ";

class ScopedSceneTimerPause {
public:
	explicit ScopedSceneTimerPause(HarvesterEngine &engine) : _engine(engine) {
		pause();
	}

	~ScopedSceneTimerPause() {
		resume();
	}

	void pause() {
		if (_paused)
			return;
		if (EntityManager *entityManager = _engine.getRuntimeEntities()) {
			entityManager->pauseTimerCountdowns();
			_paused = true;
		}
	}

	void resume() {
		if (!_paused)
			return;
		if (EntityManager *entityManager = _engine.getRuntimeEntities())
			entityManager->resumeTimerCountdowns();
		_paused = false;
	}

private:
	HarvesterEngine &_engine;
	bool _paused = false;
};

enum MainMenuItem {
	kMainMenuItemNewGame = 0,
	kMainMenuItemSaveGame,
	kMainMenuItemLoadGame,
	kMainMenuItemOptions,
	kMainMenuItemHelp,
	kMainMenuItemQuitGame
};

static void buildDisplayMainMenuItems(const Common::Array<Common::String> &source,
		bool canSaveGame, bool canLoadGame, Common::Array<Common::String> &dest) {
	dest = source;
	if (dest.size() > 1 && !canSaveGame)
		dest[1] = kBlankMenuSlot;
	if (dest.size() > 2 && !canLoadGame)
		dest[2] = kBlankMenuSlot;
}

} // End of anonymous namespace

struct MenuSystem::MenuSelection {
	Common::Array<Common::String> items;
	int selectedItem = -1;
	bool needsRedraw = true;
};

struct MenuSystem::MainMenu {
	MainMenu(HarvesterEngine &engine, const Art &art, Flow &flow)
		: art(art), flow(flow), pausedTimers(engine) {}

	const Art &art;
	Flow &flow;
	ScopedSceneTimerPause pausedTimers;
	MenuSelection selection;
	Common::String statusMessage;
};

struct MenuSystem::RoomMenu {
	RoomMenu(const IndexedBitmap &backdrop, const byte *palette, float paletteBrightness, Flow &flow)
		: backdrop(backdrop), palette(palette), paletteBrightness(paletteBrightness), flow(flow) {}

	const IndexedBitmap &backdrop;
	const byte *palette;
	float paletteBrightness;
	Flow &flow;
	MenuSelection selection;
};

struct MenuSystem::RoomMenuActivationResult {
	Common::Error error;
	bool closeMenu;

	RoomMenuActivationResult(Common::Error e, bool c) : error(e), closeMenu(c) {}
};

MenuSystem::MenuNavigationCommand MenuSystem::handleMenuNavigationEvent(MenuSelection &selection,
		const Common::Event &event, bool useBackdrop) const {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE: {
		const int hoveredItem = useBackdrop
			? getBackdropMenuItemAt(_mousePos, selection.items)
			: getMenuItemAt(_mousePos, selection.items);
		if (hoveredItem != -1 && hoveredItem != selection.selectedItem) {
			selection.selectedItem = hoveredItem;
			selection.needsRedraw = true;
		}
		break;
	}
	case Common::EVENT_KEYDOWN:
		if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
			return kCloseMenu;

		if (selection.items.empty())
			break;

		if (event.kbd.keycode == Common::KEYCODE_UP) {
			selection.selectedItem = (selection.selectedItem + selection.items.size() - 1) % selection.items.size();
			selection.needsRedraw = true;
		} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
			selection.selectedItem = (selection.selectedItem + 1) % selection.items.size();
			selection.needsRedraw = true;
		} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
				event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
			return kActivateMenuItem;
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		if (selection.items.empty())
			break;

		selection.selectedItem = useBackdrop
			? getBackdropMenuItemAt(_mousePos, selection.items)
			: getMenuItemAt(_mousePos, selection.items);
		if (selection.selectedItem == -1)
			break;

		return kActivateMenuItem;
	default:
		break;
	}
	return kNoMenuCommand;
}

bool MenuSystem::captureMainMenuBackdrop(MainMenu &menu, IndexedBitmap &backdrop) {
	if (_hasMainMenuBackdrop) {
		renderBackdropMenuScreen(_mainMenuBackdrop, _mainMenuBackdropPalette, 1.0f,
			menu.selection.items, menu.selection.selectedItem);
	} else {
		renderMainMenuScreen(menu.selection.selectedItem, menu.statusMessage, false, menu.selection.items);
	}
	Graphics::Screen *screen = _engine.getScreen();
	return screen && captureScreenBackdrop(*screen, backdrop);
}

Common::Error MenuSystem::runMainMenuRoomLoop(MainMenu &menu, const Common::String &targetName) {
	menu.pausedTimers.resume();
	Common::Error roomError = menu.flow.runRoomLoop(targetName);
	menu.pausedTimers.pause();
	_engine.stopMusic();
	_engine.stopSound();
	if (roomError.getCode() == Common::kReadingFailed) {
		menu.statusMessage = Common::String::format(
			"Unable to resolve %s room setup from HARVEST.SCR.", targetName.c_str());
		menu.selection.needsRedraw = true;
		return Common::kNoError;
	}
	if (roomError.getCode() != Common::kNoError)
		return roomError;

	menu.statusMessage.clear();
	menu.selection.needsRedraw = true;
	(void)menu.flow.takePendingMainMenuReturn();
	if (menu.flow.takePendingGameOverReturn()) {
		Common::Error gameOverError = showGameOverBackdrop(menu.flow);
		if (gameOverError.getCode() != Common::kNoError)
			return gameOverError;
	} else {
		clearMainMenuBackdrop();
	}
	return Common::kNoError;
}

Common::Error MenuSystem::startNewGameFromMainMenu(MainMenu &menu, const byte *menuPalette) {
	const MenuTextConfig &config = menu.flow.getMenuTextConfig();

	IndexedBitmap menuBackdrop;
	if (!captureMainMenuBackdrop(menu, menuBackdrop))
		return Common::kReadingFailed;

	bool confirmed = false;
	Common::Error confirmError = runConfirmPrompt(
		menuBackdrop, menuPalette, 1.0f, menu.flow, config.newGamePrompt, confirmed);
	if (confirmError.getCode() != Common::kNoError)
		return confirmError;
	menu.selection.needsRedraw = true;
	if (!confirmed)
		return Common::kNoError;

	menu.flow.prepareForNewGame();
	return runMainMenuRoomLoop(menu, "START");
}

Common::Error MenuSystem::loadGameFromMainMenu(MainMenu &menu, const byte *menuPalette) {
	bool loadedGame = false;
	Common::Error loadError = runLoadGameMenu(menuPalette, 1.0f, menu.flow, loadedGame);
	if (loadError.getCode() != Common::kNoError)
		return loadError;
	menu.selection.needsRedraw = true;
	if (!loadedGame)
		return Common::kNoError;

	Common::String targetName = _engine.getPendingLoadedSaveRoomState().entranceName;
	if (targetName.empty())
		targetName = _engine.getPendingLoadedSaveRoomState().roomName;
	if (targetName.empty()) {
		menu.statusMessage = "Loaded save is missing its room target.";
		return Common::kNoError;
	}

	return runMainMenuRoomLoop(menu, targetName);
}

MenuSystem::RoomMenuActivationResult MenuSystem::startNewGameFromRoomMenu(RoomMenu &menu) {
	const MenuTextConfig &config = menu.flow.getMenuTextConfig();

	bool confirmed = false;
	Common::Error confirmError = runConfirmPrompt(
		menu.backdrop, menu.palette, menu.paletteBrightness, menu.flow, config.newGamePrompt, confirmed);
	if (confirmError.getCode() != Common::kNoError)
		return RoomMenuActivationResult(confirmError, false);
	menu.selection.needsRedraw = true;
	if (confirmed) {
		menu.flow.requestNewGameRestart();
		return RoomMenuActivationResult(Common::kNoError, true);
	}
	return RoomMenuActivationResult(Common::kNoError, false);
}

MenuSystem::RoomMenuActivationResult MenuSystem::saveGameFromRoomMenu(RoomMenu &menu) {
	bool savedGame = false;
	Common::Error saveError = runSaveGameMenu(menu.palette, menu.paletteBrightness, menu.flow, savedGame);
	if (saveError.getCode() != Common::kNoError)
		return RoomMenuActivationResult(saveError, false);
	if (savedGame) {
		// Native exits the in-room ESC menu after a successful save and restores the room view.
		Common::Error restoreError = restoreRoomBackdropAfterSave(
			menu.backdrop, menu.palette, menu.paletteBrightness, menu.flow);
		if (restoreError.getCode() != Common::kNoError)
			return RoomMenuActivationResult(restoreError, false);
		return RoomMenuActivationResult(Common::kNoError, true);
	}
	menu.selection.needsRedraw = true;
	return RoomMenuActivationResult(Common::kNoError, false);
}

Common::Error MenuSystem::activateMainMenuItem(MainMenu &menu) {
	if (menu.selection.selectedItem < 0 || menu.selection.selectedItem >= (int)menu.selection.items.size())
		return Common::kNoError;

	const Common::String &item = menu.selection.items[menu.selection.selectedItem];
	const byte *menuPalette = _hasMainMenuBackdrop ? _mainMenuBackdropPalette : menu.art.getWaitPalette();
	menu.statusMessage.clear();
	if (item.empty() || item == kBlankMenuSlot)
		return Common::kNoError;
	debugC(2, kDebugGeneral, "Harvester: main menu selected index=%d label='%s'",
		menu.selection.selectedItem, item.c_str());

	if (menu.selection.selectedItem == kMainMenuItemNewGame)
		return startNewGameFromMainMenu(menu, menuPalette);

	if (menu.selection.selectedItem == kMainMenuItemLoadGame)
		return loadGameFromMainMenu(menu, menuPalette);

	if (menu.selection.selectedItem == kMainMenuItemSaveGame) {
		bool savedGame = false;
		Common::Error saveError = runSaveGameMenu(menuPalette, 1.0f, menu.flow, savedGame);
		(void)savedGame;
		menu.selection.needsRedraw = true;
		return saveError;
	}

	if (menu.selection.selectedItem == kMainMenuItemOptions) {
		IndexedBitmap menuBackdrop;
		if (_hasMainMenuBackdrop) {
			menuBackdrop = _mainMenuBackdrop;
		} else if (!captureMainMenuBackdrop(menu, menuBackdrop)) {
			return Common::kReadingFailed;
		}
		Common::Error optionsError = runOptionsMenu(menuBackdrop, menuPalette, 1.0f, menu.flow);
		menu.selection.needsRedraw = true;
		return optionsError;
	}

	if (menu.selection.selectedItem == kMainMenuItemHelp) {
		Common::Error helpError = runHelpScreen(menuPalette, 1.0f, menu.flow);
		menu.selection.needsRedraw = true;
		return helpError;
	}

	if (menu.selection.selectedItem == kMainMenuItemQuitGame) {
		IndexedBitmap menuBackdrop;
		if (!captureMainMenuBackdrop(menu, menuBackdrop))
			return Common::kReadingFailed;
		Common::Error quitError = runQuitGameConfirm(menuBackdrop, menuPalette, 1.0f, menu.flow);
		menu.selection.needsRedraw = true;
		return quitError;
	}

	return Common::kNoError;
}

MenuSystem::RoomMenuActivationResult MenuSystem::activateRoomMenuItem(RoomMenu &menu) {
	if (menu.selection.selectedItem < 0 || menu.selection.selectedItem >= (int)menu.selection.items.size())
		return RoomMenuActivationResult(Common::kNoError, false);

	const Common::String &item = menu.selection.items[menu.selection.selectedItem];
	if (item.empty() || item == kBlankMenuSlot)
		return RoomMenuActivationResult(Common::kNoError, false);
	debugC(2, kDebugGeneral, "Harvester: room menu selected index=%d label='%s'",
		menu.selection.selectedItem, item.c_str());

	if (menu.selection.selectedItem == kMainMenuItemNewGame)
		return startNewGameFromRoomMenu(menu);

	if (menu.selection.selectedItem == kMainMenuItemLoadGame) {
		bool loadedGame = false;
		Common::Error loadError = runLoadGameMenu(menu.palette, menu.paletteBrightness, menu.flow, loadedGame);
		if (loadError.getCode() != Common::kNoError)
			return RoomMenuActivationResult(loadError, false);
		menu.selection.needsRedraw = true;
		return RoomMenuActivationResult(Common::kNoError, loadedGame);
	}

	if (menu.selection.selectedItem == kMainMenuItemOptions) {
		Common::Error optionsError = runOptionsMenu(menu.backdrop, menu.palette, menu.paletteBrightness, menu.flow);
		if (optionsError.getCode() != Common::kNoError)
			return RoomMenuActivationResult(optionsError, false);
		menu.selection.needsRedraw = true;
		return RoomMenuActivationResult(Common::kNoError, false);
	}

	if (menu.selection.selectedItem == kMainMenuItemHelp) {
		Common::Error helpError = runHelpScreen(menu.palette, menu.paletteBrightness, menu.flow);
		if (helpError.getCode() != Common::kNoError)
			return RoomMenuActivationResult(helpError, false);
		menu.selection.needsRedraw = true;
		return RoomMenuActivationResult(Common::kNoError, false);
	}

	if (menu.selection.selectedItem == kMainMenuItemSaveGame)
		return saveGameFromRoomMenu(menu);

	if (menu.selection.selectedItem == kMainMenuItemQuitGame) {
		Common::Error quitError = runQuitGameConfirm(menu.backdrop, menu.palette, menu.paletteBrightness, menu.flow);
		if (quitError.getCode() != Common::kNoError)
			return RoomMenuActivationResult(quitError, false);
		menu.selection.needsRedraw = true;
		return RoomMenuActivationResult(Common::kNoError, false);
	}

	debugC(1, kDebugGeneral,
		"Harvester: room menu index=%d label='%s' selected but not implemented",
		menu.selection.selectedItem, item.c_str());
	return RoomMenuActivationResult(Common::kNoError, false);
}

Common::Error MenuSystem::runMainMenu(Flow &flow) {
	const Art *art = _engine.getArt();
	if (!art)
		return Common::kReadingFailed;
	MainMenu menu(_engine, *art, flow);

	buildDisplayMainMenuItems(_menuItems,
		_engine.canSaveGameStateCurrently(),
		_engine.canLoadGameStateCurrently(),
		menu.selection.items);
	if (menu.flow.takePendingGameOverReturn()) {
		Common::Error gameOverError = showGameOverBackdrop(menu.flow);
		if (gameOverError.getCode() != Common::kNoError)
			return gameOverError;
	} else {
		clearMainMenuBackdrop();
	}

	Graphics::FrameLimiter limiter(g_system, 60);
	menu.selection.selectedItem = menu.selection.items.empty() ? -1 : 0;

	while (!_engine.shouldQuit()) {
		if (menu.selection.needsRedraw) {
			if (_hasMainMenuBackdrop)
				renderBackdropMenuScreen(_mainMenuBackdrop, _mainMenuBackdropPalette, 1.0f,
					menu.selection.items, menu.selection.selectedItem);
			else
				renderMainMenuScreen(menu.selection.selectedItem, menu.statusMessage, true, menu.selection.items);
			menu.selection.needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (menu.flow.handleSystemEvent(event, result))
				return result;

			const MenuNavigationCommand command = handleMenuNavigationEvent(menu.selection, event, _hasMainMenuBackdrop);
			if (command == kCloseMenu)
				return Common::kNoError;
			if (command == kActivateMenuItem) {
				Common::Error activateError = activateMainMenuItem(menu);
				if (activateError.getCode() != Common::kNoError)
					return activateError;
			}
		}

		if (menu.flow.tickRuntimeEntities())
			menu.selection.needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error MenuSystem::runRoomMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow, bool canSaveGame) {
	Graphics::FrameLimiter limiter(g_system, 60);
	RoomMenu menu(backdrop, palette, paletteBrightness, flow);
	buildDisplayMainMenuItems(_menuItems,
		canSaveGame && _engine.canSaveGameStateCurrently(),
		_engine.canLoadGameStateCurrently(),
		menu.selection.items);
	menu.selection.selectedItem = menu.selection.items.empty() ? -1 : 0;
	menu.flow.resetCursorAnimationSequence();

	while (!_engine.shouldQuit()) {
		if (menu.selection.needsRedraw) {
			renderBackdropMenuScreen(menu.backdrop, menu.palette, menu.paletteBrightness,
				menu.selection.items, menu.selection.selectedItem);
			menu.selection.needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (menu.flow.handleSystemEvent(event, result))
				return result;

			const MenuNavigationCommand command = handleMenuNavigationEvent(menu.selection, event, true);
			if (command == kCloseMenu)
				return Common::kNoError;
			if (command == kActivateMenuItem) {
				RoomMenuActivationResult activationResult = activateRoomMenuItem(menu);
				if (activationResult.error.getCode() != Common::kNoError)
					return activationResult.error;
				if (activationResult.closeMenu)
					return Common::kNoError;
			}
		}

		EntityManager *entityManager = _engine.getRuntimeEntities();
		if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
			menu.selection.needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error MenuSystem::showGameOverBackdrop(Flow &flow) {
	ResourceManager *resources = _engine.getResources();
	Graphics::Screen *screen = _engine.getScreen();
	if (!resources || !screen)
		return Common::kReadingFailed;

	IndexedBitmap backdrop;
	byte palette[256 * 3];
	if (!loadBitmapResource(*resources, kGameOverBitmapPath, backdrop) ||
			!loadPaletteResource(*resources, kGameOverPalettePath, palette)) {
		return Common::kReadingFailed;
	}

	_mainMenuBackdrop = Common::move(backdrop);
	memcpy(_mainMenuBackdropPalette, palette, sizeof(_mainMenuBackdropPalette));
	_hasMainMenuBackdrop = true;
	flow.resetCursorAnimationSequence();
	(void)_engine.playMusic(kGameOverMusicPath);

	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);
	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			MenuRendering::applyMenuPalette(*screen, _engine, _mainMenuBackdropPalette, 1.0f);
			screen->fillRect(screen->getBounds(), 0);
			MenuRendering::blitBitmap(*screen, _mainMenuBackdrop, 0, 0);
			if (_engine.getRuntimeEntities())
				_engine.getRuntimeEntities()->drawCursor(*screen);
			screen->makeAllDirty();
			screen->update();
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONDOWN:
			case Common::EVENT_KEYDOWN:
				return Common::kNoError;
			default:
				break;
			}
		}

		if (EntityManager *entityManager = _engine.getRuntimeEntities()) {
			if (entityManager->syncCursorEntityPosition(_mousePos))
				needsRedraw = true;
		}

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

void MenuSystem::clearMainMenuBackdrop() {
	_mainMenuBackdrop = IndexedBitmap();
	memset(_mainMenuBackdropPalette, 0, sizeof(_mainMenuBackdropPalette));
	_hasMainMenuBackdrop = false;
}

int MenuSystem::getMenuItemAt(const Common::Point &mousePos,
		const Common::Array<Common::String> &menuItems) const {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		return -1;

	for (uint i = 0; i < menuItems.size(); ++i) {
		const int width = font->getStringWidth(menuItems[i]);
		const int x = (640 - width) / 2;
		const int y = MenuRendering::kMenuStartY + (int)i * MenuRendering::kMenuLineSpacing;
		const Common::Rect bounds(x - 12, y - 2, x + width + 12, y + font->getFontHeight() + 2);
		if (bounds.contains(mousePos))
			return (int)i;
	}

	return -1;
}

int MenuSystem::getBackdropMenuItemAt(const Common::Point &mousePos,
		const Common::Array<Common::String> &menuItems) const {
	const Text *text = _engine.getText();
	const CftFontResource *selectedFontResource = text ? text->findFont("HARVFONT") : nullptr;
	if (!selectedFontResource)
		return getMenuItemAt(mousePos, menuItems);

	HarvesterCftFont selectedFont(*selectedFontResource);
	if (!selectedFont.isValid())
		return getMenuItemAt(mousePos, menuItems);

	return getNativeRoomMenuSelectionFromMouse(selectedFont, menuItems.size(), mousePos);
}

} // End of namespace Harvester
