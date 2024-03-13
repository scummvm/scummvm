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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/archive.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/keyboard.h"
#include "common/savefile.h"
#include "common/textconsole.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"
#include "gui/debugger.h"

#include "tucker/tucker.h"
#include "tucker/graphics.h"

namespace Tucker {

TuckerEngine::TuckerEngine(OSystem *system, Common::Language language, uint32 flags)
	: Engine(system), _gameLang(language), _gameFlags(flags), _rnd("tucker") {
	setDebugger(new TuckerConsole(this));

	resetVariables();

	_execData3Counter = 0;
	_currentSaveLoadGameState = 1;
	_fileLoadSize = 0;
	_csDataSize = 0;
	_startSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	_player = nullptr;
	_loadTempBuf = nullptr;
	_cursorGfxBuf = nullptr;
	_charsetGfxBuf = nullptr;
	_panelGfxBuf = nullptr;
	_itemsGfxBuf = nullptr;
	_spritesGfxBuf = nullptr;
	_locationBackgroundGfxBuf = nullptr;
	_data5Buf = nullptr;
	_data3GfxBuf = nullptr;
	_quadBackgroundGfxBuf = nullptr;
	_objTxtBuf = nullptr;
	_panelObjectsGfxBuf = nullptr;
	_ptTextBuf = nullptr;
	_infoBarBuf = nullptr;
	_bgTextBuf = nullptr;
	_charNameBuf = nullptr;
	_locationBackgroundMaskBuf = nullptr;
	_csDataBuf = nullptr;
}

TuckerEngine::~TuckerEngine() {
}

bool TuckerEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsReturnToLauncher:
	case kSupportsLoadingDuringRuntime:
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	}
}

Common::Error TuckerEngine::run() {
	initGraphics(kScreenWidth, kScreenHeight);
	syncSoundSettings();
	_compressedSound.openFile();
	if (_startSlot == -1)
		handleIntroSequence();
	if ((_gameFlags & kGameFlagIntroOnly) == 0 && !shouldQuit()) {
		mainLoop();
	}
	_compressedSound.closeFile();
	return Common::kNoError;
}

int TuckerEngine::getRandomNumber() {
	return _rnd.getRandomNumber(0x7FFF);
}

void TuckerEngine::allocateBuffers() {
	_locationBackgroundGfxBuf = (uint8 *)calloc(1, 640 * 200);
	_loadTempBuf = (uint8 *)calloc(1, 64010);
	_panelGfxBuf = (uint8 *)calloc(1, 64010);
	_itemsGfxBuf = (uint8 *)calloc(1, 19200);
	_charsetGfxBuf = (uint8 *)calloc(1, 22400);
	_cursorGfxBuf = (uint8 *)calloc(1, 256 * 7);
	_infoBarBuf = (uint8 *)calloc(1, 1000);
	_charNameBuf = nullptr;
	_bgTextBuf = nullptr;
	_objTxtBuf = nullptr;
	_panelObjectsGfxBuf = (uint8 *)calloc(1, 20000);
	_data5Buf = nullptr;
	_data3GfxBuf = (uint8 *)calloc(1, 250000);
	_quadBackgroundGfxBuf = (uint8 *)calloc(1, 320 * 140 * 4);
	_locationBackgroundMaskBuf = (uint8 *)calloc(1, 640 * 140);
	_csDataBuf = nullptr;
	_spritesGfxBuf = (uint8 *)calloc(1, 160000);
	_ptTextBuf = nullptr;
	memset(_charWidthTable, 0, sizeof(_charWidthTable));
}

void TuckerEngine::freeBuffers() {
	free(_locationBackgroundGfxBuf);
	free(_loadTempBuf);
	free(_panelGfxBuf);
	free(_itemsGfxBuf);
	free(_charsetGfxBuf);
	free(_cursorGfxBuf);
	free(_infoBarBuf);
	free(_charNameBuf);
	free(_bgTextBuf);
	free(_objTxtBuf);
	free(_panelObjectsGfxBuf);
	free(_data5Buf);
	free(_data3GfxBuf);
	free(_quadBackgroundGfxBuf);
	free(_locationBackgroundMaskBuf);
	free(_csDataBuf);
	free(_spritesGfxBuf);
	free(_ptTextBuf);
}

void TuckerEngine::resetVariables() {
	_quitGame = false;
	_fastMode = false;
	_syncCounter = 0;
	_lastFrameTime = _system->getMillis();
	_mainLoopCounter1 = _mainLoopCounter2 = 0;
	_timerCounter2 = 0;
	_part = _currentPart = kPartInit;
	_location = kLocationNone;
	_nextLocation = (_gameFlags & kGameFlagDemo) ? kLocationInitDemo : kLocationInit;
	_gamePaused = false;
	_gameDebug = false;
	_displaySpeechText = (_gameFlags & kGameFlagNoSubtitles) ? false : ConfMan.getBool("subtitles");
	memset(_flagsTable, 0, sizeof(_flagsTable));

	_gameHintsIndex = 0;
	_gameHintsCounter = 0;
	_gameHintsStringNum = 0;

	_displayGameHints = false;
	_displayHintsText = false;

	if ((_gameFlags & kGameFlagDemo) == 0) {
		_locationWidthTable = _locationWidthTableGame;
		_locationHeightTable = _locationHeightTableGame;
	} else {
		_locationWidthTable = _locationWidthTableDemo;
		_locationHeightTable = _locationHeightTableDemo;
	}

	memset(_sprA02Table, 0, sizeof(_sprA02Table));
	memset(_sprC02Table, 0, sizeof(_sprC02Table));
	memset(_actionsTable, 0, sizeof(_actionsTable));
	_actionsCount = 0;
	memset(_locationObjectsTable, 0, sizeof(_locationObjectsTable));
	_locationObjectsCount = 0;
	memset(_spritesTable, 0, sizeof(_spritesTable));
	_spritesCount = 0;
	memset(_locationAnimationsTable, 0, sizeof(_locationAnimationsTable));
	_locationAnimationsCount = 0;
	memset(_dataTable, 0, sizeof(_dataTable));
	_dataCount = 0;
	memset(_charPosTable, 0, sizeof(_charPosTable));
	_charPosCount = 0;
	memset(_locationSoundsTable, 0, sizeof(_locationSoundsTable));
	_locationSoundsCount = 0;
	memset(_locationMusicsTable, 0, sizeof(_locationMusicsTable));
	_locationMusicsCount = 0;

	_mousePosX = _mousePosY = 0;
	_prevMousePosX = _prevMousePosY = 0;
	_mouseButtonsMask = 0;
	_mouseClick = 0;
	_saveOrLoadGamePanel = 0;
	_mouseIdleCounter = 0;
	_leftMouseButtonPressed = _rightMouseButtonPressed = false;
	_lastKeyPressed = 0;
	memset(_inputKeys, 0, sizeof(_inputKeys));
	_cursorStyle = kCursorNormal;
	_cursorState = kCursorStateNormal;
	_updateCursorFlag = false;

	_panelStyle = kPanelStyleIcons;
	_panelState = kPanelStateNormal;
	_panelType  = kPanelTypeNormal;
	_forceRedrawPanelItems = true;
	_redrawPanelItemsCounter = 0;
	memset(_panelObjectsOffsetTable, 0, sizeof(_panelObjectsOffsetTable));
	_switchPanelCounter = 0;
	_conversationOptionsCount = 0;
	_fadedPanel = false;
	_panelLockedFlag = false;
	_conversationOptionLinesCount = 0;
	memset(_inventoryItemsState, 0, sizeof(_inventoryItemsState));
	memset(_inventoryObjectsList, 0, sizeof(_inventoryObjectsList));
	_inventoryObjectsOffset = 0;
	_inventoryObjectsCount = 0;
	_lastInventoryObjectIndex = 0;

	_currentFxSet = 0;
	_currentFxDist = 0;
	_currentFxScale = 0;
	_currentFxVolume = 0;
	_currentFxIndex = 0;
	_speechSoundNum = 0;
	_speechVolume = kMaxSoundVolume;
	memset(_miscSoundFxNum, 0, sizeof(_miscSoundFxNum));
	memset(_speechHistoryTable, 0, sizeof(_speechHistoryTable));
	_charSpeechSoundCounter = 0;
	memset(_miscSoundFxDelayCounter, 0, sizeof(_miscSoundFxDelayCounter));
	_characterSoundFxDelayCounter = 0;
	_characterSoundFxNum = 0;
	_speechSoundBaseNum = 0;

	_pendingActionIndex = 0;
	_pendingActionDelay = 0;
	_charPositionFlagNum = 0;
	_charPositionFlagValue = 0;
	_actionVerb = _currentActionVerb = _previousActionVerb = kVerbWalk;
	_actionVerbLocked = false;
	_nextAction = 0;
	_selectedObjectNum = 0;
	_selectedObjectType = 0;
	_selectedCharacterNum = 0;
	_actionObj1Type = _actionObj2Type = 0;
	_actionObj1Num = _actionObj2Num = 0;
	_actionRequiresTwoObjects = false;
	_actionPosX = 0;
	_actionPosY = 0;
	_selectedObjectLocationMask = false;
	memset(&_selectedObject, 0, sizeof(_selectedObject));
	_selectedCharacterDirection = 0;
	_selectedCharacter2Num = 0;
	_currentActionObj1Num = _currentActionObj2Num = 0;
	_currentInfoString1SourceType = _currentInfoString2SourceType = 0;
	memset(_speechActionCounterTable, 0, sizeof(_speechActionCounterTable));
	_actionCharacterNum = 0;

	_csDataLoaded = false;
	_csDataHandled = false;
	_stopActionOnSoundFlag = false;
	_stopActionOnSpeechFlag = false;
	_stopActionOnPanelLock = false;
	_csDataTableCount = 0;
	_stopActionCounter = 0;
	_actionTextColor = 0;
	_nextTableToLoadIndex = 0;
	memset(_nextTableToLoadTable, 0, sizeof(_nextTableToLoadTable));
	_soundInstructionIndex = 0;
	_tableInstructionsPtr = nullptr;
	memset(_tableInstructionObj1Table, 0, sizeof(_tableInstructionObj1Table));
	memset(_tableInstructionObj2Table, 0, sizeof(_tableInstructionObj2Table));
	_tableInstructionFlag = false;
	_tableInstructionItemNum1 = _tableInstructionItemNum2 = 0;
	memset(_instructionsActionsTable, 0, sizeof(_instructionsActionsTable));
	_validInstructionId = false;

	memset(_spriteFramesTable, 0, sizeof(_spriteFramesTable));
	memset(_spriteAnimationsTable, 0, sizeof(_spriteAnimationsTable));
	memset(_spriteAnimationFramesTable, 0, sizeof(_spriteAnimationFramesTable));
	_spriteAnimationFrameIndex = 0;
	_backgroundSpriteCurrentFrame = 0;
	_backgroundSpriteLastFrame = 0;
	_backgroundSpriteCurrentAnimation = -1;
	_disableCharactersPath = false;
	_skipCurrentCharacterDraw = false;
	_yPosCurrent = 131;
	_xPosCurrent = 160;
	_characterSpeechDataPtr = nullptr;
	_ptTextOffset = 0;
	memset(_characterAnimationsTable, 0, sizeof(_characterAnimationsTable));
	memset(_characterStateTable, 0, sizeof(_characterStateTable));
	_backgroundSprOffset = 0;
	_mainSpritesBaseOffset = 0;
	_currentSpriteAnimationLength = 0;
	_currentSpriteAnimationFrame = 0;
	_currentSpriteAnimationFrame2 = 0;
	_characterAnimationIndex = -1;
	_characterFacingDirection = _characterPrevFacingDirection = 0;
	_characterBackFrontFacing = _characterPrevBackFrontFacing = false;
	_characterAnimationNum = 0;
	_noCharacterAnimationChange = 0;
	_characterSpriteAnimationFrameCounter = 0;
	_locationMaskIgnore = false;
	_locationMaskType = 0;
	_locationMaskCounter = 0;
	_handleMapCounter = 0;
	_noPositionChangeAfterMap = false;
	_changeBackgroundSprite = false;
	_updateSpriteFlag1 = false;
	_updateSpriteFlag2 = false;

	_mirroredDrawing = false;
	_loadLocBufPtr = nullptr;
	_backgroundSpriteDataPtr = nullptr;
	_locationHeight = 0;
	_scrollOffset = 0;
	_currentGfxBackgroundCounter = 0;
	_currentGfxBackground = nullptr;
	_fadePaletteCounter = 0;
	memset(_currentPalette, 0, sizeof(_currentPalette));
	_fullRedraw = false;
	_dirtyRectsPrevCount = _dirtyRectsCount = 0;

	_updateLocationFadePaletteCounter = 0;
	_updateLocationCounter = 10;
	_updateLocationPos = 0;
	for (int i = 0; i < 5; ++i) {
		_updateLocationXPosTable[i] = 160;
		_updateLocationYPosTable[i] = 131;
	}
	memset(_updateLocationFlagsTable, 0, sizeof(_updateLocationFlagsTable));
	memset(_updateLocationXPosTable2, 0, sizeof(_updateLocationXPosTable2));
	memset(_updateLocationYPosTable2, 0, sizeof(_updateLocationYPosTable2));
	memset(_updateLocationYMaxTable, 0, sizeof(_updateLocationYMaxTable));
	memset(_updateLocation14Step, 0, sizeof(_updateLocation14Step));
	memset(_updateLocation14ObjNum, 0, sizeof(_updateLocation14ObjNum));
	memset(_updateLocation14Delay, 0, sizeof(_updateLocation14Delay));
	_updateLocationCounter2 = 0;
	_updateLocationFlag = false;
	_updateLocation70StringLen = 0;
	memset(_updateLocation70String, 0, sizeof(_updateLocation70String));
}

void TuckerEngine::mainLoop() {
	allocateBuffers();
	resetVariables();

	loadCharSizeDta();
	if ((_gameFlags & kGameFlagDemo) != 0) {
		addObjectToInventory(30);
		addObjectToInventory(12);
	}
	loadCharset();
	loadPanel();

	loadFile("infobar.txt", _infoBarBuf);
	// WORKAROUND capitalized "With"/"Con" in the English/Spanish versions
	// Fixes Trac#10445.
	if (_gameLang == Common::EN_ANY) {
		_infoBarBuf[getPositionForLine(kVerbPrepositionWith, _infoBarBuf)] = 'w';
	} else if (_gameLang == Common::ES_ESP) {
		_infoBarBuf[getPositionForLine(kVerbPrepositionWith, _infoBarBuf)] = 'c';
	}

	_data5Buf    = loadFile("data5.c", nullptr);
	_bgTextBuf   = loadFile("bgtext.c", nullptr);
	_charNameBuf = loadFile("charname.c", nullptr);
	_csDataBuf   = loadFile("csdata.c", nullptr);
	_csDataSize = _fileLoadSize;

	_currentSaveLoadGameState = 1;

	loadBudSpr();
	loadCursor();
	setCursorStyle(_cursorStyle);
	setCursorState(_cursorState);

	_flagsTable[219] = 1;
	_flagsTable[105] = 1;

	_spriteAnimationFrameIndex =  _spriteAnimationsTable[14]._firstFrameIndex;

	if (ConfMan.hasKey("save_slot")) {
		const int slot = ConfMan.getInt("save_slot");
		if (slot >= 0 && slot <= kLastSaveSlot) {
			loadGameState(slot);
		}
	} else if (ConfMan.hasKey("boot_param")) {
		_nextLocation = (Location)ConfMan.getInt("boot_param");
	}

	do {
		++_syncCounter;
		if (_flagsTable[137] != _flagsTable[138]) {
			loadBudSpr();
			_flagsTable[138] = _flagsTable[137];
		}
		if (_syncCounter >= 2) {
			_syncCounter = 0;
			waitForTimer(2);
		}
		updateMouseState();
		if (_fadePaletteCounter < 16) {
			if (_fadePaletteCounter > 1) {
				fadeOutPalette();
			}
			++_fadePaletteCounter;
		}
		if (_fadePaletteCounter > 19 && _fadePaletteCounter < 34) {
			fadeInPalette();
			++_fadePaletteCounter;
		}
		if (_nextAction != 0) {
			loadActionsTable();
		}
		if (_nextLocation != kLocationNone) {
			setupNewLocation();
		}
		updateCharPosition();
		if (_cursorState == kCursorStateNormal) {
			updateCursor();
		} else if (_panelType == kPanelTypeLoadSavePlayQuit) {
			handleMouseOnPanel();
		}
		if (_mainLoopCounter2 == 0) {
			updateFlagsForCharPosition();
		}
		if (_syncCounter == 0 || !_disableCharactersPath) {
			updateCharactersPath();
		}
		if (_mainLoopCounter2 == 0) {
			updateCharacterAnimation();
			if (_backgroundSpriteCurrentAnimation == -1) {
				_flagsTable[207] = 0;
				if (_flagsTable[220] > 0) {
					_flagsTable[_flagsTable[220]] = _flagsTable[221];
					_flagsTable[220] = 0;
				}
				if (_flagsTable[158] == 1) {
					_flagsTable[158] = 0;
					_skipCurrentCharacterDraw = true;
				}
				_mainLoopCounter1 = 0;
			}
		}
		if (_mainLoopCounter1 == 0) {
			updateSprites();
			updateData3();
			updateSfxData3_1();
		}
		++_mainLoopCounter2;
		handleMap();
		updateScreenScrolling();
		if (_mainLoopCounter2 > 4) {
			_mainLoopCounter2 = 0;
			updateSfxData3_2();
		}
		++_mainLoopCounter1;
		if (_mainLoopCounter1 > 5) {
			_mainLoopCounter1 = 0;
		}
		if (_locationHeight == 140) {
			togglePanelStyle();
			redrawPanelItems();
			if (_displayGameHints && _gameHintsIndex < 6) {
				updateGameHints();
			}
			if (_panelType == kPanelTypeNormal) {
				if (_panelLockedFlag || _pendingActionDelay > 0) {
					if (!_fadedPanel) {
						updateItemsGfxColors(0x60, 0x80);
						_fadedPanel = true;
					}
				} else {
					_fadedPanel = false;
					clearItemsGfx();
					if (_gamePaused) {
						drawPausedInfoBar();
					} else if (_displayHintsText && _mouseIdleCounter > 1000) {
						drawGameHintString();
					} else {
						drawInfoString();
					}
				}
			}
		}
		_mainSpritesBaseOffset = 0;
		if (_locationWidthTable[_location] > 3) {
			++_currentGfxBackgroundCounter;
			if (_currentGfxBackgroundCounter > 39) {
				_currentGfxBackgroundCounter = 0;
			}
			_currentGfxBackground = _quadBackgroundGfxBuf + (_currentGfxBackgroundCounter / 10) * 44800;
			if (_fadePaletteCounter < 34 && _location == kLocationFishingTrawler) {
				int offset = (_currentGfxBackgroundCounter > 29 ? 1 : (_currentGfxBackgroundCounter / 10));
				_spritesTable[0]._gfxBackgroundOffset = offset * 640;
				_mainSpritesBaseOffset = offset;
			}
			_fullRedraw = true;
		} else {
			_currentGfxBackground = _quadBackgroundGfxBuf;
		}
		if (_syncCounter != 0) {
			continue;
		}
		if (_scrollOffset < 320) {
			Graphics::copyRect(_locationBackgroundGfxBuf + _scrollOffset, 640, _currentGfxBackground + _scrollOffset, 320, 320 - _scrollOffset, _locationHeight);
		}
		if (_scrollOffset > 0) {
			Graphics::copyRect(_locationBackgroundGfxBuf + 320, 640, _currentGfxBackground + 44800, 320, _scrollOffset, _locationHeight);
		}
		drawData3();
		execData3PreUpdate();
		for (int i = 0; i < _spritesCount; ++i) {
			if (!_spritesTable[i]._disabled) {
				drawSprite(i);
			}
		}
		if (!_skipCurrentCharacterDraw) {
			if (_backgroundSpriteCurrentAnimation > -1 && _backgroundSpriteCurrentFrame > 0) {
				drawBackgroundSprites();
			} else {
				drawCurrentSprite();
			}
		}
		if (_locationHeight == 140) {
			redrawPanelOverBackground();
		}
		if (_panelType == kPanelTypeLoadSaveSavegame) {
			saveOrLoad();
		}
		execData3PostUpdate();
		if (_timerCounter2 > 45) {
			_timerCounter2 = 0;
		}
		updateSoundsTypes3_4();
		if (_currentFxSet != 0) {
			setSoundVolumeDistance();
		}
		updateCharSpeechSound(_displaySpeechText);
		redrawScreen(_scrollOffset);
		startCharacterSounds();
		for (int num = 0; num < 2; ++num) {
			if (_miscSoundFxDelayCounter[num] > 0) {
				--_miscSoundFxDelayCounter[num];
				if (_miscSoundFxDelayCounter[num] == 0) {
					const int index = _miscSoundFxNum[num];
					startSound(_locationSoundsTable[index]._offset, index, _locationSoundsTable[index]._volume);
				}
			}
		}
		if (_gamePaused && _charSpeechSoundCounter == 0) {
			stopSounds();
			while (1) {
				waitForTimer(1);
				if (_inputKeys[kInputKeyPause]) {
					_inputKeys[kInputKeyPause] = false;
					if (_charSpeechSoundCounter <= 0) {
						break;
					}
				}
				if (_charSpeechSoundCounter == 0) {
					if (_lastKeyPressed >= Common::KEYCODE_1 && _lastKeyPressed <= Common::KEYCODE_5) {
						if (_speechHistoryTable[_lastKeyPressed - Common::KEYCODE_1] > 0) {
							startSpeechSound(_speechHistoryTable[_lastKeyPressed - Common::KEYCODE_1], 100);
							_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
						}
						_lastKeyPressed = 0;
					}
				}
				updateCharSpeechSound(false);
			}
			playSounds();
			_gamePaused = false;
		}
		if (_inputKeys[kInputKeyPause]) {
			_inputKeys[kInputKeyPause] = false;
			if (_location != kLocationComputerScreen) {
				_gamePaused = true;
			}
		}
		if (_inputKeys[kInputKeyToggleTextSpeech]) {
			_inputKeys[kInputKeyToggleTextSpeech] = false;
			if ((_gameFlags & kGameFlagNoSubtitles) == 0) {
				_displaySpeechText = !_displaySpeechText;
				ConfMan.setBool("subtitles", _displaySpeechText);
			}
		}
		if (_inputKeys[kInputKeyHelp]) {
			_inputKeys[kInputKeyHelp] = false;
			if (_displayGameHints && _displayHintsText) {
				_gameHintsStringNum = _gameHintsIndex + 1;
				_mouseIdleCounter = 1100;
			}
		}
		if (_inputKeys[kInputKeyEscape]) {
			_inputKeys[kInputKeyEscape] = false;
			if (_gameDebug) {
				_flagsTable[236] = 74;
			}
		}

		if (_flagsTable[236] > 70) {
			handleCreditsSequence();
			_quitGame = true;
		}
	} while (!_quitGame && _flagsTable[100] == 0);

	// auto save on quit
	saveAutosaveIfEnabled();

	if (_flagsTable[100] == 1) {
		handleCongratulationsSequence();
	}
	unloadSprA02_01();
	unloadSprC02_01();
	freeBuffers();
}

void TuckerEngine::waitForTimer(int ticksCount) {
	uint32 end = _lastFrameTime + ticksCount * 1000 / 46;
	do {
		parseEvents();
		_system->delayMillis(10);
		_lastFrameTime = _system->getMillis();
	} while (!_fastMode && _lastFrameTime < end);
	_timerCounter2 += ticksCount;
}

void TuckerEngine::parseEvents() {
	Common::Event ev;
	while (_eventMan->pollEvent(ev)) {
		switch (ev.type) {
		case Common::EVENT_KEYDOWN:
			switch (ev.kbd.ascii) {
			// do not use KEYCODE_PERIOD here so that it works with most keyboard layouts
			case '.':
				_inputKeys[kInputKeySkipSpeech] = true;
				break;
			default:
				break;
			}
			switch (ev.kbd.keycode) {
			case Common::KEYCODE_f:
				if (ev.kbd.hasFlags(Common::KBD_CTRL)) {
					_fastMode = !_fastMode;
				}
				break;
			case Common::KEYCODE_p:
				_inputKeys[kInputKeyPause] = true;
				break;
			case Common::KEYCODE_F1:
				_inputKeys[kInputKeyTogglePanelStyle] = true;
				break;
			case Common::KEYCODE_F2:
				_inputKeys[kInputKeyToggleTextSpeech] = true;
				break;
			case Common::KEYCODE_F3:
				_inputKeys[kInputKeyHelp] = true;
				break;
			case Common::KEYCODE_ESCAPE:
				_inputKeys[kInputKeyEscape] = true;
				_inputKeys[kInputKeySkipSpeech] = true;
				break;
			default:
				break;
			}
			_lastKeyPressed = ev.kbd.keycode;
			break;
		case Common::EVENT_MOUSEMOVE:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			break;
		case Common::EVENT_LBUTTONDOWN:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			_mouseButtonsMask |= 1;
			break;
		case Common::EVENT_LBUTTONUP:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			break;
		case Common::EVENT_RBUTTONDOWN:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			_mouseButtonsMask |= 2;
			_inputKeys[kInputKeySkipSpeech] = true;
			break;
		case Common::EVENT_RBUTTONUP:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			break;
		case Common::EVENT_WHEELUP:
			_mouseButtonsMask |= 4;
			break;
		case Common::EVENT_WHEELDOWN:
			_mouseButtonsMask |= 8;
			break;
		default:
			break;
		}
	}

	if (_inputKeys[kInputKeyTogglePanelStyle]) {
		if (_panelType == kPanelTypeNormal && _panelState == kPanelStateNormal) {
			_switchPanelCounter = 1;
			_panelState = kPanelStateShrinking;
		}
		_inputKeys[kInputKeyTogglePanelStyle] = false;
	}

	if (_inputKeys[kInputKeySkipSpeech]) {
		if (isSpeechSoundPlaying()) {
			stopSpeechSound();
		}
		_inputKeys[kInputKeySkipSpeech] = false;
	}

	_quitGame = shouldQuit();
}

void TuckerEngine::updateCursorPos(int x, int y) {
	_prevMousePosX = _mousePosX;
	_prevMousePosY = _mousePosY;
	_mousePosX = x;
	_mousePosY = y;
}

void TuckerEngine::setCursorStyle(CursorStyle style) {
	_cursorStyle = style;
	static const int cursorW = 16;
	static const int cursorH = 16;
	CursorMan.replaceCursor(_cursorGfxBuf + _cursorStyle * 256, cursorW, cursorH, 1, 1, 0);
}

void TuckerEngine::setCursorState(CursorState state) {
	_cursorState = state;
	CursorMan.showMouse(_cursorState != kCursorStateDisabledHidden);
}

void TuckerEngine::showCursor(bool visible) {
	CursorMan.showMouse(visible);
}

void TuckerEngine::setupNewLocation() {
	debug(2, "setupNewLocation() current %d next %d", _location, _nextLocation);
	_location = _nextLocation;
	loadObj();
	_nextLocation = kLocationNone;
	_fadePaletteCounter = 0;
	_mainLoopCounter2 = 0;
	_mainLoopCounter1 = 0;
	_characterFacingDirection = 0;
	_actionVerbLocked = false;
	_locationMaskIgnore = false;
	_backgroundSprOffset = 0;
	if (_backgroundSpriteCurrentAnimation > 0 && _backgroundSpriteCurrentFrame > 0) {
		_backgroundSpriteCurrentAnimation = -1;
		_backgroundSpriteCurrentFrame = 0;
	}
	if (!_panelLockedFlag || (_backgroundSpriteCurrentAnimation > 0 && _location != kLocationVentSystem)) {
		_locationMaskType = 0;
	} else {
		_locationMaskType = 3;
	}
	while (_spriteAnimationFramesTable[_spriteAnimationFrameIndex] != 999) {
		++_spriteAnimationFrameIndex;
	}
	_execData3Counter = 0;
	stopSounds();
	loadLoc();
	loadData4();
	loadData3();
	loadActionFile();
	loadCharPos();
	loadSprA02_01();
	loadSprC02_01();
	loadFx();
	playSounds();
	if (_flagsTable[215] > 0) {
		handleMeanwhileSequence();
		_flagsTable[215] = 0;
	}
	if (_flagsTable[231] > 0) {
		handleMeanwhileSequence();
		_flagsTable[231] = 0;
	}
}

void TuckerEngine::copyLocBitmap(const char *filename, int offset, bool isMask) {
	int type = !isMask ? 1 : 0;
	if (offset > 0 && _location == kLocationPark) {
		type = 0;
	}
	loadImage(filename, _loadTempBuf, type);
	uint8 *dst = isMask ? _locationBackgroundMaskBuf : _locationBackgroundGfxBuf;
	dst += offset;
	const uint8 *src = _loadTempBuf;
	for (int y = 0; y < _locationHeight; ++y) {
		memcpy(dst, src, 320);
		src += 320;
		dst += 640;
	}
}

void TuckerEngine::updateMouseState() {
	if (_cursorState != kCursorStateDisabledHidden) {
		_leftMouseButtonPressed = (_mouseButtonsMask & 1) != 0;
		if (_leftMouseButtonPressed) {
			_mouseIdleCounter = 0;
			_gameHintsStringNum = 0;
		}
		_rightMouseButtonPressed = (_mouseButtonsMask & 2) != 0;
		_mouseWheelUp   = _mouseButtonsMask & 4;
		_mouseWheelDown = _mouseButtonsMask & 8;
		_mouseButtonsMask = 0;
		if (_prevMousePosX == _mousePosX && _prevMousePosY == _mousePosY) {
			++_mouseIdleCounter;
		} else {
			_mouseIdleCounter = 0;
			_gameHintsStringNum = 0;
		}
	}
	if (_cursorState == kCursorStateDialog) {
		if (_panelType == kPanelTypeEmpty) {
			setCursorStyle(kCursorTalk);
		}
#if 0
		// confine cursor to dialog area
		if (_mousePosY < 140) {
			_mousePosY = 140;
			_system->warpMouse(_mousePosX, _mousePosY);
		}
#endif
	}
}

void TuckerEngine::updateCharPositionHelper() {
	setCursorState(kCursorStateDisabledHidden );
	_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
	_currentActionVerb = kVerbWalk;
	startSpeechSound(_speechSoundNum, _speechVolume);
	int pos = getPositionForLine(_speechSoundNum, _characterSpeechDataPtr);
	_characterSpeechDataPtr += pos;
	_speechSoundNum = 0;
}

void TuckerEngine::updateCharPosition() {
	if (_currentActionVerb == kVerbWalk || _locationMaskCounter == 0) {
		return;
	}
	if (_currentActionVerb == kVerbLook && _location != kLocationRoystonsHomeBoxroom) {
		int pos;
		_actionPosX = _xPosCurrent;
		_actionPosY = _yPosCurrent - 64;
		_actionTextColor = 1;
		_actionCharacterNum = 99;
		switch (_currentInfoString1SourceType) {
		case 0:
			if (_currentActionObj1Num == 0) {
				return;
			}
			if (_currentActionObj1Num == 259) {
				handleSpecialObjectSelectionSequence();
				_currentActionVerb = kVerbWalk;
				return;
			}
			_speechSoundNum = _currentActionObj1Num;
			_characterSpeechDataPtr = _ptTextBuf;
			pos = getPositionForLine(_speechSoundNum + 1865, _characterSpeechDataPtr);
			if (_characterSpeechDataPtr[pos] == '*') {
				switch (_characterSpeechDataPtr[pos + 1]) {
				case 'E':
					++_flagsTable[200];
					if (_flagsTable[200] > 6) {
						_flagsTable[200] = 1;
					}
					_speechSoundNum = 262 + _flagsTable[200];
					break;
				case 'M':
					++_flagsTable[200];
					if (_flagsTable[200] > 10) {
						_flagsTable[200] = 1;
					}
					_speechSoundNum = 268 + _flagsTable[200];
					break;
				case 'R':
					++_flagsTable[200];
					if (_flagsTable[200] > 10) {
						_flagsTable[200] = 1;
					}
					_speechSoundNum = 281 + _flagsTable[200];
					break;
				default:
					break;
				}
			}
			_speechSoundNum += 1865;
			updateCharPositionHelper();
			return;
		case 1:
			if (_locationAnimationsTable[_selectedCharacter2Num]._getFlag == 1) {
				_speechSoundNum = _speechSoundBaseNum + _locationAnimationsTable[_selectedCharacter2Num]._inventoryNum;
				_characterSpeechDataPtr = _ptTextBuf;
				updateCharPositionHelper();
				return;
			} else if (_currentActionObj1Num == 91) {
				handleSpecialObjectSelectionSequence();
				_currentActionVerb = kVerbWalk;
				return;
			}
			break;
		case 2:
			_characterSpeechDataPtr = _ptTextBuf;
			_speechSoundNum = 2175 + _charPosTable[_selectedCharacterNum]._description;
			if (_charPosTable[_selectedCharacterNum]._description != 0) {
				updateCharPositionHelper();
				return;
			}
			break;
		default:
			break;
		}
	}
	int actionKey = _currentActionObj2Num * 1000000 + _currentInfoString2SourceType * 100000 + _currentActionVerb * 10000 + _currentInfoString1SourceType * 1000 + _currentActionObj1Num;
	debug(3, "updateCharPosition() actionKey %d",  actionKey);
	bool skip = false;
	Action *action = nullptr;
	for (int i = 0; i < _actionsCount && !skip; ++i) {
		action = &_actionsTable[i];
		if (action->_key == actionKey) {
			skip = true;
			if (action->_testFlag1Num != 0) {
				if (action->_testFlag1Num < 500) {
					if (action->_testFlag1Num >= 300)
						error("updateCharPosition() - Unexpected value for _testFlag1Num : %d", action->_testFlag1Num);
					if (_flagsTable[action->_testFlag1Num] != action->_testFlag1Value)
						skip = false;
				} else if (_inventoryItemsState[action->_testFlag1Num - 500] != action->_testFlag1Value) {
					skip = false;
				}
				debug(3, "updateCharPosition() flag1 %d value %d", action->_testFlag1Num, action->_testFlag1Value);
			}
			if (action->_testFlag2Num != 0) {
				if (action->_testFlag2Num < 500) {
					if (action->_testFlag2Num >= 300)
						error("updateCharPosition() - Unexpected value for _testFlag1Num : %d", action->_testFlag1Num);
					if (_flagsTable[action->_testFlag2Num] != action->_testFlag2Value)
						skip = false;
				} else if (_inventoryItemsState[action->_testFlag2Num - 500] != action->_testFlag2Value) {
					skip = false;
				}
				debug(3, "updateCharPosition() flag2 %d value %d", action->_testFlag2Num, action->_testFlag2Value);
			}
		}
	}
	if (!skip) {
		playSpeechForAction(_currentActionVerb);
		_currentActionVerb = kVerbWalk;
		return;
	}
	assert(action);
	if (action->_speech != 6) {
		if (action->_speech < 100) {
			_spriteAnimationFrameIndex = _spriteAnimationsTable[action->_speech]._firstFrameIndex;
			_currentSpriteAnimationLength = _spriteAnimationsTable[action->_speech]._numParts;
			_mirroredDrawing = (action->_flipX != 0);
			_characterFacingDirection = 5;
			_mainLoopCounter2 = 0;
		} else {
			_backgroundSpriteCurrentAnimation = action->_speech - 100;
			_backgroundSpriteCurrentFrame = 0;
			_mirroredDrawing = false;
		}
	}
	_pendingActionDelay = action->_delay;
	_charPositionFlagNum = action->_setFlagNum;
	_charPositionFlagValue = action->_setFlagValue;
	_pendingActionIndex = action->_index;
	_characterSoundFxDelayCounter = action->_fxDelay;
	_characterSoundFxNum = action->_fxNum;
	_previousActionVerb = _currentActionVerb;
	_currentActionVerb = kVerbWalk;
}

void TuckerEngine::updateFlagsForCharPosition() {
	if (_pendingActionDelay != 0) {
		--_pendingActionDelay;
		if (_pendingActionDelay > 0) {
			return;
		}
		switch (_previousActionVerb) {
		case kVerbTalk:
		case kVerbOpen:
		case kVerbClose:
		case kVerbUse:
			debug(3, "updateFlagsForCharPosition() set flag %d value %d", _charPositionFlagNum, _charPositionFlagValue);
			_flagsTable[_charPositionFlagNum] = _charPositionFlagValue;
			break;
		case kVerbTake:
			if (_charPositionFlagValue == 1) {
				addObjectToInventory(_charPositionFlagNum);
				_forceRedrawPanelItems = true;
			}
			break;
		default:
			break;
		}
		if (_pendingActionIndex > 0) {
			_nextAction = _pendingActionIndex;
		}
	}
}

void TuckerEngine::fadeOutPalette(int colorsCount) {
	uint8 pal[256 * 3];
	_system->getPaletteManager()->grabPalette(pal, 0, colorsCount);
	for (int color = 0; color < colorsCount; ++color) {
		for (int i = 0; i < 3; ++i) {
			const int c = int(pal[color * 3 + i]) + kFadePaletteStep * 4;
			pal[color * 3 + i] = MIN<int>(c, _currentPalette[color * 3 + i]);
		}
	}
	_system->getPaletteManager()->setPalette(pal, 0, colorsCount);
	_system->updateScreen();
}

void TuckerEngine::fadeInPalette(int colorsCount) {
	uint8 pal[256 * 3];
	_system->getPaletteManager()->grabPalette(pal, 0, colorsCount);
	for (int color = 0; color < colorsCount; ++color) {
		for (int i = 0; i < 3; ++i) {
			const int c = int(pal[color * 3 + i]) - kFadePaletteStep * 4;
			pal[color * 3 + i] = MAX<int>(c, 0);
		}
	}
	_system->getPaletteManager()->setPalette(pal, 0, colorsCount);
	_system->updateScreen();
}

void TuckerEngine::fadePaletteColor(int color, int step) {
	uint8 rgb[4];
	_system->getPaletteManager()->grabPalette(rgb, color, 1);
	for (int i = 0; i < 3; ++i) {
		const int c = _currentPalette[color * 3 + i] + step * 4;
		rgb[i] = MIN(c, 255);
	}
	_system->getPaletteManager()->setPalette(rgb, color, 1);
}

void TuckerEngine::setBlackPalette() {
	uint8 pal[256 * 3];
	memset(pal, 0, sizeof(pal));
	_system->getPaletteManager()->setPalette(pal, 0, 256);
}

void TuckerEngine::updateCursor() {
	setCursorStyle(kCursorNormal);
	if (_backgroundSpriteCurrentAnimation == -1 && !_panelLockedFlag && _selectedObject._locationObjectLocation != kLocationNone) {
		_selectedObject._locationObjectLocation = kLocationNone;
	}
	if (_locationMaskType > 0 || _selectedObject._locationObjectLocation != kLocationNone || _pendingActionDelay > 0) {
		return;
	}
	if (_rightMouseButtonPressed) {
		if (!_updateCursorFlag) {
			if (_actionVerb == kVerbLast) {
				_actionVerb = kVerbFirst;
			} else {
				_actionVerb = (Verb)(_actionVerb + 1);
			}
			_updateCursorFlag = true;
			_actionVerbLocked = true;
			_actionRequiresTwoObjects = false;
		}
	} else {
		_updateCursorFlag = false;
	}
	if (!_actionVerbLocked) {
		setActionVerbUnderCursor();
		if (_actionVerb == kVerbWalk && _location == kLocationTV) {
			_actionVerb = kVerbUse;
		}
	}
	_selectedObjectNum = 0;
	_selectedObjectType = 0;
	int num = setCharacterUnderCursor();
	if (_selectedObjectType == 0) {
		num = setLocationAnimationUnderCursor();
	}
	if (_selectedObjectType > 0) {
		_selectedObjectNum = num;
	} else {
		num = getObjectUnderCursor();
		if (num > -1) {
			_selectedObjectNum = _locationObjectsTable[num]._textNum;
		}
	}
	handleMouseClickOnInventoryObject();
	if (_actionVerb == kVerbTalk && _selectedObjectType != 2) {
		_selectedObjectNum = 0;
		_selectedObjectType = 0;
	} else if (_actionVerb == kVerbGive && _selectedObjectType != 3 && !_actionRequiresTwoObjects) {
		_selectedObjectNum = 0;
		_selectedObjectType = 0;
	}
	if (!_actionVerbLocked && _selectedObjectType == 2 && _selectedObjectNum != 21) {
		_actionVerb = kVerbTalk;
	}
	if (!_actionRequiresTwoObjects) {
		_actionObj1Num = _selectedObjectNum;
		_actionObj1Type = _selectedObjectType;
		_actionObj2Num = 0;
		_actionObj2Type = 0;
	} else if (_actionObj1Num == _selectedObjectNum && _actionObj1Type == _selectedObjectType) {
		_selectedObjectNum = 0;
		_selectedObjectType = 0;
		_actionObj2Num = 0;
		_actionObj2Type = 0;
	} else {
		_actionObj2Num = _selectedObjectNum;
		_actionObj2Type = _selectedObjectType;
	}
	if (!_leftMouseButtonPressed) {
		_mouseClick = 0;
	}
	if (_mousePosY >= 150) {
		if (_mouseWheelUp)
			moveDownInventoryObjects();
		else if (_mouseWheelDown)
			moveUpInventoryObjects();
	}
	if (_leftMouseButtonPressed && _mouseClick == 0) {
		_fadedPanel = false;
		_mouseClick = 1;
		clearItemsGfx();
		drawInfoString();
		if (_mousePosY >= 150 && _mousePosX < 212) {
			if (_mousePosX < 200) {
				setActionVerbUnderCursor();
				_actionVerbLocked = true;
				_actionRequiresTwoObjects = false;
			} else if (_mousePosY < 175) {
				moveDownInventoryObjects();
			} else {
				moveUpInventoryObjects();
			}
		} else {
			if (_selectedObjectType == 3) {
				setActionForInventoryObject();
			} else if (_actionVerb != kVerbWalk) {
				_actionVerbLocked = false;
				setActionState();
			} else if (_actionObj1Num == 261 || (_actionObj1Num == 205 && _flagsTable[143] == 0)) {
				_actionVerbLocked = false;
				setActionState();
			} else {
				_actionVerbLocked = false;
				_actionRequiresTwoObjects = false;
				_currentActionVerb = kVerbWalk;
				setSelectedObjectKey();
			}
		}
	}
}

void TuckerEngine::stopSounds() {
	for (int i = 0; i < _locationSoundsCount; ++i) {
		stopSound(i);
	}
	for (int i = 0; i < _locationMusicsCount; ++i) {
		stopMusic(i);
	}
}

void TuckerEngine::playSounds() {
	for (int i = 0; i < _locationSoundsCount; ++i) {
		if (_locationSoundsTable[i]._type == 1 || _locationSoundsTable[i]._type == 2 || _locationSoundsTable[i]._type == 5 ||
			(_locationSoundsTable[i]._type == 7 && _flagsTable[_locationSoundsTable[i]._flagNum] == _locationSoundsTable[i]._flagValueStartFx)) {
			startSound(_locationSoundsTable[i]._offset, i, _locationSoundsTable[i]._volume);
		}
	}
	for (int i = 0; i < _locationMusicsCount; ++i) {
		if (_locationMusicsTable[i]._flag > 0) {
			startMusic(_locationMusicsTable[i]._offset, i, _locationMusicsTable[i]._volume);
		}
	}
}

void TuckerEngine::updateCharactersPath() {
	if (!_panelLockedFlag) {
		return;
	}
	if (_backgroundSpriteCurrentAnimation != -1 && _location != kLocationVentSystem) {
		if (_xPosCurrent == _selectedObject._xPos && _yPosCurrent == _selectedObject._yPos) {
			_locationMaskCounter = 1;
			_panelLockedFlag = false;
		}
		return;
	}
	int xPos = _xPosCurrent;
	int yPos = _yPosCurrent;
	if (_characterFacingDirection == 5) {
		_characterPrevFacingDirection = 5;
	}
	bool flag = false;
	if (_yPosCurrent > _selectedObject._yPos) {
		if (testLocationMask(_xPosCurrent, _yPosCurrent - 1)) {
			--_yPosCurrent;
			_characterFacingDirection = 4;
			flag = true;
		}
	} else if (_yPosCurrent < _selectedObject._yPos) {
		if (testLocationMask(_xPosCurrent, _yPosCurrent + 1)) {
			++_yPosCurrent;
			_characterFacingDirection = 2;
			flag = true;
		}
	}
	if (_xPosCurrent > _selectedObject._xPos) {
		if (testLocationMask(_xPosCurrent - 1, _yPosCurrent)) {
			--_xPosCurrent;
			_characterFacingDirection = 3;
			_characterBackFrontFacing = false;
			flag = true;
		}
	} else if (_xPosCurrent < _selectedObject._xPos) {
		if (testLocationMask(_xPosCurrent + 1, _yPosCurrent)) {
			++_xPosCurrent;
			_characterFacingDirection = 1;
			_characterBackFrontFacing = true;
			flag = true;
		}
	}
	if (!flag) {
		if (_selectedObjectLocationMask) {
			_selectedObjectLocationMask = false;
			_selectedObject._xPos = _selectedObject._xDefaultPos;
			_selectedObject._yPos = _selectedObject._yDefaultPos;
		} else {
			_panelLockedFlag = false;
			_characterFacingDirection = 0;
			if (_xPosCurrent == _selectedObject._xPos && _yPosCurrent == _selectedObject._yPos) {
				_locationMaskCounter = 1;
			}
		}
	}
	if (_location == kLocationVentSystem) {
		if ((_backgroundSpriteCurrentAnimation != 3 || _characterBackFrontFacing) && (_backgroundSpriteCurrentAnimation != 6 || !_characterBackFrontFacing)) {
			_xPosCurrent = xPos;
			_yPosCurrent = yPos;
			return;
		}
	}
	if (_xPosCurrent != _selectedObject._xPos || _yPosCurrent != _selectedObject._yPos) {
		return;
	}
	if (_selectedObjectLocationMask) {
		return;
	}
	_locationMaskCounter = 1;
	_panelLockedFlag = false;
	_locationMaskIgnore = false;
	if (_characterPrevFacingDirection <= 0 || _characterPrevFacingDirection >= 5) {
		return;
	}
	if (_selectedObject._locationObjectLocation == kLocationNone) {
		_characterFacingDirection = 5;
		while (_spriteAnimationFramesTable[_spriteAnimationFrameIndex] != 999) {
			++_spriteAnimationFrameIndex;
		}
		++_spriteAnimationFrameIndex;
	}
}

void TuckerEngine::setSoundVolumeDistance() {
	int w = ABS(_xPosCurrent - _currentFxDist);
	int d = w * _currentFxScale / 10;
	int volume = (d > _currentFxVolume) ? 0 : _currentFxVolume - d;
	setVolumeSound(_currentFxIndex, volume);
}

void TuckerEngine::updateData3DrawFlag() {
	for (int i = 0; i < _locationAnimationsCount; ++i) {
		LocationAnimation *a = &_locationAnimationsTable[i];
		if (a->_flagNum > 0 && a->_flagValue != _flagsTable[a->_flagNum]) {
			a->_drawFlag = false;
		} else if (a->_getFlag == 0) {
			a->_drawFlag = true;
		} else {
			a->_drawFlag = (_inventoryItemsState[a->_inventoryNum] == 0);
		}
	}
}

void TuckerEngine::updateData3() {
	updateData3DrawFlag();
	for (int i = 0; i < _locationAnimationsCount; ++i) {
		LocationAnimation *a = &_locationAnimationsTable[i];
		if (a->_animLastCounter != 0 && a->_drawFlag) {
			if (a->_animLastCounter == a->_animCurrentCounter) {
				a->_animCurrentCounter = a->_animInitCounter;
			} else {
				++a->_animCurrentCounter;
			}
			const int index = a->_animCurrentCounter;
			if (_staticData3Table[index] == 998) {
				_flagsTable[_staticData3Table[index + 1]] = _staticData3Table[index + 2];
				a->_animCurrentCounter = a->_animInitCounter;
				a->_drawFlag = false;
			}
			if (_location == kLocationStoreRoom && i == 0) {
				// workaround bug #4642: update fish animation sequence for correct
				// position in aquarium.
				if (a->_animInitCounter == 505 && a->_animCurrentCounter == 513) {
					a->_animCurrentCounter = 525;
				}
			}
			a->_graphicNum = _staticData3Table[a->_animCurrentCounter];
		}
	}
	updateData3DrawFlag();
}

void TuckerEngine::updateSfxData3_1() {
	for (int i = 0; i < _locationSoundsCount; ++i) {
		LocationSound *s = &_locationSoundsTable[i];
		if ((s->_type == 6 || s->_type == 7) && s->_updateType == 1) {
			for (int j = 0; j < _spritesCount; ++j) {
				if (_spritesTable[j]._animationFrame == s->_startFxSpriteNum && _spritesTable[j]._state == s->_startFxSpriteState) {
					if (s->_type == 7) {
						_flagsTable[s->_flagNum] = s->_flagValueStartFx;
					}
					startSound(s->_offset, i, s->_volume);
				} else if (s->_type == 7) {
					if (_spritesTable[j]._animationFrame == s->_stopFxSpriteNum && _spritesTable[j]._state == s->_stopFxSpriteState) {
						_flagsTable[s->_flagNum] = s->_flagValueStopFx;
						stopSound(i);
					}
				}
			}
		}
	}
}

void TuckerEngine::updateSfxData3_2() {
	for (int i = 0; i < _locationSoundsCount; ++i) {
		LocationSound *s = &_locationSoundsTable[i];
		if ((s->_type == 6 || s->_type == 7) && s->_updateType == 0) {
			if (s->_startFxSpriteNum == _backgroundSpriteCurrentFrame && s->_startFxSpriteState == _backgroundSpriteCurrentAnimation) {
				if (s->_type == 7) {
					_flagsTable[s->_flagNum] = s->_flagValueStartFx;
				}
				startSound(s->_offset, i, s->_volume);
			} else if (s->_type == 7) {
				if (s->_stopFxSpriteNum == _backgroundSpriteCurrentFrame && s->_stopFxSpriteState == _backgroundSpriteCurrentAnimation) {
					_flagsTable[s->_flagNum] = s->_flagValueStopFx;
					stopSound(i);
				}
			}
		}
	}
}

void TuckerEngine::saveOrLoad() {
	bool hasSavegame = existsSavegame();

	if (!_leftMouseButtonPressed) {
		_mouseClick = 0;
	}
	if (_currentSaveLoadGameState > 0) {
		if (_saveOrLoadGamePanel == 0 && !hasSavegame) {
			drawSpeechText(_scrollOffset + 120, 170, _infoBarBuf, _saveOrLoadGamePanel + 21, 102);
		} else {
			drawSpeechText(_scrollOffset + 120, 170, _infoBarBuf, _saveOrLoadGamePanel + 19, 102);
			int len = getStringWidth(_saveOrLoadGamePanel + 19, _infoBarBuf);
			drawStringInteger(_currentSaveLoadGameState, len / 2 + 128, 160, 2);
		}
	} else {
		drawSpeechText(_scrollOffset + 120, 170, _infoBarBuf, 21, 102);
	}
	if (_mousePosY > 140) {
		if (_mouseWheelUp && _currentSaveLoadGameState < kLastSaveSlot) {
			++_currentSaveLoadGameState;
			_forceRedrawPanelItems = true;
			return;
		} else if (_mouseWheelDown && _currentSaveLoadGameState > 1) {
			--_currentSaveLoadGameState;
			_forceRedrawPanelItems = true;
			return;
		}
	}
	if (_leftMouseButtonPressed && _mouseClick == 0) {
		_mouseClick = 1;
		if (_mousePosX > 228 && _mousePosX < 240 && _mousePosY > 154 && _mousePosY < 170) {
			if (_currentSaveLoadGameState < kLastSaveSlot) {
				++_currentSaveLoadGameState;
				_forceRedrawPanelItems = true;
			}
			return;
		}
		if (_mousePosX > 228 && _mousePosX < 240 && _mousePosY > 170 && _mousePosY < 188) {
			if (_currentSaveLoadGameState > 1) {
				--_currentSaveLoadGameState;
				_forceRedrawPanelItems = true;
			}
			return;
		}
		if (_mousePosX > 244 && _mousePosX < 310 && _mousePosY > 170 && _mousePosY < 188) {
			_forceRedrawPanelItems = true;
			_panelType = kPanelTypeLoadSavePlayQuit;
			return;
		}
		if (_mousePosX > 260 && _mousePosX < 290 && _mousePosY > 152 && _mousePosY < 168) {
			if (_saveOrLoadGamePanel == 1) {
				saveGameState(_currentSaveLoadGameState, "");
			} else if (hasSavegame && _currentSaveLoadGameState > 0) {
				loadGameState(_currentSaveLoadGameState);
			}
			_forceRedrawPanelItems = true;
			_panelType = kPanelTypeNormal;
			setCursorState(kCursorStateNormal);
			return;
		}
	}
}

void TuckerEngine::handleMouseOnPanel() {
	if (!_leftMouseButtonPressed) {
		_mouseClick = 0;
	}
	if (_leftMouseButtonPressed && _mouseClick == 0) {
		_mouseClick = 1;
		if (_mousePosY < 160 || _mousePosY > 176) {
			return;
		}
		if (_mousePosX < 45 || _mousePosX > 275) {
			return;
		}
		if (_mousePosX < 96) {
			_saveOrLoadGamePanel = 0;
			_forceRedrawPanelItems = true;
			_panelType = kPanelTypeLoadSaveSavegame;
		} else if (_mousePosX < 158) {
			_saveOrLoadGamePanel = 1;
			_forceRedrawPanelItems = true;
			_panelType = kPanelTypeLoadSaveSavegame;
		} else if (_mousePosX < 218) {
			_forceRedrawPanelItems = true;
			_panelType = kPanelTypeNormal;
			setCursorState(kCursorStateNormal);
		} else {
			_quitGame = true;
		}
	}
}

void TuckerEngine::togglePanelStyle() {
	switch (_panelState) {
	case kPanelStateShrinking:
		if (++_switchPanelCounter == 25) {
			_panelStyle = (_panelStyle == kPanelStyleVerbs) ? kPanelStyleIcons : kPanelStyleVerbs;
			loadPanel();
			_forceRedrawPanelItems = true;
			_panelState = kPanelStateExpanding;
		}
		break;

	case kPanelStateExpanding:
		if (--_switchPanelCounter == 0) {
			_panelState = kPanelStateNormal;
		}
		break;

	default:
		break;
	}
}

void TuckerEngine::redrawPanelOverBackground() {
	const uint8 *src = _itemsGfxBuf;
	uint8 *dst = _locationBackgroundGfxBuf + 640 * 140 + _scrollOffset;
	for (int y = 0; y < 10; ++y) {
		memcpy(dst, src, 320);
		src += 320;
		dst += 640;
	}
	for (int y = 0; y < _switchPanelCounter; ++y) {
		for (int x = 0; x < 320; ++x) {
			dst[x] = 0;
		}
		dst += 640;
	}
	int y2 = 50 - _switchPanelCounter * 2;
	for (int y = 0; y < y2; ++y) {
		int i = y * 50 / y2;
		memcpy(dst, src + i * 320, 320);
		dst += 640;
	}
	for (int y = 0; y < _switchPanelCounter; ++y) {
		for (int x = 0; x < 320; ++x) {
			dst[x] = 0;
		}
		dst += 640;
	}
	if (_conversationOptionsCount > 0) {
		drawConversationTexts();
	}
	addDirtyRect(_scrollOffset, 140, 320, 60);
}

void TuckerEngine::drawConversationTexts() {
	int y = 141;
	bool flag = false;
	for (int i = 0; i <  _conversationOptionsCount; ++i) {
		int color = 108;
		if ((_mousePosY > y && _mousePosY < y + 11) || _nextTableToLoadIndex == i) {
			color = 106;
		}
		drawSpeechText(0, y, _characterSpeechDataPtr, _instructionsActionsTable[i], color);
		if (_mousePosY > y && _mousePosY < _conversationOptionLinesCount * 10 + y + 1) {
			_nextTableToLoadIndex = i;
			flag = true;
		}
		y += _conversationOptionLinesCount * 10;
	}
	if (!flag) {
		_nextTableToLoadIndex = -1;
	}
}

void TuckerEngine::updateScreenScrolling() {
	int scrollPrevOffset = _scrollOffset;
	if (_locationWidthTable[_location] != 2) {
		_scrollOffset = 0;
	} else if (_validInstructionId) {
		_scrollOffset = _xPosCurrent - 200;
	} else if (_location == kLocationPark && _backgroundSpriteCurrentAnimation == 6 && _scrollOffset + 200 < _xPosCurrent) {
		++_scrollOffset;
		if (_scrollOffset > 320) {
			_scrollOffset = 320;
		}
	} else if (_scrollOffset + 120 > _xPosCurrent) {
		_scrollOffset = _xPosCurrent - 120;
		if (_scrollOffset < 0) {
			_scrollOffset = 0;
		}
	} else if (_scrollOffset + 200 < _xPosCurrent) {
		_scrollOffset = _xPosCurrent - 200;
		if (_scrollOffset > 320) {
			_scrollOffset = 320;
		}
	}
	if (scrollPrevOffset != _scrollOffset) {
		_fullRedraw = true;
	}
}

void TuckerEngine::updateGameHints() {
	if (_gameHintsIndex == 0 && _flagsTable[3] > 0) {
		_gameHintsIndex = 1;
		_gameHintsCounter = 0;
		_displayHintsText = false;
	} else if (_gameHintsIndex == 1 && _flagsTable[12] > 0) {
		_gameHintsIndex = 2;
		_gameHintsCounter = 0;
		_displayHintsText = false;
	} else if (_gameHintsIndex == 2 && _flagsTable[20] > 0) {
		_gameHintsIndex = 3;
		_gameHintsCounter = 0;
		_displayHintsText = false;
	} else if (_gameHintsIndex == 3 && _flagsTable[9] > 0) {
		_gameHintsIndex = 4;
		_gameHintsCounter = 0;
		_displayHintsText = false;
	} else if (_gameHintsIndex == 4 && _flagsTable[23] > 0) {
		_gameHintsIndex = 5;
		_gameHintsCounter = 0;
		_displayHintsText = false;
	} else if (_flagsTable[19] > 0) {
		_gameHintsIndex = 6;
		_gameHintsCounter = 0;
		_displayHintsText = false;
	}
	++_gameHintsCounter;
	if (_gameHintsCounter > 1500) {
		_displayHintsText = true;
	}
}

void TuckerEngine::startCharacterSounds() {
	if (_characterSoundFxDelayCounter != 0) {
		--_characterSoundFxDelayCounter;
		if (_characterSoundFxDelayCounter <= 0) {
			startSound(_locationSoundsTable[_characterSoundFxNum]._offset, _characterSoundFxNum, _locationSoundsTable[_characterSoundFxNum]._volume);
		}
	}
}

void TuckerEngine::updateSoundsTypes3_4() {
	if (isSoundPlaying(0)) {
		return;
	}
	for (int i = 0; i < _locationSoundsCount; ++i) {
		switch (_locationSoundsTable[i]._type) {
		case 3:
			if (getRandomNumber() >= 32300) {
				startSound(_locationSoundsTable[i]._offset, 0, _locationSoundsTable[i]._volume);
				return;
			}
			break;
		case 4:
			if (getRandomNumber() >= 32763) {
				startSound(_locationSoundsTable[i]._offset, 0, _locationSoundsTable[i]._volume);
				return;
			}
			break;
		default:
			break;
		}
	}
}

void TuckerEngine::drawData3() {
	for (int i = 0; i < _locationAnimationsCount; ++i) {
		if (_locationAnimationsTable[i]._drawFlag) {
			int num = _locationAnimationsTable[i]._graphicNum;
			const Data *d = &_dataTable[num];
			Graphics::decodeRLE(_locationBackgroundGfxBuf + d->_yDest * 640 + d->_xDest, _data3GfxBuf + d->_sourceOffset, d->_xSize, d->_ySize);
			addDirtyRect(d->_xDest, d->_yDest, d->_xSize, d->_ySize);
		}
	}
}

void TuckerEngine::execData3PreUpdate() {
	switch (_location) {
	case 1:
		execData3PreUpdate_locationNum1();
		break;
	case 2:
		execData3PreUpdate_locationNum2();
		break;
	case 3:
		execData3PreUpdate_locationNum3();
		break;
	case 4:
		execData3PreUpdate_locationNum4();
		break;
	case 6:
		execData3PreUpdate_locationNum6();
		break;
	case 9:
		execData3PreUpdate_locationNum9();
		break;
	case 10:
		execData3PreUpdate_locationNum10();
		break;
	case 12:
		execData3PreUpdate_locationNum12();
		break;
	case 13:
		execData3PreUpdate_locationNum13();
		break;
	case 14:
		execData3PreUpdate_locationNum14();
		break;
	case 15:
		execData3PreUpdate_locationNum15();
		break;
	case 16:
		execData3PreUpdate_locationNum16();
		break;
	case 19:
		execData3PreUpdate_locationNum19();
		break;
	case 21:
		execData3PreUpdate_locationNum21();
		break;
	case 22:
		execData3PreUpdate_locationNum22();
		break;
	case 24:
		execData3PreUpdate_locationNum24();
		break;
	case 25:
		execData3PreUpdate_locationNum25();
		break;
	case 26:
		execData3PreUpdate_locationNum26();
		break;
	case 27:
		execData3PreUpdate_locationNum27();
		break;
	case 28:
		execData3PreUpdate_locationNum28();
		break;
	case 29:
		execData3PreUpdate_locationNum29();
		break;
	case 30:
		execData3PreUpdate_locationNum30();
		break;
	case 31:
		execData3PreUpdate_locationNum31();
		break;
	case 32:
		execData3PreUpdate_locationNum32();
		break;
	case 33:
		execData3PreUpdate_locationNum33();
		break;
	case 34:
		execData3PreUpdate_locationNum34();
		break;
	case 35:
		execData3PreUpdate_locationNum35();
		break;
	case 36:
		execData3PreUpdate_locationNum36();
		break;
	case 38:
		execData3PreUpdate_locationNum38();
		break;
	case 41:
		execData3PreUpdate_locationNum41();
		break;
	case 42:
		execData3PreUpdate_locationNum42();
		break;
	case 43:
		execData3PreUpdate_locationNum43();
		break;
	case 44:
		execData3PreUpdate_locationNum44();
		break;
	case 49:
		execData3PreUpdate_locationNum49();
		break;
	case 52:
		execData3PreUpdate_locationNum52();
		break;
	case 53:
		execData3PreUpdate_locationNum53();
		break;
	case 57:
		execData3PreUpdate_locationNum57();
		break;
	case 58:
		execData3PreUpdate_locationNum58();
		break;
	case 61:
		execData3PreUpdate_locationNum61();
		break;
	case 63:
		execData3PreUpdate_locationNum63();
		break;
	case 64:
		execData3PreUpdate_locationNum64();
		break;
	case 65:
		execData3PreUpdate_locationNum65();
		break;
	case 66:
		execData3PreUpdate_locationNum66();
		break;
	case 70:
		execData3PreUpdate_locationNum70();
		break;
	default:
		break;
	}
}

void TuckerEngine::execData3PostUpdate() {
	switch (_location) {
	case 1:
		execData3PostUpdate_locationNum1();
		break;
	case 6:
		execData3PostUpdate_locationNum6();
		break;
	case 8:
		execData3PostUpdate_locationNum8();
		break;
	case 9:
		execData3PostUpdate_locationNum9();
		break;
	case 14:
		execData3PostUpdate_locationNum14();
		break;
	case 21:
		execData3PostUpdate_locationNum21();
		break;
	case 24:
		execData3PostUpdate_locationNum24();
		break;
	case 27:
		execData3PostUpdate_locationNum27();
		break;
	case 28:
		execData3PostUpdate_locationNum28();
		break;
	case 32:
		execData3PostUpdate_locationNum32();
		break;
	case 60:
		execData3PostUpdate_locationNum60();
		break;
	case 66:
		execData3PostUpdate_locationNum66();
		break;
	default:
		break;
	}
}

void TuckerEngine::drawBackgroundSprites() {
	if (_backgroundSpriteDataPtr && _backgroundSpriteCurrentFrame != 0 && _backgroundSpriteCurrentFrame <= _backgroundSpriteLastFrame) {
		int frameOffset = READ_LE_UINT24(_backgroundSpriteDataPtr + _backgroundSpriteCurrentFrame * 4);
		int srcW = READ_LE_UINT16(_backgroundSpriteDataPtr + frameOffset);
		int srcH = READ_LE_UINT16(_backgroundSpriteDataPtr + frameOffset + 2);
		int srcX = READ_LE_UINT16(_backgroundSpriteDataPtr + frameOffset + 8);
		int srcY = READ_LE_UINT16(_backgroundSpriteDataPtr + frameOffset + 10);
		if (_location == kLocationFishingTrawler && _backgroundSpriteCurrentAnimation > 1) {
			srcY += _mainSpritesBaseOffset;
		}
		if (_location == kLocationSubmarineHangar && _backgroundSpriteCurrentAnimation == 3) {
			srcX += 228;
		} else if (_location == kLocationInsideMuseumPartThree && _backgroundSpriteCurrentAnimation == 1) {
			srcX += 100;
		} else if (_xPosCurrent > 320 && _xPosCurrent < 640) {
			srcX += 320;
		}
		srcX += _backgroundSprOffset;
		Graphics::decodeRLE_248(_locationBackgroundGfxBuf + srcY * 640 + srcX, _backgroundSpriteDataPtr + frameOffset + 12, srcW, srcH, 0, _locationHeightTable[_location], false);
		addDirtyRect(srcX, srcY, srcW, srcH);
	}
}

void TuckerEngine::drawCurrentSprite() {
	// WORKAROUND: original game glitch
	// Locations 48 and 61 contain reserved colors from [0xE0-0xF8] in a walkable area which
	// results in a number of pixels being falsely drawn in the foreground (on top of Bud).
	// Even worse, location 61 uses some of the same colors in places which actually _should_
	// be drawn in the foreground.
	// We whitelist these colors based on location number and, in case of location 61, also
	// based on Bud's location (pun not intended).
	// This fixes Trac#10423.
	const int *whitelistReservedColors = nullptr;
	//                                                      [0xE0, ...                            ..., 0xEF]
	static const int whitelistReservedColorsLocation48[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 };
	static const int whitelistReservedColorsLocation61[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 };
	switch (_location) {
	case kLocationCorridor:
		whitelistReservedColors = (const int *)&whitelistReservedColorsLocation48;
		break;

	case kLocationParkPartThree:
		if (_xPosCurrent <= 565)
			whitelistReservedColors = (const int *)&whitelistReservedColorsLocation61;
		break;

	default:
		break;
	}

	SpriteFrame *chr = &_spriteFramesTable[_currentSpriteAnimationFrame];
	int yPos = _yPosCurrent + _mainSpritesBaseOffset - 54 + chr->_yOffset;
	int xPos = _xPosCurrent;
	if (!_mirroredDrawing) {
		xPos += chr->_xOffset - 14;
	} else {
		xPos -= chr->_xSize + chr->_xOffset - 14;
	}
	Graphics::decodeRLE_248(_locationBackgroundGfxBuf + yPos * 640 + xPos, _spritesGfxBuf + chr->_sourceOffset, chr->_xSize, chr->_ySize,
		chr->_yOffset, _locationHeightTable[_location], _mirroredDrawing, whitelistReservedColors);
	addDirtyRect(xPos, yPos, chr->_xSize, chr->_ySize);
	if (_currentSpriteAnimationLength > 1) {
		SpriteFrame *chr2 = &_spriteFramesTable[_currentSpriteAnimationFrame2];
		yPos = _yPosCurrent + _mainSpritesBaseOffset - 54 + chr2->_yOffset;
		xPos = _xPosCurrent;
		if (!_mirroredDrawing) {
			xPos += chr2->_xOffset - 14;
		} else {
			xPos -= chr2->_xSize + chr2->_xOffset - 14;
		}
		Graphics::decodeRLE_248(_locationBackgroundGfxBuf + yPos * 640 + xPos, _spritesGfxBuf + chr2->_sourceOffset, chr2->_xSize, chr2->_ySize,
			chr2->_yOffset, _locationHeightTable[_location], _mirroredDrawing, whitelistReservedColors);
		addDirtyRect(xPos, yPos, chr2->_xSize, chr2->_ySize);
	}
}

void TuckerEngine::setVolumeSound(int index, int volume) {
	if (volume < 0) {
		volume = 0;
	}
	_mixer->setChannelVolume(_sfxHandles[index], volume * Audio::Mixer::kMaxChannelVolume / kMaxSoundVolume);
}

void TuckerEngine::setVolumeMusic(int index, int volume) {
	if (volume < 0) {
		volume = 0;
	}
	_mixer->setChannelVolume(_musicHandles[index], volume * Audio::Mixer::kMaxChannelVolume / kMaxSoundVolume);
}

void TuckerEngine::startSound(int offset, int index, int volume) {
	bool loop = (_locationSoundsTable[index]._type == 2 || _locationSoundsTable[index]._type == 5 || _locationSoundsTable[index]._type == 7);
	loadSound(Audio::Mixer::kSFXSoundType, _locationSoundsTable[index]._num, volume, loop, &_sfxHandles[index]);
}

void TuckerEngine::stopSound(int index) {
	_mixer->stopHandle(_sfxHandles[index]);
}

bool TuckerEngine::isSoundPlaying(int index) {
	return _mixer->isSoundHandleActive(_sfxHandles[index]);
}

void TuckerEngine::startMusic(int offset, int index, int volume) {
	bool loop = (_locationMusicsTable[index]._flag == 2);
	loadSound(Audio::Mixer::kMusicSoundType, _locationMusicsTable[index]._num, volume, loop, &_musicHandles[index]);
}

void TuckerEngine::stopMusic(int index) {
	_mixer->stopHandle(_musicHandles[index]);
}

void TuckerEngine::startSpeechSound(int num, int volume) {
	loadSound(Audio::Mixer::kSpeechSoundType, num, volume, false, &_speechHandle);
}

void TuckerEngine::stopSpeechSound() {
	_mixer->stopHandle(_speechHandle);
}

bool TuckerEngine::isSpeechSoundPlaying() {
	return _mixer->isSoundHandleActive(_speechHandle);
}

void TuckerEngine::rememberSpeechSound() {
	for (int i = 4; i > 0; --i) {
		_speechHistoryTable[i] = _speechHistoryTable[i - 1];
	}
	_speechHistoryTable[0] = _part * 3000 + _ptTextOffset + _speechSoundNum - 3000;
}

void TuckerEngine::redrawPanelItems() {
	if (_forceRedrawPanelItems || (_redrawPanelItemsCounter != 0 && _panelType == kPanelTypeNormal)) {
		_forceRedrawPanelItems = false;
		if (_redrawPanelItemsCounter > 0) {
			--_redrawPanelItemsCounter;
		}

		const uint8 *src = nullptr;
		uint8 *dst = nullptr;
		int sz = 0;

		switch (_panelType) {
		case kPanelTypeNormal:
			src = _panelGfxBuf;
			dst = _itemsGfxBuf + 3200;
			sz = 16000;
			break;
		case kPanelTypeEmpty:
			src = _panelGfxBuf + 16320;
			dst = _itemsGfxBuf;
			sz = 19200;
			break;
		case kPanelTypeLoadSavePlayQuit:
			// The following offset does not match disassembly on purpose to fix a
			// "glitch" in the original game.
			// This ensures that the background image ends up in the same place as
			// in the case of kPanelTypeLoadSaveSavegame.
			// This fixes Trac#10496.
			src = _panelGfxBuf + 16000;
			dst = _itemsGfxBuf;
			sz = 19200;
			memcpy(dst, src, sz);
			src = _panelGfxBuf + 55040;
			dst = _itemsGfxBuf + 6400;
			sz = 5120;
			break;
		case kPanelTypeLoadSaveSavegame:
			src = _panelGfxBuf + 35200;
			dst = _itemsGfxBuf;
			sz = 19200;
			break;
		default:
			break;
		}
		memcpy(dst, src, sz);
		if (_panelType == kPanelTypeNormal) {
			redrawPanelItemsHelper();
		}
	}
}

void TuckerEngine::redrawPanelItemsHelper() {
	const int k = (_redrawPanelItemsCounter / 4) - ((_redrawPanelItemsCounter / 8) * 2);
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (i * 3 + j + _inventoryObjectsOffset >= _inventoryObjectsCount) {
				continue;
			}
			if (i * 3 + j + _inventoryObjectsOffset == _lastInventoryObjectIndex && k != 0) {
				continue;
			}
			const int obj = _inventoryObjectsList[i * 3 + j + _inventoryObjectsOffset];
			const uint8 *src = _panelObjectsGfxBuf + _panelObjectsOffsetTable[obj];
			uint8 *dst = _itemsGfxBuf + 3412 + i * 8320 + j * 34;
			Graphics::decodeRLE_320(dst, src, 32, 24);
		}
	}
}

void TuckerEngine::drawSprite(int num) {
	Sprite *s = &_spritesTable[num];
	if (s->_animationFrame <= s->_firstFrame && s->_animationFrame > 0 && s->_state != -1) {
		const uint8 *p = s->_animationData;
		if (!p) {
			return;
		}
		int frameOffset = READ_LE_UINT24(p + s->_animationFrame * 4);
		int srcW = READ_LE_UINT16(p + frameOffset);
		int srcH = READ_LE_UINT16(p + frameOffset + 2);
		int srcX = READ_LE_UINT16(p + frameOffset + 8);
		int srcY = READ_LE_UINT16(p + frameOffset + 10);
		s->_gfxBackgroundOffset += s->_backgroundOffset;
		int xPos = s->_gfxBackgroundOffset + srcX;
		if (xPos < 600 && (_scrollOffset + 320 < xPos || _scrollOffset - srcW > xPos)) {
			return;
		}
		s->_xSource = srcX;
		uint8 *dstPtr = _locationBackgroundGfxBuf + srcY * 640 + xPos;
		const uint8 *srcPtr = p + frameOffset + 12;
		switch (s->_colorType) {
		case 0:
			Graphics::decodeRLE(dstPtr, srcPtr, srcW, srcH);
			break;
		case 99:
			Graphics::decodeRLE_224(dstPtr, srcPtr, srcW, srcH);
			break;
		default:
			Graphics::decodeRLE_248(dstPtr, srcPtr, srcW, srcH, 0, s->_yMaxBackground, s->_flipX);
			break;
		}
		const int xR = (srcX +  s->_gfxBackgroundOffset) % 640;
		const int yR =  srcY + (s->_gfxBackgroundOffset  / 640);
		addDirtyRect(xR, yR, srcW, srcH);
	}
}

void TuckerEngine::clearItemsGfx() {
	memset(_itemsGfxBuf, 0, 3200);
}

void TuckerEngine::drawPausedInfoBar() {
	const int len = getStringWidth(36, _infoBarBuf);
	const int x = (kScreenWidth / 2) - 1 - (len / 2);
	drawItemString(x, 36, _infoBarBuf);
}

const uint8 *TuckerEngine::getStringBuf(int type) const {
	const uint8 *p = nullptr;
	switch (type) {
	case 0:
		p = _data5Buf;
		break;
	case 1:
		p = _bgTextBuf;
		break;
	case 2:
		p = _charNameBuf;
		break;
	case 3:
		p = _objTxtBuf;
		break;
	default:
		break;
	}
	return p;
}

void TuckerEngine::drawInfoString() {
	const uint8 *obj1StrBuf = getStringBuf(_actionObj1Type);
	const uint8 *obj2StrBuf = getStringBuf(_actionObj2Type);
	int infoStringWidth = 0;
	int object1NameWidth = 0;
	int verbWidth = getStringWidth(_actionVerb + 1, _infoBarBuf);
	if (_actionObj1Num > 0 || _actionObj1Type > 0) {
		object1NameWidth = getStringWidth(_actionObj1Num + 1, obj1StrBuf) + 4;
		infoStringWidth = verbWidth + object1NameWidth;
	} else {
		infoStringWidth = verbWidth;
	}
	VerbPreposition verbPreposition = kVerbPrepositionNone;
	int verbPrepositionWidth = 0;
	if (_actionRequiresTwoObjects) {
		verbPreposition = (_actionVerb == kVerbGive) ? kVerbPrepositionTo : kVerbPrepositionWith;
		verbPrepositionWidth = getStringWidth(verbPreposition, _infoBarBuf) + 4;
		if (_gameLang != Common::EN_ANY && (_actionObj2Num > 0 || _actionObj2Type > 0) && verbPreposition != kVerbPrepositionNone) {
			infoStringWidth = 0;
			verbWidth = 0;
			object1NameWidth = 0;
		}
		infoStringWidth += verbPrepositionWidth;
		if (_actionObj2Num > 0 || _actionObj2Type > 0) {
			infoStringWidth += getStringWidth(_actionObj2Num + 1, obj2StrBuf);
		}
	}
	const int xPos = (kScreenWidth / 2) - 1 - (infoStringWidth / 2);
	if (_gameLang == Common::EN_ANY || (_actionObj2Num == 0 && _actionObj2Type == 0) || verbPreposition == kVerbPrepositionNone) {
		drawItemString(xPos, _actionVerb + 1, _infoBarBuf);
		if (_actionObj1Num > 0 || _actionObj1Type > 0) {
			drawItemString(xPos + 4 + verbWidth, _actionObj1Num + 1, obj1StrBuf);
		}
	}
	if (verbPreposition > 0) {
		drawItemString(xPos + 4 + verbWidth + object1NameWidth, verbPreposition, _infoBarBuf);
		if (_actionObj2Num > 0 || _actionObj2Type > 0) {
			drawItemString(xPos + 4 + verbWidth + object1NameWidth + verbPrepositionWidth, _actionObj2Num + 1, obj2StrBuf);
		}
	}
}

void TuckerEngine::drawGameHintString() {
	const int len = getStringWidth(_gameHintsStringNum + 29, _infoBarBuf);
	const int x = (kScreenWidth / 2) - 1 - (len / 2);
	drawItemString(x, _gameHintsStringNum + 29, _infoBarBuf);
}

void TuckerEngine::updateCharacterAnimation() {
	if (_characterAnimationIndex > -1) {
		if (_backgroundSpriteCurrentFrame == 0) {
			_backgroundSpriteCurrentAnimation = _characterAnimationsTable[_characterAnimationIndex];
			++_characterAnimationIndex;
			_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
			_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
			_backgroundSpriteCurrentFrame = _characterAnimationsTable[_characterAnimationIndex];
			++_characterAnimationIndex;
		} else if (_characterAnimationsTable[_characterAnimationIndex] == 99) {
			_characterAnimationIndex = -1;
			_backgroundSpriteCurrentAnimation = -1;
			if (_nextAction == 0) {
				setCursorState(kCursorStateNormal);
			}
		} else {
			_backgroundSpriteCurrentFrame = _characterAnimationsTable[_characterAnimationIndex];
			if (_noCharacterAnimationChange == 0) {
				++_characterAnimationIndex;
			}
		}
	} else if (_backgroundSpriteCurrentAnimation > -1) {
		while (_spriteAnimationFramesTable[_spriteAnimationFrameIndex] != 999) {
			++_spriteAnimationFrameIndex;
		}
		_characterFacingDirection = 0;
		if (_changeBackgroundSprite) {
			if (_backgroundSpriteCurrentFrame == 0) {
				_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
				_backgroundSpriteCurrentFrame = _backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
			} else {
				--_backgroundSpriteCurrentFrame;
				if (_backgroundSpriteCurrentFrame < 1) {
					_backgroundSpriteCurrentAnimation = -1;
					_backgroundSpriteCurrentFrame = 0;
					_changeBackgroundSprite = false;
					if (_nextAction == 0) {
						setCursorState(kCursorStateNormal);
					}
				}
			}
		} else {
			if (_backgroundSpriteCurrentFrame == 0) {
				_backgroundSpriteCurrentFrame = 1;
				assert(_backgroundSpriteCurrentAnimation >= 0 && _backgroundSpriteCurrentAnimation < kSprA02TableSize);
				_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
				_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
			} else if (_location == kLocationVentSystem && !_panelLockedFlag && (_backgroundSpriteCurrentAnimation == 3 || _backgroundSpriteCurrentAnimation == 6)) {
				_backgroundSpriteCurrentFrame = 0;
				_backgroundSpriteCurrentAnimation = -1;
			} else {
				++_backgroundSpriteCurrentFrame;
				if (_backgroundSpriteCurrentFrame > _backgroundSpriteLastFrame) {
					_backgroundSpriteCurrentAnimation = -1;
					_backgroundSpriteCurrentFrame = 0;
					if (_nextAction == 0 && _panelType == kPanelTypeNormal) {
						setCursorState(kCursorStateNormal);
					}
				}
			}
		}
	}
	if (_location == kLocationStoreRoom && _flagsTable[103] == 0) {
		if (_panelLockedFlag) {
			_panelLockedFlag = false;
			_selectedObject._locationObjectLocation = kLocationNone;
			if (_actionVerb != kVerbTalk) {
				_speechSoundNum = 2236;
				startSpeechSound(_speechSoundNum, _speechVolume);
				_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_speechSoundNum, _ptTextBuf);
				_speechSoundNum = 0;
				_actionPosX = _xPosCurrent;
				_actionPosY = _yPosCurrent - 64;
				_actionTextColor = 1;
				_actionCharacterNum = 99;
				setCursorState(kCursorStateDisabledHidden);
				_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
			}
		}
		if (_charSpeechSoundCounter == 0 || _actionCharacterNum != 99) {
			if (_backgroundSpriteCurrentAnimation == 5) {
				_backgroundSpriteCurrentFrame = 0;
			}
		} else {
			if (_backgroundSpriteCurrentAnimation != 5) {
				_backgroundSpriteCurrentFrame = 0;
			}
		}
		if (_backgroundSpriteCurrentFrame == 0) {
			if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 99) {
				_backgroundSpriteCurrentAnimation = 5;
			} else {
				_backgroundSpriteCurrentAnimation = (getRandomNumber() < 33000) ? 2 : 3;
			}
			_backgroundSpriteCurrentFrame = 1;
			_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
			_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
		}
	} else if (_location == kLocationVentSystem) {
		if (_backgroundSpriteCurrentFrame == 0) {
			if (!_characterBackFrontFacing) {
				if (_characterBackFrontFacing != _characterPrevBackFrontFacing) {
					_backgroundSpriteCurrentAnimation = 10;
				} else if (_panelLockedFlag) {
					_backgroundSpriteCurrentAnimation = 3;
				} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 99) {
					_backgroundSpriteCurrentAnimation = 8;
				} else {
					_backgroundSpriteCurrentAnimation = (getRandomNumber() < 32000) ? 11 : 5;
				}
			} else {
				if (_characterBackFrontFacing != _characterPrevBackFrontFacing) {
					_backgroundSpriteCurrentAnimation = 2;
				} else if (_panelLockedFlag) {
					_backgroundSpriteCurrentAnimation = 6;
				} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 99) {
					_backgroundSpriteCurrentAnimation = 9;
				} else {
					_backgroundSpriteCurrentAnimation = (getRandomNumber() < 32000) ? 12 : 7;
				}
			}
			_characterPrevBackFrontFacing = _characterBackFrontFacing;
			_backgroundSpriteCurrentFrame = 1;
			_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
			_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
		}
		_backgroundSprOffset = _xPosCurrent - 160;
	} else if (_location == kLocationTV && _backgroundSpriteCurrentFrame == 0) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 99) {
			_backgroundSpriteCurrentAnimation = 1;
		} else {
			_backgroundSpriteCurrentAnimation = (getRandomNumber() < 32000) ? 3 : 2;
		}
		_backgroundSpriteCurrentFrame = 1;
		_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
		_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
	}
	int frame = _spriteAnimationFramesTable[_spriteAnimationFrameIndex];
	if (!_panelLockedFlag && _characterFacingDirection < 5 && _selectedObject._locationObjectLocation == kLocationNone) {
		_characterFacingDirection = 0;
	}
	if (_charSpeechSoundCounter > 0 && _characterFacingDirection != 6 && _actionCharacterNum == 99) {
		_characterFacingDirection = 6;
		frame = 999;
	} else if (_characterFacingDirection == 6 && (_charSpeechSoundCounter == 0 || _actionCharacterNum != 99)) {
		_characterFacingDirection = 0;
		frame = 999;
	}
	int num = 0;
	if (frame == 999 || (_characterFacingDirection != _characterPrevFacingDirection && _characterFacingDirection < 5)) {
		_mirroredDrawing = false;
		if (_characterFacingDirection == 6) {
			if (_csDataHandled) {
				switch (_selectedCharacterDirection) {
				case 1:
					num = 17;
					break;
				case 2:
					num = 16;
					break;
				case 4:
					num = 15;
					break;
				default:
					num = 16;
					_mirroredDrawing = true;
					break;
				}
			} else {
				num = 15;
			}
		}
		if (_characterFacingDirection == 5) {
			_characterFacingDirection = 0;
		}
		if (_characterFacingDirection == 0) {
			if (_csDataHandled) {
				_mirroredDrawing = false;
				switch (_selectedCharacterDirection) {
				case 1:
					num = 3;
					break;
				case 2:
					num = 1;
					break;
				case 3:
					num = 1;
					_mirroredDrawing = true;
					break;
				default:
					num = 5;
					break;
				}
			} else if (getRandomNumber() < 2000) {
				num = 13;
			} else if (getRandomNumber() < 3000) {
				num = 14;
				if (_location == kLocationFishShopPartThree) {
					num = 18;
				}
			} else {
				num = (getRandomNumber() < 20000) ? 18 : 6;
			}
		} else {
			switch (_characterFacingDirection) {
			case 1:
				num = 0;
				break;
			case 2:
				num = 4;
				break;
			case 3:
				num = 0;
				_mirroredDrawing = true;
				break;
			case 4:
				num = 2;
				break;
			default:
				break;
			}
		}
		_currentSpriteAnimationLength = _spriteAnimationsTable[num]._numParts;
		_spriteAnimationFrameIndex = _spriteAnimationsTable[num]._firstFrameIndex;
		frame = _spriteAnimationFramesTable[_spriteAnimationFrameIndex];
	}
	if (_characterAnimationNum > 0) {
		num = _characterAnimationNum;
		_currentSpriteAnimationLength = _spriteAnimationsTable[num]._numParts;
		_spriteAnimationFrameIndex = _spriteAnimationsTable[num]._firstFrameIndex;
		frame = _spriteAnimationFramesTable[_spriteAnimationFrameIndex];
		_characterAnimationNum = 0;
	}
	_currentSpriteAnimationFrame = frame;
	++_spriteAnimationFrameIndex;
	if (_currentSpriteAnimationLength > 1) {
		_currentSpriteAnimationFrame2 = _spriteAnimationFramesTable[_spriteAnimationFrameIndex];
		++_spriteAnimationFrameIndex;
		if (_characterSpriteAnimationFrameCounter > 0) {
			++_characterSpriteAnimationFrameCounter;
			if (_characterSpriteAnimationFrameCounter > 121) {
				_characterSpriteAnimationFrameCounter = 0;
			}
			if (_selectedCharacterDirection == 1) {
				_currentSpriteAnimationFrame = (_characterSpriteAnimationFrameCounter > 2 && _characterSpriteAnimationFrameCounter < 120) ? 122 : 121;
			} else {
				_currentSpriteAnimationFrame = (_characterSpriteAnimationFrameCounter > 2 && _characterSpriteAnimationFrameCounter < 120) ? 120 : 119;
			}
		}
	}
	_characterPrevFacingDirection = _characterFacingDirection;
}

void TuckerEngine::addObjectToInventory(int num) {
	_inventoryObjectsList[_inventoryObjectsCount] = num;
	_lastInventoryObjectIndex = _inventoryObjectsCount;
	_redrawPanelItemsCounter = 50;
	++_inventoryObjectsCount;
	_inventoryItemsState[num] = 1;
	if (_inventoryObjectsOffset + 5 < _lastInventoryObjectIndex) {
		_inventoryObjectsOffset += 3;
	}
}

void TuckerEngine::removeObjectFromInventory(int num) {
	for (int i = 0; i < _inventoryObjectsCount; ++i) {
		if (_inventoryObjectsList[i] == num) {
			--_inventoryObjectsCount;
			_inventoryItemsState[num] = 2;
			const int count = _inventoryObjectsCount - i;
			if (count != 0) {
				memmove(_inventoryObjectsList + i, _inventoryObjectsList + i + 1, count * sizeof(int));
			}
			break;
		}
	}
}

void TuckerEngine::handleMap() {
	if (_handleMapCounter > 0) {
		++_handleMapCounter;
		if (_handleMapCounter > 19) {
			_handleMapCounter = 0;
			_locationMaskCounter = 1;
			_panelLockedFlag = false;
		}
	}
	if (!_panelLockedFlag && (_backgroundSpriteCurrentAnimation == -1 || _location == kLocationVentSystem) && _locationMaskType == 3) {
		setCursorState(kCursorStateNormal);
		if (_locationMaskCounter == 1) {
			_characterFacingDirection = 0;
			_locationMaskType = 0;
		}
		return;
	}
	if (_selectedObject._locationObjectLocation != kLocationNone && _locationMaskCounter != 0 && (_backgroundSpriteCurrentAnimation <= -1 || _location == kLocationVentSystem)) {
		// TODO
		// This is actually "_locationNum != 25" in disassembly. Is this a typo?
		if (_location == kLocationVentSystem || _backgroundSpriteCurrentAnimation != 4) {
			if (_locationMaskType == 0) {
				_locationMaskType = 1;
				setCursorState(kCursorStateDisabledHidden);
				if (_selectedObject._locationObjectToWalkX2 > 800) {
					_backgroundSpriteCurrentAnimation = _selectedObject._locationObjectToWalkX2 - 900;
					if (_selectedObject._locationObjectToWalkY2 > 499) {
						_changeBackgroundSprite = true;
						_backgroundSprOffset = _selectedObject._locationObjectToWalkY2 - 500;
					} else {
						_backgroundSprOffset = _selectedObject._locationObjectToWalkY2;
						_changeBackgroundSprite = false;
					}
					_backgroundSpriteCurrentFrame = 0;
					_mirroredDrawing = false;
					if (_location == kLocationVentSystem) {
						_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
						_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
						_backgroundSpriteCurrentFrame = 1;
					}
				} else {
					_locationMaskCounter = 0;
					_selectedObject._xPos = _selectedObject._locationObjectToWalkX2;
					_selectedObject._yPos = _selectedObject._locationObjectToWalkY2;
					_handleMapCounter = 1;
					_panelLockedFlag = true;
				}
				return;
			}
			_locationMaskType = 2;
			_panelType = kPanelTypeNormal;
			setCursorState(kCursorStateNormal);
			if (_selectedObject._locationObjectLocation == kLocationMap) {
				_noPositionChangeAfterMap = true;
				handleMapSequence();
				return;
			}
			for (int i = 0; i < 14; ++i) {
				fadeInPalette();
				redrawScreen(_scrollOffset);
				_fadePaletteCounter = 34;
			}
			_nextLocation = _selectedObject._locationObjectLocation;
			_xPosCurrent = _selectedObject._locationObjectToX;
			_yPosCurrent = _selectedObject._locationObjectToY;
			if (_selectedObject._locationObjectToX2 > 800) {
				_backgroundSpriteCurrentAnimation = _selectedObject._locationObjectToX2 - 900;
				if (_selectedObject._locationObjectToY2 > 499) {
					_changeBackgroundSprite = true;
					_backgroundSprOffset = _selectedObject._locationObjectToY2 - 500;
				} else {
					_changeBackgroundSprite = false;
					_backgroundSprOffset = _selectedObject._locationObjectToY2;
				}
				_backgroundSpriteCurrentFrame = 0;
			} else {
				_selectedObject._xPos = _selectedObject._locationObjectToX2;
				_selectedObject._yPos = _selectedObject._locationObjectToY2;
				_panelLockedFlag = true;
			}
			_scrollOffset = 0;
			_handleMapCounter = 0;
			_locationMaskCounter = 0;
			_selectedObject._locationObjectLocation = kLocationNone;
		}
	}
}

void TuckerEngine::clearSprites() {
	memset(_spritesTable, 0, sizeof(_spritesTable));
	for (int i = 0; i < kMaxCharacters; ++i) {
		_spritesTable[i]._state = -1;
		_spritesTable[i]._stateIndex = -1;
	}
}

void TuckerEngine::updateSprites() {
	const int count = (_location == kLocationMall) ? 3 : _spritesCount;
	for (int i = 0; i < count; ++i) {
		if (_spritesTable[i]._stateIndex > -1) {
			++_spritesTable[i]._stateIndex;
			if (_characterStateTable[_spritesTable[i]._stateIndex] == 99) {
				_spritesTable[i]._stateIndex = -1;
				_spritesTable[i]._state = -1;
				updateSprite(i);
			} else {
				_spritesTable[i]._animationFrame = _characterStateTable[_spritesTable[i]._stateIndex];
			}
			continue;
		}
		if (_spritesTable[i]._state == -1) {
			updateSprite(i);
			continue;
		}
		if (_charSpeechSoundCounter > 0 && i == _actionCharacterNum && !_spritesTable[i]._needUpdate) {
			updateSprite(i);
			continue;
		}
		if (_charSpeechSoundCounter == 0 && _spritesTable[i]._needUpdate) {
			updateSprite(i);
			continue;
		}
		if (_spritesTable[i]._updateDelay > 0) {
			--_spritesTable[i]._updateDelay;
			if (_spritesTable[i]._updateDelay == 0) {
				updateSprite(i);
			}
			continue;
		}
		if (_spritesTable[i]._defaultUpdateDelay > 0) {
			_spritesTable[i]._updateDelay = _spritesTable[i]._defaultUpdateDelay - 1;
			++_spritesTable[i]._animationFrame;
			if (_spritesTable[i]._animationFrame == _spritesTable[i]._firstFrame) {
				updateSprite(i);
			}
			continue;
		}
		if (!_spritesTable[i]._nextAnimationFrame) {
			++_spritesTable[i]._animationFrame;
			if (_spritesTable[i]._firstFrame - 1 < _spritesTable[i]._animationFrame) {
				if (_spritesTable[i]._prevAnimationFrame) {
					--_spritesTable[i]._animationFrame;
					_spritesTable[i]._nextAnimationFrame = true;
				} else {
					updateSprite(i);
				}
			}
			continue;
		}
		--_spritesTable[i]._animationFrame;
		if (_spritesTable[i]._animationFrame == 0) {
			updateSprite(i);
		}
	}
}

void TuckerEngine::updateSprite(int i) {
	_spritesTable[i]._prevState = _spritesTable[i]._state;
	_spritesTable[i]._prevAnimationFrame = false;
	_spritesTable[i]._nextAnimationFrame = false;
	_updateSpriteFlag1 = false;
	_updateSpriteFlag2 = false;
	_spritesTable[i]._defaultUpdateDelay = 0;
	_spritesTable[i]._updateDelay = 0;
	switch (_location) {
	case 2:
		updateSprite_locationNum2();
		break;
	case 3:
		if (i == 0) {
			updateSprite_locationNum3_0(i);
		} else if (i == 1) {
			updateSprite_locationNum3_1(i);
		} else {
			updateSprite_locationNum3_2(i);
		}
		break;
	case 4:
		updateSprite_locationNum4(0);
		break;
	case 5:
		if (i == 0) {
			updateSprite_locationNum5_0();
		} else {
			updateSprite_locationNum5_1(1);
		}
		break;
	case 6:
		if (i == 0) {
			updateSprite_locationNum6_0(0);
		} else if (i == 1) {
			updateSprite_locationNum6_1(1);
		} else {
			updateSprite_locationNum6_2(2);
		}
		break;
	case 7:
		if (i == 0) {
			updateSprite_locationNum7_0(0);
		} else {
			updateSprite_locationNum7_1(1);
		}
		break;
	case 8:
		if (i == 0) {
			updateSprite_locationNum8_0(0);
		} else {
			updateSprite_locationNum8_1(1);
		}
		break;
	case 9:
		if (i == 0) {
			updateSprite_locationNum9_0(i);
		} else if (i == 1) {
			updateSprite_locationNum9_1(i);
		} else {
			updateSprite_locationNum9_2(i);
		}
		break;
	case 10:
		updateSprite_locationNum10();
		break;
	case 11:
		if (i == 0) {
			updateSprite_locationNum11_0(0);
		} else if (i == 1) {
			updateSprite_locationNum11_1(1);
		} else if (i == 2) {
			updateSprite_locationNum11_2(2);
		} else if (i == 3) {
			updateSprite_locationNum11_3(3);
		} else if (i == 4) {
			updateSprite_locationNum11_4(4);
		}
		break;
	case 12:
		if (i == 0) {
			updateSprite_locationNum12_0(0);
		} else if (i == 1) {
			updateSprite_locationNum12_1(1);
		}
		break;
	case 13:
		updateSprite_locationNum13(0);
		break;
	case 14:
		updateSprite_locationNum14(0);
		break;
	case 15:
		if (i == 1) {
			updateSprite_locationNum15_1(1);
		} else if (i == 2) {
			updateSprite_locationNum15_2(2);
		} else if (i == 0) {
			updateSprite_locationNum15_0(0);
		}
		break;
	case 16:
		if (i == 0) {
			updateSprite_locationNum16_0(0);
		} else if (i == 1) {
			updateSprite_locationNum16_1(1);
		} else {
			updateSprite_locationNum16_2(2);
		}
		break;
	case 17:
		updateSprite_locationNum17();
		break;
	case 18:
		updateSprite_locationNum18();
		break;
	case 19:
		if (i == 0) {
			updateSprite_locationNum19_0(0);
		} else if (i == 1) {
			updateSprite_locationNum19_1(1);
		} else if (i == 2) {
			updateSprite_locationNum19_2(2);
		} else {
			updateSprite_locationNum19_3(3);
		}
		break;
	case 21:
		updateSprite_locationNum21();
		break;
	case 22:
		updateSprite_locationNum22();
		break;
	case 23:
		if (i == 0) {
			updateSprite_locationNum23_0(0);
		} else if (i == 1) {
			updateSprite_locationNum23_1(1);
		} else if (i == 2) {
			updateSprite_locationNum23_2(2);
		} else {
			updateSprite_locationNum23_3(3);
		}
		break;
	case 24:
		if (i == 0) {
			updateSprite_locationNum24_0(0);
		} else if (i == 1) {
			updateSprite_locationNum24_1(1);
		} else if (i == 2) {
			updateSprite_locationNum24_2(2);
		} else {
			updateSprite_locationNum24_3(3);
		}
		break;
	case 26:
		if (i == 0) {
			updateSprite_locationNum26_0(0);
		} else {
			updateSprite_locationNum26_1(1);
		}
		break;
	case 27:
		updateSprite_locationNum27(0);
		break;
	case 28:
		if (i == 0) {
			updateSprite_locationNum28_0(0);
		} else if (i == 1) {
			updateSprite_locationNum28_1(1);
		} else {
			updateSprite_locationNum28_2(2);
		}
		break;
	case 29:
		if (i == 0) {
			updateSprite_locationNum29_0(0);
		} else if (i == 1) {
			updateSprite_locationNum29_1(1);
		} else {
			updateSprite_locationNum29_2(2);
		}
		break;
	case 30:
		updateSprite_locationNum30_34(i);
		break;
	case 31:
		if (i == 0) {
			updateSprite_locationNum31_0(0);
		} else {
			updateSprite_locationNum31_1(1);
		}
		break;
	case 32:
		if (i == 0) {
			updateSprite_locationNum32_0(0);
		} else {
			_spritesTable[i]._state = -1;
		}
		break;
	case 33:
		if (i == 1) {
			updateSprite_locationNum33_1(1);
		} else if (i == 0) {
			updateSprite_locationNum33_0(0);
		} else if (i == 2) {
			updateSprite_locationNum33_2(2);
		} else {
			_spritesTable[i]._state = 12;
		}
		break;
	case 34:
		updateSprite_locationNum30_34(0);
		break;
	case 36:
		updateSprite_locationNum36(0);
		break;
	case 37:
		if (i == 0) {
			_spritesTable[0]._state = -1;
		} else {
			updateSprite_locationNum37(i);
		}
		break;
	case 41:
		updateSprite_locationNum41(i);
		break;
	case 42:
		updateSprite_locationNum42(i);
		break;
	case 43:
		if (i == 2) {
			updateSprite_locationNum43_2(i);
		} else if (i < 2) {
			if (_flagsTable[236] < 4) {
				_spritesTable[i]._state = i + 1;
			} else {
				_spritesTable[i]._state = -1;
			}
		} else if (i == 3) {
			updateSprite_locationNum43_3(3);
		} else if (i == 4) {
			updateSprite_locationNum43_4(4);
		} else if (i == 5) {
			updateSprite_locationNum43_5(5);
		} else {
			updateSprite_locationNum43_6(6);
		}
		break;
	case 45:
		_spritesTable[0]._state = 1;
		break;
	case 47:
		_spritesTable[i]._state = i + 1;
		break;
	case 48:
		updateSprite_locationNum48(0);
		break;
	case 49:
		updateSprite_locationNum49(0);
		break;
	case 50:
		if (i < 6) {
			updateSprite_locationNum50(i);
		} else {
			_spritesTable[i]._state = i + 1;
		}
		break;
	case 51:
		updateSprite_locationNum51(i);
		break;
	case 53:
		if (i == 0) {
			updateSprite_locationNum53_0(0);
		} else if (i == 1) {
			updateSprite_locationNum53_1(1);
		}
		break;
	case 54:
		updateSprite_locationNum54(0);
		break;
	case 55:
		updateSprite_locationNum55(0);
		break;
	case 56:
		updateSprite_locationNum56(0);
		break;
	case 57:
		if (i == 0) {
			updateSprite_locationNum57_0(0);
		} else if (i == 1) {
			updateSprite_locationNum57_1(1);
		}
		break;
	case 58:
		updateSprite_locationNum58(0);
		break;
	case 59:
		updateSprite_locationNum59(0);
		break;
	case 60:
		if (i == 0) {
			updateSprite_locationNum60_0(0);
		} else {
			updateSprite_locationNum60_1(1);
		}
		break;
	case 61:
		if (i == 0) {
			updateSprite_locationNum61_0(0);
		} else if (i == 2) {
			updateSprite_locationNum61_2(2);
		} else {
			updateSprite_locationNum61_1(1);
		}
		break;
	case 63:
		if (i == 0) {
			updateSprite_locationNum63_0(0);
		} else if (i == 1) {
			updateSprite_locationNum63_1(1);
		} else if (i == 2) {
			updateSprite_locationNum63_2(2);
		} else if (i == 3) {
			updateSprite_locationNum63_3(3);
		} else if (i == 4) {
			updateSprite_locationNum63_4(4);
		}
		break;
	case 65:
		updateSprite_locationNum65(0);
		break;
	case 66:
		if (i == 0) {
			updateSprite_locationNum66_0(0);
		} else if (i == 1) {
			updateSprite_locationNum66_1(1);
		} else if (i == 2) {
			updateSprite_locationNum66_2(2);
		} else if (i == 3) {
			updateSprite_locationNum66_3(3);
		} else {
			updateSprite_locationNum66_4(4);
		}
		break;
	case 69:
		if (i == 0) {
			_spritesTable[0]._state = 1;
		} else if (i == 1) {
			updateSprite_locationNum69_1(1);
		} else if (i == 2) {
			updateSprite_locationNum69_2(2);
		} else if (i == 3) {
			updateSprite_locationNum69_3(3);
		}
		break;
	case 71:
		updateSprite_locationNum71(0);
		break;
	case 72:
		updateSprite_locationNum72(0);
		break;
	case 74:
		updateSprite_locationNum74(i);
		break;
	case 79:
		updateSprite_locationNum79(0);
		break;
	case 81:
		if (i == 0) {
			updateSprite_locationNum81_0(0);
		} else if (i == 1) {
			updateSprite_locationNum81_1(1);
		}
		break;
	case 82:
		updateSprite_locationNum82(0);
		break;
	case 98:
		_spritesTable[0]._state = 1;
		break;
	default:
		break;
	}
	if (_spritesTable[i]._stateIndex <= -1) {
		if (!_updateSpriteFlag1) {
			_spritesTable[i]._animationFrame = 1;
		}
		if (_spritesTable[i]._state < 0 || !_sprC02Table[_spritesTable[i]._state]) {
			// WORKAROUND
			// The original game unconditionally reads into _sprC02Table[] below which
			// results in out-of-bounds reads when _spritesTable[i]._state == -1.
			// We reset the sprite's animation data in this case so sprite updates
			// are triggered correctly. This most prominently fixes a bug where Lola's
			// transition from dancing -> sitting happens too late.
			// This fixes Trac#6644.
			_spritesTable[i]._animationData = nullptr;
			_spritesTable[i]._firstFrame = 0;
			return;
		}
		_spritesTable[i]._animationData = _sprC02Table[_spritesTable[i]._state];
		_spritesTable[i]._firstFrame = READ_LE_UINT16(_spritesTable[i]._animationData);
		if (_updateSpriteFlag2) {
			_spritesTable[i]._state = _spritesTable[i]._firstFrame;
			_spritesTable[i]._nextAnimationFrame = true;
			_spritesTable[i]._prevAnimationFrame = true;
		}
	}
}

void TuckerEngine::drawStringInteger(int num, int x, int y, int digits) {
	const int xStart = x;
	char numStr[4];
	assert(num < 1000);
	Common::sprintf_s(numStr, "%03d", num);
	int i = (digits > 2) ? 0 : 1;
	for (; i < 3; ++i) {
		Graphics::drawStringChar(_locationBackgroundGfxBuf, _scrollOffset + x, y, 640, numStr[i], 102, _charsetGfxBuf);
		x += 8;
	}
	addDirtyRect(_scrollOffset + xStart, y, Graphics::_charset._charW * 3, Graphics::_charset._charH);
}

void TuckerEngine::drawStringAlt(int x, int y, int color, const uint8 *str, int strLen) {
	const int xStart = x;
	int pos = 0;
	while (pos != strLen && str[pos] != '\n') {
		const uint8 chr = str[pos];
		Graphics::drawStringChar(_locationBackgroundGfxBuf, x, y, 640, chr, color, _charsetGfxBuf);
		x += _charWidthTable[chr];
		++pos;
	}
	addDirtyRect(xStart, y, x - xStart, Graphics::_charset._charH);
}

void TuckerEngine::drawItemString(int x, int num, const uint8 *str) {
	int pos = getPositionForLine(num, str);
	while (str[pos] != '\n') {
		const uint8 chr = str[pos];
		// Different versions of the game use different character set dimensions (charset.pcx).
		// The default (English) set uses a height of 8 pixels whereas others use 10 pixels.
		// This needs to be taken into consideration when drawing the language bar so text
		// gets vertically centered in all languages.
		Graphics::drawStringChar(_itemsGfxBuf, x, (10 - (Graphics::_charset._charH)) / 2, 320, chr, 1, _charsetGfxBuf);
		x += _charWidthTable[chr];
		++pos;
	}
}

void TuckerEngine::drawCreditsString(int x, int y, int num) {
	int pos = getPositionForLine(num, _ptTextBuf);
	while (_ptTextBuf[pos] != '\n') {
		const uint8 chr = _ptTextBuf[pos];
		Graphics::drawStringChar(_locationBackgroundGfxBuf, x, y, 640, chr, 1, _charsetGfxBuf);
		x += _charWidthTable[chr];
		++pos;
	}
}

void TuckerEngine::updateCharSpeechSound(bool displayText) {
	if (_charSpeechSoundCounter == 0) {
		return;
	}
	if (_displaySpeechText) {
		_charSpeechSoundCounter = 0;
	} else {
		--_charSpeechSoundCounter;
	}
	if (_charSpeechSoundCounter == 0) {
		_charSpeechSoundCounter = isSpeechSoundPlaying() ? 1 : 0;
		if (_charSpeechSoundCounter == 0) {
			_characterSpriteAnimationFrameCounter = 0;
		}
	}
	if (_charSpeechSoundCounter == 0 && !_csDataHandled) {
		setCursorState(kCursorStateNormal);
	} else if (displayText) {
		drawSpeechText(_actionPosX, _actionPosY, _characterSpeechDataPtr, _speechSoundNum, _actionTextColor);
	}
}

void TuckerEngine::updateItemsGfxColors(int color1, int color128) {
	for (int i = 0; i < 3200; ++i) {
		if (_itemsGfxBuf[i] == 1) {
			_itemsGfxBuf[i] = color1;
		} else if (_itemsGfxBuf[i] == 128) {
			_itemsGfxBuf[i] = color128;
		}
	}
}

bool TuckerEngine::testLocationMask(int x, int y) {
	if (_locationMaskType > 0 || _locationMaskIgnore) {
		return true;
	}
	if (_location == kLocationSubwayTunnel || _location == kLocationKitchen) {
		y -= 3;
	}
	const int offset = y * 640 + x;
	return (_locationBackgroundMaskBuf[offset] > 0);
}

int TuckerEngine::getStringWidth(int num, const uint8 *ptr) {
	int w = 0;
	int pos = getPositionForLine(num, ptr);
	uint8 chr;
	while ((chr = ptr[pos]) != '\n') {
		w += _charWidthTable[chr];
		++pos;
	}
	return w;
}

int TuckerEngine::getPositionForLine(int num, const uint8 *ptr) {
	int linesCount = 0;
	int i = 0;
	while (linesCount < num) {
		if (ptr[i] == '\n') {
			++linesCount;
			if (ptr[i + 1] == '\r') {
				++i;
			}
		}
		++i;
	}
	while (1) {
		if (ptr[i] != '\n' && ptr[i] != '\r') {
			break;
		}
		++i;
	}
	return i;
}

void TuckerEngine::resetCharacterAnimationIndex(int count) {
	_backgroundSpriteCurrentFrame = 0;
	_characterAnimationIndex = 0;
	for (int i = 0; i < count; ++i) {
		while (_characterAnimationsTable[_characterAnimationIndex] != 99) {
			++_characterAnimationIndex;
		}
		++_characterAnimationIndex;
	}
}

enum TableInstructionCode {
	kCode_invalid,
	kCode_pan,
	kCode_bua,
	kCode_bub,
	kCode_buc,
	kCode_bsd,
	kCode_bof,
	kCode_buh,
	kCode_bon,
	kCode_bso,
	kCode_bus,
	kCode_buw,
	kCode_bux,
	kCode_c0a,
	kCode_c0c,
	kCode_c0s,
	kCode_end,
	kCode_fad,
	kCode_fw,
	kCode_flx,
	kCode_fxx,
	kCode_fx,
	kCode_gfg,
	kCode_gv,
	kCode_loc,
	kCode_mof,
	kCode_opt,
	kCode_opf,
	kCode_ofg,
	kCode_snc,
	kCode_sse,
	kCode_ssp,
	kCode_s0p,
	kCode_sp,
	kCode_tpo,
	kCode_wa_,
	kCode_wsm,
	kCode_wat,
	kCode_was,
	kCode_wfx,
	kCode_xhr,
	kCode_xhm,
	kCode_no3 // NOOP, throw away 3-byte parameter
};

static const struct {
	const char *name;
	int code;
} _instructions[] = {
	{ "pan", kCode_pan },
	{ "bua", kCode_bua },
	{ "bub", kCode_bub },
	{ "buc", kCode_buc },
	{ "bsd", kCode_bsd },
	{ "bcd", kCode_bsd }, // only ref 6.27
	{ "bud", kCode_bsd }, // only ref 13.3
	{ "bof", kCode_bof },
	{ "buh", kCode_buh },
	{ "bon", kCode_bon },
	{ "bso", kCode_bso },
	{ "bus", kCode_bus },
	{ "b0s", kCode_bus }, // only ref 65.25
	{ "buv", kCode_no3 },
	{ "buw", kCode_buw },
	{ "bdx", kCode_bux },
	{ "bux", kCode_bux },
	{ "c0a", kCode_c0a },
	{ "c0c", kCode_c0c },
	{ "c0s", kCode_c0s },
	{ "c0v", kCode_no3 },
	{ "end", kCode_end },
	{ "fad", kCode_fad },
	{ "fw",  kCode_fw  },
	{ "flx", kCode_flx },
	{ "fxx", kCode_fxx },
	{ "fx",  kCode_fx  },
	{ "gfg", kCode_gfg },
	{ "gv",  kCode_gv  },
	{ "loc", kCode_loc },
	{ "mof", kCode_mof },
	{ "opt", kCode_opt },
	{ "opf", kCode_opf },
	{ "ofg", kCode_ofg },
	{ "snc", kCode_snc },
	{ "sse", kCode_sse },
	{ "ssp", kCode_ssp },
	{ "s0p", kCode_s0p },
	{ "sp",  kCode_sp  },
	{ "tpo", kCode_tpo },
	{ "wa+", kCode_wa_ },
	{ "wsm", kCode_wsm },
	{ "wat", kCode_wat },
	{ "was", kCode_was },
	{ "wfx", kCode_wfx },
	{ "xhr", kCode_xhr },
	{ "xhm", kCode_xhm },
	{ nullptr, 0 }
};

int TuckerEngine::readTableInstructionCode(int *index) {
	bool match = false;
	int nameLen = 0;
	for (int i = 0; _instructions[i].name; ++i) {
		nameLen = strlen(_instructions[i].name);
		if (_instructions[i].name[1] == '0') {
			if (_instructions[i].name[0] == _tableInstructionsPtr[0] && _instructions[i].name[2] == _tableInstructionsPtr[2]) {
				const char digit = _tableInstructionsPtr[1];
				assert(digit >= '0' && digit <= '9');
				*index = digit - '0';
				match = true;
			}
		} else {
			if (strncmp(_instructions[i].name, (const char *)_tableInstructionsPtr, nameLen) == 0) {
				*index = 0;
				match = true;
			}
		}
		if (match) {
			_tableInstructionsPtr += nameLen + 1;
			return _instructions[i].code;
		}
	}
	warning("Unhandled instruction '%c%c%c'", _tableInstructionsPtr[0], _tableInstructionsPtr[1], _tableInstructionsPtr[2]);
	_tableInstructionsPtr += nameLen + 1;
	return kCode_invalid;
}

int TuckerEngine::readTableInstructionParam(int len) {
	// skip duplicated minus signs (bua,--1, c0a,--1, ...)
	if (len >= 3 && memcmp(_tableInstructionsPtr, "--", 2) == 0) {
		++_tableInstructionsPtr;
		--len;
	}
	char *end = nullptr;
	const int param = strtol((const char *)_tableInstructionsPtr, &end, 10);
	if (end != (const char *)_tableInstructionsPtr + len) {
		warning("Unexpected instruction parameter length %d (%d)", (int)(end - (const char *)_tableInstructionsPtr), len);
	}
	_tableInstructionsPtr += len + 1;
	return param;
}

int TuckerEngine::executeTableInstruction() {
	int i, index = 0;

	debug(2, "executeTableInstruction() instruction %c%c%c", _tableInstructionsPtr[0], _tableInstructionsPtr[1], _tableInstructionsPtr[2]);
	const int code = readTableInstructionCode(&index);
	switch (code) {
	case kCode_pan:
		_panelType = (PanelType)readTableInstructionParam(2);
		_forceRedrawPanelItems = true;
		return 0;
	case kCode_bua:
		_backgroundSpriteCurrentAnimation = readTableInstructionParam(3);
		_backgroundSpriteCurrentFrame = 0;
		_backgroundSprOffset = 0;
		_mainLoopCounter2 = 0;
		return 0;
	case kCode_bub:
		i = readTableInstructionParam(3);
		_spriteAnimationFrameIndex = _spriteAnimationsTable[i]._firstFrameIndex;
		_characterFacingDirection = 5;
		_mainLoopCounter2 = 0;
		return 0;
	case kCode_buc:
		i = readTableInstructionParam(3);
		resetCharacterAnimationIndex(i);
		_backgroundSpriteCurrentFrame = 0;
		_backgroundSprOffset = 0;
		return 0;
	case kCode_bsd:
		_selectedCharacterDirection = readTableInstructionParam(2);
		return 0;
	case kCode_bof:
		_skipCurrentCharacterDraw = true;
		return 0;
	case kCode_buh:
		_noCharacterAnimationChange = readTableInstructionParam(2);
		return 0;
	case kCode_bon:
		_skipCurrentCharacterDraw = false;
		return 0;
	case kCode_bso:
		_backgroundSprOffset = readTableInstructionParam(3);
		return 0;
	case kCode_bus:
		_speechSoundNum = readTableInstructionParam(3) - 1;
		rememberSpeechSound();
		startSpeechSound(_part * 3000 + _ptTextOffset + _speechSoundNum - 3000, _speechVolume);
		_actionPosX = _xPosCurrent;
		_actionPosY = _yPosCurrent - 64;
		_actionTextColor = 1;
		_actionCharacterNum = 99;
		_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
		return 0;
	case kCode_buw:
		_selectedObject._xPos = readTableInstructionParam(3);
		_selectedObject._yPos = readTableInstructionParam(3);

		// WORKAROUND: original game bug
		// When Bud is walked to specific coordinates using the 'buw' opcode the
		// walkable area is not enforced (_locationMaskIgnore == true).
		// This is usually not a problem because the player is not allowed to click,
		// however, when entering the club, this allows the player to move Bud to
		// coordinates from which he can never return, leaving him stuck there.
		// As a workaround, do not ignore the location mask during this specific
		// action when entering the club.
		// This fixes Trac#5838.
		if (!(_location == kLocationStripJoint && _nextAction == 59)) {
			_locationMaskIgnore = true;
		}
		_panelLockedFlag = true;
		return 0;
	case kCode_bux:
		_xPosCurrent = readTableInstructionParam(3);
		_yPosCurrent = readTableInstructionParam(3);
		return 0;
	case kCode_c0a:
		_spritesTable[index]._state = readTableInstructionParam(3);
		if (_spritesTable[index]._state == 999) {
			_spritesTable[index]._state = -1;
		}
		_mainLoopCounter1 = 0;
		_spritesTable[index]._updateDelay = 0;
		_spritesTable[index]._nextAnimationFrame = false;
		_spritesTable[index]._prevAnimationFrame = false;
		return 0;
	case kCode_c0c:
		setCharacterAnimation(readTableInstructionParam(3), index);
		return 0;
	case kCode_c0s:
		_speechSoundNum = readTableInstructionParam(3) - 1;
		rememberSpeechSound();
		startSpeechSound(_part * 3000 + _ptTextOffset + _speechSoundNum - 3000, kMaxSoundVolume);
		_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
		_actionTextColor = 181 + index;
		if (!_tableInstructionFlag) {
			_actionPosX = _tableInstructionItemNum1;
			_actionPosY = _tableInstructionItemNum2;
		} else {
			_actionPosX = _tableInstructionObj1Table[index];
			_actionPosY = _tableInstructionObj2Table[index];
		}
		_actionCharacterNum = index;
		return 0;
	case kCode_end:
		return 2;
	case kCode_fad:
		_fadePaletteCounter = readTableInstructionParam(2);
		return 0;
	case kCode_fw:
		_selectedCharacterNum = readTableInstructionParam(2);
		_actionVerb = kVerbWalk;
		_selectedObjectType = 0;
		_selectedObjectNum = 1;
		setSelectedObjectKey();
		return 0;
	case kCode_flx:
		i = readTableInstructionParam(2);
		_locationSoundsTable[i]._type = 2;
		startSound(_locationSoundsTable[i]._offset, i, _locationSoundsTable[i]._volume);
		return 0;
	case kCode_fxx:
		i = readTableInstructionParam(2);
		if (isSoundPlaying(i)) {
			stopSound(i);
		}
		return 0;
	case kCode_fx:
		i = readTableInstructionParam(2);
		startSound(_locationSoundsTable[i]._offset, i, _locationSoundsTable[i]._volume);
		_soundInstructionIndex = i;
		return 0;
	case kCode_gfg:
		i = readTableInstructionParam(3);
		assert(i >= 0 && i < kFlagsTableSize);
		_flagsTable[i] = readTableInstructionParam(2);
		debug(2, "executeTableInstruction() set flag %d to %d", i, _flagsTable[i]);
		return 0;
	case kCode_gv:
		_characterAnimationNum = readTableInstructionParam(2);
		return 0;
	case kCode_loc:
		_nextLocation = (Location)readTableInstructionParam(2);
		return 1;
	case kCode_mof:
		setCursorState(kCursorStateDisabledHidden);
		return 0;
	case kCode_opt:
		_conversationOptionsCount = readTableInstructionParam(2);
		for (i = 0; i < _conversationOptionsCount; ++i) {
			_instructionsActionsTable[i] = readTableInstructionParam(3) - 1;
			_nextTableToLoadTable[i] = readTableInstructionParam(3);
		}
		_nextTableToLoadIndex = -1;
		setCursorState(kCursorStateDialog);
		return 1;
	case kCode_opf:
		_conversationOptionsCount = 0;
		for (i = readTableInstructionParam(2); i > 0; --i) {
			const int flag = readTableInstructionParam(3);
			const int value = readTableInstructionParam(2);
			debug(2, "executeTableInstruction() compare flag %d to %d (%d)", i, value, _flagsTable[i]);
			assert(flag >= 0 && flag < kFlagsTableSize);
			if (value == _flagsTable[flag]) {
				assert(_conversationOptionsCount < 6);
				_instructionsActionsTable[_conversationOptionsCount] = readTableInstructionParam(3) - 1;
				_nextTableToLoadTable[_conversationOptionsCount] = readTableInstructionParam(3);
				++_conversationOptionsCount;
			} else {
				readTableInstructionParam(3);
				readTableInstructionParam(3);
			}
		}
		_nextTableToLoadIndex = -1;
		setCursorState(kCursorStateDialog);
		return 1;
	case kCode_ofg:
		i = readTableInstructionParam(3);
		if (readTableInstructionParam(2) == 0) {
			removeObjectFromInventory(i);
		} else {
			addObjectToInventory(i);
		}
		return 0;
	case kCode_snc:
		_mainLoopCounter1 = 0;
		return 0;
	case kCode_sse:
		_nextAction = readTableInstructionParam(3);
		_csDataLoaded = false;
		return 3;
	case kCode_ssp:
		_tableInstructionFlag = false;
		_tableInstructionItemNum1 = readTableInstructionParam(3);
		_tableInstructionItemNum2 = readTableInstructionParam(3);
		return 0;
	case kCode_s0p:
		_tableInstructionFlag = true;
		_tableInstructionObj1Table[index] = readTableInstructionParam(3);
		_tableInstructionObj2Table[index] = readTableInstructionParam(3);
		return 0;
	case kCode_sp:
		_characterSpriteAnimationFrameCounter = 1;
		return 0;
	case kCode_tpo:
		_ptTextOffset = readTableInstructionParam(4);
		_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_ptTextOffset, _ptTextBuf);
		return 0;
	case kCode_wa_:
		_stopActionOnSpeechFlag = true;
		_stopActionCounter = 20;
		return 1;
	case kCode_wsm:
		_stopActionOnPanelLock = true;

		// WORKAROUND
		// Some versions have a script bug which allows you to freely click around
		// during the sequence of Bud freeing the professor in part two which even
		// allows Bud to leave the room while talking to the professor resulting in
		// general glitchiness. The Spanish and Polish versions (and possibly others)
		// fixed this by introducing the 'mof' opcode to disable the mouse during the
		// sequence.
		//
		// The difference is as follows:
		//   Buggy: 61dw buw,148,125,wsm,buw,148,132,wsm,wat,050[...]
		//   Fixed: 61dw buw,148,125,wsm,buw,148,132,wsm,mof,pan,01,wat,050[...]
		//                                               ^^^^^^^^^^
		// To work around the issue in the problematic versions we inject these two
		// instructions after the first occurrence of the 'wsm' instruction (which
		// proves good enough).
		if (_location == kLocationStoreRoom && _nextAction == 61) {
			setCursorState(kCursorStateDisabledHidden);
			_panelType = kPanelTypeEmpty;
		}

		return 1;
	case kCode_wat:
		_stopActionCounter = readTableInstructionParam(3);
		return 1;
	case kCode_was:
		_stopActionOnSpeechFlag = true;
		return 1;
	case kCode_wfx:
		_stopActionOnSoundFlag = true;
		return 1;
	case kCode_xhr:
		_validInstructionId = true;
		return 0;
	case kCode_xhm:
		_validInstructionId = false;
		return 0;
	case kCode_no3:
		// opcodes mapped here are treated as NOOPs
		readTableInstructionParam(3);
		return 0;
	default:
		break;
	}
	return 2;
}

void TuckerEngine::moveUpInventoryObjects() {
	if (_inventoryObjectsOffset + 6 < _inventoryObjectsCount) {
		_inventoryObjectsOffset += 3;
		_forceRedrawPanelItems = true;
	}
}

void TuckerEngine::moveDownInventoryObjects() {
	if (_inventoryObjectsOffset > 2) {
		_inventoryObjectsOffset -= 3;
		_forceRedrawPanelItems = true;
	}
}

void TuckerEngine::setActionVerbUnderCursor() {
	if (_mousePosY < 150) {
		_actionVerb = kVerbWalk;
	} else if (_mousePosX > 195) {
		_actionVerb = kVerbLook;
	} else if (_panelStyle == kPanelStyleVerbs) {
		_actionVerb = (Verb)(((_mousePosY - 150) / 17) * 3 + (_mousePosX / 67));
	} else {
		_actionVerb = kVerbWalk;
		if (_mousePosX < 30) {
			_actionVerb = kVerbMove;
		} else if (_mousePosX > 130 && _mousePosX < 165) {
			_actionVerb = kVerbGive;
		} else {
			if (_mousePosY < 175) {
				if (_mousePosX < 67) {
					_actionVerb = kVerbOpen;
				} else if (_mousePosX > 164) {
					_actionVerb = kVerbTake;
				} else if (_mousePosX > 99) {
					_actionVerb = kVerbClose;
				}
			} else {
				if (_mousePosX < 85) {
					_actionVerb = kVerbLook;
				} else if (_mousePosX > 165) {
					_actionVerb = kVerbTalk;
				} else {
					_actionVerb = kVerbUse;
				}
			}
		}
	}
}

int TuckerEngine::getObjectUnderCursor() {
	if (_mousePosY > 140) {
		return -1;
	}
	for (int i = 0; i < _locationObjectsCount; ++i) {
		if (_mousePosX + _scrollOffset + 1 <= _locationObjectsTable[i]._xPos) {
			continue;
		}
		if (_mousePosX + _scrollOffset >= _locationObjectsTable[i]._xPos + _locationObjectsTable[i]._xSize) {
			continue;
		}
		if (_mousePosY <= _locationObjectsTable[i]._yPos) {
			continue;
		}
		if (_mousePosY >= _locationObjectsTable[i]._yPos + _locationObjectsTable[i]._ySize) {
			continue;
		}
		_selectedObjectType = 0;
		_selectedCharacterNum = i;
		setCursorStyle(_locationObjectsTable[i]._cursorStyle);
		return i;
	}
	return -1;
}

void TuckerEngine::setSelectedObjectKey() {
	const int x = _mousePosX + _scrollOffset;
	if (_mousePosY > 139 && _nextAction == 0) {
		return;
	}
	_panelLockedFlag = true;
	_locationMaskCounter = 0;
	_actionRequiresTwoObjects = false;
	_selectedObject._yPos = 0;
	_selectedObject._locationObjectLocation = kLocationNone;
	_pendingActionIndex = 0;
	if (_selectedObjectType == 0) {
		if (_selectedObjectNum == 0) {
			_selectedObject._xPos = x;
			_selectedObject._yPos = _mousePosY;
		} else {
			_selectedObject._xPos = _locationObjectsTable[_selectedCharacterNum]._standX;
			_selectedObject._yPos = _locationObjectsTable[_selectedCharacterNum]._standY;
			if (_actionVerb == kVerbWalk || _actionVerb == kVerbUse) {
				_selectedObject._locationObjectLocation = _locationObjectsTable[_selectedCharacterNum]._location;
				_selectedObject._locationObjectToX = _locationObjectsTable[_selectedCharacterNum]._toX;
				_selectedObject._locationObjectToY = _locationObjectsTable[_selectedCharacterNum]._toY;
				_selectedObject._locationObjectToX2 = _locationObjectsTable[_selectedCharacterNum]._toX2;
				_selectedObject._locationObjectToY2 = _locationObjectsTable[_selectedCharacterNum]._toY2;
				_selectedObject._locationObjectToWalkX2 = _locationObjectsTable[_selectedCharacterNum]._toWalkX2;
				_selectedObject._locationObjectToWalkY2 = _locationObjectsTable[_selectedCharacterNum]._toWalkY2;
			}
		}
	} else {
		switch (_selectedObjectType) {
		case 1:
			_selectedObject._xPos = _locationAnimationsTable[_selectedCharacterNum]._standX;
			_selectedObject._yPos = _locationAnimationsTable[_selectedCharacterNum]._standY;
			break;
		case 2:
			_selectedObject._xPos = _charPosTable[_selectedCharacterNum]._xWalkTo;
			_selectedObject._yPos = _charPosTable[_selectedCharacterNum]._yWalkTo;
			break;
		case 3:
			_selectedObject._xPos = _xPosCurrent;
			_selectedObject._yPos = _yPosCurrent;
			break;
		default:
			break;
		}
	}
	if (_selectedObject._yPos == 0) {
		_selectedObject._xPos = x;
		_selectedObject._yPos = _mousePosY;
	}
	_selectedObjectLocationMask = testLocationMask(_selectedObject._xPos, _selectedObject._yPos);
	if (!_selectedObjectLocationMask && _objectKeysLocationTable[_location] == 1) {
		if (_selectedObject._yPos < _objectKeysPosYTable[_location]) {
			while (!_selectedObjectLocationMask && _selectedObject._yPos < _objectKeysPosYTable[_location]) {
				++_selectedObject._yPos;
				_selectedObjectLocationMask = testLocationMask(_selectedObject._xPos, _selectedObject._yPos);
			}
		} else {
			while (!_selectedObjectLocationMask && _selectedObject._yPos < _objectKeysPosYTable[_location]) {
				--_selectedObject._yPos;
				_selectedObjectLocationMask = testLocationMask(_selectedObject._xPos, _selectedObject._yPos);
			}
		}
	}
	if (_selectedObjectLocationMask) {
		_selectedObjectLocationMask = testLocationMaskArea(_xPosCurrent, _yPosCurrent, _selectedObject._xPos, _selectedObject._yPos);
		if (_selectedObjectLocationMask && _objectKeysPosXTable[_location] > 0) {
			_selectedObject._xDefaultPos = _selectedObject._xPos;
			_selectedObject._yDefaultPos = _selectedObject._yPos;
			_selectedObject._xPos = _objectKeysPosXTable[_location];
			_selectedObject._yPos = _objectKeysPosYTable[_location];
			if (_objectKeysLocationTable[_location] == 1) {
				_selectedObject._xPos = _selectedObject._xDefaultPos;
			}
		}
	}
}

void TuckerEngine::setCharacterAnimation(int count, int spr) {
	_spritesTable[spr]._animationFrame = 0;
	_spritesTable[spr]._stateIndex = 0;
	for (int i = 0; i < count; ++i) {
		while (_characterStateTable[_spritesTable[spr]._stateIndex] != 99) {
			++_spritesTable[spr]._stateIndex;
		}
		++_spritesTable[spr]._stateIndex;
	}
	_spritesTable[spr]._state = _characterStateTable[_spritesTable[spr]._stateIndex];
	++_spritesTable[spr]._stateIndex;
	_spritesTable[spr]._animationFrame = _characterStateTable[_spritesTable[spr]._stateIndex];
	++_spritesTable[spr]._stateIndex;
	_spritesTable[spr]._animationData = _sprC02Table[_spritesTable[spr]._state];
	_spritesTable[spr]._firstFrame = READ_LE_UINT16(_spritesTable[spr]._animationData);
}

bool TuckerEngine::testLocationMaskArea(int xBase, int yBase, int xPos, int yPos) {
	while (true) {
		bool loop = false;
		if (yBase > yPos) {
			if (testLocationMask(xBase, yBase - 1)) {
				--yBase;
				loop = true;
			}
		} else if (yBase < yPos) {
			if (testLocationMask(xBase, yBase + 1)) {
				++yBase;
				loop = true;
			}
		}
		if (xBase > xPos) {
			if (testLocationMask(xBase - 1, yBase)) {
				--xBase;
				loop = true;
			}
		} else if (xBase < xPos) {
			if (testLocationMask(xBase + 1, yBase)) {
				++xBase;
				loop = true;
			}
		}
		if (xBase == xPos && yBase == yPos) {
			return false;
		}
		if (!loop) {
			break;
		}
	}
	return true;
}

void TuckerEngine::handleMouseClickOnInventoryObject() {
	if (_panelType != kPanelTypeNormal) {
		return;
	}
	if (_mousePosY < 150 || _mousePosX < 212) {
		return;
	}
	int pos = ((_mousePosY - 150) / 25) * 3 + (_mousePosX - 212) / 36;
	int obj = _inventoryObjectsOffset + pos;
	if (_inventoryObjectsCount - 1 < obj) {
		return;
	}
	_selectedObjectNum = _inventoryObjectsList[obj];
	_selectedObjectType = 3;
	switch (_selectedObjectNum) {
	case 30:
		if (_leftMouseButtonPressed) {
			_selectedObjectType = 0;
			_selectedObjectNum = 0;
			_actionVerb = kVerbWalk;
			_actionVerbLocked = false;
			_forceRedrawPanelItems = true;
			_panelType = kPanelTypeLoadSavePlayQuit;
			setCursorState(kCursorStateDialog);
		}
		break;
	case 1:
		if (_actionVerb == kVerbUse && _leftMouseButtonPressed) {
			if (_mapSequenceFlagsLocationTable[_location - 1] == 1) {
				handleMapSequence();
			} else {
				_actionPosX = _xPosCurrent;
				_actionPosY = _yPosCurrent - 64;
				_actionTextColor = 1;
				_actionCharacterNum = 99;
				setCursorState(kCursorStateDisabledHidden);
				_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
				_currentActionVerb = kVerbWalk;
				_speechSoundNum = 2235;
				startSpeechSound(_speechSoundNum, _speechVolume);
				_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_speechSoundNum, _ptTextBuf);
				_speechSoundNum = 0;
				_actionVerb = kVerbWalk;
				_selectedObjectType = 0;
				_selectedObjectNum = 0;
				_actionVerbLocked = false;
			}
		}
		break;
	default:
		break;
	}
}

int TuckerEngine::setCharacterUnderCursor() {
	if (_mousePosY > 140) {
		return -1;
	}
	for (int i = 0; i < _charPosCount; ++i) {
		if (_mousePosX + _scrollOffset <= _charPosTable[i]._xPos) {
			continue;
		}
		if (_mousePosX + _scrollOffset >= _charPosTable[i]._xPos + _charPosTable[i]._xSize) {
			continue;
		}
		if (_mousePosY <= _charPosTable[i]._yPos) {
			continue;
		}
		if (_mousePosY >= _charPosTable[i]._yPos + _charPosTable[i]._ySize) {
			continue;
		}
		if (_charPosTable[i]._flagNum == 0 || _flagsTable[_charPosTable[i]._flagNum] == _charPosTable[i]._flagValue) {
			_selectedObjectType = 2;
			_selectedCharacterDirection = _charPosTable[i]._direction;
			_selectedCharacterNum = i;
			return _charPosTable[i]._name;
		}
	}
	return -1;
}

int TuckerEngine::setLocationAnimationUnderCursor() {
	if (_mousePosY > 140) {
		return -1;
	}
	for (int i = _locationAnimationsCount - 1; i >= 0; --i) {
		if (!_locationAnimationsTable[i]._drawFlag)
			continue;

		int num = _locationAnimationsTable[i]._graphicNum;
		if (_mousePosX + _scrollOffset + 1 <= _dataTable[num]._xDest) {
			continue;
		}
		if (_mousePosX + _scrollOffset >= _dataTable[num]._xDest + _dataTable[num]._xSize) {
			continue;
		}
		if (_mousePosY <= _dataTable[num]._yDest) {
			continue;
		}
		if (_mousePosY >= _dataTable[num]._yDest + _dataTable[num]._ySize) {
			continue;
		}
		if (_locationAnimationsTable[i]._selectable == 0) {
			// WORKAROUND
			// The original game does a "return -1" here which is not correct in
			// case of overlapping hotspots.
			// This most prominently fixes Trac#6645, a bug where the cellar in part three
			// could be entered without having done the cellar door puzzle first.
			continue;
		}
		_selectedObjectType = 1;
		_selectedCharacterNum = i;
		_selectedCharacter2Num = i;
		return _locationAnimationsTable[i]._selectable;
	}
	return -1;
}

void TuckerEngine::setActionForInventoryObject() {
	if (_actionVerb == kVerbWalk || _actionVerb == kVerbTalk || _actionVerb == kVerbTake || _actionVerb == kVerbMove) {
		playSpeechForAction(_actionVerb);
		_actionVerbLocked = false;
		_actionRequiresTwoObjects = false;
		return;
	}
	if (_actionVerb == kVerbOpen || _actionVerb == kVerbClose) {
		if (!(_part == kPartTwo && _selectedObjectNum == 19) && !(_part == kPartThree && _selectedObjectNum == 42)) {
			playSpeechForAction(_actionVerb);
			_actionVerbLocked = false;
			_actionRequiresTwoObjects = false;
			return;
		}
	}
	_currentActionObj1Num = _actionObj1Num;
	_currentInfoString1SourceType = _actionObj1Type;
	_currentActionObj2Num = _actionObj2Num;
	_currentInfoString2SourceType = _actionObj2Type;
	if (_actionVerb == kVerbLook && _selectedObjectType == 3) {
		if (_panelLockedFlag) {
			if (_locationMaskType != 0) {
				return;
			}
			_panelLockedFlag = false;
		}
		if (handleSpecialObjectSelectionSequence()) {
			return;
		}
		_speechSoundNum = _actionObj1Num + _speechSoundBaseNum;
		startSpeechSound(_speechSoundNum, _speechVolume);
		_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_speechSoundNum, _ptTextBuf);
		_speechSoundNum = 0;
		_actionPosX = _xPosCurrent;
		_actionPosY = _yPosCurrent - 64;
		_actionTextColor = 1;
		_actionCharacterNum = 99;
		setCursorState(kCursorStateDisabledHidden);
		_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
		_actionVerbLocked = false;
		_actionRequiresTwoObjects = false;
		return;
	}
	// Items with unary usage i.e. "Use X", rather than "Use X on Y"
	if (
		(_part == kPartTwo && _actionObj1Num == 19) || // radio
		(_part == kPartThree && (
			 _actionObj1Num ==  3 || // pizza
			 _actionObj1Num ==  6 || // raincoat
			 _actionObj1Num == 17 || // ear plugs
			 _actionObj1Num == 18 || // glue
			 _actionObj1Num == 33 || // peg
			(_actionObj1Num == 42 && _selectedObjectNum == 18) // skate + cue
		))
	) {
		_actionVerbLocked = false;
		_actionRequiresTwoObjects = false;
		_locationMaskCounter = 1;
		setActionState();
		return;
	}
	if (!_actionRequiresTwoObjects) {
		_actionRequiresTwoObjects = true;
	} else {
		_actionVerbLocked = false;
		_actionRequiresTwoObjects = false;
		_locationMaskCounter = 1;
		setActionState();
	}
}

void TuckerEngine::setActionState() {
	_currentActionVerb = (_actionVerb == kVerbWalk) ? kVerbUse : _actionVerb;
	_currentActionObj1Num = _actionObj1Num;
	_currentInfoString1SourceType = _actionObj1Type;
	_currentActionObj2Num = _actionObj2Num;
	_currentInfoString2SourceType = _actionObj2Type;
	_actionRequiresTwoObjects = false;
	if (_selectedObjectType < 3) {
		setSelectedObjectKey();
	}
}

void TuckerEngine::playSpeechForAction(int i) {
	static const int speechActionTable[] = { 0, 2235, 2235, 2251, 2261, 2276, 2294, 2312, 2235 };
	static const int maxCounterTable[] = { 0, 1, 13, 7, 12, 15, 15, 15, 14 };
	++_speechActionCounterTable[i];
	if (_speechActionCounterTable[i] > maxCounterTable[i]) {
		_speechActionCounterTable[i] = 0;
	}
	if (speechActionTable[i] >= 2000) {
		if (_currentActionVerb == kVerbUse && _currentActionObj1Num == 6 && _currentInfoString1SourceType == 3) {
			_speechSoundNum = 2395;
		} else {
			_speechSoundNum = _speechActionCounterTable[i] + speechActionTable[i];
		}
		startSpeechSound(_speechSoundNum, _speechVolume);
		_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_speechSoundNum, _ptTextBuf);
		_speechSoundNum = 0;
		_actionPosX = _xPosCurrent;
		_actionPosY = _yPosCurrent - 64;
		_actionTextColor = 1;
		_actionCharacterNum = 99;
		setCursorState(kCursorStateDisabledHidden);
		_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
	}
}

void TuckerEngine::drawSpeechText(int xStart, int y, const uint8 *dataPtr, int num, int color) {
	int x = (xStart - _scrollOffset) * 2;
	int offset = (_scrollOffset + 320 - xStart) * 2;
	if (_conversationOptionsCount > 0) {
		x = 304;
	} else {
		if (x > offset) {
			x = offset;
		}
		if (x > 180) {
			x = 180;
		} else if (x < 150) {
			x = 150;
		}
	}
	int count = 0;
	bool flag = false;
	struct {
		int w, count, offset;
	} lines[5];
	lines[0].offset = getPositionForLine(num, dataPtr);
	while (!flag && count < 4) {
		int lineCharsCount, lineWidth;
		flag = splitSpeechTextLines(dataPtr, lines[count].offset, x, lineCharsCount, lineWidth);
		lines[count].w = lineWidth;
		lines[count].count = lineCharsCount;
		lines[count + 1].offset = lines[count].offset + lineCharsCount + 1;
		++count;
	}
	if (count * 10 > y) {
		y = count * 10;
	}
	for (int i = 0; i < count; ++i) {
		int yPos, xPos = xStart - lines[i].w / 2;
		if (xPos < _scrollOffset) {
			xPos = _scrollOffset;
		} else if (xPos > _scrollOffset + 320 - lines[i].w) {
			xPos = _scrollOffset + 320 - lines[i].w;
		}
		if (_conversationOptionsCount != 0) {
			xPos = xStart + _scrollOffset;
			yPos = i * 10 + y;
			_conversationOptionLinesCount = count;
		} else {
			yPos = y - (count - i) * 10;
		}
		drawSpeechTextLine(dataPtr, lines[i].offset, lines[i].count, xPos, yPos, color);
	}
}

bool TuckerEngine::splitSpeechTextLines(const uint8 *dataPtr, int pos, int x, int &lineCharsCount, int &lineWidth) {
	int count = 0;
	int w = 0;
	lineCharsCount = 0;
	lineWidth = 0;
	while (x + 1 > w && dataPtr[pos] != '\n' && dataPtr[pos] != '\r') {
		if (dataPtr[pos] == ' ') {
			lineCharsCount = count;
			lineWidth = w;
		}
		w += _charWidthTable[dataPtr[pos]];
		++count;
		++pos;
	}
	bool ret = false;
	if (x + 1 > w) {
		lineCharsCount = count;
		lineWidth = w;
		ret = true;
	}
	return ret;
}

void TuckerEngine::drawSpeechTextLine(const uint8 *dataPtr, int pos, int count, int x, int y, uint8 color) {
	const int xStart = x;
	for (int i = 0; i < count && dataPtr[pos] != '\n'; ++i) {
		Graphics::drawStringChar(_locationBackgroundGfxBuf, x, y, 640, dataPtr[pos], color, _charsetGfxBuf);
		x += _charWidthTable[dataPtr[pos]];
		++pos;
	}
	// At least in the English version of the game many glyphs in the character set are one pixel
	// wider than specified in the character width table. This ensures that, when rendering text,
	// characters are overlapping one pixel (i.e. their outlines overlap).
	// This has the negative side effect that when a text line ends with a glyph whose specified
	// size is narrower than its actual size, the calculated width for the dirty rect is wrong.
	// To compensate for this we add the current character set's maximum glyph width to make sure
	// that the dirty rect always covers the whole line.
	// This fixes Bug #6370.
	addDirtyRect(xStart, y, x - xStart + Graphics::_charset._charW, Graphics::_charset._charH);
}

void TuckerEngine::redrawScreen(int offset) {
	debug(9, "redrawScreen() _fullRedraw %d offset %d _dirtyRectsCount %d", _fullRedraw, offset, _dirtyRectsCount);
	assert(offset <= kScreenWidth);
	if (_fullRedraw) {
		_fullRedraw = false;
		_system->copyRectToScreen(_locationBackgroundGfxBuf + offset, kScreenPitch, 0, 0, kScreenWidth, kScreenHeight);
	} else {
		Common::Rect clipRect(offset, 0, offset + kScreenWidth, kScreenHeight);
		for (int i = 0; i < _dirtyRectsPrevCount + _dirtyRectsCount; ++i) {
			redrawScreenRect(clipRect, _dirtyRectsTable[i]);
		}
	}
	if (_dirtyRectsPrevCount + _dirtyRectsCount < kMaxDirtyRects) {
		for (int i = 0; i < _dirtyRectsCount; ++i) {
			_dirtyRectsTable[i] = _dirtyRectsTable[_dirtyRectsPrevCount + i];
		}
		_dirtyRectsPrevCount = _dirtyRectsCount;
	} else {
		_dirtyRectsPrevCount = 0;
		_fullRedraw = true;
	}
	_dirtyRectsCount = 0;
	_system->updateScreen();
}

void TuckerEngine::redrawScreenRect(const Common::Rect &clip, const Common::Rect &dirty) {
	if (dirty.intersects(clip)) {
		Common::Rect r(dirty);
		r.clip(clip);
		const uint8 *src = _locationBackgroundGfxBuf + r.top * 640 + r.left;
		r.translate(-clip.left, -clip.top);
		const int w = r.right - r.left;
		const int h = r.bottom - r.top;
		if (w <= 0 || h <= 0) {
			return;
		}
#if 0
		static const uint8 outlineColor = 0;
		memset(_locationBackgroundGfxBuf + r.top           * 640 + r.left, outlineColor, w);
		memset(_locationBackgroundGfxBuf + (r.top + h - 1) * 640 + r.left, outlineColor, w);
		for (int y = r.top; y < r.top + h; ++y) {
			_locationBackgroundGfxBuf[y * 640 + r.left] = outlineColor;
			_locationBackgroundGfxBuf[y * 640 + r.left + w - 1] = outlineColor;
		}
#endif
		_system->copyRectToScreen(src, 640, r.left, r.top, w, h);
	}
}

void TuckerEngine::addDirtyRect(int x, int y, int w, int h) {
	if (_dirtyRectsPrevCount + _dirtyRectsCount < kMaxDirtyRects) {
		Common::Rect r(x, y, x + w, y + h);
		for (int i = 0; i < _dirtyRectsCount; ++i) {
			if (_dirtyRectsTable[_dirtyRectsPrevCount + i].contains(r)) {
				return;
			}
		}
		_dirtyRectsTable[_dirtyRectsPrevCount + _dirtyRectsCount] = r;
		++_dirtyRectsCount;
	} else {
		_fullRedraw = true;
	}
}

} // namespace Tucker
