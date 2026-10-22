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
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/script/script.h"
#include "harvester/text.h"

namespace Harvester {

static const char *const kOptionsVolumeBitmapPath = "1:/GRAPHIC/OTHER/VOLUME.BM";
static const char *const kOptionsIndicatorBitmapPath = "1:/GRAPHIC/OTHER/INDICATR.BM";
static const char *const kOptionsPreviewSoundPath = "1:/SOUND/EFFECTS/WHIP2.WAV";
static const int kOptionsSliderMaxX = 0x22b;
static const int kOptionsSliderHeight = 0x1e;
static const int kStartupOptionMaxLevel = 9;

static int clampStartupOptionLevel(int level) {
	if (level < 0)
		return 0;
	if (level > kStartupOptionMaxLevel)
		return kStartupOptionMaxLevel;

	return level;
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

struct MenuSystem::OptionsMenu {
	OptionsMenu(const IndexedBitmap &backdrop, const byte *palette, float paletteBrightness,
		Flow &flow, Script &script, const MenuTextConfig &config)
		: backdrop(backdrop), palette(palette), paletteBrightness(paletteBrightness),
		  flow(flow), script(script), config(config) {}

	const IndexedBitmap &backdrop;
	const byte *palette;
	float paletteBrightness;
	Flow &flow;
	Script &script;
	const MenuTextConfig &config;
	int lineHeight = 0;
	int selectedItem = 0;
	int capturedSlider = -1;
	bool showingQuickTips = false;
	bool needsRedraw = true;
	uint quickTipIndex = 0;
	QuickTipsLayout quickTipsLayout;
};

void MenuSystem::updateOptionsSlider(OptionsMenu &menu, int sliderIndex) {
	const int newLevel = resolveOptionsSliderIndexFromMouseX(_mousePos.x);
	switch (sliderIndex) {
	case 0:
		if (newLevel != menu.script.getFxVolumeLevel()) {
			_engine.setFxVolumeLevel(newLevel);
			_engine.playSingleSound(kOptionsPreviewSoundPath);
			menu.script.saveConfig();
			menu.needsRedraw = true;
		}
		break;
	case 1:
		if (newLevel != menu.script.getMusicVolumeLevel()) {
			_engine.setMusicVolumeLevel(newLevel);
			menu.script.saveConfig();
			menu.needsRedraw = true;
		}
		break;
	case 2:
		if (newLevel != menu.script.getGammaLevel()) {
			_engine.setGammaLevel(newLevel);
			menu.script.saveConfig();
			menu.needsRedraw = true;
		}
		break;
	default:
		break;
	}
}

void MenuSystem::cycleOptionsTextMode(OptionsMenu &menu) {
	switch (menu.script.getDialogueTextMode()) {
	case kStartupDialogueTextNone:
		menu.script.setDialogueTextMode(kStartupDialogueTextYes);
		break;
	case kStartupDialogueTextYes:
		menu.script.setDialogueTextMode(kStartupDialogueTextClick);
		break;
	case kStartupDialogueTextClick:
	default:
		menu.script.setDialogueTextMode(kStartupDialogueTextNone);
		break;
	}
	menu.script.saveConfig();
	menu.needsRedraw = true;
}

void MenuSystem::toggleOptionsGore(OptionsMenu &menu) {
	menu.script.setGoreEnabled(!menu.script.isGoreEnabled());
	ConfMan.setBool("gore", menu.script.isGoreEnabled());
	menu.script.saveConfig();
	menu.needsRedraw = true;
}

void MenuSystem::toggleOptionsPassword(OptionsMenu &menu) {
	if (menu.script.getParentalPassword().empty()) {
		debugC(2, kDebugGeneral, "Harvester: parental password set prompt opened");
		PasswordPromptResult prompt;
		Common::Error promptError = runPasswordPrompt(
			menu.backdrop, menu.palette, menu.paletteBrightness, menu.flow, true, prompt);
		if (promptError.getCode() != Common::kNoError) {
			debugC(1, kDebugGeneral,
				"Harvester: parental password set prompt failed error=%d",
				static_cast<int>(promptError.getCode()));
			return;
		}
		if (prompt.accepted) {
			menu.script.setParentalPassword(prompt.password);
			if (!menu.script.saveConfig()) {
				menu.script.setParentalPassword(Common::String());
				debugC(1, kDebugGeneral,
					"Harvester: parental password enable failed");
			} else {
				debugC(1, kDebugGeneral,
					"Harvester: parental password enabled");
			}
			menu.needsRedraw = true;
		} else {
			debugC(2, kDebugGeneral,
				"Harvester: parental password set prompt cancelled state=disabled");
		}
	} else {
		const Common::String previousPassword = menu.script.getParentalPassword();
		menu.script.setParentalPassword(Common::String());
		if (!menu.script.saveConfig()) {
			menu.script.setParentalPassword(previousPassword);
			debugC(1, kDebugGeneral,
				"Harvester: parental password disable failed");
		} else {
			debugC(1, kDebugGeneral,
				"Harvester: parental password disabled");
		}
		menu.needsRedraw = true;
	}
}

Common::Error MenuSystem::activateOptionsItem(OptionsMenu &menu) {
	switch (menu.selectedItem) {
	case 3:
		cycleOptionsTextMode(menu);
		break;
	case 4:
		toggleOptionsGore(menu);
		break;
	case 5:
		if (menu.flow.getQuickTipCount() != 0) {
			if (!resolveQuickTipsLayout(_engine, menu.config, menu.quickTipsLayout))
				return Common::kReadingFailed;
			menu.showingQuickTips = true;
			menu.needsRedraw = true;
		}
		break;
	case 6:
		toggleOptionsPassword(menu);
		break;
	default:
		break;
	}

	return Common::kNoError;
}

Common::Error MenuSystem::handleOptionsQuickTipsEvent(OptionsMenu &menu, const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		menu.needsRedraw = true;
		break;
	case Common::EVENT_LBUTTONDOWN:
		if (menu.quickTipsLayout.exitRect.contains(_mousePos)) {
			menu.showingQuickTips = false;
			menu.needsRedraw = true;
		} else if (menu.quickTipsLayout.nextRect.contains(_mousePos)) {
			menu.quickTipIndex = (menu.quickTipIndex + 1) % menu.flow.getQuickTipCount();
			menu.needsRedraw = true;
		} else if (menu.quickTipsLayout.toggleRect.contains(_mousePos)) {
			menu.script.setQuickTipsEnabled(!menu.script.isQuickTipsEnabled());
			if (!resolveQuickTipsLayout(_engine, menu.config, menu.quickTipsLayout))
				return Common::kReadingFailed;
			menu.script.saveConfig();
			menu.needsRedraw = true;
		}
		break;
	case Common::EVENT_RBUTTONDOWN:
		menu.showingQuickTips = false;
		menu.needsRedraw = true;
		break;
	case Common::EVENT_KEYDOWN:
		if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			menu.showingQuickTips = false;
			menu.needsRedraw = true;
		}
		break;
	default:
		break;
	}
	return Common::kNoError;
}

void MenuSystem::handleOptionsMouseMove(OptionsMenu &menu, const Graphics::Font &selectedFont) {
	if (menu.capturedSlider != -1) {
		menu.selectedItem = menu.capturedSlider;
		updateOptionsSlider(menu, menu.capturedSlider);
		menu.needsRedraw = true;
	} else {
		const int hoveredItem = getNativeRoomMenuSelectionFromMouse(selectedFont,
			menu.config.optionItems.size(), _mousePos);
		if (hoveredItem != -1 && hoveredItem != menu.selectedItem) {
			menu.selectedItem = hoveredItem;
			menu.needsRedraw = true;
		}
	}
}

Common::Error MenuSystem::handleOptionsMouseDown(OptionsMenu &menu, const Graphics::Font &selectedFont) {
	bool handled = false;
	for (int sliderIndex = 0; sliderIndex < 3; ++sliderIndex) {
		if (optionsSliderRect(sliderIndex, menu.lineHeight).contains(_mousePos)) {
			menu.selectedItem = sliderIndex;
			menu.capturedSlider = sliderIndex;
			updateOptionsSlider(menu, sliderIndex);
			menu.needsRedraw = true;
			handled = true;
			break;
		}
	}
	if (handled)
		return Common::kNoError;

	const int clickedItem = getNativeRoomMenuSelectionFromMouse(selectedFont,
		menu.config.optionItems.size(), _mousePos);
	if (clickedItem == -1)
		return Common::kNoError;

	menu.selectedItem = clickedItem;
	menu.needsRedraw = true;
	if (menu.selectedItem >= 3) {
		Common::Error activateError = activateOptionsItem(menu);
		if (activateError.getCode() != Common::kNoError)
			return activateError;
	}
	return Common::kNoError;
}

Common::Error MenuSystem::handleOptionsKey(OptionsMenu &menu, const Common::Event &event, bool &closed) {
	switch (event.kbd.keycode) {
	case Common::KEYCODE_ESCAPE:
		closed = true;
		return Common::kNoError;
	case Common::KEYCODE_UP:
		menu.selectedItem = (menu.selectedItem + static_cast<int>(menu.config.optionItems.size()) - 1)
			% static_cast<int>(menu.config.optionItems.size());
		menu.needsRedraw = true;
		break;
	case Common::KEYCODE_DOWN:
		menu.selectedItem = (menu.selectedItem + 1) % static_cast<int>(menu.config.optionItems.size());
		menu.needsRedraw = true;
		break;
	case Common::KEYCODE_LEFT:
		if (menu.selectedItem < 3) {
			_mousePos.x = MAX<int>(MenuRendering::kOptionsSliderMinX, _mousePos.x - MenuRendering::kOptionsSliderStep);
			updateOptionsSlider(menu, menu.selectedItem);
			menu.needsRedraw = true;
		}
		break;
	case Common::KEYCODE_RIGHT:
		if (menu.selectedItem < 3) {
			_mousePos.x = MIN<int>(kOptionsSliderMaxX, _mousePos.x + MenuRendering::kOptionsSliderStep);
			updateOptionsSlider(menu, menu.selectedItem);
			menu.needsRedraw = true;
		}
		break;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER: {
		Common::Error activateError = activateOptionsItem(menu);
		if (activateError.getCode() != Common::kNoError)
			return activateError;
		break;
	}
	case Common::KEYCODE_t:
		menu.selectedItem = 3;
		cycleOptionsTextMode(menu);
		break;
	case Common::KEYCODE_g:
		menu.selectedItem = 4;
		toggleOptionsGore(menu);
		break;
	case Common::KEYCODE_q:
		menu.selectedItem = 5;
		if (menu.flow.getQuickTipCount() != 0) {
			menu.showingQuickTips = true;
			menu.needsRedraw = true;
		}
		break;
	case Common::KEYCODE_p:
		menu.selectedItem = 6;
		toggleOptionsPassword(menu);
		break;
	default:
		break;
	}
	return Common::kNoError;
}

Common::Error MenuSystem::handleOptionsEvent(OptionsMenu &menu, const Common::Event &event,
		const Graphics::Font &selectedFont, bool &closed) {
	if (menu.showingQuickTips)
		return handleOptionsQuickTipsEvent(menu, event);

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		handleOptionsMouseMove(menu, selectedFont);
		break;
	case Common::EVENT_LBUTTONDOWN:
		return handleOptionsMouseDown(menu, selectedFont);
	case Common::EVENT_LBUTTONUP:
		menu.capturedSlider = -1;
		break;
	case Common::EVENT_RBUTTONDOWN:
		closed = true;
		return Common::kNoError;
	case Common::EVENT_KEYDOWN:
		return handleOptionsKey(menu, event, closed);
	default:
		break;
	}
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

	OptionsMenu menu(backdrop, palette, paletteBrightness, flow, *script, config);
	menu.lineHeight = MenuRendering::getNativeRoomMenuLineHeight(selectedFont);
	menu.quickTipIndex = flow.getQuickTipCount() == 0 ? 0 : _engine.getRandomNumber(flow.getQuickTipCount() - 1);

	Graphics::FrameLimiter limiter(g_system, 60);
	while (!_engine.shouldQuit()) {
		if (menu.needsRedraw) {
			if (menu.showingQuickTips) {
				MenuRendering::Renderer(_engine, menu.backdrop, menu.palette, menu.paletteBrightness).renderQuickTipsOverlay(
					menu.config, menu.quickTipsLayout, menu.flow.getQuickTip(menu.quickTipIndex));
			} else {
				MenuRendering::Renderer(_engine, menu.backdrop, menu.palette, menu.paletteBrightness).renderOptionsMenu(
					MenuRendering::FontPair(selectedFont, unselectedFont), *art, menu.config,
					MenuRendering::SliderBitmaps(volumeBar, indicator), menu.selectedItem);
			}
			menu.needsRedraw = false;
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (menu.flow.handleSystemEvent(event, result))
				return result;

			bool closed = false;
			Common::Error eventError = handleOptionsEvent(menu, event, selectedFont, closed);
			if (eventError.getCode() != Common::kNoError || closed)
				return eventError;
		}

		EntityManager *entityManager = _engine.getRuntimeEntities();
		if (entityManager && entityManager->syncCursorEntityPosition(_mousePos))
			menu.needsRedraw = true;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

} // End of namespace Harvester
