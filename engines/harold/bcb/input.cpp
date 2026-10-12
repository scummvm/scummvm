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

#include "common/events.h"
#include "common/system.h"
#include "graphics/screen.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"
#include "video/avi_decoder.h"

namespace BCB {

static const StaticRect kMainSceneMciControlHitRect = {16, 48, 416, 248};
static const StaticRect kTitleMciControlHitRect = {120, 100, 520, 300};
static const StaticRect kMovieQueueControlHitRect = {341, 414, 407, 452};
static const StaticRect kSystemMenuControlHitRect = {265, 414, 330, 452};
static const uint32 kCursorRefreshMillis = 10;
static const int kMovieControlStripNormalSourceX = 0;
static const int kMovieControlStripSkipPressedSourceX = 0x98;
static const int kMovieControlStripPausePressedSourceX = 0x130;

void BCBEngine::handleLeftButtonDown(const Common::Point &mousePosition) {
	if (_activeMovieVideo && shouldDrawMovieControlStrip()) {
		if (kSystemMenuControlHitRect.contains(mousePosition)) {
			_activeMoviePaused = !_activeMoviePaused;
			_activeMovieVideo->pauseVideo(_activeMoviePaused);
			drawMovieControlStrip(_activeMoviePaused ? kMovieControlStripPausePressedSourceX : kMovieControlStripNormalSourceX);
			return;
		}

		if (kMovieQueueControlHitRect.contains(mousePosition)) {
			drawMovieControlStrip(kMovieControlStripSkipPressedSourceX);
			delayWithEventPolling(160, false);
			stopActiveMovieAndDispatch();
			return;
		}
	}

	const bool activeMovieHit =
		(_currentDisplayState == 0 && kTitleMciControlHitRect.contains(mousePosition)) ||
		((_currentDisplayState == 1 || _currentDisplayState == 7) && kMainSceneMciControlHitRect.contains(mousePosition)) ||
		(_currentDisplayState == 8 && kMapSubmodeMciControlHitRect.contains(mousePosition));
	if (_activeMovieVideo && activeMovieHit) {
		if (shouldDrawMovieControlStrip())
			drawMovieControlStrip(kMovieControlStripSkipPressedSourceX);
		delayWithEventPolling(160, false);
		stopActiveMovieAndDispatch();
		return;
	}

	if (_currentDisplayState == 10) {
		transitionFromLogoToTitle();
		return;
	}

	if (_currentDisplayState == 6) {
		playUiSound();
		completePendingStillImageScreen();
		return;
	}

	if ((_currentDisplayState == 1 || _currentDisplayState == 2) && !_activeMovieVideo) {
		if (kSystemMenuControlHitRect.contains(mousePosition)) {
			drawSceneChoiceControlStrip(kMovieControlStripPausePressedSourceX);
			enterSystemMenu();
			return;
		}

		if (kMovieQueueControlHitRect.contains(mousePosition)) {
			playUiSound();
			drawSceneChoiceControlStrip(kMovieControlStripSkipPressedSourceX);
			delayWithEventPolling(160, false);
			_previousDisplayStateBeforeSystem = _currentDisplayState;
			_sceneChoiceActive = false;
			_mapControlsActive = false;
			if (_currentDisplayState == 1) {
				stopSceneIdleMovie();
				_currentDisplayState = 7;
			} else {
				stopMapMoveMovie();
				drawMapSubmodeMovieBackdrop();
				_currentDisplayState = 8;
			}
			_queuedMoviePlaybackIndex = 0;
			processQueuedMovieSequence();
			return;
		}
	}

	if (_currentDisplayState == 1 && _sceneChoiceActive) {
		handleSceneChoiceClick(mousePosition);
		return;
	}

	if (handleComputerClick(mousePosition))
		return;

	if (handleMapControlClick(mousePosition))
		return;

	if (handleSystemClick(mousePosition))
		return;

	if (_currentDisplayState != 0 || !_titleButtonsActive)
		return;

	for (uint i = 1; i < ARRAYSIZE(kTitleButtonTable); ++i) {
		const ButtonControlRecord &button = kTitleButtonTable[i];
		const Common::Rect rect(button.x, button.y, button.x + button.width, button.y + button.height);
		if (!rect.contains(mousePosition))
			continue;

		drawTitleButton(i, true);
		delayWithEventPolling(80);

		switch (button.controlId) {
		case 51:
			startNewGameFromTitle();
			break;
		case 52:
			drawTitleButton(i, false);
			_titleButtonsActive = false;
			_titleLoadSlotsActive = false;
			if (loadGameState(0).getCode() != Common::kNoError)
				_titleButtonsActive = true;
			break;
		case 53:
			_systemButtonsVisible = true;
			_systemSaveLoadSlotActionMode = 2;
			_titleLoadSlotsActive = true;
			drawTitleButton(i, false);
			drawSystemSaveLoadSlots();
			break;
		case 54:
			if (_titleLoadSlotsActive) {
				_titleLoadSlotsActive = false;
				_systemSaveLoadSlotActionMode = 0;
				restoreTitleScreenRegion(Common::Rect(0, 0, 640, 480));
			}
			drawTitleButton(i, false);
			if (showSystemConfirmationDialog(_titleExitConfirmText, true))
				quitGame();
			break;
		default:
			drawTitleButton(i, false);
			break;
		}
		return;
	}
}

bool BCBEngine::pollEvents() {
	return pollEvents(true);
}

bool BCBEngine::pollEvents(bool handleClicks) {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quitGame();
			return false;
		case Common::EVENT_MOUSEMOVE:
			_mousePosition = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mousePosition = event.mouse;
			if (handleClicks && !_suppressClickHandling)
				handleLeftButtonDown(_mousePosition);
			break;
		case Common::EVENT_KEYDOWN:
			if (handleClicks && !_suppressClickHandling) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE && handleEscapeAction()) {
					break;
				} else if (handleComputerKey(event.kbd.keycode)) {
					break;
				} else if (event.kbd.keycode == Common::KEYCODE_SPACE) {
					handleSkipKey();
				} else if (_sceneChoiceActive &&
						   event.kbd.keycode >= Common::KEYCODE_1 &&
						   event.kbd.keycode <= Common::KEYCODE_9) {
					handleSceneChoiceKeyboardSelection(event.kbd.keycode - Common::KEYCODE_1 + 1);
				} else if (_sceneChoiceActive &&
						   event.kbd.keycode >= Common::KEYCODE_KP1 &&
						   event.kbd.keycode <= Common::KEYCODE_KP9) {
					handleSceneChoiceKeyboardSelection(event.kbd.keycode - Common::KEYCODE_KP1 + 1);
				}
			}
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (handleClicks && !_suppressClickHandling)
				handleKeymapperAction(event.customType);
			break;
		default:
			break;
		}
	}

	return !shouldQuit();
}

void BCBEngine::handleKeymapperAction(Common::CustomEventType action) {
	if (action == kBCBActionInteract) {
		handleLeftButtonDown(_mousePosition);
	} else if (action == kBCBActionEscape) {
		handleEscapeAction();
	} else if (action == kBCBActionSkip) {
		handleSkipKey();
	} else if (action >= kBCBActionChoice1 && action <= kBCBActionChoice9) {
		handleSceneChoiceKeyboardSelection(action - kBCBActionChoice1 + 1);
	}
}

bool BCBEngine::handleEscapeAction() {
	if ((_currentDisplayState == 1 || _currentDisplayState == 2) && !_activeMovieVideo) {
		handleLeftButtonDown(Common::Point(kSystemMenuControlHitRect.left, kSystemMenuControlHitRect.top));
		return true;
	}

	if (_currentDisplayState == 3) {
		activateSystemExitButton();
		return true;
	}

	return handleComputerKey(Common::KEYCODE_ESCAPE);
}

void BCBEngine::handleSkipKey() {
	if (_activeMovieVideo) {
		if (shouldDrawMovieControlStrip())
			drawMovieControlStrip(kMovieControlStripSkipPressedSourceX);
		delayWithEventPolling(160, false);
		stopActiveMovieAndDispatch();
		return;
	}

	if (_currentDisplayState == 10) {
		transitionFromLogoToTitle();
		return;
	}

	if (_currentDisplayState == 6) {
		playUiSound();
		completePendingStillImageScreen();
	}
}

void BCBEngine::serviceCursorRefresh(bool screenUpdated) {
	const uint32 now = g_system->getMillis();

	if (screenUpdated) {
		_lastCursorRefreshMillis = now;
		return;
	}

	if (now - _lastCursorRefreshMillis >= kCursorRefreshMillis) {
		_lastCursorRefreshMillis = now;
		_screen->update();
	}
}

void BCBEngine::delayWithEventPolling(uint32 millis) {
	delayWithEventPolling(millis, true);
}

void BCBEngine::delayWithEventPolling(uint32 millis, bool handleClicks) {
	const uint32 start = g_system->getMillis();

	while (!shouldQuit() && g_system->getMillis() - start < millis) {
		pollEvents(handleClicks);
		updateMenuBackgroundMusic();
		serviceCursorRefresh();

		const uint32 elapsed = g_system->getMillis() - start;
		if (elapsed >= millis)
			break;

		g_system->delayMillis(MIN<uint32>(10, millis - elapsed));
	}
}

} // End of namespace BCB
