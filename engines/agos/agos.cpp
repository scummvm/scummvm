/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/events.h"

#include "agos/debugger.h"
#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/vga.h"

#include "graphics/surface.h"

#include "sound/mididrv.h"
#include "sound/mods/protracker.h"
#include "sound/audiocd.h"

using Common::File;

namespace AGOS {

static const GameSpecificSettings simon1_settings = {
	"EFFECTS",                              // effects_filename
	"SIMON",                                // speech_filename
};

static const GameSpecificSettings simon2_settings = {
	"",                                     // effects_filename
	"SIMON2",                               // speech_filename
};

static const GameSpecificSettings puzzlepack_settings = {
	"",                                     // effects_filename
	"MUSIC",                               // speech_filename
};

AGOSEngine_PuzzlePack::AGOSEngine_PuzzlePack(OSystem *system)
	: AGOSEngine_Feeble(system) {

	_oopsValid = false;
	_iconToggleCount = 0;
	_voiceCount = 0;

	_gameTime = 0;
	_lastTickCount = 0;
	_thisTickCount = 0;
	_startSecondCount = 0;
	_tSecondCount = 0;
}

AGOSEngine_Simon2::AGOSEngine_Simon2(OSystem *system)
	: AGOSEngine_Simon1(system) {
}

AGOSEngine_Simon1::AGOSEngine_Simon1(OSystem *system)
	: AGOSEngine_Waxworks(system) {
}

AGOSEngine_Waxworks::AGOSEngine_Waxworks(OSystem *system)
	: AGOSEngine_Elvira2(system) {

	_boxCR = false;
	_boxLineCount = 0;
	memset(_boxBuffer, 0, sizeof(_boxBuffer));
	_boxBufferPtr = _boxBuffer;

	_linePtrs[0] = 0;
	_linePtrs[1] = 0;
	_linePtrs[2] = 0;
	_linePtrs[3] = 0;
	_linePtrs[4] = 0;
	_linePtrs[5] = 0;
	memset(_lineCounts, 0, sizeof(_lineCounts));
}

AGOSEngine_Elvira2::AGOSEngine_Elvira2(OSystem *system)
	: AGOSEngine_Elvira1(system) {
}

AGOSEngine_Elvira1::AGOSEngine_Elvira1(OSystem *system)
	: AGOSEngine(system) {
}

AGOSEngine::AGOSEngine(OSystem *syst)
	: Engine(syst) {

	_vcPtr = 0;
	_vcGetOutOfCode = 0;
	_gameOffsetsPtr = 0;

	_debugger = 0;

	_gameFile = 0;
	_opcode = 0;

	_itemMemSize = 0;
	_tableMemSize = 0;
	_vgaMemSize = 0;

	_musicIndexBase = 0;
	_soundIndexBase = 0;
	_tableIndexBase = 0;
	_textIndexBase = 0;

	_numMusic = 0;
	_numSFX = 0;
	_numSpeech = 0;

	_numBitArray1 = 0;
	_numBitArray2 = 0;
	_numBitArray3 = 0;
	_numItemStore = 0;
	_numTextBoxes = 0;
	_numVars = 0;
	_numVideoOpcodes = 0;
	_vgaBaseDelay = 0;
	_vgaPeriod = 0;

	_strippedTxtMem = 0;
	_textMem = 0;
	_textSize = 0;
	_stringTabNum = 0;
	_stringTabPos = 0;
	_stringTabSize = 0;
	_stringTabPtr = 0;

	_itemArrayPtr = 0;
	_itemArraySize = 0;
	_itemArrayInited = 0;

	_iconFilePtr = 0;

	_codePtr = 0;

	_localStringtable = 0;
	_stringIdLocalMin = 0;
	_stringIdLocalMax = 0;

	_roomStates = 0;
	_numRoomStates = 0;

	_menuBase = 0;
	_roomsList = 0;
	_roomsListPtr = 0;

	_xtblList = 0;
	_xtablesHeapPtrOrg = 0;
	_xtablesHeapCurPosOrg = 0;
	_xsubroutineListOrg = 0;

	_tblList = 0;
	_tablesHeapPtr = 0;
	_tablesHeapPtrOrg = 0;
	_tablesheapPtrNew = 0;
	_tablesHeapSize = 0;
	_tablesHeapCurPos = 0;
	_tablesHeapCurPosOrg = 0;
	_tablesHeapCurPosNew = 0;
	_subroutineListOrg = 0;

	_subroutineList = 0;

	_recursionDepth = 0;

	_lastVgaTick = 0;

	_marks = 0;
	_scanFlag = false;

	_scriptVar2 = 0;
	_runScriptReturn1 = 0;
	_skipVgaWait = 0;
	_noParentNotify = 0;
	_beardLoaded = 0;
	_litBoxFlag = 0;
	_mortalFlag = 0;
	_displayScreen = false;
	_syncFlag2 = 0;
	_inCallBack = 0;
	_cepeFlag = 0;
	_fastMode = 0;

	_backFlag = 0;

	_debugMode = 0;
	_dumpScripts = false;
	_dumpOpcodes = false;
	_dumpVgaScripts = false;
	_dumpVgaOpcodes = false;
	_dumpImages = false;

	_copyProtection = false;
	_pause = false;
	_speech = false;
	_subtitles = false;

	_animatePointer = 0;
	_maxCursorWidth = 0;
	_maxCursorHeight = 0;
	_mouseAnim = 0;
	_mouseAnimMax = 0;
	_mouseCursor = 0;
	_mouseData = 0;
	_oldMouseCursor = 0;
	_currentMouseCursor = 0;
	_currentMouseAnim = 0;
	_oldMouseAnimMax = 0;

	_vgaVar9 = 0;
	_chanceModifier = 0;
	_restoreWindow6 = 0;
	_scrollX = 0;
	_scrollY = 0;
	_scrollXMax = 0;
	_scrollYMax = 0;
	_scrollCount = 0;
	_scrollFlag = 0;
	_scrollHeight = 0;
	_scrollWidth = 0;
	_scrollImage = 0;
	_boxStarHeight = 0;

	_scriptVerb = 0;
	_scriptNoun1 = 0;
	_scriptNoun2 = 0;
	_scriptAdj1 = 0;
	_scriptAdj2 = 0;

	_curWindow = 0;
	_textWindow = 0;

	_subjectItem = 0;
	_objectItem = 0;
	_currentPlayer = 0;

	_iOverflow = false;
	_nameLocked = false;
	_hitAreaObjectItem = 0;
	_lastHitArea = 0;
	_lastNameOn = 0;
	_lastHitArea3 = 0;
	_hitAreaSubjectItem = 0;
	_currentBox = 0;
	_currentBoxNum = 0;
	_currentVerbBox = 0;
	_lastVerbOn = 0;
	_needHitAreaRecalc = 0;
	_verbHitArea = 0;
	_defaultVerb = 0;
	_mouseHideCount = 0;

	_dragAccept = false;
	_dragEnd = false;
	_dragFlag = false;
	_dragMode = false;
	_dragCount = 0;
	_lastClickRem = 0;

	_windowNum = 0;

	_printCharCurPos = 0;
	_printCharMaxPos = 0;
	_printCharPixelCount = 0;
	_numLettersToPrint = 0;

	_clockStopped = 0;
	_gameStoppedClock = 0;
	_lastTime = 0;
	_lastMinute = 0;

	_firstTimeStruct = 0;
	_pendingDeleteTimeEvent = 0;

	_initMouse = 0;
	_leftButtonDown = false;
	_rightButtonDown = false;
	_clickOnly = false;
	_oneClick = 0;
	_leftClick = 0;
	_rightClick = 0;
	_noRightClick = false;

	_leftButton = 0;
	_leftButtonCount = 0;
	_leftButtonOld = 0;

	_dummyItem1 = new Item();
	_dummyItem2 = new Item();
	_dummyItem3 = new Item();

	_videoLockOut = 0;
	_scrollUpHitArea = 0;
	_scrollDownHitArea = 0;

	_noOverWrite = 0;
	_rejectBlock = false;

	_fastFadeCount = 0;
	_fastFadeInFlag = 0;
	_fastFadeOutFlag = 0;
	_exitCutscene = 0;
	_paletteFlag = 0;
	_bottomPalette = false;
	_picture8600 = 0;

	_soundFileId = 0;
	_lastMusicPlayed = 0;
	_nextMusicToPlay = 0;
	_sampleEnd = 0;
	_sampleWait = 0;

	_showPreposition = 0;
	_showMessageFlag = 0;

	_newDirtyClip = false;
	_wiped = false;
	_copyScnFlag = 0;
	_vgaSpriteChanged = 0;

	_block = 0;
	_blockEnd = 0;
	_vgaMemPtr = 0;
	_vgaMemEnd = 0;
	_vgaMemBase = 0;
	_vgaFrozenBase = 0;
	_vgaRealBase = 0;
	_zoneBuffers = 0;

	_curVgaFile1 = 0;
	_curVgaFile2 = 0;
	_curSfxFile = 0;
	_curSfxFileSize = 0;

	_syncCount = 0;

	_frameCount = 0;

	_zoneNumber = 0;

	_vgaWaitFor = 0;
	_lastVgaWaitFor = 0;

	_vgaCurZoneNum = 0;
	_vgaCurSpriteId = 0;

	_baseY = 0;
	_scale = 0;

	_feebleRect.left = 0;
	_feebleRect.right = 0;
	_feebleRect.top = 0;
	_feebleRect.bottom = 0;

	_scaleX = 0;
	_scaleY = 0;
	_scaleWidth = 0;
	_scaleHeight = 0;

	_nextVgaTimerToProcess = 0;

	_opcode177Var1 = 1;
	_opcode177Var2 = 0;
	_opcode178Var1 = 1;
	_opcode178Var2 = 0;

	_classLine = 0;
	_classMask = 0;
	_classMode1 = 0;
	_classMode2 = 0;
	_currentLine = 0;
	_currentTable = 0;
	_findNextPtr = 0;

	_agosMenu = 0;
	_currentRoom = 0;
	_superRoomNumber = 0;
	_wallOn = 0;

	memset(_objectArray, 0, sizeof(_objectArray));
	memset(_itemStore, 0, sizeof(_itemStore));
	memset(_textMenu, 0, sizeof(_textMenu));

	memset(_shortText, 0, sizeof(_shortText));
	memset(_shortTextX, 0, sizeof(_shortText));
	memset(_shortTextY, 0, sizeof(_shortText));
	memset(_longText, 0, sizeof(_longText));
	memset(_longSound, 0, sizeof(_longSound));

	memset(_bitArray, 0, sizeof(_bitArray));
	memset(_bitArrayTwo, 0, sizeof(_bitArrayTwo));
	memset(_bitArrayThree, 0, sizeof(_bitArrayThree));

	_variableArray = 0;
	_variableArray2 = 0;
	_variableArrayPtr = 0;

	memset(_windowArray, 0, sizeof(_windowArray));

	memset(_fcsData1, 0, sizeof(_fcsData1));
	memset(_fcsData2, 0, sizeof(_fcsData2));

	_awaitTwoByteToken = 0;
	_byteTokens = 0;
	_byteTokenStrings = 0;
	_twoByteTokens = 0;
	_twoByteTokenStrings = 0;
	_secondTwoByteTokenStrings = 0;
	_thirdTwoByteTokenStrings = 0;
	memset(_textBuffer, 0, sizeof(_textBuffer));
	_textCount = 0;

	_freeStringSlot = 0;

	memset(_stringReturnBuffer, 0, sizeof(_stringReturnBuffer));

	memset(_pathFindArray, 0, sizeof(_pathFindArray));

	memset(_pathValues, 0, sizeof(_pathValues));
	_PVCount = 0;
	_GPVCount = 0;

	memset(_pathValues1, 0, sizeof(_pathValues1));
	_PVCount1 = 0;
	_GPVCount1 = 0;

	memset(_currentPalette, 0, sizeof(_currentPalette));
	memset(_displayPalette, 0, sizeof(_displayPalette));

	memset(_videoBuf1, 0, sizeof(_videoBuf1));
	memset(_videoWindows, 0, sizeof(_videoWindows));

	_dummyWindow = new WindowBlock;
	_windowList = new WindowBlock[16];

	memset(_lettersToPrintBuf, 0, sizeof(_lettersToPrintBuf));

	_planarBuf = 0;

	_midiEnabled = false;
	_nativeMT32 = false;

	_vgaTickCounter = 0;

	_sound = 0;

	_effectsPaused = false;
	_ambientPaused = false;
	_musicPaused = false;

	_saveLoadType = 0;
	_saveLoadSlot = 0;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));

	_saveGameNameLen = 0;
	_saveLoadRowCurPos = 0;
	_numSaveGameRows = 0;
	_saveDialogFlag = false;
	_saveOrLoad = false;
	_saveLoadEdit = false;

	_hyperLink = 0;
	_interactY = 0;
	_oracleMaxScrollY = 0;
	_noOracleScroll = 0;

	_backGroundBuf = 0;
	_backBuf = 0;
	_scaleBuf = 0;
	_window4BackScn = 0;
	_window6BackScn = 0;
	printf("Cleared all\n");

	_window3Flag = 0;
	_window4Flag = 0;
	_window6Flag = 0;

	_moveXMin = 0;
	_moveYMin = 0;
	_moveXMax = 0;
	_moveYMax = 0;

	_vc10BasePtrOld = 0;
	memcpy (_hebrewCharWidths,
		"\x5\x5\x4\x6\x5\x3\x4\x5\x6\x3\x5\x5\x4\x6\x5\x3\x4\x6\x5\x6\x6\x6\x5\x5\x5\x6\x5\x6\x6\x6\x6\x6", 32);


	// Add default file directories for Acorn version of
	// Simon the Sorcerer 1
	File::addDefaultDirectory(_gameDataDir.getChild("execute"));
	File::addDefaultDirectory(_gameDataDir.getChild("EXECUTE"));

	// Add default file directories for Amiga/Macintosh
	// verisons of Simon the Sorcerer 2
	File::addDefaultDirectory(_gameDataDir.getChild("voices"));
	File::addDefaultDirectory(_gameDataDir.getChild("VOICES"));

	// Add default file directories for Amiga & Macintosh
	// versions of The Feeble Files
	File::addDefaultDirectory(_gameDataDir.getChild("gfx"));
	File::addDefaultDirectory(_gameDataDir.getChild("GFX"));
	File::addDefaultDirectory(_gameDataDir.getChild("movies"));
	File::addDefaultDirectory(_gameDataDir.getChild("MOVIES"));
	File::addDefaultDirectory(_gameDataDir.getChild("sfx"));
	File::addDefaultDirectory(_gameDataDir.getChild("SFX"));
	File::addDefaultDirectory(_gameDataDir.getChild("speech"));
	File::addDefaultDirectory(_gameDataDir.getChild("SPEECH"));

	syst->getEventManager()->registerRandomSource(_rnd, "agos");
}

Common::Error AGOSEngine::init() {
	if (getGameId() == GID_DIMP) {
		_screenWidth = 496;
		_screenHeight = 400;
	} else if (getGameType() == GType_FF || getGameType() == GType_PP) {
		_screenWidth = 640;
		_screenHeight = 480;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
	}

	initGraphics(_screenWidth, _screenHeight, getGameType() == GType_FF || getGameType() == GType_PP);

	if ((getGameType() == GType_SIMON2 && getPlatform() == Common::kPlatformWindows) ||
		(getGameType() == GType_SIMON1 && getPlatform() == Common::kPlatformWindows) ||
		((getFeatures() & GF_TALKIE) && getPlatform() == Common::kPlatformAcorn) ||
		(getPlatform() == Common::kPlatformPC)) {

		// Setup midi driver
		int midiDriver = MidiDriver::detectMusicDriver(MDT_ADLIB | MDT_MIDI);
		_nativeMT32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));

		_driver = MidiDriver::createMidi(midiDriver);

		if (_nativeMT32) {
			_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		}

		_midi.mapMT32toGM (getGameType() != GType_SIMON2 && !_nativeMT32);

		_midi.setDriver(_driver);

		int ret = _midi.open();
		if (ret)
			warning("MIDI Player init failed: \"%s\"", _midi.getErrorName (ret));

		_midi.setVolume(ConfMan.getInt("music_volume"), ConfMan.getInt("sfx_volume"));


		_midiEnabled = true;
	} else {
		_driver = NULL;
	}

	// Setup mixer
	syncSoundSettings();

	// allocate buffers
	_backGroundBuf = new Graphics::Surface();
	_backGroundBuf->create(_screenWidth, _screenHeight, 1);

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		_backBuf = new Graphics::Surface();
		_backBuf->create(_screenWidth, _screenHeight, 1);
		_scaleBuf = new Graphics::Surface();
		_scaleBuf->create(_screenWidth, _screenHeight, 1);
	}

	if (getGameType() == GType_SIMON2) {
		_window4BackScn = new Graphics::Surface();
		_window4BackScn->create(_screenWidth, _screenHeight, 1);
	} else if (getGameType() == GType_SIMON1) {
		_window4BackScn = new Graphics::Surface();
		_window4BackScn->create(_screenWidth, 134, 1);
	} else if (getGameType() == GType_WW || getGameType() == GType_ELVIRA2) {
		_window4BackScn = new Graphics::Surface();
		_window4BackScn->create(224, 127, 1);
	} else if (getGameType() == GType_ELVIRA1) {
		_window4BackScn = new Graphics::Surface();
		if (getPlatform() == Common::kPlatformAmiga && (getFeatures() & GF_DEMO)) {
			_window4BackScn->create(224, 196, 1);
		} else {
			_window4BackScn->create(224, 144, 1);
		}
		_window6BackScn = new Graphics::Surface();
		_window6BackScn->create(48, 80, 1);
	}

	setupGame();

	_debugger = new Debugger(this);
	_sound = new Sound(this, gss, _mixer);

	if (ConfMan.hasKey("music_mute") && ConfMan.getBool("music_mute") == 1) {
		_musicPaused = true;
		if (_midiEnabled) {
			_midi.pause(_musicPaused);
		}
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, 0);
	}

	if (ConfMan.hasKey("sfx_mute") && ConfMan.getBool("sfx_mute") == 1) {
		if (getGameId() == GID_SIMON1DOS)
			_midi._enable_sfx ^= 1;
		else
			_sound->effectsPause(_effectsPaused ^= 1);
	}

	_copyProtection = ConfMan.getBool("copy_protection");
	_language = Common::parseLanguage(ConfMan.get("language"));

	if (getGameType() == GType_PP) {
		_speech = true;
		_subtitles = false;
	} else if (getFeatures() & GF_TALKIE) {
		_speech = !ConfMan.getBool("speech_mute");
		_subtitles = ConfMan.getBool("subtitles");

		if (getGameType() == GType_SIMON1) {
			// English and German versions don't have full subtitles
			 if (_language == Common::EN_ANY || _language == Common::DE_DEU)
				_subtitles = false;
			// Other versions require speech to be enabled
			else
				_speech = true;
		}

		// Default to speech only, if both speech and subtitles disabled
		if (!_speech && !_subtitles)
			_speech = true;
	} else {
		_speech = false;
		_subtitles = true;
	}

	// TODO: Use special debug levels instead of the following hack.
	_debugMode = (gDebugLevel >= 0);
	if (gDebugLevel == 2)
		_dumpOpcodes = true;
	if (gDebugLevel == 3)
		_dumpVgaOpcodes = true;
	if (gDebugLevel == 4)
		_dumpScripts = true;
	if (gDebugLevel == 5)
		_dumpVgaScripts = true;

	return Common::kNoError;
}

static const uint16 initialVideoWindows_Simon[20] = {
	0,  0, 20, 200,
	0,  0,  3, 136,
	17, 0,  3, 136,
	0,  0, 20, 200,
	0,  0, 20, 134
};

static const uint16 initialVideoWindows_Common[20] = {
	 3, 0, 14, 136,
	 0, 0,  3, 136,
	17, 0,  3, 136,
	 0, 0, 20, 200,
	 3, 3, 14, 127,
};

static const uint16 initialVideoWindows_PN[20] = {
	 3, 0, 14, 136,
	 0, 0,  3, 136,
	17, 0,  3, 136,
	 0, 0, 20, 200,
	 3, 2, 14, 129,
};

void AGOSEngine_PuzzlePack::setupGame() {
	gss = &puzzlepack_settings;
	_numVideoOpcodes = 85;
	_vgaMemSize = 7500000;
	_itemMemSize = 20000;
	_tableMemSize = 200000;
	_frameCount = 1;
	_vgaBaseDelay = 5;
	_vgaPeriod = (getGameId() == GID_DIMP) ? 35 : 30;
	_numBitArray1 = 128;
	_numItemStore = 10;
	_numTextBoxes = 40;
	_numVars = 2048;

	AGOSEngine::setupGame();
}

void AGOSEngine_Simon2::setupGame() {
	gss = &simon2_settings;
	_tableIndexBase = 1580 / 4;
	_textIndexBase = 1500 / 4;
	_numVideoOpcodes = 75;
#if defined(__DS__)
	_vgaMemSize = 1300000;
#else
	_vgaMemSize = 2000000;
#endif
	_itemMemSize = 20000;
	_tableMemSize = 100000;
	// Check whether to use MT-32 MIDI tracks in Simon the Sorcerer 2
	if (getGameType() == GType_SIMON2 && _nativeMT32)
		_musicIndexBase = (1128 + 612) / 4;
	else
		_musicIndexBase = 1128 / 4;
	_soundIndexBase = 1660 / 4;
	_frameCount = 1;
	_vgaBaseDelay = 1;
	_vgaPeriod = 45;
	_numBitArray1 = 16;
	_numBitArray2 = 16;
	_numItemStore = 10;
	_numTextBoxes = 20;
	_numVars = 255;

	_numMusic = 93;
	_numSFX = 222;
	_numSpeech = 11997;

	AGOSEngine::setupGame();
}

void AGOSEngine_Simon1::setupGame() {
	gss = &simon1_settings;
	_tableIndexBase = 1576 / 4;
	_textIndexBase = 1460 / 4;
	_numVideoOpcodes = 64;
	_vgaMemSize = 1000000;
	_itemMemSize = 20000;
	_tableMemSize = 50000;
	_musicIndexBase = 1316 / 4;
	_soundIndexBase = 0;
	_frameCount = 1;
	_vgaBaseDelay = 1;
	_vgaPeriod = 50;
	_numBitArray1 = 16;
	_numBitArray2 = 16;
	_numItemStore = 10;
	_numTextBoxes = 20;
	_numVars = 255;

	_numMusic = 34;
	_numSFX = 127;
	_numSpeech = 3623;

	AGOSEngine::setupGame();
}

void AGOSEngine_Waxworks::setupGame() {
	gss = &simon1_settings;
	_numVideoOpcodes = 64;
	_vgaMemSize = 1000000;
	_itemMemSize = 80000;
	_tableMemSize = 50000;
	_frameCount = 4;
	_vgaBaseDelay = 1;
	_vgaPeriod = 50;
	_numBitArray1 = 16;
	_numBitArray2 = 15;
	_numItemStore = 50;
	_numTextBoxes = 10;
	_numVars = 255;

	_numMusic = 26;

	AGOSEngine::setupGame();
}

void AGOSEngine_Elvira2::setupGame() {
	gss = &simon1_settings;
	_numVideoOpcodes = 60;
	_vgaMemSize = 1000000;
	_itemMemSize = 64000;
	_tableMemSize = 100000;
	_frameCount = 4;
	_vgaBaseDelay = 1;
	_vgaPeriod = 50;
	_numBitArray1 = 16;
	_numBitArray2 = 15;
	_numItemStore = 50;
	_numVars = 255;

	_numMusic = 9;

	AGOSEngine::setupGame();
}

void AGOSEngine_Elvira1::setupGame() {
	gss = &simon1_settings;
	_numVideoOpcodes = 57;
	_vgaMemSize = 1000000;
	_itemMemSize = 64000;
	_tableMemSize = 256000;
	_frameCount = 4;
	_vgaBaseDelay = 1;
	_vgaPeriod = 50;
	_numVars = 512;

	_numMusic = 14;

	AGOSEngine::setupGame();
}

#ifdef ENABLE_PN
void AGOSEngine_PN::setupGame() {
	gss = &simon1_settings;
	_numVideoOpcodes = 57;
	_vgaMemSize = 1000000;
	_frameCount = 4;
	_vgaBaseDelay = 1;
	_vgaPeriod = 50;
	_numVars = 256;

	AGOSEngine::setupGame();
}
#endif

void AGOSEngine::setupGame() {
	allocItemHeap();
	allocTablesHeap();

	if (getGameType() != GType_SIMON2)
		initMouse();

	_variableArray = (int16 *)calloc(_numVars, sizeof(int16));
	_variableArrayPtr = _variableArray;
	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		_variableArray2 = (int16 *)calloc(_numVars, sizeof(int16));
	}

	setupOpcodes();
	setupVgaOpcodes();

	setZoneBuffers();

	_currentMouseCursor = 255;
	_currentMouseAnim = 255;

	_lastMusicPlayed = -1;
	_nextMusicToPlay = -1;

	_noOverWrite = 0xFFFF;

	_stringIdLocalMin = 1;

	_agosMenu = 1;
	_superRoomNumber = 1;

	for (int i = 0; i < 20; i++) {
		if (getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) {
			_videoWindows[i] = initialVideoWindows_Simon[i];
		} else if (getGameType() == GType_PN) {
			_videoWindows[i] = initialVideoWindows_PN[i];
		} else {
			_videoWindows[i] = initialVideoWindows_Common[i];
		}
	}

	if (getGameType() == GType_ELVIRA2 && getPlatform() == Common::kPlatformAtariST) {
		_videoWindows[9] = 75;
	}
}

AGOSEngine::~AGOSEngine() {
	// In Simon 2, this gets deleted along with _sound further down
	if (getGameType() != GType_SIMON2)
		delete _gameFile;

	_midi.close();
	if (_driver)
		delete _driver;

	AudioCD.destroy();

	for (uint i = 0; i < _itemHeap.size(); i++) {
		delete[] _itemHeap[i];
	}
	_itemHeap.clear();

	free(_tablesHeapPtr - _tablesHeapCurPos);

	free(_mouseData);

	free(_gameOffsetsPtr);
	free(_iconFilePtr);
	free(_itemArrayPtr);
	free(_menuBase);
	free(_roomsList);
	free(_roomStates);
	free(_stringTabPtr);
	free(_strippedTxtMem);
	free(_tblList);
	free(_textMem);
	free(_xtblList);

	free(_backGroundBuf);
	free(_backBuf);
	free(_planarBuf);
	free(_scaleBuf);
	free(_zoneBuffers);

	free(_window4BackScn);
	free(_window6BackScn);

	free(_firstTimeStruct);
	free(_pendingDeleteTimeEvent);

	free(_variableArray);
	free(_variableArray2);

	delete _dummyItem1;
	delete _dummyItem2;
	delete _dummyItem3;

	delete _dummyWindow;
	delete[] _windowList;

	delete _debugger;
	delete _sound;
}

GUI::Debugger *AGOSEngine::getDebugger() {
	return _debugger;
}

void AGOSEngine::pauseEngineIntern(bool pauseIt) {
	if (pauseIt) {
		_keyPressed.reset();
		_pause = true;

		_midi.pause(true);
		_mixer->pauseAll(true);
	} else {
		_pause = false;

		_midi.pause(_musicPaused);
		_mixer->pauseAll(false);
	}
}

void AGOSEngine::pause() {
	pauseEngine(true);

	while (_pause && !shouldQuit()) {
		delay(1);
		if (_keyPressed.keycode == Common::KEYCODE_PAUSE) {
			pauseEngine(false);
			_keyPressed.reset();
		}
	}
}

Common::Error AGOSEngine::go() {
	loadGamePcFile();

	addTimeEvent(0, 1);

	if (getFileName(GAME_GMEFILE) != NULL) {
		openGameFile();
	}

	if (getGameType() == GType_FF) {
		loadIconData();
	} else if (getFileName(GAME_ICONFILE) != NULL) {
		loadIconFile();
	}

	if (getFileName(GAME_MENUFILE) != NULL) {
		loadMenuFile();
	}

	vc34_setMouseOff();

	if (getGameType() != GType_PP && getGameType() != GType_FF) {
		uint16 count = (getGameType() == GType_SIMON2) ? 5 : _frameCount;
		addVgaEvent(count, ANIMATE_INT, NULL, 0, 0);
	}

	if (getGameType() == GType_ELVIRA1 && getPlatform() == Common::kPlatformAtariST &&
		(getFeatures() & GF_DEMO)) {
		setWindowImage(3, 9900);
		while (!shouldQuit())
			delay(0);
	}

	if (getGameType() == GType_ELVIRA1 && getPlatform() == Common::kPlatformAmiga &&
		(getFeatures() & GF_DEMO)) {
		playMusic(0, 0);
	}

	runSubroutine101();
	permitInput();

	while (!shouldQuit()) {
		waitForInput();
		handleVerbClicked(_verbHitArea);
		delay(100);
	}

	return Common::kNoError;
}


uint32 AGOSEngine::getTime() const {
	return _system->getMillis() / 1000;
}

void AGOSEngine::syncSoundSettings() {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	if (_midiEnabled)
		_midi.setVolume(ConfMan.getInt("music_volume"), ConfMan.getInt("sfx_volume"));
}

} // End of namespace AGOS
