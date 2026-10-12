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
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"
#include "video/avi_decoder.h"

#include <math.h>

namespace BCB {

bool BCBEngine::showMainSceneScreen() {
	_sceneChoiceActive = false;
	_mapControlsActive = false;
	freeSceneChoiceBacking();
	return drawMainSceneScreen(true);
}

bool BCBEngine::drawMainSceneScreen(bool lineReveal) {
	const bool mainSceneNightFlag = stateDword(kStateMainSceneNightFlag) != 0;
	Graphics::Surface *mainScene = loadBitmap(mainSceneNightFlag ? "16mmainn.bmp" : "16mmain.bmp");
	if (!mainScene)
		return false;

	applyPendingClockTickDelta();
	renderClockAndChipToSurface(*mainScene, 0, stateDword(kStateGameClockTicks));
	if (lineReveal) {
		blitScreenWithLineReveal(*mainScene, false);
	} else {
		g_system->copyRectToScreen(mainScene->getPixels(), mainScene->pitch, 0, 0, MIN<int>(mainScene->w, 640), MIN<int>(mainScene->h, 480));
	}
	mainScene->free();
	delete mainScene;
	_currentDisplayState = 1;
	_sceneIdleMovieTimerTicks = 0;
	_lastTimerTick = g_system->getMillis();
	g_system->updateScreen();
	return true;
}

void BCBEngine::captureSceneChoiceBacking() {
	freeSceneChoiceBacking();
	for (uint i = 1; i < ARRAYSIZE(kSceneChoiceButtonTable); ++i) {
		const ButtonControlRecord &button = kSceneChoiceButtonTable[i];
		const Common::Rect rect(button.x, button.y, button.x + button.width, button.y + button.height);
		if (i == 1)
			_sceneChoiceBackingRect = rect;
		else
			_sceneChoiceBackingRect.extend(rect);
	}

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;

	_sceneChoiceBacking = new Graphics::Surface();
	_sceneChoiceBacking->create(_sceneChoiceBackingRect.width(), _sceneChoiceBackingRect.height(), screen->format);
	_sceneChoiceBacking->copyRectToSurface(*screen, 0, 0, _sceneChoiceBackingRect);
	g_system->unlockScreen();
}

void BCBEngine::restoreSceneChoiceBacking(bool present) {
	if (!_sceneChoiceBacking)
		return;

	for (int buttonIndex = 1; buttonIndex <= _sceneChoiceEnabledButtonLimit && buttonIndex < (int)ARRAYSIZE(kSceneChoiceButtonTable); ++buttonIndex) {
		if (!isSceneChoiceButtonVisible(buttonIndex))
			continue;

		const ButtonControlRecord &button = kSceneChoiceButtonTable[buttonIndex];
		g_system->copyRectToScreen(_sceneChoiceBacking->getBasePtr(button.x - _sceneChoiceBackingRect.left, button.y - _sceneChoiceBackingRect.top), _sceneChoiceBacking->pitch, button.x, button.y, button.width, button.height);
	}

	if (present)
		g_system->updateScreen();
}

void BCBEngine::freeSceneChoiceBacking() {
	if (!_sceneChoiceBacking)
		return;

	_sceneChoiceBacking->free();
	delete _sceneChoiceBacking;
	_sceneChoiceBacking = nullptr;
}

void BCBEngine::clearSceneChoiceAfterSelection() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (screen) {
		screen->fillRect(Common::Rect(0x10, 0x30, 0x1a0, 0xf8), screen->format.RGBToColor(0, 0, 0));
		g_system->unlockScreen();
	}

	freeSceneChoiceBacking();
	g_system->updateScreen();
}

bool BCBEngine::isSceneChoiceButtonVisible(int buttonIndex) const {
	if (buttonIndex <= 0 || buttonIndex >= (int)ARRAYSIZE(kSceneChoiceButtonTable))
		return false;

	const int sceneChoiceUiSetToShowId = stateDword(kStateSceneChoiceUiSetToShowId);
	const int choiceCount = (sceneChoiceUiSetToShowId >= 0 && sceneChoiceUiSetToShowId < (int)_sceneChoiceText.size()) ? MIN<int>(_sceneChoiceText[sceneChoiceUiSetToShowId].size(), 8) : 0;

	return buttonIndex <= choiceCount ||
		   (buttonIndex == _sceneChoiceTerminatorButtonIndex && stateDword(kStateSceneChoiceExtraButtonFlag) != 0);
}

void BCBEngine::applyPendingClockTickDelta() {
	stateDword(kStateGameClockTicks) += _pendingClockTickDelta;
	_pendingClockTickDelta = 0;
	notifyGameClockTicksChanged();
}

void BCBEngine::notifyGameClockTicksChanged() {
	const int ticks = stateDword(kStateGameClockTicks);
	if (_lastLoggedGameClockTicks == ticks)
		return;

	_lastLoggedGameClockTicks = ticks;
	const int clockTicks = ticks % 1440;
	const int dayOfMonth = 10 + CLIP<int>(stateDword(kStateRequiredDiscIndex), 0, 4);
	const int hour = clockTicks / 60;
	const int minute = clockTicks % 60;
	const Common::String message = Common::String::format("BCB clock: 11/%02d %02d:%02d (ticks=%d / 0x%04x)\n", dayOfMonth, hour, minute, ticks, ticks);
	g_system->logMessage(LogMessageType::kInfo, message.c_str());
}

void BCBEngine::drawClockAndChipIndicator(bool present) {
	applyPendingClockTickDelta();
	drawAnalogClockAndChip(0, stateDword(kStateGameClockTicks), present);
}

void BCBEngine::renderClockAndChipToSurface(Graphics::Surface &surface, int baseHourOffset, int gameClockTicks) {
	static const double kClockDegreesToRadians = 0.017453292519943295769;
	const int clockTicks = gameClockTicks % 1440;
	const double hourAngle = (30.0 * (baseHourOffset % 24) + clockTicks / 2.0) * kClockDegreesToRadians;
	const double minuteAngle = (clockTicks * 6.0) * kClockDegreesToRadians;
	const int centerX = 0x55;
	const int centerY = 0x198;

	const uint32 handColor = surface.format.RGBToColor(0x50, 0x50, 0x50);
	const int hourEndX = centerX + (int)floor(sin(hourAngle) * 25.0 + 0.5);
	const int hourEndY = centerY - (int)floor(cos(hourAngle) * 25.0 + 0.5);
	const int minuteEndX = centerX + (int)floor(sin(minuteAngle) * 32.0 + 0.5);
	const int minuteEndY = centerY - (int)floor(cos(minuteAngle) * 32.0 + 0.5);

	surface.drawThickLine(centerX, centerY, hourEndX, hourEndY, 4, 4, handColor);
	surface.drawThickLine(centerX, centerY, minuteEndX, minuteEndY, 3, 3, handColor);

	const Graphics::Surface *chip = getUiBitmap("16mchip.bmp");
	if (chip) {
		const int sourceX = CLIP<int>(stateDword(kStateRequiredDiscIndex), 0, 4) * 0x58;
		if (sourceX + 0x58 <= chip->w && 0x32 <= chip->h)
			surface.copyRectToSurface(*chip, 0x8c, 0x16c, Common::Rect(sourceX, 0, sourceX + 0x58, 0x32));
	}
}

void BCBEngine::drawAnalogClockAndChip(int baseHourOffset, int gameClockTicks, bool present) {
	const Graphics::Surface *clockBacking = nullptr;
	if (_currentDisplayState == 2 || _currentDisplayState == 8) {
		clockBacking = getUiBitmap("16mmap.bmp");
	} else {
		clockBacking = getUiBitmap(stateDword(kStateMainSceneNightFlag) != 0 ? "16mmainn.bmp" : "16mmain.bmp");
	}
	if (clockBacking) {
		g_system->copyRectToScreen(clockBacking->getBasePtr(0x25, 0x167), clockBacking->pitch, 0x25, 0x167, 0x60, 0x60);
	}

	Graphics::Surface *screen = g_system->lockScreen();
	if (screen) {
		renderClockAndChipToSurface(*screen, baseHourOffset, gameClockTicks);
		g_system->unlockScreen();
	}

	if (present)
		g_system->updateScreen();
}

} // End of namespace BCB
