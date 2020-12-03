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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef ENABLE_EOB

#include "kyra/engine/kyra_rpg.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/sound_intern.h"
#include "kyra/sound/sound_pc_v1.h"
#include "kyra/script/script_eob.h"
#include "kyra/engine/timer.h"
#include "kyra/gui/debugger.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/translation.h"

#include "gui/error.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"

namespace Kyra {

const char *const EoBCoreEngine::kKeymapName = "eob";

EoBCoreEngine::EoBCoreEngine(OSystem *system, const GameFlags &flags) : KyraRpgEngine(system, flags), _numLargeItemShapes(flags.gameID == GI_EOB1 ? 14 : 11),
	_numSmallItemShapes(flags.gameID == GI_EOB1 ? 23 : 26),	_numThrownItemShapes(flags.gameID == GI_EOB1 ? 12 : 9),
	_numItemIconShapes(flags.gameID == GI_EOB1 ? 89 : 112),	_teleporterWallId(flags.gameID == GI_EOB1 ? 52 : 44) {

	_screen = 0;
	_gui = 0;

	_playFinale = false;
	_runFlag = true;
	_configMouse = _config2431 = true;
	_mouseSpeed = _padSpeed = 1;
	_inputMode = 0;
	_loading = false;

	_enableHiResDithering = false;

	_tickLength = (_flags.platform == Common::kPlatformSegaCD) ? 38 : 55;
	_envAudioTimer = 0;
	_flashShapeTimer = 0;
	_flashShapeTimerIntv0 = (_flags.platform == Common::kPlatformSegaCD ? 2 * _tickLength : 0);
	_flashShapeTimerIntv1 = _tickLength * (_flags.platform == Common::kPlatformSegaCD ? 2 : 8);
	_flashShapeTimerIntv2 = _tickLength * 8;
	_drawSceneTimer = 0;
	_vcnFilePattern = "%s.VCN";
	_vmpFilePattern = "%s.VMP";

	_largeItemShapes = _smallItemShapes = _thrownItemShapes = _spellShapes = _firebeamShapes = 0;
	_itemIconShapes = _blueItemIconShapes = _xtraItemIconShapes = _wallOfForceShapes = _teleporterShapes = _sparkShapes = _compassShapes = 0;
	_redSplatShape = _greenSplatShape = _deadCharShape = _disabledCharGrid = _swapShape = 0;
	_blackBoxSmallGrid = _weaponSlotGrid = _blackBoxWideGrid = _lightningColumnShape = 0;

	memset(_redSplatBG, 0, sizeof(_redSplatBG));
	memset(_largeItemShapesScl, 0, sizeof(_largeItemShapesScl));
	memset(_smallItemShapesScl, 0, sizeof(_smallItemShapesScl));
	memset(_thrownItemShapesScl, 0, sizeof(_thrownItemShapesScl));

	_monsterAcHitChanceTable1 = _monsterAcHitChanceTable2 = 0;

	_monsterDustStrings = 0;
	_enemyMageSpellList = 0;
	_enemyMageSfx = 0;
	_beholderSpellList = 0;
	_beholderSfx = 0;

	_faceShapes = 0;
	_characters = 0;
	_items = 0;
	_itemTypes = 0;
	_itemNames = 0;
	_itemNamesStatic = 0;
	_itemInHand = -1;
	_numItems = _numItemNames = _numItemNamesStatic = 0;

	_castScrollSlot = 0;
	_currentSub = 0;

	_itemsOverlay = 0;

	_partyEffectFlags = 0;
	_lastUsedItem = 0;

	_levelDecorationRects = 0;
	_doorSwitches = 0;
	_monsterProps = 0;
	_monsterDecorations = 0;
	_monsterFlashOverlay = _monsterStoneOverlay = 0;
	_monsters = 0;
	_dstMonsterIndex = 0;
	_preventMonsterFlash = false;
	_sceneShakeCountdown = 0;

	_teleporterPulse = 0;

	_dscShapeCoords = 0;
	_dscItemPosIndex = 0;
	_dscItemShpX = 0;
	_dscItemScaleIndex = 0;
	_dscItemTileIndex = 0;
	_dscItemShapeMap = 0;
	_dscDoorScaleOffs = 0;
	_dscDoorScaleMult1 = 0;
	_dscDoorScaleMult2 = 0;
	_dscDoorScaleMult3 = 0;
	_dscDoorY1 = 0;
	_dscDoorXE = 0;

	_shapeShakeOffsetX = _shapeShakeOffsetY = 0;
	_greenFadingTable = _blueFadingTable = _lightBlueFadingTable = _blackFadingTable = _greyFadingTable = 0;

	_menuDefs = 0;

	_exchangeCharacterId = -1;
	_charExchangeSwap = 0;
	_configHpBarGraphs = true;
	_configMouseBtSwap = false;

	memset(_dialogueLastBitmap, 0, 13);
	_npcSequenceSub = 0;
	_moveCounter = 0;
	_partyResting = false;

	_flyingObjects = 0;

	_inf = 0;
	_stepCounter = 0;
	_stepsUntilScriptCall = 0;
	_scriptTimersMode = 3;
	_currentDirection = 0;

	_openBookSpellLevel = 0;
	_openBookSpellSelectedItem = 0;
	_openBookSpellListOffset = 0;
	_openBookChar = _openBookCharBackup = _openBookCasterLevel = 0;
	_openBookType = _openBookTypeBackup = 0;
	_openBookSpellList = 0;
	_openBookAvailableSpells = 0;
	_activeSpellCharId = 0;
	_activeSpellCharacterPos = 0;
	_activeSpell = 0;
	_characterSpellTarget = 0;
	_returnAfterSpellCallback = false;
	_spells = 0;
	_spellAnimBuffer = 0;
	_clericSpellOffset = 0;
	_restPartyElapsedTime = _disableElapsedTime = 0;
	_allowSkip = false;
	_allowImport = false;

	_wallsOfForce = 0;

	_rrCount = 0;
	memset(_rrNames, 0, 10 * sizeof(const char *));
	memset(_rrId, 0, 10 * sizeof(int8));

	_mainMenuStrings = _levelGainStrings = _monsterSpecAttStrings = _characterGuiStringsHp = 0;
	_characterGuiStringsWp = _characterGuiStringsWr = _characterGuiStringsSt = 0;
	_characterGuiStringsIn = _characterStatusStrings7 = _characterStatusStrings8 = 0;
	_characterStatusStrings9 = _characterStatusStrings12 = _characterStatusStrings13 = 0;
	_classModifierFlags = _saveThrowLevelIndex = _saveThrowModDiv = _saveThrowModExt = 0;
	_wandTypes = _drawObjPosIndex = _flightObjFlipIndex = _expObjectTblIndex = 0;
	_expObjectShpStart = _expObjectTlMode = _expObjectAnimTbl1 = _expObjectAnimTbl2 = _expObjectAnimTbl3 = 0;
	_monsterStepTable0 = _monsterStepTable1 = _monsterStepTable2 = _monsterStepTable3 = 0;
	_projectileWeaponAmmoTypes = _flightObjShpMap = _flightObjSclIndex = 0;
	_monsterCloseAttPosTable1 = _monsterCloseAttPosTable2 = _monsterCloseAttChkTable1 = 0;
	_monsterCloseAttChkTable2 = _monsterCloseAttDstTable1 = _monsterCloseAttDstTable2 = 0;
	_monsterProximityTable = _findBlockMonstersTable = _wallOfForceDsY = _wallOfForceDsNumW = 0;
	_wallOfForceDsNumH = _wallOfForceShpId = _wllFlagPreset = _teleporterShapeCoords = 0;
	_monsterCloseAttUnkTable = _monsterFrmOffsTable1 = _monsterFrmOffsTable2 = 0;
	_monsterDirChangeTable = _portalSeq = 0;
	_wallOfForceDsX = 0;
	_expObjectAnimTbl1Size = _expObjectAnimTbl2Size = _expObjectAnimTbl3Size = 0;
	_wllFlagPresetSize = _scriptTimersCount = _buttonList1Size = _buttonList2Size = 0;
	_buttonList3Size = _buttonList4Size = _buttonList5Size = _buttonList6Size = 0;
	_buttonList7Size = _buttonList8Size = 0;
	_inventorySlotsY = _mnDef = 0;
	_invFont1 = _invFont2 = _conFont = Screen::FID_6_FNT;
	_invFont3 = Screen::FID_8_FNT;
	_transferStringsScummVM = 0;
	_buttonDefs = 0;
	_npcPreset = 0;
	_npcPresetNames = 0;
	_chargenStatStrings = _chargenRaceSexStrings = _chargenClassStrings = 0;
	_chargenAlignmentStrings = _pryDoorStrings = _warningStrings = _ripItemStrings = 0;
	_cursedString = _enchantedString = _magicObjectStrings = _magicObjectString5 = 0;
	_patternSuffix = _patternGrFix1 = _patternGrFix2 = _validateArmorString = 0;
	_validateCursedString = _validateNoDropString = _potionStrings = _wandStrings = 0;
	_itemMisuseStrings = _suffixStringsRings = _suffixStringsPotions = 0;
	_suffixStringsWands = _takenStrings = _potionEffectStrings = _yesNoStrings = 0;
	_npcMaxStrings = _okStrings = _npcJoinStrings = _cancelStrings = 0;
	_abortStrings = _saveLoadStrings = _mnWord = _mnPrompt = _bookNumbers = 0;
	_mageSpellList = _clericSpellList = _spellNames = _magicStrings1 = 0;
	_magicStrings2 = _magicStrings3 = _magicStrings4 = _magicStrings6 = 0;
	_magicStrings7 = _magicStrings8 = _magicStrings9 = _saveNamePatterns = 0;
	_spellAnimBuffer = 0;
	_sparkEffectDefSteps = _sparkEffectDefSubSteps = _sparkEffectDefShift = 0;
	_sparkEffectDefAdd = _sparkEffectDefX = _sparkEffectDefY = _sparkEffectOfShift = 0;
	_sparkEffectOfX = _sparkEffectOfY = _magicFlightObjectProperties = 0;
	_turnUndeadEffect = _burningHandsDest = _coneOfColdGfxTbl = 0;
	_sparkEffectOfFlags1 = _sparkEffectOfFlags2 = 0;
	_coneOfColdDest1 = _coneOfColdDest2 = _coneOfColdDest3 = _coneOfColdDest4 = 0;
	_coneOfColdGfxTblSize = 0;
	_menuButtonDefs = 0;
	_updateCharNum = 0;
	_menuStringsMain = _menuStringsSaveLoad = _menuStringsOnOff = _menuStringsSpells = 0;
	_menuStringsRest = _menuStringsDrop = _menuStringsExit = _menuStringsStarve = 0;
	_menuStringsScribe = _menuStringsDrop2 = _menuStringsHead = _menuStringsPoison = 0;
	_menuStringsMgc = _menuStringsPrefs = _menuStringsRest2 = _menuStringsRest3 = 0;
	_menuStringsRest4 = _menuStringsDefeat = _menuStringsTransfer = _menuStringsSpec = 0;
	_menuStringsSpellNo = _menuYesNoStrings = _2431Strings = _textInputCharacterLines = _textInputSelectStrings = 0;
	_errorSlotEmptyString = _errorSlotNoNameString = _menuOkString = 0;
	_spellLevelsMage = _spellLevelsCleric = _numSpellsCleric = _numSpellsWisAdj = _numSpellsPal = _numSpellsMage = 0;
	_mnNumWord = _numSpells = _mageSpellListSize = _spellLevelsMageSize = _spellLevelsClericSize = _textInputCharacterLinesSize = 0;
	_inventorySlotsX = _slotValidationFlags = _encodeMonsterShpTable = 0;
	_cgaMappingDefault = _cgaMappingAlt = _cgaMappingInv = _cgaLevelMappingIndex = _cgaMappingItemsL = _cgaMappingItemsS = _cgaMappingThrown = _cgaMappingIcons = _cgaMappingDeco = 0;
	_amigaLevelSoundList1 = _amigaLevelSoundList2 = 0;
	_dcrShpDataPos = 0;
	_amigaSoundMap = 0;
	_amigaCurSoundFile = -1;
	_prefMenuPlatformOffset = 0;
	_lastVIntTick = _lastSecTick = _totalPlaySecs = _totalEnemiesKilled = _totalSteps = 0;
	_levelMaps = 0;
	_closeSpellbookAfterUse = false;
	_wndBackgrnd = 0;

	memset(_cgaMappingLevel, 0, sizeof(_cgaMappingLevel));
	memset(_expRequirementTables, 0, sizeof(_expRequirementTables));
	memset(_saveThrowTables, 0, sizeof(_saveThrowTables));
	memset(_doorType, 0, sizeof(_doorType));
	memset(_noDoorSwitch, 0, sizeof(_noDoorSwitch));
	memset(_scriptTimers, 0, sizeof(_scriptTimers));
	memset(_monsterBlockPosArray, 0, sizeof(_monsterBlockPosArray));
	memset(_foundMonstersArray, 0, sizeof(_foundMonstersArray));

#define DWM0 _dscWallMapping.push_back(0)
#define DWM(x) _dscWallMapping.push_back(&_sceneDrawVar##x)
	DWM0;       DWM0;       DWM(Down);  DWM(Right);
	DWM(Down);  DWM(Right); DWM(Down);  DWM0;
	DWM(Down);  DWM(Left);  DWM(Down);  DWM(Left);
	DWM0;       DWM0;       DWM(Down);  DWM(Right);
	DWM(Down);  DWM(Right); DWM(Down);  DWM0;
	DWM(Down);  DWM(Left);  DWM(Down);  DWM(Left);
	DWM(Down);  DWM(Right); DWM(Down);  DWM0;
	DWM(Down);  DWM(Left);  DWM0;       DWM(Right);
	DWM(Down);  DWM0;       DWM0;       DWM(Left);
#undef DWM
#undef DWM0
}

EoBCoreEngine::~EoBCoreEngine() {
	releaseItemsAndDecorationsShapes();
	releaseTempData();

	if (_faceShapes) {
		for (int i = 0; i < 44; i++) {
			if (_characters) {
				for (int ii = 0; ii < 6; ii++) {
					if (_characters[ii].faceShape == _faceShapes[i])
						_characters[ii].faceShape = 0;
				}
			}
			delete[] _faceShapes[i];
			_faceShapes[i] = 0;
		}
		delete[] _faceShapes;
	}

	if (_characters) {
		for (int i = 0; i < 6; i++) {
			delete[] _characters[i].faceShape;
			delete[] _characters[i].nameShape;
		}
	}

	delete[] _characters;
	delete[] _items;
	delete[] _itemTypes;

	releaseShpArr(_itemNames, 130);
	delete[] _flyingObjects;

	delete[] _monsterFlashOverlay;
	delete[] _monsterStoneOverlay;
	delete[] _monsters;

	if (_monsterDecorations) {
		releaseMonsterShapes(0, 36);
		delete[] _monsterShapes;
		delete[] _monsterDecorations;

		for (int i = 0; i < 24; i++)
			delete[] _monsterPalettes[i];
		delete[] _monsterPalettes;
	}

	delete[] _monsterProps;

	if (_doorSwitches) {
		releaseDoorShapes();
		delete[] _doorSwitches;
	}

	releaseDecorations();
	delete[] _levelDecorationRects;
	_dscWallMapping.clear();

	delete[] _greenFadingTable;
	delete[] _blueFadingTable;
	delete[] _lightBlueFadingTable;
	delete[] _blackFadingTable;
	delete[] _greyFadingTable;

	delete[] _spells;
	delete[] _spellAnimBuffer;
	delete[] _wallsOfForce;
	delete[] _buttonDefs;

	for (int i = 0; i < 6; i++)
		delete[] _redSplatBG[i];

	delete _gui;
	_gui = 0;
	delete _screen;
	_screen = 0;

	delete[] _menuDefs;
	_menuDefs = 0;

	delete[] _amigaSoundMap;
	_amigaSoundMap = 0;

	delete _inf;
	_inf = 0;
	delete _timer;
	_timer = 0;
	delete _txt;
	_txt = 0;
}

Common::KeymapArray EoBCoreEngine::initKeymaps(const Common::String &gameId) {
	Common::Keymap *const keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, kKeymapName, "Eye of the Beholder");

	addKeymapAction(keyMap, "LCLK", _("Interact via Left Click"), &Common::Action::setLeftClickEvent, "MOUSE_LEFT", "JOY_A");
	addKeymapAction(keyMap, "RCLK", _("Interact via Right Click"), &Common::Action::setRightClickEvent, "MOUSE_RIGHT", "JOY_B");
	addKeymapAction(keyMap, "MVF", _("Move Forward"), Common::KeyState(Common::KEYCODE_UP), "UP", "JOY_UP");
	addKeymapAction(keyMap, "MVB", _("Move Back"), Common::KeyState(Common::KEYCODE_DOWN), "DOWN", "JOY_DOWN");
	addKeymapAction(keyMap, "MVL", _("Move Left"), Common::KeyState(Common::KEYCODE_LEFT), "LEFT", "JOY_LEFT_TRIGGER");
	addKeymapAction(keyMap, "MVR", _("Move Right"), Common::KeyState(Common::KEYCODE_RIGHT), "RIGHT", "JOY_RIGHT_TRIGGER");
	addKeymapAction(keyMap, "TL", _("Turn Left"), Common::KeyState(Common::KEYCODE_HOME), "HOME", "JOY_LEFT");
	addKeymapAction(keyMap, "TR", _("Turn Right"), Common::KeyState(Common::KEYCODE_PAGEUP), "PAGEUP", "JOY_RIGHT");
	addKeymapAction(keyMap, "INV", _("Open/Close Inventory"), Common::KeyState(Common::KEYCODE_i, 'i'), "i", "JOY_X");
	addKeymapAction(keyMap, "SCE", _("Switch Inventory/Character screen"), Common::KeyState(Common::KEYCODE_p, 'p'), "p", "JOY_Y");
	addKeymapAction(keyMap, "CMP", _("Camp"), Common::KeyState(Common::KEYCODE_c, 'c'), "c", "");
	addKeymapAction(keyMap, "CSP", _("Cast Spell"), Common::KeyState(Common::KEYCODE_SPACE, ' '), "SPACE", "JOY_LEFT_SHOULDER");
	// TODO: Spell cursor, but this needs more thought, since different
	// game versions use different keycodes.
	addKeymapAction(keyMap, "SL1", _("Spell Level 1"), Common::KeyState(Common::KEYCODE_1, '1'), "1", "");
	addKeymapAction(keyMap, "SL2", _("Spell Level 2"), Common::KeyState(Common::KEYCODE_2, '2'), "2", "");
	addKeymapAction(keyMap, "SL3", _("Spell Level 3"), Common::KeyState(Common::KEYCODE_3, '3'), "3", "");
	addKeymapAction(keyMap, "SL4", _("Spell Level 4"), Common::KeyState(Common::KEYCODE_4, '4'), "4", "");
	addKeymapAction(keyMap, "SL5", _("Spell Level 5"), Common::KeyState(Common::KEYCODE_5, '5'), "5", "");
	if (gameId == "eob2")
		addKeymapAction(keyMap, "SL6", _("Spell Level 6"), Common::KeyState(Common::KEYCODE_6, '6'), "6", "");

	return Common::Keymap::arrayOf(keyMap);
}

Common::Error EoBCoreEngine::init() {
	if (ConfMan.hasKey("render_mode"))
		_configRenderMode = Common::parseRenderMode(ConfMan.get("render_mode"));

	_enableHiResDithering = (_configRenderMode == Common::kRenderEGA && _flags.useHiRes);

	_screen = new Screen_EoB(this, _system);
	assert(_screen);
	_screen->setResolution();

	_res = new Resource(this);
	assert(_res);
	_res->reset();

	_staticres = new StaticResource(this);
	assert(_staticres);
	if (!_staticres->init())
		error("_staticres->init() failed");

	// We start the respective sound driver even if "No Music" has been selected, because we
	// don't have a null driver class (and don't really need one). We just disable the sound here.
	MidiDriver::DeviceHandle dev = 0;
	switch (_flags.platform) {
	case Common::kPlatformDOS: {
		int flags = MDT_ADLIB | MDT_PCSPK;
		dev = MidiDriver::detectDevice(_flags.gameID == GI_EOB1 ? flags | MDT_PCJR : flags);
		MusicType type = MidiDriver::getMusicType(dev);
		_sound = new SoundPC_v1(this, _mixer, type == MT_ADLIB ? Sound::kAdLib : type == MT_PCSPK ? Sound::kPCSpkr : Sound::kPCjr);
		} break;
	case Common::kPlatformFMTowns:
		dev = MidiDriver::detectDevice(MDT_TOWNS);
		_sound = new SoundTowns_Darkmoon(this, _mixer);
		break;
	case Common::kPlatformPC98:
		if (_flags.gameID == GI_EOB1) {
			dev = MidiDriver::detectDevice(MDT_PC98);
			_sound = new SoundPC98_EoB(this, _mixer);
		} else {
			dev = MidiDriver::detectDevice(MDT_PC98 | MDT_MIDI);
			/**/
		}
		break;
	case Common::kPlatformAmiga:
		dev = MidiDriver::detectDevice(MDT_AMIGA);
		_sound = new SoundAmiga_EoB(this, _mixer);
		break;
	case Common::kPlatformSegaCD:
		dev = MidiDriver::detectDevice(MDT_SEGACD);
		_sound = new SoundSegaCD_EoB(this, _mixer);
		break;
	default:
		// Dummy error message. Unsupported platforms don't have detection entries.
		error("Unsupported platform '%d'", _flags.platform);
	}

	assert(_sound);
	_sound->init();

	if (_flags.platform == Common::kPlatformPC98)
		_sound->loadSfxFile("EFECT.OBJ");

	// Setup volume settings (and read in all ConfigManager settings)
	_configNullSound = (MidiDriver::getMusicType(dev) == MT_NULL);
	syncSoundSettings();

	if (!_screen->init())
		error("screen()->init() failed");

	if (ConfMan.hasKey("save_slot")) {
		_gameToLoad = ConfMan.getInt("save_slot");
		if (!saveFileLoadable(_gameToLoad))
			_gameToLoad = -1;
	}

	setupKeyMap();

	if (_flags.platform != Common::kPlatformSegaCD) {
		_gui = new GUI_EoB(this);
		assert(_gui);
		_txt = new TextDisplayer_rpg(this, _screen);
		assert(_txt);
	}

	_inf = new EoBInfProcessor(this, _screen);
	assert(_inf);
	setDebugger(new Debugger_EoB(this));

	loadFonts();

	Common::Error err = KyraRpgEngine::init();
	if (err.getCode() != Common::kNoError)
		return err;

	initButtonData();
	initMenus();
	initStaticResource();
	initSpells();

	_timer = new TimerManager(this, _system);
	assert(_timer);
	setupTimers();

	_wllVmpMap[1] = 1;
	_wllVmpMap[2] = 2;
	memset(&_wllVmpMap[3], 3, 20);
	_wllVmpMap[23] = 4;
	_wllVmpMap[24] = 5;

	memcpy(_wllWallFlags, _wllFlagPreset, _wllFlagPresetSize);

	memset(&_specialWallTypes[3], 1, 5);
	memset(&_specialWallTypes[13], 1, 5);
	_specialWallTypes[8] = _specialWallTypes[18] = 6;

	memset(&_wllShapeMap[3], -1, 5);
	memset(&_wllShapeMap[13], -1, 5);

	_wllVcnOffset = (_flags.platform == Common::kPlatformFMTowns) ? 0 : 16;
	int bpp = (_flags.platform == Common::kPlatformFMTowns) ? 2 : 1;

	_greenFadingTable = new uint8[256 * bpp];
	_blueFadingTable = new uint8[256 * bpp];
	_lightBlueFadingTable = new uint8[256 * bpp];
	_blackFadingTable = new uint8[256 * bpp];
	_greyFadingTable = new uint8[256 * bpp];

	_monsters = new EoBMonsterInPlay[30];
	memset(_monsters, 0, 30 * sizeof(EoBMonsterInPlay));

	_characters = new EoBCharacter[6];
	memset(_characters, 0, sizeof(EoBCharacter) * 6);

	_items = new EoBItem[600];
	memset(_items, 0, sizeof(EoBItem) * 600);

	_itemNames = new char*[130];
	for (int i = 0; i < 130; i++) {
		_itemNames[i] = new char[35];
		memset(_itemNames[i], 0, 35);
	}

	_flyingObjects = new EoBFlyingObject[_numFlyingObjects];
	_flyingObjectsPtr = _flyingObjects;
	memset(_flyingObjects, 0, _numFlyingObjects * sizeof(EoBFlyingObject));

	int bufferSize = _flags.useHiColorMode ? 8192 : 4096;
	_spellAnimBuffer = new uint8[bufferSize];
	memset(_spellAnimBuffer, 0, bufferSize);

	_wallsOfForce = new WallOfForce[5];
	memset(_wallsOfForce, 0, 5 * sizeof(WallOfForce));

	memset(_doorType, 0, sizeof(_doorType));
	memset(_noDoorSwitch, 0, sizeof(_noDoorSwitch));

	_monsterShapes = new uint8*[36];
	memset(_monsterShapes, 0, 36 * sizeof(uint8 *));
	_monsterDecorations = new SpriteDecoration[36];
	memset(_monsterDecorations, 0, 36 * sizeof(SpriteDecoration));
	_monsterPalettes = new uint8*[24];
	for (int i = 0; i < 24; i++)
		_monsterPalettes[i] = new uint8[16];

	_doorSwitches = new SpriteDecoration[6];
	memset(_doorSwitches, 0, 6 * sizeof(SpriteDecoration));

	_monsterFlashOverlay = new uint8[16];
	_monsterStoneOverlay = new uint8[16];
	memset(_monsterFlashOverlay, (_configRenderMode == Common::kRenderCGA) ? 0xFF : guiSettings()->colors.guiColorWhite, 16 * sizeof(uint8));
	memset(_monsterStoneOverlay, (_flags.platform == Common::kPlatformAmiga) ? guiSettings()->colors.guiColorWhite : 0x0D, 16 * sizeof(uint8));
	_monsterFlashOverlay[0] = _monsterStoneOverlay[0] = 0;

	return Common::kNoError;
}

void EoBCoreEngine::loadFonts() {
	// Only the fonts that are based on game resource files are loaded here. ScummVM builtin fonts like the
	// FM-Towns ROM font or the PC-98 SJIS font get initialized in Screen::init() and Screen_EoB::init().

	if (_flags.platform == Common::kPlatformAmiga) {
		if (_res->exists("EOBF6.FONT"))
			_screen->loadFont(Screen::FID_6_FNT, "EOBF6.FONT");
		else if (_res->exists("FONTS/EOBF6.FONT"))
			_screen->loadFont(Screen::FID_6_FNT, "FONTS/EOBF6.FONT");
		else
			AmigaDOSFont::errorDialog(0);

		if (_res->exists("EOBF8.FONT"))
			_screen->loadFont(Screen::FID_8_FNT, "EOBF8.FONT");
		else if (_res->exists("FONTS/EOBF8.FONT"))
			_screen->loadFont(Screen::FID_8_FNT, "FONTS/EOBF8.FONT");
		else
			AmigaDOSFont::errorDialog(0);

	} else if (_flags.platform != Common::kPlatformSegaCD) {
		_screen->loadFont(Screen::FID_6_FNT, "FONT6.FNT");
		_screen->loadFont(Screen::FID_8_FNT, "FONT8.FNT");
	}

	if (_flags.platform == Common::kPlatformFMTowns) {
		_screen->loadFont(Screen::FID_SJIS_SMALL_FNT, "FONT.DMP");
	} else if (_flags.platform == Common::kPlatformPC98) {
		_screen->loadFont(Screen::FID_SJIS_SMALL_FNT, "FONT12.FNT");
		_invFont1 = Screen::FID_SJIS_SMALL_FNT;
		_conFont = _invFont3 = Screen::FID_SJIS_FNT;
	} else if (_flags.platform == Common::kPlatformSegaCD) {
		_screen->loadFont(Screen::FID_8_FNT, "FONTK12");
		_screen->setFontStyles(Screen::FID_8_FNT, Font::kStyleNone);
		_invFont1 = _invFont2 = _conFont = Screen::FID_8_FNT;
	}
}

Common::Error EoBCoreEngine::go() {
	static_cast<Debugger_EoB *>(getDebugger())->initialize();
	_txt->removePageBreakFlag();
	_screen->setFont(_flags.platform == Common::kPlatformPC98 ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
	loadItemsAndDecorationsShapes();

	_screen->setMouseCursor(0, 0, _itemIconShapes[0]);

	// Import original save game files (especially the "Quick Start Party").
	// The SegaCD version has a "Default Party" main menu option instead.
	if (ConfMan.getBool("importOrigSaves")) {
		if (_flags.platform != Common::kPlatformSegaCD)
			importOriginalSaveFile(-1);
		ConfMan.setBool("importOrigSaves", false);
		ConfMan.flushToDisk();
	}

	loadItemDefs();
	int action = 0;

	for (bool repeatLoop = true; repeatLoop; repeatLoop ^= true) {
		action = 0;

		if (_gameToLoad != -1) {
			startupLoad();
			if (loadGameState(_gameToLoad).getCode() != Common::kNoError)
				error("Couldn't load game slot %d on startup", _gameToLoad);
			_gameToLoad = -1;
		} else {
			_screen->showMouse();
			action = mainMenu();
		}

		if (action == -1) {
			// load game
			startupLoad();
			repeatLoop = _gui->runLoadMenu(_flags.platform == Common::kPlatformSegaCD ? 80 : 72, _flags.platform == Common::kPlatformSegaCD ? 16 : 14, true);
			if (!repeatLoop)
				startupReset();
		} else if (action == -2 || action == -4) {
			// new game
			repeatLoop = startCharacterGeneration(action == -4);
			if (repeatLoop && !shouldQuit())
				startupNew();
			else
				startupReset();
		} else if (action == -3) {
			// transfer party
			repeatLoop = startPartyTransfer();
			if (repeatLoop && !shouldQuit())
				startupNew();
		}
	}

	if (!shouldQuit() && action >= -4) {
		runLoop();

		if (_playFinale) {
			// make final save for party transfer
			saveGameStateIntern(-1, 0, 0);
			_sound->selectAudioResourceSet(kMusicFinale);
			seq_playFinale();
		}
	}

	return Common::kNoError;
}

void EoBCoreEngine::registerDefaultSettings() {
	KyraEngine_v1::registerDefaultSettings();
	ConfMan.registerDefault("hpbargraphs", true);
	ConfMan.registerDefault("mousebtswap", false);
	ConfMan.registerDefault("importOrigSaves", true);
}

void EoBCoreEngine::readSettings() {
	_configHpBarGraphs = ConfMan.getBool("hpbargraphs");
	_configMouseBtSwap = ConfMan.getBool("mousebtswap");
	_configSounds = ConfMan.getBool("sfx_mute") ? 0 : 1;
	_configMusic = (_flags.platform == Common::kPlatformPC98 || _flags.platform == Common::kPlatformSegaCD) ? (ConfMan.getBool("music_mute") ? 0 : 1) : (_configSounds ? 1 : 0);

	if (_sound) {
		_sound->enableMusic(_configNullSound ? false : _configMusic);
		_sound->enableSFX(_configNullSound ? false : _configSounds);
	}
}

void EoBCoreEngine::writeSettings() {
	ConfMan.setBool("hpbargraphs", _configHpBarGraphs);
	ConfMan.setBool("mousebtswap", _configMouseBtSwap);
	ConfMan.setBool("sfx_mute", _configSounds == 0);
	if (_flags.platform == Common::kPlatformPC98 || _flags.platform == Common::kPlatformSegaCD)
		ConfMan.setBool("music_mute", _configMusic == 0);

	if (_sound) {
		if (_flags.platform == Common::kPlatformPC98 || _flags.platform == Common::kPlatformSegaCD) {
			if (!_configMusic)
				snd_playSong(0);
		} else if (!_configSounds) {
			_sound->haltTrack();
		}
		_sound->enableMusic(_configNullSound ? false : _configMusic);
		_sound->enableSFX(_configNullSound ? false : _configSounds);
	}

	ConfMan.flushToDisk();
}

void EoBCoreEngine::startupNew() {
	gui_setPlayFieldButtons();
	_screen->_curPage = 0;
	gui_drawPlayField(false);
	_screen->_curPage = 0;
	gui_drawAllCharPortraitsWithStats();
	drawScene(1);
	_updateFlags = 0;
	_updateCharNum = 0;
}

void EoBCoreEngine::runLoop() {
	_envAudioTimer = _system->getMillis() + (rollDice(1, 10, 3) * 18 * _tickLength);
	_flashShapeTimer = 0;
	_drawSceneTimer = _system->getMillis();
	_screen->setFont(_conFont);
	_screen->setScreenDim(7);

	_runFlag = true;

	while (!shouldQuit() && _runFlag) {
		checkPartyStatus(true);
		checkInput(_activeButtons, true, 0);
		removeInputTop();

		if (!_runFlag)
			break;

		_timer->update();
		updateScriptTimers();
		updateWallOfForceTimers();

		if (_sceneUpdateRequired && !_sceneShakeCountdown)
			drawScene(1);

		updateAnimTimers();

		uint32 curTime = _system->getMillis();
		if (_envAudioTimer < curTime && !(_flags.gameID == GI_EOB1 && (_flags.platform == Common::kPlatformSegaCD || _flags.platform == Common::kPlatformAmiga || _currentLevel == 0 || _currentLevel > 3))) {
			_envAudioTimer = curTime + (rollDice(1, 10, 3) * 18 * _tickLength);
			snd_processEnvironmentalSoundEffect(_flags.gameID == GI_EOB1 ? 30 : (rollDice(1, 2, -1) ? 27 : 28), _currentBlock + rollDice(1, 12, -1));
		}

		snd_updateLevelScore();
		snd_updateEnvironmentalSfx(0);
		turnUndeadAuto();
	}
}

bool EoBCoreEngine::checkPartyStatus(bool handleDeath) {
	int numChars = 0;
	for (int i = 0; i < 6; i++)
		numChars += testCharacter(i, 13);

	if (numChars)
		return false;

	if (!handleDeath)
		return true;

	gui_drawAllCharPortraitsWithStats();

	if (checkPartyStatusExtra()) {
		Screen::FontId of = _screen->setFont(_flags.use16ColorMode ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
		gui_updateControls();
		int x = 0;
		int y = 0;
		if (_flags.platform == Common::kPlatformSegaCD) {
			startupLoad();
			x = 80;
			y = 16;
		}
		if (_gui->runLoadMenu(x, y)) {
			_screen->setFont(of);
			return true;
		}
	}

	if (_flags.platform == Common::kPlatformSegaCD)
		_screen->sega_fadeToBlack(1);

	if (!shouldQuit())
		quitGame();

	return false;
}

void EoBCoreEngine::updateAnimTimers() {
	uint32 curTime = _system->getMillis();
	if (_lastSecTick + 1000 <= curTime) {
		_lastSecTick = curTime;
		_totalPlaySecs++;
	}

	if (_lastVIntTick + 16 <= curTime) {
		_lastVIntTick = curTime;
		gui_updateAnimations();
	}
}

void EoBCoreEngine::loadItemsAndDecorationsShapes() {
	releaseItemsAndDecorationsShapes();
	int div = (_flags.gameID == GI_EOB1) ? 3 : 8;
	int mul = (_flags.gameID == GI_EOB1) ? 64 : 24;

	_largeItemShapes = new const uint8*[_numLargeItemShapes];
	_screen->loadShapeSetBitmap("ITEML1", 5, 3);
	for (int i = 0; i < _numLargeItemShapes; i++)
		_largeItemShapes[i] = _screen->encodeShape((i / div) << 3, (i % div) * mul, 8, 24, false, _cgaMappingItemsL);

	if (_flags.gameID == GI_EOB1) {
		for (int c = 0; c < 3; ++c) {
			_largeItemShapesScl[c] = new const uint8*[_numLargeItemShapes];
			for (int i = 0; i < _numLargeItemShapes; i++)
				_largeItemShapesScl[c][i] = _screen->encodeShape((i / div) << 3, (i % div) * mul + 24 + (c << 4), 6 - 2 * c, 16 - ((c >> 1) << 3), false, _cgaMappingItemsL);
		}
	}

	_smallItemShapes = new const uint8*[_numSmallItemShapes];
	_screen->loadShapeSetBitmap("ITEMS1", 5, 3);
	for (int i = 0; i < _numSmallItemShapes; i++)
		_smallItemShapes[i] = _screen->encodeShape((i / div) << 2, (i % div) * mul, 4, 24, false, _cgaMappingItemsS);

	if (_flags.gameID == GI_EOB1) {
		for (int c = 0; c < 3; ++c) {
			_smallItemShapesScl[c] = new const uint8*[_numSmallItemShapes];
			for (int i = 0; i < _numSmallItemShapes; i++)
				_smallItemShapesScl[c][i] = _screen->encodeShape((i / div) << 2, (i % div) * mul + 24 + (c << 4), 3 - c, 16 - ((c >> 1) << 3), false, _cgaMappingItemsS);
		}
	}

	_thrownItemShapes = new const uint8*[_numThrownItemShapes];
	if (_flags.gameID == GI_EOB2)
		_spellShapes = new const uint8*[4];
	_firebeamShapes = new const uint8*[3];

	_screen->loadShapeSetBitmap("THROWN", 5, 3);
	for (int i = 0; i < _numThrownItemShapes; i++)
		_thrownItemShapes[i] = _screen->encodeShape((i / div) << 2, (i % div) * mul, 4, 24, false, _cgaMappingThrown);

	if (_flags.gameID == GI_EOB1) {
		for (int c = 0; c < 3; ++c) {
			_thrownItemShapesScl[c] = new const uint8*[_numThrownItemShapes];
			for (int i = 0; i < _numThrownItemShapes; i++)
				_thrownItemShapesScl[c][i] = _screen->encodeShape((i / div) << 2, (i % div) * mul + 24 + (c << 4), 3 - c, 16 - ((c >> 1) << 3), false, _cgaMappingThrown);
		}
	} else {
		for (int i = 0; i < 4; i++)
			_spellShapes[i] = _screen->encodeShape(8, i << 5, 6, 32, false, _cgaMappingThrown);
	}

	_firebeamShapes[0] = _screen->encodeShape(16, 0, 4, 24, false, _cgaMappingThrown);
	_firebeamShapes[1] = _screen->encodeShape(16, 24, 4, 24, false, _cgaMappingThrown);
	_firebeamShapes[2] = _screen->encodeShape(16, 48, 3, 24, false, _cgaMappingThrown);
	_redSplatShape = _screen->encodeShape(16, _flags.gameID == GI_EOB1 ? 144 : 72, 5, 24, false, _cgaMappingThrown);
	_greenSplatShape = _screen->encodeShape(16, _flags.gameID == GI_EOB1 ? 168 : 96, 5, 16, false, _cgaMappingThrown);

	_itemIconShapes = new const uint8*[_numItemIconShapes];
	_screen->loadShapeSetBitmap("ITEMICN", 5, 3);
	for (int i = 0; i < _numItemIconShapes; i++)
		_itemIconShapes[i] = _screen->encodeShape((i % 0x14) << 1, (i / 0x14) << 4, 2, 0x10, false, _cgaMappingIcons);

	if (_flags.platform == Common::kPlatformAmiga) {
		const uint8 offsY = (_flags.gameID == GI_EOB1) ? 80 : 96;
		_blueItemIconShapes = new const uint8*[_numItemIconShapes];
		for (int i = 0; i < _numItemIconShapes; i++) {
			int bx = (i % 0x14) << 1;
			int by = (i / 0x14) << 4;
			_blueItemIconShapes[i] = _screen->getPagePixel(2, (bx << 3) + 8, by + offsY + 8) ? _screen->encodeShape(bx, by + offsY, 2, 0x10, false, 0) : _screen->encodeShape(bx, by, 2, 0x10, false, 0);
		}
	}

	_teleporterShapes = new const uint8*[6];
	_sparkShapes = new const uint8*[4];
	_compassShapes = new const uint8*[12];
	if (_flags.gameID == GI_EOB2)
		_wallOfForceShapes = new const uint8*[6];

	_screen->loadShapeSetBitmap("DECORATE", 5, 3);
	if (_flags.gameID == GI_EOB2) {
		_lightningColumnShape = _screen->encodeShape(18, 88, 4, 64);
		for (int i = 0; i < 6; i++)
			_wallOfForceShapes[i] = _screen->encodeShape(_wallOfForceShapeDefs[(i << 2)], _wallOfForceShapeDefs[(i << 2) + 1], _wallOfForceShapeDefs[(i << 2) + 2], _wallOfForceShapeDefs[(i << 2) + 3]);
	}

	for (int i = 0; i < 6; i++)
		_teleporterShapes[i] = _screen->encodeShape(_teleporterShapeDefs[(i << 2)], _teleporterShapeDefs[(i << 2) + 1], _teleporterShapeDefs[(i << 2) + 2], _teleporterShapeDefs[(i << 2) + 3], false, _cgaMappingDefault);

	_sparkShapes[0] = _screen->encodeShape(29, 0, 2, 16, false, _cgaMappingDeco);
	_sparkShapes[1] = _screen->encodeShape(31, 0, 2, 16, false, _cgaMappingDeco);
	_sparkShapes[2] = _screen->encodeShape(33, 0, 2, 16, false, _cgaMappingDeco);
	_sparkShapes[3] = 0;
	_deadCharShape = _screen->encodeShape(0, 88, 4, 32, false, _cgaMappingDeco);
	_disabledCharGrid = _screen->encodeShape(4, 88, 4, 32, false, _cgaMappingDeco);
	_blackBoxSmallGrid = _screen->encodeShape(9, 88, 2, 8, false, _cgaMappingDeco);
	_weaponSlotGrid = _screen->encodeShape(8, 88, 4, 16, false, _cgaMappingDeco);
	_blackBoxWideGrid = _screen->encodeShape(8, 104, 4, 8, false, _cgaMappingDeco);

	static const uint8 dHeight[] = { 17, 10, 10 };
	static const uint8 dY[] = { 120, 137, 147 };

	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 4; x++)
			_compassShapes[(y << 2) + x] = _screen->encodeShape(x * 3, dY[y], 3, dHeight[y], false, _cgaMappingDeco);
	}
}

void EoBCoreEngine::releaseItemsAndDecorationsShapes() {
	if (_flags.platform != Common::kPlatformFMTowns || _flags.gameID != GI_EOB2) {
		releaseShpArr(_largeItemShapes, _numLargeItemShapes);
		releaseShpArr(_smallItemShapes, _numSmallItemShapes);
		releaseShpArr(_thrownItemShapes, _numThrownItemShapes);
		releaseShpArr(_spellShapes, 4);
		releaseShpArr(_itemIconShapes, _numItemIconShapes);
		releaseShpArr(_blueItemIconShapes, _numItemIconShapes);
		releaseShpArr(_xtraItemIconShapes, 3);
		releaseShpArr(_sparkShapes, 4);
		releaseShpArr(_wallOfForceShapes, 6);
		releaseShpArr(_compassShapes, 12);
		releaseShpArr(_firebeamShapes, 3);

		// SegaCD uses the spark shapes for drawing the teleporters. We copy only the pointers, not the whole shapes.
		if (_flags.platform != Common::kPlatformSegaCD) {
			releaseShpArr(_teleporterShapes, 6);
		}

		delete[] _redSplatShape;
		delete[] _greenSplatShape;
		delete[] _swapShape;
		delete[] _deadCharShape;
		delete[] _disabledCharGrid;
		delete[] _blackBoxSmallGrid;
		delete[] _weaponSlotGrid;
		delete[] _blackBoxWideGrid;
		delete[] _lightningColumnShape;
	}

	delete[] _largeItemShapes;
	delete[] _smallItemShapes;
	delete[] _thrownItemShapes;
	delete[] _spellShapes;
	delete[] _itemIconShapes;
	delete[] _blueItemIconShapes;
	delete[] _xtraItemIconShapes;
	delete[] _sparkShapes;
	delete[] _wallOfForceShapes;
	delete[] _teleporterShapes;
	delete[] _compassShapes;
	delete[] _firebeamShapes;

	for (int i = 0; i < 3; ++i) {
		releaseShpArr(_largeItemShapesScl[i], _numLargeItemShapes);
		releaseShpArr(_smallItemShapesScl[i], _numSmallItemShapes);
		releaseShpArr(_thrownItemShapesScl[i], _numThrownItemShapes);
		delete[] _largeItemShapesScl[i];
		delete[] _smallItemShapesScl[i];
		delete[] _thrownItemShapesScl[i];
	}
}

void EoBCoreEngine::setHandItem(Item itemIndex) {
	if (itemIndex == -1) {
		if (_flags.platform == Common::kPlatformFMTowns)
			_screen->setMouseCursor(8, 8, _itemIconShapes[37], 0);
		return;
	}

	if (_screen->curDimIndex() == 7 && itemIndex) {
		printFullItemName(itemIndex);
		_txt->printMessage(_takenStrings[0]);
	}

	_itemInHand = itemIndex;
	int icon = _items[_itemInHand].icon;
	const uint8 *shp = _itemIconShapes[icon];
	const uint8 *ovl = 0;
	bool applyBluePal = ((_partyEffectFlags & 2) && (_items[_itemInHand].flags & 0x80)) ? true : false;

	if (_xtraItemIconShapes) {
		bool applyBluePalC = applyBluePal;
		applyBluePal = false;
		if (_items[_itemInHand].nameUnid == 23)
			shp = _xtraItemIconShapes[0];
		else if (_items[_itemInHand].nameUnid == 97)
			shp = _xtraItemIconShapes[1];
		else if (_items[_itemInHand].nameId == 39)
			shp = _xtraItemIconShapes[2];
		else
			applyBluePal = applyBluePalC;
	}

	if (icon && applyBluePal) {
		if (_blueItemIconShapes)
			shp = _blueItemIconShapes[icon];
		else
			ovl = _flags.gameID == GI_EOB1 ? ((_configRenderMode == Common::kRenderCGA) ? _itemsOverlayCGA : &_itemsOverlay[icon << 4]) : _screen->generateShapeOverlay(shp, _lightBlueFadingTable);
	}

	int mouseOffs = itemIndex ? 8 : 0;
	_screen->setMouseCursor(mouseOffs, mouseOffs, shp, ovl);

	if (_flags.useHiColorMode) {
		_screen->setFadeTable(_greyFadingTable);
		_screen->setShapeFadingLevel(0);
	}
}

int EoBCoreEngine::getDexterityArmorClassModifier(int dexterity) {
	static const int8 mod[] = { 5, 5, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1, -2, -3, -4, -4, -5, -5, -5, -6, -6 };
	return mod[dexterity];
}

int EoBCoreEngine::generateCharacterHitpointsByLevel(int charIndex, int levelIndex) {
	EoBCharacter *c = &_characters[charIndex];
	int m = getClassAndConstHitpointsModifier(c->cClass, c->constitutionCur);

	int h = 0;

	for (int i = 0; i < 3; i++) {
		if (!(levelIndex & (1 << i)))
			continue;

		int d = getCharacterClassType(c->cClass, i);

		if (c->level[i] <= _hpIncrPerLevel[6 + i])
			h += rollDice(1, (d >= 0) ? _hpIncrPerLevel[d] : 0);
		else
			h += _hpIncrPerLevel[12 + i];

		h += m;
	}

	h /= _numLevelsPerClass[c->cClass];

	if (h < 1)
		h = 1;

	return h;
}

int EoBCoreEngine::getClassAndConstHitpointsModifier(int cclass, int constitution) {
	int res = _hpConstModifiers[constitution];
	// This also applies to EOB1 despite being coded differently there
	if (res <= 2 || (_classModifierFlags[cclass] & 0x31))
		return res;

	return 2;
}

int EoBCoreEngine::getCharacterClassType(int cclass, int levelIndex) {
	return _characterClassType[cclass * 3 + levelIndex];
}

int EoBCoreEngine::getModifiedHpLimits(int hpModifier, int constModifier, int level, bool mode) {
	int s = _hpIncrPerLevel[6 + hpModifier] > level ? level : _hpIncrPerLevel[6 + hpModifier];
	int res = s;

	if (!mode)
		res *= (hpModifier >= 0 ? _hpIncrPerLevel[hpModifier] : 0);

	if (level > s) {
		s = level - s;
		res += (s * _hpIncrPerLevel[12 + hpModifier]);
	}

	if (!mode || (constModifier > 0))
		res += (level * constModifier);

	return res;
}

Common::String EoBCoreEngine::getCharStrength(int str, int strExt, bool twoDigitsPadding) {
	if (strExt) {
		if (strExt == 100)
			strExt = 0;
		_strenghtStr = Common::String::format(twoDigitsPadding ? "%02d/%02d" : "%d/%02d", str, strExt);
	} else {
		_strenghtStr = Common::String::format(twoDigitsPadding ? "%02d" : "%d", str);
	}

	return _strenghtStr;
}

int EoBCoreEngine::testCharacter(int16 index, int flags) {
	if (index == -1)
		return 0;

	EoBCharacter *c = &_characters[index];
	int res = 1;

	if (flags & 1)
		res &= (c->flags & 1);

	if (flags & 2)
		res &= ((c->hitPointsCur <= -10) || (c->flags & 8)) ? 0 : 1;

	if (flags & 4)
		res &= ((c->hitPointsCur <= 0) || (c->flags & 8)) ? 0 : 1;

	if (flags & 8)
		res &= (c->flags & 12) ? 0 : 1;

	if (flags & 0x20)
		res &= (c->flags & 4) ? 0 : 1;

	if (flags & 0x10)
		res &= (c->flags & 2) ? 0 : 1;

	if (flags & 0x40)
		res &= (c->food <= 0) ? 0 : 1;

	return res;
}

int EoBCoreEngine::getNextValidCharIndex(int curCharIndex, int searchStep) {
	do {
		curCharIndex += searchStep;
		if (curCharIndex < 0)
			curCharIndex = 5;
		if (curCharIndex > 5)
			curCharIndex = 0;
	} while (!testCharacter(curCharIndex, 1));

	return curCharIndex;
}

void EoBCoreEngine::recalcArmorClass(int index) {
	EoBCharacter *c = &_characters[index];
	int acm = getDexterityArmorClassModifier(c->dexterityCur);
	c->armorClass = 10 + acm;

	static uint8 slot[] = { 17, 0, 1, 18 };
	for (int i = 0; i < 4; i++) {
		int itm = c->inventory[slot[i]];
		if (!itm)
			continue;

		if (i == 2) {
			if (!validateWeaponSlotItem(index, 1))
				continue;
		}

		int tp = _items[itm].type;

		if (!(_itemTypes[tp].allowedClasses & _classModifierFlags[c->cClass]) || (_itemTypes[tp].extraProperties & 0x7F) || (i >= 1 && i <= 2 && tp != 27 && !(_flags.gameID == GI_EOB2 && tp == 57)))
			continue;

		c->armorClass += _itemTypes[tp].armorClass;
		c->armorClass -= _items[itm].value;
	}

	if (!_items[c->inventory[17]].value) {
		int8 m1 = 0;
		int8 m2 = 0;

		if (c->inventory[25]) {
			if (!(_itemTypes[_items[c->inventory[25]].type].extraProperties & 0x7F))
				m1 = _items[c->inventory[25]].value;
		}

		if (c->inventory[26]) {
			if (!(_itemTypes[_items[c->inventory[26]].type].extraProperties & 0x7F))
				m2 = _items[c->inventory[26]].value;
		}

		c->armorClass -= MAX(m1, m2);
	}

	if (c->effectsRemainder[0] > 0) {
		if (c->armorClass <= (acm + 6))
			c->effectsRemainder[0] = 0;
		else
			c->armorClass = (acm + 6);
	}

	// shield
	if ((c->effectFlags & 8) && (c->armorClass > 4))
		c->armorClass = 4;

	// magical vestment
	if (c->effectFlags & 0x4000) {
		int8 m1 = 5;

		if (getClericPaladinLevel(index) > 5)
			m1 += ((getClericPaladinLevel(index) - 5) / 3);

		if (c->armorClass > m1)
			c->armorClass = m1;
	}

	if (c->armorClass < -10)
		c->armorClass = -10;
}

int EoBCoreEngine::validateWeaponSlotItem(int index, int slot) {
	EoBCharacter *c = &_characters[index];
	int itm1 = c->inventory[0];
	int r = itemUsableByCharacter(index, itm1);
	int tp1 = _items[itm1].type;

	if (!slot)
		return (!itm1 || r) ? 1 : 0;

	int itm2 = c->inventory[1];
	r = itemUsableByCharacter(index, itm2);
	int tp2 = _items[itm2].type;

	if (itm1 && _itemTypes[tp1].requiredHands == 2)
		return 0;

	if (!itm2)
		return 1;

	int f = (_itemTypes[tp2].extraProperties & 0x7F);
	if (f <= 0 || f > 3)
		return r;

	if (_itemTypes[tp2].requiredHands)
		return 0;

	return r;
}

int EoBCoreEngine::getClericPaladinLevel(int index) {
	if (_castScrollSlot)
		return 9;

	if (index == -1)
		return (_currentLevel < 7) ? 5 : 9;

	int l = getCharacterLevelIndex(2, _characters[index].cClass);
	if (l > -1)
		return _characters[index].level[l];

	l = getCharacterLevelIndex(4, _characters[index].cClass);
	if (l > -1) {
		if (_characters[index].level[l] > 8)
			return _characters[index].level[l] - 8;
	}

	return 1;
}

int EoBCoreEngine::getMageLevel(int index) {
	if (_castScrollSlot)
		return 9;

	if (index == -1)
		return (_currentLevel < 7) ? 5 : 9;

	int l = getCharacterLevelIndex(1, _characters[index].cClass);
	return (l > -1) ? _characters[index].level[l] : 1;
}

int EoBCoreEngine::getCharacterLevelIndex(int type, int cClass) {
	if (getCharacterClassType(cClass, 0) == type)
		return 0;

	if (getCharacterClassType(cClass, 1) == type)
		return 1;

	if (getCharacterClassType(cClass, 2) == type)
		return 2;

	return -1;
}

int EoBCoreEngine::countCharactersWithSpecificItems(int16 itemType, int16 itemValue) {
	int res = 0;
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		if (checkInventoryForItem(i, itemType, itemValue) != -1)
			res++;
	}
	return res;
}

int EoBCoreEngine::checkInventoryForItem(int character, int16 itemType, int16 itemValue) {
	if (character < 0)
		return -1;

	for (int i = 0; i < 27; i++) {
		uint16 inv = _characters[character].inventory[i];
		if (!inv)
			continue;
		if (_items[inv].type != itemType && itemType != -1)
			continue;
		if (_items[inv].value == itemValue || itemValue == -1)
			return i;
	}
	return -1;
}

void EoBCoreEngine::modifyCharacterHitpoints(int character, int16 points) {
	if (!testCharacter(character, 3))
		return;

	EoBCharacter *c = &_characters[character];
	c->hitPointsCur += points;
	if (c->hitPointsCur > c->hitPointsMax)
		c->hitPointsCur = c->hitPointsMax;

	gui_drawHitpoints(character);
	gui_drawCharPortraitWithStats(character);
}

void EoBCoreEngine::neutralizePoison(int character) {
	_characters[character].flags &= ~2;
	_characters[character].effectFlags &= ~0x2000;
	deleteCharEventTimer(character, -34);
	gui_drawCharPortraitWithStats(character);
}

void EoBCoreEngine::npcSequence(int npcIndex) {
	if (_flags.platform != Common::kPlatformSegaCD) {
		_screen->loadShapeSetBitmap("OUTTAKE", 5, 3);
		_screen->copyRegion(0, 0, 0, 0, 176, 120, 0, 6, Screen::CR_NO_P_CHECK);

		drawNpcScene(npcIndex);

		Common::SeekableReadStream *s = _res->createReadStream("TEXT.DAT");
		if (s) {
			_screen->loadFileDataToPage(s, 5, 32000);
		} else {
			s = _res->createReadStream("TEXT.CPS");
			if (s->readSint32BE() + 12 == s->size())
				_screen->loadSpecialAmigaCPS("TEXT.CPS", 5, false);
			else
				_screen->loadBitmap("TEXT.CPS", 5, 5, 0, true);
		}
		delete s;

		gui_drawBox(0, 121, 320, 79, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
		_txt->setupField(9, true);
		_txt->resetPageBreakString();
	}

	runNpcDialogue(npcIndex);

	_txt->removePageBreakFlag();
	gui_restorePlayField();
}

void EoBCoreEngine::initNpc(int npcIndex) {
	EoBCharacter *c = _characters;
	int i = 0;
	for (; i < 6; i++) {
		if (!(_characters[i].flags & 1)) {
			c = &_characters[i];
			break;
		}
	}

	delete[] c->faceShape;
	memcpy(c, &_npcPreset[npcIndex], sizeof(EoBCharacter));
	Common::strlcpy(c->name, _npcPresetNames[npcIndex], 21);
	recalcArmorClass(i);
	makeFaceShapes(i);
	makeNameShapes(i);

	for (i = 0; i < 25; i++) {
		if (!c->inventory[i])
			continue;
		c->inventory[i] = duplicateItem(c->inventory[i]);
	}
}

int EoBCoreEngine::npcJoinDialogue(int npcIndex, int queryJoinTextId, int confirmJoinTextId, int noJoinTextId) {
	gui_drawDialogueBox();
	int r = runDialogue(queryJoinTextId, _flags.platform == Common::kPlatformSegaCD ? 3 : 2, _flags.platform == Common::kPlatformSegaCD ? 3 : -1, _yesNoStrings[0], _yesNoStrings[1], _flags.platform == Common::kPlatformSegaCD ? _yesNoStrings[2] : 0) - 1;
	if (r == 0) {
		if (confirmJoinTextId == -1) {
			Common::String tmp = Common::String::format(_npcJoinStrings[0], _npcPresetNames[npcIndex]);
			_txt->printDialogueText(tmp.c_str(), true);
		} else {
			_txt->printDialogueText(confirmJoinTextId, _okStrings[0]);
		}

		if (prepareForNewPartyMember(33, npcIndex + 1))
			initNpc(npcIndex);

	} else if (r == 1 && noJoinTextId != -1) {
		_txt->printDialogueText(noJoinTextId, _okStrings[0]);
	}

	return r ^ 1;
}

int EoBCoreEngine::prepareForNewPartyMember(int16 itemType, int16 itemValue) {
	int numChars = 0;
	for (int i = 0; i < 6; i++)
		numChars += (_characters[i].flags & 1);

	if (numChars < 6) {
		deletePartyItems(itemType, itemValue);
	} else {
		gui_drawDialogueBox();
		_screen->set16bitShadingLevel(4);
		_txt->printDialogueText(_npcMaxStrings[0]);
		_screen->set16bitShadingLevel(0);

		if (_flags.platform == Common::kPlatformSegaCD) {
			resetSkipFlag();
			_allowSkip = true;
			while (!(shouldQuit() || skipFlag()))
				delay(20);
			_allowSkip = false;
			resetSkipFlag();
		}

		int r = runDialogue(-1, 7, -1, _characters[0].name, _characters[1].name, _characters[2].name, _characters[3].name, _characters[4].name, _characters[5].name, _abortStrings[0]) - 1;

		if (r == 6)
			return 0;

		deletePartyItems(itemType, itemValue);
		removeCharacterFromParty(r);
	}

	return 1;
}

void EoBCoreEngine::dropCharacter(int charIndex) {
	if (!testCharacter(charIndex, 1))
		return;

	removeCharacterFromParty(charIndex);

	if (charIndex < 5)
		exchangeCharacters(charIndex, testCharacter(5, 1) ? 5 : 4);

	gui_processCharPortraitClick(0);
	gui_setPlayFieldButtons();
	setupCharacterTimers();
}

void EoBCoreEngine::removeCharacterFromParty(int charIndex) {
	EoBCharacter *c = &_characters[charIndex];
	c->flags = 0;

	for (int i = 0; i < 27; i++) {
		if (i == 16 || !c->inventory[i])
			continue;

		setItemPosition((Item *)&_levelBlockProperties[_currentBlock & 0x3FF].drawObjects, _currentBlock, c->inventory[i], _dropItemDirIndex[(_currentDirection << 2) + rollDice(1, 2, -1)]);
		c->inventory[i] = 0;
	}

	while (c->inventory[16])
		setItemPosition((Item *)&_levelBlockProperties[_currentBlock & 0x3FF].drawObjects, _currentBlock, getQueuedItem(&c->inventory[16], 0, -1), _dropItemDirIndex[(_currentDirection << 2) + rollDice(1, 2, -1)]);

	c->inventory[16] = 0;

	if (_updateCharNum == charIndex)
		_updateCharNum = 0;

	setupCharacterTimers();
}

void EoBCoreEngine::exchangeCharacters(int charIndex1, int charIndex2) {
	EoBCharacter temp;
	memcpy(&temp, &_characters[charIndex1], sizeof(EoBCharacter));
	memcpy(&_characters[charIndex1], &_characters[charIndex2], sizeof(EoBCharacter));
	memcpy(&_characters[charIndex2], &temp, sizeof(EoBCharacter));
}

void EoBCoreEngine::increasePartyExperience(int16 points) {
	int cnt = 0;
	for (int i = 0; i < 6; i++) {
		if (testCharacter(i, 3))
			cnt++;
	}

	if (cnt <= 0)
		return;

	points /= cnt;

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 3))
			continue;
		increaseCharacterExperience(i, points);
	}
}

void EoBCoreEngine::increaseCharacterExperience(int charIndex, int32 points) {
	int cl = _characters[charIndex].cClass;
	points /= _numLevelsPerClass[cl];

	for (int i = 0; i < 3; i++) {
		if (getCharacterClassType(cl, i) == -1)
			continue;
		_characters[charIndex].experience[i] += points;

		uint32 er = getRequiredExperience(cl, i, _characters[charIndex].level[i] + 1);
		if (er == 0xFFFFFFFF)
			continue;

		if (_characters[charIndex].experience[i] >= er)
			increaseCharacterLevel(charIndex, i);
	}
}

uint32 EoBCoreEngine::getRequiredExperience(int cClass, int levelIndex, int level) {
	cClass = getCharacterClassType(cClass, levelIndex);
	if (cClass == -1)
		return 0xFFFFFFFF;

	const uint32 *tbl = _expRequirementTables[cClass];
	return tbl[level - 1];
}

void EoBCoreEngine::increaseCharacterLevel(int charIndex, int levelIndex) {
	_characters[charIndex].level[levelIndex]++;
	int hpInc = generateCharacterHitpointsByLevel(charIndex, 1 << levelIndex);
	_characters[charIndex].hitPointsCur += hpInc;
	_characters[charIndex].hitPointsMax += hpInc;

	gui_drawCharPortraitWithStats(charIndex);
	_txt->printMessage(_levelGainStrings[0], -1, _characters[charIndex].name);
	snd_playSoundEffect(_flags.platform == Common::kPlatformSegaCD ? 0x1017 : 0x17);
}

void EoBCoreEngine::setWeaponSlotStatus(int charIndex, int mode, int slot) {
	if (mode == 0 || mode == 2)
		_characters[charIndex].disabledSlots ^= (1 << slot);
	else if (mode != 1)
		return;

	_characters[charIndex].slotStatus[slot] = 0;
	gui_drawCharPortraitWithStats(charIndex);
}

void EoBCoreEngine::setupDialogueButtons(int presetfirst, int numStr, va_list &args) {
	_dialogueNumButtons = numStr;
	_dialogueHighlightedButton = 0;

	Screen::FontId of = _screen->setFont((_flags.gameID == GI_EOB2 && _flags.platform == Common::kPlatformFMTowns) ? Screen::FID_8_FNT : _screen->_currentFont);

	for (int i = 0; i < numStr; i++) {
		const char *s = va_arg(args, const char*);
		if (s)
			_dialogueButtonString[i] = s;
		else
			_dialogueNumButtons = numStr = i;
	}

	const ScreenDim *dm = screen()->_curDim;
	int yOffs = (_txt->lineCount() + 1) * _screen->getFontHeight() + dm->sy + 4;

	_dialogueButtonPosX = &guiSettings()->buttons.posX[presetfirst];
	_dialogueButtonPosY = &guiSettings()->buttons.posY[presetfirst];
	_dialogueButtonXoffs = (_flags.platform == Common::kPlatformSegaCD) ? 8 : 0;
	_dialogueButtonYoffs = (_flags.platform == Common::kPlatformSegaCD) ? 160 : yOffs;

	drawDialogueButtons();

	_screen->setFont(of);

	if (!shouldQuit())
		removeInputTop();
}

void EoBCoreEngine::initDialogueSequence() {
	_npcSequenceSub = -1;
	_txt->setWaitButtonMode(0);
	_dialogueField = true;
	_dialogueLastBitmap[0] = 0;

	_txt->resetPageBreakString();
	gui_updateControls();
	//_allowSkip = true;

	// WORKAROUND for bug in the original code (all platforms). Sequence sound would be terminated prematurely.
	if (_flags.gameID == GI_EOB2 && _currentLevel == 2 && _currentBlock == 654)
		_sound->stopAllSoundEffects();
	else
		snd_stopSound();

	Common::SeekableReadStream *s = _res->createReadStream("TEXT.DAT");
	if (s) {
		_screen->loadFileDataToPage(s, 5, 32000);
	} else {
		s = _res->createReadStream("TEXT.CPS");
		if (s->readSint32BE() + 12 == s->size())
			_screen->loadSpecialAmigaCPS("TEXT.CPS", 5, false);
		else
			_screen->loadBitmap("TEXT.CPS", 5, 5, 0, true);
	}
	delete s;

	_txt->setupField(9, 0);
}

void EoBCoreEngine::restoreAfterDialogueSequence() {
	_txt->allowPageBreak(false);
	_dialogueField = _dialogueFieldAmiga = false;

	_dialogueLastBitmap[0] = 0;

	gui_restorePlayField();
	//_allowSkip = false;
	_screen->setScreenDim(7);

	if (_flags.gameID == GI_EOB2)
		snd_playSoundEffect(2);

	_sceneUpdateRequired = true;
}

void EoBCoreEngine::drawSequenceBitmap(const char *file, int destRect, int x1, int y1, int flags) {
	static const uint8 frameX[] = { 1, 0 };
	static const uint8 frameY[] = { 8, 0 };
	static const uint8 frameW[] = { 20, 40 };
	static const uint8 frameH[] = { 96, 121 };

	int page = ((flags & 2) || destRect) ? 0 : 6;
	int amigaPalIndex = (x1 ? 1 : 0) + (y1 ? 2 : 0) + 1;

	if (scumm_stricmp(_dialogueLastBitmap, file)) {
		_screen->clearPage(2);
		if (!destRect) {
			if (!(flags & 1)) {
				_screen->loadEoBBitmap("BORDER", 0, 3, 3, 2);
				if (_flags.platform == Common::kPlatformAmiga)
					_screen->copyRegion(0, 0, 0, 0, 320, 122, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->copyRegion(0, 0, 0, 0, 184, 121, 2, page, Screen::CR_NO_P_CHECK);
			} else {
				_screen->copyRegion(0, 0, 0, 0, 184, 121, 0, page, Screen::CR_NO_P_CHECK);
			}

			if (!page)
				_screen->copyRegion(0, 0, 0, 0, 184, 121, 2, 6, Screen::CR_NO_P_CHECK);
		}

		_screen->loadEoBBitmap(file, 0, 3, 3, 2);
		strcpy(_dialogueLastBitmap, file);
	}

	if (_flags.platform == Common::kPlatformAmiga) {
		int cp = _screen->setCurPage(0);
		if (!_dialogueFieldAmiga)
			gui_drawDialogueBox();
		_screen->drawClippedLine(0, 120, 319, 120, 9);
		_screen->drawClippedLine(0, 121, 319, 121, guiSettings()->colors.fill);
		_screen->setPagePixel(0, 319, 121, 9);
		_screen->setCurPage(cp);
		_screen->setDualPalettes(_screen->getPalette(amigaPalIndex), _screen->getPalette(7));
		_dialogueFieldAmiga = true;
	}

	if (flags & 2)
		_screen->crossFadeRegion(x1 << 3, y1, frameX[destRect] << 3, frameY[destRect], frameW[destRect] << 3, frameH[destRect], 2, page);
	else
		_screen->copyRegion(x1 << 3, y1, frameX[destRect] << 3, frameY[destRect], frameW[destRect] << 3, frameH[destRect], 2, page, Screen::CR_NO_P_CHECK);

	if (page == 6)
		_screen->copyRegion(0, 0, 0, 0, 184, (_flags.platform == Common::kPlatformAmiga) ? 110 : 121, 6, 0, Screen::CR_NO_P_CHECK);

	_screen->updateScreen();
}

int EoBCoreEngine::runDialogue(int dialogueTextId, int numStr, int loopButtonId, ...) {
	int res;
	do {
		res = 0;
		if (dialogueTextId != -1)
			txt()->printDialogueText(dialogueTextId, 0);

		va_list args;
		va_start(args, loopButtonId);
		if (_flags.platform == Common::kPlatformSegaCD && numStr > 3)
			setupDialogueButtons(numStr == 4 ? 14 : 5, numStr, args);
		else if (numStr > 2)
			setupDialogueButtons(2, numStr, args);
		else
			setupDialogueButtons(0, numStr, args);
		va_end(args);

		while (res == 0 && !shouldQuit())
			res = processDialogue();
	} while (res == loopButtonId && !shouldQuit());

	if (_flags.platform != Common::kPlatformSegaCD)
		gui_drawDialogueBox();

	return res;
}

void EoBCoreEngine::restParty_displayWarning(const char *str) {
	int od = _screen->curDimIndex();
	_screen->setScreenDim(7);
	Screen::FontId of = _screen->setFont(_conFont);
	_screen->setCurPage(0);

	_txt->printMessage(Common::String::format(_flags.platform == Common::kPlatformSegaCD ? "%s" : "\r%s\r", str).c_str());

	_screen->setFont(of);
	_screen->setScreenDim(od);
}

bool EoBCoreEngine::restParty_updateMonsters() {
	bool sfxEnabled = _sound->sfxEnabled();
	bool musicEnabled = _sound->musicEnabled();
	_sound->enableSFX(false);
	_sound->enableMusic(false);

	for (int i = 0; i < 5; i++) {
		_partyResting = true;

		// The original SegaCD code does not update the monsters during resting. I presume to
		// avoid graphical issues which I have (apparently successfully) tried to fix. At first
		// I had disabled the monster updated just like the original, but it annoyed me, since
		// it felt like a step backwards...
		Screen::FontId of = _screen->setFont(_conFont);
		int od = _screen->curDimIndex();
		_screen->setScreenDim(7);
		updateMonsters(0);
		updateMonsters(1);
		timerProcessFlyingObjects(0);
		_screen->setScreenDim(od);
		_screen->setFont(of);

		_partyResting = false;

		for (int ii = 0; ii < 30; ii++) {
			if (_monsters[ii].mode == 8)
				continue;
			if (getBlockDistance(_currentBlock, _monsters[ii].block) >= 2)
				continue;

			restParty_displayWarning(_menuStringsRest4[0]);
			_sound->enableSFX(sfxEnabled);
			_sound->enableMusic(musicEnabled);
			return true;
		}
	}

	_sound->enableSFX(sfxEnabled);
	_sound->enableMusic(musicEnabled);
	return false;
}

int EoBCoreEngine::restParty_getCharacterWithLowestHp() {
	int lhp = 900;
	int res = -1;

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 3))
			continue;
		if (_characters[i].hitPointsCur >= _characters[i].hitPointsMax)
			continue;
		if (_characters[i].hitPointsCur < lhp) {
			lhp = _characters[i].hitPointsCur;
			res = i;
		}
	}

	return res + 1;
}

bool EoBCoreEngine::restParty_checkHealSpells(int charIndex) {
	static const uint8 eob1healSpells[] = { 2, 15, 20 };
	static const uint8 eob2healSpells[] = { 3, 16, 20 };
	const uint8 *spells = _flags.gameID == GI_EOB1 ? eob1healSpells : eob2healSpells;
	const int8 *list = _characters[charIndex].clericSpells;

	for (int i = 0; i < 80; i++) {
		int s = list[i] < 0 ? -list[i] : list[i];
		if (s == spells[0] || s == spells[1] || s == spells[2])
			return true;
	}

	return false;
}

bool EoBCoreEngine::restParty_checkSpellsToLearn() {
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 0x43))
			continue;

		if ((getCharacterLevelIndex(2, _characters[i].cClass) != -1 || getCharacterLevelIndex(4, _characters[i].cClass) != -1) && (checkInventoryForItem(i, 30, -1) != -1)) {
			for (int ii = 0; ii < 80; ii++) {
				if (_characters[i].clericSpells[ii] < 0)
					return true;
			}
		}

		if ((getCharacterLevelIndex(1, _characters[i].cClass) != -1) && (checkInventoryForItem(i, 29, -1) != -1)) {
			for (int ii = 0; ii < 80; ii++) {
				if (_characters[i].mageSpells[ii] < 0)
					return true;
			}
		}
	}

	return false;
}

bool EoBCoreEngine::restParty_extraAbortCondition() {
	return false;
}

void EoBCoreEngine::delay(uint32 millis, bool, bool) {
	while (millis && !shouldQuit() && !(_allowSkip && skipFlag())) {
		updateInput();
		uint32 step = MIN<uint32>(millis, (_tickLength / 5));
		_system->delayMillis(step);
		millis -= step;
	}
}

void EoBCoreEngine::pauseEngineIntern(bool pause) {
	KyraEngine_v1::pauseEngineIntern(pause);
	seq_segaPausePlayer(pause);
}

void EoBCoreEngine::displayParchment(int id) {
	_txt->setWaitButtonMode(1);
	_txt->resetPageBreakString();
	gui_updateControls();

	if (id >= 0) {
		// display text
		Common::SeekableReadStream *s = _res->createReadStream("TEXT.DAT");
		if (s) {
			_screen->loadFileDataToPage(s, 5, 32000);
		} else {
			s = _res->createReadStream("TEXT.CPS");
			if (s->readSint32BE() + 12 == s->size())
				_screen->loadSpecialAmigaCPS("TEXT.CPS", 5, false);
			else
				_screen->loadBitmap("TEXT.CPS", 5, 5, 0, true);
		}
		delete s;
		_screen->set16bitShadingLevel(4);
		gui_drawBox(0, 0, 176, 175, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
		_screen->set16bitShadingLevel(0);
		_txt->setupField(12, 1);
		if (_flags.gameID == GI_EOB2)
			id++;
		_txt->printDialogueText(id, _okStrings[0]);

	} else {
		// display bitmap
		static const uint8 x[] = { 0, 20, 0 };
		static const uint8 y[] = { 0, 0, 96 };
		id = -id - 1;

		if (_flags.platform == Common::kPlatformAmiga)
			_txt->setupField(9, 0);

		drawSequenceBitmap("MAP", 0, x[id], y[id], 0);

		removeInputTop();
		while (!shouldQuit()) {
			delay(_tickLength);
			if (checkInput(0, false, 0) & 0xFF)
				break;
			removeInputTop();
		}
		removeInputTop();
	}

	restoreAfterDialogueSequence();
}

int EoBCoreEngine::countResurrectionCandidates() {
	_rrCount = 0;
	memset(_rrNames, 0, 10 * sizeof(const char *));

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		if (_characters[i].hitPointsCur != -10)
			continue;

		_rrNames[_rrCount] = _characters[i].name;
		_rrId[_rrCount++] = i;
	}

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;

		for (int ii = 0; ii < 27; ii++) {
			uint16 inv = _characters[i].inventory[ii];
			if (!inv)
				continue;

			if ((_flags.gameID == GI_EOB1 && ((_itemTypes[_items[inv].type].extraProperties & 0x7F) != 8)) || (_flags.gameID == GI_EOB2 && _items[inv].type != 33))
				continue;

			_rrNames[_rrCount] = _npcPresetNames[_items[inv].value - 1];
			_rrId[_rrCount++] = -_items[inv].value;
		}
	}

	if (_itemInHand > 0) {
		if ((_flags.gameID == GI_EOB1 && ((_itemTypes[_items[_itemInHand].type].extraProperties & 0x7F) == 8)) || (_flags.gameID == GI_EOB2 && _items[_itemInHand].type == 33)) {
			_rrNames[_rrCount] = _npcPresetNames[_items[_itemInHand].value - 1];
			_rrId[_rrCount++] = -_items[_itemInHand].value;
		}
	}

	return _rrCount;
}

void EoBCoreEngine::seq_portal() {
	const uint8 **shapes = makePortalShapes();
	assert(shapes);

	_screen->copyRegion(24, 0, 24, 0, 144, 104, 2, 5, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(24, 0, 24, 0, 144, 104, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->drawShape(2, shapes[11], 28, 9, 0);
	_screen->drawShape(2, shapes[1], 34, 28, 0);
	_screen->drawShape(2, shapes[6], 120, 28, 0);
	_screen->drawShape(2, shapes[0], 56, 27, 0);

	if (_flags.platform == Common::kPlatformSegaCD) {
		snd_playSoundEffect(19);
		_screen->copyRegion(24, 0, 24, 0, 144, 104, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	} else {
		snd_playSoundEffect(33);
		snd_playSoundEffect(19);
		_screen->crossFadeRegion(24, 0, 24, 0, 144, 104, 2, 0);
		delay(30 * _tickLength);
	}

	_screen->copyRegion(24, 0, 24, 0, 144, 104, 5, 2, Screen::CR_NO_P_CHECK);

	for (const int8 *pos = _portalSeq; *pos > -1 && !shouldQuit();) {
		int s = *pos++;
		_screen->drawShape(0, shapes[11 + s], 28, 9, 0);
		_screen->drawShape(0, shapes[1 + s], 34, 28, 0);
		_screen->drawShape(0, shapes[6 + s], 120, 28, 0);

		if (_flags.platform != Common::kPlatformSegaCD) {
			if ((s == 1) && (pos >= _portalSeq + 3)) {
				if (*(pos - 3) == 0) {
					snd_playSoundEffect(24);
					snd_playSoundEffect(86);
				}
			}
		}

		s = *pos++;
		if (s == 0) {
			_screen->drawShape(0, shapes[0], 56, 27, 0);
		} else {
			s--;
			_screen->copyRegion((s % 5) << 6, s / 5 * 77, 56, 27, 64, 77, 2, 0, Screen::CR_NO_P_CHECK);
		}

		if (_flags.platform != Common::kPlatformSegaCD) {
			if (s == 1)
				snd_playSoundEffect(31);
			else if (s == 3) {
				if (*(pos - 2) == 3)
					snd_playSoundEffect(90);
			}
		}

		_screen->updateScreen();
		delay(2 * _tickLength);
	}

	for (int i = 0; i < 16; i++)
		delete[] shapes[i];
	delete[] shapes;
}

const uint8 **EoBCoreEngine::makePortalShapes() {
	const uint8 **shapes = new const uint8*[16];
	_screen->loadShapeSetBitmap("PORTALA", 5, 3);

	for (int i = 0; i < 5; i++) {
		shapes[1 + i] = _screen->encodeShape(i * 3, 0, 3, 75, false, _cgaMappingDefault);
		shapes[6 + i] = _screen->encodeShape(i * 3, 80, 3, 75, false, _cgaMappingDefault);
		shapes[11 + i] = _screen->encodeShape(15, i * 18, 15, 18, false, _cgaMappingDefault);
	}

	shapes[0] = _screen->encodeShape(30, 0, 8, 77, false, _cgaMappingDefault);
	_screen->loadEoBBitmap("PORTALB", _cgaMappingDefault, 5, 3, 2);

	return shapes;
}

bool EoBCoreEngine::checkPassword() {
	char answ[20];
	Screen::FontId of = _screen->setFont(Screen::FID_8_FNT);
	_screen->copyPage(0, Screen_EoB::kCheckPwBackupPage);

	_screen->setScreenDim(13);
	gui_drawBox(_screen->_curDim->sx << 3, _screen->_curDim->sy, _screen->_curDim->w << 3, _screen->_curDim->h, guiSettings()->colors.frame1, guiSettings()->colors.frame2, -1);
	gui_drawBox((_screen->_curDim->sx << 3) + 1, _screen->_curDim->sy + 1, (_screen->_curDim->w << 3) - 2, _screen->_curDim->h - 2, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
	_screen->modifyScreenDim(13, _screen->_curDim->sx + 1, _screen->_curDim->sy + 2, _screen->_curDim->w - 2, _screen->_curDim->h - 16);

	for (int i = 0; i < 3; i++) {
		_screen->fillRect(_screen->_curDim->sx << 3, _screen->_curDim->sy, ((_screen->_curDim->sx + _screen->_curDim->w) << 3) - 1, (_screen->_curDim->sy + _screen->_curDim->h) - 1, guiSettings()->colors.fill);
		int c = rollDice(1, _mnNumWord - 1, -1);
		const uint8 *shp = (_mnDef[c << 2] < _numLargeItemShapes) ? _largeItemShapes[_mnDef[c << 2]] : (_mnDef[c << 2] < 15 ? 0 : _smallItemShapes[_mnDef[c << 2] - 15]);
		assert(shp);
		_screen->drawShape(0, shp, 100, 2, 13);
		_screen->printShadedText(Common::String::format(_mnPrompt[0], _mnDef[(c << 2) + 1], _mnDef[(c << 2) + 2]).c_str(), (_screen->_curDim->sx + 1) << 3, _screen->_curDim->sy, guiSettings()->colors.guiColorWhite, guiSettings()->colors.fill, guiSettings()->colors.guiColorBlack);
		memset(answ, 0, 20);
		gui_drawBox(76, 100, 133, 14, guiSettings()->colors.frame2, guiSettings()->colors.frame1, -1);
		gui_drawBox(77, 101, 131, 12, guiSettings()->colors.frame2, guiSettings()->colors.frame1, -1);
		if (_gui->getTextInput(answ, 10, 103, 15, guiSettings()->colors.guiColorWhite, guiSettings()->colors.fill, guiSettings()->colors.guiColorDarkRed) < 0)
			i = 3;
		if (!scumm_stricmp(_mnWord[c], answ))
			break;
		else if (i == 2)
			return false;
	}

	_screen->modifyScreenDim(13, _screen->_curDim->sx - 1, _screen->_curDim->sy - 2, _screen->_curDim->w + 2, _screen->_curDim->h + 16);
	_screen->setFont(of);
	_screen->copyPage(Screen_EoB::kCheckPwBackupPage, 0);
	return true;
}

Common::String EoBCoreEngine::convertAsciiToSjis(Common::String str) {
	if (_flags.platform != Common::kPlatformFMTowns)
		return str;

	Common::String n;
	const char *src = str.c_str();
	int pos = 0;
	for (uint32 i = 0; i < str.size(); ++i) {
		if (src[i] & 0x80) {
			n.insertChar(src[i++], pos++);
			n.insertChar(src[i], pos++);
		} else if (src[i] >= 32 && src[i] <= 64) {
			n.insertChar(_ascii2SjisTables[1][(src[i] - 32) * 2], pos++);
			n.insertChar(_ascii2SjisTables[1][(src[i] - 32) * 2 + 1], pos++);
		} else if ((src[i] >= 97 && src[i] <= 122) || (src[i] >= 65 && src[i] <= 90)) {
			char c = (src[i] >= 97) ? src[i] - 97 : src[i] - 65;
			n.insertChar(_ascii2SjisTables2[0][c * 2], pos++);
			n.insertChar(_ascii2SjisTables2[0][c * 2 + 1], pos++);
		}
	}

	return n;
}

void EoBCoreEngine::useSlotWeapon(int charIndex, int slotIndex, Item item) {
	EoBCharacter *c = &_characters[charIndex];
	int tp = item ? _items[item].type : 0;

	if (c->effectFlags & 0x40)
		removeCharacterEffect(_flags.gameID == GI_EOB1 ? 8 : 10, charIndex, 1); // remove invisibility effect

	int ep = _itemTypes[tp].extraProperties & 0x7F;
	int8 inflict = 0;

	if (ep == 1) {
		inflict = closeDistanceAttack(charIndex, item);
		if (!inflict)
			inflict = -1;
		snd_playSoundEffect(32);
		playStrikeAnimation(inflict > 0 ? (_monsters[_dstMonsterIndex].pos == 4 ? 4 : _dscItemPosIndex[(_currentDirection << 2) | (_monsters[_dstMonsterIndex].pos & 3)]) : 4, item);
	} else if (ep == 2) {
		inflict = thrownAttack(charIndex, slotIndex, item);
	} else if (ep == 3) {
		inflict = projectileWeaponAttack(charIndex, item);
		gui_drawCharPortraitWithStats(charIndex);
	}

	if (inflict > 0) {
		if (_items[item].flags & 8) {
			c->hitPointsCur += inflict;
			gui_drawCharPortraitWithStats(charIndex);
		}

		if (_items[item].flags & 0x10)
			c->inventory[slotIndex] = 0;

		inflictMonsterDamage(&_monsters[_dstMonsterIndex], inflict, true);
	}

	c->disabledSlots ^= (1 << slotIndex);
	c->slotStatus[slotIndex] = inflict;

	gui_drawCharPortraitWithStats(charIndex);
	setCharEventTimer(charIndex, 18, inflict >= -2 ? slotIndex + 2 : slotIndex, 1);
}

int EoBCoreEngine::closeDistanceAttack(int charIndex, Item item) {
	if (charIndex > 1)
		return -3;

	uint16 d = calcNewBlockPosition(_currentBlock, _currentDirection);
	int r = getClosestMonster(charIndex, d);

	if (r == -1) {
		uint8 w = _specialWallTypes[_levelBlockProperties[d].walls[_sceneDrawVarDown]];
		if (w == 0xFF) {
			if (_flags.gameID == GI_EOB1) {
				_levelBlockProperties[d].walls[_sceneDrawVarDown]++;
				_levelBlockProperties[d].walls[_sceneDrawVarDown ^ 2]++;

			} else {
				for (int i = 0; i < 4; i++) {
					if (_specialWallTypes[_levelBlockProperties[d].walls[i]] == 0xFF)
						_levelBlockProperties[d].walls[i]++;
				}
			}
			_sceneUpdateRequired = true;

		} else if ((_flags.gameID == GI_EOB1) || (_flags.gameID == GI_EOB2 && w != 8 && w != 9)) {
			return -1;
		}

		return (_flags.gameID == GI_EOB2 && ((_itemTypes[_items[item].type].allowedClasses & 4) || !item)) ? -5 : -2;

	} else {
		if (_monsters[r].flags & 0x20) {
			killMonster(&_monsters[r], 1);
			_txt->printMessage(_monsterDustStrings[0]);
			return -2;
		}

		if (!characterAttackHitTest(charIndex, r, item, 1))
			return -1;

		uint16 flg = 0x100;

		if (isMagicEffectItem(item))
			flg |= 1;

		_dstMonsterIndex = r;
		return calcMonsterDamage(&_monsters[r], charIndex, item, 1, flg, 5, 3);
	}

	return 0;
}

int EoBCoreEngine::thrownAttack(int charIndex, int slotIndex, Item item) {
	int d = charIndex > 3 ? charIndex - 2 : charIndex;
	if (!launchObject(charIndex, item, _currentBlock, _dropItemDirIndex[(_currentDirection << 2) + d], _currentDirection, _items[item].type))
		return 0;

	snd_playSoundEffect(11);
	_characters[charIndex].inventory[slotIndex] = 0;
	reloadWeaponSlot(charIndex, slotIndex, -1, 0);
	_sceneUpdateRequired = true;
	return 0;
}

int EoBCoreEngine::projectileWeaponAttack(int charIndex, Item item) {
	int tp = _items[item].type;

	if (_flags.gameID == GI_EOB1)
		assert(tp >= 7);

	int t = _projectileWeaponAmmoTypes[_flags.gameID == GI_EOB1 ? tp - 7 : tp];
	Item ammoItem = 0;

	if (t == 16) {
		if (_characters[charIndex].inventory[0] && _items[_characters[charIndex].inventory[0]].type == 16)
			SWAP(ammoItem, _characters[charIndex].inventory[0]);
		else if (_characters[charIndex].inventory[1] && _items[_characters[charIndex].inventory[1]].type == 16)
			SWAP(ammoItem, _characters[charIndex].inventory[1]);
		else if (_characters[charIndex].inventory[16])
			ammoItem = getQueuedItem(&_characters[charIndex].inventory[16], 0, -1);

	} else {
		for (int i = 0; i < 27; i++) {
			if (_items[_characters[charIndex].inventory[i]].type == t) {
				SWAP(ammoItem, _characters[charIndex].inventory[i]);
				if (i < 2)
					gui_drawCharPortraitWithStats(charIndex);
				break;
			}
		}
	}

	if (!ammoItem)
		return -4;

	int c = charIndex;
	if (c > 3)
		c -= 2;

	if (launchObject(charIndex, ammoItem, _currentBlock, _dropItemDirIndex[(_currentDirection << 2) + c], _currentDirection, tp)) {
		snd_playSoundEffect(tp == 7 ? 26 : 11);
		_sceneUpdateRequired = true;
	}

	return 0;
}

void EoBCoreEngine::inflictMonsterDamage(EoBMonsterInPlay *m, int damage, bool giveExperience) {
	m->hitPointsCur -= damage;
	m->flags = (m->flags & 0xF7) | 1;

	if (_monsterProps[m->type].capsFlags & 0x2000) {
		explodeMonster(m);
		checkSceneUpdateNeed(m->block);
		m->hitPointsCur = 0;
	} else {
		if (checkSceneUpdateNeed(m->block)) {
			m->flags |= 2;
			if (_preventMonsterFlash)
				return;
			flashMonsterShape(m);
		}
	}

	if (m->hitPointsCur <= 0) {
		if (_flags.platform == Common::kPlatformSegaCD)
			snd_playSoundEffect(0x1082);
		killMonster(m, giveExperience);
	} else if (getBlockDistance(m->block, _currentBlock) < 4) {
		m->dest = _currentBlock;
	}
}

void EoBCoreEngine::calcAndInflictMonsterDamage(EoBMonsterInPlay *m, int times, int pips, int offs, int flags, int savingThrowType, int savingThrowEffect) {
	int dmg = calcMonsterDamage(m, times, pips, offs, flags, savingThrowType, savingThrowEffect);
	if (dmg > 0)
		inflictMonsterDamage(m, dmg, flags & 0x800 ? true : false);
}

void EoBCoreEngine::calcAndInflictCharacterDamage(int charIndex, int times, int itemOrPips, int useStrModifierOrBase, int flags, int savingThrowType, int savingThrowEffect) {
	int dmg = calcCharacterDamage(charIndex, times, itemOrPips, useStrModifierOrBase, flags, savingThrowType, savingThrowEffect);
	if (dmg)
		inflictCharacterDamage(charIndex, dmg);
}

int EoBCoreEngine::calcCharacterDamage(int charIndex, int times, int itemOrPips, int useStrModifierOrBase, int flags, int savingThrowType, int savingThrowEffect) {
	int s = (flags & 0x100) ? calcDamageModifers(times, 0, itemOrPips, _items[itemOrPips].type, useStrModifierOrBase) : rollDice(times, itemOrPips, useStrModifierOrBase);
	EoBCharacter *c = &_characters[charIndex];

	if (savingThrowType != 5) {
		if (trySavingThrow(c, _charClassModifier[c->cClass], c->level[0], savingThrowType, c->raceSex >> 1 /*fix bug in original code by adding a right shift*/))
			s = savingThrowReduceDamage(savingThrowEffect, s);
	}

	if ((flags & 0x110) == 0x110) {
		if (!calcDamageCheckItemType(_items[itemOrPips].type))
			s = 1;
	}

	if (flags & 4) {
		if (checkInventoryForRings(charIndex, 3))
			s = 0;
	}

	if (flags & 0x400) {
		if (c->effectFlags & 0x2000)
			s = 0;
		else
			_txt->printMessage(_characterStatusStrings8[0], -1, c->name);
	}

	return s;
}

void EoBCoreEngine::inflictCharacterDamage(int charIndex, int damage) {
	EoBCharacter *c = &_characters[charIndex];
	if (!testCharacter(charIndex, 3))
		return;

	if (c->effectsRemainder[3])
		c->effectsRemainder[3] = (damage < c->effectsRemainder[3]) ? (c->effectsRemainder[3] - damage) : 0;

	c->hitPointsCur -= damage;
	c->damageTaken = damage;

	if (c->hitPointsCur > -10) {
		snd_playSoundEffect(21);
		if (_flags.platform == Common::kPlatformSegaCD)
			_sceneShakeCountdown = c->gfxUpdateCountdown = 32;
	} else {
		c->hitPointsCur = -10;
		c->flags &= 1;
		c->food = 0;
		removeAllCharacterEffects(charIndex);
		snd_playSoundEffect(_flags.platform == Common::kPlatformSegaCD ? 0x8001 + (c->raceSex & 1) : 22);
	}

	if (c->effectsRemainder[0]) {
		c->effectsRemainder[0] = (damage < c->effectsRemainder[0]) ? (c->effectsRemainder[0] - damage) : 0;
		if (!c->effectsRemainder[0])
			removeCharacterEffect(1, charIndex, 1);
	}

	if (_currentControlMode)
		gui_drawFaceShape(charIndex);
	else
		gui_drawCharPortraitWithStats(charIndex);

	if (c->hitPointsCur <= 0 && _updateFlags == 1 && charIndex == _openBookChar) {
		Button b;
		clickedSpellbookAbort(&b);
	}

	setCharEventTimer(charIndex, 18, 6, 1);
}

bool EoBCoreEngine::characterAttackHitTest(int charIndex, int monsterIndex, int item, int attackType) {
	if (charIndex < 0)
		return true;

	int p = item ? (_flags.gameID == GI_EOB1 ? _items[item].type : (_itemTypes[_items[item].type].extraProperties & 0x7F)) : 0;

	if (_monsters[monsterIndex].flags & 0x20)
		return true;// EOB 2 only ?

	int t = _monsters[monsterIndex].type;
	int d = (p < 1 || p > 3) ? 0 : _items[item].value;

	if (_flags.gameID == GI_EOB2) {
		if ((p > 0 && p < 4) || !item) {
			if (((_monsterProps[t].immunityFlags & 0x200) && (d <= 0)) || ((_monsterProps[t].immunityFlags & 0x1000) && (d <= 1)))
				return false;
		}
	}

	d += (attackType ? getStrHitChanceModifier(charIndex) : getDexHitChanceModifier(charIndex));

	int m = getMonsterAcHitChanceModifier(charIndex, _monsterProps[t].armorClass) - d;
	int s = rollDice(1, 20);

	_monsters[monsterIndex].flags |= 1;

	if (_flags.gameID == GI_EOB1) {
		if (_partyEffectFlags & 0x30)
			s++;
		if (_characters[charIndex].effectFlags & 0x40)
			s++;
	} else if ((_partyEffectFlags & 0x8400) || (_characters[charIndex].effectFlags & 0x1000)) {
		s++;
	}

	s = CLIP(s, 1, 20);

	return s >= m;
}

bool EoBCoreEngine::monsterAttackHitTest(EoBMonsterInPlay *m, int charIndex) {
	int tp = m->type;
	EoBMonsterProperty *p = &_monsterProps[tp];

	int r = rollDice(1, 20);
	if (r != 20) {
		// Prot from evil
		if (_characters[charIndex].effectFlags & 0x800)
			r -= 2;
		// blur
		if (_characters[charIndex].effectFlags & 0x10)
			r -= 2;
		// prayer
		if (_partyEffectFlags & 0x8000)
			r--;
	}

	return ((r == 20) || (r >= (p->hitChance - _characters[charIndex].armorClass)));
}

bool EoBCoreEngine::flyingObjectMonsterHit(EoBFlyingObject *fo, int monsterIndex) {
	if (fo->attackerId != -1) {
		if (!characterAttackHitTest(fo->attackerId, monsterIndex, fo->item, 0))
			return false;
	}
	calcAndInflictMonsterDamage(&_monsters[monsterIndex], fo->attackerId, fo->item, 0, (fo->attackerId == -1) ? 0x110 : 0x910, 5, 3);
	return true;
}

bool EoBCoreEngine::flyingObjectPartyHit(EoBFlyingObject *fo) {
	int ps = _dscItemPosIndex[(_currentDirection << 2) + (_items[fo->item].pos & 3)];
	bool res = false;

	bool b = ((_currentDirection == fo->direction || _currentDirection == (fo->direction ^ 2)) && ps > 2);
	int s = ps << 1;
	if (ps > 2)
		s += rollDice(1, 2, -1);

	static const int8 charId[] = { 0, -1, 1, -1, 2, 4, 3, 5 };

	for (int i = 0; i < 2; i++) {
		int c = charId[s];
		s ^= 1;
		if (!testCharacter(c, 3))
			continue;
		calcAndInflictCharacterDamage(c, -1, fo->item, 0, 0x110, 5, 3);
		res = true;
		if (ps < 2 || b == 0)
			break;
	}

	return res;
}

void EoBCoreEngine::monsterCloseAttack(EoBMonsterInPlay *m) {
	int first = _monsterCloseAttDstTable1[(_currentDirection << 2) + m->dir] * 12;
	int v = (m->pos == 4) ? rollDice(1, 2, -1) : _monsterCloseAttChkTable2[(m->dir << 2) + m->pos];
	if (!v)
		first += 6;

	int last = first + 6;
	for (int i = first; i < last; i++) {
		int c = _monsterCloseAttDstTable2[i];
		if (!testCharacter(c, 3))
			continue;

		// Character Invisibility
		if ((_characters[c].effectFlags & 0x140) && (rollDice(1, 20) >= 5))
			continue;

		int dmg = 0;
		for (int ii = 0; ii < _monsterProps[m->type].attacksPerRound; ii++) {
			if (!monsterAttackHitTest(m, c))
				continue;
			dmg += rollDice(_monsterProps[m->type].dmgDc[ii].times, _monsterProps[m->type].dmgDc[ii].pips, _monsterProps[m->type].dmgDc[ii].base);
			if (_characters[c].effectsRemainder[1]) {
				if (--_characters[c].effectsRemainder[1])
					dmg = 0;
			}
		}

		if (dmg > 0) {
			if ((_monsterProps[m->type].capsFlags & 0x80) && rollDice(1, 4, -1) != 3) {
				int slot = rollDice(1, 27, -1);
				for (int iii = 0; iii < 27; iii++) {
					Item itm = _characters[c].inventory[slot];
					if (!itm || !(_itemTypes[_items[itm].type].extraProperties & 0x80)) {
						if (++slot == 27)
							slot = 0;
						continue;
					}

					_characters[c].inventory[slot] = 0;
					_txt->printMessage(_ripItemStrings[(_characters[c].raceSex & 1) ^ 1], -1, _characters[c].name);
					printFullItemName(itm);
					_txt->printMessage(_ripItemStrings[2]);
					break;
				}
				gui_drawCharPortraitWithStats(c);
			}

			inflictCharacterDamage(c, dmg);

			if (_monsterProps[m->type].capsFlags & 0x10) {
				statusAttack(c, 2, _monsterSpecAttStrings[_flags.gameID == GI_EOB1 ? 3 : 2], 0, 1, 8, 1);
				_characters[c].effectFlags &= ~0x2000;
			}

			if (_monsterProps[m->type].capsFlags & 0x20)
				statusAttack(c, 4, _monsterSpecAttStrings[_flags.gameID == GI_EOB1 ? 4 : 3], 2, 5, 9, 1);

			if (_monsterProps[m->type].capsFlags & 0x8000)
				statusAttack(c, 8, _monsterSpecAttStrings[4], 2, 0, 0, 1);

		}

		if (!(_monsterProps[m->type].capsFlags & 0x4000))
			return;
	}
}

void EoBCoreEngine::monsterSpellCast(EoBMonsterInPlay *m, int type) {
	launchMagicObject(-1, type, m->block, m->pos, m->dir);
	snd_processEnvironmentalSoundEffect(_spells[_magicFlightObjectProperties[type << 2]].sound, m->block);
}

void EoBCoreEngine::statusAttack(int charIndex, int attackStatusFlags, const char *attackStatusString, int savingThrowType, uint32 effectDuration, int restoreEvent, int noRefresh) {
	EoBCharacter *c = &_characters[charIndex];
	if ((c->flags & attackStatusFlags) && noRefresh)
		return;
	if (!testCharacter(charIndex, 3))
		return;

	if (savingThrowType != 5 && specialAttackSavingThrow(charIndex, savingThrowType))
		return;

	if (attackStatusFlags & 8) {
		removeAllCharacterEffects(charIndex);
		c->flags = (c->flags & 1) | 8;
	} else {
		c->flags |= attackStatusFlags;
	}

	if ((attackStatusFlags & 0x0C) && (_openBookChar == charIndex) && _updateFlags) {
		Button b;
		clickedSpellbookAbort(&b);
	}

	if (effectDuration)
		setCharEventTimer(charIndex, effectDuration * 546, restoreEvent, 1);

	gui_drawCharPortraitWithStats(charIndex);
	_txt->printMessage(_characterStatusStrings13[0], -1, c->name, attackStatusString);
}

int EoBCoreEngine::calcMonsterDamage(EoBMonsterInPlay *m, int times, int pips, int offs, int flags, int savingThrowType, int savingThrowEffect) {
	int s = flags & 0x100 ? calcDamageModifers(times, m, pips, _items[pips].type, offs) : rollDice(times, pips, offs);
	EoBMonsterProperty *p = &_monsterProps[m->type];

	if (savingThrowType != 5) {
		if (trySavingThrow(m, 0, p->level, savingThrowType, 6))
			s = savingThrowReduceDamage(savingThrowEffect, s);
	}

	if ((flags & 0x110) == 0x110) {
		if (!calcDamageCheckItemType(_items[pips].type))
			s = 1;
	}

	if ((flags & 0x100) && (!(_itemTypes[_items[pips].type].allowedClasses & 4 /* bug in original code ??*/))
	    && ((_flags.gameID == GI_EOB2 && (p->immunityFlags & 0x100)) || (_flags.gameID == GI_EOB1 && (p->capsFlags & 4))))
		s >>= 1;

	if (p->immunityFlags & 0x2000) {
		if (flags & 0x100) {
			if (_items[pips].value < 3)
				s >>= 2;
			if (_items[pips].value == 3)
				s >>= 1;
			if (s == 0)
				s = _items[pips].value;

		} else {
			s >>= 1;
		}
	}

	if (flags & 1) {
		if (tryMonsterAttackEvasion(m))
			s = 0;
	}

	if (_flags.gameID == GI_EOB1)
		return s;

	static const uint16 damageImmunityFlags[] = { 0x01, 0x10, 0x02, 0x20, 0x80, 0x400, 0x20, 0x800, 0x40, 0x80, 0x400, 0x40 };
	for (int i = 0; i < 12; i += 2) {
		if ((flags & damageImmunityFlags[i]) && (p->immunityFlags & damageImmunityFlags[i + 1]))
			s = 0;
	}

	return s;
}

int EoBCoreEngine::calcDamageModifers(int charIndex, EoBMonsterInPlay *m, int item, int itemType, int useStrModifier) {
	int s = (useStrModifier && (charIndex != -1)) ? getStrDamageModifier(charIndex) : 0;
	if (item) {
		EoBItemType *p = &_itemTypes[itemType];
		int t = m ? m->type : 0;
		s += ((m && (_monsterProps[t].capsFlags & 1)) ? rollDice(p->dmgNumDiceL, p->dmgNumPipsL, p->dmgIncS /* bug in original code ? */) :
		      rollDice(p->dmgNumDiceS, p->dmgNumPipsS, p->dmgIncS));
		s += _items[item].value;
	} else {
		s += rollDice(1, 2);
	}

	return (s < 0) ? 0 : s;
}

bool EoBCoreEngine::trySavingThrow(void *target, int hpModifier, int level, int type, int race) {
	static const int8 constMod[] = { 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5 };

	if (type == 5)
		return false;

	int s = getSaveThrowModifier(hpModifier, level, type);
	if (((race == 3 || race == 5) && (type == 4 || type == 1 || type == 0)) || (race == 4 && (type == 4 || type == 1))) {
		EoBCharacter *c = (EoBCharacter *)target;
		s -= constMod[c->constitutionCur];
	}

	return rollDice(1, 20) >= s;
}

bool EoBCoreEngine::specialAttackSavingThrow(int charIndex, int type) {
	return trySavingThrow(&_characters[charIndex], _charClassModifier[_characters[charIndex].cClass], _characters[charIndex].level[0], type, _characters[charIndex].raceSex >> 1);
}

int EoBCoreEngine::getSaveThrowModifier(int hpModifier, int level, int type) {
	const uint8 *tbl = _saveThrowTables[hpModifier];
	if (_saveThrowLevelIndex[hpModifier] < level)
		level = _saveThrowLevelIndex[hpModifier];
	level /= _saveThrowModDiv[hpModifier];
	level += (_saveThrowModExt[hpModifier] * type);

	return tbl[level];
}

bool EoBCoreEngine::calcDamageCheckItemType(int itemType) {
	itemType = _itemTypes[itemType].extraProperties & 0x7F;
	return (itemType == 2 || itemType == 3) ? true : false;
}

int EoBCoreEngine::savingThrowReduceDamage(int savingThrowEffect, int damage) {
	if (savingThrowEffect == 3)
		return 0;

	if (savingThrowEffect == 0 || savingThrowEffect == 1)
		return damage >> 1;

	return damage;
}

bool EoBCoreEngine::tryMonsterAttackEvasion(EoBMonsterInPlay *m) {
	return rollDice(1, 100) < _monsterProps[m->type].dmgModifierEvade ? true : false;
}

int EoBCoreEngine::getStrHitChanceModifier(int charIndex) {
	static const int8 strExtLimit[] = { 1, 51, 76, 91, 100 };
	static const int8 strExtMod[] = { 1, 2, 2, 2, 3 };
	static const int8 strMod[] = { -4, -3, -3, -2, -2, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 3, 4, 4, 5, 6, 7 };

	int r = strMod[_characters[charIndex].strengthCur - 1];
	if (_characters[charIndex].strengthExtCur) {
		for (int i = 0; i < 5; i++) {
			if (_characters[charIndex].strengthExtCur >= strExtLimit[i])
				r = strExtMod[i];
		}
	}

	return r;
}

int EoBCoreEngine::getStrDamageModifier(int charIndex) {
	static const int8 strExtLimit[] = { 1, 51, 76, 91, 100 };
	static const int8 strExtMod[] = { 3, 3, 4, 5, 6 };
	static const int8 strMod[] = { -3, -2, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 7, 8, 9, 10, 11, 12, 14 };

	int r = strMod[_characters[charIndex].strengthCur - 1];
	if (_characters[charIndex].strengthExtCur) {
		for (int i = 0; i < 5; i++) {
			if (_characters[charIndex].strengthExtCur >= strExtLimit[i])
				r = strExtMod[i];
		}
	}

	return r;
}

int EoBCoreEngine::getDexHitChanceModifier(int charIndex) {
	static const int8 dexMod[] = { -5, -4, -3, -2, -1, 0, 0, 0, 0, 0, 0, 1, 2, 2, 3, 3, 4, 4, 4 };
	return dexMod[_characters[charIndex].dexterityCur - 1];
}

int EoBCoreEngine::getMonsterAcHitChanceModifier(int charIndex, int monsterAc) {
	int l = _characters[charIndex].level[0] - 1;
	int cm = _charClassModifier[_characters[charIndex].cClass];

	return (20 - ((l / _monsterAcHitChanceTable1[cm]) * _monsterAcHitChanceTable2[cm])) - monsterAc;
}

void EoBCoreEngine::explodeMonster(EoBMonsterInPlay *m) {
	m->flags |= 2;
	if (getBlockDistance(m->block, _currentBlock) < 2) {
		explodeObject(0, _currentBlock, 2);
		for (int i = 0; i < 6; i++)
			calcAndInflictCharacterDamage(i, 6, 6, 0, 8, 1, 0);
	} else {
		explodeObject(0, m->block, 2);
	}
	m->flags &= ~2;
}

void EoBCoreEngine::addLevelMap(int level) {
	assert(level);
	_levelMaps |= (1 << (level - 1));
}

bool EoBCoreEngine::hasLevelMap(int level) const {
	return _levelMaps & (1 << (level - 1));
}

uint32 EoBCoreEngine::countMaps() const {
	uint32 res = 0;
	for (int i = 1; i < 13; ++i) {
		if (hasLevelMap(i))
			res++;
	}
	return res;
}

uint32 EoBCoreEngine::countArrows() const {
	uint32 res = 0;
	for (int i = 0; i < 6; ++i)
		res += countQueuedItems(_characters[i].inventory[16], -1, -1, 1, 1);
	return res;
}

void EoBCoreEngine::snd_playSong(int track, bool loop) {
	if (_flags.platform == Common::kPlatformSegaCD && !loop)
		track |= 0x80;
	_sound->playTrack(track);
}

void EoBCoreEngine::snd_playLevelScore() {
	if (_flags.platform == Common::kPlatformPC98) {
		if (_flags.gameID == GI_EOB1)
			snd_playSong(_currentLevel + 1);
	} else if (_flags.platform == Common::kPlatformSegaCD) {
		static const uint8 levelTracksSegaCD[13] = { 7, 7, 7, 7, 6, 6, 6, 4, 4, 4, 5, 5, 10 };
		snd_playSong(levelTracksSegaCD[_currentLevel]);
	}
}

void EoBCoreEngine::snd_playSoundEffect(int track, int volume) {
	if ((track < 1) || (_flags.gameID == GI_EOB2 && track > 119) || shouldQuit())
		return;

	if (_flags.platform == Common::kPlatformSegaCD) {
		if (volume == 0xFF)
			volume = 0x0E;
		if (track == 23 || track == 28)
			track |= 0x1000;
	}

	_sound->playSoundEffect(track, volume);
}

void EoBCoreEngine::snd_stopSound() {
	_sound->haltTrack();
	_sound->stopAllSoundEffects();
}

void EoBCoreEngine::snd_fadeOut(int del) {
	_sound->beginFadeOut(del);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
