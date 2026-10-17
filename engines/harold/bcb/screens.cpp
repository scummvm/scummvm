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

#include "common/system.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"

namespace BCB {

static const int kGameOverTextLineGap = 2;

bool BCBEngine::showInitialLogoScreen() {
	Graphics::Surface *logo = loadBitmap("16mlogo.bmp");
	if (!logo)
		return false;

	blitScreenWithLineReveal(*logo);

	logo->free();
	delete logo;

	_currentDisplayState = 10;
	_titleButtonsActive = false;
	_systemButtonsVisible = false;
	_initialLogoElapsedTimerTicks = 0;
	_lastTimerTick = g_system->getMillis();
	return true;
}

bool BCBEngine::showTitleScreen() {
	stopGameOverMusic();
	if (_titleBitmap && _titleBitmap->format != g_system->getScreenFormat()) {
		_titleBitmap->free();
		delete _titleBitmap;
		_titleBitmap = nullptr;
	}

	if (!_titleBitmap)
		_titleBitmap = loadBitmap("16mtitle.bmp");
	if (!_titleBitmap)
		return false;

	blitScreenWithLineReveal(*_titleBitmap);

	_currentDisplayState = 0;
	_titleButtonsActive = true;
	_systemButtonsVisible = false;
	_titleLoadSlotsActive = false;
	return true;
}

void BCBEngine::restoreTitleScreenRegion(const Common::Rect &rect, bool present) {
	if (!_titleBitmap)
		return;

	const Common::Rect titleBounds(0, 0, _titleBitmap->w, _titleBitmap->h);
	Common::Rect clippedRect = rect;
	clippedRect.clip(titleBounds);
	if (clippedRect.isEmpty())
		return;

	_screen->copyRectToSurface(_titleBitmap->getBasePtr(clippedRect.left, clippedRect.top), _titleBitmap->pitch,
							   clippedRect.left, clippedRect.top, clippedRect.width(), clippedRect.height());
	if (present)
		_screen->update();
}

bool BCBEngine::transitionFromLogoToTitle() {
	if (!showTitleScreen())
		return false;

	startMenuBackgroundMusic();
	return true;
}

bool BCBEngine::startNewGameFromTitle() {
	_titleButtonsActive = false;
	_systemButtonsVisible = false;
	stopMenuBackgroundMusic();
	stopGameOverMusic();

	_screen->clear(g_system->getScreenFormat().RGBToColor(0, 0, 0));
	_screen->update();

	stateDword(kStateCurrentScriptSequenceId) = 1;
	return playNumberedMovie(6, true);
}

bool BCBEngine::showPendingStillImageScreen() {
	const int stillImageToShowId = stateDword(kStateStillImageToShowId);
	if (stillImageToShowId == 0)
		return false;

	const int stillIndex = stillImageToShowId - 0x34;
	if (stillIndex < 0 || stillIndex >= ARRAYSIZE(kStillImageHighColorPaths)) {
		warning("BCBEngine::showPendingStillImageScreen(): Unsupported still id %d", stillImageToShowId);
		stateDword(kStateStillImageToShowId) = 0;
		return false;
	}

	Graphics::Surface *still = loadDiscBitmap(kStillImageHighColorPaths[stillIndex]);
	if (!still)
		return false;

	_screen->copyRectToSurface(still->getPixels(), still->pitch, 0, 0, MIN<int>(still->w, 640), MIN<int>(still->h, 480));
	_screen->update();
	still->free();
	delete still;

	_currentDisplayState = 6;
	_pendingStillElapsedTimerTicks = 0;
	_lastTimerTick = g_system->getMillis();
	_pendingStillDurationIndex = stillIndex;
	stateDword(kStateStillImageToShowId) = 0;
	return true;
}

void BCBEngine::completePendingStillImageScreen() {
	if (_currentDisplayState != 6)
		return;

	if (stateDword(kStateClearDisplayBeforeMediaCheckFlag) == 0)
		showMainSceneScreen();
	processGameStateDispatcher(true);
}

bool BCBEngine::dispatchGameOverScreen() {
	stopSceneIdleMovie();
	stopMapMoveMovie();
	stopActiveMovie();
	stopMenuBackgroundMusic();
	_sceneChoiceActive = false;
	_mapControlsActive = false;
	freeSceneChoiceBacking();

	Graphics::Surface *gameOver = loadDiscBitmapFromAnyDisc("bmp/still/16m/pover.bmp");
	if (!gameOver)
		return false;

	const uint selectedSceneChoiceValue = stateDword(kStateSelectedSceneChoiceValue);
	if (selectedSceneChoiceValue < _gameOverText.size() && !_gameOverText[selectedSceneChoiceValue].empty()) {
		const uint32 white = gameOver->format.RGBToColor(255, 255, 255);
		const Common::Rect textRect(100, 200, 540, 400);
		if (_gameOverTextFont) {
			const Common::U32String &text = _gameOverText[selectedSceneChoiceValue];
			int y = textRect.top - (_gameDescription->desc.language == Common::KO_KOR ? 1 : 0);
			for (uint lineStart = 0; lineStart < text.size() && y < textRect.bottom;) {
				uint lineEnd = lineStart;
				uint lastFit = lineStart;
				while (lineEnd < text.size() && text[lineEnd] != '\n') {
					++lineEnd;
					const Common::U32String candidate(text.c_str() + lineStart, lineEnd - lineStart);
					if (_gameOverTextFont->getStringWidth(candidate) <= textRect.width())
						lastFit = lineEnd;
					else
						break;
				}

				if (lastFit == lineStart && lineEnd > lineStart)
					lastFit = lineStart + 1;
				if (lineEnd < text.size() && text[lineEnd] == '\n')
					lastFit = lineEnd;

				_gameOverTextFont->drawString(gameOver, Common::U32String(text.c_str() + lineStart, lastFit - lineStart), textRect.left, y, textRect.width(), white, Graphics::kTextAlignLeft);
				y += _gameOverTextFont->getFontHeight() + kGameOverTextLineGap;
				lineStart = lastFit;
				if (lineStart < text.size() && text[lineStart] == '\n')
					++lineStart;
			}
		}
	}

	blitScreenWithLineReveal(*gameOver, false);
	gameOver->free();
	delete gameOver;

	_currentDisplayState = 9;
	_gameOverElapsedTimerTicks = 0;
	_lastTimerTick = g_system->getMillis();
	_screen->update();
	startGameOverMusic();
	return true;
}

bool BCBEngine::dispatchEndingStillDisplayState() {
	_currentDisplayState = 9;
	return true;
}

void BCBEngine::drawTitleButton(uint buttonIndex, bool pressed) {
	if (!_titleBitmap || buttonIndex == 0 || buttonIndex >= ARRAYSIZE(kTitleButtonTable))
		return;

	const ButtonControlRecord &button = kTitleButtonTable[buttonIndex];
	const int sourceX = pressed ? 0xa2 : button.x;
	const int sourceY = pressed ? (int)((buttonIndex + 0x16) * 0x20) : button.y;

	if (sourceX + button.width > _titleBitmap->w || sourceY + button.height > _titleBitmap->h)
		return;

	_screen->copyRectToSurface(_titleBitmap->getBasePtr(sourceX, sourceY), _titleBitmap->pitch, button.x, button.y, button.width, button.height);
	_screen->update();
}

void BCBEngine::blitScreenWithLineReveal(const Graphics::Surface &surface, bool presentFinalFrame) {
	const int width = MIN<int>(surface.w, 640);
	const int height = MIN<int>(surface.h, 480);
	const int revealSteps = (height + 1) / 2;

	const uint32 revealStartMillis = g_system->getMillis();
	int revealedSteps = 0;
	while (revealedSteps < revealSteps && !shouldQuit()) {
		uint32 elapsedMillis = g_system->getMillis() - revealStartMillis;
		int targetSteps = MIN<int>(elapsedMillis / 2 + 1, revealSteps);
		if (targetSteps <= revealedSteps) {
			pollEvents(false);
			serviceCursorRefresh();
			g_system->delayMillis(1);
			continue;
		}

		for (; revealedSteps < targetSteps; ++revealedSteps) {
			const int topY = revealedSteps * 2;
			const int bottomY = height - 1 - revealedSteps * 2;
			if (topY < height)
				_screen->copyRectToSurface(surface.getBasePtr(0, topY), surface.pitch, 0, topY, width, 1);
			if (bottomY != topY && bottomY >= 0)
				_screen->copyRectToSurface(surface.getBasePtr(0, bottomY), surface.pitch, 0, bottomY, width, 1);
		}

		if (presentFinalFrame || revealedSteps < revealSteps)
			_screen->update();
		pollEvents(false);
	}
}

} // End of namespace BCB
