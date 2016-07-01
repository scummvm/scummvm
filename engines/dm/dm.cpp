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

namespace DM {

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

uint16 getFlag(uint16 val, uint16 mask) {
	return val & mask;
}

void setFlag(uint16 &val, uint16 mask) {
	val |= mask;
}

void clearFlag(uint16 &val, uint16 mask) {
	val &= ~mask;
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
	_stopWaitingForPlayerInput = false;
	_gameTimeTicking = false;
	_restartGameAllowed = false;
	_pressingEye = false;
	_pressingMouth = false;
	_stopPressingEye = false;
	_stopPressingMouth = false;
	_highlightBoxInversionRequested = false;

	debug("DMEngine::DMEngine");
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

	// clear debug channels
	DebugMan.clearAllDebugChannels();
}

void DMEngine::initializeGame() {
	_displayMan->loadGraphics();
	// DUMMY CODE: next line
	_displayMan->loadPalette(g19_PalCredits);

	_eventMan->initMouse();

	while (_loadsaveMan->loadgame() != kLoadgameSuccess) {
		warning("TODO: F0441_STARTEND_ProcessEntrance");
	}

	_displayMan->loadFloorSet(k0_FloorSetStone);
	_displayMan->loadWallSet(k0_WallSetStone);
	_objectMan->loadObjectNames();

	startGame();
	warning("MISSING CODE: F0267_MOVE_GetMoveResult_CPSCE (if newGame)");
	_eventMan->showMouse(true);
	warning("MISSING CODE: F0357_COMMAND_DiscardAllInput");
}


void DMEngine::startGame() {
	_pressingEye = false;
	_stopPressingEye = false;
	_pressingMouth = false;
	_stopPressingMouth = false;
	_highlightBoxInversionRequested = false;
	_eventMan->_highlightBoxEnabled = false;
	_championMan->_g300_partyIsSleeping = false;
	_championMan->_g506_actingChampionOrdinal = indexToOrdinal(kM1_ChampionNone);
	_menuMan->_actionAreaContainsIcons = true;
	_eventMan->_useChampionIconOrdinalAsMousePointerBitmap = indexToOrdinal(kM1_ChampionNone);

	_eventMan->_primaryMouseInput = gPrimaryMouseInput_Interface;
	_eventMan->_secondaryMouseInput = gSecondaryMouseInput_Movement;
	warning("MISSING CODE: set primary/secondary keyboard input");

	processNewPartyMap(_dungeonMan->_currMap._currPartyMapIndex);

	if (!_dungeonMan->_messages._newGame) {
		warning("TODO: loading game");
	} else {
		_displayMan->_g578_useByteBoxCoordinates = false;
		warning("TODO: clear screen");
	}

	warning("TODO: build copper");
	_menuMan->drawMovementArrows();
	_championMan->resetDataToStartGame();
	_gameTimeTicking = true;
}

void DMEngine::processNewPartyMap(uint16 mapIndex) {
	warning("MISSING CODE: F0194_GROUP_RemoveAllActiveGroups");
	_dungeonMan->setCurrentMapAndPartyMap(mapIndex);
	_displayMan->loadCurrentMapGraphics();
	warning("MISSING CODE: F0195_GROUP_AddAllActiveGroups");
	warning("MISSING CODE: F0337_INVENTORY_SetDungeonViewPalette");
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
	_displayMan->setUpScreens(320, 200);

	initializeGame(); // @ F0463_START_InitializeGame_CPSADEF
	while (true) {
		gameloop();
		warning("TODO: F0444_STARTEND_Endgame(G0303_B_PartyDead);");
	}

	return Common::kNoError;
}

void DMEngine::gameloop() {
	warning("DUMMY CODE SETTING PARTY POS AND DIRECTION");
	_dungeonMan->_currMap._partyPosX = 10;
	_dungeonMan->_currMap._partyPosY = 4;
	_dungeonMan->_currMap._partyDir = kDirNorth;


	warning("DUMMY CODE: setting InventoryMan::_inventoryChampionOrdinal to zero");
	_inventoryMan->_inventoryChampionOrdinal = 0;
	warning("DUMMY CODE: clearing screen to black"); // in loop below
	while (true) {
		_stopWaitingForPlayerInput = false;

		_menuMan->refreshActionAreaAndSetChampDirMaxDamageReceived();

		//do {
		_eventMan->processInput();
		_eventMan->processCommandQueue();
		//} while (!_stopWaitingForPlayerInput || !_gameTimeTicking);

		if (!_inventoryMan->_inventoryChampionOrdinal && !_championMan->_g300_partyIsSleeping) {
			Box box(0, 224, 0, 126);
			_displayMan->clearScreenBox(k0_ColorBlack, box, g296_DungeonViewport); // dummy code
			_displayMan->drawDungeon(_dungeonMan->_currMap._partyDir, _dungeonMan->_currMap._partyPosX, _dungeonMan->_currMap._partyPosY);
		}
		// DUMMY CODE: next line
		_menuMan->drawMovementArrows();
		_displayMan->updateScreen();
		_system->delayMillis(10);
	}
}

int16 DMEngine::ordinalToIndex(int16 val) {
	return val - 1;
}

int16 DMEngine::indexToOrdinal(int16 val) {
	return val + 1;
}

} // End of namespace DM
