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
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/textconsole.h"

#include "engines/util.h"

#include "agos/debugger.h"
#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/midi.h"
#include "agos/sound.h"

#include "backends/audiocd/audiocd.h"

#include "graphics/surface.h"
#include "graphics/sjis.h"

#include "audio/mididrv.h"

namespace AGOS {

static const GameSpecificSettings simon1_settings = {
	"",                                     // base_filename
	"",                                     // restore_filename
	"",                                     // tbl_filename
	"EFFECTS",                              // effects_filename
	"SIMON",                                // speech_filename
};

static const GameSpecificSettings simon2_settings = {
	"",                                     // base_filename
	"",                                     // restore_filename
	"",                                     // tbl_filename
	"",                                     // effects_filename
	"SIMON2",                               // speech_filename
};

static const GameSpecificSettings dimp_settings = {
	"Gdimp",                                // base_filename
	"",                                     // restore_filename
	"",                                     // tbl_filename
	"",                                     // effects_filename
	"MUSIC",                                // speech_filename
};

static const GameSpecificSettings jumble_settings = {
	"Gjumble",                              // base_filename
	"",                                     // restore_filename
	"",                                     // tbl_filename
	"",                                     // effects_filename
	"MUSIC",                                // speech_filename
};

static const GameSpecificSettings puzzle_settings = {
	"Gpuzzle",                              // base_filename
	"",                                     // restore_filename
	"",                                     // tbl_filename
	"",                                     // effects_filename
	"MUSIC",                                // speech_filename
};

static const GameSpecificSettings swampy_settings = {
	"Gswampy",                              // base_filename
	"",                                     // restore_filename
	"",                                     // tbl_filename
	"",                                     // effects_filename
	"MUSIC",                                // speech_filename
};

#ifdef ENABLE_AGOS2
AGOSEngine_DIMP::AGOSEngine_DIMP(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine_PuzzlePack(system, gd) {

	_iconToggleCount = 0;
	_voiceCount = 0;

	_lastTickCount = 0;
	_startSecondCount = 0;
	_tSecondCount = 0;
}

AGOSEngine_PuzzlePack::AGOSEngine_PuzzlePack(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine_Feeble(system, gd) {

	_oopsValid = false;
	_gameTime = 0;
}
#endif

AGOSEngine_Simon2::AGOSEngine_Simon2(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine_Simon1(system, gd) {
}

AGOSEngine_Simon1::AGOSEngine_Simon1(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine_Waxworks(system, gd) {
}

AGOSEngine_Waxworks::AGOSEngine_Waxworks(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine_Elvira2(system, gd) {

	_boxCR = false;
	_boxLineCount = 0;
	memset(_boxBuffer, 0, sizeof(_boxBuffer));
	_boxBufferPtr = _boxBuffer;

	_linePtrs[0] = nullptr;
	_linePtrs[1] = nullptr;
	_linePtrs[2] = nullptr;
	_linePtrs[3] = nullptr;
	_linePtrs[4] = nullptr;
	_linePtrs[5] = nullptr;
	memset(_lineCounts, 0, sizeof(_lineCounts));
}

AGOSEngine_Elvira2::AGOSEngine_Elvira2(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine_Elvira1(system, gd) {
}

AGOSEngine_Elvira1::AGOSEngine_Elvira1(OSystem *system, const AGOSGameDescription *gd)
	: AGOSEngine(system, gd), _sjisCurChar(0), _sjisFont(nullptr) {
}

AGOSEngine_Elvira1::~AGOSEngine_Elvira1() {
	delete _sjisFont;
}

Common::Error AGOSEngine_Elvira1::init() {
	Common::Error ret = AGOSEngine::init();
	if (ret.getCode() == Common::kNoError && getPlatform() == Common::kPlatformPC98) {
		_sjisFont = Graphics::FontSJIS::createFont(Common::kPlatformPC98);
		if (_sjisFont)
			_sjisFont->toggleFatPrint(true);
		else
			error("AGOSEngine_Elvira1::init(): Failed to load SJIS font.");
	}
	return ret;
}

AGOSEngine::AGOSEngine(OSystem *system, const AGOSGameDescription *gd)
	: Engine(system), _rnd("agos"), _gameDescription(gd) {

	//Image dumping command disabled as it doesn't work well
#if 0
	DebugMan.addDebugChannel(kDebugImageDump, "image_dump", "Enable dumping of images to files");
#endif
	_vcPtr = nullptr;
	_vcGetOutOfCode = 0;
	_gameOffsetsPtr = nullptr;

	_gameFile = nullptr;
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
	_numZone = 0;

	_numBitArray1 = 0;
	_numBitArray2 = 0;
	_numBitArray3 = 0;
	_numItemStore = 0;
	_numTextBoxes = 0;
	_numVars = 0;
	_numVideoOpcodes = 0;
	_vgaBaseDelay = 0;
	_vgaPeriod = 0;

	_strippedTxtMem = nullptr;
	_textMem = nullptr;
	_textSize = 0;
	_stringTabNum = 0;
	_stringTabPos = 0;
	_stringTabSize = 0;
	_stringTabPtr = nullptr;

	_itemArrayPtr = nullptr;
	_itemArraySize = 0;
	_itemArrayInited = 0;

	_iconFilePtr = nullptr;

	_codePtr = nullptr;

	_localStringtable = nullptr;
	_stringIdLocalMin = 0;
	_stringIdLocalMax = 0;

	_roomStates = nullptr;
	_numRoomStates = 0;

	_menuBase = nullptr;
	_roomsList = nullptr;
	_roomsListPtr = nullptr;

	_xtblList = nullptr;
	_xtablesHeapPtrOrg = nullptr;
	_xtablesHeapCurPosOrg = 0;
	_xsubroutineListOrg = nullptr;

	_tblList = nullptr;
	_tablesHeapPtr = nullptr;
	_tablesHeapPtrOrg = nullptr;
	_tablesheapPtrNew = nullptr;
	_tablesHeapSize = 0;
	_tablesHeapCurPos = 0;
	_tablesHeapCurPosOrg = 0;
	_tablesHeapCurPosNew = 0;
	_subroutineListOrg = nullptr;

	_subroutineList = nullptr;

	_recursionDepth = 0;

	_lastVgaTick = 0;

	_marks = 0;
	_scanFlag = false;

	_scriptVar2 = 0;
	_runScriptReturn1 = false;
	_skipVgaWait = false;
	_noParentNotify = false;
	_beardLoaded = false;
	_litBoxFlag = false;
	_mortalFlag = false;
	_displayFlag = 0;
	_syncFlag2 = false;
	_inCallBack = false;
	_cepeFlag = false;
	_fastMode = false;

	_backFlag = false;

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
	_mouseData = nullptr;
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
	_scrollImage = nullptr;
	_boxStarHeight = 0;

	_scriptVerb = 0;
	_scriptNoun1 = 0;
	_scriptNoun2 = 0;
	_scriptAdj1 = 0;
	_scriptAdj2 = 0;

	_curWindow = 0;
	_textWindow = nullptr;

	_subjectItem = nullptr;
	_objectItem = nullptr;
	_currentPlayer = nullptr;

	_iOverflow = false;
	_nameLocked = false;
	_hitAreaObjectItem = nullptr;
	_lastHitArea = nullptr;
	_lastNameOn = nullptr;
	_lastHitArea3 = nullptr;
	_hitAreaSubjectItem = nullptr;
	_currentBox = nullptr;
	_currentBoxNum = 0;
	_currentVerbBox = nullptr;
	_lastVerbOn = nullptr;
	_needHitAreaRecalc = 0;
	_verbHitArea = 0;
	_defaultVerb = 0;
	_mouseHideCount = 0;

	_dragAccept = false;
	_dragEnd = false;
	_dragFlag = false;
	_dragMode = false;
	_dragCount = 0;
	_lastClickRem = nullptr;

	_windowNum = 0;

	_printCharCurPos = 0;
	_printCharMaxPos = 0;
	_printCharPixelCount = 0;
	_numLettersToPrint = 0;

	_clockStopped = 0;
	_gameStoppedClock = 0;
	_lastTime = 0;
	_lastMinute = 0;

	_firstTimeStruct = nullptr;
	_pendingDeleteTimeEvent = nullptr;

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
	_fastFadeOutFlag = false;
	_neverFade = false;
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

	_block = nullptr;
	_blockEnd = nullptr;
	_vgaMemPtr = nullptr;
	_vgaMemEnd = nullptr;
	_vgaMemBase = nullptr;
	_vgaFrozenBase = nullptr;
	_vgaRealBase = nullptr;
	_zoneBuffers = nullptr;

	_curVgaFile1 = nullptr;
	_curVgaFile2 = nullptr;
	_curSfxFile = nullptr;
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

	_nextVgaTimerToProcess = nullptr;

	_opcode177Var1 = 1;
	_opcode177Var2 = 0;
	_opcode178Var1 = 1;
	_opcode178Var2 = 0;

	_classLine = nullptr;
	_classMask = 0;
	_classMode1 = 0;
	_classMode2 = 0;
	_currentLine = nullptr;
	_currentTable = nullptr;
	_findNextPtr = nullptr;

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

	_variableArray = nullptr;
	_variableArray2 = nullptr;
	_variableArrayPtr = nullptr;

	memset(_windowArray, 0, sizeof(_windowArray));

	memset(_fcsData1, 0, sizeof(_fcsData1));
	memset(_fcsData2, 0, sizeof(_fcsData2));

	_awaitTwoByteToken = 0;
	_byteTokens = nullptr;
	_byteTokenStrings = nullptr;
	_twoByteTokens = nullptr;
	_twoByteTokenStrings = nullptr;
	_secondTwoByteTokenStrings = nullptr;
	_thirdTwoByteTokenStrings = nullptr;
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

	_planarBuf = nullptr;
	_pak98Buf = nullptr;
	_paletteModNext = 16;

	_midiEnabled = false;

	_vgaTickCounter = 0;

	_sound = nullptr;

	_effectsMuted = false;
	_ambientMuted = false;
	_musicMuted = false;
	// Initialize at default ScummVM volumes; these will be overwritten by
	// syncSoundSettings.
	_musicVolume = 192;
	_effectsVolume = 192;
	_useDigitalSfx = true;

	_saveLoadType = 0;
	_saveLoadSlot = 0;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));
	memset(_saveBuf, 0, sizeof(_saveBuf));

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

	_backGroundBuf = nullptr;
	_backBuf = nullptr;
	_scaleBuf = nullptr;
	_window4BackScn = nullptr;
	_window6BackScn = nullptr;

	_window3Flag = 0;
	_window4Flag = 0;
	_window6Flag = 0;

	_moveXMin = 0;
	_moveYMin = 0;
	_moveXMax = 0;
	_moveYMax = 0;

	_forceAscii = false;

	_vc10BasePtrOld = nullptr;
	memcpy (_hebrewCharWidths,
		"\x5\x5\x4\x6\x5\x3\x4\x5\x6\x3\x5\x5\x4\x6\x5\x3\x4\x6\x5\x6\x6\x6\x5\x5\x5\x6\x5\x6\x6\x6\x6\x6", 32);

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	// Add default file directories for Acorn version of
	// Simon the Sorcerer 1
	SearchMan.addSubDirectoryMatching(gameDataDir, "execute");

	// Add default file directories for Amiga/Macintosh
	// versions of Simon the Sorcerer 2
	SearchMan.addSubDirectoryMatching(gameDataDir, "voices");

	// Add default file directories for Amiga & Macintosh
	// versions of The Feeble Files
	SearchMan.addSubDirectoryMatching(gameDataDir, "gfx");
	SearchMan.addSubDirectoryMatching(gameDataDir, "movies");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sfx");
	SearchMan.addSubDirectoryMatching(gameDataDir, "speech");
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

	_internalWidth = _screenWidth;
	_internalHeight = _screenHeight;

	if (getPlatform() == Common::kPlatformPC98) {
		_internalWidth <<= 1;
		_internalHeight <<= 1;
	}

	if (ConfMan.hasKey("disable_fade_effects"))
		_neverFade = ConfMan.getBool("disable_fade_effects");

	initGraphics(_internalWidth, _internalHeight);

	_midi = new MidiPlayer(this);

	if ((getGameType() == GType_SIMON2 && getPlatform() == Common::kPlatformWindows) ||
		(getGameType() == GType_SIMON1 && getPlatform() == Common::kPlatformWindows) ||
		((getFeatures() & GF_TALKIE) && getPlatform() == Common::kPlatformAcorn) ||
		(getPlatform() == Common::kPlatformDOS || getPlatform() == Common::kPlatformPC98)) {

		int ret = _midi->open();
		if (ret)
			warning("MIDI Player init failed: \"%s\"", MidiDriver::getErrorName(ret));

		_midiEnabled = true;
	}
	// Digital SFX are used if MIDI SFX are not available or if the "prefer
	// digital SFX" setting is set to true or is not present at all.
	// Two exceptions to this are:
	// - Elvira 2 DOS needs an optional file to enable digital SFX. If it is
	//   not present, MIDI SFX are used.
	// - Simon 1 DOS floppy has only MIDI SFX.
	// Note that MIDI SFX can be safely used if the MidiPlayer failed to
	// initialize; they just will not play.
	_useDigitalSfx = !_midiEnabled || !_midi->hasMidiSfx() || !ConfMan.hasKey("prefer_digitalsfx") || ConfMan.getBool("prefer_digitalsfx");
	if ((getGameType() == GType_ELVIRA2 && getPlatform() == Common::kPlatformDOS && !SearchMan.hasFile("013.VGA")) ||
			(getGameType() == GType_SIMON1 && getPlatform() == Common::kPlatformDOS && !(getFeatures() & GF_TALKIE))) {
		_useDigitalSfx = false;
	}
	if (!_useDigitalSfx && (getGameType() == GType_ELVIRA2 || getGameType() == GType_WW) && getPlatform() == Common::kPlatformDOS) {
		// Load the MIDI SFX data file for Elvira 2 and Waxworks DOS.
		loadMidiSfx();
	}

	// allocate buffers
	_backGroundBuf = new Graphics::Surface();
	_backGroundBuf->create(_screenWidth, _screenHeight, Graphics::PixelFormat::createFormatCLUT8());

	if (getGameType() == GType_FF || getGameType() == GType_PP || (getGameType() == GType_ELVIRA1 && getPlatform() == Common::kPlatformPC98)) {
		_backBuf = new Graphics::Surface();
		_backBuf->create(_screenWidth, _screenHeight, Graphics::PixelFormat::createFormatCLUT8());
		_scaleBuf = new Graphics::Surface();
		_scaleBuf->create(_internalWidth, _internalHeight, Graphics::PixelFormat::createFormatCLUT8());
	}

	if (getGameType() == GType_SIMON2) {
		_window4BackScn = new Graphics::Surface();
		_window4BackScn->create(_screenWidth, _screenHeight, Graphics::PixelFormat::createFormatCLUT8());
	} else if (getGameType() == GType_SIMON1) {
		_window4BackScn = new Graphics::Surface();
		_window4BackScn->create(_screenWidth, 134, Graphics::PixelFormat::createFormatCLUT8());
	} else if (getGameType() == GType_WW || getGameType() == GType_ELVIRA2) {
		_window4BackScn = new Graphics::Surface();
		_window4BackScn->create(224, 127, Graphics::PixelFormat::createFormatCLUT8());
	} else if (getGameType() == GType_ELVIRA1) {
		_window4BackScn = new Graphics::Surface();
		if (getPlatform() == Common::kPlatformAmiga && (getFeatures() & GF_DEMO)) {
			_window4BackScn->create(224, 196, Graphics::PixelFormat::createFormatCLUT8());
		} else {
			_window4BackScn->create(224, 144, Graphics::PixelFormat::createFormatCLUT8());
		}
		_window6BackScn = new Graphics::Surface();
		_window6BackScn->create(48, 80, Graphics::PixelFormat::createFormatCLUT8());
	}

	setupGame();

	setDebugger(new Debugger(this));
	_sound = new Sound(this, gss, _mixer);

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

	// Setup mixer
	syncSoundSettings();

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

#ifdef ENABLE_AGOS2
void AGOSEngine_PuzzlePack::setupGame() {
	if (getGameId() == GID_DIMP) {
		gss = &dimp_settings;
	} else if (getGameId() == GID_JUMBLE) {
		gss = &jumble_settings;
	} else if (getGameId() == GID_PUZZLE) {
		gss = &puzzle_settings;
	} else if (getGameId() == GID_SWAMPY) {
		gss = &swampy_settings;
	}
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

	_numZone = 450;

	AGOSEngine::setupGame();
}
#endif

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
	if (getGameType() == GType_SIMON2 && getPlatform() == Common::kPlatformDOS && _midi->usesMT32Data())
		_musicIndexBase = MUSIC_INDEX_BASE_SIMON2_MT32;
	else
		_musicIndexBase = MUSIC_INDEX_BASE_SIMON2_GM;
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
	_numZone = 140;

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
	_numZone = 164;

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
	_numZone = 155;

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
	_numZone = 99;

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
	_numZone = 74;

	AGOSEngine::setupGame();
}

void AGOSEngine_PN::setupGame() {
	gss = &simon1_settings;
	_numVideoOpcodes = 57;
	_vgaMemSize = 1000000;
	_frameCount = 4;
	_vgaBaseDelay = 1;
	_vgaPeriod = 50;
	_numVars = 256;

	_numZone = 26;

	AGOSEngine::setupGame();
}

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
	_system->getAudioCDManager()->stop();

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

	if (_backGroundBuf)
		_backGroundBuf->free();
	delete _backGroundBuf;
	if (_backBuf)
		_backBuf->free();
	delete _backBuf;
	free(_planarBuf);
	delete[] _pak98Buf;
	if (_scaleBuf)
		_scaleBuf->free();
	delete _scaleBuf;
	free(_zoneBuffers);

	if (_window4BackScn)
		_window4BackScn->free();
	delete _window4BackScn;
	if (_window6BackScn)
		_window6BackScn->free();
	delete _window6BackScn;

	delete _midi;

	free(_firstTimeStruct);
	free(_pendingDeleteTimeEvent);

	free(_variableArray);
	free(_variableArray2);

	delete _dummyItem1;
	delete _dummyItem2;
	delete _dummyItem3;

	delete _dummyWindow;
	delete[] _windowList;

	delete _sound;
	delete _gameFile;
}

void AGOSEngine::pauseEngineIntern(bool pauseIt) {
	if (pauseIt) {
		_keyPressed.reset();
		_pause = true;

		_midi->pause(true);
		_mixer->pauseAll(true);
	} else {
		_pause = false;

		_midi->pause(false);
		_mixer->pauseAll(false);
	}
}

void AGOSEngine::pause() {
	PauseToken pt = pauseEngine();

	while (_pause && !shouldQuit()) {
		delay(1);
		if (_keyPressed.keycode == Common::KEYCODE_PAUSE) {
			pt.clear();
			_keyPressed.reset();
		}
	}
}

Common::Error AGOSEngine::go() {
#ifdef ENABLE_AGOS2
	loadArchives();
#endif

	loadGamePcFile();

	addTimeEvent(0, 1);

	if (getFileName(GAME_GMEFILE) != nullptr) {
		openGameFile();
	}

	if (getGameType() == GType_FF) {
		loadIconData();
	} else if (getFileName(GAME_ICONFILE) != nullptr) {
		loadIconFile();
	}

	if (getFileName(GAME_MENUFILE) != nullptr) {
		loadMenuFile();
	}

	vc34_setMouseOff();

	if (getGameType() != GType_PP && getGameType() != GType_FF) {
		uint16 count = (getGameType() == GType_SIMON2) ? 5 : _frameCount;
		addVgaEvent(count, ANIMATE_INT, nullptr, 0, 0);
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
	Engine::syncSoundSettings();

	int newMusicVolume = ConfMan.getInt("music_volume");
	int newEffectsVolume = ConfMan.getInt("sfx_volume");

	_musicMuted = newMusicVolume == 0;
	if (newMusicVolume != 0)
		_musicVolume = newMusicVolume;
	if (getGameType() == GType_SIMON2) {
		// Simon 2 has regular and ambient SFX, which can be toggled on and off
		// separately.
		if (newEffectsVolume == 0) {
			// Global SFX volume 0 mutes both regular and ambient SFX.
			_effectsMuted = _ambientMuted = true;
		} else {
			// If global SFX volume is > 0 and both regular and ambient SFX are
			// muted, unmute them. If only one of them is muted, the volume
			// change will only affect that type of SFX. If both are not muted,
			// it will affect both types.
			if (_effectsMuted && _ambientMuted)
				_effectsMuted = _ambientMuted = false;
			_effectsVolume = newEffectsVolume;
		}
		// Engine::syncSoundSettings applies SFX volume to all SFX handles,
		// so manage the regular and ambient handles separately here.
		_sound->effectsMute(_effectsMuted, _effectsVolume);
		_sound->ambientMute(_ambientMuted, _effectsVolume);
	} else {
		// Other games only have one SFX setting.
		_effectsMuted = newEffectsVolume == 0;
		if (newEffectsVolume != 0)
			_effectsVolume = newEffectsVolume;
	}
	_speech = !ConfMan.getBool("speech_mute");

	if (_midiEnabled)
		_midi->syncSoundSettings();
}

void AGOSEngine::syncSoundSettingsIntern() {
	ConfMan.setBool("speech_mute", !_speech);
	ConfMan.setInt("music_volume", _musicMuted ? 0 : _musicVolume);
	bool sfxMute = getGameType() == GType_SIMON2 ?
		_effectsMuted && _ambientMuted : _effectsMuted;
	ConfMan.setInt("sfx_volume", sfxMute ? 0 : _effectsVolume);

	Engine::syncSoundSettings();

	if (getGameType() == GType_SIMON2) {
		// Simon 2 has ambient sound effects, which can be toggled on and off
		// separately from the other SFX.
		// Engine::syncSoundSettings applies SFX volume to all SFX handles,
		// so manage the regular and ambient handles separately here.
		_sound->effectsMute(_effectsMuted, _effectsVolume);
		_sound->ambientMute(_ambientMuted, _effectsVolume);
	}

	if (_midiEnabled)
		_midi->syncSoundSettings();
}

} // End of namespace AGOS
