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

static const char *const kLoadGameBitmapPath = "1:/GRAPHIC/OTHER/LOADGAME.BM";
static const char *const kLoadGamePalettePath = "1:/GRAPHIC/PAL/LOADGAME.PAL";
static const char *const kSaveGameBitmapPath = "1:/GRAPHIC/OTHER/SAVEGAME.BM";
static const char *const kSaveGamePalettePath = "1:/GRAPHIC/PAL/SAVEGAME.PAL";
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

} // End of anonymous namespace

namespace {

static Common::String buildDefaultSaveSlotLabel(int slotIndex) {
	return Common::String::format("GAME %02d", slotIndex + 1);
}

} // End of anonymous namespace

bool MenuSystem::loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *palette) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(palette, data.data(), 256 * 3);
	return true;
}

bool MenuSystem::loadBitmapResource(ResourceManager &resources, const Common::String &path, IndexedBitmap &bitmap) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 12)
		return false;

	return decodeBitmap(data, bitmap);
}

int MenuSystem::getNativeRoomMenuSelectionFromMouse(const Graphics::Font &selectedFont, uint itemCount,
		const Common::Point &mousePos) {
	if (itemCount == 0)
		return -1;

	int selection = mousePos.y - MenuRendering::kMenuStartY;
	if (selection < 1)
		selection = 1;

	selection /= MAX<int>(1, MenuRendering::getNativeRoomMenuLineHeight(selectedFont));
	if (selection >= static_cast<int>(itemCount))
		selection = static_cast<int>(itemCount) - 1;

	return selection;
}

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

} // End of namespace Harvester
