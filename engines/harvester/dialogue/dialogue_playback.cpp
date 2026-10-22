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

#include "harvester/dialogue/dialogue_session.h"
#include "harvester/dialogue/dialogue_render.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/framelimiter.h"
#include "harvester/art.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/flow/flow.h"
#include "harvester/flow/flow_support.h"
#include "harvester/fst_player.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "harvester/scene/room_presentation.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const char *const kCdChangePromptPalettePath = "1:/GRAPHIC/PAL/CD1.PAL";

static bool isDialogueVoiceInterruptEvent(const Common::Event &event) {
	return event.type == Common::EVENT_LBUTTONDOWN ||
		event.type == Common::EVENT_RBUTTONDOWN ||
		(event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE);
}

static bool isDialoguePointerPressEvent(const Common::Event &event) {
	return event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN;
}

static bool isDialogueContinueEvent(const Common::Event &event) {
	return isDialoguePointerPressEvent(event) || event.type == Common::EVENT_KEYDOWN;
}

static bool loadPaletteResource(ResourceManager &resources, const Common::String &path, byte *dest) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 256 * 3)
		return false;

	memcpy(dest, data.data(), 256 * 3);
	return true;
}

static bool isPrimaryMouseDown() {
	return g_system && g_system->getEventManager() &&
		((g_system->getEventManager()->getButtonState() & 1) != 0);
}

static bool isSecondaryMouseDown() {
	return g_system && g_system->getEventManager() &&
		((g_system->getEventManager()->getButtonState() & 2) != 0);
}

static Common::String buildDialogueVoicePath(const Script &script, int wavId) {
	if (wavId <= 0)
		return Common::String();

	return Common::String::format("%s%d.CMP", script.getVoicePath().c_str(), wavId);
}

} // End of anonymous namespace

Common::Error RoomNpcDialogueSession::playDialogueLineWithVariant(int wavId, const Common::String &speakerId,
		int headVariant) {
	setActiveSpeakerPortrait(speakerId, headVariant);

	const StartupDialogueTextMode textMode = _script->getDialogueTextMode();
	Common::String subtitleText;
	const bool textEnabled = textMode != kStartupDialogueTextNone &&
		_text->resolveDialogueSubtitle(wavId, subtitleText);
	Common::Array<Common::String> subtitleLines;
	const IndexedBitmap *textboxBitmap = nullptr;
	if (textEnabled) {
		DialogueRendering::wrapDialogueTextLikeNative(*_subtitleFont, _subtitleFontUsesCft,
			subtitleText, DialogueRendering::kDialogueSubtitleTextWidth, subtitleLines);
		textboxBitmap = _art->getTextboxBitmap(DialogueRendering::resolveDialogueTextboxIndex(subtitleLines.size()));
	}

	const Common::String voicePath = buildDialogueVoicePath(*_script, wavId);
	const bool voiceStarted = !voicePath.empty() && _engine.playSpeech(voicePath);
	debugC(2, kDebugDialogue,
		"Harvester: dialogue line wav=0x%x speaker='%s' headVariant=%d voice='%s' textMode=%d subtitle='%s'",
		wavId, speakerId.c_str(), headVariant, voicePath.c_str(), (int)textMode,
		textEnabled ? subtitleText.c_str() : "");
	Common::Error releaseError = waitForPointerRelease();
	if (releaseError.getCode() != Common::kNoError) {
		_engine.stopSpeech();
		return releaseError;
	}

	bool voiceInterrupted = false;
	bool pointerInterrupted = false;
	Graphics::FrameLimiter limiter(g_system, 60);
	for (;;) {
		drawDialogueOverlay(textboxBitmap, textEnabled ? &subtitleLines : nullptr, nullptr, -1, false, nullptr);

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (_flow.handleSystemEvent(event, result)) {
				_engine.stopSpeech();
				return result;
			}

			if (isDialogueVoiceInterruptEvent(event)) {
				voiceInterrupted = true;
				if (isDialoguePointerPressEvent(event))
					pointerInterrupted = true;
			}
		}

		if (_entityManager)
			(void)_entityManager->syncCursorEntityPosition(_mousePos);
		if (voiceInterrupted || (!voiceStarted || !_engine.isSpeechPlaying()))
			break;

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	_engine.stopSpeech();
	if (pointerInterrupted) {
		Common::Error releaseResult = waitForPointerRelease();
		if (releaseResult.getCode() != Common::kNoError)
			return releaseResult;
	} else if (textMode == kStartupDialogueTextClick) {
		return waitForDialogueContinue(textboxBitmap, textEnabled ? &subtitleLines : nullptr);
	}

	return Common::kNoError;
}

Common::Error RoomNpcDialogueSession::waitForDialogueContinue(const IndexedBitmap *textboxBitmap,
		const Common::Array<Common::String> *subtitleLines) {
	Graphics::FrameLimiter clickLimiter(g_system, 60);
	for (;;) {
		drawDialogueOverlay(textboxBitmap, subtitleLines,
			nullptr, -1, false, nullptr);

		bool continuePressed = false;
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (_flow.handleSystemEvent(event, result))
				return result;

			if (isDialogueContinueEvent(event))
				continuePressed = true;
		}

		if (_entityManager)
			(void)_entityManager->syncCursorEntityPosition(_mousePos);
		if (continuePressed)
			break;

		clickLimiter.delayBeforeSwap();
		clickLimiter.startFrame();
	}

	Common::Error releaseResult = waitForPointerRelease();
	if (releaseResult.getCode() != Common::kNoError)
		return releaseResult;

	return Common::kNoError;
}

Common::Error RoomNpcDialogueSession::playDialogueLine(int wavId, const Common::String &speakerId) {
	return playDialogueLineWithVariant(wavId, speakerId, 0);
}

Common::Error RoomNpcDialogueSession::playDialogueEntrySequence(const DialogueLineEntry *lines, uint count) {
	for (uint i = 0; i < count; ++i) {
		Common::Error lineError = playDialogueLineWithVariant(
			lines[i].wavId, lines[i].speakerId, lines[i].headVariant);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return Common::kNoError;
}

Common::Error RoomNpcDialogueSession::playDialogueFst(const Common::String &path) {
	FstPlayer fstPlayer(_engine);
	if (!fstPlayer.play(path))
		return Common::kReadingFailed;

	return Common::kNoError;
}

Common::Error RoomNpcDialogueSession::clearScreenToBlack() {
	Graphics::Screen *screen = _engine.getScreen();
	if (!screen)
		return Common::kReadingFailed;

	byte blackPalette[256 * 3];
	memset(blackPalette, 0, sizeof(blackPalette));
	screen->fillRect(screen->getBounds(), 0);
	setScaledPalette(*screen, blackPalette, 1.0f);
	screen->makeAllDirty();
	screen->update();
	return Common::kNoError;
}

Common::Error RoomNpcDialogueSession::showCdChangePrompt(int discNumber) {
	if (discNumber <= 0)
		return Common::kNoError;

	ResourceManager *resources = _engine.getResources();
	if (!resources)
		return Common::kReadingFailed;
	const int previousDisc = resources->getCurrentDisc();

	if (_engine.shouldShowCdChangePrompts()) {
		Graphics::Screen *screen = _engine.getScreen();
		if (!screen)
			return Common::kReadingFailed;

		const Common::String bitmapPath =
			Common::String::format("1:/GRAPHIC/OTHER/CD%d.BM", discNumber);
		IndexedBitmap promptBitmap;
		byte promptPalette[256 * 3];
		if (!loadBitmapResource(*resources, bitmapPath, promptBitmap) ||
				!loadPaletteResource(*resources, kCdChangePromptPalettePath, promptPalette)) {
			warning("Harvester: unable to load CD change prompt assets for disc %d", discNumber);
			return Common::kReadingFailed;
		}

		_engine.stopMusic();

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (_flow.handleSystemEvent(event, result))
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
				screen->fillRect(screen->getBounds(), 0);
				RoomPresentation::blitBitmap(*screen, promptBitmap, 0, 0);
				setScaledPalette(*screen, promptPalette, 1.0f);
				if (_entityManager)
					_entityManager->drawCursor(*screen);
				screen->makeAllDirty();
				screen->update();
				needsRedraw = false;
			}

			while (g_system->getEventManager()->pollEvent(event)) {
				Common::Error result = Common::kNoError;
				if (_flow.handleSystemEvent(event, result))
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

			if (_entityManager && _entityManager->syncCursorEntityPosition(_mousePos))
				needsRedraw = true;

			limiter.delayBeforeSwap();
			limiter.startFrame();
		}
	}

	return activateDialogueDisc(*resources, discNumber, previousDisc);
}

Common::Error RoomNpcDialogueSession::activateDialogueDisc(ResourceManager &resources,
		int discNumber, int previousDisc) {
	if (!_engine.activateDisc(discNumber)) {
		warning("Harvester: unable to activate disc %d resources", discNumber);
		return Common::kReadingFailed;
	}
	if ((discNumber == 1 || discNumber == 3) &&
			previousDisc > 0 && previousDisc != resources.getCurrentDisc()) {
		Script *script = _engine.getScript();
		if (!script || !script->reloadTownWorld(resources)) {
			warning("Harvester: unable to reload town script after disc prompt %d -> %d",
				previousDisc, discNumber);
			return Common::kReadingFailed;
		}
	}

	return Common::kNoError;
}

Common::Error RoomNpcDialogueSession::runGameOverScreen() {
	debugC(1, kDebugDialogue, "Harvester: dialogue death requesting game-over menu");
	_flow.requestGameOverReturn();
	return Common::kNoError;
}

Common::Error RoomNpcDialogueSession::waitForPointerRelease() {
	Graphics::FrameLimiter limiter(g_system, 60);
	while (!_engine.shouldQuit() && (isPrimaryMouseDown() || isSecondaryMouseDown())) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			Common::Error result = Common::kNoError;
			if (_flow.handleSystemEvent(event, result))
				return result;
		}

		if (_entityManager)
			(void)_entityManager->syncCursorEntityPosition(_mousePos);
		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

} // End of namespace Harvester
