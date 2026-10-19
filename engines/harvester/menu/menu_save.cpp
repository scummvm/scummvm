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
#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/metaengine.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/palette_utils.h"
#include "harvester/text.h"

namespace Harvester {

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
static const uint32 kPaletteFadeTickMs = 4;
static const float kPaletteFadeStep = 0.1f;
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

static Common::String buildDefaultSaveSlotLabel(int slotIndex) {
	return Common::String::format("GAME %02d", slotIndex + 1);
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

struct MenuSystem::SaveSlotEventResult {
	explicit SaveSlotEventResult(SaveSlotCommand command = kNoSlotCommand) : command(command) {}

	SaveSlotCommand command;
	bool redrawAfterActivation = false;
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

MenuSystem::SaveSlotEventResult MenuSystem::handleSaveSlotEvent(SaveSlotMenu &menu,
		SaveLoadMode mode, const Common::Event &event) const {
	SaveSlotEventResult result;
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
			result.command = kConfirmSlot;
			// Cancelling a row-opened save editor does not force a browser redraw.
			result.redrawAfterActivation = mode == kLoadMode;
		} else {
			const SaveSlotCommand command = getSaveSlotCommand(event);
			if (command == kCancelSlot)
				return SaveSlotEventResult(kCancelSlot);
			result.command = command;
			result.redrawAfterActivation = true;
		}
		break;
	}
	case Common::EVENT_KEYDOWN:
		if (getSaveSlotCommand(event) == kCancelSlot)
			return SaveSlotEventResult(kCancelSlot);
		if (event.kbd.keycode == Common::KEYCODE_UP || event.kbd.keycode == Common::KEYCODE_DOWN) {
			const int step = event.kbd.keycode == Common::KEYCODE_UP ? kSaveSlotCount - 1 : 1;
			menu.activeSlot = (menu.activeSlot + step) % kSaveSlotCount;
			menu.statusMessage.clear();
			menu.needsRedraw = true;
		} else if (getSaveSlotCommand(event) == kConfirmSlot) {
			menu.statusMessage.clear();
			result.command = kConfirmSlot;
			result.redrawAfterActivation = true;
		}
		break;
	default:
		break;
	}
	return result;
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

			const SaveSlotEventResult selection = handleSaveSlotEvent(menu, mode, event);
			if (selection.command == kCancelSlot)
				return Common::kNoError;

			if (selection.command == kConfirmSlot) {
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
			if (selection.redrawAfterActivation)
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

} // End of namespace Harvester
