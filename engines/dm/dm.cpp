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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "common/scummsys.h"
#include "common/system.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"

#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/events.h"
#include "common/array.h"
#include "common/algorithm.h"

#include "dm/dm.h"
#include "gfx.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "champion.h"
#include "loadsave.h"
#include "objectman.h"
#include "inventory.h"
#include "text.h"
#include "movesens.h"
#include "group.h"
#include "timeline.h"
#include "projexpl.h"

namespace DM {
void warning(bool repeat, const char* s, ...) {
	va_list va;

	va_start(va, s);
	Common::String output = Common::String::vformat(s, va);
	va_end(va);

	if (repeat) {
		::warning(output.c_str());
	} else {
		static Common::Array<Common::String> stringsPrinted;

		if (Common::find(stringsPrinted.begin(), stringsPrinted.end(), s) == stringsPrinted.end()) {
			stringsPrinted.push_back(output);
			::warning(output.c_str());
		}
	}
}

void turnDirRight(direction &dir) { dir = (direction)((dir + 1) & 3); }
void turnDirLeft(direction &dir) { dir = (direction)((dir - 1) & 3); }
direction returnOppositeDir(direction dir) { return (direction)((dir + 2) & 3); }

uint16 returnPrevVal(uint16 val) {
	return (direction)((val + 3) & 3);
}

uint16 returnNextVal(uint16 val) {
	return (val + 1) & 0x3;
}

bool isOrientedWestEast(direction dir) { return dir & 1; }

uint16 toggleFlag(uint16& val, uint16 mask) {
	return val ^= mask;
}

uint16 M75_bitmapByteCount(uint16 pixelWidth, uint16 height) {
	return pixelWidth / 2 * height;
}

uint16 M21_normalizeModulo4(uint16 val) {
	return val & 3;
}

int32 M30_time(int32 map_time) {
	return map_time & 0x00FFFFFF;
}

int32 M33_setMapAndTime(int32 &map_time, uint32 map, uint32 time) {
	return (map_time) = ((time) | (((long)(map)) << 24));
}

uint16 M29_map(int32 map_time) {
	return ((uint16)((map_time) >> 24));
}

Thing M15_thingWithNewCell(Thing thing, int16 cell) {
	return Thing(((thing.toUint16()) & 0x3FFF) | ((cell) << 14));
}

int16 M38_distance(int16 mapx1, int16 mapy1, int16 mapx2, int16 mapy2) {
	return ABS(mapx1 - mapx2) + ABS(mapy1 - mapy2);
}

DMEngine::DMEngine(OSystem *syst) : Engine(syst), _console(nullptr) {
// Do not load data files
// Do not initialize graphics here
// Do not initialize audio devices here
// Do these from run

//Specify all default directories
//const Common::FSNode gameDataDir(ConfMan.get("example"));
//SearchMan.addSubDirectoryMatching(gameDataDir, "example2");
	DebugMan.addDebugChannel(kDMDebugExample, "example", "example desc");

	// register random source
	_rnd = new Common::RandomSource("quux");

	_dungeonMan = nullptr;
	_displayMan = nullptr;
	_eventMan = nullptr;
	_menuMan = nullptr;
	_championMan = nullptr;
	_loadsaveMan = nullptr;
	_objectMan = nullptr;
	_inventoryMan = nullptr;
	_textMan = nullptr;
	_movsens = nullptr;
	_groupMan = nullptr;
	_timeline = nullptr;
	_projexpl = nullptr;

	_g298_newGame = false;
	_g523_restartGameRequest = false;
	_g321_stopWaitingForPlayerInput = true;
	_g301_gameTimeTicking = false;
	_g524_restartGameAllowed = false;
	_g525_gameId = 0;
	_g331_pressingEye = false;
	_g332_stopPressingEye = false;
	_g333_pressingMouth = false;
	_g334_stopPressingMouth = false;
	_g340_highlightBoxInversionRequested = false;
	_g311_projectileDisableMovementTicks = 0;
	_g312_lastProjectileDisabledMovementDirection = 0;
	_g302_gameWon = false;
	_g327_newPartyMapIndex = kM1_mapIndexNone;
	_g325_setMousePointerToObjectInMainLoop = false;
	_g310_disabledMovementTicks = 0;
	_g313_gameTime = 0;
	_g353_stringBuildBuffer[0] = '\0';
	debug("DMEngine::DMEngine");


	warning(false, "DUMMY CODE: setting _g298_newGame to true, should be in processEntrance");
	_g298_newGame = true;
}

DMEngine::~DMEngine() {
	debug("DMEngine::~DMEngine");

	// dispose of resources
	delete _rnd;
	delete _console;
	delete _displayMan;
	delete _dungeonMan;
	delete _eventMan;
	delete _menuMan;
	delete _championMan;
	delete _loadsaveMan;
	delete _objectMan;
	delete _inventoryMan;
	delete _textMan;
	delete _movsens;
	delete _groupMan;
	delete _timeline;
	delete _projexpl;

	// clear debug channels
	DebugMan.clearAllDebugChannels();
}

void DMEngine::waitMs(uint16 ms) {
	_system->delayMillis(ms * 20);
}

uint16 DMEngine::f30_getScaledProduct(uint16 val, uint16 scale, uint16 vale2) {
	return ((uint32)val * vale2) >> scale;
}

void DMEngine::f463_initializeGame() {
	_displayMan->f479_loadGraphics();
	_displayMan->f460_initializeGraphicData();
	// DUMMY CODE: next line
	_displayMan->loadPalette(g21_PalDungeonView[0]);
	_displayMan->f94_loadFloorSet(k0_FloorSetStone);
	_displayMan->f95_loadWallSet(k0_WallSetStone);

	_textMan->f54_textInitialize();
	_objectMan->loadObjectNames();
	_eventMan->initMouse();
	//F0441_STARTEND_ProcessEntrance();
	while (_loadsaveMan->f435_loadgame() != k1_LoadgameSuccess) {
		warning(false, "TODO: F0441_STARTEND_ProcessEntrance");
	}
	//F0396_MENUS_LoadSpellAreaLinesBitmap() is not needed, every bitmap has been loaded


	// There was some memory wizardy for the Amiga platform, I skipped that part
	_displayMan->f461_allocateFlippedWallBitmaps();

	f462_startGame();
	if (_g298_newGame)
		_movsens->f267_getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY);
	_eventMan->f78_showMouse();
	_eventMan->f357_discardAllInput();
}

void DMEngine::f448_initMemoryManager() {
	warning(false, "STUB FUNCTION");
	for (uint16 i = 0; i < 16; ++i)
		_displayMan->_g347_paletteTopAndBottomScreen[i] = g21_PalDungeonView[0][i];
}

void DMEngine::f462_startGame() {
	static Box g61_boxScreenTop(0, 319, 0, 32); // @ G0061_s_Graphic562_Box_ScreenTop
	static Box g62_boxScreenRight(224, 319, 33, 169); // @ G0062_s_Graphic562_Box_ScreenRight
	static Box g63_boxScreenBottom(0, 319, 169, 199); // @ G0063_s_Graphic562_Box_ScreenBottom


	_g331_pressingEye = false;
	_g332_stopPressingEye = false;
	_g333_pressingMouth = false;
	_g334_stopPressingMouth = false;
	_g340_highlightBoxInversionRequested = false;
	_eventMan->_g341_highlightBoxEnabled = false;
	_championMan->_g300_partyIsSleeping = false;
	_championMan->_g506_actingChampionOrdinal = M0_indexToOrdinal(kM1_ChampionNone);
	_menuMan->_g509_actionAreaContainsIcons = true;
	_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap = M0_indexToOrdinal(kM1_ChampionNone);

	_eventMan->_g441_primaryMouseInput = g447_PrimaryMouseInput_Interface;
	_eventMan->_g442_secondaryMouseInput = g448_SecondaryMouseInput_Movement;
	_eventMan->_g443_primaryKeyboardInput = g458_primaryKeyboardInput_interface;
	_eventMan->_g444_secondaryKeyboardInput = g459_secondaryKeyboardInput_movement;

	f3_processNewPartyMap(_dungeonMan->_g309_partyMapIndex);

	if (!_g298_newGame) {
		warning(false, "TODO: loading game");
		assert(false);
	} else {
		_displayMan->_g578_useByteBoxCoordinates = false;
		_displayMan->D24_fillScreenBox(g61_boxScreenTop, k0_ColorBlack);
		_displayMan->D24_fillScreenBox(g62_boxScreenRight, k0_ColorBlack);
		_displayMan->D24_fillScreenBox(g63_boxScreenBottom, k0_ColorBlack);
	}

	warning(false, "TODO: build copper");
	_menuMan->f395_drawMovementArrows();
	_championMan->f278_resetDataToStartGame();
	_g301_gameTimeTicking = true;
}

void DMEngine::f3_processNewPartyMap(uint16 mapIndex) {
	_groupMan->f194_removeAllActiveGroups();
	_dungeonMan->f174_setCurrentMapAndPartyMap(mapIndex);
	_displayMan->f96_loadCurrentMapGraphics();
	_groupMan->f195_addAllActiveGroups();
	_inventoryMan->f337_setDungeonViewPalette();
}

Common::Error DMEngine::run() {
	initArrays();

	// scummvm/engine specific
	initGraphics(320, 200, false);
	_console = new Console(this);
	_displayMan = new DisplayMan(this);
	_dungeonMan = new DungeonMan(this);
	_eventMan = new EventManager(this);
	_menuMan = new MenuMan(this);
	_championMan = new ChampionMan(this);
	_loadsaveMan = new LoadsaveMan(this);
	_objectMan = new ObjectMan(this);
	_inventoryMan = new InventoryMan(this);
	_textMan = new TextMan(this);
	_movsens = new MovesensMan(this);
	_groupMan = new GroupMan(this);
	_timeline = new Timeline(this);
	_projexpl = new ProjExpl(this);
	_displayMan->setUpScreens(320, 200);

	f463_initializeGame(); // @ F0463_START_InitializeGame_CPSADEF
	while (true) {
		f2_gameloop();
		warning(false, "TODO: F0444_STARTEND_Endgame(G0303_B_PartyDead);");
	}

	return Common::kNoError;
}

void DMEngine::f2_gameloop() {
	warning(false, "DUMMY CODE SETTING PARTY POS AND DIRECTION");
	_dungeonMan->_g306_partyMapX = 10;
	_dungeonMan->_g307_partyMapY = 4;
	_dungeonMan->_g308_partyDir = kDirNorth;
	warning(false, "DUMMY CODE: setting InventoryMan::_g432_inventoryChampionOrdinal to zero");
	_inventoryMan->_g432_inventoryChampionOrdinal = 0;

	while (true) {
		if (_g327_newPartyMapIndex != kM1_mapIndexNone) {
T0002002:
			f3_processNewPartyMap(_g327_newPartyMapIndex);
			_movsens->f267_getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY);
			_g327_newPartyMapIndex = kM1_mapIndexNone;
			_eventMan->f357_discardAllInput();
		}
		_timeline->f261_processTimeline();

		if (_g327_newPartyMapIndex != kM1_mapIndexNone)
			goto T0002002;

		if (!_inventoryMan->_g432_inventoryChampionOrdinal && !_championMan->_g300_partyIsSleeping) {
			Box box(0, 223, 0, 135);
			warning(false, "DUMMY CODE: clearing screen to black");
			_displayMan->f135_fillBoxBitmap(_displayMan->_g296_bitmapViewport, box, k0_ColorBlack, k112_byteWidthViewport, k136_heightViewport); // dummy code
			_displayMan->f128_drawDungeon(_dungeonMan->_g308_partyDir, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY);
			if (_g325_setMousePointerToObjectInMainLoop) {
				_g325_setMousePointerToObjectInMainLoop = false;
				_eventMan->f78_showMouse();
				_eventMan->f68_setPointerToObject(_objectMan->_g412_objectIconForMousePointer);
				_eventMan->f77_hideMouse();

			}
			if (_eventMan->_g326_refreshMousePointerInMainLoop) {
				_eventMan->_g326_refreshMousePointerInMainLoop = false;
				_eventMan->_g598_mousePointerBitmapUpdated = true;
				_eventMan->f78_showMouse();
				_eventMan->f77_hideMouse();
			}
		}

		if (_championMan->_g303_partyDead)
			break;
		_g313_gameTime++;

		if (!(_g313_gameTime & 511))
			_inventoryMan->f338_decreaseTorchesLightPower();
			if (_g310_disabledMovementTicks) {
				_g310_disabledMovementTicks--;
			}
		if (_championMan->_g407_party._freezeLifeTicks) {
			_championMan->_g407_party._freezeLifeTicks -= 1;
		}
		_menuMan->f390_refreshActionAreaAndSetChampDirMaxDamageReceived();

		if (_g311_projectileDisableMovementTicks)
			_g311_projectileDisableMovementTicks--;

		_g321_stopWaitingForPlayerInput = false;

		do {
			_eventMan->processInput();

			if (_g332_stopPressingEye) {
				_g331_pressingEye = false;
				_g332_stopPressingEye = false;
				_inventoryMan->f353_drawStopPressingEye();
			} else if (_g334_stopPressingMouth) {
				_g333_pressingMouth = false;
				_g334_stopPressingMouth = false;
				_inventoryMan->f350_drawStopPressingMouth();
			}

			_eventMan->f380_processCommandQueue();
			_displayMan->updateScreen();
			// if (!_vm->_g321_stopWaitingForPlayerInput) {
			warning(false, "MISSING CODE: F0363_COMMAND_HighlightBoxDisable");
			// }
		} while (!_g321_stopWaitingForPlayerInput || !_g301_gameTimeTicking);

		_system->delayMillis(18);
	}
}

int16 DMEngine::M1_ordinalToIndex(int16 val) {
	return val - 1;
}

int16 DMEngine::M0_indexToOrdinal(int16 val) {
	return val + 1;
}

} // End of namespace DM
