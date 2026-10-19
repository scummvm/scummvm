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

#include "common/events.h"
#include "common/system.h"
#include "graphics/framelimiter.h"
#include "harvester/cft_font.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/harvester.h"
#include "harvester/scene/room_support.h"
#include "harvester/script/script.h"
#include "harvester/text.h"

namespace Harvester {

static const int kPasswordMaxCharacters = 8;
static const uint32 kPasswordCursorBlinkMs = 480;

static Common::Rect quitConfirmYesRect() {
	return Common::Rect(0xd2, 0xee, 0x104 + 1, 0x108 + 1);
}

static Common::Rect quitConfirmNoRect() {
	return Common::Rect(0x172, 0xee, 0x19a + 1, 0x108 + 1);
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
		float paletteBrightness, Flow &flow, bool drawLogo, PasswordPromptResult &prompt) const {
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

	prompt.password.clear();
	prompt.accepted = false;
	bool needsRedraw = true;
	bool cursorVisible = true;
	uint32 cursorToggleTicks = g_system->getMillis() + kPasswordCursorBlinkMs;
	Graphics::FrameLimiter limiter(g_system, 60);

	while (!_engine.shouldQuit()) {
		if (needsRedraw) {
			MenuRendering::Renderer(_engine, backdrop, palette, paletteBrightness).renderPasswordPrompt(
				MenuRendering::FontPair(titleFont, entryFont), *art,
				flow.getMenuTextConfig().enterPasswordLabel, prompt.password, cursorVisible, drawLogo);
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
					prompt.accepted = !prompt.password.empty();
					return Common::kNoError;
				}
				if (event.kbd.keycode == Common::KEYCODE_BACKSPACE ||
						event.kbd.keycode == Common::KEYCODE_LEFT) {
					needsRedraw |= deleteTextEntryCharacter(prompt.password);
					break;
				}
				if (event.kbd.keycode == Common::KEYCODE_HOME) {
					if (!prompt.password.empty()) {
						prompt.password.clear();
						needsRedraw = true;
					}
					break;
				}
				if (event.kbd.ascii >= 32 && event.kbd.ascii <= 126 && event.kbd.ascii != '~') {
					needsRedraw |= appendTextEntryCharacter(prompt.password, entryFont, event.kbd.ascii,
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
	PasswordPromptResult prompt;
	Common::Error promptError = runPasswordPrompt(
		backdrop, nullptr, 1.0f, flow, false, prompt);
	if (promptError.getCode() != Common::kNoError || _engine.shouldQuit())
		return promptError;

	if (!prompt.accepted || !configuredPassword.equalsIgnoreCase(prompt.password)) {
		debugC(1, kDebugGeneral,
			"Harvester: parental password blocked startup reason=%s",
			prompt.accepted ? "mismatch" : "empty-or-cancelled");
		_engine.quitGame();
		return Common::kNoError;
	}

	debugC(1, kDebugGeneral, "Harvester: parental password accepted; startup permitted");
	return Common::kNoError;
}

} // End of namespace Harvester
