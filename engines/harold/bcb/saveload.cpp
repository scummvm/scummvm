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

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"

#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "harold/bcb/static_data.h"

namespace BCB {

enum {
	kSavegameMagic = MKTAG('B', 'C', 'B', 'S'),
	kSavegameVersion = 1,
	kManualSaveSlotCount = 8,
	kNativeGameStateDwordCount = 0x100,
	kNativeGameStateByteSize = kNativeGameStateDwordCount * 4
};

static const StaticRect kSystemWaveVolumeDownRect = {268, 407, 290, 439};
static const StaticRect kSystemWaveVolumeUpRect = {444, 407, 466, 439};
static const StaticRect kSystemWaveVolumeSeekRect = {294, 414, 442, 439};
static const StaticRect kSystemWaveVolumeIndicatorRect = {294, 425, 442, 449};

static const Common::Point kSystemSubmodeNormalSources[5] = {
	Common::Point(0, 0),
	Common::Point(0, 736),
	Common::Point(0, 824),
	Common::Point(240, 736),
	Common::Point(274, 824)};

static const Common::Point kSystemSubmodeSelectedSources[5] = {
	Common::Point(0, 0),
	Common::Point(0, 780),
	Common::Point(0, 868),
	Common::Point(240, 780),
	Common::Point(368, 824)};

static Common::String formatEnglishSaveDescription(int savedDay, int savedClockTicks) {
	const int discIndex = CLIP<int>(savedDay, 0, 4);
	const int dayOfMonth = 10 + discIndex;
	const int clockTicks = savedClockTicks % (24 * 60);
	const int hour24 = clockTicks / 60;
	const int minute = clockTicks % 60;
	const int hour12 = hour24 % 12 == 0 ? 12 : hour24 % 12;
	const char *period = hour24 < 12 ? "AM" : "PM";

	const Common::String chapterName = discIndex == 0 ? Common::String("Prologue") : Common::String::format("Chapter %d", discIndex);
	return Common::String::format("%s: November %d, %d:%02d %s",
								  chapterName.c_str(), dayOfMonth, hour12, minute, period);
}

static bool isValidBCBSaveSlot(int slot) {
	return slot >= 0 && slot <= kManualSaveSlotCount;
}

bool BCBEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsLoadingDuringRuntime ||
		   f == kSupportsSavingDuringRuntime ||
		   f == kSupportsReturnToLauncher;
}

bool BCBEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return true;
}

bool BCBEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return true;
}

bool BCBEngine::canSaveAutosaveCurrently() {
	return false;
}

Common::Error BCBEngine::saveGameState(int slot, const Common::String &, bool isAutosave) {
	if (!isValidBCBSaveSlot(slot))
		return Common::kWritingFailed;

	Common::OutSaveFile *saveFile = _saveFileMan->openForSaving(getSaveStateName(slot));
	if (!saveFile)
		return Common::kWritingFailed;

	saveFile->writeUint32BE(kSavegameMagic);
	saveFile->writeUint16LE(kSavegameVersion);
	saveFile->writeUint16LE(kNativeGameStateByteSize);

	for (uint i = 0; i < kNativeGameStateDwordCount; ++i)
		saveFile->writeUint32LE(_gameState[i]);

	Common::Error result = saveFile->err() ? Common::kWritingFailed : Common::kNoError;
	if (result.getCode() == Common::kNoError) {
		const Common::String saveDescription = formatEnglishSaveDescription(stateDword(kStateRequiredDiscIndex), stateDword(kStateGameClockTicks));
		getMetaEngine()->appendExtendedSave(saveFile, getTotalPlayTime(), saveDescription, isAutosave);
	}

	delete saveFile;
	return result;
}

Common::Error BCBEngine::loadGameState(int slot) {
	if (!isValidBCBSaveSlot(slot))
		return Common::kReadingFailed;

	Common::InSaveFile *saveFile = _saveFileMan->openForLoading(getSaveStateName(slot));
	if (!saveFile)
		return Common::kReadingFailed;

	bool validSave = false;
	const uint32 magic = saveFile->readUint32BE();
	if (magic == kSavegameMagic) {
		const uint16 version = saveFile->readUint16LE();
		const uint16 stateByteSize = saveFile->readUint16LE();
		validSave = version == kSavegameVersion && stateByteSize == kNativeGameStateByteSize;
	} else if (saveFile->size() == kNativeGameStateByteSize) {
		saveFile->seek(0);
		validSave = true;
	}

	if (validSave) {
		for (uint i = 0; i < kNativeGameStateDwordCount; ++i)
			_gameState[i] = saveFile->readUint32LE();
		validSave = !saveFile->err();
	}

	delete saveFile;
	if (!validSave)
		return Common::kReadingFailed;

	resetTransientRuntimeStateAfterLoad();
	processGameStateDispatcher(false);
	return Common::kNoError;
}

void BCBEngine::resetTransientRuntimeStateAfterLoad() {
	stopActiveMovie();
	stopMenuBackgroundMusic();
	stopSceneIdleMovie();
	stopMapMoveMovie();
	stopGameOverMusic();
	restoreActiveCaptionBackingRect();
	freeSceneChoiceBacking();

	_scriptStackDepth = 0;
	_scriptStopFlag = false;
	_timedCaptionRecordData.clear();
	_timedCaptionCursorOffset = 0;
	_nextTimedCaptionThreshold = 99999;
	_timedCaptionText.clear();
	_timedCaptionTextSjis.clear();
	_activeCaptionBackingRect = Common::Rect();
	_queuedMoviePlaybackIndex = 0;
	_previousDisplayStateBeforeSystem = 0;
	_computerChoiceGroupIndex = 0;
	_computerDetailVariantIndex = 0;
	_sceneChoiceTerminatorButtonIndex = 0;
	_sceneChoiceEnabledButtonLimit = 0;
	_computerControlsActive = false;
	_computerDetailControlsActive = false;
	_computerDetailNextVariantEnabled = false;
	_computerActiveChoiceIds.clear();
	_computerActiveChoiceRects.clear();
	_sceneChoiceActive = false;
	_mapControlsActive = false;
	_suppressClickHandling = false;
	_titleButtonsActive = false;
	_systemButtonsVisible = false;
	_titleLoadSlotsActive = false;
	_activeMovieDispatchOnFinish = false;
	_activeMovieOnBlankScreen = false;
	_activeMoviePaused = false;
	_pendingStillElapsedTimerTicks = 0;
	_gameOverElapsedTimerTicks = 0;
	_pendingStillDurationIndex = 0;
	_pendingClockTickDelta = 0;
	_sceneIdleMovieTimerTicks = 0;
	_systemSaveLoadSlotActionMode = 0;
	_lastTimerTick = g_system->getMillis();
	_lastLoggedGameClockTicks = -1;
	_currentDisplayState = 0;
}

void BCBEngine::enterSystemMenu() {
	_previousDisplayStateBeforeSystem = _currentDisplayState;
	_systemSaveLoadSlotActionMode = 0;
	_sceneChoiceActive = false;
	_mapControlsActive = false;

	if (_previousDisplayStateBeforeSystem == 1)
		stopSceneIdleMovie();
	else if (_previousDisplayStateBeforeSystem == 2)
		stopMapMoveMovie();

	stopActiveMovie();
	playUiSound();
	startMenuBackgroundMusic();
	showSystemScreen();
}

bool BCBEngine::showSystemScreen() {
	Graphics::Surface *system = loadBitmap("16msys.bmp");
	if (!system)
		return false;

	applyPendingClockTickDelta();
	syncSystemWaveVolumeFromMixer();
	renderClockAndChipToSurface(*system, 0, stateDword(kStateGameClockTicks));
	const int fillWidth = (_systemWaveVolume * kSystemWaveVolumeIndicatorRect.width()) / 0xffff;
	if (fillWidth > 0) {
		const Common::Rect fillRect(kSystemWaveVolumeIndicatorRect.left, kSystemWaveVolumeIndicatorRect.top,
									kSystemWaveVolumeIndicatorRect.left + fillWidth, kSystemWaveVolumeIndicatorRect.bottom);
		system->fillRect(fillRect, system->format.RGBToColor(255, 255, 255));
	}
	g_system->copyRectToScreen(system->getPixels(), system->pitch, 0, 0, MIN<int>(system->w, 640), MIN<int>(system->h, 480));
	system->free();
	delete system;

	_currentDisplayState = 3;
	drawSystemSubmodeButtons(false);
	g_system->updateScreen();
	return true;
}

void BCBEngine::drawSystemSubmodeButtons(bool present) {
	const Graphics::Surface *system = getUiBitmap("16msys.bmp");
	if (!system)
		return;

	for (uint i = 1; i < ARRAYSIZE(kSystemSubmodeButtonTable); ++i) {
		const ButtonControlRecord &button = kSystemSubmodeButtonTable[i];
		Common::Point source = kSystemSubmodeNormalSources[i];
		if ((_systemSaveLoadSlotActionMode == 1 && i == 1) ||
			(_systemSaveLoadSlotActionMode == 2 && i == 2))
			source = kSystemSubmodeSelectedSources[i];
		if (source.x + button.width <= system->w && source.y + button.height <= system->h)
			g_system->copyRectToScreen(system->getBasePtr(source.x, source.y), system->pitch,
									   button.x, button.y, button.width, button.height);
	}

	if (present)
		g_system->updateScreen();
}

void BCBEngine::drawSystemSubmodeButton(uint buttonIndex, bool selected, bool present) {
	if (buttonIndex == 0 || buttonIndex >= ARRAYSIZE(kSystemSubmodeButtonTable))
		return;

	const Graphics::Surface *system = getUiBitmap("16msys.bmp");
	if (!system)
		return;

	const ButtonControlRecord &button = kSystemSubmodeButtonTable[buttonIndex];
	const Common::Point source = selected ? kSystemSubmodeSelectedSources[buttonIndex] : kSystemSubmodeNormalSources[buttonIndex];
	if (source.x + button.width <= system->w && source.y + button.height <= system->h)
		g_system->copyRectToScreen(system->getBasePtr(source.x, source.y), system->pitch, button.x, button.y, button.width, button.height);

	if (present)
		g_system->updateScreen();
}

bool BCBEngine::handleSystemClick(const Common::Point &mousePosition) {
	if (_currentDisplayState == 0 && _titleLoadSlotsActive)
		return handleSaveLoadSlotClick(mousePosition);

	if (_currentDisplayState != 3)
		return false;

	const Common::Rect saveRect(kSystemSubmodeButtonTable[1].x, kSystemSubmodeButtonTable[1].y,
								kSystemSubmodeButtonTable[1].x + kSystemSubmodeButtonTable[1].width,
								kSystemSubmodeButtonTable[1].y + kSystemSubmodeButtonTable[1].height);
	const Common::Rect loadRect(kSystemSubmodeButtonTable[2].x, kSystemSubmodeButtonTable[2].y,
								kSystemSubmodeButtonTable[2].x + kSystemSubmodeButtonTable[2].width,
								kSystemSubmodeButtonTable[2].y + kSystemSubmodeButtonTable[2].height);
	const Common::Rect titleRect(kSystemSubmodeButtonTable[3].x, kSystemSubmodeButtonTable[3].y,
								 kSystemSubmodeButtonTable[3].x + kSystemSubmodeButtonTable[3].width,
								 kSystemSubmodeButtonTable[3].y + kSystemSubmodeButtonTable[3].height);
	const Common::Rect exitRect(kSystemSubmodeButtonTable[4].x, kSystemSubmodeButtonTable[4].y,
								kSystemSubmodeButtonTable[4].x + kSystemSubmodeButtonTable[4].width,
								kSystemSubmodeButtonTable[4].y + kSystemSubmodeButtonTable[4].height);

	if (handleSystemWaveVolumeClick(mousePosition))
		return true;

	if (saveRect.contains(mousePosition)) {
		_systemSaveLoadSlotActionMode = 1;
		drawSystemSubmodeButtons(false);
		drawSystemSaveLoadSlots();
		return true;
	}

	if (loadRect.contains(mousePosition)) {
		_systemSaveLoadSlotActionMode = 2;
		drawSystemSubmodeButtons(false);
		drawSystemSaveLoadSlots();
		return true;
	}

	if (titleRect.contains(mousePosition)) {
		drawSystemSubmodeButton(3, true);
		delayWithEventPolling(80, false);
		playUiSound();
		_systemSaveLoadSlotActionMode = 0;
		showSystemScreen();
		if (showSystemConfirmationDialog(decodeExecutableTextForDisplay(_returnToTitleConfirmTextSjis), true)) {
			stopMenuBackgroundMusic();
			resetGameState();
			showTitleScreen();
			startMenuBackgroundMusic();
		}
		return true;
	}

	if (exitRect.contains(mousePosition)) {
		activateSystemExitButton();
		return true;
	}

	if (handleSaveLoadSlotClick(mousePosition))
		return true;

	return false;
}

void BCBEngine::activateSystemExitButton() {
	drawSystemSubmodeButton(4, true);
	delayWithEventPolling(80, false);
	playUiSound();
	returnFromSystemMenuToPreviousState();
}

void BCBEngine::writeContinueState() {
	saveGameState(0, formatEnglishSaveDescription(stateDword(kStateRequiredDiscIndex), stateDword(kStateGameClockTicks)), true);
}

Common::U32String BCBEngine::formatSystemTextForDisplay(const Common::String &originalFormat, int slot) const {
	const Common::String formatted = Common::String::format(originalFormat.c_str(), slot);
	return decodeExecutableTextForDisplay(formatted);
}

Common::U32String BCBEngine::formatSystemClockTextForDisplay(const Common::String &originalFormat, int slot, int savedDay, int savedClockTicks) const {
	const int hour = savedClockTicks / 60;
	const int minute = savedClockTicks % 60;
	const Common::String formatted = Common::String::format(originalFormat.c_str(), slot, savedDay, hour, minute);
	return decodeExecutableTextForDisplay(formatted);
}

void BCBEngine::drawSystemSaveLoadSlots(bool present) {
	error("BCB: drawSystemSaveLoadSlots is not implemented");
}

bool BCBEngine::handleSaveLoadSlotClick(const Common::Point &mousePosition) {
	error("BCB: handleSaveLoadSlotClick is not implemented");
}

bool BCBEngine::handleSystemWaveVolumeClick(const Common::Point &mousePosition) {
	error("BCB: handleSystemWaveVolumeClick is not implemented");
}

void BCBEngine::returnFromSystemMenuToPreviousState() {
	error("BCB: returnFromSystemMenuToPreviousState is not implemented");
}

bool BCBEngine::showSystemConfirmationDialog(const Common::U32String &message, bool showCancelButton) {
	error("BCB: showSystemConfirmationDialog is not implemented");
}

void BCBEngine::syncSystemWaveVolumeFromMixer() {
	error("BCB: syncSystemWaveVolumeFromMixer is not implemented");
}

} // End of namespace BCB
