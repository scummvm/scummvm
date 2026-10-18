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

#include "harvester/menu.h"
#include "harvester/menu_render.h"

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/metaengine.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/resources.h"
#include "harvester/room_support.h"
#include "harvester/entity.h"
#include "harvester/art.h"
#include "harvester/flow.h"
#include "harvester/script.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const char *const kOptionsVolumeBitmapPath = "1:/GRAPHIC/OTHER/VOLUME.BM";
static const char *const kOptionsIndicatorBitmapPath = "1:/GRAPHIC/OTHER/INDICATR.BM";
static const char *const kOptionsPreviewSoundPath = "1:/SOUND/EFFECTS/WHIP2.WAV";
static const char *const kLoadGameBitmapPath = "1:/GRAPHIC/OTHER/LOADGAME.BM";
static const char *const kLoadGamePalettePath = "1:/GRAPHIC/PAL/LOADGAME.PAL";
static const char *const kSaveGameBitmapPath = "1:/GRAPHIC/OTHER/SAVEGAME.BM";
static const char *const kSaveGamePalettePath = "1:/GRAPHIC/PAL/SAVEGAME.PAL";
static const char *const kGameOverBitmapPath = "1:/GRAPHIC/OTHER/GAMEOVER.BM";
static const char *const kGameOverPalettePath = "1:/GRAPHIC/PAL/GAMEOVER.PAL";
static const char *const kGameOverMusicPath = "SOUND/MUSIC/ANXIETY.CMP";
static const int kOptionsSliderMaxX = 0x22b;
static const int kOptionsSliderHeight = 0x1e;
static const int kStartupOptionMaxLevel = 9;
static const int kSaveSlotCount = 25;
static const int kSaveSlotLabelX = 8;
static const int kSaveSlotNameX = 0x50;
static const int kSaveSlotStartY = 0x19;
static const int kSaveSlotStride = 0x11;
static const int kSaveSlotTextWidth = 0x27b - kSaveSlotNameX;
static const int kSaveStatusTextY = 0x1b8;
static const int kSaveSlotMaxCharacters = 0xf6;
static const int kPasswordMaxCharacters = 8;
static const uint32 kPasswordCursorBlinkMs = 480;
static const uint32 kPaletteFadeTickMs = 4;
static const float kPaletteFadeStep = 0.1f;
static const char *const kCdChangePromptPalettePath = "1:/GRAPHIC/PAL/CD1.PAL";
static const float kPaletteBrightnessBlack = 0.0f;
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

static int clampStartupOptionLevel(int level) {
	if (level < 0)
		return 0;
	if (level > kStartupOptionMaxLevel)
		return kStartupOptionMaxLevel;

	return level;
}

static Common::Rect saveSlotRect(int slotIndex) {
	const int top = kSaveSlotStartY + slotIndex * kSaveSlotStride;
	return Common::Rect(8, top, 0x27b + 1, top + kSaveSlotStride);
}

static Common::Rect saveConfirmRect() {
	return Common::Rect(3, 0x1cc, 0x13d + 1, 0x1dc + 1);
}

static Common::Rect saveCancelRect() {
	return Common::Rect(0x141, 0x1cc, 0x27b + 1, 0x1dc + 1);
}

static Common::Rect quitConfirmYesRect() {
	return Common::Rect(0xd2, 0xee, 0x104 + 1, 0x108 + 1);
}

static Common::Rect quitConfirmNoRect() {
	return Common::Rect(0x172, 0xee, 0x19a + 1, 0x108 + 1);
}

static Common::Rect optionsSliderRect(int sliderIndex, int lineHeight) {
	const int y = MenuRendering::kOptionsIndicatorY + sliderIndex * lineHeight;
	return Common::Rect(MenuRendering::kOptionsSliderMinX, y, kOptionsSliderMaxX + 1, y + kOptionsSliderHeight + 1);
}

static int resolveOptionsSliderIndexFromMouseX(int mouseX) {
	const int relativeX = mouseX - MenuRendering::kOptionsSliderMinX;
	return clampStartupOptionLevel(
		(relativeX + (MenuRendering::kOptionsSliderStep / 2)) / MenuRendering::kOptionsSliderStep);
}

} // End of anonymous namespace

namespace {

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

static bool loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	return decodeBitmap(data, bitmap);
}

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *palette) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(palette, data.data(), 256 * 3);
	return true;
}

static Common::String buildDefaultSaveSlotLabel(int slotIndex) {
	return Common::String::format("GAME %02d", slotIndex + 1);
}

static int getNativeRoomMenuSelectionFromMouse(const Graphics::Font &selectedFont, uint itemCount,
		const Common::Point &mousePos) {
	if (itemCount == 0)
		return -1;

	int selection = mousePos.y - MenuRendering::kMenuStartY;
	if (selection < 1)
		selection = 1;

	selection /= MAX<int>(1, MenuRendering::getNativeRoomMenuLineHeight(selectedFont));
	if (selection >= (int)itemCount)
		selection = (int)itemCount - 1;

	return selection;
}

} // End of anonymous namespace

MenuSystem::MenuSystem(HarvesterEngine &engine, Common::Point &mousePos,
		const Common::Array<Common::String> &menuItems)
	: _engine(engine), _mousePos(mousePos), _menuItems(menuItems) {
}

Common::Error MenuSystem::showCdChangePrompt(int discNumber, ResourceManager &resources,
		Flow &flow, bool &promptCompleted) {
	promptCompleted = false;
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen)
		return Common::kReadingFailed;

	const Common::String bitmapPath = Common::String::format("1:/GRAPHIC/OTHER/CD%d.BM", discNumber);
	IndexedBitmap promptBitmap;
	byte promptPalette[256 * 3];
	if (!loadBitmapResource(resources, bitmapPath, promptBitmap) ||
			!loadPaletteResource(resources, kCdChangePromptPalettePath, promptPalette)) {
		warning("Harvester: unable to load CD change prompt assets for disc %d", discNumber);
		return Common::kReadingFailed;
	}

	debugC(1, kDebugScene, "Harvester: showing CD change prompt for disc %d", discNumber);
	_engine.stopMusic();

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		Common::Error result = Common::kNoError;
		if (flow.handleSystemEvent(event, result))
			return result;
		if (event.type == Common::EVENT_MOUSEMOVE)
			_mousePos = event.mouse;
	}

	bool waitingForRelease = false;
	bool needsRedraw = true;
	bool activateRequestedDisc = false;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit() && !activateRequestedDisc) {
		if (needsRedraw) {
			MenuRendering::renderCdChangePromptScreen(_engine, promptBitmap, promptPalette);
			needsRedraw = false;
		}

		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				_mousePos = event.mouse;
				needsRedraw = true;
				break;
			case Common::EVENT_LBUTTONDOWN:
				waitingForRelease = true;
				break;
			case Common::EVENT_LBUTTONUP:
				if (waitingForRelease)
					activateRequestedDisc = true;
				break;
			default:
				break;
			}
		}

		EntityManager *entityManager = _engine.getRuntimeEntities();
		if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}
	promptCompleted = true;
	return Common::kNoError;
}

Common::Error MenuSystem::runMainMenu(Flow &flow) {
	const Art *art = _engine.getArt();
	if (!art)
		return Common::kReadingFailed;
	ScopedSceneTimerPause pausedTimers(_engine);

	Common::Array<Common::String> mainMenuItems;
	buildDisplayMainMenuItems(_menuItems,
		_engine.canSaveGameStateCurrently(),
		_engine.canLoadGameStateCurrently(),
		mainMenuItems);
	if (flow.takePendingGameOverReturn()) {
		Common::Error gameOverError = showGameOverBackdrop(flow);
		if (gameOverError.getCode() != Common::kNoError)
			return gameOverError;
	} else {
		clearMainMenuBackdrop();
	}

	Graphics::FrameLimiter limiter(g_system, 60);
	int selectedItem = mainMenuItems.empty() ? -1 : 0;
	Common::String statusMessage;
	bool needsRedraw = true;

	auto captureMenuBackdrop = [&](IndexedBitmap &backdrop) -> bool {
		if (_hasMainMenuBackdrop) {
			renderBackdropMenuScreen(_mainMenuBackdrop, _mainMenuBackdropPalette, 1.0f,
				mainMenuItems, selectedItem);
		} else {
			renderMainMenuScreen(selectedItem, statusMessage, false, mainMenuItems);
		}
		Graphics::Screen *screen = _engine.getScreen();
		return screen && captureScreenBackdrop(*screen, backdrop);
	};
	auto runSelectedRoomLoop = [&](const Common::String &targetName) -> Common::Error {
		pausedTimers.resume();
		Common::Error roomError = flow.runRoomLoop(targetName);
		pausedTimers.pause();
		_engine.stopMusic();
		_engine.stopSound();
		if (roomError.getCode() == Common::kReadingFailed) {
			statusMessage = Common::String::format(
				"Unable to resolve %s room setup from HARVEST.SCR.", targetName.c_str());
			needsRedraw = true;
			return Common::kNoError;
		}
		if (roomError.getCode() != Common::kNoError)
			return roomError;

		statusMessage.clear();
		needsRedraw = true;
		(void)flow.takePendingMainMenuReturn();
		if (flow.takePendingGameOverReturn()) {
			Common::Error gameOverError = showGameOverBackdrop(flow);
			if (gameOverError.getCode() != Common::kNoError)
				return gameOverError;
		} else {
			clearMainMenuBackdrop();
		}
		return Common::kNoError;
	};
	auto activateSelectedItem = [&]() -> Common::Error {
		if (selectedItem < 0 || selectedItem >= (int)mainMenuItems.size())
			return Common::kNoError;

		const Common::String &item = mainMenuItems[selectedItem];
		const byte *menuPalette = _hasMainMenuBackdrop ? _mainMenuBackdropPalette : art->getWaitPalette();
		statusMessage.clear();
		if (item.empty() || item == kBlankMenuSlot)
			return Common::kNoError;
		debugC(2, kDebugGeneral, "Harvester: main menu selected index=%d label='%s'",
			selectedItem, item.c_str());

		if (selectedItem == kMainMenuItemNewGame) {
			const MenuTextConfig &config = flow.getMenuTextConfig();

			IndexedBitmap menuBackdrop;
			if (!captureMenuBackdrop(menuBackdrop))
				return Common::kReadingFailed;

			bool confirmed = false;
			Common::Error confirmError = runConfirmPrompt(
				menuBackdrop, menuPalette, 1.0f, flow, config.newGamePrompt, confirmed);
			if (confirmError.getCode() != Common::kNoError)
				return confirmError;
			needsRedraw = true;
			if (!confirmed)
				return Common::kNoError;

			flow.prepareForNewGame();
			return runSelectedRoomLoop("START");
		}

		if (selectedItem == kMainMenuItemLoadGame) {
			bool loadedGame = false;
			Common::Error loadError = runLoadGameMenu(menuPalette, 1.0f, flow, loadedGame);
			if (loadError.getCode() != Common::kNoError)
				return loadError;
			needsRedraw = true;
			if (!loadedGame)
				return Common::kNoError;

			Common::String targetName = _engine.getPendingLoadedSaveRoomState().entranceName;
			if (targetName.empty())
				targetName = _engine.getPendingLoadedSaveRoomState().roomName;
			if (targetName.empty()) {
				statusMessage = "Loaded save is missing its room target.";
				return Common::kNoError;
			}

			return runSelectedRoomLoop(targetName);
		}

		if (selectedItem == kMainMenuItemSaveGame) {
			bool savedGame = false;
			Common::Error saveError = runSaveGameMenu(menuPalette, 1.0f, flow, savedGame);
			(void)savedGame;
			needsRedraw = true;
			return saveError;
		}

		if (selectedItem == kMainMenuItemOptions) {
			IndexedBitmap menuBackdrop;
			if (_hasMainMenuBackdrop) {
				menuBackdrop = _mainMenuBackdrop;
			} else if (!captureMenuBackdrop(menuBackdrop)) {
				return Common::kReadingFailed;
			}
			Common::Error optionsError = runOptionsMenu(menuBackdrop, menuPalette, 1.0f, flow);
			needsRedraw = true;
			return optionsError;
		}

		if (selectedItem == kMainMenuItemHelp) {
			Common::Error helpError = runHelpScreen(menuPalette, 1.0f, flow);
			needsRedraw = true;
			return helpError;
		}

		if (selectedItem == kMainMenuItemQuitGame) {
			IndexedBitmap menuBackdrop;
			if (!captureMenuBackdrop(menuBackdrop))
				return Common::kReadingFailed;
			Common::Error quitError = runQuitGameConfirm(menuBackdrop, menuPalette, 1.0f, flow);
			needsRedraw = true;
			return quitError;
		}

		return Common::kNoError;
	};

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			if (_hasMainMenuBackdrop)
				renderBackdropMenuScreen(_mainMenuBackdrop, _mainMenuBackdropPalette, 1.0f, mainMenuItems, selectedItem);
			else
				renderMainMenuScreen(selectedItem, statusMessage, true, mainMenuItems);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredItem = _hasMainMenuBackdrop
					? getBackdropMenuItemAt(_mousePos, mainMenuItems)
					: getMenuItemAt(_mousePos, mainMenuItems);
				if (hoveredItem != -1 && hoveredItem != selectedItem) {
					selectedItem = hoveredItem;
					needsRedraw = true;
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;

				if (mainMenuItems.empty())
					break;

				if (event.kbd.keycode == Common::KEYCODE_UP) {
					selectedItem = (selectedItem + mainMenuItems.size() - 1) % mainMenuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					selectedItem = (selectedItem + 1) % mainMenuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					Common::Error activateError = activateSelectedItem();
					if (activateError.getCode() != Common::kNoError)
						return activateError;
				}
				break;
			case Common::EVENT_LBUTTONDOWN: {
				if (mainMenuItems.empty())
					break;

				selectedItem = _hasMainMenuBackdrop
					? getBackdropMenuItemAt(_mousePos, mainMenuItems)
					: getMenuItemAt(_mousePos, mainMenuItems);
				if (selectedItem == -1)
					break;

				Common::Error activateError = activateSelectedItem();
				if (activateError.getCode() != Common::kNoError)
					return activateError;
				break;
			}
			default:
				break;
			}
		}

		if (flow.tickRuntimeEntities())
			needsRedraw = true;

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

Common::Error MenuSystem::runRoomMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow, bool canSaveGame) {
	Graphics::FrameLimiter limiter(g_system, 60);
	Common::Array<Common::String> roomMenuItems;
	buildDisplayMainMenuItems(_menuItems,
		canSaveGame && _engine.canSaveGameStateCurrently(),
		_engine.canLoadGameStateCurrently(),
		roomMenuItems);
	int selectedItem = roomMenuItems.empty() ? -1 : 0;
	bool needsRedraw = true;
	flow.resetCursorAnimationSequence();

	struct RoomMenuActivationResult {
		Common::Error error;
		bool closeMenu;

		RoomMenuActivationResult(Common::Error e, bool c) : error(e), closeMenu(c) {}
	};
	auto activateSelectedItem = [&]() -> RoomMenuActivationResult {
		if (selectedItem < 0 || selectedItem >= (int)roomMenuItems.size())
			return RoomMenuActivationResult(Common::kNoError, false);

		const Common::String &item = roomMenuItems[selectedItem];
		if (item.empty() || item == kBlankMenuSlot)
			return RoomMenuActivationResult(Common::kNoError, false);
		debugC(2, kDebugGeneral, "Harvester: room menu selected index=%d label='%s'",
			selectedItem, item.c_str());

		if (selectedItem == kMainMenuItemNewGame) {
			const MenuTextConfig &config = flow.getMenuTextConfig();

			bool confirmed = false;
			Common::Error confirmError = runConfirmPrompt(
				backdrop, palette, paletteBrightness, flow, config.newGamePrompt, confirmed);
			if (confirmError.getCode() != Common::kNoError)
				return RoomMenuActivationResult(confirmError, false);
			needsRedraw = true;
			if (confirmed) {
				flow.requestNewGameRestart();
				return RoomMenuActivationResult(Common::kNoError, true);
			}
			return RoomMenuActivationResult(Common::kNoError, false);
		}

		if (selectedItem == kMainMenuItemLoadGame) {
			bool loadedGame = false;
			Common::Error loadError = runLoadGameMenu(palette, paletteBrightness, flow, loadedGame);
			if (loadError.getCode() != Common::kNoError)
				return RoomMenuActivationResult(loadError, false);
			needsRedraw = true;
			return RoomMenuActivationResult(Common::kNoError, loadedGame);
		}

		if (selectedItem == kMainMenuItemOptions) {
			Common::Error optionsError = runOptionsMenu(backdrop, palette, paletteBrightness, flow);
			if (optionsError.getCode() != Common::kNoError)
				return RoomMenuActivationResult(optionsError, false);
			needsRedraw = true;
			return RoomMenuActivationResult(Common::kNoError, false);
		}

		if (selectedItem == kMainMenuItemHelp) {
			Common::Error helpError = runHelpScreen(palette, paletteBrightness, flow);
			if (helpError.getCode() != Common::kNoError)
				return RoomMenuActivationResult(helpError, false);
			needsRedraw = true;
			return RoomMenuActivationResult(Common::kNoError, false);
		}

		if (selectedItem == kMainMenuItemSaveGame) {
			bool savedGame = false;
			Common::Error saveError = runSaveGameMenu(palette, paletteBrightness, flow, savedGame);
			if (saveError.getCode() != Common::kNoError)
				return RoomMenuActivationResult(saveError, false);
			if (savedGame) {
				// Native exits the in-room ESC menu after a successful save and restores the room view.
				Common::Error restoreError = restoreRoomBackdropAfterSave(
					backdrop, palette, paletteBrightness, flow);
				if (restoreError.getCode() != Common::kNoError)
					return RoomMenuActivationResult(restoreError, false);
				return RoomMenuActivationResult(Common::kNoError, true);
			}
			needsRedraw = true;
			return RoomMenuActivationResult(Common::kNoError, false);
		}

		if (selectedItem == kMainMenuItemQuitGame) {
			Common::Error quitError = runQuitGameConfirm(backdrop, palette, paletteBrightness, flow);
			if (quitError.getCode() != Common::kNoError)
				return RoomMenuActivationResult(quitError, false);
			needsRedraw = true;
			return RoomMenuActivationResult(Common::kNoError, false);
		}

		debugC(1, kDebugGeneral,
			"Harvester: room menu index=%d label='%s' selected but not implemented",
			selectedItem, item.c_str());
		return RoomMenuActivationResult(Common::kNoError, false);
	};

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderBackdropMenuScreen(backdrop, palette, paletteBrightness, roomMenuItems, selectedItem);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredItem = getBackdropMenuItemAt(_mousePos, roomMenuItems);
				if (hoveredItem != -1 && hoveredItem != selectedItem) {
					selectedItem = hoveredItem;
					needsRedraw = true;
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;

				if (roomMenuItems.empty())
					break;

				if (event.kbd.keycode == Common::KEYCODE_UP) {
					selectedItem = (selectedItem + roomMenuItems.size() - 1) % roomMenuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_DOWN) {
					selectedItem = (selectedItem + 1) % roomMenuItems.size();
					needsRedraw = true;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					RoomMenuActivationResult activationResult = activateSelectedItem();
					if (activationResult.error.getCode() != Common::kNoError)
						return activationResult.error;
					if (activationResult.closeMenu)
						return Common::kNoError;
				}
				break;
			case Common::EVENT_LBUTTONDOWN: {
				if (roomMenuItems.empty())
					break;

				selectedItem = getBackdropMenuItemAt(_mousePos, roomMenuItems);
				if (selectedItem == -1)
					break;

				RoomMenuActivationResult activationResult = activateSelectedItem();
				if (activationResult.error.getCode() != Common::kNoError)
					return activationResult.error;
				if (activationResult.closeMenu)
					return Common::kNoError;
				break;
			}
			default:
				break;
			}
		}

		EntityManager *entityManager = _engine.getRuntimeEntities();
		if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

struct MenuSystem::SaveSlotMenu {
	SaveSlotMenu(MetaEngine &meta, const CftFontResource &nameFont, const CftFontResource &labelFont)
		: metaEngine(meta), slotNameFont(nameFont), slotLabelFont(labelFont) {}

	MetaEngine &metaEngine;
	HarvesterCftFont slotNameFont;
	HarvesterCftFont slotLabelFont;
	IndexedBitmap background;
	byte palette[256 * 3];
	Common::Array<Common::String> slotTitles;
	int activeSlot = 0;
	Common::String statusMessage;
	bool needsRedraw = true;
};

void MenuSystem::reloadSaveSlotTitles(SaveSlotMenu &menu) const {
	menu.slotTitles.clear();
	menu.slotTitles.resize(kSaveSlotCount);
	const SaveStateList saves = menu.metaEngine.listSaves(ConfMan.getActiveDomainName().c_str());
	for (const SaveStateDescriptor &save : saves) {
		const int slot = save.getSaveSlot();
		if (slot >= 0 && slot < kSaveSlotCount)
			menu.slotTitles[slot] = save.getDescription();
	}
}

int MenuSystem::getSaveSlotAt(const Common::Point &mousePos) const {
	for (int slot = 0; slot < kSaveSlotCount; ++slot) {
		if (saveSlotRect(slot).contains(mousePos))
			return slot;
	}
	return -1;
}

MenuSystem::SaveSlotCommand MenuSystem::getSaveSlotCommand(const Common::Event &event) const {
	if (event.type == Common::EVENT_KEYDOWN) {
		if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
			return kCancelSlot;
		if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER)
			return kConfirmSlot;
	} else if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (saveConfirmRect().contains(_mousePos))
			return kConfirmSlot;
		if (saveCancelRect().contains(_mousePos))
			return kCancelSlot;
	}
	return kNoSlotCommand;
}

void MenuSystem::renderSaveSlotMenu(const SaveSlotMenu &menu, const Common::String *editingText) const {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen)
		return;

	MenuRendering::applyMenuPalette(*screen, _engine, menu.palette, 1.0f);
	MenuRendering::blitBitmap(*screen, menu.background, 0, 0);

	for (int i = 0; i < kSaveSlotCount; ++i) {
		const Graphics::Font &labelFont = (i == menu.activeSlot) ? menu.slotNameFont : menu.slotLabelFont;
		const int y = kSaveSlotStartY + i * kSaveSlotStride;
		const Common::String slotLabel = buildDefaultSaveSlotLabel(i);
		labelFont.drawString(screen, slotLabel, kSaveSlotLabelX, y,
			labelFont.getStringWidth(slotLabel), 0);

		Common::String displayTitle = menu.slotTitles[i];
		if (i == menu.activeSlot && editingText)
			displayTitle = *editingText + "_";
		menu.slotNameFont.drawString(screen, displayTitle, kSaveSlotNameX, y, kSaveSlotTextWidth, 0);
	}

	if (!editingText && !menu.statusMessage.empty()) {
		MenuRendering::drawShadowedString(*screen, menu.slotNameFont, menu.statusMessage,
			MenuRendering::TextLayout(20, kSaveStatusTextY, screen->w - 40),
			MenuRendering::kTextColorNormal, Graphics::kTextAlignCenter);
	}

	if (_engine.getRuntimeEntities())
		_engine.getRuntimeEntities()->drawCursor(*screen);
	screen->makeAllDirty();
	screen->update();
}

Common::Error MenuSystem::runSaveSlotEditor(SaveSlotMenu &menu, Flow &flow,
		Common::String &editedTitle, bool &confirmed) {
	confirmed = false;
	bool needsRedraw = true;
	flow.resetCursorAnimationSequence();
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			renderSaveSlotMenu(menu, &editedTitle);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			const SaveSlotCommand command = getSaveSlotCommand(event);
			if (command != kNoSlotCommand) {
				confirmed = command == kConfirmSlot;
				return Common::kNoError;
			}

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE:
				needsRedraw = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
					needsRedraw |= deleteTextEntryCharacter(editedTitle);
					break;
				}
				if (event.kbd.ascii >= 32 && event.kbd.ascii <= 126 && event.kbd.ascii != '~') {
					needsRedraw |= appendTextEntryCharacter(editedTitle, menu.slotNameFont,
						event.kbd.ascii, kSaveSlotMaxCharacters, kSaveSlotTextWidth);
				}
				break;
			default:
				break;
			}
		}

		if (EntityManager *entityManager = _engine.getRuntimeEntities())
			(void)entityManager->syncCursorEntityPosition(_mousePos);

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

void MenuSystem::loadSelectedSlot(SaveSlotMenu &menu, bool &loaded) {
	if (menu.slotTitles[menu.activeSlot].empty())
		return;

	Common::Error loadError = _engine.loadGameState(menu.activeSlot);
	if (loadError.getCode() == Common::kNoError) {
		loaded = true;
		return;
	}

	menu.statusMessage = loadError.getDesc();
	menu.needsRedraw = true;
}

Common::Error MenuSystem::saveSelectedSlot(SaveSlotMenu &menu, Flow &flow, bool &saved) {
	saved = false;
	Common::String editedTitle = menu.slotTitles[menu.activeSlot];
	bool confirmed = false;
	Common::Error entryError = runSaveSlotEditor(menu, flow, editedTitle, confirmed);
	if (entryError.getCode() != Common::kNoError)
		return entryError;
	if (!confirmed)
		return Common::kNoError;

	Common::Error saveError = _engine.saveGameState(menu.activeSlot, editedTitle);
	if (saveError.getCode() == Common::kNoError) {
		saved = true;
		return Common::kNoError;
	}

	menu.statusMessage = saveError.getDesc();
	reloadSaveSlotTitles(menu);
	menu.needsRedraw = true;
	return Common::kNoError;
}

Common::Error MenuSystem::runLoadGameMenu(const byte *palette, float paletteBrightness,
		Flow &flow, bool &loadedGame) {
	return runSaveSlotMenu(flow, kLoadMode, loadedGame);
}

Common::Error MenuSystem::runSaveGameMenu(const byte *palette, float paletteBrightness,
		Flow &flow, bool &savedGame) {
	return runSaveSlotMenu(flow, kSaveMode, savedGame);
}

Common::Error MenuSystem::runSaveSlotMenu(Flow &flow, SaveLoadMode mode, bool &completed) {
	completed = false;
	ResourceManager *resources = _engine.getResources();
	MetaEngine *metaEngine = _engine.getMetaEngine();
	const Text *text = _engine.getText();
	const CftFontResource *slotNameFontResource = text ? text->findFont("MEDFONT1") : nullptr;
	const CftFontResource *slotLabelFontResource = text ? text->findFont("MEDFONT2") : nullptr;
	if (!resources || !metaEngine || !slotNameFontResource || !slotLabelFontResource)
		return Common::kReadingFailed;
	if (mode == kSaveMode ? !_engine.canSaveGameStateCurrently() : !_engine.canLoadGameStateCurrently())
		return Common::kNoError;

	SaveSlotMenu menu(*metaEngine, *slotNameFontResource, *slotLabelFontResource);
	if (!menu.slotNameFont.isValid() || !menu.slotLabelFont.isValid())
		return Common::kReadingFailed;

	if (!loadBitmapResource(*resources, mode == kSaveMode ? kSaveGameBitmapPath : kLoadGameBitmapPath, menu.background) ||
			!loadPaletteResource(*resources, mode == kSaveMode ? kSaveGamePalettePath : kLoadGamePalettePath, menu.palette)) {
		return Common::kReadingFailed;
	}

	reloadSaveSlotTitles(menu);
	if (mode == kLoadMode) {
		for (int slot = 0; slot < kSaveSlotCount; ++slot) {
			if (!menu.slotTitles[slot].empty()) {
				menu.activeSlot = slot;
				break;
			}
		}
	}

	flow.resetCursorAnimationSequence();
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (menu.needsRedraw) {
			renderSaveSlotMenu(menu);
			menu.needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			bool activateSlot = false;
			bool redrawAfterActivation = false;
			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				const int hoveredSlot = getSaveSlotAt(_mousePos);
				if (hoveredSlot != -1 && hoveredSlot != menu.activeSlot)
					menu.activeSlot = hoveredSlot;
				menu.needsRedraw = true;
				break;
			}
			case Common::EVENT_LBUTTONDOWN: {
				const int hoveredSlot = getSaveSlotAt(_mousePos);
				menu.statusMessage.clear();
				if (hoveredSlot != -1) {
					menu.activeSlot = hoveredSlot;
					activateSlot = true;
					// Cancelling a row-opened save editor does not force a browser redraw.
					redrawAfterActivation = mode == kLoadMode;
				} else {
					const SaveSlotCommand command = getSaveSlotCommand(event);
					if (command == kCancelSlot)
						return Common::kNoError;
					activateSlot = command == kConfirmSlot;
					redrawAfterActivation = true;
				}
				break;
			}
			case Common::EVENT_KEYDOWN:
				if (getSaveSlotCommand(event) == kCancelSlot)
					return Common::kNoError;
				if (event.kbd.keycode == Common::KEYCODE_UP || event.kbd.keycode == Common::KEYCODE_DOWN) {
					const int step = event.kbd.keycode == Common::KEYCODE_UP ? kSaveSlotCount - 1 : 1;
					menu.activeSlot = (menu.activeSlot + step) % kSaveSlotCount;
					menu.statusMessage.clear();
					menu.needsRedraw = true;
				} else if (getSaveSlotCommand(event) == kConfirmSlot) {
					menu.statusMessage.clear();
					activateSlot = true;
					redrawAfterActivation = true;
				}
				break;
			default:
				break;
			}

			if (activateSlot) {
				Common::Error actionError = Common::kNoError;
				if (mode == kSaveMode)
					actionError = saveSelectedSlot(menu, flow, completed);
				else
					loadSelectedSlot(menu, completed);
				if (actionError.getCode() != Common::kNoError)
					return actionError;
				if (completed)
					return Common::kNoError;
			}
			if (redrawAfterActivation)
				menu.needsRedraw = true;
		}

		if (EntityManager *entityManager = _engine.getRuntimeEntities())
			(void)entityManager->syncCursorEntityPosition(_mousePos);

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error MenuSystem::restoreRoomBackdropAfterSave(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow) const {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen || !backdrop.isValid())
		return Common::kNoError;

	byte displayPalette[256 * 3];
	screen->getPalette(displayPalette);

	bool hasVisiblePalette = false;
	for (byte value : displayPalette) {
		if (value != 0) {
			hasVisiblePalette = true;
			break;
		}
	}

	if (hasVisiblePalette) {
		for (float brightness = 1.0f - kPaletteFadeStep;
				brightness > kPaletteBrightnessBlack; brightness -= kPaletteFadeStep) {
			byte fadedPalette[256 * 3];
			buildHarvesterDisplayPalette(displayPalette, brightness, fadedPalette);
			screen->setPalette(fadedPalette);
			screen->makeAllDirty();
			screen->update();

			const uint32 nextTick = g_system->getMillis() + kPaletteFadeTickMs;
			while ((int32)(nextTick - g_system->getMillis()) > 0) {
				Common::Event event;
				while (g_system->getEventManager()->pollEvent(event)) {
					Common::Error result = Common::kNoError;
					if (flow.handleSystemEvent(event, result))
						return result;
				}
				g_system->delayMillis(1);
			}
		}

		byte blackPalette[256 * 3] = { 0 };
		screen->setPalette(blackPalette);
		screen->makeAllDirty();
		screen->update();
	}

	screen->fillRect(screen->getBounds(), 0);
	MenuRendering::blitBitmap(*screen, backdrop, 0, 0);
	MenuRendering::applyMenuPalette(*screen, _engine, palette, kPaletteBrightnessBlack);
	screen->makeAllDirty();
	screen->update();

	return flow.fadeInRoomScene(palette, paletteBrightness);
}

Common::Error MenuSystem::runConfirmPrompt(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow, const Common::String &promptText,
		bool &confirmed) {
	const Art *art = _engine.getArt();
	const Text *text = _engine.getText();
	const CftFontResource *promptFontResource = text ? text->findFont("MEDFONT1") : nullptr;
	const CftFontResource *choiceFontResource = text ? text->findFont("MEDFONT2") : nullptr;
	if (!art || !promptFontResource || !choiceFontResource)
		return Common::kReadingFailed;

	HarvesterCftFont promptFont(*promptFontResource);
	HarvesterCftFont choiceFont(*choiceFontResource);
	if (!promptFont.isValid() || !choiceFont.isValid())
		return Common::kReadingFailed;

	const MenuTextConfig &config = flow.getMenuTextConfig();
	const IndexedBitmap *textbox = art->getTextboxBitmap(3);
	if (!textbox || !textbox->isValid())
		return Common::kReadingFailed;

	confirmed = false;
	flow.resetCursorAnimationSequence();
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		const bool hoverYes = quitConfirmYesRect().contains(_mousePos);
		const bool hoverNo = quitConfirmNoRect().contains(_mousePos);
		if (needsRedraw) {
			const Graphics::Font &yesFont = hoverYes ? static_cast<const Graphics::Font &>(promptFont)
				: static_cast<const Graphics::Font &>(choiceFont);
			const Graphics::Font &noFont = hoverNo ? static_cast<const Graphics::Font &>(promptFont)
				: static_cast<const Graphics::Font &>(choiceFont);
			MenuRendering::Renderer(_engine, backdrop, palette, paletteBrightness).renderConfirmPrompt(
				promptFont, yesFont, noFont, *textbox, promptText, config);
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
			case Common::EVENT_RBUTTONDOWN:
				return Common::kNoError;
			case Common::EVENT_LBUTTONDOWN:
				if (quitConfirmYesRect().contains(_mousePos)) {
					confirmed = true;
					return Common::kNoError;
				}
				if (quitConfirmNoRect().contains(_mousePos))
					return Common::kNoError;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;
				if (event.kbd.keycode == Common::KEYCODE_y || event.kbd.ascii == 'y' || event.kbd.ascii == 'Y') {
					confirmed = true;
					return Common::kNoError;
				}
				if (event.kbd.keycode == Common::KEYCODE_n || event.kbd.ascii == 'n' || event.kbd.ascii == 'N')
					return Common::kNoError;
				break;
			default:
				break;
			}
		}

		if (EntityManager *entityManager = _engine.getRuntimeEntities())
			(void)entityManager->syncCursorEntityPosition(_mousePos);

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error MenuSystem::runQuitGameConfirm(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow) {
	const MenuTextConfig &config = flow.getMenuTextConfig();

	bool confirmed = false;
	Common::Error confirmError = runConfirmPrompt(
		backdrop, palette, paletteBrightness, flow, config.quitGamePrompt, confirmed);
	if (confirmError.getCode() != Common::kNoError)
		return confirmError;
	if (!confirmed)
		return Common::kNoError;

	_engine.stopMusic();
	_engine.stopSound();
	_engine.quitGame();
	return Common::kNoError;
}

Common::Error MenuSystem::runPasswordPrompt(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow, bool drawLogo, Common::String &password,
		bool &accepted) const {
	const Art *art = _engine.getArt();
	const Text *text = _engine.getText();
	const CftFontResource *titleFontResource = text ? text->findFont("HARVFONT") : nullptr;
	const CftFontResource *entryFontResource = text ? text->findFont("HARVFNT2") : nullptr;
	if (!art || !titleFontResource || !entryFontResource)
		return Common::kReadingFailed;

	HarvesterCftFont titleFont(*titleFontResource);
	HarvesterCftFont entryFont(*entryFontResource);
	if (!titleFont.isValid() || !entryFont.isValid())
		return Common::kReadingFailed;

	password.clear();
	accepted = false;
	bool needsRedraw = true;
	bool cursorVisible = true;
	uint32 cursorToggleTicks = g_system->getMillis() + kPasswordCursorBlinkMs;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			MenuRendering::Renderer(_engine, backdrop, palette, paletteBrightness).renderPasswordPrompt(
				MenuRendering::FontPair(titleFont, entryFont), *art,
				flow.getMenuTextConfig().enterPasswordLabel, password, cursorVisible, drawLogo);
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			switch (event.type) {
			case Common::EVENT_RBUTTONDOWN:
				return Common::kNoError;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					return Common::kNoError;
				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					accepted = !password.empty();
					return Common::kNoError;
				}
				if (event.kbd.keycode == Common::KEYCODE_BACKSPACE ||
						event.kbd.keycode == Common::KEYCODE_LEFT) {
					needsRedraw |= deleteTextEntryCharacter(password);
					break;
				}
				if (event.kbd.keycode == Common::KEYCODE_HOME) {
					if (!password.empty()) {
						password.clear();
						needsRedraw = true;
					}
					break;
				}
				if (event.kbd.ascii >= 32 && event.kbd.ascii <= 126 && event.kbd.ascii != '~') {
					needsRedraw |= appendTextEntryCharacter(password, entryFont, event.kbd.ascii,
						kPasswordMaxCharacters, MenuRendering::kPasswordEntryWidth);
				}
				break;
			default:
				break;
			}
		}

		const uint32 now = g_system->getMillis();
		if ((int32)(now - cursorToggleTicks) >= 0) {
			cursorVisible = !cursorVisible;
			cursorToggleTicks = now + kPasswordCursorBlinkMs;
			needsRedraw = true;
		}

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error MenuSystem::validateParentalPassword(Flow &flow) {
	Script *script = _engine.getScript();
	Graphics::Screen *screen = _engine.getScreen();
	if (!script || !screen)
		return Common::kReadingFailed;

	const Common::String &configuredPassword = script->getParentalPassword();
	if (configuredPassword.empty()) {
		debugC(2, kDebugGeneral, "Harvester: parental password startup gate bypassed state=disabled");
		return Common::kNoError;
	}

	IndexedBitmap backdrop;
	if (!captureScreenBackdrop(*screen, backdrop))
		return Common::kReadingFailed;

	debugC(1, kDebugGeneral, "Harvester: parental password startup gate active");
	Common::String enteredPassword;
	bool accepted = false;
	Common::Error promptError = runPasswordPrompt(
		backdrop, nullptr, 1.0f, flow, false, enteredPassword, accepted);
	if (promptError.getCode() != Common::kNoError || _engine.shouldQuit())
		return promptError;

	if (!accepted || !configuredPassword.equalsIgnoreCase(enteredPassword)) {
		debugC(1, kDebugGeneral,
			"Harvester: parental password blocked startup reason=%s",
			accepted ? "mismatch" : "empty-or-cancelled");
		_engine.quitGame();
		return Common::kNoError;
	}

	debugC(1, kDebugGeneral, "Harvester: parental password accepted; startup permitted");
	return Common::kNoError;
}

Common::Error MenuSystem::runOptionsMenu(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, Flow &flow) {
	const Art *art = _engine.getArt();
	Script *script = _engine.getScript();
	ResourceManager *resources = _engine.getResources();
	const Text *text = _engine.getText();
	const CftFontResource *selectedFontResource = text ? text->findFont("HARVFONT") : nullptr;
	const CftFontResource *unselectedFontResource = text ? text->findFont("HARVFNT2") : nullptr;
	if (!art || !script || !resources || !selectedFontResource || !unselectedFontResource)
		return Common::kReadingFailed;

	HarvesterCftFont selectedFont(*selectedFontResource);
	HarvesterCftFont unselectedFont(*unselectedFontResource);
	if (!selectedFont.isValid() || !unselectedFont.isValid())
		return Common::kReadingFailed;

	const MenuTextConfig &config = flow.getMenuTextConfig();

	IndexedBitmap volumeBar;
	IndexedBitmap indicator;
	if (!loadBitmapResource(*resources, kOptionsVolumeBitmapPath, volumeBar) ||
			!loadBitmapResource(*resources, kOptionsIndicatorBitmapPath, indicator)) {
		return Common::kReadingFailed;
	}

	const int lineHeight = MenuRendering::getNativeRoomMenuLineHeight(selectedFont);
	int selectedItem = 0;
	int capturedSlider = -1;
	bool showingQuickTips = false;
	bool needsRedraw = true;
	uint quickTipIndex = flow.getQuickTipCount() == 0 ? 0 : _engine.getRandomNumber(flow.getQuickTipCount() - 1);
	QuickTipsLayout quickTipsLayout;

	auto persistConfig = [&]() -> bool {
		return script->saveConfig();
	};

	auto updateSlider = [&](int sliderIndex) {
		const int newLevel = resolveOptionsSliderIndexFromMouseX(_mousePos.x);
		switch (sliderIndex) {
		case 0:
			if (newLevel != script->getFxVolumeLevel()) {
				_engine.setFxVolumeLevel(newLevel);
				_engine.playSingleSound(kOptionsPreviewSoundPath);
				persistConfig();
				needsRedraw = true;
			}
			break;
		case 1:
			if (newLevel != script->getMusicVolumeLevel()) {
				_engine.setMusicVolumeLevel(newLevel);
				persistConfig();
				needsRedraw = true;
			}
			break;
		case 2:
			if (newLevel != script->getGammaLevel()) {
				_engine.setGammaLevel(newLevel);
				persistConfig();
				needsRedraw = true;
			}
			break;
		default:
			break;
		}
	};

	auto cycleTextMode = [&]() {
		switch (script->getDialogueTextMode()) {
		case kStartupDialogueTextNone:
			script->setDialogueTextMode(kStartupDialogueTextYes);
			break;
		case kStartupDialogueTextYes:
			script->setDialogueTextMode(kStartupDialogueTextClick);
			break;
		case kStartupDialogueTextClick:
		default:
			script->setDialogueTextMode(kStartupDialogueTextNone);
			break;
		}
		persistConfig();
		needsRedraw = true;
	};

	auto toggleGore = [&]() {
		script->setGoreEnabled(!script->isGoreEnabled());
		ConfMan.setBool("gore", script->isGoreEnabled());
		persistConfig();
		needsRedraw = true;
	};

	auto togglePassword = [&]() {
		if (script->getParentalPassword().empty()) {
			debugC(2, kDebugGeneral, "Harvester: parental password set prompt opened");
			Common::String password;
			bool accepted = false;
			Common::Error promptError = runPasswordPrompt(
				backdrop, palette, paletteBrightness, flow, true, password, accepted);
			if (promptError.getCode() != Common::kNoError) {
				debugC(1, kDebugGeneral,
					"Harvester: parental password set prompt failed error=%d",
					(int)promptError.getCode());
				return;
			}
			if (accepted) {
				script->setParentalPassword(password);
				if (!persistConfig()) {
					script->setParentalPassword(Common::String());
					debugC(1, kDebugGeneral,
						"Harvester: parental password enable failed");
				} else {
					debugC(1, kDebugGeneral,
						"Harvester: parental password enabled");
				}
				needsRedraw = true;
			} else {
				debugC(2, kDebugGeneral,
					"Harvester: parental password set prompt cancelled state=disabled");
			}
		} else {
			const Common::String previousPassword = script->getParentalPassword();
			script->setParentalPassword(Common::String());
			if (!persistConfig()) {
				script->setParentalPassword(previousPassword);
				debugC(1, kDebugGeneral,
					"Harvester: parental password disable failed");
			} else {
				debugC(1, kDebugGeneral,
					"Harvester: parental password disabled");
			}
			needsRedraw = true;
		}
	};

	auto activateSelectedItem = [&]() -> Common::Error {
		switch (selectedItem) {
		case 3:
			cycleTextMode();
			break;
		case 4:
			toggleGore();
			break;
		case 5:
			if (flow.getQuickTipCount() != 0) {
				if (!resolveQuickTipsLayout(_engine, config, quickTipsLayout))
					return Common::kReadingFailed;
				showingQuickTips = true;
				needsRedraw = true;
			}
			break;
		case 6:
			togglePassword();
			break;
		default:
			break;
		}

		return Common::kNoError;
	};

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			if (showingQuickTips) {
				MenuRendering::Renderer(_engine, backdrop, palette, paletteBrightness).renderQuickTipsOverlay(
					config, quickTipsLayout, flow.getQuickTip(quickTipIndex));
			} else {
				MenuRendering::Renderer(_engine, backdrop, palette, paletteBrightness).renderOptionsMenu(
					MenuRendering::FontPair(selectedFont, unselectedFont), *art, config,
					MenuRendering::SliderBitmaps(volumeBar, indicator), selectedItem);
			}
			needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (flow.handleSystemEvent(event, result))
				return result;

			if (showingQuickTips) {
				switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					needsRedraw = true;
					break;
				case Common::EVENT_LBUTTONDOWN:
					if (quickTipsLayout.exitRect.contains(_mousePos)) {
						showingQuickTips = false;
						needsRedraw = true;
					} else if (quickTipsLayout.nextRect.contains(_mousePos)) {
						quickTipIndex = (quickTipIndex + 1) % flow.getQuickTipCount();
						needsRedraw = true;
					} else if (quickTipsLayout.toggleRect.contains(_mousePos)) {
						script->setQuickTipsEnabled(!script->isQuickTipsEnabled());
						if (!resolveQuickTipsLayout(_engine, config, quickTipsLayout))
							return Common::kReadingFailed;
						persistConfig();
						needsRedraw = true;
					}
					break;
				case Common::EVENT_RBUTTONDOWN:
					showingQuickTips = false;
					needsRedraw = true;
					break;
				case Common::EVENT_KEYDOWN:
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						showingQuickTips = false;
						needsRedraw = true;
					}
					break;
				default:
					break;
				}
				continue;
			}

			switch (event.type) {
			case Common::EVENT_MOUSEMOVE: {
				if (capturedSlider != -1) {
					selectedItem = capturedSlider;
					updateSlider(capturedSlider);
					needsRedraw = true;
				} else {
					const int hoveredItem = getNativeRoomMenuSelectionFromMouse(selectedFont,
						config.optionItems.size(), _mousePos);
					if (hoveredItem != -1 && hoveredItem != selectedItem) {
						selectedItem = hoveredItem;
						needsRedraw = true;
					}
				}
				break;
			}
			case Common::EVENT_LBUTTONDOWN: {
				bool handled = false;
				for (int sliderIndex = 0; sliderIndex < 3; ++sliderIndex) {
					if (optionsSliderRect(sliderIndex, lineHeight).contains(_mousePos)) {
						selectedItem = sliderIndex;
						capturedSlider = sliderIndex;
						updateSlider(sliderIndex);
						needsRedraw = true;
						handled = true;
						break;
					}
				}
				if (handled)
					break;

				const int clickedItem = getNativeRoomMenuSelectionFromMouse(selectedFont,
					config.optionItems.size(), _mousePos);
				if (clickedItem == -1)
					break;

				selectedItem = clickedItem;
				needsRedraw = true;
				if (selectedItem >= 3) {
					Common::Error activateError = activateSelectedItem();
					if (activateError.getCode() != Common::kNoError)
						return activateError;
				}
				break;
			}
			case Common::EVENT_LBUTTONUP:
				capturedSlider = -1;
				break;
			case Common::EVENT_RBUTTONDOWN:
				return Common::kNoError;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					return Common::kNoError;
				case Common::KEYCODE_UP:
					selectedItem = (selectedItem + (int)config.optionItems.size() - 1) % (int)config.optionItems.size();
					needsRedraw = true;
					break;
				case Common::KEYCODE_DOWN:
					selectedItem = (selectedItem + 1) % (int)config.optionItems.size();
					needsRedraw = true;
					break;
				case Common::KEYCODE_LEFT:
					if (selectedItem < 3) {
						_mousePos.x = MAX<int>(MenuRendering::kOptionsSliderMinX, _mousePos.x - MenuRendering::kOptionsSliderStep);
						updateSlider(selectedItem);
						needsRedraw = true;
					}
					break;
				case Common::KEYCODE_RIGHT:
					if (selectedItem < 3) {
						_mousePos.x = MIN<int>(kOptionsSliderMaxX, _mousePos.x + MenuRendering::kOptionsSliderStep);
						updateSlider(selectedItem);
						needsRedraw = true;
					}
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER: {
					Common::Error activateError = activateSelectedItem();
					if (activateError.getCode() != Common::kNoError)
						return activateError;
					break;
				}
				case Common::KEYCODE_t:
					selectedItem = 3;
					cycleTextMode();
					break;
				case Common::KEYCODE_g:
					selectedItem = 4;
					toggleGore();
					break;
				case Common::KEYCODE_q:
					selectedItem = 5;
					if (flow.getQuickTipCount() != 0) {
						showingQuickTips = true;
						needsRedraw = true;
					}
					break;
				case Common::KEYCODE_p:
					selectedItem = 6;
					togglePassword();
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		EntityManager *entityManager = _engine.getRuntimeEntities();
		if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error MenuSystem::runHelpScreen(const byte *palette, float paletteBrightness, Flow &flow) {
	ResourceManager *resources = _engine.getResources();
	if (!resources)
		return Common::kReadingFailed;

	IndexedBitmap mouseHelp;
	IndexedBitmap keysHelp;
	byte mouseHelpPalette[256 * 3];
	byte keysHelpPalette[256 * 3];
	if (!loadBitmapResource(*resources, "1:/GRAPHIC/OTHER/MOUSHELP.BM", mouseHelp) ||
			!loadBitmapResource(*resources, "1:/GRAPHIC/OTHER/KEYSHELP.BM", keysHelp) ||
			!loadPaletteResource(*resources, "1:/GRAPHIC/PAL/MOUSHELP.PAL", mouseHelpPalette) ||
			!loadPaletteResource(*resources, "1:/GRAPHIC/PAL/KEYSHELP.PAL", keysHelpPalette)) {
		return Common::kReadingFailed;
	}

	int page = 0;
	bool needsRedraw = true;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			MenuRendering::renderHelpScreen(_engine, page == 0 ? mouseHelp : keysHelp,
				page == 0 ? mouseHelpPalette : keysHelpPalette);
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
			case Common::EVENT_RBUTTONDOWN:
				if (Graphics::Screen *screen = _engine.getScreen()) {
					MenuRendering::applyMenuPalette(*screen, _engine, palette, paletteBrightness);
					screen->fillRect(screen->getBounds(), 0);
				}
				return Common::kNoError;
			case Common::EVENT_LBUTTONDOWN:
				page ^= 1;
				needsRedraw = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					if (Graphics::Screen *screen = _engine.getScreen()) {
						MenuRendering::applyMenuPalette(*screen, _engine, palette, paletteBrightness);
						screen->fillRect(screen->getBounds(), 0);
					}
					return Common::kNoError;
				}
				page ^= 1;
				needsRedraw = true;
				break;
			default:
				break;
			}
		}

		EntityManager *entityManager = _engine.getRuntimeEntities();
		if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
			needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
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
