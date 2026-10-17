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
#include "graphics/screen.h"
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
	_screen->copyRectToSurface(system->getPixels(), system->pitch, 0, 0, MIN<int>(system->w, 640), MIN<int>(system->h, 480));
	system->free();
	delete system;

	_currentDisplayState = 3;
	drawSystemSubmodeButtons(false);
	_screen->update();
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
			_screen->copyRectToSurface(system->getBasePtr(source.x, source.y), system->pitch,
									   button.x, button.y, button.width, button.height);
	}

	if (present)
		_screen->update();
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
		_screen->copyRectToSurface(system->getBasePtr(source.x, source.y), system->pitch, button.x, button.y, button.width, button.height);

	if (present)
		_screen->update();
}

void BCBEngine::applySystemWaveVolume() {
	const int mixerVolume = CLIP<int>((_systemWaveVolume * Audio::Mixer::kMaxMixerVolume + 0x7fff) / 0xffff, 0, Audio::Mixer::kMaxMixerVolume);
	const int configVolume = CLIP<int>((_systemWaveVolume * Audio::Mixer::kMaxChannelVolume + 0x7fff) / 0xffff, 0, Audio::Mixer::kMaxChannelVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, mixerVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, mixerVolume);
	ConfMan.setInt("music_volume", configVolume);
	ConfMan.setInt("sfx_volume", configVolume);
}

void BCBEngine::syncSystemWaveVolumeFromMixer() {
	const int configMusicVolume = ConfMan.hasKey("music_volume") ? ConfMan.getInt("music_volume") : Audio::Mixer::kMaxChannelVolume;
	const int configSfxVolume = ConfMan.hasKey("sfx_volume") ? ConfMan.getInt("sfx_volume") : Audio::Mixer::kMaxChannelVolume;
	const int configVolume = MAX<int>(configMusicVolume, configSfxVolume);
	if (configVolume >= Audio::Mixer::kMaxChannelVolume) {
		_systemWaveVolume = 0xffff;
		return;
	}

	_systemWaveVolume = CLIP<int>((configVolume * 0xffff + Audio::Mixer::kMaxChannelVolume / 2) / Audio::Mixer::kMaxChannelVolume, 0, 0xffff);
}

void BCBEngine::drawSystemWaveVolumeIndicator(bool present) {
	const Graphics::Surface *system = getUiBitmap("16msys.bmp");
	if (!system)
		return;

	if (kSystemWaveVolumeIndicatorRect.right <= system->w && kSystemWaveVolumeIndicatorRect.bottom <= system->h) {
		_screen->copyRectToSurface(system->getBasePtr(kSystemWaveVolumeIndicatorRect.left, kSystemWaveVolumeIndicatorRect.top),
								   system->pitch, kSystemWaveVolumeIndicatorRect.left, kSystemWaveVolumeIndicatorRect.top,
								   kSystemWaveVolumeIndicatorRect.width(), kSystemWaveVolumeIndicatorRect.height());
	}

	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen) {
		const int fillWidth = (_systemWaveVolume * kSystemWaveVolumeIndicatorRect.width()) / 0xffff;
		if (fillWidth > 0) {
			const Common::Rect fillRect(kSystemWaveVolumeIndicatorRect.left, kSystemWaveVolumeIndicatorRect.top,
										kSystemWaveVolumeIndicatorRect.left + fillWidth, kSystemWaveVolumeIndicatorRect.bottom);
			_screen->fillRect(fillRect, screen->format.RGBToColor(255, 255, 255));
		}
	}

	if (present)
		_screen->update();
}

bool BCBEngine::handleSystemWaveVolumeClick(const Common::Point &mousePosition) {
	if (kSystemWaveVolumeIndicatorRect.contains(mousePosition)) {
		const int seekX = CLIP<int>(mousePosition.x - kSystemWaveVolumeIndicatorRect.left, 0, kSystemWaveVolumeIndicatorRect.width());
		const int volume = CLIP<int>((seekX * 0xffff) / kSystemWaveVolumeIndicatorRect.width(), 0, 0xffff);
		if (volume != _systemWaveVolume) {
			_systemWaveVolume = volume;
			applySystemWaveVolume();
			drawSystemWaveVolumeIndicator();
		}
		ConfMan.flushToDisk();
		return true;
	}

	int delta = 0;
	if (kSystemWaveVolumeUpRect.contains(mousePosition))
		delta = 0x500;
	else if (kSystemWaveVolumeDownRect.contains(mousePosition))
		delta = -0x500;
	else
		return false;

	bool done = false;
	while (!done && !shouldQuit()) {
		const int volume = CLIP<int>(_systemWaveVolume + delta, 0, 0xffff);
		if (volume != _systemWaveVolume) {
			_systemWaveVolume = volume;
			applySystemWaveVolume();
			drawSystemWaveVolumeIndicator();
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT) {
				quitGame();
				done = true;
				break;
			}
			if (event.type == Common::EVENT_MOUSEMOVE) {
				_mousePosition = event.mouse;
			} else if (event.type == Common::EVENT_LBUTTONUP) {
				_mousePosition = event.mouse;
				done = true;
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END && event.customType == kBCBActionInteract) {
				done = true;
			}
		}
		serviceCursorRefresh();
		updateMenuBackgroundMusic();
		g_system->delayMillis(1);
	}
	ConfMan.flushToDisk();
	return true;
}

bool BCBEngine::readSaveSlotHeader(int slot, int &savedDay, int &savedClockTicks) {
	Common::InSaveFile *saveFile = _saveFileMan->openForLoading(getSaveStateName(slot));
	if (!saveFile)
		return false;

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

	if (!validSave) {
		delete saveFile;
		return false;
	}

	saveFile->seek(magic == kSavegameMagic ? 8 + kStateRequiredDiscIndex : kStateRequiredDiscIndex);
	savedDay = saveFile->readUint32LE();
	savedClockTicks = saveFile->readUint32LE();
	validSave = !saveFile->err();
	delete saveFile;
	return validSave;
}

bool BCBEngine::isSaveSlotOccupied(int slot) {
	int savedDay = 0;
	int savedClockTicks = 0;
	return readSaveSlotHeader(slot, savedDay, savedClockTicks);
}

void BCBEngine::drawSystemSaveLoadSlots(bool present) {
	const Graphics::Surface *system = getUiBitmap("16msys.bmp");
	if (!system)
		return;

	for (int slot = 1; slot <= kManualSaveSlotCount; ++slot) {
		const ButtonControlRecord &button = kSystemButtonTable[slot];
		const int sourceY = 480 + (slot - 1) * button.height;
		if (sourceY + button.height <= system->h)
			_screen->copyRectToSurface(system->getBasePtr(0, sourceY), system->pitch, button.x, button.y, button.width, button.height);

		int savedDay = 0;
		int savedClockTicks = 0;
		Common::U32String slotText;
		if (readSaveSlotHeader(slot, savedDay, savedClockTicks)) {
			slotText = formatSystemClockTextForDisplay(_saveSlotOccupiedTextFormatSjis, slot, savedDay, savedClockTicks);
		} else {
			slotText = formatSystemTextForDisplay(_saveSlotEmptyTextFormatSjis, slot);
		}

		Graphics::Surface *screen = _screen->surfacePtr();
		if (screen) {
			const uint32 textColor = screen->format.RGBToColor(0, 0, 0);
			const Graphics::Font *font = _ownerDrawUiTextFont ? _ownerDrawUiTextFont : FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
			const int textY = button.y + (button.height - font->getFontHeight()) / 2 - (_gameDescription->desc.language == Common::KO_KOR ? 1 : 0);
			font->drawString(screen, slotText, button.x, textY, button.width, textColor);
			_screen->addDirtyRect(Common::Rect(button.x, textY, button.x + button.width, textY + font->getFontHeight()));
		}
	}

	if (present)
		_screen->update();
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

bool BCBEngine::handleSaveLoadSlotClick(const Common::Point &mousePosition) {
	if (_systemSaveLoadSlotActionMode != 1 && _systemSaveLoadSlotActionMode != 2)
		return false;

	for (int slot = 1; slot <= kManualSaveSlotCount; ++slot) {
		const ButtonControlRecord &button = kSystemButtonTable[slot];
		const Common::Rect slotRect(button.x, button.y, button.x + button.width, button.y + button.height);
		if (!slotRect.contains(mousePosition))
			continue;

		if (_systemSaveLoadSlotActionMode == 1) {
			int savedDay = 0;
			int savedClockTicks = 0;
			Common::U32String message;
			if (readSaveSlotHeader(slot, savedDay, savedClockTicks)) {
				message = formatSystemClockTextForDisplay(_saveSlotOverwriteConfirmTextFormatSjis, slot, savedDay, savedClockTicks);
			} else {
				message = formatSystemTextForDisplay(_saveSlotEmptyConfirmTextFormatSjis, slot);
			}

			if (showSystemConfirmationDialog(message, true)) {
				saveGameState(slot, formatEnglishSaveDescription(stateDword(kStateRequiredDiscIndex), stateDword(kStateGameClockTicks)));
				drawSystemSaveLoadSlots();
			}
			return true;
		}

		if (_systemSaveLoadSlotActionMode == 2) {
			int savedDay = 0;
			int savedClockTicks = 0;
			if (!readSaveSlotHeader(slot, savedDay, savedClockTicks)) {
				showSystemConfirmationDialog(decodeExecutableTextForDisplay(_noSaveDataMessageSjis), false);
				return true;
			}
			Common::U32String message = formatSystemClockTextForDisplay(_loadSlotConfirmTextFormatSjis, slot, savedDay, savedClockTicks);
			if (showSystemConfirmationDialog(message, true)) {
				_titleLoadSlotsActive = false;
				loadGameState(slot);
			}
			return true;
		}

		return true;
	}

	return false;
}

void BCBEngine::writeContinueState() {
	saveGameState(0, formatEnglishSaveDescription(stateDword(kStateRequiredDiscIndex), stateDword(kStateGameClockTicks)), true);
}

bool BCBEngine::showSystemConfirmationDialog(const Common::U32String &message, bool showCancelButton) {
	const Graphics::Surface *chip = getUiBitmap("16mchip.bmp");
	if (!chip)
		return false;

	const Common::Rect dialogRect(144, 192, 496, 288);
	const Common::Rect okRect(showCancelButton ? 344 : 420, 254, showCancelButton ? 408 : 484, 278);
	const Common::Rect cancelRect(420, 254, 484, 278);
	const int dialogSourceY = 0x50;
	const int buttonSourceY = 0x32;
	Graphics::Surface dialogBacking;
	dialogBacking.create(dialogRect.width(), dialogRect.height(), g_system->getScreenFormat());
	Graphics::Surface *screenBacking = _screen->surfacePtr();
	if (screenBacking) {
		dialogBacking.copyRectToSurface(*screenBacking, 0, 0, dialogRect);
	}

	auto drawDialogButton = [&](const Common::Rect &rect, int sourceX, bool present) {
		if (sourceX + rect.width() <= chip->w && chip->h >= buttonSourceY + rect.height())
			_screen->copyRectToSurface(chip->getBasePtr(sourceX, buttonSourceY), chip->pitch,
									   rect.left, rect.top, rect.width(), rect.height());
		if (present)
			_screen->update();
	};

	if (dialogRect.width() <= chip->w && chip->h >= dialogSourceY + dialogRect.height())
		_screen->copyRectToSurface(chip->getBasePtr(0, dialogSourceY), chip->pitch, dialogRect.left, dialogRect.top, dialogRect.width(), dialogRect.height());

	Graphics::Surface *screen = _screen->surfacePtr();
	if (screen) {
		const Graphics::Font *font = _ownerDrawUiTextFont ? _ownerDrawUiTextFont : FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		const uint32 shadowColor = screen->format.RGBToColor(0, 0, 0);
		const uint32 textColor = screen->format.RGBToColor(255, 255, 255);
		const int textYOffset = _gameDescription->desc.language == Common::KO_KOR ? -1 : 0;
		font->drawString(screen, message, dialogRect.left + 10, dialogRect.top + 20 + textYOffset, 340, shadowColor);
		font->drawString(screen, message, dialogRect.left + 11, dialogRect.top + 21 + textYOffset, 339, textColor);
		_screen->addDirtyRect(dialogRect);
	}

	drawDialogButton(okRect, 0x17, false);
	if (showCancelButton)
		drawDialogButton(cancelRect, 0x97, false);
	_screen->update();

	bool result = false;
	bool done = false;
	auto handleDialogClick = [&]() {
		if (okRect.contains(_mousePosition)) {
			drawDialogButton(okRect, 0x57, true);
			delayWithEventPolling(80, false);
			result = true;
			done = true;
		} else if (showCancelButton && cancelRect.contains(_mousePosition)) {
			drawDialogButton(cancelRect, 0xd7, true);
			delayWithEventPolling(80, false);
			done = true;
		}
	};
	while (!done && !shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT) {
				quitGame();
				done = true;
				break;
			}
			if (event.type == Common::EVENT_MOUSEMOVE) {
				_mousePosition = event.mouse;
			} else if (event.type == Common::EVENT_LBUTTONDOWN) {
				_mousePosition = event.mouse;
				handleDialogClick();
			} else if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_RETURN) {
					result = true;
					done = true;
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					done = true;
				}
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				if (event.customType == kBCBActionEscape) {
					done = true;
				} else if (event.customType == kBCBActionInteract) {
					handleDialogClick();
				}
			}
		}
		serviceCursorRefresh();
		updateMenuBackgroundMusic();
		g_system->delayMillis(10);
	}

	_screen->copyRectToSurface(dialogBacking.getPixels(), dialogBacking.pitch, dialogRect.left, dialogRect.top,
							   dialogBacking.w, dialogBacking.h);
	dialogBacking.free();
	_screen->update();
	return result;
}

void BCBEngine::returnFromSystemMenuToPreviousState() {
	stopMenuBackgroundMusic();
	_systemSaveLoadSlotActionMode = 0;

	if (_previousDisplayStateBeforeSystem == 2) {
		processGameStateDispatcher(false);
		return;
	} else {
		showMainSceneScreen();
		if (stateDword(kStateSceneChoiceUiSetToShowId) != 0)
			showActiveSceneChoiceControls();
	}

	processGameStateDispatcher(false);
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

} // End of namespace BCB
