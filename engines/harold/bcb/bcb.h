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

#ifndef BCB_H
#define BCB_H

#include "audio/mixer.h"
#include "common/archive.h"
#include "common/array.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/rect.h"
#include "common/str-enc.h"
#include "common/ustr.h"
#include "engines/engine.h"
#include "harold/bcb/ui.h"
#include "harold/detection.h"

namespace Graphics {
class Font;
class FontSJIS;
class Screen;
struct Surface;
} // namespace Graphics

namespace Video {
class AVIDecoder;
}

namespace BCB {

enum {
	kBCBDebugGeneral = 1
};

enum BCBAction {
	kBCBActionInteract,
	kBCBActionEscape,
	kBCBActionSkip,
	kBCBActionChoice1,
	kBCBActionChoice2,
	kBCBActionChoice3,
	kBCBActionChoice4,
	kBCBActionChoice5,
	kBCBActionChoice6,
	kBCBActionChoice7,
	kBCBActionChoice8,
	kBCBActionChoice9
};

class Debugger;

class BCBEngine : public Engine {
	friend class Debugger;

public:
	BCBEngine(OSystem *syst, const HaroldGameDescription *desc);
	~BCBEngine();

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	Common::Error loadGameState(int slot) override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveAutosaveCurrently() override;

private:
	void loadFonts();
	bool initializeSearchPaths();
	bool showInitialLogoScreen();
	bool showTitleScreen();
	bool transitionFromLogoToTitle();
	bool startNewGameFromTitle();
	bool playNumberedMovie(int movieId, bool blankScreenMovie = false);
	Common::SeekableReadStream *createMovieReadStream(int movieId) const;
	bool switchActiveMediaDisc(int discNumber);
	void stopActiveMovie();
	void playUiSound();
	void startMenuBackgroundMusic();
	void updateMenuBackgroundMusic();
	void stopMenuBackgroundMusic();
	void startGameOverMusic();
	void stopGameOverMusic();
	void startSceneIdleMovie();
	void stopSceneIdleMovie();
	void enterSystemMenu();
	bool showSystemScreen();
	void restoreTitleScreenRegion(const Common::Rect &rect, bool present = true);
	void drawSystemSaveLoadSlots(bool present = true);
	void drawSystemSubmodeButtons(bool present = true);
	void drawSystemSubmodeButton(uint buttonIndex, bool selected, bool present = true);
	void drawSystemWaveVolumeIndicator(bool present = true);
	void syncSystemWaveVolumeFromMixer();
	void applySystemWaveVolume();
	bool handleSystemWaveVolumeClick(const Common::Point &mousePosition);
	bool handleSystemClick(const Common::Point &mousePosition);
	void activateSystemExitButton();
	bool readSaveSlotHeader(int slot, int &savedDay, int &savedClockTicks);
	bool isSaveSlotOccupied(int slot);
	void writeContinueState();
	bool showSystemConfirmationDialog(const Common::U32String &message, bool showCancelButton);
	void returnFromSystemMenuToPreviousState();
	bool handleSaveLoadSlotClick(const Common::Point &mousePosition);
	bool openMapMoveMovie();
	void startMapMoveMovie();
	void stopMapMoveMovie();
	void resetGameState();
	void resetTransientRuntimeStateAfterLoad();
	bool loadScriptData();
	bool loadExecutableDataTables();
	bool loadByteFile(const char *fileName, Common::Array<byte> &data) const;
	void processGameStateDispatcher(bool runScriptBeforeDispatch);
	bool dispatchGameStateControlCode(uint32 dispatcherControlCode);
	bool dispatchGameStateNoOp();
	bool dispatchSceneChoiceControls();
	bool dispatchMapMoveControls();
	bool dispatchComputerMainControls();
	bool dispatchComputerDetailControls();
	bool dispatchGameOverScreen();
	bool dispatchEndingStillDisplayState();
	const Graphics::Surface *getComputerBitmap();
	bool showComputerScreen();
	void drawComputerChoiceGroupControls();
	void drawComputerConditionalChoiceControls(const ButtonControlRecord *choiceButtonRecords, int firstStateIndex, int stateCount);
	void drawComputerButton(const ButtonControlRecord &sourceButton, const ButtonControlRecord &destButton);
	void drawComputerButtonPressed(const ButtonControlRecord &button, int sourceX, int sourceY);
	void drawComputerDetailButtonNormal(const ButtonControlRecord &button);
	void drawComputerUnavailableMarker(const ButtonControlRecord &destButton);
	void drawXorRectangleBorder(const Common::Rect &rect);
	void processXorRectangleBorderAnimation(bool expand, const Common::Rect &sourceRect, const Common::Rect &targetRect);
	bool showComputerDetailScreen(int compositeIndex);
	void checkCurrentMediaPathReady();
	void clearDisplayAndResetState();
	void executeCurrentScriptSequence();
	uint32 executeScriptExpression(uint32 &scriptPc, int &branchTokenSeen);
	bool pushScriptValue(uint32 value);
	uint32 popScriptValue();
	void popScriptOperands(uint32 &leftValue, uint32 &rightValue);
	uint32 dereferenceScriptValue(uint32 value);
	uint32 &scriptOperandDword(uint32 encodedOperand);
	uint32 &stateDword(uint byteOffset);
	uint32 stateDword(uint byteOffset) const;
	bool processPendingMovieRequest();
	void loadTimedCaptionRecord(int movieId);
	void loadNextTimedCaptionText();
	bool readTimedCaptionLine(Common::String &line);
	const byte *findAllBinRecordData(const Common::String &recordName, uint32 &recordSize) const;
	bool drawTimedVideoTextOverlay();
	void restoreActiveCaptionBackingRect();
	Common::Rect getCaptionBackingRect(const Common::String &sjisText, const Common::U32String &unicodeText, const Common::Rect &nativeRect) const;
	void drawCaptionTextBlock(const Common::String &sjisText, const Common::U32String &unicodeText, const Common::Rect &rect, uint32 color);
	void restoreCaptionBackingRect(const Common::Rect &rect);
	Common::Point getActiveMoviePosition() const;
	bool shouldDrawMovieControlStrip() const;
	void drawMapSubmodeMovieBackdrop(bool present = true);
	void drawSceneChoiceControlStrip(int sourceX = 0, bool present = true);
	void drawMovieControlStrip(int sourceX = 0, bool present = true);
	void stopActiveMovieAndDispatch();
	void processQueuedMovieSequence();
	bool isMapButtonEnabled(uint buttonIndex) const;
	void drawMapButtonPressed(uint buttonIndex, bool present = true);
	void composeMapControlsToSurface(Graphics::Surface &surface, const Graphics::Surface &mapArt);
	void drawMapControls(bool present = true);
	bool handleMapControlClick(const Common::Point &mousePosition);
	bool showPendingStillImageScreen();
	void completePendingStillImageScreen();
	bool showMainSceneScreen();
	bool drawMainSceneScreen(bool lineReveal);
	bool showMapScreen();
	bool showActiveSceneChoiceControls();
	void captureSceneChoiceBacking();
	void restoreSceneChoiceBacking(bool present = true);
	void freeSceneChoiceBacking();
	void clearSceneChoiceAfterSelection();
	bool isSceneChoiceButtonVisible(int buttonIndex) const;
	void drawSceneChoiceButtons(int sceneChoiceUiSetToShowId, bool present = true, int pressedButtonIndex = -1);
	void handleSceneChoiceClick(const Common::Point &mousePosition);
	void handleSceneChoiceSelection(int buttonIndex);
	bool handleComputerClick(const Common::Point &mousePosition);
	bool handleComputerKey(Common::KeyCode keycode);
	void applyPendingClockTickDelta();
	void notifyGameClockTicksChanged();
	void drawClockAndChipIndicator(bool present = true);
	void drawAnalogClockAndChip(int baseHourOffset, int gameClockTicks, bool present = true);
	void renderClockAndChipToSurface(Graphics::Surface &surface, int baseHourOffset, int gameClockTicks);
	Graphics::Surface *loadBitmap(const char *fileName);
	const Graphics::Surface *getUiBitmap(const char *fileName);
	Graphics::Surface *loadBitmapFromStream(Common::SeekableReadStream *stream, const char *debugName) const;
	Graphics::Surface *loadDiscBitmap(const char *relativePath) const;
	Graphics::Surface *loadDiscBitmapFromAnyDisc(const char *relativePath) const;
	Common::SeekableReadStream *createProgramReadStream(const char *relativePath) const;
	Common::SeekableReadStream *createDiscReadStream(int discNumber, const char *relativePath) const;
	Common::SeekableReadStream *createActiveDiscReadStream(const char *relativePath) const;
	Common::SeekableReadStream *createActiveDiscHighColorReadStream(const char *relativePath) const;
	void blitScreenWithLineReveal(const Graphics::Surface &surface, bool presentFinalFrame = true);
	void drawTitleButton(uint buttonIndex, bool pressed);
	void handleLeftButtonDown(const Common::Point &mousePosition);
	void handleKeymapperAction(Common::CustomEventType action);
	bool handleEscapeAction();
	void handleSkipKey();
	void handleSceneChoiceKeyboardSelection(int choiceNumber);
	bool parseCheatNumber(const Common::String &text, uint32 &value) const;
	void addCheatTime(int minutes);
	void jumpToCheatSequence(uint sequenceId);
	bool pollEvents();
	bool pollEvents(bool handleClicks);
	void serviceCursorRefresh(bool screenUpdated = false);
	void delayWithEventPolling(uint32 millis);
	void delayWithEventPolling(uint32 millis, bool handleClicks);
	Common::U32String decodeExecutableTextForDisplay(const Common::String &text) const;
	Common::U32String formatSystemTextForDisplay(const Common::String &originalFormat, int slot) const;
	Common::U32String formatSystemClockTextForDisplay(const Common::String &originalFormat, int slot, int savedDay, int savedClockTicks) const;
	void mainLoop();

	const HaroldGameDescription *_gameDescription;
	Graphics::Screen *_screen = nullptr;
	Common::SearchSet _programFiles;
	Common::SearchSet _discFiles[5];
	Common::Array<byte> _scriptData;
	Common::Array<byte> _locationTableData;
	Common::Array<byte> _locationTimingData;
	Common::Array<byte> _allBinData;
	Common::Array<byte> _timedCaptionRecordData;
	Common::Array<Common::Array<Common::U32String>> _sceneChoiceText;
	Common::CodePage _executableTextEncoding = Common::kWindows932;
	Common::String _sceneChoiceMoveTextSjis;
	Common::U32String _sceneChoiceMoveText;
	Common::Array<Common::String> _mapButtonTextSjis;
	Common::Array<Common::U32String> _mapButtonText;
	Common::Array<Common::U32String> _gameOverText;
	Common::U32String _titleExitConfirmText;
	Common::String _saveSlotOccupiedTextFormatSjis;
	Common::String _saveSlotEmptyTextFormatSjis;
	Common::String _loadSlotConfirmTextFormatSjis;
	Common::String _saveSlotOverwriteConfirmTextFormatSjis;
	Common::String _saveSlotEmptyConfirmTextFormatSjis;
	Common::String _noSaveDataMessageSjis;
	Common::String _returnToTitleConfirmTextSjis;
	Graphics::FontSJIS *_sjisFont = nullptr;
	const Graphics::Font *_captionTextFont = nullptr;
	const Graphics::Font *_gameOverTextFont = nullptr;
	const Graphics::Font *_ownerDrawUiTextFont = nullptr;
	Video::AVIDecoder *_menuBackgroundVideo = nullptr;
	Video::AVIDecoder *_gameOverVideo = nullptr;
	Video::AVIDecoder *_sceneIdleVideo = nullptr;
	Video::AVIDecoder *_mapMoveVideo = nullptr;
	Video::AVIDecoder *_activeMovieVideo = nullptr;
	Graphics::Surface *_titleBitmap = nullptr;
	Graphics::Surface *_uiBitmap = nullptr;
	Common::String _uiBitmapName;
	Graphics::Surface *_chipBitmap = nullptr;
	Graphics::Surface *_sceneChoiceBacking = nullptr;
	Common::Rect _sceneChoiceBackingRect;
	Graphics::Surface *_computerBitmap = nullptr;
	Graphics::Surface *_computerDetailBacking = nullptr;
	Audio::SoundHandle _uiSoundHandle;
	uint32 _gameState[0x100] = {};
	uint32 _scriptValueStack[0x100] = {};
	int _scriptStackDepth = 0;
	bool _scriptStopFlag = false;
	int _currentDisplayState = 0;
	int _initialLogoElapsedTimerTicks = 0;
	int _pendingStillElapsedTimerTicks = 0;
	int _gameOverElapsedTimerTicks = 0;
	int _pendingStillDurationIndex = 0;
	int _pendingClockTickDelta = 0;
	int _sceneIdleMovieTimerTicks = 0;
	int _activeMediaDiscNumber = 1;
	uint _timedCaptionCursorOffset = 0;
	int _nextTimedCaptionThreshold = 99999;
	int _queuedMoviePlaybackIndex = 0;
	int _previousDisplayStateBeforeSystem = 0;
	int _computerChoiceGroupIndex = 0;
	int _computerDetailVariantIndex = 0;
	int _sceneChoiceTerminatorButtonIndex = 0;
	int _sceneChoiceEnabledButtonLimit = 0;
	bool _computerControlsActive = false;
	bool _computerDetailControlsActive = false;
	bool _computerDetailNextVariantEnabled = false;
	Common::Array<int> _computerActiveChoiceIds;
	Common::Array<Common::Rect> _computerActiveChoiceRects;
	Common::Rect _computerSelectionXorRect;
	Common::U32String _timedCaptionText;
	Common::String _timedCaptionTextSjis;
	Common::Rect _activeCaptionBackingRect;
	uint32 _lastTimerTick = 0;
	int _lastLoggedGameClockTicks = -1;
	uint32 _lastCursorRefreshMillis = 0;
	Common::Point _mousePosition;
	bool _activeMovieDispatchOnFinish = false;
	bool _activeMovieOnBlankScreen = false;
	bool _activeMoviePaused = false;
	bool _sceneChoiceActive = false;
	bool _mapControlsActive = false;
	bool _suppressClickHandling = false;
	bool _titleButtonsActive = false;
	bool _systemButtonsVisible = false;
	int _systemSaveLoadSlotActionMode = 0;
	int _systemWaveVolume = 0xffff;
	bool _titleLoadSlotsActive = false;
};

} // End of namespace BCB

#endif // BCB_H
