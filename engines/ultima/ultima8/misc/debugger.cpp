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

#include "image/png.h"
#include "image/bmp.h"
#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/graphics/inverter_process.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/gumps/fast_area_vis_gump.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/gumps/minimap_gump.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/gumps/shape_viewer_gump.h"
#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/misc/id_man.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/usecode/bit_set.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/item_selection_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/pathfinder.h"

namespace Ultima {
namespace Ultima8 {

Debugger *g_debugger;

// Console out/err pointers
console_ostream<char> *ppout;
console_err_ostream<char> *pperr;

Debugger::Debugger() : Shared::Debugger() {
	g_debugger = this;

	// Set output pointers
	ppout = &_strOut;
	pperr = &_errOut;

	// WARNING: Not only can the methods below be executed directly in the debugger,
	// they also act as the methods keybindings are made to. So be wary of changing names
	registerCmd("quit", WRAP_METHOD(Debugger, cmdQuit));
	registerCmd("Ultima8Engine::quit", WRAP_METHOD(Debugger, cmdQuit));
	registerCmd("Ultima8Engine::saveGame", WRAP_METHOD(Debugger, cmdSaveGame));
	registerCmd("Ultima8Engine::loadGame", WRAP_METHOD(Debugger, cmdLoadGame));
	registerCmd("Ultima8Engine::newGame", WRAP_METHOD(Debugger, cmdNewGame));
	registerCmd("Ultima8Engine::engineStats", WRAP_METHOD(Debugger, cmdEngineStats));
	registerCmd("Ultima8Engine::changeGame", WRAP_METHOD(Debugger, cmdChangeGame));
	registerCmd("Ultima8Engine::listGames", WRAP_METHOD(Debugger, cmdListGames));
	registerCmd("Ultima8Engine::memberVar", WRAP_METHOD(Debugger, cmdMemberVar));
	registerCmd("Ultima8Engine::setVideoMode", WRAP_METHOD(Debugger, cmdSetVideoMode));
	registerCmd("Ultima8Engine::toggleAvatarInStasis", WRAP_METHOD(Debugger, cmdToggleAvatarInStasis));
	registerCmd("Ultima8Engine::togglePaintEditorItems", WRAP_METHOD(Debugger, cmdTogglePaintEditorItems));
	registerCmd("Ultima8Engine::toggleShowTouchingItems", WRAP_METHOD(Debugger, cmdToggleShowTouchingItems));
	registerCmd("Ultima8Engine::closeItemGumps", WRAP_METHOD(Debugger, cmdCloseItemGumps));

	registerCmd("AvatarMoverProcess::startJump", WRAP_METHOD(Debugger, cmdStartJump));
	registerCmd("AvatarMoverProcess::stopJump", WRAP_METHOD(Debugger, cmdStopJump));
	registerCmd("AvatarMoverProcess::startTurnLeft", WRAP_METHOD(Debugger, cmdStartTurnLeft));
	registerCmd("AvatarMoverProcess::startTurnRight", WRAP_METHOD(Debugger, cmdStartTurnRight));
	registerCmd("AvatarMoverProcess::startMoveForward", WRAP_METHOD(Debugger, cmdStartMoveForward));
	registerCmd("AvatarMoverProcess::startMoveBack", WRAP_METHOD(Debugger, cmdStartMoveBack));
	registerCmd("AvatarMoverProcess::stopTurnLeft", WRAP_METHOD(Debugger, cmdStopTurnLeft));
	registerCmd("AvatarMoverProcess::stopTurnRight", WRAP_METHOD(Debugger, cmdStopTurnRight));
	registerCmd("AvatarMoverProcess::stopMoveForward", WRAP_METHOD(Debugger, cmdStopMoveForward));
	registerCmd("AvatarMoverProcess::stopMoveBack", WRAP_METHOD(Debugger, cmdStopMoveBack));

	registerCmd("AvatarMoverProcess::startMoveLeft", WRAP_METHOD(Debugger, cmdStartMoveLeft));
	registerCmd("AvatarMoverProcess::startMoveRight", WRAP_METHOD(Debugger, cmdStartMoveRight));
	registerCmd("AvatarMoverProcess::startMoveUp", WRAP_METHOD(Debugger, cmdStartMoveUp));
	registerCmd("AvatarMoverProcess::startMoveDown", WRAP_METHOD(Debugger, cmdStartMoveDown));
	registerCmd("AvatarMoverProcess::stopMoveLeft", WRAP_METHOD(Debugger, cmdStopMoveLeft));
	registerCmd("AvatarMoverProcess::stopMoveRight", WRAP_METHOD(Debugger, cmdStopMoveRight));
	registerCmd("AvatarMoverProcess::stopMoveUp", WRAP_METHOD(Debugger, cmdStopMoveUp));
	registerCmd("AvatarMoverProcess::stopMoveDown", WRAP_METHOD(Debugger, cmdStopMoveDown));

	registerCmd("AvatarMoverProcess::startMoveRun", WRAP_METHOD(Debugger, cmdStartMoveRun));
	registerCmd("AvatarMoverProcess::stopMoveRun", WRAP_METHOD(Debugger, cmdStopMoveRun));
	registerCmd("AvatarMoverProcess::startMoveStep", WRAP_METHOD(Debugger, cmdStartMoveStep));
	registerCmd("AvatarMoverProcess::stopMoveStep", WRAP_METHOD(Debugger, cmdStopMoveStep));
	registerCmd("AvatarMoverProcess::tryAttack", WRAP_METHOD(Debugger, cmdAttack));

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

	registerCmd("GameMapGump::toggleHighlightItems", WRAP_METHOD(Debugger, cmdToggleHighlightItems));
	registerCmd("GameMapGump::dumpMap", WRAP_METHOD(Debugger, cmdDumpMap));
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
	registerCmd("MainActor::detonateBomb", WRAP_METHOD(Debugger, cmdDetonateBomb));
	registerCmd("MainActor::toggleCombat", WRAP_METHOD(Debugger, cmdToggleCombat));
	registerCmd("ItemSelectionProcess::startSelection", WRAP_METHOD(Debugger, cmdStartSelection));
	registerCmd("ItemSelectionProcess::useSelectedItem", WRAP_METHOD(Debugger, cmdUseSelection));

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
#ifdef DEBUG
	registerCmd("UCMachine::traceObjID", WRAP_METHOD(Debugger, cmdTraceObjID));
	registerCmd("UCMachine::tracePID", WRAP_METHOD(Debugger, cmdTracePID));
	registerCmd("UCMachine::traceClass", WRAP_METHOD(Debugger, cmdTraceClass));
	registerCmd("UCMachine::traceEvents", WRAP_METHOD(Debugger, cmdTraceEvents));
	registerCmd("UCMachine::traceAll", WRAP_METHOD(Debugger, cmdTraceAll));
	registerCmd("UCMachine::stopTrace", WRAP_METHOD(Debugger, cmdStopTrace));
#endif

	registerCmd("FastAreaVisGump::toggle", WRAP_METHOD(Debugger, cmdToggleFastArea));
	registerCmd("InverterProcess::invertScreen", WRAP_METHOD(Debugger, cmdInvertScreen));
	registerCmd("MenuGump::showMenu", WRAP_METHOD(Debugger, cmdShowMenu));
	registerCmd("MiniMapGump::toggle", WRAP_METHOD(Debugger, cmdToggleMinimap));
	registerCmd("MiniMapGump::generateWholeMap", WRAP_METHOD(Debugger, cmdGenerateWholeMap));
	registerCmd("MovieGump::play", WRAP_METHOD(Debugger, cmdPlayMovie));
	registerCmd("MusicProcess::playMusic", WRAP_METHOD(Debugger, cmdPlayMusic));
	registerCmd("QuitGump::verifyQuit", WRAP_METHOD(Debugger, cmdVerifyQuit));
	registerCmd("ShapeViewerGump::U8ShapeViewer", WRAP_METHOD(Debugger, cmdU8ShapeViewer));

#ifdef DEBUG
	registerCmd("Pathfinder::visualDebug", WRAP_METHOD(Debugger, cmdVisualDebugPathfinder));
#endif
}

Debugger::~Debugger() {
	g_debugger = nullptr;
	ppout = nullptr;
	pperr = nullptr;
}


void Debugger::executeCommand(const ArgsType &args) {
	ArgvType argv;
	StringToArgv(args, argv);

	executeCommand(argv);
}

void Debugger::executeCommand(const ArgvType &argv) {
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
		// Save a _game with the given name into the quicksave slot
		Ultima8Engine::get_instance()->saveGame(1, argv[1]);
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

bool Debugger::cmdChangeGame(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Current _game is: %s\n", Ultima8Engine::get_instance()->_gameInfo->_name.c_str());
	} else {
		Ultima8Engine::get_instance()->changeGame(argv[1]);
	}

	return true;
}

bool Debugger::cmdListGames(int argc, const char **argv) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	Std::vector<istring> games;
	games = app->_settingMan->listGames();
	Std::vector<istring>::const_iterator iter;
	for (iter = games.begin(); iter != games.end(); ++iter) {
		const istring &_game = *iter;
		GameInfo *info = app->getGameInfo(_game);
		debugPrintf("%s: ", _game.c_str());
		if (info) {
			Std::string details = info->getPrintDetails();
			debugPrintf("%s\n", details.c_str());
		} else {
			debugPrintf("(unknown)\n");
		}
	}

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
	return true;
}

bool Debugger::cmdCloseItemGumps(int argc, const char **argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->getDesktopGump()->CloseItemDependents();
	return false;
}

bool Debugger::cmdMemberVar(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Usage: Ultima8Engine::memberVar <member> [newvalue] [updateini]\n");
		return true;
	}

	Ultima8Engine *g = Ultima8Engine::get_instance();

	// Set the pointer to the correct type
	bool *b = nullptr;
	int *i = nullptr;
	Std::string *str = nullptr;
	istring *istr = nullptr;

	// ini entry name if supported
	const char *ini = nullptr;

	if (!scumm_stricmp(argv[1], "_frameLimit")) {
		b = &g->_frameLimit;
		ini = "frameLimit";
	} else if (!scumm_stricmp(argv[1], "_frameSkip")) {
		b = &g->_frameSkip;
		ini = "frameSkip";
	} else if (!scumm_stricmp(argv[1], "_interpolate")) {
		b = &g->_interpolate;
		ini = "interpolate";
	} else {
		debugPrintf("Unknown member: %s\n", argv[1]);
		return true;
	}

	// Set the value
	if (argc >= 3) {
		if (b)
			*b = !scumm_stricmp(argv[2], "yes") || !scumm_stricmp(argv[2], "true");
		else if (istr)
			*istr = argv[2];
		else if (i)
			*i = Std::strtol(argv[2], 0, 0);
		else if (str)
			*str = argv[2];

		// Set config value
		if (argc >= 4 && ini && *ini && (!scumm_stricmp(argv[3], "yes") || !scumm_stricmp(argv[3], "true"))) {
			if (b)
				g->_settingMan->set(ini, *b);
			else if (istr)
				g->_settingMan->set(ini, *istr);
			else if (i)
				g->_settingMan->set(ini, *i);
			else if (str)
				g->_settingMan->set(ini, *str);
		}
	}

	// Print the value
	debugPrintf("Ultima8Engine::%s = ", argv[1]);
	if (b)
		debugPrintf("%s", strBool(*b));
	else if (istr)
		debugPrintf("%s", istr->c_str());
	else if (i)
		debugPrintf("%d", *i);
	else if (str)
		debugPrintf("%s", str->c_str());
	debugPrintf("\n");

	return true;
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

	// constants!!
	mainActor->setStr(25);
	mainActor->setDex(25);
	mainActor->setInt(25);
	mainActor->setHP(mainActor->getMaxHP());
	mainActor->setMana(mainActor->getMaxMana());

	AudioProcess *audioproc = AudioProcess::get_instance();
	if (audioproc)
		audioproc->playSFX(0x36, 0x60, 1, 0); //constants!!
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


bool Debugger::cmdToggleHighlightItems(int argc, const char **argv) {
	GameMapGump::Set_highlightItems(!GameMapGump::is_highlightItems());
	return false;
}

bool Debugger::cmdDumpMap(int argc, const char **argv) {
	// Save because we're going to potentially break the game by enlarging
	// the fast area and available object IDs.
	int slot = Ultima8Engine::get_instance()->getAutosaveSlot();
	if (!Ultima8Engine::get_instance()->saveGame(slot, "Pre-dumpMap save")) {
		debugPrintf("Could not dump map: pre-dumpMap save failed\n");
		return false;
	}

	// Increase number of available object IDs.
	ObjectManager::get_instance()->allow64kObjects();

	// Actual size
	int32 awidth = 8192;
	int32 aheight = 8192;

	int32 xpos = 0;
	int32 ypos = 0;

	int32 left = 16384;
	int32 right = -16384;
	int32 top = 16384;
	int32 bot = -16384;

	int32 camheight = 256;

	// Work out the map limit we do this very coarsly
	// Now render the map
	for (int32 y = 0; y < 64; y++) {
		for (int32 x = 0; x < 64; x++) {
			const Std::list<Item *> *list =
				World::get_instance()->getCurrentMap()->getItemList(x, y);

			// Should iterate the items!
			// (items could extend outside of this chunk and they have height)
			if (list && list->size() != 0) {
				int32 l = (x * 512 - y * 512) / 4 - 128;
				int32 r = (x * 512 - y * 512) / 4 + 128;
				int32 t = (x * 512 + y * 512) / 8 - 256;
				int32 b = (x * 512 + y * 512) / 8;

				t -= 256; // approx. adjustment for height of items in chunk

				if (l < left) left = l;
				if (r > right) right = r;
				if (t < top) top = t;
				if (b > bot) bot = b;
			}
		}
	}

	if (right == -16384) {
		return false;
	}

	// camera height
	bot += camheight;
	top += camheight;

	awidth = right - left;
	aheight = bot - top;

	ypos = top;
	xpos = left;

	// Buffer Size
	int32 bwidth = awidth;
	//int32 bheight = 256;

	// Original version of this command wrote the image out in rows to save memory
	// This version can only write out a full texture
	int32 bheight = aheight;

	// Tile size
	int32 twidth = bwidth / 8;
	int32 theight = 256;

	GameMapGump *g = new GameMapGump(0, 0, twidth, theight);

	// HACK: Setting both INVISIBLE and TRANSPARENT flags on the Avatar
	// will make him completely invisible.
	getMainActor()->setFlag(Item::FLG_INVISIBLE);
	getMainActor()->setExtFlag(Item::EXT_TRANSPARENT);

	CurrentMap *currentMap = World::get_instance()->getCurrentMap();
	currentMap->setWholeMapFast();

	RenderSurface *s = RenderSurface::CreateSecondaryRenderSurface(bwidth,
		bheight);

	debugPrintf("Rendering map...\n");

	// Now render the map
	for (int32 y = 0; y < aheight; y += theight) {
		for (int32 x = 0; x < awidth; x += twidth) {
			// Work out 'effective' and world coords
			int32 ex = xpos + x + twidth / 2;
			int32 ey = ypos + y + theight / 2;
			int32 wx = ex * 2 + ey * 4;
			int32 wy = ey * 4 - ex * 2;

			s->SetOrigin(x, y % bheight);
			CameraProcess::SetCameraProcess(new CameraProcess(wx + 4 * camheight, wy + 4 * camheight, camheight));
			g->Paint(s, 256, false);

		}

		// Write out the current buffer
		//if (((y + theight) % bheight) == 0) {
		//	for (int i = 0; i < bwidth * bheight; ++i) {
		//		// Convert to correct pixel format
		//		uint8 r, g, b;
		//		UNPACK_RGB8(t->buffer[i], r, g, b);
		//		uint8 *buf = reinterpret_cast<uint8 *>(&t->buffer[i]);
		//		buf[0] = b;
		//		buf[1] = g;
		//		buf[2] = r;
		//		buf[3] = 0xFF;
		//	}

		//	pngw->writeRows(bheight, t);

		//	// clear buffer for next set
		//	t->clear();
		//}
	}

#ifdef USE_PNG
	Std::string filename = Common::String::format("map_%02d.png", currentMap->getNum());
#else
	Std::string filename = Common::String::format("map_%02d.bmp", currentMap->getNum());
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
		debugPrintf("Map dumped: %s\n", filename.c_str());
	} else {
		debugPrintf("Could not write file: %s\n", filename.c_str());
	}

	delete g;
	delete s;

	// Reload
	Ultima8Engine::get_instance()->loadGameState(slot);
	return false;
}

bool Debugger::cmdIncrementSortOrder(int argc, const char **argv) {
	GameMapGump *gump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gump)
		gump->IncSortOrder(1);
	return false;
}

bool Debugger::cmdDecrementSortOrder(int argc, const char **argv) {
	GameMapGump *gump = Ultima8Engine::get_instance()->getGameMapGump();
	if (gump)
		gump->IncSortOrder(-1);
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
			if (argc == 1 || p->_itemNum == item)
				p->dumpInfo();
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
			p->dumpInfo();
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

	SettingManager *settings = SettingManager::get_instance();
	MainActor *mainActor = getMainActor();
	int curmap = mainActor->getMapNum();
	int32 x, y, z;
	mainActor->getLocation(x, y, z);

	istring confkey = Common::String::format("marks/%s", argv[1]);
	char buf[100]; // large enough for 4 ints
	sprintf(buf, "%d %d %d %d", curmap, x, y, z);

	settings->set(confkey, buf);
	settings->write(); //!! FIXME: clean this up

	debugPrintf("Set mark \"%s\" to %s\n", argv[1], buf);
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

	SettingManager *settings = SettingManager::get_instance();
	MainActor *mainActor = getMainActor();
	Common::String confKey = Common::String::format("marks/%s", argv[1]);
	Std::string target;
	if (!settings->get(confKey, target)) {
		debugPrintf("recall: no such mark\n");
		return true;
	}

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
	SettingManager *settings = SettingManager::get_instance();
	Std::vector<istring> marks;
	marks = settings->listDataKeys("marks");
	for (Std::vector<istring>::const_iterator iter = marks.begin();
		iter != marks.end(); ++iter) {
		debugPrintf("%s\n", iter->c_str());
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
		debugPrintf("Can't use backpack: avatarInStasis\n");
		return false;
	}
	MainActor *av = getMainActor();
	Item *backpack = getItem(av->getEquip(7));
	if (backpack)
		backpack->callUsecodeEvent_use();
	return false;
}

bool Debugger::cmdNextInventory(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use inventory: avatarInStasis\n");
		return false;
	}
	MainActor *av = getMainActor();
	av->nextInvItem();
	return false;
}

bool Debugger::cmdNextWeapon(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't change weapon: avatarInStasis\n");
		return false;
	}
	MainActor *av = getMainActor();
	av->nextWeapon();
	return false;
}

bool Debugger::cmdUseInventoryItem(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use active inventory item: avatarInStasis\n");
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
		debugPrintf("Can't use medikit: avatarInStasis\n");
		return false;
	}
	MainActor *av = getMainActor();
	av->useInventoryItem(0x351);
	return false;
}

bool Debugger::cmdDetonateBomb(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't detonate bomb: avatarInStasis\n");
		return false;
	}
	MainActor *av = getMainActor();
	av->detonateBomb();
	return false;
}

bool Debugger::cmdUseInventory(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use inventory: avatarInStasis\n");
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

bool Debugger::cmdAttack(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't attack: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->tryAttack();
	}
	return false;
}

bool Debugger::cmdCameraOnAvatar(int argc, const char **argv) {
	MainActor *actor = getMainActor();
	if (actor) {
		int32 x, y, z;
		actor->getLocation(x, y, z);
		CameraProcess::SetCameraProcess(new CameraProcess(x, y, z));
	}
	return false;
}

bool Debugger::cmdStartJump(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't jump: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_JUMP);
	}
	return false;
}


bool Debugger::cmdStopJump(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_JUMP);
	}
	return false;
}

bool Debugger::cmdStartTurnLeft(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't turn left: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_TURN_LEFT);
	}
	return false;
}

bool Debugger::cmdStartTurnRight(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't turn right: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_TURN_RIGHT);
	}
	return false;
}

bool Debugger::cmdStartMoveForward(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't move forward: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_FORWARD);
	}
	return false;
}

bool Debugger::cmdStartMoveBack(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't move back: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_BACK);
	}
	return false;
}

bool Debugger::cmdStartMoveLeft(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't move left: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_LEFT);
	}
	return false;
}

bool Debugger::cmdStartMoveRight(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't move right: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_RIGHT);
	}
	return false;
}

bool Debugger::cmdStartMoveUp(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't move up: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_UP);
	}
	return false;
}

bool Debugger::cmdStartMoveDown(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't move down: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_DOWN);
	}
	return false;
}

bool Debugger::cmdStopTurnLeft(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_TURN_LEFT);
	}
	return false;
}

bool Debugger::cmdStopTurnRight(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_TURN_RIGHT);
	}
	return false;
}

bool Debugger::cmdStopMoveForward(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_FORWARD);
	}
	return false;
}

bool Debugger::cmdStopMoveBack(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		// Clear both back and forward as avatar turns then moves forward when not in combat 
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_BACK | AvatarMoverProcess::MOVE_FORWARD);
	}
	return false;
}

bool Debugger::cmdStopMoveLeft(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_LEFT);
	}
	return false;
}

bool Debugger::cmdStopMoveRight(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_RIGHT);
	}
	return false;
}

bool Debugger::cmdStopMoveUp(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_UP);
	}
	return false;
}

bool Debugger::cmdStopMoveDown(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_DOWN);
	}
	return false;
}

bool Debugger::cmdStartMoveRun(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't run: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_RUN);
	}
	return false;
}

bool Debugger::cmdStopMoveRun(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_RUN);
	}
	return false;
}

bool Debugger::cmdStartMoveStep(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();
	if (engine->isAvatarInStasis()) {
		debugPrintf("Can't step: avatarInStasis\n");
		return false;
	}
	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();

	if (proc) {
		proc->setMovementFlag(AvatarMoverProcess::MOVE_STEP);
	}
	return false;
}

bool Debugger::cmdStopMoveStep(int argc, const char **argv) {
	Ultima8Engine *engine = Ultima8Engine::get_instance();
	engine->moveKeyEvent();

	AvatarMoverProcess *proc = engine->getAvatarMoverProcess();
	if (proc) {
		proc->clearMovementFlag(AvatarMoverProcess::MOVE_STEP);
	}
	return false;
}

bool Debugger::cmdToggleCombat(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't toggle combat: avatarInStasis\n");
		return false;
	}

	MainActor *av = getMainActor();
	av->toggleInCombat();
	return false;
}

bool Debugger::cmdStartSelection(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't select items: avatarInStasis\n");
		return false;
	}

	ItemSelectionProcess *proc = ItemSelectionProcess::get_instance();
	if (proc)
		proc->selectNextItem();
	return false;
}

bool Debugger::cmdUseSelection(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->isAvatarInStasis()) {
		debugPrintf("Can't use items: avatarInStasis\n");
		return false;
	}

	ItemSelectionProcess *proc = ItemSelectionProcess::get_instance();
	if (proc)
		proc->useSelectedItem();
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
			obj->dumpInfo();
		}
	}

	return true;
}

bool Debugger::cmdStartQuickMoveUp(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(-64, -64, 0, 0);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartQuickMoveDown(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(+64, +64, 0, 1);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartQuickMoveLeft(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(-64, +64, 0, 2);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartQuickMoveRight(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(+64, -64, 0, 3);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartQuickMoveAscend(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(0, 0, 8, 4);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStartQuickMoveDescend(int argc, const char **argv) {
	if (Ultima8Engine::get_instance()->areCheatsEnabled()) {
		QuickAvatarMoverProcess::startMover(0, 0, -8, 5);
		return false;
	} else {
		debugPrintf("Cheats aren't enabled\n");
		return true;
	}
}

bool Debugger::cmdStopQuickMoveUp(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(0);
	return false;
}

bool Debugger::cmdStopQuickMoveDown(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(1);
	return false;
}

bool Debugger::cmdStopQuickMoveLeft(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(2);
	return false;
}

bool Debugger::cmdStopQuickMoveRight(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(3);
	return false;
}

bool Debugger::cmdStopQuickMoveAscend(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(4);
	return false;
}

bool Debugger::cmdStopQuickMoveDescend(int argc, const char **argv) {
	QuickAvatarMoverProcess::terminateMover(5);
	return false;
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

#ifdef DEBUG

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

bool Debugger::cmdTraceEvents(int argc, const char **argv) {
	UCMachine *uc = UCMachine::get_instance();
	uc->_tracingEnabled = true;
	uc->_traceEvents = true;

	debugPrintf("UCMachine: tracing usecode events\n");
	return true;
}

bool Debugger::cmdStopTrace(int argc, const char **argv) {
	UCMachine *uc = UCMachine::get_instance();
	uc->_traceObjIDs.clear();
	uc->_tracePIDs.clear();
	uc->_traceClasses.clear();
	uc->_tracingEnabled = false;
	uc->_traceAll = false;
	uc->_traceEvents = false;

	debugPrintf("Trace stopped\n");
	return true;
}

#endif


bool Debugger::cmdVerifyQuit(int argc, const char **argv) {
	QuitGump::verifyQuit();
	return false;
}

bool Debugger::cmdU8ShapeViewer(int argc, const char **argv) {
	ShapeViewerGump::U8ShapeViewer();
	return false;
}

bool Debugger::cmdShowMenu(int argc, const char **argv) {
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

	Std::string filename = Common::String::format("@game/static/%s.skf", argv[1]);
	FileSystem *filesys = FileSystem::get_instance();
	Common::SeekableReadStream *skf = filesys->ReadFile(filename);
	if (!skf) {
		debugPrintf("movie not found.\n");
		return true;
	}

	MovieGump::U8MovieViewer(skf, false);
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
	} else {
		mmg->Close();
	}

	return false;
}

bool Debugger::cmdGenerateWholeMap(int argc, const char **argv) {
	World *world = World::get_instance();
	CurrentMap *currentmap = world->getCurrentMap();
	currentmap->setWholeMapFast();
	return false;
}

#ifdef DEBUG
bool Debugger::cmdVisualDebugPathfinder(int argc, const char **argv) {
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

	return true;
}
#endif

} // End of namespace Ultima8
} // End of namespace Ultima
