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
#include "common/file.h"
#include "common/tokenizer.h"
#include "image/png.h"
#include "image/bmp.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/inverter_process.h"
#include "ultima/ultima8/gfx/main_shape_archive.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/texture.h"
#include "ultima/ultima8/gumps/fast_area_vis_gump.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/gumps/minimap_gump.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/gumps/shape_viewer_gump.h"
#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/misc/id_man.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/bit_set.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/actors/pathfinder.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/item_selection_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"

namespace Ultima {
namespace Ultima8 {

Debugger *g_debugger;

Debugger::Debugger() : GUI::Debugger() {
	g_debugger = this;

	// WARNING: Not only can the methods below be executed directly in the debugger,
	// they also act as the methods keybindings are made to. So be wary of changing names
	registerCmd("quit", WRAP_METHOD(Debugger, cmdQuit));
	registerCmd("Ultima8Engine::quit", WRAP_METHOD(Debugger, cmdQuit));
	registerCmd("Ultima8Engine::saveGame", WRAP_METHOD(Debugger, cmdSaveGame));
	registerCmd("Ultima8Engine::loadGame", WRAP_METHOD(Debugger, cmdLoadGame));
	registerCmd("Ultima8Engine::newGame", WRAP_METHOD(Debugger, cmdNewGame));
	registerCmd("Ultima8Engine::engineStats", WRAP_METHOD(Debugger, cmdEngineStats));
	registerCmd("Ultima8Engine::setVideoMode", WRAP_METHOD(Debugger, cmdSetVideoMode));
	registerCmd("Ultima8Engine::toggleAvatarInStasis", WRAP_METHOD(Debugger, cmdToggleAvatarInStasis));
	registerCmd("Ultima8Engine::togglePaintEditorItems", WRAP_METHOD(Debugger, cmdTogglePaintEditorItems));
	registerCmd("Ultima8Engine::toggleShowTouchingItems", WRAP_METHOD(Debugger, cmdToggleShowTouchingItems));
	registerCmd("Ultima8Engine::closeItemGumps", WRAP_METHOD(Debugger, cmdCloseItemGumps));

	registerCmd("CameraProcess::moveToAvatar", WRAP_METHOD(Debugger, cmdCameraOnAvatar));

	registerCmd("AudioProcess::listSFX", WRAP_METHOD(Debugger, cmdListSFX));
	registerCmd("AudioProcess::playSFX", WRAP_METHOD(Debugger, cmdPlaySFX));
	registerCmd("AudioProcess::stopSFX", WRAP_METHOD(Debugger, cmdStopSFX));

	registerCmd("Cheat::toggle", WRAP_METHOD(Debugger, cmdToggleCheatMode));
	registerCmd("Cheat::maxstats", WRAP_METHOD(Debugger, cmdMaxStats));
	registerCmd("Cheat::heal", WRAP_METHOD(Debugger, cmdHeal));
	registerCmd("Cheat::toggleInvincibility", WRAP_METHOD(Debugger, cmdToggleInvincibility));
	registerCmd("Cheat::items", WRAP_METHOD(Debugger, cmdCheatItems));
	registerCmd("Cheat::equip", WRAP_METHOD(Debugger, cmdCheatEquip));

	registerCmd("GameMapGump::startHighlightItems", WRAP_METHOD(Debugger, cmdStartHighlightItems));
	registerCmd("GameMapGump::stopHighlightItems", WRAP_METHOD(Debugger, cmdStopHighlightItems));
	registerCmd("GameMapGump::toggleHighlightItems", WRAP_METHOD(Debugger, cmdToggleHighlightItems));
	registerCmd("GameMapGump::toggleFootpads", WRAP_METHOD(Debugger, cmdToggleFootpads));
	registerCmd("GameMapGump::dumpMap", WRAP_METHOD(Debugger, cmdDumpMap));
	registerCmd("GameMapGump::dumpAllMaps", WRAP_METHOD(Debugger, cmdDumpAllMaps));
	registerCmd("GameMapGump::incrementSortOrder", WRAP_METHOD(Debugger, cmdIncrementSortOrder));
	registerCmd("GameMapGump::decrementSortOrder", WRAP_METHOD(Debugger, cmdDecrementSortOrder));

	registerCmd("Kernel::processTypes", WRAP_METHOD(Debugger, cmdProcessTypes));
	registerCmd("Kernel::processInfo", WRAP_METHOD(Debugger, cmdProcessInfo));
	registerCmd("Kernel::listProcesses", WRAP_METHOD(Debugger, cmdListProcesses));
	registerCmd("Kernel::toggleFrameByFrame", WRAP_METHOD(Debugger, cmdToggleFrameByFrame));
	registerCmd("Kernel::advanceFrame", WRAP_METHOD(Debugger, cmdAdvanceFrame));

	registerCmd("MainActor::teleport", WRAP_METHOD(Debugger, cmdTeleport));
	registerCmd("MainActor::mark", WRAP_METHOD(Debugger, cmdMark));
	registerCmd("MainActor::recall", WRAP_METHOD(Debugger, cmdRecall));
	registerCmd("MainActor::listmarks", WRAP_METHOD(Debugger, cmdListMarks));
	registerCmd("MainActor::name", WRAP_METHOD(Debugger, cmdName));
	registerCmd("MainActor::useBackpack", WRAP_METHOD(Debugger, cmdUseBackpack));
	registerCmd("MainActor::useInventory", WRAP_METHOD(Debugger, cmdUseInventory));
	registerCmd("MainActor::useRecall", WRAP_METHOD(Debugger, cmdUseRecall));
	registerCmd("MainActor::useBedroll", WRAP_METHOD(Debugger, cmdUseBedroll));
	registerCmd("MainActor::useKeyring", WRAP_METHOD(Debugger, cmdUseKeyring));
	registerCmd("MainActor::nextWeapon", WRAP_METHOD(Debugger, cmdNextWeapon));
	registerCmd("MainActor::nextInvItem", WRAP_METHOD(Debugger, cmdNextInventory));
	registerCmd("MainActor::useInventoryItem", WRAP_METHOD(Debugger, cmdUseInventoryItem));
	registerCmd("MainActor::useMedikit", WRAP_METHOD(Debugger, cmdUseMedikit));
	registerCmd("MainActor::useEnergyCube", WRAP_METHOD(Debugger, cmdUseEnergyCube));
	registerCmd("MainActor::detonateBomb", WRAP_METHOD(Debugger, cmdDetonateBomb));
	registerCmd("MainActor::dropWeapon", WRAP_METHOD(Debugger, cmdDropWeapon));
	registerCmd("MainActor::toggleCombat", WRAP_METHOD(Debugger, cmdToggleCombat));
	registerCmd("ItemSelectionProcess::startSelection", WRAP_METHOD(Debugger, cmdStartSelection));
	registerCmd("ItemSelectionProcess::useSelectedItem", WRAP_METHOD(Debugger, cmdUseSelection));
	registerCmd("ItemSelectionProcess::grabItems", WRAP_METHOD(Debugger, cmdGrabItems));

	registerCmd("ObjectManager::objectTypes", WRAP_METHOD(Debugger, cmdObjectTypes));
	registerCmd("ObjectManager::objectInfo", WRAP_METHOD(Debugger, cmdObjectInfo));

	registerCmd("QuickAvatarMoverProcess::startMoveUp", WRAP_METHOD(Debugger, cmdStartQuickMoveUp));
	registerCmd("QuickAvatarMoverProcess::startMoveDown", WRAP_METHOD(Debugger, cmdStartQuickMoveDown));
	registerCmd("QuickAvatarMoverProcess::startMoveLeft", WRAP_METHOD(Debugger, cmdStartQuickMoveLeft));
	registerCmd("QuickAvatarMoverProcess::startMoveRight", WRAP_METHOD(Debugger, cmdStartQuickMoveRight));
	registerCmd("QuickAvatarMoverProcess::startAscend", WRAP_METHOD(Debugger, cmdStartQuickMoveAscend));
	registerCmd("QuickAvatarMoverProcess::startDescend", WRAP_METHOD(Debugger, cmdStartQuickMoveDescend));
	registerCmd("QuickAvatarMoverProcess::stopMoveUp", WRAP_METHOD(Debugger, cmdStopQuickMoveUp));
	registerCmd("QuickAvatarMoverProcess::stopMoveDown", WRAP_METHOD(Debugger, cmdStopQuickMoveDown));
	registerCmd("QuickAvatarMoverProcess::stopMoveLeft", WRAP_METHOD(Debugger, cmdStopQuickMoveLeft));
	registerCmd("QuickAvatarMoverProcess::stopMoveRight", WRAP_METHOD(Debugger, cmdStopQuickMoveRight));
	registerCmd("QuickAvatarMoverProcess::stopAscend", WRAP_METHOD(Debugger, cmdStopQuickMoveAscend));
	registerCmd("QuickAvatarMoverProcess::stopDescend", WRAP_METHOD(Debugger, cmdStopQuickMoveDescend));
	registerCmd("QuickAvatarMoverProcess::toggleQuarterSpeed", WRAP_METHOD(Debugger, cmdToggleQuarterSpeed));
	registerCmd("QuickAvatarMoverProcess::toggleClipping", WRAP_METHOD(Debugger, cmdToggleClipping));

	registerCmd("UCMachine::getGlobal", WRAP_METHOD(Debugger, cmdGetGlobal));
	registerCmd("UCMachine::setGlobal", WRAP_METHOD(Debugger, cmdSetGlobal));
	registerCmd("UCMachine::traceObjID", WRAP_METHOD(Debugger, cmdTraceObjID));
	registerCmd("UCMachine::tracePID", WRAP_METHOD(Debugger, cmdTracePID));
	registerCmd("UCMachine::traceClass", WRAP_METHOD(Debugger, cmdTraceClass));
	registerCmd("UCMachine::traceAll", WRAP_METHOD(Debugger, cmdTraceAll));
	registerCmd("UCMachine::stopTrace", WRAP_METHOD(Debugger, cmdStopTrace));

	registerCmd("FastAreaVisGump::toggle", WRAP_METHOD(Debugger, cmdToggleFastArea));
	registerCmd("InverterProcess::invertScreen", WRAP_METHOD(Debugger, cmdInvertScreen));
	registerCmd("MenuGump::showMenu", WRAP_METHOD(Debugger, cmdShowMenu));
	registerCmd("MiniMapGump::toggle", WRAP_METHOD(Debugger, cmdToggleMinimap));
	registerCmd("MiniMapGump::generate", WRAP_METHOD(Debugger, cmdGenerateMinimap));
	registerCmd("MiniMapGump::clear", WRAP_METHOD(Debugger, cmdClearMinimap));
	registerCmd("MovieGump::play", WRAP_METHOD(Debugger, cmdPlayMovie));
	registerCmd("MusicProcess::playMusic", WRAP_METHOD(Debugger, cmdPlayMusic));
	registerCmd("QuitGump::verifyQuit", WRAP_METHOD(Debugger, cmdVerifyQuit));
	registerCmd("ShapeViewerGump::U8ShapeViewer", WRAP_METHOD(Debugger, cmdU8ShapeViewer));
	registerCmd("RenderSurface::benchmark", WRAP_METHOD(Debugger, cmdBenchmarkRenderSurface));

#ifdef DEBUG_PATHFINDER
	registerCmd("Pathfinder::visualDebug", WRAP_METHOD(Debugger, cmdVisualDebugPathfinder));
#endif
}

Debugger::~Debugger() {
	g_debugger = nullptr;
}


void Debugger::executeCommand(const Common::String &args) {
	Common::Array<Common::String> argv;
	StringToArgv(args, argv);

	executeCommand(argv);
}

void Debugger::executeCommand(const Common::Array<Common::String> &argv) {
	if (argv.empty())
		return;

	Common::String commandName = argv[0];
	if (commandName.hasPrefix("GUIApp::"))
		commandName = "Ultima8Engine::" + Common::String(commandName.c_str() + 8);

	Common::Array<const char *> cArgs;
	cArgs.push_back(commandName.c_str());
	for (uint idx = 1; idx < argv.size(); ++idx)
		cArgs.push_back(argv[idx].c_str());

	bool keepRunning = false;
	if (!handleCommand(argv.size(), &cArgs[0], keepRunning)) {
		debugPrintf("Unknown command - %s\n", commandName.c_str());
		keepRunning = true;
	}

	// If any message occurred, then we need to ensure the debugger is opened if it isn't already
	if (keepRunning)
		attach();
}


bool Debugger::cmdSaveGame(int argc, const char **argv) {
	if (argc == 2) {
		if (!Ultima8Engine::get_instance()->canSaveGameStateCurrently()) {
			debugPrintf("Saving game is currently unavailable\n");
			return true;
		}

		// Save a _game with the given name into the quicksave slot
		Common::Error result = Ultima8Engine::get_instance()->saveGameState(1, argv[1]);
		if (result.getCode() != Common::kNoError) {
			debugPrintf("Saving game failed: %s\n", result.getDesc().c_str());
			return true;
		}
	} else {
		Ultima8Engine::get_instance()->saveGameDialog();
	}

	return false;
}

bool Debugger::cmdLoadGame(int argc, const char **argv) {
	if (argc == 2) {
		// Load a _game from the quicksave slot. The second parameter is ignored,
		// it just needs to be present to differentiate from showing the GUI load dialog
		Ultima8Engine::get_instance()->loadGameState(1);
	} else {
		Ultima8Engine::get_instance()->loadGameDialog();
	}

	return false;
}

bool Debugger::cmdNewGame(int argc, const char **argv) {
	Ultima8Engine::get_instance()->newGame();
	return false;
}

bool Debugger::cmdQuit(int argc, const char **argv) {
	Ultima8Engine::get_instance()->_isRunning = false;
	return false;
}

bool Debugger::cmdEngineStats(int argc, const char **argv) {
	Kernel::get_instance()->kernelStats();
	ObjectManager::get_instance()->objectStats();
	UCMachine::get_instance()->usecodeStats();
	World::get_instance()->worldStats();


	return true;
}

bool Debugger::cmdSetVideoMode(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Usage: Ultima8Engine::setVidMode width height\n");
		return true;
	} else {
		Ultima8Engine::get_instance()->changeVideoMode(strtol(argv[1], 0, 0), strtol(argv[2], 0, 0));
		return false;
	}
}

bool Debugger::cmdToggleAvatarInStasis(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->toggleAvatarInStasis();
	debugPrintf("avatarInStasis = %s\n", strBool(g->isAvatarInStasis()));
	return true;
}

bool Debugger::cmdTogglePaintEditorItems(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->togglePaintEditorItems();
	debugPrintf("paintEditorItems = %s\n", strBool(g->isPaintEditorItems()));
	return false;
}

bool Debugger::cmdToggleShowTouchingItems(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->toggleShowTouchingItems();
	debugPrintf("ShowTouchingItems = %s\n", strBool(g->isShowTouchingItems()));
	return false;
}

bool Debugger::cmdCloseItemGumps(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->getDesktopGump()->CloseItemDependents();
	return false;
}

bool Debugger::cmdListSFX(int argc, const char **argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		debugPrintf("Error: No AudioProcess\n");

	} else {
		Std::list<AudioProcess::SampleInfo>::const_iterator it;
		for (it = ap->_sampleInfo.begin(); it != ap->_sampleInfo.end(); ++it) {
			debugPrintf("Sample: num %d, obj %d, loop %d, prio %d",
				it->_sfxNum, it->_objId, it->_loops, it->_priority);
			if (!it->_barked.empty()) {
				debugPrintf(", speech: \"%s\"",
					it->_barked.substr(it->_curSpeechStart, it->_curSpeechEnd - it->_curSpeechStart).c_str());
			}
			debugPrintf("\n");
		}
	}

	return true;
}

bool Debugger::cmdStopSFX(int argc, const char **argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		debugPrintf("Error: No AudioProcess\n");
		return true;
	} else if (argc < 2) {
		debugPrintf("usage: stopSFX <_sfxNum> [<_objId>]\n");
		return true;
	} else {
		int sfxNum = static_cast<int>(strtol(argv[1], 0, 0));
		ObjId objId = (argc >= 3) ? static_cast<ObjId>(strtol(argv[2], 0, 0)) : 0;

		ap->stopSFX(sfxNum, objId);
		return false;
	}
}

bool Debugger::cmdPlaySFX(int argc, const char **argv) {
	AudioProcess *ap = AudioProcess::get_instance();
	if (!ap) {
		debugPrintf("Error: No AudioProcess\n");
		return true;
	} else if (argc < 2) {
		debugPrintf("usage: playSFX <_sfxNum>\n");
		return true;
	} else {
		int sfxNum = static_cast<int>(strtol(argv[1], 0, 0));
		ap->playSFX(sfxNum, 0x60, 0, 0);
		return false;
	}
}


bool Debugger::cmdToggleCheatMode(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->setCheatMode(!g->areCheatsEnabled());
	debugPrintf("Cheats = %s\n", strBool(g->areCheatsEnabled()));
	return true;
}

bool Debugger::cmdMaxStats(int argc, const char **argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		debugPrintf("Cheats are disabled\n");
		return true;
	}
	MainActor *mainActor = getMainActor();

	if (GAME_IS_CRUSADER) {
		mainActor->setHP(mainActor->getMaxHP());
		mainActor->setMana(mainActor->getMaxMana());
	} else {
		// constants!!
		mainActor->setStr(25);
		mainActor->setDex(25);
		mainActor->setInt(25);
		mainActor->setHP(mainActor->getMaxHP());
		mainActor->setMana(mainActor->getMaxMana());

		AudioProcess *audioproc = AudioProcess::get_instance();
		if (audioproc)
			audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
	}
	return false;
}

bool Debugger::cmdCheatItems(int argc, const char **argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		debugPrintf("Cheats are disabled\n");
		return true;
	}
	MainActor *av = getMainActor();
	if (!av) return true;
	Container *backpack = getContainer(av->getEquip(7)); // CONSTANT!
	if (!backpack) return true;

	// obsidian
	Item *money = ItemFactory::createItem(143, 7, 500, 0, 0, 0, 0, true);
	money->moveToContainer(backpack);
	money->setGumpLocation(40, 20);

	// skull of quakes
	Item *skull = ItemFactory::createItem(814, 0, 0, 0, 0, 0, 0, true);
	skull->moveToContainer(backpack);
	skull->setGumpLocation(60, 20);

	// recall item
	Item *recall = ItemFactory::createItem(833, 0, 0, 0, 0, 0, 0, true);
	recall->moveToContainer(backpack);
	recall->setGumpLocation(20, 20);

	// sword
	Item *sword = ItemFactory::createItem(420, 0, 0, 0, 0, 0, 0, true);
	sword->moveToContainer(backpack);
	sword->setGumpLocation(20, 30);

	Item *flamesting = ItemFactory::createItem(817, 0, 0, 0, 0, 0, 0, true);
	flamesting->moveToContainer(backpack);
	flamesting->setGumpLocation(20, 30);

	Item *hammer = ItemFactory::createItem(815, 0, 0, 0, 0, 0, 0, true);
	hammer->moveToContainer(backpack);
	hammer->setGumpLocation(20, 30);

	Item *slayer = ItemFactory::createItem(816, 0, 0, 0, 0, 0, 0, true);
	slayer->moveToContainer(backpack);
	slayer->setGumpLocation(20, 30);

	// necromancy reagents
	Item *bagitem = ItemFactory::createItem(637, 0, 0, 0, 0, 0, 0, true);
	bagitem->moveToContainer(backpack);
	bagitem->setGumpLocation(70, 40);

	bagitem = ItemFactory::createItem(637, 0, 0, 0, 0, 0, 0, true);
	Container *bag = dynamic_cast<Container *>(bagitem);

	Item *reagents = ItemFactory::createItem(395, 0, 50, 0, 0, 0, 0, true);
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(10, 10);
	reagents = ItemFactory::createItem(395, 6, 50, 0, 0, 0, 0, true);
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(30, 10);
	reagents = ItemFactory::createItem(395, 8, 50, 0, 0, 0, 0, true);
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(50, 10);
	reagents = ItemFactory::createItem(395, 9, 50, 0, 0, 0, 0, true);
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(20, 30);
	reagents = ItemFactory::createItem(395, 10, 50, 0, 0, 0, 0, true);
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(40, 30);
	reagents = ItemFactory::createItem(395, 14, 50, 0, 0, 0, 0, true);
	reagents->moveToContainer(bag);
	reagents->setGumpLocation(60, 30);

	bagitem->moveToContainer(backpack);
	bagitem->setGumpLocation(70, 20);

	// theurgy foci
	bagitem = ItemFactory::createItem(637, 0, 0, 0, 0, 0, 0, true);
	bag = dynamic_cast<Container *>(bagitem);

	Item *focus = ItemFactory::createItem(396, 8, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(10, 10);
	focus = ItemFactory::createItem(396, 9, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(25, 10);
	focus = ItemFactory::createItem(396, 10, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(40, 10);
	focus = ItemFactory::createItem(396, 11, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(55, 10);
	focus = ItemFactory::createItem(396, 12, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(70, 10);
	focus = ItemFactory::createItem(396, 13, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(10, 30);
	focus = ItemFactory::createItem(396, 14, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(30, 30);
	focus = ItemFactory::createItem(396, 15, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(50, 30);
	focus = ItemFactory::createItem(396, 17, 0, 0, 0, 0, 0, true);
	focus->moveToContainer(bag);
	focus->setGumpLocation(70, 30);

	bagitem->moveToContainer(backpack);
	bagitem->setGumpLocation(0, 30);


	// oil flasks
	Item *flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0, true);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);
	flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0, true);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);
	flask = ItemFactory::createItem(579, 0, 0, 0, 0, 0, 0, true);
	flask->moveToContainer(backpack);
	flask->setGumpLocation(30, 40);

	// zealan shield
	Item *shield = ItemFactory::createItem(828, 0, 0, 0, 0, 0, 0, true);
	shield->moveToContainer(backpack);
	shield->randomGumpLocation();

	shield = ItemFactory::createItem(539, 0, 0, 0, 0, 0, 0, true);
	shield->moveToContainer(backpack);
	shield->randomGumpLocation();

	// armour
	Item *armour = ItemFactory::createItem(64, 0, 0, 0, 0, 0, 0, true);
	armour->moveToContainer(backpack);
	armour->randomGumpLocation();

	// death disks
	Item *disk = ItemFactory::createItem(750, 0, 0, 0, 0, 0, 0, true);
	disk->moveToContainer(backpack);
	disk->randomGumpLocation();

	disk = ItemFactory::createItem(750, 0, 0, 0, 0, 0, 0, true);
	disk->moveToContainer(backpack);
	disk->randomGumpLocation();

	disk = ItemFactory::createItem(750, 0, 0, 0, 0, 0, 0, true);
	disk->moveToContainer(backpack);
	disk->randomGumpLocation();

	return false;
}

bool Debugger::cmdCheatEquip(int argc, const char **argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		debugPrintf("Cheats are disabled\n");
		return true;
	}
	MainActor *av = getMainActor();
	if (!av) return false;
	Container *backpack = getContainer(av->getEquip(7)); // CONSTANT!
	if (!backpack) return false;

	Item *item;

	// move all current equipment to backpack
	for (unsigned int i = 0; i < 7; ++i) {
		item = getItem(av->getEquip(i));
		if (item) {
			item->moveToContainer(backpack, false); // no weight/volume check
			item->randomGumpLocation();
		}
	}

	// give new equipment:

	// deceiver
	item = ItemFactory::createItem(822, 0, 0, 0, 0, 0, 0, true);
	av->setEquip(item, false);

	// armour
	item = ItemFactory::createItem(841, 0, 0, 0, 0, 0, 0, true);
	av->setEquip(item, false);

	// shield
	item = ItemFactory::createItem(842, 0, 0, 0, 0, 0, 0, true);
	av->setEquip(item, false);

	// helmet
	item = ItemFactory::createItem(843, 0, 0, 0, 0, 0, 0, true);
	av->setEquip(item, false);

	// arm guards
	item = ItemFactory::createItem(844, 0, 0, 0, 0, 0, 0, true);
	av->setEquip(item, false);

	// leggings
	item = ItemFactory::createItem(845, 0, 0, 0, 0, 0, 0, true);
	av->setEquip(item, false);

	return false;
}

bool Debugger::cmdHeal(int argc, const char **argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		debugPrintf("Cheats are disabled\n");
		return true;
	}
	MainActor *mainActor = getMainActor();

	mainActor->setHP(mainActor->getMaxHP());
	mainActor->setMana(mainActor->getMaxMana());
	return false;
}

bool Debugger::cmdToggleInvincibility(int argc, const char **argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		debugPrintf("Cheats are disabled\n");
		return true;
	}
	MainActor *av = getMainActor();

	if (av->hasActorFlags(Actor::ACT_INVINCIBLE)) {
		av->clearActorFlag(Actor::ACT_INVINCIBLE);
		debugPrintf("Avatar is no longer invincible.\n");
	} else {
		av->setActorFlag(Actor::ACT_INVINCIBLE);
		debugPrintf("Avatar invincible.\n");
	}

	return true;
}


bool Debugger::cmdStartHighlightItems(int argc, const char **argv) {
	GameMapGump::Set_highlightItems(true);
	return false;
}
bool Debugger::cmdStopHighlightItems(int argc, const char **argv) {
	GameMapGump::Set_highlightItems(false);
	return false;
}
bool Debugger::cmdToggleHighlightItems(int argc, const char **argv) {
	GameMapGump::Set_highlightItems(!GameMapGump::is_highlightItems());
	return false;
}

bool Debugger::cmdToggleFootpads(int argc, const char **argv) {
	GameMapGump::toggleFootpads();
	return false;
}

void Debugger::dumpCurrentMap() {
	// Increase number of available object IDs.
	ObjectManager::get_instance()->allow64kObjects();

	// top/bottom/left/right render coordinates
	int32 left = 16384;
	int32 right = -16384;
	int32 top = 16384;
	int32 bot = -16384;

	const int32 camheight = 256;
	const CurrentMap *curmap = World::get_instance()->getCurrentMap();
	const int32 chunksize = curmap->getChunkSize();

	// Work out the map limits in chunks
	for (int32 y = 0; y < MAP_NUM_CHUNKS; y++) {
		for (int32 x = 0; x < MAP_NUM_CHUNKS; x++) {
			const Std::list<Item *> *list = curmap->getItemList(x, y);

			// Should iterate the items!
			// (items could extend outside of this chunk and they have height)
			if (list && list->size() != 0) {
				// Bounds of render coordinates for items in this chunk
				int32 l = (x * chunksize - y * chunksize) / 4 - (chunksize / 4);
				int32 r = (x * chunksize - y * chunksize) / 4 + (chunksize / 4);
				int32 t = (x * chunksize + y * chunksize) / 8 - (chunksize / 2);
				int32 b = (x * chunksize + y * chunksize) / 8;

				t -= 256; // approx. adjustment for height of items in chunk

				if (l < left) left = l;
				if (r > right) right = r;
				if (t < top) top = t;
				if (b > bot) bot = b;
			}
		}
	}

	if (right == -16384) {
		debugPrintf("Map seems empty, nothing to dump.\n");
		// No objects?
		return;
	}

	// camera height
	bot += camheight;
	top += camheight;

	const int32 awidth = right - left;
	const int32 aheight = bot - top;

	//
	// If you are doing a once-off dump and get this error, try building ScummVM
	// with int32 size for Surfaces.  It breaks other engines but this is just a
	// once-off type debugging feature anyway.
	//
	// Most U8 maps can be dumped without needing int32, but most Crusader maps
	// need a patch to work.
	//
	Graphics::Surface nullsurface;
	if ((sizeof(nullsurface.pitch) == 2 && awidth > 8191) ||
		(sizeof(nullsurface.h) == 2 && aheight > 32767 )) {
		warning("WARN: Can't dump map, image will not fit into 16 bit dimensions.");
		return;
	}

	GameMapGump *g = new GameMapGump(0, 0, awidth, aheight);

	// HACK: Setting both INVISIBLE and TRANSPARENT flags on the Avatar
	// will make him completely invisible.
	getMainActor()->setFlag(Item::FLG_INVISIBLE);
	getMainActor()->setExtFlag(Item::EXT_TRANSPARENT);

	CurrentMap *currentMap = World::get_instance()->getCurrentMap();
	currentMap->setWholeMapFast();

	Graphics::Screen *screen = Ultima8Engine::get_instance()->getScreen();
	RenderSurface *s = new RenderSurface(awidth, aheight, screen->format);

	debugPrintf("Rendering map...\n");

	// Camera coordinates in world-coords (in the middle of the map)
	int32 midx = left + (right - left) / 2;
	int32 midy = top + (bot - top) / 2;
	int32 cx = midx * 2 + midy * 4;
	int32 cy = midy * 4 - midx * 2;

	// Now render the map
	s->BeginPainting();
	s->SetOrigin(0, 0);
	Point3 pt(cx + camheight * 4, cy + camheight * 4, camheight);
	CameraProcess::SetCameraProcess(new CameraProcess(pt));
	g->Paint(s, 256, false);
	s->EndPainting();

#ifdef USE_PNG
	Common::Path filename(Common::String::format("map_%03d.png", currentMap->getNum()));
#else
	Common::Path filename(Common::String::format("map_%03d.bmp", currentMap->getNum()));
#endif

	Common::DumpFile dumpFile;
	bool result = dumpFile.open(filename);
	if (result) {
#ifdef USE_PNG
		result = Image::writePNG(dumpFile, *(s->getRawSurface()));
#else
		result = Image::writeBMP(dumpFile, *(s->getRawSurface()));
#endif
	}

	if (result) {
		debugPrintf("Map dumped: %s\n", filename.toString().c_str());
	} else {
		debugPrintf("Could not write file: %s\n", filename.toString().c_str());
	}

	delete g;
	delete s;

}

bool Debugger::cmdDumpMap(int argc, const char **argv) {
	// Save because we're going to potentially break the game by enlarging
	// the fast area and available object IDs.
	int slot = Ultima8Engine::get_instance()->getAutosaveSlot();
	Common::Error result = Ultima8Engine::get_instance()->saveGameState(slot, "Pre-dumpMap save");
	if (result.getCode() != Common::kNoError) {
		debugPrintf("Could not dump map: pre-dumpMap save failed\n");
		return false;
	}

	if (argc > 1) {
		int mapno = atoi(argv[1]);
		debugPrintf("Switching to map %d\n", mapno);
		bool success = World::get_instance()->switchMap(mapno);
		if (!success) {
			debugPrintf("Dump failed: switch to map %d FAILED\n", mapno);
			return false;
		}
	}
	dumpCurrentMap();

	// Reload
	Ultima8Engine::get_instance()->loadGameState(slot);
	return false;
}


bool Debugger::cmdDumpAllMaps(int argc, const char **argv) {
	// Save because we're going to potentially break the game by enlarging
	// the fast area and available object IDs and changing maps
	int slot = Ultima8Engine::get_instance()->getAutosaveSlot();
	Common::Error result = Ultima8Engine::get_instance()->saveGameState(slot, "Pre-dumpMap save");
	if (result.getCode() != Common::kNoError) {
		debugPrintf("Could not dump map: pre-dumpMap save failed\n");
		return false;
	}

	for (int i = 0; i < 256; i++) {
		if (World::get_instance()->switchMap(i)) {
			debugPrintf("Dumping map %d...\n", i);
			dumpCurrentMap();
		}
	}

	// Reload
	Ultima8Engine::get_instance()->loadGameState(slot);
	return false;
}


bool Debugger::cmdIncrementSortOrder(int argc, const char **argv) {
	int32 count = argc > 1 ? strtol(argv[1], 0, 0) : 1;
	GameMapGump *gump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gump)
		gump->IncSortOrder(count);
	return false;
}

bool Debugger::cmdDecrementSortOrder(int argc, const char **argv) {
	int32 count = argc > 1 ? strtol(argv[1], 0, 0) : 1;
	GameMapGump *gump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gump)
		gump->IncSortOrder(-count);
	return false;
}


bool Debugger::cmdProcessTypes(int argc, const char **argv) {
	Kernel::get_instance()->processTypes();
	return true;
}

bool Debugger::cmdListProcesses(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("usage: listProcesses [<itemnum>]\n");
	} else {
		Kernel *kern = Kernel::get_instance();
		ObjId item = 0;
		if (argc == 2) {
			item = static_cast<ObjId>(strtol(argv[1], 0, 0));
			debugPrintf("Processes for item %d:\n", item);
		} else {
			debugPrintf("Processes:\n");
		}
		for (ProcessIterator it = kern->_processes.begin();
			it != kern->_processes.end(); ++it) {
			Process *p = *it;
			if (argc == 1 || p->_itemNum == item) {
				debugPrintf("%s\n", p->dumpInfo().c_str());
			}
		}
	}

	return true;
}

bool Debugger::cmdProcessInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("usage: processInfo <objectnum>\n");
	} else {
		Kernel *kern = Kernel::get_instance();

		ProcId procid = static_cast<ProcId>(strtol(argv[1], 0, 0));

		Process *p = kern->getProcess(procid);
		if (p == 0) {
			debugPrintf("No such process: %d\n", procid);
		} else {
			debugPrintf("%s\n", p->dumpInfo().c_str());
		}
	}

	return true;
}

bool Debugger::cmdToggleFrameByFrame(int argc, const char **argv) {
	Kernel *kern = Kernel::get_instance();
	bool fbf = !kern->isFrameByFrame();
	kern->setFrameByFrame(fbf);
	debugPrintf("FrameByFrame = %s\n", strBool(fbf));

	if (fbf)
		kern->pause();
	else
		kern->unpause();

	return true;
}

bool Debugger::cmdAdvanceFrame(int argc, const char **argv) {
	Kernel *kern = Kernel::get_instance();
	if (kern->isFrameByFrame()) {
		kern->unpause();
		debugPrintf("FrameByFrame: Next Frame\n");
	}

	return true;
}


bool Debugger::cmdTeleport(int argc, const char **argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		debugPrintf("Cheats are disabled\n");
		return true;
	}
	MainActor *mainActor = getMainActor();
	int curmap = mainActor->getMapNum();

	switch (argc - 1) {
	case 1:
		mainActor->teleport(curmap,
			strtol(argv[1], 0, 0));
		break;
	case 2:
		mainActor->teleport(strtol(argv[1], 0, 0),
			strtol(argv[2], 0, 0));
		break;
	case 3:
		mainActor->teleport(curmap,
			strtol(argv[1], 0, 0),
			strtol(argv[2], 0, 0),
			strtol(argv[3], 0, 0));
		break;
	case 4:
		mainActor->teleport(strtol(argv[1], 0, 0),
			strtol(argv[2], 0, 0),
			strtol(argv[3], 0, 0),
			strtol(argv[4], 0, 0));
		break;
	default:
		debugPrintf("teleport usage:\n");
		debugPrintf("teleport <mapnum> <x> <y> <z>: teleport to (x,y,z) on map mapnum\n");
		debugPrintf("teleport <x> <y> <z>: teleport to (x,y,z) on current map\n");
		debugPrintf("teleport <mapnum> <eggnum>: teleport to target egg eggnum on map mapnum\n");
		debugPrintf("teleport <eggnum>: teleport to target egg eggnum on current map\n");
		return true;
	}

	return false;
}

bool Debugger::cmdMark(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: mark <mark>: set named mark to this location\n");
		return true;
	}

	MainActor *mainActor = getMainActor();
	int curmap = mainActor->getMapNum();
	Point3 pt = mainActor->getLocation();

	Common::String key = Common::String::format("mark_%s", argv[1]);
	Common::String value = Common::String::format("%d %d %d %d", curmap, pt.x, pt.y, pt.z);
	ConfMan.set(key, value);

	debugPrintf("Set mark \"%s\" to %s\n", argv[1], value.c_str());
	return true;
}

bool Debugger::cmdRecall(int argc, const char **argv) {
	if (!Ultima8Engine::get_instance()->areCheatsEnabled()) {
		debugPrintf("Cheats are disabled\n");
		return true;
	}
	if (argc == 1) {
		debugPrintf("Usage: recall <mark>: recall to named mark\n");
		return true;
	}

	MainActor *mainActor = getMainActor();
	Common::String key = Common::String::format("mark_%s", argv[1]);
	if (!ConfMan.hasKey(key)) {
		debugPrintf("recall: no such mark\n");
		return true;
	}

	Common::String target = ConfMan.get(key);
	int t[4];
	int n = sscanf(target.c_str(), "%d%d%d%d", &t[0], &t[1], &t[2], &t[3]);
	if (n != 4) {
		debugPrintf("recall: invalid mark\n");
		return true;
	}

	mainActor->teleport(t[0], t[1], t[2], t[3]);
	return false;
}

bool Debugger::cmdListMarks(int argc, const char **argv) {
	const Common::ConfigManager::Domain *domain = ConfMan.getActiveDomain();
	Common::ConfigManager::Domain::const_iterator dit;
	Common::StringArray marks;
	for (dit = domain->begin(); dit != domain->end(); ++dit) {
		if (dit->_key.hasPrefix("mark_")) {
			marks.push_back(dit->_key.substr(5));
		}
	}

	Common::sort(marks.begin(), marks.end());
	Common::StringArray::const_iterator mit;
	for (mit = marks.begin(); mit != marks.end(); ++mit) {
		debugPrintf("%s\n", mit->c_str());
	}

	return true;
}

bool Debugger::cmdName(int argc, const char **argv) {
	MainActor *av = getMainActor();
	if (argc > 1)
		av->setName(argv[1]);

	debugPrintf("MainActor::name = \"%s\"\n", av->getName().c_str());
	return true;
}

bool Debugger::cmdUseBackpack(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use backpack: avatarInStasis");
		return false;
	}
	MainActor *av = getMainActor();
	Item *backpack = getItem(av->getEquip(ShapeInfo::SE_BACKPACK));
	if (backpack)
		backpack->callUsecodeEvent_use();
	return false;
}

static bool _isAvatarControlled() {
	World *world = World::get_instance();
	return (world && world->getControlledNPCNum() == 1);
}

bool Debugger::cmdNextInventory(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use inventory: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	MainActor *av = getMainActor();
	av->nextInvItem();
	return false;
}

bool Debugger::cmdNextWeapon(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't change weapon: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	MainActor *av = getMainActor();
	av->nextWeapon();
	return false;
}

bool Debugger::cmdUseInventoryItem(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use active inventory item: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	MainActor *av = getMainActor();
	ObjId activeitemid = av->getActiveInvItem();
	if (activeitemid) {
		Item *item = getItem(activeitemid);
		if (item) {
			av->useInventoryItem(item);
		}
	}
	return false;
}

bool Debugger::cmdUseMedikit(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use medikit: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	MainActor *av = getMainActor();
	av->useInventoryItem(0x351);
	return false;
}

bool Debugger::cmdUseEnergyCube(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use energy cube: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	MainActor *av = getMainActor();
	av->useInventoryItem(0x582);
	return false;
}

bool Debugger::cmdDetonateBomb(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't detonate bomb: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	MainActor *av = getMainActor();
	av->detonateBomb();
	return false;
}

bool Debugger::cmdDropWeapon(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't drop weapon: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	MainActor *av = getMainActor();
	av->dropWeapon();
	return false;
}

bool Debugger::cmdUseInventory(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use inventory: avatarInStasis");
		return false;
	}
	MainActor *av = getMainActor();
	av->callUsecodeEvent_use();
	return false;
}

bool Debugger::cmdUseRecall(int argc, const char **argv) {
	MainActor *av = getMainActor();
	if (GAME_IS_U8)
		av->useInventoryItem(833);
	else {
		TargetReticleProcess *reticle = TargetReticleProcess::get_instance();
		if (reticle) {
			reticle->toggle();
		}
	}
	return false;
}

bool Debugger::cmdUseBedroll(int argc, const char **argv) {
	MainActor *av = getMainActor();
	av->useInventoryItem(534);
	return false;
}

bool Debugger::cmdUseKeyring(int argc, const char **argv) {
	MainActor *av = getMainActor();
	av->useInventoryItem(79);
	return false;
}

bool Debugger::cmdCameraOnAvatar(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isCruStasis()) {
		debugPrintf("Can't move camera: cruStasis");
		return false;
	}
	Actor *actor = getControlledActor();
	if (actor) {
		Point3 pt = actor->getCentre();
		if (pt.x > 0 || pt.y > 0)
			CameraProcess::SetCameraProcess(new CameraProcess(pt));
	}
	return false;
}

bool Debugger::cmdToggleCombat(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't toggle combat: avatarInStasis");
		return false;
	}

	MainActor *av = getMainActor();
	av->toggleInCombat();
	return false;
}

bool Debugger::cmdStartSelection(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't select items: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	// Clear this flag on selection to match original behavior.
	Ultima8Engine::get_instance()->setCrusaderTeleporting(false);

	ItemSelectionProcess *proc = ItemSelectionProcess::get_instance();
	if (proc)
		proc->selectNextItem(false);
	return false;
}

bool Debugger::cmdUseSelection(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use items: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	ItemSelectionProcess *proc = ItemSelectionProcess::get_instance();
	if (proc)
		proc->useSelectedItem();
	return false;
}

bool Debugger::cmdGrabItems(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't grab items: avatarInStasis");
		return false;
	}

	// Only if controlling avatar.
	if (!_isAvatarControlled()) {
		return false;
	}

	// Clear this flag on selection to match original behavior.
	Ultima8Engine::get_instance()->setCrusaderTeleporting(false);

	ItemSelectionProcess *proc = ItemSelectionProcess::get_instance();
	if (proc)
		proc->selectNextItem(true);
	return false;
}

bool Debugger::cmdObjectTypes(int argc, const char **argv) {
	ObjectManager::get_instance()->objectTypes();
	return true;
}

bool Debugger::cmdObjectInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("usage: objectInfo <objectnum>\n");
	} else {
		ObjectManager *objMan = ObjectManager::get_instance();

		ObjId objid = static_cast<ObjId>(strtol(argv[1], 0, 0));

		Object *obj = objMan->getObject(objid);
		if (obj == 0) {
			bool reserved = false;
			if (objid >= 256) // CONSTANT!
				reserved = objMan->_objIDs->isIDUsed(objid);
			else
				reserved = objMan->_actorIDs->isIDUsed(objid);
			if (reserved)
				debugPrintf("Reserved objid: %d\n", objid);
			else
				debugPrintf("No such object: %d\n", objid);
		} else {
			debugPrintf("%s\n", obj->dumpInfo().c_str());
		}
	}

	return true;
}

static bool _quickMoveKey(uint32 flag, const char *debugname) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	if (engine->isAvatarInStasis()) {
		g_debugger->debugPrintf("Can't %s: avatarInStasis", debugname);
		return true;
	}
	if (!engine->areCheatsEnabled()) {
		g_debugger->debugPrintf("Can't %s: Cheats aren't enabled", debugname);
		return true;
	}

	QuickAvatarMoverProcess *proc = QuickAvatarMoverProcess::get_instance();
	if (proc) {
		proc->setMovementFlag(flag);
	}
	return false;
}

static bool _quickMoveKeyEnd(uint32 flag) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	if (engine->isAvatarInStasis()) {
		return false;
	}
	if (!engine->areCheatsEnabled()) {
		return false;
	}

	QuickAvatarMoverProcess *proc = QuickAvatarMoverProcess::get_instance();
	if (proc) {
		proc->clearMovementFlag(flag);
	}
	return false;
}

bool Debugger::cmdStartQuickMoveUp(int argc, const char **argv) {
	return _quickMoveKey(QuickAvatarMoverProcess::MOVE_UP, "move up");
}

bool Debugger::cmdStartQuickMoveDown(int argc, const char **argv) {
	return _quickMoveKey(QuickAvatarMoverProcess::MOVE_DOWN, "move down");
}

bool Debugger::cmdStartQuickMoveLeft(int argc, const char **argv) {
	return _quickMoveKey(QuickAvatarMoverProcess::MOVE_LEFT, "move left");
}

bool Debugger::cmdStartQuickMoveRight(int argc, const char **argv) {
	return _quickMoveKey(QuickAvatarMoverProcess::MOVE_RIGHT, "move right");
}

bool Debugger::cmdStartQuickMoveAscend(int argc, const char **argv) {
	return _quickMoveKey(QuickAvatarMoverProcess::MOVE_ASCEND, "move ascend");
}

bool Debugger::cmdStartQuickMoveDescend(int argc, const char **argv) {
	return _quickMoveKey(QuickAvatarMoverProcess::MOVE_DESCEND, "move descend");
}

bool Debugger::cmdStopQuickMoveUp(int argc, const char **argv) {
	return _quickMoveKeyEnd(QuickAvatarMoverProcess::MOVE_UP);
}

bool Debugger::cmdStopQuickMoveDown(int argc, const char **argv) {
	return _quickMoveKeyEnd(QuickAvatarMoverProcess::MOVE_DOWN);
}

bool Debugger::cmdStopQuickMoveLeft(int argc, const char **argv) {
	return _quickMoveKeyEnd(QuickAvatarMoverProcess::MOVE_LEFT);
}

bool Debugger::cmdStopQuickMoveRight(int argc, const char **argv) {
	return _quickMoveKeyEnd(QuickAvatarMoverProcess::MOVE_RIGHT);
}

bool Debugger::cmdStopQuickMoveAscend(int argc, const char **argv) {
	return _quickMoveKeyEnd(QuickAvatarMoverProcess::MOVE_ASCEND);
}

bool Debugger::cmdStopQuickMoveDescend(int argc, const char **argv) {
	return _quickMoveKeyEnd(QuickAvatarMoverProcess::MOVE_DESCEND);
}

bool Debugger::cmdToggleQuarterSpeed(int argc, const char **argv) {
	QuickAvatarMoverProcess::setQuarterSpeed(!QuickAvatarMoverProcess::isQuarterSpeed());
	return false;
}

bool Debugger::cmdToggleClipping(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::toggleClipping();
		debugPrintf("QuickAvatarMoverProcess::_clipping = %s\n",
			strBool(QuickAvatarMoverProcess::isClipping()));
	} else {
		debugPrintf("Cheats aren't enabled\n");
	}
	return true;
}


bool Debugger::cmdGetGlobal(int argc, const char **argv) {
	UCMachine *uc = UCMachine::get_instance();
	if (argc != 3) {
		debugPrintf("usage: UCMachine::getGlobal offset size\n");
		return true;
	}

	unsigned int offset = strtol(argv[1], 0, 0);
	unsigned int size = strtol(argv[2], 0, 0);

	debugPrintf("[%04X %02X] = %d\n", offset, size, uc->_globals->getEntries(offset, size));
	return true;
}

bool Debugger::cmdSetGlobal(int argc, const char **argv) {
	UCMachine *uc = UCMachine::get_instance();
	if (argc != 4) {
		debugPrintf("usage: UCMachine::setGlobal offset size value\n");
		return true;
	}

	unsigned int offset = strtol(argv[1], 0, 0);
	unsigned int size = strtol(argv[2], 0, 0);
	unsigned int value = strtol(argv[3], 0, 0);

	uc->_globals->setEntries(offset, size, value);

	debugPrintf("[%04X %02X] = %d\n", offset, size, uc->_globals->getEntries(offset, size));
	return true;
}

bool Debugger::cmdTracePID(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: UCMachine::tracePID _pid\n");
		return true;
	}

	uint16 pid = static_cast<uint16>(strtol(argv[1], 0, 0));

	UCMachine *uc = UCMachine::get_instance();
	uc->_tracingEnabled = true;
	uc->_tracePIDs.insert(pid);

	debugPrintf("UCMachine: tracing process %d\n", pid);
	return true;
}

bool Debugger::cmdTraceObjID(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: UCMachine::traceObjID objid\n");
		return true;
	}

	uint16 objid = static_cast<uint16>(strtol(argv[1], 0, 0));

	UCMachine *uc = UCMachine::get_instance();
	uc->_tracingEnabled = true;
	uc->_traceObjIDs.insert(objid);

	debugPrintf("UCMachine: tracing object %d\n", objid);
	return true;
}

bool Debugger::cmdTraceClass(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: UCMachine::traceClass class\n");
		return true;
	}

	uint16 ucclass = static_cast<uint16>(strtol(argv[1], 0, 0));

	UCMachine *uc = UCMachine::get_instance();
	uc->_tracingEnabled = true;
	uc->_traceClasses.insert(ucclass);

	debugPrintf("UCMachine: tracing class %d\n", ucclass);
	return true;
}

bool Debugger::cmdTraceAll(int argc, const char **argv) {
	UCMachine *uc = UCMachine::get_instance();
	uc->_tracingEnabled = true;
	uc->_traceAll = true;

	debugPrintf("UCMachine: tracing all usecode\n");
	return true;
}

bool Debugger::cmdStopTrace(int argc, const char **argv) {
	UCMachine *uc = UCMachine::get_instance();
	uc->_traceObjIDs.clear();
	uc->_tracePIDs.clear();
	uc->_traceClasses.clear();
	uc->_tracingEnabled = false;
	uc->_traceAll = false;

	debugPrintf("Trace stopped\n");
	return true;
}

bool Debugger::cmdVerifyQuit(int argc, const char **argv) {
	QuitGump::verifyQuit();
	return false;
}

bool Debugger::cmdU8ShapeViewer(int argc, const char **argv) {
	ShapeViewerGump::U8ShapeViewer();
	return false;
}

bool Debugger::cmdShowMenu(int argc, const char **argv) {
	World *world = World::get_instance();
	// In Crusader escape is also used to stop controlling another NPC
	if (world && world->getControlledNPCNum() != kMainActorId) {
		world->setControlledNPCNum(kMainActorId);
		return false;
	}
	if (Ultima8Engine::get_instance()->isCruStasis()) {
		Ultima8Engine::get_instance()->moveKeyEvent();
		debugPrintf("Not opening menu: cruStasis\n");
		return false;
	}
	Gump *gump = Ultima8Engine::get_instance()->getDesktopGump()->FindGump<ModalGump>();
	if (gump) {
		// ensure any modal gump gets the message to close before we open the menu.
		gump->Close();
		return false;
	}
	MenuGump::showMenu();
	return false;
}

bool Debugger::cmdToggleFastArea(int argc, const char **argv) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	Gump *desktop = app->getDesktopGump();
	Gump *favg = desktop->FindGump<FastAreaVisGump>();

	if (!favg) {
		favg = new FastAreaVisGump;
		favg->InitGump(0);
		favg->setRelativePosition(Gump::TOP_RIGHT, -4, 4);
	} else {
		favg->Close();
	}

	return false;
}

bool Debugger::cmdInvertScreen(int argc, const char **argv) {
	InverterProcess::invertScreen();
	return false;
}

bool Debugger::cmdPlayMovie(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("play usage: play <moviename>\n");
		return true;
	}

	Common::String filename = Common::String::format("static/%s.skf", argv[1]);
	auto *skf = new Common::File();
	if (!skf->open(filename.c_str())) {
		debugPrintf("movie not found.\n");
		delete skf;
		return true;
	}

	MovieGump::U8MovieViewer(skf, false, false, true);
	return false;
}

bool Debugger::cmdPlayMusic(int argc, const char **argv) {
	if (MusicProcess::_theMusicProcess) {
		if (argc != 2) {
			debugPrintf("MusicProcess::playMusic (tracknum)\n");
		} else {
			debugPrintf("Playing track %s\n", argv[1]);
			MusicProcess::_theMusicProcess->playMusic_internal(atoi(argv[1]));
			return false;
		}
	} else {
		debugPrintf("No Music Process\n");
	}

	return true;
}

bool Debugger::cmdToggleMinimap(int argc, const char **argv) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	Gump *desktop = app->getDesktopGump();
	Gump *mmg = desktop->FindGump<MiniMapGump>();

	if (!mmg) {
		mmg = new MiniMapGump(4, 4);
		mmg->InitGump(0);
		mmg->setRelativePosition(Gump::TOP_LEFT, 4, 4);
	} else if (mmg->IsHidden()) {
		mmg->UnhideGump();
	} else {
		mmg->HideGump();
	}

	return false;
}

bool Debugger::cmdGenerateMinimap(int argc, const char **argv) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	Gump *desktop = app->getDesktopGump();
	MiniMapGump *gump = dynamic_cast<MiniMapGump *>(desktop->FindGump<MiniMapGump>());

	if (gump) {
		gump->generate();

		if (argc > 1) {
			Common::Path filename(argv[1]);
			bool result = gump->dump(filename);
			if (result) {
				debugPrintf("Mini map dumped: %s\n", filename.toString().c_str());
			} else {
				debugPrintf("Could not write file: %s\n", filename.toString().c_str());
			}
		}
	}
	return false;
}


bool Debugger::cmdClearMinimap(int argc, const char **argv) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	Gump *desktop = app->getDesktopGump();
	MiniMapGump *gump = dynamic_cast<MiniMapGump *>(desktop->FindGump<MiniMapGump>());

	if (gump) {
		gump->clear();
	}
	return false;
}

bool Debugger::cmdBenchmarkRenderSurface(int argc, const char **argv) {
	if (argc != 4) {
		debugPrintf("usage: RenderSurface::benchmark shapenum framenum iterations\n");
		return true;
	}

	int shapenum = atoi(argv[1]);
	int frame = atoi(argv[2]);
	int count = atoi(argv[3]);

	GameData *gamedata = GameData::get_instance();
	Shape *s = gamedata->getMainShapes()->getShape(shapenum);

	Graphics::Screen *screen = Ultima8Engine::get_instance()->getScreen();
	RenderSurface *surface = new RenderSurface(320, 200, screen->format);
	surface->BeginPainting();

	uint32 start, end;
	uint32 blendColor = TEX32_PACK_RGBA(0x7F, 0x00, 0x00, 0x7F);

	start = g_system->getMillis();
	for (int i = 0; i < count; i++) {
		surface->Paint(s, frame, 160, 100);
	}
	end = g_system->getMillis();
	debugPrintf("Paint: %d\n", end - start);

	start = g_system->getMillis();
	for (int i = 0; i < count; i++) {
		surface->PaintTranslucent(s, frame, 160, 100);
	}
	end = g_system->getMillis();
	debugPrintf("PaintTranslucent: %d\n", end - start);

	start = g_system->getMillis();
	for (int i = 0; i < count; i++) {
		surface->Paint(s, frame, 160, 100, true);
	}
	end = g_system->getMillis();
	debugPrintf("PaintMirrored: %d\n", end - start);

	start = g_system->getMillis();
	for (int i = 0; i < count; i++) {
		surface->PaintInvisible(s, frame, 160, 100, false, false);
	}
	end = g_system->getMillis();
	debugPrintf("PaintInvisible: %d\n", end - start);

	start = g_system->getMillis();
	for (int i = 0; i < count; i++) {
		surface->PaintHighlight(s, frame, 160, 100, false, false, blendColor);
	}
	end = g_system->getMillis();
	debugPrintf("PaintHighlight: %d\n", end - start);

	start = g_system->getMillis();
	for (int i = 0; i < count; i++) {
		surface->PaintHighlightInvis(s, frame, 160, 100, false, false, blendColor);
	}
	end = g_system->getMillis();
	debugPrintf("PaintHighlightInvis: %d\n", end - start);

	surface->EndPainting();
	delete surface;

	return true;
}

bool Debugger::cmdVisualDebugPathfinder(int argc, const char **argv) {
#ifdef DEBUG_PATHFINDER
	if (argc != 2) {
		debugPrintf("Usage: Pathfinder::visualDebug objid\n");
		debugPrintf("Specify objid -1 to stop tracing.\n");
		return true;
	}
	int p = strtol(argv[1], 0, 0);
	if (p == -1) {
		Pathfinder::_visualDebugActor = 0xFFFF;
		debugPrintf("Pathfinder: stopped visual tracing\n");
	} else {
		Pathfinder::_visualDebugActor = (uint16)p;
		debugPrintf("Pathfinder: visually tracing _actor %d\n", Pathfinder::_visualDebugActor);
	}
#endif
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
