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
#include "graphics/screen.h"
#include "graphics/surface.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"
#include "video/avi_decoder.h"

namespace BCB {

bool BCBEngine::showMainSceneScreen() {
	stopMapMoveMovie();
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
		_screen->copyRectToSurface(mainScene->getPixels(), mainScene->pitch, 0, 0, MIN<int>(mainScene->w, 640), MIN<int>(mainScene->h, 480));
	}
	mainScene->free();
	delete mainScene;
	_currentDisplayState = 1;
	_sceneIdleMovieTimerTicks = 0;
	_lastTimerTick = g_system->getMillis();
	_screen->update();
	return true;
}

bool BCBEngine::showActiveSceneChoiceControls() {
	const int sceneChoiceUiSetToShowId = stateDword(kStateSceneChoiceUiSetToShowId);
	if (_currentDisplayState != 1 && _currentDisplayState != 7)
		showMainSceneScreen();
	_currentDisplayState = 1;

	if (sceneChoiceUiSetToShowId == 0)
		return false;

	if (sceneChoiceUiSetToShowId < 0 || sceneChoiceUiSetToShowId >= (int)_sceneChoiceText.size() || _sceneChoiceText[sceneChoiceUiSetToShowId].empty()) {
		warning("BCBEngine::showActiveSceneChoiceControls(): Scene choice set %d has no parsed text", sceneChoiceUiSetToShowId);
		return false;
	}
	uint32 &sceneChoiceExtraButtonFlag = stateDword(kStateSceneChoiceExtraButtonFlag);
	const uint locationTableOffset = sceneChoiceUiSetToShowId * 2 - 1;
	sceneChoiceExtraButtonFlag = locationTableOffset < _locationTableData.size() ? _locationTableData[locationTableOffset] : 0;

	_sceneChoiceTerminatorButtonIndex = MIN<int>(_sceneChoiceText[sceneChoiceUiSetToShowId].size() + 1, 9);
	_sceneChoiceEnabledButtonLimit = sceneChoiceExtraButtonFlag == 0 ? _sceneChoiceTerminatorButtonIndex - 1 : _sceneChoiceTerminatorButtonIndex;

	const Common::String overlayPath = Common::String::format("bmp/select/16m/sel%03d.bmp", sceneChoiceUiSetToShowId);
	Graphics::Surface *overlay = loadDiscBitmap(overlayPath.c_str());
	if (overlay) {
		_screen->copyRectToSurface(overlay->getPixels(), overlay->pitch, 16, 48, MIN<int>(overlay->w, 400), MIN<int>(overlay->h, 200));
		overlay->free();
		delete overlay;
	}

	drawSceneChoiceControlStrip(0, false);

	captureSceneChoiceBacking();
	drawSceneChoiceButtons(sceneChoiceUiSetToShowId);
	_sceneChoiceActive = true;
	_sceneIdleMovieTimerTicks = 0;
	writeContinueState();
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

	Graphics::Surface *screen = _screen->surfacePtr();
	if (!screen)
		return;

	_sceneChoiceBacking = new Graphics::Surface();
	_sceneChoiceBacking->create(_sceneChoiceBackingRect.width(), _sceneChoiceBackingRect.height(), screen->format);
	_sceneChoiceBacking->copyRectToSurface(*screen, 0, 0, _sceneChoiceBackingRect);
}

void BCBEngine::restoreSceneChoiceBacking(bool present) {
	if (!_sceneChoiceBacking)
		return;

	for (int buttonIndex = 1; buttonIndex <= _sceneChoiceEnabledButtonLimit && buttonIndex < (int)ARRAYSIZE(kSceneChoiceButtonTable); ++buttonIndex) {
		if (!isSceneChoiceButtonVisible(buttonIndex))
			continue;

		const ButtonControlRecord &button = kSceneChoiceButtonTable[buttonIndex];
		_screen->copyRectToSurface(_sceneChoiceBacking->getBasePtr(button.x - _sceneChoiceBackingRect.left, button.y - _sceneChoiceBackingRect.top), _sceneChoiceBacking->pitch, button.x, button.y, button.width, button.height);
	}

	if (present)
		_screen->update();
}

void BCBEngine::freeSceneChoiceBacking() {
	if (!_sceneChoiceBacking)
		return;

	_sceneChoiceBacking->free();
	delete _sceneChoiceBacking;
	_sceneChoiceBacking = nullptr;
}

void BCBEngine::clearSceneChoiceAfterSelection() {
	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen) {
		_screen->fillRect(Common::Rect(0x10, 0x30, 0x1a0, 0xf8), screen->format.RGBToColor(0, 0, 0));
	}

	freeSceneChoiceBacking();
	_screen->update();
}

bool BCBEngine::isSceneChoiceButtonVisible(int buttonIndex) const {
	if (buttonIndex <= 0 || buttonIndex >= (int)ARRAYSIZE(kSceneChoiceButtonTable))
		return false;

	const int sceneChoiceUiSetToShowId = stateDword(kStateSceneChoiceUiSetToShowId);
	const int choiceCount = (sceneChoiceUiSetToShowId >= 0 && sceneChoiceUiSetToShowId < (int)_sceneChoiceText.size()) ? MIN<int>(_sceneChoiceText[sceneChoiceUiSetToShowId].size(), 8) : 0;

	return buttonIndex <= choiceCount ||
		   (buttonIndex == _sceneChoiceTerminatorButtonIndex && stateDword(kStateSceneChoiceExtraButtonFlag) != 0);
}

void BCBEngine::drawSceneChoiceButtons(int sceneChoiceUiSetToShowId, bool present, int pressedButtonIndex) {
	const Graphics::Font *font = _ownerDrawUiTextFont;
	if (!font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);

	if (pressedButtonIndex < 0) {
		restoreSceneChoiceBacking(false);
	} else if (_sceneChoiceBacking && pressedButtonIndex < (int)ARRAYSIZE(kSceneChoiceButtonTable)) {
		const ButtonControlRecord &button = kSceneChoiceButtonTable[pressedButtonIndex];
		_screen->copyRectToSurface(_sceneChoiceBacking->getBasePtr(button.x - _sceneChoiceBackingRect.left, button.y - _sceneChoiceBackingRect.top), _sceneChoiceBacking->pitch, button.x, button.y, button.width, button.height);
	}

	Graphics::Surface *screen = _screen->surfacePtr();
	if (!screen)
		return;

	const uint32 normalShadowColor = screen->format.RGBToColor(0xf6, 0xd3, 0x68);
	const uint32 normalTextColor = screen->format.RGBToColor(0x73, 0x4b, 0x20);
	const uint32 pressedShadowColor = screen->format.RGBToColor(0xff, 0xd3, 0x83);
	const uint32 pressedTextColor = screen->format.RGBToColor(0x8c, 0x3c, 0x05);
	const Common::Array<Common::U32String> &originalLabels = _sceneChoiceText[sceneChoiceUiSetToShowId];
	const auto getChoiceLabel = [&](uint choiceIndex) -> Common::U32String {
		return originalLabels[choiceIndex];
	};
	const auto drawChoiceLabel = [&](const Common::U32String &label, int buttonIndex) {
		if (!font)
			return;
		const ButtonControlRecord &button = kSceneChoiceButtonTable[buttonIndex];
		const bool pressed = buttonIndex == pressedButtonIndex;
		const uint32 shadowColor = pressed ? pressedShadowColor : normalShadowColor;
		const uint32 textColor = pressed ? pressedTextColor : normalTextColor;
		const int textX = button.x + 10;
		const int textY = button.y + (button.height - font->getFontHeight()) / 2 - (_gameDescription->desc.language == Common::KO_KOR ? 1 : 0);
		const int textWidth = button.width - 10;
		font->drawString(screen, label, textX + 1, textY + 1, textWidth, shadowColor, Graphics::kTextAlignLeft);
		font->drawString(screen, label, textX, textY, textWidth, textColor, Graphics::kTextAlignLeft);
		_screen->addDirtyRect(Common::Rect(textX, textY, textX + textWidth + 1, textY + font->getFontHeight() + 1));
	};

	const Graphics::Surface *mainScene = getUiBitmap(stateDword(kStateMainSceneNightFlag) != 0 ? "16mmainn.bmp" : "16mmain.bmp");
	if (mainScene) {
		const int firstButtonIndex = pressedButtonIndex < 0 ? 1 : pressedButtonIndex;
		const int lastButtonIndex = pressedButtonIndex < 0 ? _sceneChoiceEnabledButtonLimit : pressedButtonIndex;
		for (int buttonIndex = firstButtonIndex; buttonIndex <= lastButtonIndex && buttonIndex < (int)ARRAYSIZE(kSceneChoiceButtonTable); ++buttonIndex) {
			if (!isSceneChoiceButtonVisible(buttonIndex))
				continue;

			const ButtonControlRecord &button = kSceneChoiceButtonTable[buttonIndex];
			if (button.width <= mainScene->w && 0x210 + button.height <= mainScene->h)
				_screen->copyRectToSurface(mainScene->getBasePtr(0, 0x210), mainScene->pitch, button.x, button.y, button.width, button.height);
		}
	}

	screen = _screen->surfacePtr();
	if (!screen)
		return;

	if (pressedButtonIndex < 0) {
		for (uint i = 0; i < originalLabels.size() && (int)i < _sceneChoiceEnabledButtonLimit && i + 1 < ARRAYSIZE(kSceneChoiceButtonTable); ++i)
			drawChoiceLabel(getChoiceLabel(i), i + 1);
	} else if (pressedButtonIndex - 1 >= 0 && pressedButtonIndex - 1 < (int)originalLabels.size()) {
		drawChoiceLabel(getChoiceLabel(pressedButtonIndex - 1), pressedButtonIndex);
	}

	if ((pressedButtonIndex < 0 || pressedButtonIndex == _sceneChoiceTerminatorButtonIndex) &&
		stateDword(kStateSceneChoiceExtraButtonFlag) != 0 &&
		_sceneChoiceTerminatorButtonIndex > 0 &&
		_sceneChoiceTerminatorButtonIndex < (int)ARRAYSIZE(kSceneChoiceButtonTable)) {
		drawChoiceLabel(_sceneChoiceMoveText, _sceneChoiceTerminatorButtonIndex);
	}

	if (present)
		_screen->update();
}

void BCBEngine::handleSceneChoiceClick(const Common::Point &mousePosition) {
	if (!_sceneChoiceActive)
		return;

	for (int buttonIndex = 1; buttonIndex <= _sceneChoiceEnabledButtonLimit && buttonIndex < (int)ARRAYSIZE(kSceneChoiceButtonTable); ++buttonIndex) {
		if (!isSceneChoiceButtonVisible(buttonIndex))
			continue;

		const ButtonControlRecord &button = kSceneChoiceButtonTable[buttonIndex];
		const Common::Rect rect(button.x, button.y, button.x + button.width, button.y + button.height);
		if (!rect.contains(mousePosition))
			continue;

		handleSceneChoiceSelection(buttonIndex);
		return;
	}
}

void BCBEngine::handleSceneChoiceSelection(int buttonIndex) {
	if (!_sceneChoiceActive || buttonIndex <= 0 || buttonIndex > _sceneChoiceEnabledButtonLimit ||
		buttonIndex >= (int)ARRAYSIZE(kSceneChoiceButtonTable) || !isSceneChoiceButtonVisible(buttonIndex))
		return;

	const int sceneChoiceUiSetToShowId = stateDword(kStateSceneChoiceUiSetToShowId);
	drawSceneChoiceButtons(sceneChoiceUiSetToShowId, true, buttonIndex);
	delayWithEventPolling(80);

	stopSceneIdleMovie();
	_sceneChoiceActive = false;
	stateDword(kStateMovieHistoryCount) = 0;

	if (buttonIndex == _sceneChoiceTerminatorButtonIndex && stateDword(kStateSceneChoiceExtraButtonFlag) != 0) {
		stateDword(kStateSelectedSceneChoiceValue) = 0xff;
	} else {
		stateDword(kStateSelectedSceneChoiceValue) = kSceneChoiceValueByButtonIndex[buttonIndex];
		const uint timingOffset = (sceneChoiceUiSetToShowId - 1) * 8 + (buttonIndex - 1);
		if (timingOffset < _locationTimingData.size())
			_pendingClockTickDelta = (int8)_locationTimingData[timingOffset];
	}

	clearSceneChoiceAfterSelection();
	processGameStateDispatcher(true);
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
		_screen->copyRectToSurface(clockBacking->getBasePtr(0x25, 0x167), clockBacking->pitch, 0x25, 0x167, 0x60, 0x60);
	}

	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen) {
		renderClockAndChipToSurface(*screen, baseHourOffset, gameClockTicks);
		_screen->addDirtyRect(Common::Rect(0x25, 0x167, 0xe4, 0x1c7));
	}

	if (present)
		_screen->update();
}

void BCBEngine::handleSceneChoiceKeyboardSelection(int choiceNumber) {
	if (!_sceneChoiceActive || choiceNumber <= 0 || choiceNumber > 9)
		return;

	if (choiceNumber == 9 && stateDword(kStateSceneChoiceExtraButtonFlag) != 0) {
		handleSceneChoiceSelection(_sceneChoiceTerminatorButtonIndex);
		return;
	}

	handleSceneChoiceSelection(choiceNumber);
}

} // End of namespace BCB
