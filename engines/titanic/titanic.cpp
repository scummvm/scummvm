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
	static const char *const NAMES[46] = {
		"LeftArmWith", "LeftArmWithout", "RightArmWith", "RightArmWithout", "BridgeRed",
		"BridgeYellow", "BridgeBlue", "BridgeGreen", "Parrot", "CentralCore", "BrainGreen",
		"BrainYellow", "BrainRed", "BrainBlue", "ChickenGreasy", "ChickenPlain", "ChickenPurple",
		"ChickenRed", "ChickenYellow", "CrushedTV", "Ear", "Ear1", "Eyeball", "Eyeball1",
		"Feather", "Lemon", "GlassEmpty", "GlassPurple", "GlassRed", "GlassYellow", "Hammer",
		"Hose", "HoseEnd", "LiftHead", "LongStick", "Magazine", "Mouth", "MusicKey", "Napkin",
		"Nose", "Perch", "PhonoCylinder", "PhonoCylinder1", "PhonoCylinder2", "PhonoCylinder3",
		"Photo"
	};
	for (uint idx = 0; idx < 46; ++idx)
		_itemNames[idx] = NAMES[idx];

	// Item descriptions
	static const char *const DESCRIPTIONS[46] = {
		"The Maitre d'Bot's left arm holding a key", "The Maitre d'Bot's left arm",
		"The Maitre d'Bot's right arm holding Titania's auditory center",
		"The Maitre d'Bot's right arm", "Red Fuse", "Yellow Fuse", "Blue Fuse",
		"Green Fuse", "The Parrot", "Titania's central intelligence core",
		"Titania's auditory center", "Titania's olfactory center",
		"Titania's speech center", "Titania's vision center", "rather greasy chicken",
		"very plain chicken", "chicken smeared with starling pur$e",
		"chicken covered with tomato sauce", "chicken coated in mustard sauce",
		"A crushed television set", "Titania's ear", "Titania's ear", "Titania's eye",
		"Titania's eye", "A parrot feather", "A nice fat juicy lemon",
		"An empty beer glass", "A beer glass containing pur$ed flock of starlings",
		"A beer glass containing tomato sauce", "A beer glass containing mustard sauce",
		"A hammer", "A hose", "The other end of a hose", "The LiftBot's head",
		"A rather long stick", "A magazine", "Titania's mouth", "A key",
		"A super-absorbent napkin", "Titania's nose", "A perch", "A phonograph cylinder",
		"A phonograph cylinder", "A phonograph cylinder", "A phonograph cylinder",
		"A photograph"
	};
	for (uint idx = 0; idx < 46; ++idx)
		_itemDescriptions[idx] = DESCRIPTIONS[idx];

	// Short descriptions.. maybe?
	static const char *const SHORT_DESC[40] = {
		"MaitreD Left Arm", "MaitreD Right Arm", "OlfactoryCentre", "AuditoryCentre",
		"SpeechCentre", "VisionCentre", "CentralCore", "Perch", "SeasonBridge",
		"FanBridge", "BeamBridge", "ChickenBridge", "CarryParrot", "Chicken",
		"CrushedTV", "Feathers", "Lemon", "BeerGlass", "BigHammer", "Ear1", "Ear 2",
		"Eye1", "Eye2", "Mouth", "Nose", "NoseSpare", "Hose", "DeadHoseSpare",
		"HoseEnd", "DeadHoseEndSpare", "BrokenLiftbotHead", "LongStick", "Magazine",
		"Napkin", "Phonograph Cylinder", "Phonograph Cylinder 1", "Phonograph Cylinder 2",
		"Phonograph Cylinder 3", "Photograph", "Music System Key"
	};
	for (uint idx = 0; idx < 40; ++idx)
		_itemShortDesc[idx] = SHORT_DESC[idx];
}


} // End of namespace Titanic
