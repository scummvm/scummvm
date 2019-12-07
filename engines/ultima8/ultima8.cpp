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
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/translation.h"
#include "common/unzip.h"
#include "ultima8/ultima8.h"
#include "ultima8/detection.h"
#include "ultima8/ultima8.h"
#include "ultima8/misc/pent_include.h"
#include "ultima8/ultima8.h"

 //!! a lot of these includes are just for some hacks... clean up sometime
#include "ultima8/kernel/kernel.h"
#include "ultima8/filesys/file_system.h"
#include "ultima8/conf/setting_manager.h"
#include "ultima8/conf/config_file_manager.h"
#include "ultima8/kernel/object_manager.h"
#include "ultima8/games/game_info.h"
#include "ultima8/graphics/fonts/font_manager.h"
#include "ultima8/kernel/memory_manager.h"
#include "ultima8/kernel/hid_manager.h"
#include "ultima8/kernel/joystick.h"

#include "ultima8/graphics/render_surface.h"
#include "ultima8/graphics/texture.h"
#include "ultima8/graphics/fonts/fixed_width_font.h"
#include "ultima8/graphics/palette_manager.h"
#include "ultima8/graphics/palette.h"
#include "ultima8/games/game_data.h"
#include "ultima8/world/world.h"
#include "ultima8/misc/direction.h"
#include "ultima8/games/game.h"
#include "ultima8/world/get_object.h"

#include "ultima8/filesys/savegame_writer.h"
#include "ultima8/filesys/savegame.h"

#include "ultima8/gumps/gump.h"
#include "ultima8/gumps/desktop_gump.h"
#include "ultima8/gumps/console_gump.h"
#include "ultima8/gumps/game_map_gump.h"
#include "ultima8/gumps/inverter_gump.h"
#include "ultima8/gumps/scaler_gump.h"
#include "ultima8/gumps/fast_area_vis_gump.h"
#include "ultima8/gumps/minimap_gump.h"
#include "ultima8/gumps/quit_gump.h"
#include "ultima8/gumps/menu_gump.h"
#include "ultima8/gumps/pentagram_menu_gump.h"

// For gump positioning... perhaps shouldn't do it this way....
#include "ultima8/gumps/bark_gump.h"
#include "ultima8/gumps/ask_gump.h"
#include "ultima8/gumps/modal_gump.h"
#include "ultima8/gumps/message_box_gump.h"


#include "ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima8/world/actors/actor.h"
#include "ultima8/world/actors/actor_anim_process.h"
#include "ultima8/world/actors/targeted_anim_process.h"
#include "ultima8/usecode/u8_intrinsics.h"
#include "ultima8/usecode/remorse_intrinsics.h"
#include "ultima8/world/egg.h"
#include "ultima8/world/current_map.h"
#include "ultima8/graphics/inverter_process.h"
#include "ultima8/world/actors/heal_process.h"
#include "ultima8/world/actors/scheduler_process.h"

#include "ultima8/world/egg_hatcher_process.h" // for a hack
#include "ultima8/usecode/uc_process.h" // more hacking
#include "ultima8/gumps/gump_notify_process.h" // guess
#include "ultima8/world/actors/actor_bark_notify_process.h" // guess
#include "ultima8/kernel/delay_process.h"
#include "ultima8/world/actors/avatar_gravity_process.h"
#include "ultima8/world/missile_process.h"
#include "ultima8/world/actors/teleport_to_egg_process.h"
#include "ultima8/world/item_factory.h"
#include "ultima8/world/actors/pathfinder_process.h"
#include "ultima8/world/actors/avatar_mover_process.h"
#include "ultima8/world/actors/resurrection_process.h"
#include "ultima8/world/split_item_process.h"
#include "ultima8/world/actors/clear_feign_death_process.h"
#include "ultima8/world/actors/loiter_process.h"
#include "ultima8/world/actors/avatar_death_process.h"
#include "ultima8/world/actors/grant_peace_process.h"
#include "ultima8/world/actors/combat_process.h"
#include "ultima8/world/fireball_process.h"
#include "ultima8/world/destroy_item_process.h"
#include "ultima8/world/actors/ambush_process.h"
#include "ultima8/world/actors/pathfinder.h"

#include "ultima8/gumps/movie_gump.h"
#include "ultima8/gumps/shape_viewer_gump.h"
#include "ultima8/audio/audio_mixer.h"
#include "ultima8/graphics/xform_blend.h"
#include "ultima8/audio/music_process.h"
#include "ultima8/audio/audio_process.h"

#include "ultima8/misc/util.h"

namespace Ultima8 {

using std::string;

DEFINE_RUNTIME_CLASSTYPE_CODE(Ultima8Engine, CoreApp)

#define DATA_FILENAME "ultima.dat"
#define DATA_VERSION_MAJOR 1
#define DATA_VERSION_MINOR 0

Ultima8Engine::Ultima8Engine(OSystem *syst, const Ultima8GameDescription *gameDesc) : 
		Engine(syst), CoreApp(0, nullptr), _gameDescription(gameDesc),
		_randomSource("Ultima8"), save_count(0), game(nullptr), kernel(nullptr),
		_dataArchive(nullptr), objectmanager(nullptr), hidmanager(nullptr), _mouse(nullptr),
		ucmachine(nullptr), screen(nullptr), fontmanager(nullptr), fullscreen(false),
		palettemanager(nullptr), gamedata(nullptr), world(nullptr), desktopGump(nullptr),
		consoleGump(nullptr), gameMapGump(nullptr), avatarMoverProcess(nullptr),
		runSDLInit(false), frameSkip(false), frameLimit(true), interpolate(true),
		animationRate(100), avatarInStasis(false), paintEditorItems(false), inversion(0),
		painting(false), showTouching(false), timeOffset(0), has_cheated(false),
		cheats_enabled(false), drawRenderStats(false), ttfoverrides(false), audiomixer(0) {
	application = this;

	for (uint16 key = 0; key < HID_LAST; ++key) {
		_lastDown[key] = false;
		_down[key] = false;
	}
}

Ultima8Engine::~Ultima8Engine() {
	FORGET_OBJECT(kernel);
	FORGET_OBJECT(objectmanager);
	FORGET_OBJECT(hidmanager);
	FORGET_OBJECT(audiomixer);
	FORGET_OBJECT(ucmachine);
	FORGET_OBJECT(palettemanager);
	FORGET_OBJECT(_mouse);
	FORGET_OBJECT(gamedata);
	FORGET_OBJECT(world);
	FORGET_OBJECT(ucmachine);
	FORGET_OBJECT(fontmanager);
	FORGET_OBJECT(screen);
}

uint32 Ultima8Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Error Ultima8Engine::run() {
	if (initialize()) {
		startup();

		runGame();

		deinitialize();
		shutdown();
	}

	return Common::kNoError;
}


bool Ultima8Engine::initialize() {
	if (!loadData())
		return false;

	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");

	con.AddConsoleCommand("quit", ConCmd_quit);
	con.AddConsoleCommand("Ultima8Engine::quit", ConCmd_quit);
	con.AddConsoleCommand("QuitGump::verifyQuit", QuitGump::ConCmd_verifyQuit);
	con.AddConsoleCommand("ShapeViewerGump::U8ShapeViewer", ShapeViewerGump::ConCmd_U8ShapeViewer);
	con.AddConsoleCommand("MenuGump::showMenu", MenuGump::ConCmd_showMenu);
	con.AddConsoleCommand("Ultima8Engine::drawRenderStats", ConCmd_drawRenderStats);
	con.AddConsoleCommand("Ultima8Engine::engineStats", ConCmd_engineStats);

	con.AddConsoleCommand("Ultima8Engine::changeGame", ConCmd_changeGame);
	con.AddConsoleCommand("Ultima8Engine::listGames", ConCmd_listGames);

	con.AddConsoleCommand("Ultima8Engine::memberVar", &Ultima8Engine::ConCmd_memberVar);
	con.AddConsoleCommand("Ultima8Engine::setVideoMode", ConCmd_setVideoMode);
	con.AddConsoleCommand("Ultima8Engine::toggleFullscreen", ConCmd_toggleFullscreen);

	con.AddConsoleCommand("Ultima8Engine::toggleAvatarInStasis", ConCmd_toggleAvatarInStasis);
	con.AddConsoleCommand("Ultima8Engine::togglePaintEditorItems", ConCmd_togglePaintEditorItems);
	con.AddConsoleCommand("Ultima8Engine::toggleShowTouchingItems", ConCmd_toggleShowTouchingItems);

	con.AddConsoleCommand("Ultima8Engine::closeItemGumps", ConCmd_closeItemGumps);

	con.AddConsoleCommand("HIDManager::bind", HIDManager::ConCmd_bind);
	con.AddConsoleCommand("HIDManager::unbind", HIDManager::ConCmd_unbind);
	con.AddConsoleCommand("HIDManager::listbinds",
		HIDManager::ConCmd_listbinds);
	con.AddConsoleCommand("HIDManager::save", HIDManager::ConCmd_save);
	con.AddConsoleCommand("Kernel::processTypes", Kernel::ConCmd_processTypes);
	con.AddConsoleCommand("Kernel::processInfo", Kernel::ConCmd_processInfo);
	con.AddConsoleCommand("Kernel::listProcesses",
		Kernel::ConCmd_listProcesses);
	con.AddConsoleCommand("Kernel::toggleFrameByFrame",
		Kernel::ConCmd_toggleFrameByFrame);
	con.AddConsoleCommand("Kernel::advanceFrame", Kernel::ConCmd_advanceFrame);
	con.AddConsoleCommand("ObjectManager::objectTypes",
		ObjectManager::ConCmd_objectTypes);
	con.AddConsoleCommand("ObjectManager::objectInfo",
		ObjectManager::ConCmd_objectInfo);
	con.AddConsoleCommand("MemoryManager::MemInfo",
		MemoryManager::ConCmd_MemInfo);
	con.AddConsoleCommand("MemoryManager::test",
		MemoryManager::ConCmd_test);

	con.AddConsoleCommand("QuickAvatarMoverProcess::startMoveUp",
		QuickAvatarMoverProcess::ConCmd_startMoveUp);
	con.AddConsoleCommand("QuickAvatarMoverProcess::startMoveDown",
		QuickAvatarMoverProcess::ConCmd_startMoveDown);
	con.AddConsoleCommand("QuickAvatarMoverProcess::startMoveLeft",
		QuickAvatarMoverProcess::ConCmd_startMoveLeft);
	con.AddConsoleCommand("QuickAvatarMoverProcess::startMoveRight",
		QuickAvatarMoverProcess::ConCmd_startMoveRight);
	con.AddConsoleCommand("QuickAvatarMoverProcess::startAscend",
		QuickAvatarMoverProcess::ConCmd_startAscend);
	con.AddConsoleCommand("QuickAvatarMoverProcess::startDescend",
		QuickAvatarMoverProcess::ConCmd_startDescend);
	con.AddConsoleCommand("QuickAvatarMoverProcess::stopMoveUp",
		QuickAvatarMoverProcess::ConCmd_stopMoveUp);
	con.AddConsoleCommand("QuickAvatarMoverProcess::stopMoveDown",
		QuickAvatarMoverProcess::ConCmd_stopMoveDown);
	con.AddConsoleCommand("QuickAvatarMoverProcess::stopMoveLeft",
		QuickAvatarMoverProcess::ConCmd_stopMoveLeft);
	con.AddConsoleCommand("QuickAvatarMoverProcess::stopMoveRight",
		QuickAvatarMoverProcess::ConCmd_stopMoveRight);
	con.AddConsoleCommand("QuickAvatarMoverProcess::stopAscend",
		QuickAvatarMoverProcess::ConCmd_stopAscend);
	con.AddConsoleCommand("QuickAvatarMoverProcess::stopDescend",
		QuickAvatarMoverProcess::ConCmd_stopDescend);
	con.AddConsoleCommand("QuickAvatarMoverProcess::toggleQuarterSpeed",
		QuickAvatarMoverProcess::ConCmd_toggleQuarterSpeed);
	con.AddConsoleCommand("QuickAvatarMoverProcess::toggleClipping",
		QuickAvatarMoverProcess::ConCmd_toggleClipping);

	con.AddConsoleCommand("GameMapGump::toggleHighlightItems",
		GameMapGump::ConCmd_toggleHighlightItems);
	con.AddConsoleCommand("GameMapGump::dumpMap",
		GameMapGump::ConCmd_dumpMap);
	con.AddConsoleCommand("GameMapGump::incrementSortOrder",
		GameMapGump::ConCmd_incrementSortOrder);
	con.AddConsoleCommand("GameMapGump::decrementSortOrder",
		GameMapGump::ConCmd_decrementSortOrder);

	con.AddConsoleCommand("AudioProcess::listSFX", AudioProcess::ConCmd_listSFX);
	con.AddConsoleCommand("AudioProcess::playSFX", AudioProcess::ConCmd_playSFX);
	con.AddConsoleCommand("AudioProcess::stopSFX", AudioProcess::ConCmd_stopSFX);

	return true;
}

bool Ultima8Engine::loadData() {
	Common::File f;

	if (!Common::File::exists(DATA_FILENAME) ||
			(_dataArchive = Common::makeZipArchive(DATA_FILENAME)) == nullptr ||
			!f.open("ultima8/version.txt", *_dataArchive)) {
		delete _dataArchive;
		GUIError(Common::String::format(_("Could not locate engine data %s"), DATA_FILENAME));
		return false;
	}

	// Validate the version
	char buffer[5];
	f.read(buffer, 4);
	buffer[4] = '\0';

	int major = 0, minor = 0;
	if (buffer[1] == '.') {
		major = buffer[0] - '0';
		minor = atoi(&buffer[2]);
	}

	if (major != DATA_VERSION_MAJOR || minor != DATA_VERSION_MINOR) {
		delete _dataArchive;
		GUIError(Common::String::format(_("Out of date engine data. Expected %d.%d, but got version %d.%d"),
			DATA_VERSION_MAJOR, DATA_VERSION_MINOR, major, minor));
		return false;
	}

	SearchMan.add("data", _dataArchive);
	return true;
}

void Ultima8Engine::deinitialize() {
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_quit);
	con.RemoveConsoleCommand(QuitGump::ConCmd_verifyQuit);
	con.RemoveConsoleCommand(ShapeViewerGump::ConCmd_U8ShapeViewer);
	con.RemoveConsoleCommand(MenuGump::ConCmd_showMenu);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_drawRenderStats);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_engineStats);

	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_changeGame);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_listGames);

	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_memberVar);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_setVideoMode);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_toggleFullscreen);

	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_toggleAvatarInStasis);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_togglePaintEditorItems);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_toggleShowTouchingItems);

	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_closeItemGumps);

	con.RemoveConsoleCommand(HIDManager::ConCmd_bind);
	con.RemoveConsoleCommand(HIDManager::ConCmd_unbind);
	con.RemoveConsoleCommand(HIDManager::ConCmd_listbinds);
	con.RemoveConsoleCommand(HIDManager::ConCmd_save);
	con.RemoveConsoleCommand(Kernel::ConCmd_processTypes);
	con.RemoveConsoleCommand(Kernel::ConCmd_processInfo);
	con.RemoveConsoleCommand(Kernel::ConCmd_listProcesses);
	con.RemoveConsoleCommand(Kernel::ConCmd_toggleFrameByFrame);
	con.RemoveConsoleCommand(Kernel::ConCmd_advanceFrame);
	con.RemoveConsoleCommand(ObjectManager::ConCmd_objectTypes);
	con.RemoveConsoleCommand(ObjectManager::ConCmd_objectInfo);
	con.RemoveConsoleCommand(MemoryManager::ConCmd_MemInfo);
	con.RemoveConsoleCommand(MemoryManager::ConCmd_test);

	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startMoveUp);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startMoveDown);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startMoveLeft);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startMoveRight);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startAscend);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startDescend);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopMoveUp);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopMoveDown);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopMoveLeft);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopMoveRight);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopAscend);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopDescend);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_toggleQuarterSpeed);
	con.RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_toggleClipping);

	con.RemoveConsoleCommand(GameMapGump::ConCmd_toggleHighlightItems);
	con.RemoveConsoleCommand(GameMapGump::ConCmd_dumpMap);
	con.RemoveConsoleCommand(GameMapGump::ConCmd_incrementSortOrder);
	con.RemoveConsoleCommand(GameMapGump::ConCmd_decrementSortOrder);

	con.RemoveConsoleCommand(AudioProcess::ConCmd_listSFX);
	con.RemoveConsoleCommand(AudioProcess::ConCmd_stopSFX);
	con.RemoveConsoleCommand(AudioProcess::ConCmd_playSFX);
}

Common::FSNode Ultima8Engine::getGameDirectory() const {
	return Common::FSNode(ConfMan.get("path"));
}

void Ultima8Engine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

void Ultima8Engine::startup() {
	// Set the console to auto paint, till we have finished initing
	con.SetAutoPaint(conAutoPaint);

	pout << "-- Initializing Pentagram -- " << std::endl;

	// parent's startup first
	CoreApp::startup();

	bool dataoverride;
	if (!settingman->get("dataoverride", dataoverride,
		SettingManager::DOM_GLOBAL))
		dataoverride = false;
	filesystem->initBuiltinData(dataoverride);

	kernel = new Kernel();

	//!! move this elsewhere
	kernel->addProcessLoader("DelayProcess",
		ProcessLoader<DelayProcess>::load);
	kernel->addProcessLoader("GravityProcess",
		ProcessLoader<GravityProcess>::load);
	kernel->addProcessLoader("AvatarGravityProcess",
		ProcessLoader<AvatarGravityProcess>::load);
	kernel->addProcessLoader("PaletteFaderProcess",
		ProcessLoader<PaletteFaderProcess>::load);
	kernel->addProcessLoader("TeleportToEggProcess",
		ProcessLoader<TeleportToEggProcess>::load);
	kernel->addProcessLoader("ActorAnimProcess",
		ProcessLoader<ActorAnimProcess>::load);
	kernel->addProcessLoader("TargetedAnimProcess",
		ProcessLoader<TargetedAnimProcess>::load);
	kernel->addProcessLoader("AvatarMoverProcess",
		ProcessLoader<AvatarMoverProcess>::load);
	kernel->addProcessLoader("QuickAvatarMoverProcess",
		ProcessLoader<QuickAvatarMoverProcess>::load);
	kernel->addProcessLoader("PathfinderProcess",
		ProcessLoader<PathfinderProcess>::load);
	kernel->addProcessLoader("SpriteProcess",
		ProcessLoader<SpriteProcess>::load);
	kernel->addProcessLoader("MissileProcess",
		ProcessLoader<MissileProcess>::load);
	kernel->addProcessLoader("CameraProcess",
		ProcessLoader<CameraProcess>::load);
	kernel->addProcessLoader("MusicProcess",
		ProcessLoader<MusicProcess>::load);
	kernel->addProcessLoader("AudioProcess",
		ProcessLoader<AudioProcess>::load);
	kernel->addProcessLoader("EggHatcherProcess",
		ProcessLoader<EggHatcherProcess>::load);
	kernel->addProcessLoader("UCProcess",
		ProcessLoader<UCProcess>::load);
	kernel->addProcessLoader("GumpNotifyProcess",
		ProcessLoader<GumpNotifyProcess>::load);
	kernel->addProcessLoader("ResurrectionProcess",
		ProcessLoader<ResurrectionProcess>::load);
	kernel->addProcessLoader("DeleteActorProcess",
		ProcessLoader<DestroyItemProcess>::load);  // YES, this is intentional
	kernel->addProcessLoader("DestroyItemProcess",
		ProcessLoader<DestroyItemProcess>::load);
	kernel->addProcessLoader("SplitItemProcess",
		ProcessLoader<SplitItemProcess>::load);
	kernel->addProcessLoader("ClearFeignDeathProcess",
		ProcessLoader<ClearFeignDeathProcess>::load);
	kernel->addProcessLoader("LoiterProcess",
		ProcessLoader<LoiterProcess>::load);
	kernel->addProcessLoader("AvatarDeathProcess",
		ProcessLoader<AvatarDeathProcess>::load);
	kernel->addProcessLoader("GrantPeaceProcess",
		ProcessLoader<GrantPeaceProcess>::load);
	kernel->addProcessLoader("CombatProcess",
		ProcessLoader<CombatProcess>::load);
	kernel->addProcessLoader("FireballProcess",
		ProcessLoader<FireballProcess>::load);
	kernel->addProcessLoader("HealProcess",
		ProcessLoader<HealProcess>::load);
	kernel->addProcessLoader("SchedulerProcess",
		ProcessLoader<SchedulerProcess>::load);
	kernel->addProcessLoader("InverterProcess",
		ProcessLoader<InverterProcess>::load);
	kernel->addProcessLoader("ActorBarkNotifyProcess",
		ProcessLoader<ActorBarkNotifyProcess>::load);
	kernel->addProcessLoader("JoystickCursorProcess",
		ProcessLoader<JoystickCursorProcess>::load);
	kernel->addProcessLoader("AmbushProcess",
		ProcessLoader<AmbushProcess>::load);

	objectmanager = new ObjectManager();
	_mouse = new Mouse();

	GraphicSysInit();

#ifdef TODO
	SDL_ShowCursor(SDL_DISABLE);
	SDL_GetMouseState(&mouseX, &mouseY);
#endif
	hidmanager = new HIDManager();

	// Audio Mixer
	audiomixer = new Pentagram::AudioMixer(22050, true, 8);

	pout << "-- Pentagram Initialized -- " << std::endl << std::endl;

	// We Attempt to startup game
	setupGameList();
	GameInfo *info = getDefaultGame();
	if (setupGame(info))
		startupGame();
	else
		startupPentagramMenu();

	// Unset the console auto paint, since we have finished initing
	con.SetAutoPaint(0);

	//	pout << "Paint Initial display" << std::endl;
	paint();
}

void Ultima8Engine::startupGame() {
	con.SetAutoPaint(conAutoPaint);

	pout  << std::endl << "-- Initializing Game: " << gameinfo->name << " --" << std::endl;

	GraphicSysInit();

	// Generic Commands
	con.AddConsoleCommand("Ultima8Engine::saveGame", ConCmd_saveGame);
	con.AddConsoleCommand("Ultima8Engine::loadGame", ConCmd_loadGame);
	con.AddConsoleCommand("Ultima8Engine::newGame", ConCmd_newGame);
#ifdef DEBUG
	con.AddConsoleCommand("Pathfinder::visualDebug",
		Pathfinder::ConCmd_visualDebug);
#endif

	// U8 Game commands
	con.AddConsoleCommand("MainActor::teleport", MainActor::ConCmd_teleport);
	con.AddConsoleCommand("MainActor::mark", MainActor::ConCmd_mark);
	con.AddConsoleCommand("MainActor::recall", MainActor::ConCmd_recall);
	con.AddConsoleCommand("MainActor::listmarks", MainActor::ConCmd_listmarks);
	con.AddConsoleCommand("Cheat::maxstats", MainActor::ConCmd_maxstats);
	con.AddConsoleCommand("Cheat::heal", MainActor::ConCmd_heal);
	con.AddConsoleCommand("Cheat::toggleInvincibility", MainActor::ConCmd_toggleInvincibility);
	con.AddConsoleCommand("Cheat::toggle", Ultima8Engine::ConCmd_toggleCheatMode);
	con.AddConsoleCommand("MainActor::name", MainActor::ConCmd_name);
	con.AddConsoleCommand("MovieGump::play", MovieGump::ConCmd_play);
	con.AddConsoleCommand("MusicProcess::playMusic", MusicProcess::ConCmd_playMusic);
	con.AddConsoleCommand("InverterProcess::invertScreen",
		InverterProcess::ConCmd_invertScreen);
	con.AddConsoleCommand("FastAreaVisGump::toggle",
		FastAreaVisGump::ConCmd_toggle);
	con.AddConsoleCommand("MiniMapGump::toggle",
		MiniMapGump::ConCmd_toggle);
	con.AddConsoleCommand("MainActor::useBackpack",
		MainActor::ConCmd_useBackpack);
	con.AddConsoleCommand("MainActor::useInventory",
		MainActor::ConCmd_useInventory);
	con.AddConsoleCommand("MainActor::useRecall",
		MainActor::ConCmd_useRecall);
	con.AddConsoleCommand("MainActor::useBedroll",
		MainActor::ConCmd_useBedroll);
	con.AddConsoleCommand("MainActor::useKeyring",
		MainActor::ConCmd_useKeyring);
	con.AddConsoleCommand("MainActor::toggleCombat",
		MainActor::ConCmd_toggleCombat);

	gamedata = new GameData(gameinfo);

	std::string bindingsfile;
	if (GAME_IS_U8) {
		bindingsfile = "@data/u8bindings.ini";
	} else if (GAME_IS_REMORSE) {
		bindingsfile = "@data/remorsebindings.ini";
	}
	if (!bindingsfile.empty()) {
		// system-wide config
		if (configfileman->readConfigFile(bindingsfile,
			"bindings", true))
			con.Printf(MM_INFO, "%s... Ok\n", bindingsfile.c_str());
		else
			con.Printf(MM_MINOR_WARN, "%s... Failed\n", bindingsfile.c_str());
	}

	hidmanager->loadBindings();

	if (GAME_IS_U8) {
		ucmachine = new UCMachine(U8Intrinsics, 256);
	} else if (GAME_IS_REMORSE) {
		ucmachine = new UCMachine(RemorseIntrinsics, 308);
	} else {
		CANT_HAPPEN_MSG("Invalid game type.");
	}

	inBetweenFrame = 0;
	lerpFactor = 256;

	// Initialize world
	world = new World();
	world->initMaps();

	game = Game::createGame(getGameInfo());

	settingman->setDefault("ttf", false);
	settingman->get("ttf", ttfoverrides);

	settingman->setDefault("frameSkip", false);
	settingman->get("frameSkip", frameSkip);

	settingman->setDefault("frameLimit", true);
	settingman->get("frameLimit", frameLimit);

	settingman->setDefault("interpolate", true);
	settingman->get("interpolate", interpolate);

	settingman->setDefault("cheat", false);
	settingman->get("cheat", cheats_enabled);

	game->loadFiles();
	gamedata->setupFontOverrides();

	// Unset the console auto paint (can't have it from here on)
	con.SetAutoPaint(0);

	// Create Midi Driver for Ultima 8
	if (getGameInfo()->type == GameInfo::GAME_U8)
		audiomixer->openMidiOutput();

	std::string savegame;
	settingman->setDefault("lastSave", "");
	settingman->get("lastSave", savegame);

	newGame(savegame);

	consoleGump->HideConsole();

	pout << "-- Game Initialized --" << std::endl << std::endl;
}

void Ultima8Engine::startupPentagramMenu() {
	con.SetAutoPaint(conAutoPaint);

	pout << std::endl << "-- Initializing Pentagram Menu -- " << std::endl;

	setupGame(getGameInfo("pentagram"));
	assert(gameinfo);

	GraphicSysInit();

	// Unset the console auto paint, since we have finished initing
	con.SetAutoPaint(0);
	consoleGump->HideConsole();

	Pentagram::Rect dims;
	desktopGump->GetDims(dims);

	Gump *menugump = new PentagramMenuGump(0, 0, dims.w, dims.h);
	menugump->InitGump(0, true);
}

void Ultima8Engine::shutdown() {
	shutdownGame(false);
}

void Ultima8Engine::shutdownGame(bool reloading) {
	pout << "-- Shutting down Game -- " << std::endl;

	// Save config here....

	textmodes.clear();

	// reset mouse cursor
	_mouse->popAllCursors();
	_mouse->pushMouseCursor();

	if (audiomixer) {
		audiomixer->closeMidiOutput();
		audiomixer->reset();
	}

	FORGET_OBJECT(world);
	objectmanager->reset();
	FORGET_OBJECT(ucmachine);
	kernel->reset();
	palettemanager->reset();
	fontmanager->resetGameFonts();

	FORGET_OBJECT(game);
	FORGET_OBJECT(gamedata);

	desktopGump = 0;
	consoleGump = 0;
	gameMapGump = 0;
	scalerGump = 0;
	inverterGump = 0;

	timeOffset = -(int32)Kernel::get_instance()->getFrameNum();
	save_count = 0;
	has_cheated = false;

	// Generic Game
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_saveGame);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_loadGame);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_newGame);
#ifdef DEBUG
	con.RemoveConsoleCommand(Pathfinder::ConCmd_visualDebug);
#endif

	// U8 Only kind of
	con.RemoveConsoleCommand(MainActor::ConCmd_teleport);
	con.RemoveConsoleCommand(MainActor::ConCmd_mark);
	con.RemoveConsoleCommand(MainActor::ConCmd_recall);
	con.RemoveConsoleCommand(MainActor::ConCmd_listmarks);
	con.RemoveConsoleCommand(MainActor::ConCmd_maxstats);
	con.RemoveConsoleCommand(MainActor::ConCmd_heal);
	con.RemoveConsoleCommand(MainActor::ConCmd_toggleInvincibility);
	con.RemoveConsoleCommand(Ultima8Engine::ConCmd_toggleCheatMode);
	con.RemoveConsoleCommand(MainActor::ConCmd_name);
	con.RemoveConsoleCommand(MovieGump::ConCmd_play);
	con.RemoveConsoleCommand(MusicProcess::ConCmd_playMusic);
	con.RemoveConsoleCommand(InverterProcess::ConCmd_invertScreen);
	con.RemoveConsoleCommand(FastAreaVisGump::ConCmd_toggle);
	con.RemoveConsoleCommand(MiniMapGump::ConCmd_toggle);
	con.RemoveConsoleCommand(MainActor::ConCmd_useBackpack);
	con.RemoveConsoleCommand(MainActor::ConCmd_useInventory);
	con.RemoveConsoleCommand(MainActor::ConCmd_useRecall);
	con.RemoveConsoleCommand(MainActor::ConCmd_useBedroll);
	con.RemoveConsoleCommand(MainActor::ConCmd_useKeyring);
	con.RemoveConsoleCommand(MainActor::ConCmd_toggleCombat);

	// Kill Game
	CoreApp::killGame();

	pout << "-- Game Shutdown -- " << std::endl;

	if (reloading) {
		Pentagram::Rect dims;
		screen->GetSurfaceDims(dims);

		con.Print(MM_INFO, "Creating Desktop...\n");
		desktopGump = new DesktopGump(0, 0, dims.w, dims.h);
		desktopGump->InitGump(0);
		desktopGump->MakeFocus();

		con.Print(MM_INFO, "Creating ScalerGump...\n");
		scalerGump = new ScalerGump(0, 0, dims.w, dims.h);
		scalerGump->InitGump(0);

		Pentagram::Rect scaled_dims;
		scalerGump->GetDims(scaled_dims);

		con.Print(MM_INFO, "Creating Graphics Console...\n");
		consoleGump = new ConsoleGump(0, 0, dims.w, dims.h);
		consoleGump->InitGump(0);
		consoleGump->HideConsole();

		con.Print(MM_INFO, "Creating Inverter...\n");
		inverterGump = new InverterGump(0, 0, scaled_dims.w, scaled_dims.h);
		inverterGump->InitGump(0);

		enterTextMode(consoleGump);
	}
}

void Ultima8Engine::changeGame(Pentagram::istring newgame) {
	change_gamename = newgame;
}

void Ultima8Engine::menuInitMinimal(Pentagram::istring gamename) {
	// Only if in the pentagram menu
	if (gameinfo->name != "pentagram") return;
	GameInfo *info = getGameInfo(gamename);
	if (!info) info = getGameInfo("pentagram");
	assert(info);

	pout  << std::endl << "-- Loading minimal game data for: " << info->name << " --" << std::endl;

	FORGET_OBJECT(game);
	FORGET_OBJECT(gamedata);


	setupGamePaths(info);

	if (info->name == "pentagram") return;

	gamedata = new GameData(info);
	game = Game::createGame(info);

	game->loadFiles();
	gamedata->setupFontOverrides();

	pout << "-- Finished loading minimal--" << std::endl << std::endl;
}

void Ultima8Engine::DeclareArgs() {
	// parent's arguments first
	CoreApp::DeclareArgs();

	// anything else?
}

void Ultima8Engine::runGame() {
	isRunning = true;

	int32 next_ticks = g_system->getMillis() * 3;  // Next time is right now!

	Common::Event event;
	while (isRunning) {
		inBetweenFrame = true;  // Will get set false if it's not an inBetweenFrame

		if (!frameLimit) {
			kernel->runProcesses();
			desktopGump->run();
			inBetweenFrame = false;
			next_ticks = animationRate + g_system->getMillis() * 3;
			lerpFactor = 256;
		} else {
			int32 ticks = g_system->getMillis() * 3;
			int32 diff = next_ticks - ticks;

			while (diff < 0) {
				next_ticks += animationRate;
				kernel->runProcesses();
				desktopGump->run();
#if 0
				perr << "--------------------------------------" << std::endl;
				perr << "NEW FRAME" << std::endl;
				perr << "--------------------------------------" << std::endl;
#endif
				inBetweenFrame = false;

				ticks = g_system->getMillis() * 3;

				// If frame skipping is off, we will only recalc next
				// ticks IF the frames are taking up 'way' too much time.
				if (!frameSkip && diff <= -animationRate * 2) next_ticks = animationRate + ticks;

				diff = next_ticks - ticks;
				if (!frameSkip) break;
			}

			// Calculate the lerp_factor
			lerpFactor = ((animationRate - diff) * 256) / animationRate;
			//pout << "lerpFactor: " << lerpFactor << " framenum: " << framenum << std::endl;
			if (!interpolate || kernel->isPaused() || lerpFactor > 256)
				lerpFactor = 256;
		}

		// get & handle all events in queue
		while (isRunning && g_system->getEventManager()->pollEvent(event)) {
			handleEvent(event);
		}
		handleDelayedEvents();

		// Paint Screen
		paint();

		if (!change_gamename.empty()) {
			pout << "Changing Game to: " << change_gamename << std::endl;

			GameInfo *info = getGameInfo(change_gamename);

			if (info) {
				shutdownGame();

				change_gamename.clear();

				if (setupGame(info))
					startupGame();
				else
					startupPentagramMenu();
			} else {
				perr << "Game '" << change_gamename << "' not found" << std::endl;
				change_gamename.clear();
			}
		}

		if (!error_message.empty()) {
			MessageBoxGump::Show(error_title, error_message, 0xFF8F3030);
			error_title.clear();
			error_message.clear();
		}

		// Do a delay
		g_system->delayMillis(5);
	}
}


// conAutoPaint hackery
void Ultima8Engine::conAutoPaint(void) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	if (app && !app->isPainting()) app->paint();
}

// Paint the screen
void Ultima8Engine::paint() {
	static long prev = 0;
	static long t = 0;
	static long tdiff = 0;
	static long tpaint = 0;
	long now = g_system->getMillis();

	if (!screen) // need to worry if the graphics system has been started. Need nicer way.
		return;

	if (prev != 0)
		tdiff += now - prev;
	prev = now;
	++t;

	painting = true;

	// Begin painting
	screen->BeginPainting();

	// We need to get the dims
	Pentagram::Rect dims;
	screen->GetSurfaceDims(dims);

	tpaint -= g_system->getMillis();
	desktopGump->Paint(screen, lerpFactor, false);
	tpaint += g_system->getMillis();

	// Draw the mouse
	_mouse->paint();

	if (drawRenderStats) {
		static long diff = 0;
		static long fps = 0;
		static long paint = 0;
		char buf[256] = { '\0' };
		FixedWidthFont *confont = con.GetConFont();
		int v_offset = 0;
		int char_w = confont->width;

		if (tdiff >= 250) {
			diff = tdiff / t;
			paint = tpaint / t;
			fps = 1000 * t / tdiff;
			t = 0;
			tdiff = 0;
			tpaint = 0;
		}

		snprintf(buf, 255, "Rendering time %li ms %li FPS ", diff, fps);
		screen->PrintTextFixed(confont, buf, dims.w - char_w * strlen(buf), v_offset);
		v_offset += confont->height;

		snprintf(buf, 255, "Paint Gumps %li ms ", paint);
		screen->PrintTextFixed(confont, buf, dims.w - char_w * strlen(buf), v_offset);
		v_offset += confont->height;

		snprintf(buf, 255, "t %02d:%02d gh %i ", I_getTimeInMinutes(0, 0), I_getTimeInSeconds(0, 0) % 60, I_getTimeInGameHours(0, 0));
		screen->PrintTextFixed(confont, buf, dims.w - char_w * strlen(buf), v_offset);
		v_offset += confont->height;
	}

	// End painting
	screen->EndPainting();

	painting = false;
}

void Ultima8Engine::GraphicSysInit() {
	settingman->setDefault("fullscreen", false);
	settingman->setDefault("width", SCREEN_WIDTH);
	settingman->setDefault("height", SCREEN_HEIGHT);
	settingman->setDefault("bpp", 32);

	bool new_fullscreen;
	int width, height, bpp;
	settingman->get("fullscreen", new_fullscreen);
	settingman->get("width", width);
	settingman->get("height", height);
	settingman->get("bpp", bpp);

#ifdef UNDER_CE
	width = 240;
	height = 320;
#endif

#if 0
	// store values in user's config file
	settingman->set("width", width);
	settingman->set("height", height);
	settingman->set("bpp", bpp);
	settingman->set("fullscreen", new_fullscreen);
#endif

	if (screen) {
		Pentagram::Rect old_dims;
		screen->GetSurfaceDims(old_dims);
		if (new_fullscreen == fullscreen && width == old_dims.w && height == old_dims.h) return;
		bpp = RenderSurface::format.s_bpp;

		delete screen;
	}
	screen = 0;

	fullscreen = new_fullscreen;

	// Set Screen Resolution
	con.Printf(MM_INFO, "Setting Video Mode %ix%ix%i %s...\n", width, height, bpp, fullscreen ? "fullscreen" : "windowed");

	RenderSurface *new_screen = RenderSurface::SetVideoMode(width, height, bpp);

	if (!new_screen) {
		perr << "Unable to set new video mode. Trying 640x480x32 windowed" << std::endl;
		new_screen = RenderSurface::SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
	}

	if (!new_screen) {
		error("Unable to set video mode. Exiting");
	}

	if (desktopGump) {
		palettemanager->RenderSurfaceChanged(new_screen);
		static_cast<DesktopGump *>(desktopGump)->RenderSurfaceChanged(new_screen);
		screen = new_screen;
		paint();
		return;
	}

	// setup normal mouse cursor
	con.Print(MM_INFO, "Loading Default Mouse Cursor...\n");
	_mouse->setup();

	std::string alt_confont;
	bool confont_loaded = false;

	if (settingman->get("console_font", alt_confont)) {
		con.Print(MM_INFO, "Alternate console font found...\n");
		confont_loaded = LoadConsoleFont(alt_confont);
	}

	if (!confont_loaded) {
		con.Print(MM_INFO, "Loading default console font...\n");
		if (!LoadConsoleFont("@data/fixedfont.ini")) {
			error("Failed to load console font. Exiting");
		}
	}

	desktopGump = new DesktopGump(0, 0, width, height);
	desktopGump->InitGump(0);
	desktopGump->MakeFocus();

	scalerGump = new ScalerGump(0, 0, width, height);
	scalerGump->InitGump(0);

	consoleGump = new ConsoleGump(0, 0, width, height);
	consoleGump->InitGump(0);

	Pentagram::Rect scaled_dims;
	scalerGump->GetDims(scaled_dims);

	inverterGump = new InverterGump(0, 0, scaled_dims.w, scaled_dims.h);
	inverterGump->InitGump(0);

	screen = new_screen;

	bool ttf_antialiasing = true;
	settingman->setDefault("ttf_antialiasing", true);
	settingman->get("ttf_antialiasing", ttf_antialiasing);

	fontmanager = new FontManager(ttf_antialiasing);
	palettemanager = new PaletteManager(new_screen);

	// TODO: assign names to these fontnumbers somehow
	fontmanager->loadTTFont(0, "Vera.ttf", 18, 0xFFFFFF, 0);
	fontmanager->loadTTFont(1, "VeraBd.ttf", 12, 0xFFFFFF, 0);
	// GameWidget's version number information:
	fontmanager->loadTTFont(2, "Vera.ttf", 8, 0xA0A0A0, 0);

	bool faded_modal = true;
	settingman->setDefault("fadedModal", faded_modal);
	settingman->get("fadedModal", faded_modal);
	DesktopGump::SetFadedModal(faded_modal);

	paint();
}

void Ultima8Engine::changeVideoMode(int width, int height, int new_fullscreen) {
	if (new_fullscreen == -2) settingman->set("fullscreen", !fullscreen);
	else if (new_fullscreen == 0) settingman->set("fullscreen", false);
	else if (new_fullscreen == 1) settingman->set("fullscreen", true);

	if (width > 0) settingman->set("width", width);
	if (height > 0) settingman->set("height", height);

	GraphicSysInit();
}

bool Ultima8Engine::LoadConsoleFont(std::string confontini) {
	// try to load the file
	con.Printf(MM_INFO, "Loading console font config: %s... ", confontini.c_str());
	if (configfileman->readConfigFile(confontini, "confont", true))
		pout << "Ok" << std::endl;
	else {
		pout << "Failed" << std::endl;
		return false;
	}

	FixedWidthFont *confont = FixedWidthFont::Create("confont");

	if (!confont) {
		perr << "Failed to load Console Font." << std::endl;
		return false;
	}

	con.SetConFont(confont);

	return true;
}

void Ultima8Engine::enterTextMode(Gump *gump) {
	uint16 key;
	for (key = 0; key < HID_LAST; ++key) {
		if (_down[key]) {
			_down[key] = false;
			_lastDown[key] = false;
			hidmanager->handleEvent((HID_Key)key, HID_EVENT_RELEASE);
		}
	}

	if (!textmodes.empty()) {
		textmodes.remove(gump->getObjId());
	} else {
#ifdef TODO
		SDL_EnableUNICODE(1);
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#endif
	}
	textmodes.push_front(gump->getObjId());
}

void Ultima8Engine::leaveTextMode(Gump *gump) {
	if (textmodes.empty()) return;
	textmodes.remove(gump->getObjId());
	if (textmodes.empty()) {
#ifdef TODO
		SDL_EnableUNICODE(0);
		SDL_EnableKeyRepeat(0, 0);
#endif
	}
}

void Ultima8Engine::handleEvent(const Common::Event &event) {
	uint32 now = g_system->getMillis();
	HID_Key key = HID_LAST;
	HID_Event evn = HID_EVENT_LAST;
	bool handled = false;

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		key = HID_translateSDLKey(event.kbd.keycode);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_KEYUP:
		key = HID_translateSDLKey(event.kbd.keycode);
		evn = HID_EVENT_RELEASE;
		if (_mouse->dragging() == Mouse::DRAG_NOT) {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_q: // Quick quit
#ifndef MACOSX
				if (event.kbd.flags & Common::KBD_CTRL)
					ForceQuit();
#else
				if (event.kbd.flags & Common::KBD_META)
					ForceQuit();
#endif
				return;
			default:
				break;
			}
		}
		break;

	case Common::EVENT_LBUTTONDOWN:
		key = HID_translateSDLMouseButton(1);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_LBUTTONUP:
		key = HID_translateSDLMouseButton(1);
		evn = HID_EVENT_RELEASE;
		break;
	case Common::EVENT_RBUTTONDOWN:
		key = HID_translateSDLMouseButton(2);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_RBUTTONUP:
		key = HID_translateSDLMouseButton(2);
		evn = HID_EVENT_RELEASE;
		break;
	case Common::EVENT_MBUTTONDOWN:
		key = HID_translateSDLMouseButton(3);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_MBUTTONUP:
		key = HID_translateSDLMouseButton(3);
		evn = HID_EVENT_RELEASE;
		break;

	case Common::EVENT_JOYBUTTON_DOWN:
		key = HID_translateSDLJoystickButton(event.joystick.button + 1);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_JOYBUTTON_UP:
		key = HID_translateSDLJoystickButton(event.joystick.button + 1);
		evn = HID_EVENT_DEPRESS;
		break;

	case Common::EVENT_MOUSEMOVE:
		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		break;

	case Common::EVENT_QUIT:
		isRunning = false;
		break;

	default:
		break;
	}

	if (_mouse->dragging() == Mouse::DRAG_NOT && evn == HID_EVENT_DEPRESS) {
		if (hidmanager->handleEvent(key, HID_EVENT_PREEMPT))
			return;
	}

	// Text mode input. A few hacks here
	if (!textmodes.empty()) {
		Gump *gump = 0;

		while (!textmodes.empty()) {
			gump = p_dynamic_cast<Gump *>(objectmanager->getObject(textmodes.front()));
			if (gump) break;

			textmodes.pop_front();
		}

		if (gump) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				// Paste from Clip-Board on Ctrl-V - Note this should be a flag of some sort
				if (event.kbd.keycode == Common::KEYCODE_v && (event.kbd.flags & Common::KBD_CTRL)) {
					if (!g_system->hasTextInClipboard())
						return;

					Common::String text = g_system->getTextFromClipboard();

					// Only read the first line of text
					while (!text.empty() && text.firstChar() >= ' ')
						gump->OnTextInput(text.firstChar());

					return;
				}

				if (event.kbd.ascii >= ' ' &&
					event.kbd.ascii <= 255 &&
					!(event.kbd.ascii >= 0x7F && // control chars
						event.kbd.ascii <= 0x9F)) {
					gump->OnTextInput(event.kbd.ascii);
				}

				gump->OnKeyDown(event.kbd.keycode, event.kbd.flags);
				return;

			case Common::EVENT_KEYUP:
				gump->OnKeyUp(event.kbd.keycode);
				return;

			default:
				break;
			}
		}
	}

	// Old style input begins here
	switch (event.type) {

		//!! TODO: handle mouse handedness. (swap left/right mouse buttons here)

		// most of these events will probably be passed to a gump manager,
		// since almost all (all?) user input will be handled by a gump

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN: {
		MouseButton button = BUTTON_LEFT;
		if (event.type == Common::EVENT_RBUTTONDOWN)
			button = BUTTON_RIGHT;
		else if (event.type == Common::EVENT_MBUTTONDOWN)
			button = BUTTON_MIDDLE;

		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		if (_mouse->buttonDown(button))
			handled = true;		
		break;
	}

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_MBUTTONUP:
	case Common::EVENT_RBUTTONUP: {
		MouseButton button = BUTTON_LEFT;
		if (event.type == Common::EVENT_RBUTTONDOWN)
			button = BUTTON_RIGHT;
		else if (event.type == Common::EVENT_MBUTTONDOWN)
			button = BUTTON_MIDDLE;

		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		if (_mouse->buttonUp(button))
			handled = true;
		break;
	}

	case Common::EVENT_KEYDOWN: {
		if (_mouse->dragging() != Mouse::DRAG_NOT) break;

		/*
		switch (event.kbd.keycode) {
			case Common::KEYCODE_KP_PLUS: {
				midi_volume+=8;
				if (midi_volume>255) midi_volume =255;
				pout << "Midi Volume is now: " << midi_volume << std::endl;
				if (midi_driver) midi_driver->setGlobalVolume(midi_volume);
			} break;
			case Common::KEYCODE_KP_MINUS: {
				midi_volume-=8;
				if (midi_volume<0) midi_volume = 0;
				pout << "Midi Volume is now: " << midi_volume << std::endl;
				if (midi_driver) midi_driver->setGlobalVolume(midi_volume);
			} break;
			default:
				break;
		}
		*/
		break;
	}

							  // any more useful events?

	default:
		break;
	}

	if (_mouse->dragging() == Mouse::DRAG_NOT && !handled) {
		if (hidmanager->handleEvent(key, evn))
			handled = true;
		if (evn == HID_EVENT_DEPRESS) {
			_down[key] = true;
			if (now - _lastDown[key] < DOUBLE_CLICK_TIMEOUT &&
				_lastDown[key] != 0) {
				if (hidmanager->handleEvent(key, HID_EVENT_DOUBLE))
					handled = true;
				_lastDown[key] = 0;
			} else {
				_lastDown[key] = now;
			}
		} else if (evn == HID_EVENT_RELEASE) {
			_down[key] = false;
			if (now - _lastDown[key] > DOUBLE_CLICK_TIMEOUT &&
				_lastDown[key] != 0) {
				_lastDown[key] = 0;
			}
		}
	}
}

void Ultima8Engine::handleDelayedEvents() {
	uint32 now = g_system->getMillis();

	_mouse->handleDelayedEvents();

	for (uint16 key = 0; key < HID_LAST; ++key) {
		if (now - _lastDown[key] > DOUBLE_CLICK_TIMEOUT &&
			_lastDown[key] != 0 && !_down[key]) {
			_lastDown[key] = 0;
			hidmanager->handleEvent((HID_Key)key, HID_EVENT_CLICK);
		}
	}

}

void Ultima8Engine::writeSaveInfo(ODataSource *ods) {
	TimeDate timeInfo;
	g_system->getTimeAndDate(timeInfo);

	ods->write2(static_cast<uint16>(timeInfo.tm_year + 1900));
	ods->write1(static_cast<uint8>(timeInfo.tm_mon + 1));
	ods->write1(static_cast<uint8>(timeInfo.tm_mday));
	ods->write1(static_cast<uint8>(timeInfo.tm_hour));
	ods->write1(static_cast<uint8>(timeInfo.tm_min));
	ods->write1(static_cast<uint8>(timeInfo.tm_sec));
	ods->write4(save_count);
	ods->write4(getGameTimeInSeconds());

	uint8 c = (has_cheated ? 1 : 0);
	ods->write1(c);

	// write game-specific info
	game->writeSaveInfo(ods);
}

bool Ultima8Engine::saveGame(std::string filename, std::string desc,
	bool ignore_modals) {
	// Don't allow saving with Modals open
	if (!ignore_modals && desktopGump->FindGump<ModalGump>()) {
		pout << "Can't save: modal gump open." << std::endl;
		return false;
	}

	// Don't allow saving when avatar is dead.
	// (Avatar is flagged dead by usecode when you finish the game as well.)
	MainActor *av = getMainActor();
	if (!av || (av->getActorFlags() & Actor::ACT_DEAD)) {
		pout << "Can't save: game over." << std::endl;
		return false;
	}

	pout << "Saving..." << std::endl;

	pout << "Savegame file: " << filename << std::endl;
	pout << "Description: " << desc << std::endl;

	// Hack - don't save mouse over status for gumps
	Gump *gump = _mouse->getMouseOverGump();
	if (gump)
		gump->OnMouseLeft();

	ODataSource *ods = filesystem->WriteFile(filename);
	if (!ods) return false;

	save_count++;

	SavegameWriter *sgw = new SavegameWriter(ods);
	sgw->writeVersion(Pentagram::savegame_version);
	sgw->writeDescription(desc);

	// We'll make it 2KB initially
	OAutoBufferDataSource buf(2048);

	gameinfo->save(&buf);
	sgw->writeFile("GAME", &buf);
	buf.clear();

	writeSaveInfo(&buf);
	sgw->writeFile("INFO", &buf);
	buf.clear();

	kernel->save(&buf);
	sgw->writeFile("KERNEL", &buf);
	buf.clear();

	objectmanager->save(&buf);
	sgw->writeFile("OBJECTS", &buf);
	buf.clear();

	world->save(&buf);
	sgw->writeFile("WORLD", &buf);
	buf.clear();

	world->saveMaps(&buf);
	sgw->writeFile("MAPS", &buf);
	buf.clear();

	world->getCurrentMap()->save(&buf);
	sgw->writeFile("CURRENTMAP", &buf);
	buf.clear();

	ucmachine->saveStrings(&buf);
	sgw->writeFile("UCSTRINGS", &buf);
	buf.clear();

	ucmachine->saveGlobals(&buf);
	sgw->writeFile("UCGLOBALS", &buf);
	buf.clear();

	ucmachine->saveLists(&buf);
	sgw->writeFile("UCLISTS", &buf);
	buf.clear();

	save(&buf);
	sgw->writeFile("APP", &buf);
	buf.clear();

	sgw->finish();

	delete sgw;

	// Restore mouse over
	if (gump) gump->OnMouseOver();

	settingman->set("lastSave", filename);

	pout << "Done" << std::endl;

	return true;
}

void Ultima8Engine::resetEngine() {
	con.Print(MM_INFO, "-- Resetting Engine --\n");

	// kill music
	if (audiomixer) audiomixer->reset();

	// now, reset everything (order matters)
	world->reset();
	ucmachine->reset();
	// ObjectManager, Kernel have to be last, because they kill
	// all processes/objects
	objectmanager->reset();
	kernel->reset();
	palettemanager->resetTransforms();

	// Reset thet gumps
	desktopGump = 0;
	consoleGump = 0;
	gameMapGump = 0;
	scalerGump = 0;
	inverterGump = 0;

	textmodes.clear();

	// reset mouse cursor
	_mouse->popAllCursors();
	_mouse->pushMouseCursor();

	// FIXME: This breaks loading processes if this process gets an ID
	//        also present in a savegame.
	// kernel->addProcess(new JoystickCursorProcess(JOY1, 0, 1));

	timeOffset = -(int32)Kernel::get_instance()->getFrameNum();
	inversion = 0;
	save_count = 0;
	has_cheated = false;

	con.Print(MM_INFO, "-- Engine Reset --\n");
}

void Ultima8Engine::setupCoreGumps() {
	con.Print(MM_INFO, "Setting up core game gumps...\n");

	Pentagram::Rect dims;
	screen->GetSurfaceDims(dims);

	con.Print(MM_INFO, "Creating Desktop...\n");
	desktopGump = new DesktopGump(0, 0, dims.w, dims.h);
	desktopGump->InitGump(0);
	desktopGump->MakeFocus();

	con.Print(MM_INFO, "Creating ScalerGump...\n");
	scalerGump = new ScalerGump(0, 0, dims.w, dims.h);
	scalerGump->InitGump(0);

	Pentagram::Rect scaled_dims;
	scalerGump->GetDims(scaled_dims);

	con.Print(MM_INFO, "Creating Graphics Console...\n");
	consoleGump = new ConsoleGump(0, 0, dims.w, dims.h);
	consoleGump->InitGump(0);
	consoleGump->HideConsole();

	con.Print(MM_INFO, "Creating Inverter...\n");
	inverterGump = new InverterGump(0, 0, scaled_dims.w, scaled_dims.h);
	inverterGump->InitGump(0);

	con.Print(MM_INFO, "Creating GameMapGump...\n");
	gameMapGump = new GameMapGump(0, 0, scaled_dims.w, scaled_dims.h);
	gameMapGump->InitGump(0);


	// TODO: clean this up
	assert(desktopGump->getObjId() == 256);
	assert(scalerGump->getObjId() == 257);
	assert(consoleGump->getObjId() == 258);
	assert(inverterGump->getObjId() == 259);
	assert(gameMapGump->getObjId() == 260);


	for (uint16 i = 261; i < 384; ++i)
		objectmanager->reserveObjId(i);
}

bool Ultima8Engine::newGame(const std::string &savegame) {
	con.Print(MM_INFO, "Starting New Game...\n");

	resetEngine();

	setupCoreGumps();

	game->startGame();

	con.Print(MM_INFO, "Create Camera...\n");
	CameraProcess::SetCameraProcess(new CameraProcess(1)); // Follow Avatar

	con.Print(MM_INFO, "Create persistent Processes...\n");
	avatarMoverProcess = new AvatarMoverProcess();
	kernel->addProcess(avatarMoverProcess);

	kernel->addProcess(new HealProcess());

	kernel->addProcess(new SchedulerProcess());

	if (audiomixer) audiomixer->createProcesses();

	//	av->teleport(40, 16240, 15240, 64); // central Tenebrae
	//	av->teleport(3, 11391, 1727, 64); // docks, near gate
	//	av->teleport(39, 16240, 15240, 64); // West Tenebrae
	//	av->teleport(41, 12000, 15000, 64); // East Tenebrae
	//	av->teleport(8, 14462, 15178, 48); // before entrance to Mythran's house
	//	av->teleport(40, 13102,9474,48); // entrance to Mordea's throne room
	//	av->teleport(54, 14783,5959,8); // shrine of the Ancient Ones; Hanoi
	//	av->teleport(5, 5104,22464,48); // East road (tenebrae end)

	game->startInitialUsecode(savegame);

	settingman->set("lastSave", savegame);

	return true;
}

bool Ultima8Engine::loadGame(std::string filename) {
	con.Print(MM_INFO, "Loading...\n");

	IDataSource *ids = filesystem->ReadFile(filename);
	if (!ids) {
		Error("Can't load file", "Error Loading savegame " + filename);
		settingman->set("lastSave", "");
		return false;
	}

	Savegame *sg = new Savegame(ids);
	uint32 version = sg->getVersion();
	if (version == 0) {
		Error("Invalid or corrupt savegame", "Error Loading savegame " + filename);
		delete sg;
		settingman->set("lastSave", "");
		return false;
	}

	if (version == 1 || version > Pentagram::savegame_version) {
		Common::String vstring = Common::String::format("%i", version);
		Error(std::string("Unsupported savegame version (") + vstring + ")", "Error Loading savegame " + filename);
		delete sg;
		settingman->set("lastSave", "");
		return false;
	}
	IDataSource *ds;
	GameInfo saveinfo;
	ds = sg->getDataSource("GAME");
	bool ok = saveinfo.load(ds, version);

	if (!ok) {
		Error("Invalid or corrupt savegame: missing GameInfo", "Error Loading savegame " + filename);
		delete sg;
		return false;
	}

	if (!gameinfo->match(saveinfo)) {
		std::string message = "Game mismatch\n";
		message += "Running game: " + gameinfo->getPrintDetails()  + "\n";
		message += "Savegame    : " + saveinfo.getPrintDetails();

#ifdef DEBUG
		bool ignore;
		settingman->setDefault("ignore_savegame_mismatch", false);
		settingman->get("ignore_savegame_mismatch", ignore);

		if (!ignore) {
			Error(message, "Error Loading savegame " + filename);
			delete sg;
			return false;
		}
		perr << message << std::endl;
#else
		settingman->set("lastSave", "");
		Error(message, "Error Loading savegame " + filename);
		return false;
#endif
	}

	resetEngine();

	setupCoreGumps();

	// and load everything back (order matters)
	bool totalok = true;

	std::string message;

	// UCSTRINGS, UCGLOBALS, UCLISTS don't depend on anything else,
	// so load these first
	ds = sg->getDataSource("UCSTRINGS");
	ok = ucmachine->loadStrings(ds, version);
	totalok &= ok;
	perr << "UCSTRINGS: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "UCSTRINGS: failed\n";
	delete ds;

	ds = sg->getDataSource("UCGLOBALS");
	ok = ucmachine->loadGlobals(ds, version);
	totalok &= ok;
	perr << "UCGLOBALS: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "UCGLOBALS: failed\n";
	delete ds;

	ds = sg->getDataSource("UCLISTS");
	ok = ucmachine->loadLists(ds, version);
	totalok &= ok;
	perr << "UCLISTS: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "UCLISTS: failed\n";
	delete ds;

	// KERNEL must be before OBJECTS, for the egghatcher
	// KERNEL must be before APP, for the avatarMoverProcess
	ds = sg->getDataSource("KERNEL");
	ok = kernel->load(ds, version);
	totalok &= ok;
	perr << "KERNEL: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "KERNEL: failed\n";
	delete ds;

	ds = sg->getDataSource("APP");
	ok = load(ds, version);
	totalok &= ok;
	perr << "APP: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "APP: failed\n";
	delete ds;

	// WORLD must be before OBJECTS, for the egghatcher
	ds = sg->getDataSource("WORLD");
	ok = world->load(ds, version);
	totalok &= ok;
	perr << "WORLD: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "WORLD: failed\n";
	delete ds;

	ds = sg->getDataSource("CURRENTMAP");
	ok = world->getCurrentMap()->load(ds, version);
	totalok &= ok;
	perr << "CURRENTMAP: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "CURRENTMAP: failed\n";
	delete ds;

	ds = sg->getDataSource("OBJECTS");
	ok = objectmanager->load(ds, version);
	totalok &= ok;
	perr << "OBJECTS: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "OBJECTS: failed\n";
	delete ds;

	ds = sg->getDataSource("MAPS");
	ok = world->loadMaps(ds, version);
	totalok &= ok;
	perr << "MAPS: " << (ok ? "ok" : "failed") << std::endl;
	if (!ok) message += "MAPS: failed\n";
	delete ds;

	if (!totalok) {
		Error(message, "Error Loading savegame " + filename, true);
		delete sg;
		return false;
	}

	pout << "Done" << std::endl;

	settingman->set("lastSave", filename);

	delete sg;
	return true;
}

void Ultima8Engine::Error(std::string message, std::string title, bool exit_to_menu) {
	if (title.empty()) title = exit_to_menu ? "Fatal Game Error" : "Error";

	perr << title << ": " << message << std::endl;

	error_message = message;
	error_title = title;

	if (exit_to_menu) {
		change_gamename = "pentagram";
		Kernel::get_instance()->killProcesses(0, 6, false);
	}
}

Gump *Ultima8Engine::getGump(uint16 gumpid) {
	return p_dynamic_cast<Gump *>(ObjectManager::get_instance()->
		getObject(gumpid));
}

void Ultima8Engine::addGump(Gump *gump) {
	// TODO: At some point, this will have to _properly_ choose to
	// which 'layer' to add the gump: inverted, scaled or neither.

	assert(desktopGump);

	if (gump->IsOfType<ShapeViewerGump>() || gump->IsOfType<MiniMapGump>() ||
		gump->IsOfType<ConsoleGump>() || gump->IsOfType<ScalerGump>() ||
		gump->IsOfType<PentagramMenuGump>() || gump->IsOfType<MessageBoxGump>()// ||
		//(ttfoverrides && (gump->IsOfType<BarkGump>() ||
		//                gump->IsOfType<AskGump>()))
		) {
		//		pout << "adding to desktopgump: "; gump->dumpInfo();
		desktopGump->AddChild(gump);
	} else if (gump->IsOfType<GameMapGump>()) {
		//		pout << "adding to invertergump: "; gump->dumpInfo();
		inverterGump->AddChild(gump);
	} else if (gump->IsOfType<InverterGump>()) {
		//		pout << "adding to scalergump: "; gump->dumpInfo();
		scalerGump->AddChild(gump);
	} else if (gump->IsOfType<DesktopGump>()) {
	} else {
		//		pout << "adding to scalergump: "; gump->dumpInfo();
		scalerGump->AddChild(gump);
	}
}

uint32 Ultima8Engine::getGameTimeInSeconds() {
	// 1 second per every 30 frames
	return (Kernel::get_instance()->getFrameNum() + timeOffset) / 30; // constant!
}


void Ultima8Engine::save(ODataSource *ods) {
	uint8 s = (avatarInStasis ? 1 : 0);
	ods->write1(s);

	int32 absoluteTime = Kernel::get_instance()->getFrameNum() + timeOffset;
	ods->write4(static_cast<uint32>(absoluteTime));
	ods->write2(avatarMoverProcess->getPid());

	Pentagram::Palette *pal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	for (int i = 0; i < 12; i++) ods->write2(pal->matrix[i]);
	ods->write2(pal->transform);

	ods->write2(static_cast<uint16>(inversion));

	ods->write4(save_count);

	uint8 c = (has_cheated ? 1 : 0);
	ods->write1(c);
}

bool Ultima8Engine::load(IDataSource *ids, uint32 version) {
	avatarInStasis = (ids->read1() != 0);

	// no gump should be moused over after load
	_mouse->resetMouseOverGump();

	int32 absoluteTime = static_cast<int32>(ids->read4());
	timeOffset = absoluteTime - Kernel::get_instance()->getFrameNum();

	uint16 amppid = ids->read2();
	avatarMoverProcess = p_dynamic_cast<AvatarMoverProcess *>(Kernel::get_instance()->getProcess(amppid));

	int16 matrix[12];
	for (int i = 0; i < 12; i++)
		matrix[i] = ids->read2();

	PaletteManager::get_instance()->transformPalette(PaletteManager::Pal_Game, matrix);
	Pentagram::Palette *pal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	pal->transform = static_cast<Pentagram::PalTransforms>(ids->read2());

	inversion = ids->read2();

	save_count = ids->read4();

	has_cheated = (ids->read1() != 0);

	return true;
}

//
// Console Commands
//

void Ultima8Engine::ConCmd_saveGame(const Console::ArgvType &argv) {
	if (argv.size() == 1) {
		pout << "Usage: Ultima8Engine::saveGame <filename>" << std::endl;
		return;
	}

	std::string filename = "@save/";
	filename += argv[1].c_str();
	Ultima8Engine::get_instance()->saveGame(filename, argv[1]);
}

void Ultima8Engine::ConCmd_loadGame(const Console::ArgvType &argv) {
	if (argv.size() == 1) {
		pout << "Usage: Ultima8Engine::loadGame <filename>" << std::endl;
		return;
	}

	std::string filename = "@save/";
	filename += argv[1].c_str();
	Ultima8Engine::get_instance()->loadGame(filename);
}

void Ultima8Engine::ConCmd_newGame(const Console::ArgvType &argv) {
	Ultima8Engine::get_instance()->newGame(std::string());
}


void Ultima8Engine::ConCmd_quit(const Console::ArgvType &argv) {
	Ultima8Engine::get_instance()->isRunning = false;
}

void Ultima8Engine::ConCmd_drawRenderStats(const Console::ArgvType &argv) {
	if (argv.size() == 1) {
		pout << "Ultima8Engine::drawRenderStats = " << Ultima8Engine::get_instance()->drawRenderStats << std::endl;
	} else {
		Ultima8Engine::get_instance()->drawRenderStats = std::strtol(argv[1].c_str(), 0, 0) != 0;
	}
}

void Ultima8Engine::ConCmd_engineStats(const Console::ArgvType &argv) {
	Kernel::get_instance()->kernelStats();
	ObjectManager::get_instance()->objectStats();
	UCMachine::get_instance()->usecodeStats();
	World::get_instance()->worldStats();
}

void Ultima8Engine::ConCmd_changeGame(const Console::ArgvType &argv) {
	if (argv.size() == 1) {
		pout << "Current game is: " << Ultima8Engine::get_instance()->gameinfo->name << std::endl;
	} else {
		Ultima8Engine::get_instance()->changeGame(argv[1]);
	}
}

void Ultima8Engine::ConCmd_listGames(const Console::ArgvType &argv) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	std::vector<Pentagram::istring> games;
	games = app->settingman->listGames();
	std::vector<Pentagram::istring>::iterator iter;
	for (iter = games.begin(); iter != games.end(); ++iter) {
		Pentagram::istring game = *iter;
		GameInfo *info = app->getGameInfo(game);
		con.Printf(MM_INFO, "%s: ", game.c_str());
		if (info) {
			std::string details = info->getPrintDetails();
			con.Print(MM_INFO, details.c_str());
		} else {
			con.Print(MM_INFO, "(unknown)");
		}
		con.Print(MM_INFO, "\n");
	}
}

void Ultima8Engine::ConCmd_setVideoMode(const Console::ArgvType &argv) {
	int fullscreen = -1;

	//if (argv.size() == 4) {
	//  if (argv[3] == "fullscreen") fullscreen = 1;
	//  else fullscreen = 0;
	//} else
	if (argv.size() != 3) {
		//pout << "Usage: Ultima8Engine::setVidMode width height [fullscreen/windowed]" << std::endl;
		pout << "Usage: Ultima8Engine::setVidMode width height" << std::endl;
		return;
	}

	Ultima8Engine::get_instance()->changeVideoMode(strtol(argv[1].c_str(), 0, 0), strtol(argv[2].c_str(), 0, 0), fullscreen);
}

void Ultima8Engine::ConCmd_toggleFullscreen(const Console::ArgvType &argv) {
	Ultima8Engine::get_instance()->changeVideoMode(-1, -1, -2);
}

void Ultima8Engine::ConCmd_toggleAvatarInStasis(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->toggleAvatarInStasis();
	pout << "avatarInStasis = " << g->isAvatarInStasis() << std::endl;
}

void Ultima8Engine::ConCmd_togglePaintEditorItems(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->togglePaintEditorItems();
	pout << "paintEditorItems = " << g->isPaintEditorItems() << std::endl;
}

void Ultima8Engine::ConCmd_toggleShowTouchingItems(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->toggleShowTouchingItems();
	pout << "ShowTouchingItems = " << g->isShowTouchingItems() << std::endl;
}

void Ultima8Engine::ConCmd_closeItemGumps(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->getDesktopGump()->CloseItemDependents();
}

void Ultima8Engine::ConCmd_toggleCheatMode(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->setCheatMode(!g->areCheatsEnabled());
	pout << "Cheats = " << g->areCheatsEnabled() << std::endl;
}

void Ultima8Engine::ConCmd_memberVar(const Console::ArgvType &argv) {
	if (argv.size() == 1) {
		pout << "Usage: Ultima8Engine::memberVar <member> [newvalue] [updateini]" << std::endl;
		return;
	}

	Ultima8Engine *g = Ultima8Engine::get_instance();

	// Set the pointer to the correct type
	bool *b = 0;
	int *i = 0;
	std::string *str = 0;
	Pentagram::istring *istr = 0;

	// ini entry name if supported
	const char *ini = 0;

	if (argv[1] == "frameLimit") {
		b = &g->frameLimit;
		ini = "frameLimit";
	} else if (argv[1] == "frameSkip") {
		b = &g->frameSkip;
		ini = "frameSkip";
	} else if (argv[1] == "interpolate") {
		b = &g->interpolate;
		ini = "interpolate";
	} else {
		pout << "Unknown member: " << argv[1] << std::endl;
		return;
	}

	// Set the value
	if (argv.size() >= 3) {
		if (b) *b = (argv[2] == "yes" || argv[2] == "true");
		else if (istr) *istr = argv[2];
		else if (i) *i = std::strtol(argv[2].c_str(), 0, 0);
		else if (str) *str = argv[2];

		// Set config value
		if (argv.size() >= 4 && ini && *ini && (argv[3] == "yes" || argv[3] == "true")) {
			if (b) g->settingman->set(ini, *b);
			else if (istr) g->settingman->set(ini, *istr);
			else if (i) g->settingman->set(ini, *i);
			else if (str) g->settingman->set(ini, *str);
		}
	}

	// Print the value
	pout << "Ultima8Engine::" << argv[1] << " = ";
	if (b) pout << ((*b) ? "true" : "false");
	else if (istr) pout << *istr;
	else if (i) pout << *i;
	else if (str) pout << *str;
	pout << std::endl;

	return;
}

//
// Intrinsics
//

uint32 Ultima8Engine::I_avatarCanCheat(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	return Ultima8Engine::get_instance()->areCheatsEnabled() ? 1 : 0;
}


uint32 Ultima8Engine::I_makeAvatarACheater(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	Ultima8Engine::get_instance()->makeCheater();
	return 0;
}

uint32 Ultima8Engine::I_getCurrentTimerTick(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	// number of ticks of a 60Hz timer, with the default animrate of 30Hz
	return Kernel::get_instance()->getFrameNum() * 2;
}

uint32 Ultima8Engine::I_setAvatarInStasis(const uint8 *args, unsigned int /*argsize*/) {
	ARG_SINT16(stasis);
	get_instance()->setAvatarInStasis(stasis != 0);
	return 0;
}

uint32 Ultima8Engine::I_getAvatarInStasis(const uint8 * /*args*/, unsigned int /*argsize*/) {
	if (get_instance()->avatarInStasis)
		return 1;
	else
		return 0;
}

uint32 Ultima8Engine::I_getTimeInGameHours(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	// 900 seconds per game hour
	return get_instance()->getGameTimeInSeconds() / 900;
}

uint32 Ultima8Engine::I_getTimeInMinutes(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	// 60 seconds per minute
	return get_instance()->getGameTimeInSeconds() / 60;
}

uint32 Ultima8Engine::I_getTimeInSeconds(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	return get_instance()->getGameTimeInSeconds();
}

uint32 Ultima8Engine::I_setTimeInGameHours(const uint8 *args,
	unsigned int /*argsize*/) {
	ARG_UINT16(newhour);

	// 1 game hour per every 27000 frames
	int32   absolute = newhour * 27000;
	get_instance()->timeOffset = absolute - Kernel::get_instance()->getFrameNum();

	return 0;
}

uint32 Ultima8Engine::I_closeItemGumps(const uint8 *args, unsigned int /*argsize*/) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->getDesktopGump()->CloseItemDependents();

	return 0;
}

} // End of namespace Ultima8
