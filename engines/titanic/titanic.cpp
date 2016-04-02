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

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "titanic/titanic.h"
#include "titanic/debugger.h"
#include "titanic/carry/hose.h"
#include "titanic/core/saveable_object.h"
#include "titanic/game/get_lift_eye2.h"
#include "titanic/game/television.h"
#include "titanic/game/parrot/parrot_lobby_object.h"
#include "titanic/game/sgt/sgt_navigation.h"
#include "titanic/game/sgt/sgt_state_room.h"
#include "titanic/moves/enter_exit_first_class_state.h"
#include "titanic/moves/enter_exit_sec_class_mini_lift.h"
#include "titanic/moves/exit_pellerator.h"

namespace Titanic {

TitanicEngine *g_vm;

TitanicEngine::TitanicEngine(OSystem *syst, const TitanicGameDescription *gameDesc)
		: _gameDescription(gameDesc), Engine(syst), _randomSource("Titanic") {
	g_vm = this;
	_debugger = nullptr;
	_events = nullptr;
	_window = nullptr;
	_screen = nullptr;
	_screenManager = nullptr;
}

TitanicEngine::~TitanicEngine() {
	delete _debugger;
	delete _events;
	delete _screen;
	delete _window;
	delete _screenManager;
	CSaveableObject::freeClassList();
}

void TitanicEngine::initializePath(const Common::FSNode &gamePath) {
	Engine::initializePath(gamePath);
	SearchMan.addSubDirectoryMatching(gamePath, "assets");
}

void TitanicEngine::initialize() {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugCore, "core", "Core engine debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound and Music handling");

	setItemNames();
	CSaveableObject::initClassList();
	CEnterExitFirstClassState::init();
	CGetLiftEye2::init();
	CHose::init();
	CParrotLobbyObject::init();
	CSGTNavigation::init();
	CSGTStateRoom::init();
	CExitPellerator::init();
	CEnterExitSecClassMiniLift::init();
	CTelevision::init();

	_debugger = new Debugger(this);
	_events = new Events(this);
	_screen = new Graphics::Screen(0, 0);
	_screenManager = new OSScreenManager(this);
	_window = new CMainGameWindow(this);
	_window->applicationStarting();
}

void TitanicEngine::deinitialize() {
	CEnterExitFirstClassState::deinit();
	CGetLiftEye2::deinit();
	CHose::deinit();
	CSGTNavigation::deinit();
	CSGTStateRoom::deinit();
	CExitPellerator::deinit();
	CEnterExitSecClassMiniLift::deinit();
	CTelevision::deinit();
}

Common::Error TitanicEngine::run() {
	initialize();

	// Main event loop
	while (!shouldQuit()) {
		_events->pollEventsAndWait();
	}

	deinitialize();
	return Common::kNoError;
}

void TitanicEngine::setItemNames() {
	// Names
	_itemNames[0] = "LeftArmWith";
	_itemNames[1] = "LeftArmWithout";
	_itemNames[2] = "RightArmWith";
	_itemNames[3] = "RightArmWithout";
	_itemNames[4] = "BridgeRed";
	_itemNames[5] = "BridgeYellow";
	_itemNames[6] = "BridgeBlue";
	_itemNames[7] = "BridgeGreen";
	_itemNames[8] = "Parrot";
	_itemNames[9] = "CentralCore";
	_itemNames[10] = "BrainGreen";
	_itemNames[11] = "BrainYellow";
	_itemNames[12] = "BrainRed";
	_itemNames[13] = "BrainBlue";
	_itemNames[14] = "ChickenGreasy";
	_itemNames[15] = "ChickenPlain";
	_itemNames[16] = "ChickenPurple";
	_itemNames[17] = "ChickenRed";
	_itemNames[18] = "ChickenYellow";
	_itemNames[19] = "CrushedTV";
	_itemNames[20] = "Ear";
	_itemNames[21] = "Ear1";
	_itemNames[22] = "Eyeball";
	_itemNames[23] = "Eyeball1";
	_itemNames[24] = "Feather";
	_itemNames[25] = "Lemon";
	_itemNames[26] = "GlassEmpty";
	_itemNames[27] = "GlassPurple";
	_itemNames[28] = "GlassRed";
	_itemNames[29] = "GlassYellow";
	_itemNames[30] = "Hammer";
	_itemNames[31] = "Hose";
	_itemNames[32] = "HoseEnd";
	_itemNames[33] = "LiftHead";
	_itemNames[34] = "LongStick";
	_itemNames[35] = "Magazine";
	_itemNames[36] = "Mouth";
	_itemNames[37] = "MusicKey";
	_itemNames[38] = "Napkin";
	_itemNames[39] = "Nose";
	_itemNames[40] = "Perch";
	_itemNames[41] = "PhonoCylinder";
	_itemNames[42] = "PhonoCylinder1";
	_itemNames[43] = "PhonoCylinder2";
	_itemNames[44] = "PhonoCylinder3";
	_itemNames[45] = "Photo";

	// Item long descriptions
	_itemDescriptions[0] = "The Maitre d'Bot's left arm holding a key";
	_itemDescriptions[1] = "The Maitre d'Bot's left arm";
	_itemDescriptions[2] = "The Maitre d'Bot's right arm holding Titania's auditory center";
	_itemDescriptions[3] = "The Maitre d'Bot's right arm";
	_itemDescriptions[4] = "Red Fuse";
	_itemDescriptions[5] = "Yellow Fuse";
	_itemDescriptions[6] = "Blue Fuse";
	_itemDescriptions[7] = "Green Fuse";
	_itemDescriptions[8] = "The Parrot";
	_itemDescriptions[9] = "Titania's central intelligence core";
	_itemDescriptions[10] = "Titania's auditory center";
	_itemDescriptions[11] = "Titania's olfactory center";
	_itemDescriptions[12] = "Titania's speech center";
	_itemDescriptions[13] = "Titania's vision center";
	_itemDescriptions[14] = "rather greasy chicken";
	_itemDescriptions[15] = "very plain chicken";
	_itemDescriptions[16] = "chicken smeared with starling pur$e";
	_itemDescriptions[17] = "chicken covered with tomato sauce";
	_itemDescriptions[18] = "chicken coated in mustard sauce";
	_itemDescriptions[19] = "A crushed television set";
	_itemDescriptions[20] = "Titania's ear";
	_itemDescriptions[21] = "Titania's ear";
	_itemDescriptions[22] = "Titania's eye";
	_itemDescriptions[23] = "Titania's eye";
	_itemDescriptions[24] = "A parrot feather";
	_itemDescriptions[25] = "A nice fat juicy lemon";
	_itemDescriptions[26] = "An empty beer glass";
	_itemDescriptions[27] = "A beer glass containing pur$ed flock of starlings";
	_itemDescriptions[28] = "A beer glass containing tomato sauce";
	_itemDescriptions[29] = "A beer glass containing mustard sauce";
	_itemDescriptions[30] = "A hammer";
	_itemDescriptions[31] = "A hose";
	_itemDescriptions[32] = "The other end of a hose";
	_itemDescriptions[33] = "The LiftBot's head";
	_itemDescriptions[34] = "A rather long stick";
	_itemDescriptions[35] = "A magazine";
	_itemDescriptions[36] = "Titania's mouth";
	_itemDescriptions[37] = "A key";
	_itemDescriptions[38] = "A super-absorbent napkin";
	_itemDescriptions[39] = "Titania's nose";
	_itemDescriptions[40] = "A perch";
	_itemDescriptions[41] = "A phonograph cylinder";
	_itemDescriptions[42] = "A phonograph cylinder";
	_itemDescriptions[43] = "A phonograph cylinder";
	_itemDescriptions[44] = "A phonograph cylinder";
	_itemDescriptions[45] = "A photograph";

	// Short descriptions.. maybe?
	_itemShortDesc[0] = "MaitreD Left Arm";
	_itemShortDesc[1] = "MaitreD Right Arm";
	_itemShortDesc[2] = "OlfactoryCentre";
	_itemShortDesc[3] = "AuditoryCentre";
	_itemShortDesc[4] = "SpeechCentre";
	_itemShortDesc[5] = "VisionCentre";
	_itemShortDesc[6] = "CentralCore";
	_itemShortDesc[7] = "Perch";
	_itemShortDesc[8] = "SeasonBridge";
	_itemShortDesc[9] = "FanBridge";
	_itemShortDesc[10] = "BeamBridge";
	_itemShortDesc[11] = "ChickenBridge";
	_itemShortDesc[12] = "CarryParrot";
	_itemShortDesc[13] = "Chicken";
	_itemShortDesc[14] = "CrushedTV";
	_itemShortDesc[15] = "Feathers";
	_itemShortDesc[16] = "Lemon";
	_itemShortDesc[17] = "BeerGlass";
	_itemShortDesc[18] = "BigHammer";
	_itemShortDesc[19] = "Ear1";
	_itemShortDesc[20] = "Ear 2";
	_itemShortDesc[21] = "Eye1";
	_itemShortDesc[22] = "Eye2";
	_itemShortDesc[23] = "Mouth";
	_itemShortDesc[24] = "Nose";
	_itemShortDesc[25] = "NoseSpare";
	_itemShortDesc[26] = "Hose";
	_itemShortDesc[27] = "DeadHoseSpare";
	_itemShortDesc[28] = "HoseEnd";
	_itemShortDesc[29] = "DeadHoseEndSpare";
	_itemShortDesc[30] = "BrokenLiftbotHead";
	_itemShortDesc[31] = "LongStick";
	_itemShortDesc[32] = "Magazine";
	_itemShortDesc[33] = "Napkin";
	_itemShortDesc[34] = "Phonograph Cylinder";
	_itemShortDesc[35] = "Phonograph Cylinder 1";
	_itemShortDesc[36] = "Phonograph Cylinder 2";
	_itemShortDesc[37] = "Phonograph Cylinder 3";
	_itemShortDesc[38] = "Photograph";
	_itemShortDesc[39] = "Music System Key";
}


} // End of namespace Titanic
