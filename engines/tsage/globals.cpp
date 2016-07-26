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

#include "tsage/globals.h"
#include "tsage/tsage.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/ringworld/ringworld_demo.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/ringworld2/ringworld2_logic.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/sherlock/sherlock_logo.h"
#include "tsage/staticres.h"

namespace TsAGE {

Globals *g_globals = NULL;
ResourceManager *g_resourceManager = NULL;

/*--------------------------------------------------------------------------*/

/**
 * Instantiates a saved object that can be instantiated
 */
static SavedObject *classFactoryProc(const Common::String &className) {
	if (className == "ObjectMover") return new ObjectMover();
	if (className == "NpcMover") return new NpcMover();
	if (className == "ObjectMover2") return new ObjectMover2();
	if (className == "ObjectMover3") return new ObjectMover3();
	if (className == "PlayerMover") return new PlayerMover();
	if (className == "SceneObjectWrapper") return new SceneObjectWrapper();
	if (className == "PaletteRotation") return new PaletteRotation();
	if (className == "PaletteFader") return new PaletteFader();
	if (className == "SceneText") return new SceneText();

	// Return to Ringworld specific classes
	if (className == "Scene205_Star") return new Ringworld2::Star();

	return NULL;
}

/*--------------------------------------------------------------------------*/

Globals::Globals() : _dialogCenter(160, 140), _gfxManagerInstance(_screen),
		_randomSource("tsage"), _color1(0), _color2(255), _color3(255) {
	reset();
	_stripNum = 0;
	_gfxEdgeAdjust = 3;
	_gfxFontNumber = 0;

	if (g_vm->getGameID() == GType_Ringworld) {
		if (g_vm->getFeatures() & GF_DEMO) {
			_gfxFontNumber = 0;
			_gfxColors.background = 6;
			_gfxColors.foreground = 0;
			_fontColors.background = 255;
			_fontColors.foreground = 6;
			_dialogCenter.y = 80;
			// Workaround in order to use later version of the engine
			_color1 = _gfxColors.foreground;
			_color2 = _gfxColors.foreground;
			_color3 = _gfxColors.foreground;
		} else if (g_vm->getFeatures() & GF_CD) {
			_gfxFontNumber = 50;
			_gfxColors.background = 53;
			_gfxColors.foreground = 0;
			_fontColors.background = 51;
			_fontColors.foreground = 54;
			_color1 = 18;
			_color2 = 18;
			_color3 = 18;
		} else {
		// Floppy version
			_gfxFontNumber = 50;
			_gfxColors.background = 53;
			_gfxColors.foreground = 18;
			_fontColors.background = 51;
			_fontColors.foreground = 54;
			// Workaround in order to use later version of the engine
			_color1 = _gfxColors.foreground;
			_color2 = _gfxColors.foreground;
			_color3 = _gfxColors.foreground;
		}
	} else if (g_vm->getGameID() == GType_BlueForce) {
		// Blue Force
		_gfxFontNumber = 0;
		_gfxColors.background = 89;
		_gfxColors.foreground = 83;
		_fontColors.background = 88;
		_fontColors.foreground = 92;
		_dialogCenter.y = 140;
	} else if (g_vm->getGameID() == GType_Ringworld2) {
		// Return to Ringworld
		_gfxFontNumber = 50;
		_gfxColors.background = 0;
		_gfxColors.foreground = 59;
		_fontColors.background = 4;
		_fontColors.foreground = 15;
		_color1 = 59;
		_color2 = 15;
		_color3 = 4;
		_dialogCenter.y = 100;
	}

	_gfxManagers.push_back(&_gfxManagerInstance);

	_sceneObjects = &_sceneObjectsInstance;
	_sceneObjects_queue.push_front(_sceneObjects);

	_prevSceneOffset = Common::Point(-1, -1);
	_sounds.push_back(&_soundHandler);
	_sounds.push_back(&_sequenceManager._soundHandler);

	_scrollFollower = nullptr;

	_inventory = nullptr;
	_game = nullptr;
	_sceneHandler = nullptr;

	switch (g_vm->getGameID()) {
	case GType_Ringworld:
		if (!(g_vm->getFeatures() & GF_DEMO)) {
			_inventory = new Ringworld::RingworldInvObjectList();
			_game = new Ringworld::RingworldGame();
		} else {
			_game = new Ringworld::RingworldDemoGame();
		}
		_sceneHandler = new SceneHandler();
		break;

	case GType_BlueForce:
		_game = new BlueForce::BlueForceGame();
		_inventory = new BlueForce::BlueForceInvObjectList();
		_sceneHandler = new BlueForce::SceneHandlerExt();
		break;

	case GType_Ringworld2:
		_inventory = new Ringworld2::Ringworld2InvObjectList();
		_game = new Ringworld2::Ringworld2Game();
		_sceneHandler = new Ringworld2::SceneHandlerExt();
		break;
#ifdef TSAGE_SHERLOCK_ENABLED
	case GType_Sherlock1:
		_inventory = nullptr;
		_sceneHandler = new Sherlock::SherlockSceneHandler();
		_game = new Sherlock::SherlockLogo();
		break;
#endif
	default:
		break;
	}

}

Globals::~Globals() {
	_scenePalette.clearListeners();
	delete _inventory;
	delete _sceneHandler;
	delete _game;
	g_globals = NULL;
}

void Globals::reset() {
	Common::fill(&_flags[0], &_flags[MAX_FLAGS], false);
	g_saver->addFactory(classFactoryProc);
}

void Globals::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		SavedObject::synchronize(s);
	assert(_gfxManagers.size() == 1);

	_sceneItems.synchronize(s);
	SYNC_POINTER(_sceneObjects);
	_sceneObjects_queue.synchronize(s);
	s.syncAsSint32LE(_gfxFontNumber);
	s.syncAsSint32LE(_gfxColors.background);
	s.syncAsSint32LE(_gfxColors.foreground);
	s.syncAsSint32LE(_fontColors.background);
	s.syncAsSint32LE(_fontColors.foreground);

	if (s.getVersion() >= 4) {
		s.syncAsByte(_color1);
		s.syncAsByte(_color2);
		s.syncAsByte(_color3);
	}

	s.syncAsSint16LE(_dialogCenter.x); s.syncAsSint16LE(_dialogCenter.y);
	_sounds.synchronize(s);
	for (int i = 0; i < 256; ++i)
		s.syncAsByte(_flags[i]);

	s.syncAsSint16LE(_sceneOffset.x); s.syncAsSint16LE(_sceneOffset.y);
	s.syncAsSint16LE(_prevSceneOffset.x); s.syncAsSint16LE(_prevSceneOffset.y);
	SYNC_POINTER(_scrollFollower);
	s.syncAsSint32LE(_stripNum);

	if (s.getVersion() >= 8)
		_walkRegions.synchronize(s);
}

void Globals::dispatchSound(ASound *obj) {
	obj->dispatch();
}

void Globals::dispatchSounds() {
	Common::for_each(_sounds.begin(), _sounds.end(), Globals::dispatchSound);
}

/*--------------------------------------------------------------------------*/

TsAGE2Globals::TsAGE2Globals() {
	_onSelectItem = NULL;
	_interfaceY = 0;
}

void TsAGE2Globals::reset() {
	Globals::reset();

	// Reset the inventory
	T2_GLOBALS._uiElements.updateInventory();
	T2_GLOBALS._uiElements._scoreValue = 0;
	T2_GLOBALS._uiElements._active = false;
}

void TsAGE2Globals::synchronize(Serializer &s) {
	Globals::synchronize(s);

	s.syncAsSint16LE(_interfaceY);
}

/*--------------------------------------------------------------------------*/

namespace BlueForce {

BlueForceGlobals::BlueForceGlobals(): TsAGE2Globals() {
	_hiddenDoorStatus = 0;
	_nico910State = 0;
	_stuart910State = 0;
	_nico910Talk = 0;
	_stuart910Talk = 0;
	_deziTopic = 0;
	_deathReason = 0;
	_driveFromScene = 300;
	_driveToScene = 0;
	_subFlagBitArr1 = 0;
	_subFlagBitArr2 = 0;
	_scene410HarrisonTalkFl = false;
	_scene410Action1Count = 0;
	_scene410TalkCount = 0;
	_scene410HarrisonMovedFl = false;
	_bookmark = bNone;
	_mapLocationId = 1;
	_clip1Bullets = 8;
	_clip2Bullets = 8;

	_dayNumber = 0;
	_tonyDialogCtr = 0;
	_marinaWomanCtr = 0;
	_kateDialogCtr = 0;
	_v4CEB6 = 0;
	_safeCombination = 0;
	_gateStatus = 0;
	_greenDay5TalkCtr = 0;
	_v4CEC8 = 1;
	_v4CECA = 0;
	_v4CECC = 0;
}

void BlueForceGlobals::synchronize(Serializer &s) {
	TsAGE2Globals::synchronize(s);
	int16 useless = 0;

	s.syncAsSint16LE(_dayNumber);
	if (s.getVersion() < 9) {
		s.syncAsSint16LE(useless);
	}
	s.syncAsSint16LE(_tonyDialogCtr);
	s.syncAsSint16LE(_marinaWomanCtr);
	s.syncAsSint16LE(_kateDialogCtr);
	s.syncAsSint16LE(_v4CEB6);
	s.syncAsSint16LE(_safeCombination);
	s.syncAsSint16LE(_gateStatus);
	s.syncAsSint16LE(_greenDay5TalkCtr);
	if (s.getVersion() < 11)
		s.syncAsSint16LE(useless);
	s.syncAsSint16LE(_v4CEC8);
	s.syncAsSint16LE(_v4CECA);
	s.syncAsSint16LE(_v4CECC);
	for (int i = 0; i < 18; i++)
		s.syncAsByte(_breakerBoxStatusArr[i]);
	s.syncAsSint16LE(_hiddenDoorStatus);
	s.syncAsSint16LE(_nico910State);
	s.syncAsSint16LE(_stuart910State);
	s.syncAsSint16LE(_nico910Talk);
	s.syncAsSint16LE(_stuart910Talk);
	s.syncAsSint16LE(_deziTopic);
	s.syncAsSint16LE(_deathReason);
	s.syncAsSint16LE(_driveFromScene);
	s.syncAsSint16LE(_driveToScene);
	if (s.getVersion() < 11) {
		s.syncAsSint16LE(useless);
		s.syncAsSint16LE(useless);
		s.syncAsSint16LE(useless);
		s.syncAsSint16LE(useless);
		s.syncAsSint16LE(useless);
	}
	s.syncAsSint16LE(_subFlagBitArr1);
	s.syncAsSint16LE(_subFlagBitArr2);
	s.syncAsSint16LE(_scene410HarrisonTalkFl);
	s.syncAsSint16LE(_scene410Action1Count);
	s.syncAsSint16LE(_scene410TalkCount);
	s.syncAsSint16LE(_scene410HarrisonMovedFl);
	if (s.getVersion() < 11) {
		s.syncAsSint16LE(useless);
		s.syncAsSint16LE(useless);
	}
	s.syncAsSint16LE(_bookmark);
	s.syncAsSint16LE(_mapLocationId);
	s.syncAsSint16LE(_clip1Bullets);
	s.syncAsSint16LE(_clip2Bullets);
}

void BlueForceGlobals::reset() {
	TsAGE2Globals::reset();
	_scenePalette.clearListeners();

	_scrollFollower = &_player;
	_bookmark = bNone;

	// Reset the inventory
	((BlueForceInvObjectList *)_inventory)->reset();

	_mapLocationId = 1;
	_driveFromScene = 300;
	_driveToScene = 0;

	_interfaceY = UI_INTERFACE_Y;
	_dayNumber = 0;
	_tonyDialogCtr = 0;
	_marinaWomanCtr = 0;
	_kateDialogCtr = 0;
	_v4CEB6 = 0;
	_safeCombination = 0;
	_gateStatus = 0;
	_greenDay5TalkCtr = 0;
	_v4CEC8 = 1;
	_v4CECA = 0;
	_v4CECC = 0;
	_breakerBoxStatusArr[0] = 2;
	_breakerBoxStatusArr[1] = 2;
	_breakerBoxStatusArr[2] = 2;
	_breakerBoxStatusArr[3] = 1;
	_breakerBoxStatusArr[4] = 2;
	_breakerBoxStatusArr[5] = 2;
	_breakerBoxStatusArr[6] = 2;
	_breakerBoxStatusArr[7] = 2;
	_breakerBoxStatusArr[8] = 2;
	_breakerBoxStatusArr[9] = 2;
	_breakerBoxStatusArr[10] = 2;
	_breakerBoxStatusArr[11] = 2;
	_breakerBoxStatusArr[12] = 1;
	_breakerBoxStatusArr[13] = 1;
	_breakerBoxStatusArr[14] = 2;
	_breakerBoxStatusArr[15] = 2;
	_breakerBoxStatusArr[16] = 3;
	_breakerBoxStatusArr[17] = 0;
	_hiddenDoorStatus = 0;
	_nico910State = 0;
	_stuart910State = 0;
	_nico910Talk = 0;
	_stuart910Talk = 0;
	_deziTopic = 0;
	_deathReason = 0;
	_subFlagBitArr1 = 0;
	_subFlagBitArr2 = 0;
	_scene410HarrisonTalkFl = false;
	_scene410Action1Count = 0;
	_scene410TalkCount = 0;
	_scene410HarrisonMovedFl = false;
	_clip1Bullets = 8;
	_clip2Bullets = 8;
}

bool BlueForceGlobals::getHasBullets() {
	if (!getFlag(fGunLoaded))
		return false;
	return BF_GLOBALS.getFlag(fLoadedSpare) ? (_clip2Bullets > 0) : (_clip1Bullets > 0);
}

void BlueForceGlobals::set2Flags(int flagNum) {
	if (!getFlag(flagNum + 1)) {
		setFlag(flagNum + 1);
		setFlag(flagNum);
	}
}

bool BlueForceGlobals::removeFlag(int flagNum) {
	bool result = getFlag(flagNum);
	clearFlag(flagNum);
	return result;
}

} // end of namespace BlueForce

namespace Ringworld2 {

Ringworld2Globals::Ringworld2Globals() {
	_scannerDialog = new ScannerDialog();
	_speechSubtitles = SPEECH_TEXT;

	// Register the inner sound objects for each of the global ASoundExt fields.
	// Normally the ASound constructor would do this, but because they're fields
	// of the globals, the g_globals reference isn't ready for them to use
	_sounds.push_back(&_sound1);
	_sounds.push_back(&_sound2);
	_sounds.push_back(&_sound3);
	_sounds.push_back(&_sound4);

	// Initialize fields
	_stripModifier = 0;
	_flubMazeArea = 1;
	_flubMazeEntryDirection = 0;
	_maze3800SceneNumb = 3800;
	_landerSuitNumber = 2;
	_desertStepsRemaining = 5;
	_desertCorrectDirection = 0;
	_desertPreviousDirection = 0;
	_desertWrongDirCtr = -1;
	_balloonAltitude = 5;
	_scene1925CurrLevel = 0;
	_walkwaySceneNumber = 0;
	_mirandaJailState = 0;
	_scientistConvIndex = 0;
	_ductMazePanel1State = 1;
	_ductMazePanel2State = 1;
	_ductMazePanel3State = 1;
	_scene180Mode = -1;
	_v57709 = 0;
	_v5780C = 0;
	_mouseCursorId = 0;
	_v57810 = 0;

	_fadePaletteFlag = false;
	_insetUp = 0;
	_frameEdgeColor = 2;
	_animationCtr = 0;
	_electromagnetChangeAmount = 0;
	_electromagnetZoom = 0;
	_tractorField = false;
	_cableAttached = 0;
	_foodCount = 0;
	_rimLocation = 0;
	_rimTransportLocation = 0;

	_debugCardGame = false;
}

Ringworld2Globals::~Ringworld2Globals() {
	delete _scannerDialog;
}

void Ringworld2Globals::reset() {
	Globals::reset();

	if (!_scannerDialog)
		_scannerDialog = new ScannerDialog();

	// Default to Quinn as the active character
	T2_GLOBALS._player._characterIndex = R2_QUINN;

	// Reset the inventory
	R2_INVENTORY.reset();
	T2_GLOBALS._uiElements.updateInventory();
	T2_GLOBALS._uiElements._active = false;

	// Set the screen to track the player
	_scrollFollower = &_player;

	// Reset fields
	Common::fill(&_fadePaletteMap[0][0], &_fadePaletteMap[9][256], 0);
	Common::fill(&_paletteMap[0], &_paletteMap[4096], 0);

	_fadePaletteFlag = false;
	_animationCtr = 0;
	_electromagnetChangeAmount = 0;
	_electromagnetZoom = 0;
	_tractorField = false;
	_cableAttached = 0;
	_foodCount = 0;
	_rimLocation = 0;
	_rimTransportLocation = 0;
	_stripModifier = 0;
	_spillLocation[0] = 0;
	_spillLocation[1] = 3;
	_spillLocation[R2_SEEKER] = 5;
	_spillLocation[3] = 1;
	_spillLocation[4] = 2;
	_spillLocation[5] = 5;
	_spillLocation[6] = 9;
	_spillLocation[7] = 14;
	_spillLocation[8] = 15;
	_spillLocation[9] = 18;
	_spillLocation[10] = 20;
	_spillLocation[11] = 25;
	_spillLocation[12] = 27;
	_spillLocation[13] = 31;

	// Initialize the vampire data within the Flub maze
	for (int i = 0; i < 18; i++) {
		_vampireData[i]._isAlive = true;
		_vampireData[i]._position = Common::Point();
	}
	_vampireData[0]._shotsRequired = 1;
	_vampireData[1]._shotsRequired = 2;
	_vampireData[2]._shotsRequired = 2;
	_vampireData[3]._shotsRequired = 3;
	_vampireData[4]._shotsRequired = 2;
	_vampireData[5]._shotsRequired = 2;
	_vampireData[6]._shotsRequired = 3;
	_vampireData[7]._shotsRequired = 1;
	_vampireData[8]._shotsRequired = 1;
	_vampireData[9]._shotsRequired = 3;
	_vampireData[10]._shotsRequired = 3;
	_vampireData[11]._shotsRequired = 1;
	_vampireData[12]._shotsRequired = 2;
	_vampireData[13]._shotsRequired = 3;
	_vampireData[14]._shotsRequired = 2;
	_vampireData[15]._shotsRequired = 3;
	_vampireData[16]._shotsRequired = 1;
	_vampireData[17]._shotsRequired = 1;

	_maze3800SceneNumb = 3800;
	_landerSuitNumber = 2;
	_flubMazeArea = 1;
	_flubMazeEntryDirection = 0;
	_desertStepsRemaining = 5;
	_desertCorrectDirection = 0;
	_desertPreviousDirection = 0;
	for (int i = 0; i < 1000; i++)
		_desertMovements[i] = 0;
	_desertWrongDirCtr = -1;
	_balloonAltitude = 5;
	_scene1925CurrLevel = 0; //_v56A9C
	_walkwaySceneNumber = 0;
	_mirandaJailState = 0;
	_scientistConvIndex = 0;
	_ventCellPos = Common::Point(60, 660);
	_ductMazePanel1State = 1;
	_ductMazePanel2State = 1;
	_ductMazePanel3State = 1;
	_scene180Mode = -1;
	_v57709 = 0;
	_v5780C = 0;
	_mouseCursorId = 0;
	_v57810 = 0;
	_s1550PlayerArea[R2_QUINN] = Common::Point(27, 4);
	_s1550PlayerArea[R2_SEEKER] = Common::Point(27, 4);
	Common::fill(&_scannerFrequencies[0], &_scannerFrequencies[MAX_CHARACTERS], 1);
	_insetUp = 0;
	_frameEdgeColor = 2;
	Common::fill(&_stripManager_lookupList[0], &_stripManager_lookupList[12], 0);
	_stripManager_lookupList[0] = 1;
	_stripManager_lookupList[1] = 1;
	_stripManager_lookupList[2] = 1;
	_stripManager_lookupList[3] = 1;
	_stripManager_lookupList[4] = 1;
	_stripManager_lookupList[5] = 1;
	_stripManager_lookupList[8] = 1;
	_stripManager_lookupList[9] = 1;
	_stripManager_lookupList[10] = 1;
	_stripManager_lookupList[11] = 1;

	// Reset junk/component data in scene 1550
	Common::copy(&scene1550JunkLocationsDefault[0], &scene1550JunkLocationsDefault[508],
		&_scene1550JunkLocations[0]);

	// Reset fields stored in the player class
	_player._characterIndex = R2_QUINN;
	_player._characterScene[R2_QUINN] = 100;
	_player._characterScene[R2_SEEKER] = 300;
	_player._characterScene[R2_MIRANDA] = 300;

	_debugCardGame = false;
}

void Ringworld2Globals::synchronize(Serializer &s) {
	TsAGE2Globals::synchronize(s);
	int i;

	s.syncAsSint16LE(_animationCtr);
	s.syncAsSint16LE(_electromagnetChangeAmount);
	s.syncAsSint16LE(_electromagnetZoom);
	s.syncAsSint16LE(_tractorField);
	s.syncAsSint16LE(_cableAttached);
	s.syncAsSint16LE(_foodCount);
	s.syncAsSint32LE(_rimLocation);
	s.syncAsSint16LE(_rimTransportLocation);
	s.syncAsSint16LE(_landerSuitNumber);
	s.syncAsSint16LE(_maze3800SceneNumb);
	s.syncAsSint16LE(_desertWrongDirCtr);
	s.syncAsSint16LE(_scene1925CurrLevel); // _v56A9C
	s.syncAsSint16LE(_walkwaySceneNumber);
	s.syncAsSint16LE(_ventCellPos.x);
	s.syncAsSint16LE(_ventCellPos.y);
	s.syncAsSint16LE(_scene180Mode);
	s.syncAsSint16LE(_v57709);
	s.syncAsSint16LE(_v5780C);
	s.syncAsSint16LE(_mouseCursorId);
	s.syncAsSint16LE(_v57810);

	s.syncAsByte(_s1550PlayerArea[R2_QUINN].x);
	s.syncAsByte(_s1550PlayerArea[R2_SEEKER].x);
	s.syncAsByte(_s1550PlayerArea[R2_QUINN].y);
	s.syncAsByte(_s1550PlayerArea[R2_SEEKER].y);

	for (i = 0; i < MAX_CHARACTERS; ++i)
		s.syncAsByte(_scannerFrequencies[i]);

	s.syncAsByte(_stripModifier);
	s.syncAsByte(_flubMazeArea);
	s.syncAsByte(_flubMazeEntryDirection);
	s.syncAsByte(_desertStepsRemaining);
	s.syncAsByte(_desertCorrectDirection);
	s.syncAsByte(_desertPreviousDirection);
	s.syncAsByte(_mirandaJailState);
	s.syncAsByte(_scientistConvIndex);
	s.syncAsByte(_ductMazePanel1State);
	s.syncAsByte(_ductMazePanel2State);
	s.syncAsByte(_ductMazePanel3State);

	for (i = 0; i < 14; ++i)
		s.syncAsByte(_spillLocation[i]);
	for (i = 0; i < 1000; ++i)
		s.syncAsByte(_desertMovements[i]);
	s.syncAsByte(_balloonAltitude);
	for (i = 0; i < 12; ++i)
		s.syncAsByte(_stripManager_lookupList[i]);

	s.syncAsSint16LE(_insetUp);
	s.syncAsByte(_frameEdgeColor);

	for (i = 0; i < 508; i += 4)
		s.syncAsByte(_scene1550JunkLocations[i + 2]);

	s.syncAsSint16LE(_balloonPosition.x);
	s.syncAsSint16LE(_balloonPosition.y);

	// Synchronize Flub maze vampire data
	for (i = 0; i < 18; ++i) {
		s.syncAsSint16LE(_vampireData[i]._isAlive);
		s.syncAsSint16LE(_vampireData[i]._shotsRequired);
		s.syncAsSint16LE(_vampireData[i]._position.x);
		s.syncAsSint16LE(_vampireData[i]._position.y);
	}
}

} // end of namespace Ringworld2

} // end of namespace TsAGE
