/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"

#include "gui/about.h"

#include "simon/debugger.h"
#include "simon/intern.h"
#include "simon/simon.h"
#include "simon/vga.h"

#include "sound/mididrv.h"
#ifdef _WIN32_WCE
extern bool isSmartphone(void);
#endif

#ifdef PALMOS_68K
#include "globals.h"
#endif

using Common::File;

namespace Simon {

#ifdef PALMOS_68K
#define PTR(a) a
static const GameSpecificSettings *simon1_settings;
static const GameSpecificSettings *simon2_settings;
static const GameSpecificSettings *feeblefiles_settings;
#else
#define PTR(a) &a
static const GameSpecificSettings simon1_settings = {
	"EFFECTS",                              // effects_filename
	"SIMON",                                // speech_filename
};

static const GameSpecificSettings simon2_settings = {
	"",                                     // effects_filename
	"SIMON2",                               // speech_filename
};

static const GameSpecificSettings feeblefiles_settings = {
	"",                                     // effects_filename
	"VOICES",                               // speech_filename
};
#endif

SimonEngine::SimonEngine(OSystem *syst)
	: Engine(syst), midi(syst) {
	_vcPtr = 0;
	_vc_get_out_of_code = 0;
	_gameOffsetsPtr = 0;

	_debugger = 0;
	setupVgaOpcodes();

	_keyPressed = 0;

	_gameFile = 0;

	_strippedTxtMem = 0;
	_textSize = 0;
	_stringTabNum = 0;
	_stringTabPos = 0;
	_stringtab_numalloc = 0;
	_stringTabPtr = 0;

	_itemArrayPtr = 0;
	_itemArraySize = 0;
	_itemArrayInited = 0;

	_itemHeapPtr = 0;
	_itemHeapCurPos = 0;
	_itemHeapSize = 0;

	_iconFilePtr = 0;

	_tblList = 0;

	_codePtr = 0;

	_localStringtable = 0;
	_stringIdLocalMin = 0;
	_stringIdLocalMax = 0;

	_tablesHeapPtr = 0;
	_tablesHeapPtrOrg = 0;
	_tablesheapPtrNew = 0;
	_tablesHeapSize = 0;
	_tablesHeapCurPos = 0;
	_tablesHeapCurPosOrg = 0;
	_tablesHeapCurPosNew = 0;

	_subroutineList = 0;
	_subroutineListOrg = 0;
	_subroutine = 0;

	_dxSurfacePitch = 0;

	_recursionDepth = 0;

	_lastVgaTick = 0;

	_marks = 0;

	_scriptVar2 = 0;
	_runScriptReturn1 = 0;
	_skipVgaWait = 0;
	_noParentNotify = 0;
	_beardLoaded = 0;
	_hitarea_unk_3 = 0;
	_mortalFlag = 0;
	_updateScreen = false;
	_usePaletteDelay = 0;
	_syncFlag2 = 0;
	_inCallBack = 0;
	_cepeFlag = 0;
	_copyPartialMode = 0;
	_fastMode = 0;
	_useBackGround = 0;

	_debugMode = 0;
	_startMainScript = false;
	_continousMainScript = false;
	_startVgaScript = false;
	_continousVgaScript = false;
	_drawImagesDebug = false;
	_dumpImages = false;

	_pause = false;
	_speech = false;
	_subtitles = false;

	_animatePointer = 0;
	_mouseCursor = 0;
	_mouseAnim = 0;
	_mouseAnimMax = 0;
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

	_currentBoxNumber = 0;
	_iOverflow = 0;
	_hitAreaObjectItem = 0;
	_lastHitArea = 0;
	_lastNameOn = 0;
	_lastHitArea3 = 0;
	_hitAreaSubjectItem = 0;
	_currentVerbBox = 0;
	_lastVerbOn = 0;
	_needHitAreaRecalc = 0;
	_verbHitArea = 0;
	_defaultVerb = 0;
	_mouseHideCount = 0;

	_windowNum = 0;

	_printCharCurPos = 0;
	_printCharMaxPos = 0;
	_printCharPixelCount = 0;
	_numLettersToPrint = 0;

	_numTextBoxes = 0;

	_clockStopped = 0;
	_gameStoppedClock = 0;
	_lastTime = 0;

	_firstTimeStruct = 0;
	_pendingDeleteTimeEvent = 0;

	_mouseX = 0;
	_mouseY = 0;
	_mouseXOld = 0;
	_mouseYOld = 0;

	_leftButtonDown = 0;
	_rightButtonDown = 0;
	_noRightClick = false;

	_dummyItem1 = new Item();
	_dummyItem2 = new Item();
	_dummyItem3 = new Item();

	_lockWord = 0;
	_scrollUpHitArea = 0;
	_scrollDownHitArea = 0;

	_paletteColorCount = 0;

	_noOverWrite = 0;
	_rejectCount = 0;
	_rejectBlock = false;

	_fastFadeOutFlag = 0;
	_unkPalFlag = 0;
	_exitCutscene = 0;
	_paletteFlag = 0;

	_soundFileId = 0;
	_lastMusicPlayed = 0;
	_nextMusicToPlay = 0;

	_showPreposition = 0;
	_showMessageFlag = 0;

	_videoNumPalColors = 0;

	_vgaSpriteChanged = 0;

	_vgaBufFreeStart = 0;
	_vgaBufEnd = 0;
	_vgaBufStart = 0;
	_vgaFileBufOrg = 0;
	_vgaFileBufOrg2 = 0;

	_curVgaFile1 = 0;
	_curVgaFile2 = 0;
	_curSfxFile = 0;

	_syncCount = 0;
	_timer5 = 0;
	_timer4 = 0;

	_frameRate = 0;

	_zoneNumber = 0;
	_vgaWaitFor = 0;
	_vgaCurZoneNum = 0;
	_vgaCurSpriteId = 0;
	_vgaCurSpritePriority = 0;

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

	memset(_objectArray, 0, sizeof(_objectArray));
	memset(_itemStore, 0, sizeof(_itemStore));

	memset(_stringIdArray2, 0, sizeof(_stringIdArray2));
	memset(_stringIdArray3, 0, sizeof(_stringIdArray3));
	memset(_speechIdArray4, 0, sizeof(_speechIdArray4));

	memset(_bitArray, 0, sizeof(_bitArray));
	memset(_bitArrayTwo, 0, sizeof(_bitArrayTwo));
	memset(_bitArrayThree, 0, sizeof(_bitArrayThree));

	memset(_variableArray, 0, sizeof(_variableArray));
	memset(_variableArray2, 0, sizeof(_variableArray2));
	_variableArrayPtr = 0;

	memset(_windowArray, 0, sizeof(_windowArray));

	memset(_fcsData1, 0, sizeof(_fcsData1));
	memset(_fcsData2, 0, sizeof(_fcsData2));

	_freeStringSlot = 0;

	memset(_stringReturnBuffer, 0, sizeof(_stringReturnBuffer));

	memset(_pathFindArray, 0, sizeof(_pathFindArray));

	memset(_pathValues, 0, sizeof(_pathValues));
	_PVCount = 0;
	_GPVCount = 0;

	memset(_pathValues1, 0, sizeof(_pathValues1));
	_PVCount1 = 0;
	_GPVCount1 = 0;

	memset(_paletteBackup, 0, sizeof(_paletteBackup));
	memset(_palette, 0, sizeof(_palette));

	memset(_videoBuf1, 0, sizeof(_videoBuf1));

	_windowList = new WindowBlock[16];

	memset(_lettersToPrintBuf, 0, sizeof(_lettersToPrintBuf));

	_vgaTickCounter = 0;

	_moviePlay = 0;
	_sound = 0;

	_effectsPaused = false;
	_ambientPaused = false;
	_musicPaused = false;

	_saveLoadType = 0;
	_saveLoadSlot = 0;
	memset(_saveLoadName, 0, sizeof(_saveLoadName));

	_saveLoadRowCurPos = 0;
	_numSaveGameRows = 0;
	_saveDialogFlag = false;
	_saveOrLoad = false;
	_saveLoadFlag = false;

	_hyperLink = 0;
	_interactY = 0;
	_oracleMaxScrollY = 0;
	_noOracleScroll = 0;

	_sdlMouseX = 0;
	_sdlMouseY = 0;

	_backGroundBuf = 0;
	_frontBuf = 0;
	_backBuf = 0;
	_scaleBuf = 0;

	_vc10BasePtrOld = 0;
	memcpy (_hebrewCharWidths,
		"\x5\x5\x4\x6\x5\x3\x4\x5\x6\x3\x5\x5\x4\x6\x5\x3\x4\x6\x5\x6\x6\x6\x5\x5\x5\x6\x5\x6\x6\x6\x6\x6", 32);


	// Add default file directories for Acorn version of
	// Simon the Sorcerer 1
	File::addDefaultDirectory(_gameDataPath + "execute/");
	File::addDefaultDirectory(_gameDataPath + "EXECUTE/");

	// Add default file directories for Amiga/Macintosh
	// verisons of Simon the Sorcerer 2
	File::addDefaultDirectory(_gameDataPath + "voices/");
	File::addDefaultDirectory(_gameDataPath + "VOICES/");

	// Add default file directories for Amiga & Macintosh
	// versions of The Feeble Files
	File::addDefaultDirectory(_gameDataPath + "gfx/");
	File::addDefaultDirectory(_gameDataPath + "GFX/");
	File::addDefaultDirectory(_gameDataPath + "movies/");
	File::addDefaultDirectory(_gameDataPath + "MOVIES/");
	File::addDefaultDirectory(_gameDataPath + "sfx/");
	File::addDefaultDirectory(_gameDataPath + "SFX/");
	File::addDefaultDirectory(_gameDataPath + "speech/");
	File::addDefaultDirectory(_gameDataPath + "SPEECH/");
}

int SimonEngine::init() {
	// Detect game
	if (!initGame()) {
		GUIErrorMessage("No valid games were found in the specified directory.");
		return -1;
	}

	setupOpcodes();

	if (getGameType() == GType_FF) {
		_screenWidth = 640;
		_screenHeight = 480;
	} else {
		_screenWidth = 320;
		_screenHeight = 200;
	}

	_system->beginGFXTransaction();
		initCommonGFX(getGameType() == GType_FF);
		_system->initSize(_screenWidth, _screenHeight);
	_system->endGFXTransaction();

	// FIXME Use auto dirty rects cleanup code to reduce CPU usage
	g_system->setFeatureState(OSystem::kFeatureAutoComputeDirtyRects, true);

	// Setup mixer
	if (!_mixer->isReady())
		warning("Sound initialization failed. "
						"Features of the game that depend on sound synchronization will most likely break");
	set_volume(ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	// Setup midi driver
	MidiDriver *driver = 0;
	if (getGameType() == GType_FF || getGameId() == GID_SIMON1CD32) {
		driver = MidiDriver::createMidi(MD_NULL);
		_native_mt32 = false;
	} else {
		int midiDriver = MidiDriver::detectMusicDriver(MDT_ADLIB | MDT_MIDI);
		_native_mt32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
		driver = MidiDriver::createMidi(midiDriver);
		if (_native_mt32) {
			driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
		}
	}

	midi.mapMT32toGM (getGameType() == GType_SIMON1 && !_native_mt32);

	midi.set_driver(driver);
	int ret = midi.open();
	if (ret)
		warning ("MIDI Player init failed: \"%s\"", midi.getErrorName (ret));
	midi.set_volume(ConfMan.getInt("music_volume"));

	if (ConfMan.hasKey("music_mute") && ConfMan.getBool("music_mute") == 1)
		midi.pause(_musicPaused ^= 1);

	_currentMouseCursor = 255;
	_currentMouseAnim = 255;

	_frameRate = 1;

	_lastMusicPlayed = -1;
	_nextMusicToPlay = -1;

	_noOverWrite = 0xFFFF;

	_stringIdLocalMin = 1;

	_variableArrayPtr = _variableArray;

	if (getGameType() == GType_FF) {
		gss = PTR(feeblefiles_settings);
		_numTextBoxes = 40;
		_numVideoOpcodes = 85;
#ifndef PALMOS_68K
		_vgaMemSize = 7500000;
#else
		_vgaMemSize = gVars->memory[kMemSimon2Games];
#endif
		_tableMemSize = 200000;
		_vgaBaseDelay = 5;
	} else if (getGameType() == GType_SIMON2) {
		gss = PTR(simon2_settings);
		_tableIndexBase = 1580 / 4;
		_textIndexBase = 1500 / 4;
		_numTextBoxes = 20;
		_numVideoOpcodes = 75;
#ifndef PALMOS_68K
		_vgaMemSize = 2000000;
#else
		_vgaMemSize = gVars->memory[kMemSimon2Games];
#endif
		_tableMemSize = 100000;
		// Check whether to use MT-32 MIDI tracks in Simon the Sorcerer 2
		if ((getGameType() == GType_SIMON2) && _native_mt32)
			_musicIndexBase = (1128 + 612) / 4;
		else
			_musicIndexBase = 1128 / 4;
		_soundIndexBase = 1660 / 4;
		_vgaBaseDelay = 1;
	} else {
		gss = PTR(simon1_settings);
		_tableIndexBase = 1576 / 4;
		_textIndexBase = 1460 / 4;
		_numTextBoxes = 20;
		_numVideoOpcodes = 64;
#ifndef PALMOS_68K
		_vgaMemSize = 1000000;
#else
		_vgaMemSize = gVars->memory[kMemSimon1Games];
#endif
		_tableMemSize = 50000;
		_musicIndexBase = 1316 / 4;
		_soundIndexBase = 0;
		_vgaBaseDelay = 1;
	}

	// allocate buffers
	_backGroundBuf = (byte *)calloc(_screenWidth * _screenHeight, 1);
	_frontBuf = (byte *)calloc(_screenWidth * _screenHeight, 1);
	_backBuf = (byte *)calloc(_screenWidth * _screenHeight, 1);
	if (getGameType() == GType_FF)
		_scaleBuf = (byte *)calloc(_screenWidth * _screenHeight, 1);

	allocItemHeap();
	allocTablesHeap();

	setZoneBuffers();

	_debugger = new Debugger(this);
	_moviePlay = new MoviePlayer(this, _mixer);
	_sound = new Sound(this, gss, _mixer);

	if (ConfMan.hasKey("sfx_mute") && ConfMan.getBool("sfx_mute") == 1) {
		if (getGameId() == GID_SIMON1DOS)
			midi._enable_sfx ^= 1;
		else
			_sound->effectsPause(_effectsPaused ^= 1);
	}

	_language = Common::parseLanguage(ConfMan.get("language"));

	if (getFeatures() & GF_TALKIE) {
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

	_debugMode = (gDebugLevel >= 0);
	if (gDebugLevel == 2)
		_continousMainScript = true;
	if (gDebugLevel == 3)
		_continousVgaScript = true;
	if (gDebugLevel == 4)
		_startMainScript = true;
	if (gDebugLevel == 5)
		_startVgaScript = true;

	return 0;
}

SimonEngine::~SimonEngine() {
	delete _gameFile;

	midi.close();

	free(_itemHeapPtr - _itemHeapCurPos);
	free(_tablesHeapPtr - _tablesHeapCurPos);

	free(_gameOffsetsPtr);
	free(_iconFilePtr);
	free(_itemArrayPtr);
	free(_stringTabPtr);
	free(_strippedTxtMem);
	free(_tblList);
	free(_textMem);

	free(_backGroundBuf);
	free(_frontBuf);
	free(_backBuf);
	free(_scaleBuf);

	delete _dummyItem1;
	delete _dummyItem2;
	delete _dummyItem3;

	delete [] _windowList;

	delete _debugger;
	delete _moviePlay;
	delete _sound;
}

void SimonEngine::errorString(const char *buf1, char *buf2) {
	strcpy(buf2, buf1);

#ifdef _WIN32_WCE
	if (isSmartphone())
		return;
#endif

	// Unless an error -originated- within the debugger, spawn the
	// debugger. Otherwise exit out normally.
	if (_debugger && !_debugger->isAttached()) {
		// (Print it again in case debugger segfaults)
		printf("%s\n", buf2);
		_debugger->attach(buf2);
		_debugger->onFrame();
	}
}

void SimonEngine::paletteFadeOut(uint32 *pal_values, uint num) {
	byte *p = (byte *)pal_values;

	do {
		if (p[0] >= 8)
			p[0] -= 8;
		else
			p[0] = 0;
		if (p[1] >= 8)
			p[1] -= 8;
		else
			p[1] = 0;
		if (p[2] >= 8)
			p[2] -= 8;
		else
			p[2] = 0;
		p += 4;
	} while (--num);
}

byte *SimonEngine::allocateItem(uint size) {
	byte *org = _itemHeapPtr;
	size = (size + 3) & ~3;

	_itemHeapPtr += size;
	_itemHeapCurPos += size;

	if (_itemHeapCurPos > _itemHeapSize)
		error("Itemheap overflow");

	return org;
}

void SimonEngine::setUserFlag(Item *item, int a, int b) {
	SubUserFlag *subUserFlag;

	subUserFlag = (SubUserFlag *) findChildOfType(item, 9);
	if (subUserFlag == NULL) {
		subUserFlag = (SubUserFlag *) allocateChildBlock(item, 9, sizeof(SubUserFlag));
	}

	if (a >= 0 && a <= 3)
		subUserFlag->userFlags[a] = b;
}

void SimonEngine::createPlayer() {
	Child *child;

	_currentPlayer = _itemArrayPtr[1];
	_currentPlayer->adjective = -1;
	_currentPlayer->noun = 10000;

	child = (Child *)allocateChildBlock(_currentPlayer, 3, sizeof(Child));
	if (child == NULL)
		error("player create failure");

	setUserFlag(_currentPlayer, 0, 0);
}

Child *SimonEngine::findChildOfType(Item *i, uint type) {
	Child *child = i->children;
	for (; child; child = child->next)
		if (child->type == type)
			return child;
	return NULL;
}

bool SimonEngine::isRoom(Item *item) {
	return findChildOfType(item, 1) != NULL;
}

bool SimonEngine::isObject(Item *item) {
	return findChildOfType(item, 2) != NULL;
}

uint SimonEngine::getOffsetOfChild2Param(SubObject *child, uint prop) {
	uint m = 1;
	uint offset = 0;
	while (m != prop) {
		if (child->objectFlags & m)
			offset++;
		m *= 2;
	}
	return offset;
}

Child *SimonEngine::allocateChildBlock(Item *i, uint type, uint size) {
	Child *child = (Child *)allocateItem(size);
	child->next = i->children;
	i->children = child;
	child->type = type;
	return child;
}

void SimonEngine::allocItemHeap() {
	_itemHeapSize = 10000;
	_itemHeapCurPos = 0;
	_itemHeapPtr = (byte *)calloc(10000, 1);
}

void SimonEngine::allocTablesHeap() {
	_tablesHeapSize = _tableMemSize;
	_tablesHeapCurPos = 0;
	_tablesHeapPtr = (byte *)calloc(_tableMemSize, 1);
}

void SimonEngine::setItemState(Item *item, int value) {
	item->state = value;
}

byte SimonEngine::getByte() {
	return *_codePtr++;
}

int SimonEngine::getNextWord() {
	int16 a = (int16)READ_BE_UINT16(_codePtr);
	_codePtr += 2;
	return a;
}

uint SimonEngine::getNextStringID() {
	return (uint16)getNextWord();
}

uint SimonEngine::getVarOrByte() {
	uint a = *_codePtr++;
	if (a != 255)
		return a;
	return readVariable(*_codePtr++);
}

uint SimonEngine::getVarOrWord() {
	uint a = READ_BE_UINT16(_codePtr);
	_codePtr += 2;
	if (a >= 30000 && a < 30512) {
		return readVariable(a - 30000);
	}
	return a;
}

Item *SimonEngine::getNextItemPtr() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subjectItem;
	case -3:
		return _objectItem;
	case -5:
		return me();
	case -7:
		return actor();
	case -9:
		return derefItem(me()->parent);
	default:
		return derefItem(a);
	}
}

Item *SimonEngine::getNextItemPtrStrange() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return _subjectItem;
	case -3:
		return _objectItem;
	case -5:
		return _dummyItem2;
	case -7:
		return NULL;
	case -9:
		return _dummyItem3;
	default:
		return derefItem(a);
	}
}

uint SimonEngine::getNextItemID() {
	int a = getNextWord();
	switch (a) {
	case -1:
		return itemPtrToID(_subjectItem);
	case -3:
		return itemPtrToID(_objectItem);
	case -5:
		return getItem1ID();
	case -7:
		return 0;
	case -9:
		return me()->parent;
	default:
		return a;
	}
}

Item *SimonEngine::me() {
	if (_currentPlayer)
		return _currentPlayer;
	return _dummyItem1;
}

Item *SimonEngine::actor() {
	error("actor: is this code ever used?");
	//if (_actorPlayer)
	//	return _actorPlayer;
	return _dummyItem1;
}

uint SimonEngine::getNextVarContents() {
	return (uint16)readVariable(getVarOrByte());
}

uint SimonEngine::readVariable(uint variable) {
	if (variable >= 255)
		error("Variable %d out of range in read", variable);

	if (getGameType() == GType_FF) {
		if (getBitFlag(83))
			return (uint16)_variableArray2[variable];
		else
			return (uint16)_variableArray[variable];
	} else {
			return _variableArray[variable];
	}
}

void SimonEngine::writeNextVarContents(uint16 contents) {
	writeVariable(getVarOrByte(), contents);
}

void SimonEngine::writeVariable(uint variable, uint16 contents) {
	if (variable >= 256)
		error("Variable %d out of range in write", variable);

	if (getGameType() == GType_FF && getBitFlag(83))
		_variableArray2[variable] = contents;
	else
		_variableArray[variable] = contents;
}

void SimonEngine::setItemParent(Item *item, Item *parent) {
	Item *old_parent = derefItem(item->parent);

	if (item == parent)
		error("Trying to set item as its own parent");

	// unlink it if it has a parent
	if (old_parent)
		unlinkItem(item);
	itemChildrenChanged(old_parent);
	linkItem(item, parent);
	itemChildrenChanged(parent);
}

void SimonEngine::itemChildrenChanged(Item *item) {
	int i;
	WindowBlock *window;

	if (_noParentNotify)
		return;

	mouseOff();

	for (i = 0; i != 8; i++) {
		window = _windowArray[i];
		if (window && window->iconPtr && window->iconPtr->itemRef == item) {
			if (_fcsData1[i]) {
				_fcsData2[i] = true;
			} else {
				_fcsData2[i] = false;
				drawIconArray(i, item, window->iconPtr->line, window->iconPtr->classMask);
			}
		}
	}

	mouseOn();
}

void SimonEngine::unlinkItem(Item *item) {
	Item *first, *parent, *next;

	// can't unlink item without parent
	if (item->parent == 0)
		return;

	// get parent and first child of parent
	parent = derefItem(item->parent);
	first = derefItem(parent->child);

	// the node to remove is first in the parent's children?
	if (first == item) {
		parent->child = item->sibling;
		item->parent = 0;
		item->sibling = 0;
		return;
	}

	for (;;) {
		if (!first)
			error("unlinkItem: parent empty");
		if (first->sibling == 0)
			error("unlinkItem: parent does not contain child");

		next = derefItem(first->sibling);
		if (next == item) {
			first->sibling = next->sibling;
			item->parent = 0;
			item->sibling = 0;
			return;
		}
		first = next;
	}
}

void SimonEngine::linkItem(Item *item, Item *parent) {
	uint id;
	// Don't allow that an item that is already linked is relinked
	if (item->parent)
		return;

	id = itemPtrToID(parent);
	item->parent = id;

	if (parent != 0) {
		item->sibling = parent->child;
		parent->child = itemPtrToID(item);
	} else {
		item->sibling = 0;
	}
}

void SimonEngine::setup_cond_c_helper() {
	HitArea *last;
	uint id;

	_noRightClick = 1;

	if (getGameType() == GType_FF) {
		int cursor = 5;
		int animMax = 16;

		if (getBitFlag(200)) {
			cursor = 11;
			animMax = 5;
		} else if (getBitFlag(201)) {
			cursor = 12;
			animMax = 5;
		} else if (getBitFlag(202)) {
			cursor = 13;
			animMax = 5;
		} else if (getBitFlag(203)) {
			cursor = 14;
			animMax = 9;
		} else if (getBitFlag(205)) {
			cursor = 17;
			animMax = 11;
		} else if (getBitFlag(206)) {
			cursor = 16;
			animMax = 2;
		} else if (getBitFlag(208)) {
			cursor = 26;
			animMax = 2;
		} else if (getBitFlag(209)) {
			cursor = 27;
			animMax = 9;
		} else if (getBitFlag(210)) {
			cursor = 28;
			animMax = 9;
		}

		_animatePointer = 0;
		_mouseCursor = cursor;
		_mouseAnimMax = animMax;
		_mouseAnim = 1;
		_needHitAreaRecalc++;
	}

	if (getGameType() == GType_SIMON2) {
		_mouseCursor = 0;
		if (_defaultVerb != 999) {
			_mouseCursor = 9;
			_needHitAreaRecalc++;
			_defaultVerb = 0;
		}
	}

	_lastHitArea = 0;
	_hitAreaObjectItem = NULL;

	last = _lastNameOn;
	clearName();
	_lastNameOn = last;

	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = 0;
		_leftButtonDown = 0;

		do {
			if (_exitCutscene && getBitFlag(9)) {
				endCutscene();
				goto out_of_here;
			}

			if (getGameType() == GType_FF) {
				if (_variableArray[254] == 63) {
					hitarea_stuff_helper();
				} else if (_variableArray[254] == 75) {
					hitarea_stuff_helper();
					_variableArray[60] = 9999;
					goto out_of_here;
				}
			}

			delay(100);
		} while (_lastHitArea3 == (HitArea *) -1 || _lastHitArea3 == 0);

		if (_lastHitArea == NULL) {
		} else if (_lastHitArea->id == 0x7FFB) {
			inventoryUp(_lastHitArea->window);
		} else if (_lastHitArea->id == 0x7FFC) {
			inventoryDown(_lastHitArea->window);
		} else if (_lastHitArea->item_ptr != NULL) {
			_hitAreaObjectItem = _lastHitArea->item_ptr;
			id = 0xFFFF;
			if (_lastHitArea->flags & kBFTextBox) {
				if (getGameType() == GType_FF && (_lastHitArea->flags & kBFHyperBox))
					id = _lastHitArea->data;
				else
					id = _lastHitArea->flags / 256;
			}
			_variableArray[60] = id;
			break;
		}
	}

out_of_here:
	_lastHitArea3 = 0;
	_lastHitArea = 0;
	_lastNameOn = NULL;
	_mouseCursor = 0;
	_noRightClick = 0;
}

void SimonEngine::endCutscene() {
	Subroutine *sub;

	_sound->stopVoice();

	sub = getSubroutineByID(170);
	if (sub != NULL)
		startSubroutineEx(sub);

	_runScriptReturn1 = true;
}

bool SimonEngine::has_item_childflag_0x10(Item *item) {
	SubObject *child = (SubObject *)findChildOfType(item, 2);
	return child && (child->objectFlags & kOFIcon) != 0;
}

uint SimonEngine::itemGetIconNumber(Item *item) {
	SubObject *child = (SubObject *)findChildOfType(item, 2);
	uint offs;

	if (child == NULL || !(child->objectFlags & kOFIcon))
		return 0;

	offs = getOffsetOfChild2Param(child, 0x10);
	return child->objectFlagValue[offs];
}

void SimonEngine::hitarea_stuff() {
	HitArea *ha;
	uint id;

	_leftButtonDown = 0;
	_lastHitArea = 0;
	_verbHitArea = 0;
	_hitAreaSubjectItem = NULL;
	_hitAreaObjectItem = NULL;

	resetVerbs();

startOver:
	for (;;) {
		_lastHitArea = NULL;
		_lastHitArea3 = NULL;

		for (;;) {
			if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) && _keyPressed == 35)
				displayBoxStars();
			processSpecialKeys();
			if (_lastHitArea3 == (HitArea *) -1)
				goto startOver;
			if (_lastHitArea3 != 0)
				break;
			hitarea_stuff_helper();
			delay(100);
		}

		ha = _lastHitArea;

		if (ha == NULL) {
		} else if (ha->id == 0x7FFB) {
			inventoryUp(ha->window);
		} else if (ha->id == 0x7FFC) {
			inventoryDown(ha->window);
		} else if (ha->id >= 101 && ha->id < 113) {
			_verbHitArea = ha->verb;
			setVerb(ha);
			_defaultVerb = 0;
		} else {
			if ((_verbHitArea != 0 || _hitAreaSubjectItem != ha->item_ptr && ha->flags & kBFBoxItem) &&
					ha->item_ptr) {
			if_1:;
				_hitAreaSubjectItem = ha->item_ptr;
				id = 0xFFFF;
				if (ha->flags & kBFTextBox) {
					if (getGameType() == GType_FF && (ha->flags & kBFHyperBox))
						id = ha->data;
					else
						id = ha->flags / 256;
				}
				_variableArray[60] = id;
				displayName(ha);
				if (_verbHitArea != 0)
					break;
			} else {
				// else 1
				if (ha->verb == 0) {
					if (ha->item_ptr)
						goto if_1;
				} else {
					_verbHitArea = ha->verb & 0xBFFF;
					if (ha->verb & 0x4000) {
						_hitAreaSubjectItem = ha->item_ptr;
						break;
					}
					if (_hitAreaSubjectItem != NULL)
						break;
				}
			}
		}
	}

	_needHitAreaRecalc++;
}

void SimonEngine::hitarea_stuff_helper() {
	time_t cur_time;

	if (getGameType() == GType_SIMON1) {
		uint subr_id = (uint16)_variableArray[254];
		if (subr_id != 0) {
			Subroutine *sub = getSubroutineByID(subr_id);
			if (sub != NULL) {
				startSubroutineEx(sub);
				permitInput();
			}
			_variableArray[254] = 0;
			_runScriptReturn1 = false;
		}
	} else {
		if (_variableArray[254] || _variableArray[249]) {
			hitarea_stuff_helper_2();
		}
	}

	time(&cur_time);
	if ((uint) cur_time != _lastTime) {
		_lastTime = cur_time;
		if (kickoffTimeEvents())
			permitInput();
	}
}

void SimonEngine::hitarea_stuff_helper_2() {
	uint subr_id;
	Subroutine *sub;

	subr_id = (uint16)_variableArray[249];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[249] = 0;
			startSubroutineEx(sub);
			permitInput();
		}
		_variableArray[249] = 0;
	}

	subr_id = (uint16)_variableArray[254];
	if (subr_id != 0) {
		sub = getSubroutineByID(subr_id);
		if (sub != NULL) {
			_variableArray[254] = 0;
			startSubroutineEx(sub);
			permitInput();
		}
		_variableArray[254] = 0;
	}

	_runScriptReturn1 = false;
}

void SimonEngine::permitInput() {
	if (!_mortalFlag) {
		_mortalFlag = true;
		showmessage_print_char(0);
		_curWindow = 0;
		if (_windowArray[0] != 0) {
			_textWindow = _windowArray[0];
			if (getGameType() == GType_FF)
				showmessage_helper_3(_textWindow->textColumn, _textWindow->width);
			else
				showmessage_helper_3(_textWindow->textLength, _textWindow->textMaxLength);
		}
		_mortalFlag = false;
	}
}

TextLocation *SimonEngine::getTextLocation(uint a) {
	switch (a) {
	case 1:
		return &_textLocation1;
	case 2:
		return &_textLocation2;
	case 101:
		return &_textLocation3;
	case 102:
		return &_textLocation4;
	default:
		error("text, invalid value %d", a);
	}
	return NULL;
}

void SimonEngine::loadZone(uint vga_res) {
	VgaPointersEntry *vpe;
	uint32 size;

	CHECK_BOUNDS(vga_res, _vgaBufferPointers);

	vpe = _vgaBufferPointers + vga_res;
	if (vpe->vgaFile1 != NULL)
		return;

	vpe->vgaFile1 = loadVGAFile(vga_res * 2, 1, size);
	vpe->vgaFile1End = vpe->vgaFile1 + size;

	vpe->vgaFile2 = loadVGAFile(vga_res * 2 + 1, 2, size);
	vpe->vgaFile2End = vpe->vgaFile2 + size;

	vpe->sfxFile = NULL;
	if (getGameType() == GType_FF && getPlatform() == Common::kPlatformWindows) {
		vpe->sfxFile = loadVGAFile(vga_res * 2, 3, size);
		vpe->sfxFileEnd = vpe->sfxFile + size;
	}
}

void SimonEngine::setZoneBuffers() {
	byte *alloced;

	alloced = (byte *)malloc(_vgaMemSize);

	_vgaBufFreeStart = alloced;
	_vgaBufStart = alloced;
	_vgaFileBufOrg = alloced;
	_vgaFileBufOrg2 = alloced;
	_vgaBufEnd = alloced + _vgaMemSize;
}

byte *SimonEngine::allocBlock(uint32 size) {
	byte *block, *blockEnd;

	_rejectCount = 0;

	for (;;) {
		block = _vgaBufFreeStart;

		blockEnd = block + size;

		if (blockEnd >= _vgaBufEnd) {
			_vgaBufFreeStart = _vgaBufStart;
		} else {
			_rejectBlock = false;
			checkNoOverWrite(blockEnd);
			if (_rejectBlock)
				continue;
			checkRunningAnims(blockEnd);
			if (_rejectBlock)
				continue;
			checkZonePtrs(blockEnd);
			_vgaBufFreeStart = blockEnd;
			return block;
		}
	}
}

void SimonEngine::checkNoOverWrite(byte *end) {
	VgaPointersEntry *vpe;

	if (_noOverWrite == 0xFFFF)
		return;

	if (_rejectCount == 2)
		error("checkNoOverWrite: _rejectCount == 2");

	vpe = &_vgaBufferPointers[_noOverWrite];

	if (vpe->vgaFile1 < end && vpe->vgaFile1End > _vgaBufFreeStart) {
		_rejectBlock = true;
		_rejectCount++;
		_vgaBufFreeStart = vpe->vgaFile1End;
	} else if (vpe->vgaFile2 < end && vpe->vgaFile2End > _vgaBufFreeStart) {
		_rejectBlock = true;
		_rejectCount++;
		_vgaBufFreeStart = vpe->vgaFile2End;
	} else if (vpe->sfxFile && vpe->sfxFile < end && vpe->sfxFileEnd > _vgaBufFreeStart) {
		_rejectBlock = true;
		_rejectCount++;
		_vgaBufFreeStart = vpe->sfxFileEnd;
	} else {
		_rejectBlock = false;
	}
}

void SimonEngine::checkRunningAnims(byte *end) {
	VgaSprite *vsp;
	if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
		(_lockWord & 0x20)) {
		return;
	}

	for (vsp = _vgaSprites; vsp->id; vsp++) {
		checkAnims(vsp->zoneNum, end);
		if (_rejectBlock == true)
			return;
	}
}

void SimonEngine::checkAnims(uint a, byte *end) {
	VgaPointersEntry *vpe;

	vpe = &_vgaBufferPointers[a];

	if (vpe->vgaFile1 < end && vpe->vgaFile1End > _vgaBufFreeStart) {
		_rejectBlock = true;
		_vgaBufFreeStart = vpe->vgaFile1End;
	} else if (vpe->vgaFile2 < end && vpe->vgaFile2End > _vgaBufFreeStart) {
		_rejectBlock = true;
		_vgaBufFreeStart = vpe->vgaFile2End;
	} else if (vpe->sfxFile && vpe->sfxFile < end && vpe->sfxFileEnd > _vgaBufFreeStart) {
		_rejectBlock = true;
		_vgaBufFreeStart = vpe->sfxFileEnd;
	} else {
		_rejectBlock = false;
	}
}

void SimonEngine::checkZonePtrs(byte *end) {
	uint count = ARRAYSIZE(_vgaBufferPointers);
	VgaPointersEntry *vpe = _vgaBufferPointers;
	do {
		if (vpe->vgaFile1 < end && vpe->vgaFile1End > _vgaBufFreeStart ||
				vpe->vgaFile2 < end && vpe->vgaFile2End > _vgaBufFreeStart ||
				vpe->sfxFile < end && vpe->sfxFileEnd > _vgaBufFreeStart) {
			vpe->vgaFile1 = NULL;
			vpe->vgaFile1End = NULL;
			vpe->vgaFile2 = NULL;
			vpe->vgaFile2End = NULL;
			vpe->sfxFile = NULL;
			vpe->sfxFileEnd = NULL;
		}

	} while (++vpe, --count);
}

void SimonEngine::set_video_mode_internal(uint16 mode, uint16 vga_res_id) {
	uint num, num_lines;
	VgaPointersEntry *vpe;
	byte *bb, *b;
	uint16 count;
	const byte *vc_ptr_org;

	_windowNum = mode;
	_lockWord |= 0x20;

	if (getGameType() == GType_FF) {
		vc27_resetSprite();
	}

	if (vga_res_id == 0) {
		if (getGameType() == GType_SIMON1) {
			_unkPalFlag = true;
		} else if (getGameType() == GType_SIMON2) {
			_useBackGround = true;
			_restoreWindow6 = true;
		}
	}

	_zoneNumber = num = vga_res_id / 100;

	for (;;) {
		vpe = &_vgaBufferPointers[num];

		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		_curSfxFile = vpe->sfxFile;

		if (vpe->vgaFile1 != NULL)
			break;

		loadZone(num);
	}

	// ensure flipping complete

	bb = _curVgaFile1;

	if (getGameType() == GType_FF) {
		b = bb + READ_LE_UINT16(&((VgaFileHeader_Feeble *) bb)->hdr2_start);
		count = READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageCount);
		b = bb + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) b)->imageTable);

		while (count--) {
			if (READ_LE_UINT16(&((ImageHeader_Feeble *) b)->id) == vga_res_id)
				break;
			b += sizeof(ImageHeader_Feeble);
		}
		assert(READ_LE_UINT16(&((ImageHeader_Feeble *) b)->id) == vga_res_id);

	} else {
		b = bb + READ_BE_UINT16(&((VgaFileHeader_Simon *) bb)->hdr2_start);
		count = READ_BE_UINT16(&((VgaFileHeader2_Simon *) b)->imageCount);
		b = bb + READ_BE_UINT16(&((VgaFileHeader2_Simon *) b)->imageTable);

		while (count--) {
			if (READ_BE_UINT16(&((ImageHeader_Simon *) b)->id) == vga_res_id)
				break;
			b += sizeof(ImageHeader_Simon);
		}
		assert(READ_BE_UINT16(&((ImageHeader_Simon *) b)->id) == vga_res_id);
	}

	if (getGameType() == GType_SIMON1) {
		if (vga_res_id == 16300) {
			clearBackFromTop(134);
			_usePaletteDelay = true;
		}
	} else {
		_scrollX = 0;
		_scrollY = 0;
		_scrollXMax = 0;
		_scrollYMax = 0;
		_scrollCount = 0;
		_scrollFlag = 0;
		_scrollHeight = 134;
		_variableArrayPtr = _variableArray;
		if (_variableArray[34] >= 0) {
			if (getGameType() == GType_FF)
				_variableArray[250] = 0;
			_variableArray[251] = 0;
		}
	}

	vc_ptr_org = _vcPtr;

	if (getGameType() == GType_FF) {
		_vcPtr = _curVgaFile1 + READ_LE_UINT16(&((ImageHeader_Feeble *) b)->scriptOffs);
	} else {
		_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((ImageHeader_Simon *) b)->scriptOffs);
	}

	//dump_vga_script(_vcPtr, num, vga_res_id);
	runVgaScript();
	_vcPtr = vc_ptr_org;


	if (getGameType() == GType_FF) {
		fillFrontFromBack(0, 0, _screenWidth, _screenHeight);
		fillBackGroundFromBack(_screenHeight);
		_syncFlag2 = 1;
	} else if (getGameType() == GType_SIMON2) {
		if (!_useBackGround) {
			num_lines = _windowNum == 4 ? 134 : 200;
			_boxStarHeight = num_lines;
			fillFrontFromBack(0, 0, _screenWidth, num_lines);
			fillBackGroundFromBack(num_lines);
			_syncFlag2 = 1;
		}
		_useBackGround = false;
	} else {
		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		if (_subroutine == 2923 || _subroutine == 2926)
			num_lines = 200;
		else
			num_lines = _windowNum == 4 ? 134 : 200;

		fillFrontFromBack(0, 0, _screenWidth, num_lines);
		fillBackGroundFromBack(num_lines);

		_syncFlag2 = 1;
		_timer5 = 0;
	}

	_lockWord &= ~0x20;

	if (getGameType() == GType_SIMON1) {
		if (_unkPalFlag) {
			_unkPalFlag = false;
			while (_paletteColorCount != 0) {
				delay(10);
			}
		}
	}
}

void SimonEngine::waitForSync(uint a) {
	const uint maxCount = (getGameType() == GType_SIMON1) ? 500 : 1000;

	_vgaWaitFor = a;
	_syncCount = 0;
	_exitCutscene = false;
	_rightButtonDown = false;
	while (_vgaWaitFor != 0) {
		if (_rightButtonDown) {
			if (_vgaWaitFor == 200 && !getBitFlag(14)) {
				skipSpeech();
				break;
			}
		}
		if (_exitCutscene) {
			if (getBitFlag(9)) {
				endCutscene();
				break;
			}
		}
		processSpecialKeys();

		if (_syncCount >= maxCount) {
			warning("waitForSync: wait timed out");
			break;
		}

		delay(1);
	}
}

void SimonEngine::skipSpeech() {
	_sound->stopVoice();
	if (!getBitFlag(28)) {
		setBitFlag(14, true);
		if (getGameType() == GType_FF) {
			_variableArray[103] = 5;
			loadSprite(4, 2, 13, 0, 0, 0);
			waitForSync(213);
			stopAnimateSimon2(2, 1);
		} else if (getGameType() == GType_SIMON2) {
			_variableArray[100] = 5;
			loadSprite(4, 1, 30, 0, 0, 0);
			waitForSync(130);
			stopAnimateSimon2(2, 1);
		} else {
			_variableArray[100] = 15;
			loadSprite(4, 1, 130, 0, 0, 0);
			waitForSync(130);
			stopAnimateSimon1(1);
		}
	}
}

Item *SimonEngine::derefItem(uint item) {
	if (item >= _itemArraySize)
		error("derefItem: invalid item %d", item);
	return _itemArrayPtr[item];
}

uint SimonEngine::itemPtrToID(Item *id) {
	uint i;
	for (i = 0; i != _itemArraySize; i++)
		if (_itemArrayPtr[i] == id)
			return i;
	error("itemPtrToID: not found");
	return 0;
}

bool SimonEngine::isSpriteLoaded(uint16 id, uint16 zoneNum) {
	VgaSprite *vsp = _vgaSprites;
	while (vsp->id) {
		if (getGameType() == GType_SIMON1) {
			if (vsp->id == id)
				return true;
		} else {
			if (vsp->id == id && vsp->zoneNum == zoneNum)
				return true;
		}
		vsp++;
	}
	return false;
}

void SimonEngine::processSpecialKeys() {
	switch (_keyPressed) {
	case 27: // escape
		_exitCutscene = true;
		break;
	case 59: // F1
		if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 40);
		} else {
			vcWriteVar(5, 50);
		}
		vcWriteVar(86, 0);
		break;
	case 60: // F2
		if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 60);
		} else {
			vcWriteVar(5, 75);
		}
		vcWriteVar(86, 1);
		break;
	case 61: // F3
		if (getGameType() == GType_SIMON1) {
			vcWriteVar(5, 100);
		} else {
			vcWriteVar(5, 125);
		}
		vcWriteVar(86, 2);
		break;
	case 63: // F5
		if (getGameType() == GType_SIMON2 || getGameType() == GType_FF)
			_exitCutscene = true;
		break;
	case 65: // F7
		if (getGameType() == GType_FF && getBitFlag(76))
			_variableArray[254] = 70;
		break;
	case 67: // F9
		if (getGameType() == GType_FF)
			setBitFlag(73, !getBitFlag(73));
		break;
	case 'p':
		pause();
		break;
	case 't':
		if (getGameType() == GType_FF || (getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE)) ||
			((getFeatures() & GF_TALKIE) && _language != Common::EN_ANY && _language != Common::DE_DEU)) {
			if (_speech)
				_subtitles ^= 1;
		}
		break;
	case 'v':
		if (getGameType() == GType_FF || (getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE))) {
			if (_subtitles)
				_speech ^= 1;
		}
	case '+':
		midi.set_volume(midi.get_volume() + 16);
		break;
	case '-':
		midi.set_volume(midi.get_volume() - 16);
		break;
	case 'm':
		midi.pause(_musicPaused ^= 1);
		break;
	case 's':
		if (getGameId() == GID_SIMON1DOS)
			midi._enable_sfx ^= 1;
		else
			_sound->effectsPause(_effectsPaused ^= 1);
		break;
	case 'b':
		_sound->ambientPause(_ambientPaused ^= 1);
		break;
	case 'r':
		if (_debugMode)
			_startMainScript ^= 1;
		break;
	case 'o':
		if (_debugMode)
			_continousMainScript ^= 1;
		break;
	case 'a':
		if (_debugMode)
			_startVgaScript ^= 1;
		break;
	case 'g':
		if (_debugMode)
			_continousVgaScript ^= 1;
		break;
	case 'i':
		if (_debugMode)
			_drawImagesDebug ^= 1;
		break;
	case 'd':
		if (_debugMode)
			_dumpImages ^=1;
		break;
	}

	_keyPressed = 0;
}

void SimonEngine::pause() {
	_keyPressed = 1;
	_pause = 1;
	bool ambient_status = _ambientPaused;
	bool music_status = _musicPaused;

	midi.pause(true);
	_sound->ambientPause(true);
	while (_pause) {
		delay(1);
		if (_keyPressed == 'p')
			_pause = 0;
	}
	midi.pause(music_status);
	_sound->ambientPause(ambient_status);

}

void SimonEngine::loadSprite(uint windowNum, uint zoneNum, uint vgaSpriteId, uint x, uint y, uint palette) {
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;
	uint count;

	_lockWord |= 0x40;

	if ((getGameType() == GType_SIMON1 || getGameType() == GType_SIMON2) &&
		isSpriteLoaded(vgaSpriteId, zoneNum)) {
		_lockWord &= ~0x40;
		return;
	}

	vsp = _vgaSprites;
	while (vsp->id != 0)
		vsp++;

	vsp->windowNum = windowNum;
	vsp->priority = 0;
	vsp->flags = 0;

	vsp->y = y;
	vsp->x = x;
	vsp->image = 0;
	vsp->palette = palette;
	vsp->id = vgaSpriteId;
	if (getGameType() == GType_SIMON1)
		vsp->zoneNum = zoneNum = vgaSpriteId / 100;
	else
		vsp->zoneNum = zoneNum;


	for (;;) {
		vpe = &_vgaBufferPointers[zoneNum];
		_zoneNumber = zoneNum;
		_curVgaFile1 = vpe->vgaFile1;
		if (vpe->vgaFile1 != NULL)
			break;
		loadZone(zoneNum);
	}

	pp = _curVgaFile1;
	if (getGameType() == GType_FF) {
		p = pp + READ_LE_UINT16(&((VgaFileHeader_Feeble *) pp)->hdr2_start);
		count = READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationCount);
		p = pp + READ_LE_UINT16(&((VgaFileHeader2_Feeble *) p)->animationTable);
	} else {
		p = pp + READ_BE_UINT16(&((VgaFileHeader_Simon *) pp)->hdr2_start);
		count = READ_BE_UINT16(&((VgaFileHeader2_Simon *) p)->animationCount);
		p = pp + READ_BE_UINT16(&((VgaFileHeader2_Simon *) p)->animationTable);
	}

	for (;;) {
		if (getGameType() == GType_FF) {
			if (READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->id) == vgaSpriteId) {
				if (_startVgaScript)
					dump_vga_script(pp + READ_LE_UINT16(&((AnimationHeader_Feeble*)p)->scriptOffs), zoneNum, vgaSpriteId);

				addVgaEvent(_vgaBaseDelay, pp + READ_LE_UINT16(&((AnimationHeader_Feeble *) p)->scriptOffs), vgaSpriteId, zoneNum);
				break;
			}
			p += sizeof(AnimationHeader_Feeble);
		} else {
			if (READ_BE_UINT16(&((AnimationHeader_Simon *) p)->id) == vgaSpriteId) {
				if (_startVgaScript)
					dump_vga_script(pp + READ_BE_UINT16(&((AnimationHeader_Simon*)p)->scriptOffs), zoneNum, vgaSpriteId);

				addVgaEvent(_vgaBaseDelay, pp + READ_BE_UINT16(&((AnimationHeader_Simon *) p)->scriptOffs), vgaSpriteId, zoneNum);
				break;
			}
			p += sizeof(AnimationHeader_Simon);
		}

		if (!--count) {
			vsp->id = 0;
			break;
		}
	}

	_lockWord &= ~0x40;
}

void SimonEngine::playSpeech(uint speech_id, uint vgaSpriteId) {
	if (getGameType() == GType_SIMON1) {
		if (speech_id == 9999) {
			if (_subtitles)
				return;
			if (!getBitFlag(14) && !getBitFlag(28)) {
				setBitFlag(14, true);
				_variableArray[100] = 15;
				loadSprite(4, 1, 130, 0, 0, 0);
				waitForSync(130);
			}
			_skipVgaWait = true;
		} else {
			if (_subtitles && _scriptVar2) {
				loadSprite(4, 2, 204, 0, 0, 0);
				waitForSync(204);
				stopAnimateSimon1(204);
			}
			stopAnimateSimon1(vgaSpriteId + 201);
			loadVoice(speech_id);
			loadSprite(4, 2, vgaSpriteId + 201, 0, 0, 0);
		}
	} else {
		if (speech_id == 0xFFFF) {
			if (_subtitles)
				return;
			if (!getBitFlag(14) && !getBitFlag(28)) {
				setBitFlag(14, true);
				_variableArray[100] = 5;
				loadSprite(4, 1, 30, 0, 0, 0);
				waitForSync(130);
			}
			_skipVgaWait = true;
		} else {
			if (getGameType() == GType_SIMON2 && _subtitles && _language != Common::HB_ISR) {
				loadVoice(speech_id);
				return;
			}

			if (_subtitles && _scriptVar2) {
				loadSprite(4, 2, 5, 0, 0, 0);
				waitForSync(205);
				stopAnimateSimon2(2,5);
			}

			stopAnimateSimon2(2, vgaSpriteId + 2);
			loadVoice(speech_id);
			loadSprite(4, 2, vgaSpriteId + 2, 0, 0, 0);
		}
	}
}

int SimonEngine::go() {

	loadGamePcFile();

	addTimeEvent(0, 1);
	openGameFile();

	if (getGameType() == GType_FF)
		loadIconData();
	else
		loadIconFile();

	vc34_setMouseOff();

	if ((getPlatform() == Common::kPlatformAmiga || getPlatform() == Common::kPlatformMacintosh) &&
		getGameType() == GType_FF) {
		_moviePlay->load((const char *)"epic.dxa");
		_moviePlay->play();
	}

	runSubroutine101();
	permitInput();

	while (1) {
		hitarea_stuff();
		handleVerbClicked(_verbHitArea);
		delay(100);
	}

	return 0;
}

void SimonEngine::shutdown() {
	delete _gameFile;

	midi.close();

	free(_stringTabPtr);
	free(_itemArrayPtr);
	free(_itemHeapPtr - _itemHeapCurPos);
	free(_tablesHeapPtr - _tablesHeapCurPos);
	free(_tblList);
	free(_iconFilePtr);
	free(_gameOffsetsPtr);

	_system->quit();
}

void SimonEngine::delay(uint amount) {
	OSystem::Event event;

	uint32 start = _system->getMillis();
	uint32 cur = start;
	uint this_delay, vga_period;

	if (_debugger->isAttached())
		_debugger->onFrame();

	if (_fastMode)
	 	vga_period = 10;
	else if (getGameType() == GType_SIMON2)
		vga_period = 45;
	else
		vga_period = 50;

	_rnd.getRandomNumber(2);

	do {
		while (!_inCallBack && cur >= _lastVgaTick + vga_period && !_pause) {
			_lastVgaTick += vga_period;

			// don't get too many frames behind
			if (cur >= _lastVgaTick + vga_period * 2)
				_lastVgaTick = cur;

			_inCallBack = true;
			timer_callback();
			_inCallBack = false;
		}

		while (_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_KEYDOWN:
				if (event.kbd.keycode >= '0' && event.kbd.keycode <='9'
					&& (event.kbd.flags == OSystem::KBD_ALT ||
						event.kbd.flags == OSystem::KBD_CTRL)) {
					_saveLoadSlot = event.kbd.keycode - '0';

					// There is no save slot 0
					if (_saveLoadSlot == 0)
						_saveLoadSlot = 10;

					sprintf(_saveLoadName, "Quicksave %d", _saveLoadSlot);
					_saveLoadType = (event.kbd.flags == OSystem::KBD_ALT) ? 1 : 2;

					// We should only allow a load or save when it was possible in original
					// This stops load/save during copy protection, conversations and cut scenes
					if (!_mouseHideCount && !_showPreposition)
						quickLoadOrSave();
				} else if (event.kbd.flags == OSystem::KBD_CTRL) {
					if (event.kbd.keycode == 'a') {
						GUI::Dialog *_aboutDialog;
						_aboutDialog = new GUI::AboutDialog();
						_aboutDialog->runModal();
					} else if (event.kbd.keycode == 'f')
						_fastMode ^= 1;
					else if (event.kbd.keycode == 'd')
						_debugger->attach();
				}
				// Make sure backspace works right (this fixes a small issue on OS X)
				if (event.kbd.keycode == 8)
					_keyPressed = 8;
				else
					_keyPressed = (byte)event.kbd.ascii;
				break;
			case OSystem::EVENT_MOUSEMOVE:
				_sdlMouseX = event.mouse.x;
				_sdlMouseY = event.mouse.y;
				break;
			case OSystem::EVENT_LBUTTONDOWN:
				if (getGameType() == GType_FF)
					setBitFlag(89, true);
				_leftButtonDown++;
#if defined (_WIN32_WCE) || defined(PALMOS_MODE)
				_sdlMouseX = event.mouse.x;
				_sdlMouseY = event.mouse.y;
#endif
				break;
			case OSystem::EVENT_LBUTTONUP:
				if (getGameType() == GType_FF)
					setBitFlag(89, false);
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				if (getGameType() == GType_FF)
					setBitFlag(92, false);
				_rightButtonDown++;
				break;
			case OSystem::EVENT_QUIT:
				shutdown();
				return;
				break;
			default:
				break;
			}
		}

		_system->updateScreen();

		if (amount == 0)
			break;

		this_delay = _fastMode ? 1 : 20;
		if (this_delay > amount)
			this_delay = amount;
		_system->delayMillis(this_delay);

		cur = _system->getMillis();
	} while (cur < start + amount);
}

void SimonEngine::loadMusic(uint music) {
	char buf[4];

	if (getGameType() == GType_SIMON2) {        // Simon 2 music
		midi.stop();
		_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
		_gameFile->read(buf, 4);
		if (!memcmp(buf, "FORM", 4)) {
			_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
			midi.loadXMIDI (_gameFile);
		} else {
			_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music - 1], SEEK_SET);
			midi.loadMultipleSMF (_gameFile);
		}

		_lastMusicPlayed = music;
		_nextMusicToPlay = -1;
	} else if (getGameType() == GType_SIMON1) {        // Simon 1 music
		if (getPlatform() == Common::kPlatformAmiga) {
			if (getFeatures() & GF_CRUNCHED) {
				// TODO Add support for decruncher
				debug(5,"loadMusic - Decrunch %dtune attempt", music);
			}
			// TODO Add Protracker support for simon1amiga/cd32
			debug(5,"playMusic - Load %dtune attempt", music);
			return;
		}

		midi.stop();
		midi.setLoop (true); // Must do this BEFORE loading music. (GMF may have its own override.)

		if (getFeatures() & GF_TALKIE) {
			// FIXME: The very last music resource, a cymbal crash for when the
			// two demons crash into each other, should NOT be looped like the
			// other music tracks. In simon1dos/talkie the GMF resource includes
			// a loop override that acomplishes this, but there seems to be nothing
			// for this in the SMF resources.
			if (music == 35)
				midi.setLoop (false);

			_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
			_gameFile->read(buf, 4);
			if (!memcmp(buf, "GMF\x1", 4)) {
				_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
				midi.loadSMF (_gameFile, music);
			} else {
				_gameFile->seek(_gameOffsetsPtr[_musicIndexBase + music], SEEK_SET);
				midi.loadMultipleSMF (_gameFile);
			}

		} else {
			char filename[15];
			File f;
			sprintf(filename, "MOD%d.MUS", music);
			f.open(filename);
			if (f.isOpen() == false) {
				warning("Can't load music from '%s'", filename);
				return;
			}
			if (getGameId() == GID_SIMON1DEMO)
				midi.loadS1D (&f);
			else
				midi.loadSMF (&f, music);
		}

		midi.startTrack (0);
	}
}

void SimonEngine::playSting(uint a) {
	if (!midi._enable_sfx)
		return;

	char filename[15];

	File mus_file;
	uint16 mus_offset;

	sprintf(filename, "STINGS%i.MUS", _soundFileId);
	mus_file.open(filename);
	if (!mus_file.isOpen()) {
		warning("Can't load sound effect from '%s'", filename);
		return;
	}

	mus_file.seek(a * 2, SEEK_SET);
	mus_offset = mus_file.readUint16LE();
	if (mus_file.ioFailed())
		error("Can't read sting %d offset", a);

	mus_file.seek(mus_offset, SEEK_SET);
	midi.loadSMF(&mus_file, a, true);
	midi.startTrack(0);
}

void SimonEngine::set_volume(int volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
}

} // End of namespace Simon

#ifdef PALMOS_68K
#include "scumm_globals.h"

_GINIT(Simon_Simon)
_GSETPTR(Simon::simon1_settings, GBVARS_SIMON1SETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GSETPTR(Simon::simon2_settings, GBVARS_SIMON2SETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GSETPTR(Simon::feeblefiles_settings, GBVARS_FEEBLEFILESSETTINGS_INDEX, Simon::GameSpecificSettings, GBVARS_SIMON)
_GEND

_GRELEASE(Simon_Simon)
_GRELEASEPTR(GBVARS_SIMON1SETTINGS_INDEX, GBVARS_SIMON)
_GRELEASEPTR(GBVARS_SIMON2SETTINGS_INDEX, GBVARS_SIMON)
_GRELEASEPTR(GBVARS_FEEBLEFILESSETTINGS_INDEX, GBVARS_SIMON)
_GEND

#endif
