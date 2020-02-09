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
#include "common/translation.h"
#include "common/unzip.h"
#include "common/translation.h"
#include "common/config-manager.h"
#include "gui/saveload.h"
#include "image/png.h"

#include "ultima/shared/engine/events.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/misc/pent_include.h"

 //!! a lot of these includes are just for some hacks... clean up sometime
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/kernel/memory_manager.h"
#include "ultima/ultima8/kernel/hid_manager.h"
#include "ultima/ultima8/kernel/joystick.h"

#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/texture.h"
#include "ultima/ultima8/graphics/fonts/fixed_width_font.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/graphics/palette.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/games/game.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/filesys/savegame.h"

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/console_gump.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/gumps/inverter_gump.h"
#include "ultima/ultima8/gumps/scaler_gump.h"
#include "ultima/ultima8/gumps/fast_area_vis_gump.h"
#include "ultima/ultima8/gumps/minimap_gump.h"
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/gumps/menu_gump.h"
#include "ultima/ultima8/gumps/pentagram_menu_gump.h"

// For gump positioning... perhaps shouldn't do it this way....
#include "ultima/ultima8/gumps/bark_gump.h"
#include "ultima/ultima8/gumps/ask_gump.h"
#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/gumps/message_box_gump.h"


#include "ultima/ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/actors/targeted_anim_process.h"
#include "ultima/ultima8/usecode/u8_intrinsics.h"
#include "ultima/ultima8/usecode/remorse_intrinsics.h"
#include "ultima/ultima8/world/egg.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/graphics/inverter_process.h"
#include "ultima/ultima8/world/actors/heal_process.h"
#include "ultima/ultima8/world/actors/scheduler_process.h"

#include "ultima/ultima8/world/egg_hatcher_process.h" // for a hack
#include "ultima/ultima8/usecode/uc_process.h" // more hacking
#include "ultima/ultima8/gumps/gump_notify_process.h" // guess
#include "ultima/ultima8/world/actors/actor_bark_notify_process.h" // guess
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/actors/avatar_gravity_process.h"
#include "ultima/ultima8/world/actors/teleport_to_egg_process.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/actors/resurrection_process.h"
#include "ultima/ultima8/world/split_item_process.h"
#include "ultima/ultima8/world/actors/clear_feign_death_process.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/avatar_death_process.h"
#include "ultima/ultima8/world/actors/grant_peace_process.h"
#include "ultima/ultima8/world/actors/combat_process.h"
#include "ultima/ultima8/world/fireball_process.h"
#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/world/actors/ambush_process.h"
#include "ultima/ultima8/world/actors/pathfinder.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/gumps/shape_viewer_gump.h"
#include "ultima/ultima8/audio/audio_mixer.h"
#include "ultima/ultima8/graphics/xform_blend.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/misc/util.h"
#include "ultima/ultima8/audio/midi_player.h"

namespace Ultima {
namespace Ultima8 {

using Std::string;

DEFINE_RUNTIME_CLASSTYPE_CODE(Ultima8Engine, CoreApp)

Ultima8Engine::Ultima8Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Shared::UltimaEngine(syst, gameDesc), CoreApp(gameDesc), _saveCount(0), _game(0),
		_kernel(0), _objectManager(0), _hidManager(0), _mouse(0), _ucMachine(0), _screen(0),
		_fontManager(0), _fullScreen(false), _paletteManager(0), _gameData(0), _world(0),
		_desktopGump(0), _consoleGump(0), _gameMapGump(0), _avatarMoverProcess(0),
		_frameSkip(false), _frameLimit(true), _interpolate(true), _animationRate(100),
		_avatarInStasis(false), _paintEditorItems(false), _inversion(0), _painting(false),
		_showTouching(false), _timeOffset(0), _hasCheated(false), _cheatsEnabled(false),
		_drawRenderStats(false), _ttfOverrides(false), _audioMixer(0) {
	_application = this;

	for (uint16 key = 0; key < HID_LAST; ++key) {
		_lastDown[key] = false;
		_down[key] = false;
	}
}

Ultima8Engine::~Ultima8Engine() {
	FORGET_OBJECT(_events);
	FORGET_OBJECT(_kernel);
	FORGET_OBJECT(_objectManager);
	FORGET_OBJECT(_hidManager);
	FORGET_OBJECT(_audioMixer);
	FORGET_OBJECT(_ucMachine);
	FORGET_OBJECT(_paletteManager);
	FORGET_OBJECT(_mouse);
	FORGET_OBJECT(_gameData);
	FORGET_OBJECT(_world);
	FORGET_OBJECT(_ucMachine);
	FORGET_OBJECT(_fontManager);
	FORGET_OBJECT(_screen);
	FORGET_OBJECT(_memoryManager);
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
	if (!Shared::UltimaEngine::initialize())
		return false;

	// Set up the events manager
	_events = new Shared::EventsManager(this);

	// Add console commands
	con->AddConsoleCommand("quit", ConCmd_quit);
	con->AddConsoleCommand("Ultima8Engine::quit", ConCmd_quit);
	con->AddConsoleCommand("QuitGump::verifyQuit", QuitGump::ConCmd_verifyQuit);
	con->AddConsoleCommand("ShapeViewerGump::U8ShapeViewer", ShapeViewerGump::ConCmd_U8ShapeViewer);
	con->AddConsoleCommand("MenuGump::showMenu", MenuGump::ConCmd_showMenu);
	con->AddConsoleCommand("Ultima8Engine::_drawRenderStats", ConCmd_drawRenderStats);
	con->AddConsoleCommand("Ultima8Engine::engineStats", ConCmd_engineStats);

	con->AddConsoleCommand("Ultima8Engine::changeGame", ConCmd_changeGame);
	con->AddConsoleCommand("Ultima8Engine::listGames", ConCmd_listGames);

	con->AddConsoleCommand("Ultima8Engine::memberVar", &Ultima8Engine::ConCmd_memberVar);
	con->AddConsoleCommand("Ultima8Engine::setVideoMode", ConCmd_setVideoMode);

	con->AddConsoleCommand("Ultima8Engine::toggleAvatarInStasis", ConCmd_toggleAvatarInStasis);
	con->AddConsoleCommand("Ultima8Engine::togglePaintEditorItems", ConCmd_togglePaintEditorItems);
	con->AddConsoleCommand("Ultima8Engine::toggleShowTouchingItems", ConCmd_toggleShowTouchingItems);

	con->AddConsoleCommand("Ultima8Engine::closeItemGumps", ConCmd_closeItemGumps);

	con->AddConsoleCommand("HIDManager::bind", HIDManager::ConCmd_bind);
	con->AddConsoleCommand("HIDManager::unbind", HIDManager::ConCmd_unbind);
	con->AddConsoleCommand("HIDManager::listbinds",
		HIDManager::ConCmd_listbinds);
	con->AddConsoleCommand("HIDManager::save", HIDManager::ConCmd_save);
	con->AddConsoleCommand("Kernel::processTypes", Kernel::ConCmd_processTypes);
	con->AddConsoleCommand("Kernel::processInfo", Kernel::ConCmd_processInfo);
	con->AddConsoleCommand("Kernel::listProcesses",
		Kernel::ConCmd_listProcesses);
	con->AddConsoleCommand("Kernel::toggleFrameByFrame",
		Kernel::ConCmd_toggleFrameByFrame);
	con->AddConsoleCommand("Kernel::advanceFrame", Kernel::ConCmd_advanceFrame);
	con->AddConsoleCommand("ObjectManager::objectTypes",
		ObjectManager::ConCmd_objectTypes);
	con->AddConsoleCommand("ObjectManager::objectInfo",
		ObjectManager::ConCmd_objectInfo);
	con->AddConsoleCommand("MemoryManager::MemInfo",
		MemoryManager::ConCmd_MemInfo);
	con->AddConsoleCommand("MemoryManager::test",
		MemoryManager::ConCmd_test);

	con->AddConsoleCommand("QuickAvatarMoverProcess::startMoveUp",
		QuickAvatarMoverProcess::ConCmd_startMoveUp);
	con->AddConsoleCommand("QuickAvatarMoverProcess::startMoveDown",
		QuickAvatarMoverProcess::ConCmd_startMoveDown);
	con->AddConsoleCommand("QuickAvatarMoverProcess::startMoveLeft",
		QuickAvatarMoverProcess::ConCmd_startMoveLeft);
	con->AddConsoleCommand("QuickAvatarMoverProcess::startMoveRight",
		QuickAvatarMoverProcess::ConCmd_startMoveRight);
	con->AddConsoleCommand("QuickAvatarMoverProcess::startAscend",
		QuickAvatarMoverProcess::ConCmd_startAscend);
	con->AddConsoleCommand("QuickAvatarMoverProcess::startDescend",
		QuickAvatarMoverProcess::ConCmd_startDescend);
	con->AddConsoleCommand("QuickAvatarMoverProcess::stopMoveUp",
		QuickAvatarMoverProcess::ConCmd_stopMoveUp);
	con->AddConsoleCommand("QuickAvatarMoverProcess::stopMoveDown",
		QuickAvatarMoverProcess::ConCmd_stopMoveDown);
	con->AddConsoleCommand("QuickAvatarMoverProcess::stopMoveLeft",
		QuickAvatarMoverProcess::ConCmd_stopMoveLeft);
	con->AddConsoleCommand("QuickAvatarMoverProcess::stopMoveRight",
		QuickAvatarMoverProcess::ConCmd_stopMoveRight);
	con->AddConsoleCommand("QuickAvatarMoverProcess::stopAscend",
		QuickAvatarMoverProcess::ConCmd_stopAscend);
	con->AddConsoleCommand("QuickAvatarMoverProcess::stopDescend",
		QuickAvatarMoverProcess::ConCmd_stopDescend);
	con->AddConsoleCommand("QuickAvatarMoverProcess::toggleQuarterSpeed",
		QuickAvatarMoverProcess::ConCmd_toggleQuarterSpeed);
	con->AddConsoleCommand("QuickAvatarMoverProcess::toggleClipping",
		QuickAvatarMoverProcess::ConCmd_toggleClipping);

	con->AddConsoleCommand("GameMapGump::toggleHighlightItems",
		GameMapGump::ConCmd_toggleHighlightItems);
	con->AddConsoleCommand("GameMapGump::dumpMap",
		GameMapGump::ConCmd_dumpMap);
	con->AddConsoleCommand("GameMapGump::incrementSortOrder",
		GameMapGump::ConCmd_incrementSortOrder);
	con->AddConsoleCommand("GameMapGump::decrementSortOrder",
		GameMapGump::ConCmd_decrementSortOrder);

	con->AddConsoleCommand("AudioProcess::listSFX", AudioProcess::ConCmd_listSFX);
	con->AddConsoleCommand("AudioProcess::playSFX", AudioProcess::ConCmd_playSFX);
	con->AddConsoleCommand("AudioProcess::stopSFX", AudioProcess::ConCmd_stopSFX);

	return true;
}

void Ultima8Engine::deinitialize() {
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_quit);
	con->RemoveConsoleCommand(QuitGump::ConCmd_verifyQuit);
	con->RemoveConsoleCommand(ShapeViewerGump::ConCmd_U8ShapeViewer);
	con->RemoveConsoleCommand(MenuGump::ConCmd_showMenu);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_drawRenderStats);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_engineStats);

	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_changeGame);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_listGames);

	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_memberVar);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_setVideoMode);

	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_toggleAvatarInStasis);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_togglePaintEditorItems);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_toggleShowTouchingItems);

	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_closeItemGumps);

	con->RemoveConsoleCommand(HIDManager::ConCmd_bind);
	con->RemoveConsoleCommand(HIDManager::ConCmd_unbind);
	con->RemoveConsoleCommand(HIDManager::ConCmd_listbinds);
	con->RemoveConsoleCommand(HIDManager::ConCmd_save);
	con->RemoveConsoleCommand(Kernel::ConCmd_processTypes);
	con->RemoveConsoleCommand(Kernel::ConCmd_processInfo);
	con->RemoveConsoleCommand(Kernel::ConCmd_listProcesses);
	con->RemoveConsoleCommand(Kernel::ConCmd_toggleFrameByFrame);
	con->RemoveConsoleCommand(Kernel::ConCmd_advanceFrame);
	con->RemoveConsoleCommand(ObjectManager::ConCmd_objectTypes);
	con->RemoveConsoleCommand(ObjectManager::ConCmd_objectInfo);
	con->RemoveConsoleCommand(MemoryManager::ConCmd_MemInfo);
	con->RemoveConsoleCommand(MemoryManager::ConCmd_test);

	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startMoveUp);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startMoveDown);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startMoveLeft);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startMoveRight);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startAscend);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_startDescend);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopMoveUp);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopMoveDown);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopMoveLeft);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopMoveRight);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopAscend);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_stopDescend);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_toggleQuarterSpeed);
	con->RemoveConsoleCommand(QuickAvatarMoverProcess::ConCmd_toggleClipping);

	con->RemoveConsoleCommand(GameMapGump::ConCmd_toggleHighlightItems);
	con->RemoveConsoleCommand(GameMapGump::ConCmd_dumpMap);
	con->RemoveConsoleCommand(GameMapGump::ConCmd_incrementSortOrder);
	con->RemoveConsoleCommand(GameMapGump::ConCmd_decrementSortOrder);

	con->RemoveConsoleCommand(AudioProcess::ConCmd_listSFX);
	con->RemoveConsoleCommand(AudioProcess::ConCmd_stopSFX);
	con->RemoveConsoleCommand(AudioProcess::ConCmd_playSFX);
}

void Ultima8Engine::startup() {
	// Set the console to auto paint, till we have finished initing
	con->SetAutoPaint(conAutoPaint);

	pout << "-- Initializing Pentagram -- " << Std::endl;

	// parent's startup first
	CoreApp::startup();

	bool dataoverride;
	if (!_settingMan->get("dataoverride", dataoverride,
		SettingManager::DOM_GLOBAL))
		dataoverride = false;
	_fileSystem->initBuiltinData(dataoverride);

	_kernel = new Kernel();
	_memoryManager = new MemoryManager();
	setDebugger(new Debugger());

	//!! move this elsewhere
	_kernel->addProcessLoader("DelayProcess",
		ProcessLoader<DelayProcess>::load);
	_kernel->addProcessLoader("GravityProcess",
		ProcessLoader<GravityProcess>::load);
	_kernel->addProcessLoader("AvatarGravityProcess",
		ProcessLoader<AvatarGravityProcess>::load);
	_kernel->addProcessLoader("PaletteFaderProcess",
		ProcessLoader<PaletteFaderProcess>::load);
	_kernel->addProcessLoader("TeleportToEggProcess",
		ProcessLoader<TeleportToEggProcess>::load);
	_kernel->addProcessLoader("ActorAnimProcess",
		ProcessLoader<ActorAnimProcess>::load);
	_kernel->addProcessLoader("TargetedAnimProcess",
		ProcessLoader<TargetedAnimProcess>::load);
	_kernel->addProcessLoader("AvatarMoverProcess",
		ProcessLoader<AvatarMoverProcess>::load);
	_kernel->addProcessLoader("QuickAvatarMoverProcess",
		ProcessLoader<QuickAvatarMoverProcess>::load);
	_kernel->addProcessLoader("PathfinderProcess",
		ProcessLoader<PathfinderProcess>::load);
	_kernel->addProcessLoader("SpriteProcess",
		ProcessLoader<SpriteProcess>::load);
	_kernel->addProcessLoader("CameraProcess",
		ProcessLoader<CameraProcess>::load);
	_kernel->addProcessLoader("MusicProcess",
		ProcessLoader<MusicProcess>::load);
	_kernel->addProcessLoader("AudioProcess",
		ProcessLoader<AudioProcess>::load);
	_kernel->addProcessLoader("EggHatcherProcess",
		ProcessLoader<EggHatcherProcess>::load);
	_kernel->addProcessLoader("UCProcess",
		ProcessLoader<UCProcess>::load);
	_kernel->addProcessLoader("GumpNotifyProcess",
		ProcessLoader<GumpNotifyProcess>::load);
	_kernel->addProcessLoader("ResurrectionProcess",
		ProcessLoader<ResurrectionProcess>::load);
	_kernel->addProcessLoader("DeleteActorProcess",
		ProcessLoader<DestroyItemProcess>::load);  // YES, this is intentional
	_kernel->addProcessLoader("DestroyItemProcess",
		ProcessLoader<DestroyItemProcess>::load);
	_kernel->addProcessLoader("SplitItemProcess",
		ProcessLoader<SplitItemProcess>::load);
	_kernel->addProcessLoader("ClearFeignDeathProcess",
		ProcessLoader<ClearFeignDeathProcess>::load);
	_kernel->addProcessLoader("LoiterProcess",
		ProcessLoader<LoiterProcess>::load);
	_kernel->addProcessLoader("AvatarDeathProcess",
		ProcessLoader<AvatarDeathProcess>::load);
	_kernel->addProcessLoader("GrantPeaceProcess",
		ProcessLoader<GrantPeaceProcess>::load);
	_kernel->addProcessLoader("CombatProcess",
		ProcessLoader<CombatProcess>::load);
	_kernel->addProcessLoader("FireballProcess",
		ProcessLoader<FireballProcess>::load);
	_kernel->addProcessLoader("HealProcess",
		ProcessLoader<HealProcess>::load);
	_kernel->addProcessLoader("SchedulerProcess",
		ProcessLoader<SchedulerProcess>::load);
	_kernel->addProcessLoader("InverterProcess",
		ProcessLoader<InverterProcess>::load);
	_kernel->addProcessLoader("ActorBarkNotifyProcess",
		ProcessLoader<ActorBarkNotifyProcess>::load);
	_kernel->addProcessLoader("JoystickCursorProcess",
		ProcessLoader<JoystickCursorProcess>::load);
	_kernel->addProcessLoader("AmbushProcess",
		ProcessLoader<AmbushProcess>::load);

	_objectManager = new ObjectManager();
	_mouse = new Mouse();

	GraphicSysInit();

#ifdef TODO
	SDL_ShowCursor(SDL_DISABLE);
	SDL_GetMouseState(&mouseX, &mouseY);
#endif
	_hidManager = new HIDManager();

	// Audio Mixer
	_audioMixer = new AudioMixer(_mixer);

	pout << "-- Pentagram Initialized -- " << Std::endl << Std::endl;

	// We Attempt to startup _game
	setupGameList();
	GameInfo *info = getDefaultGame();
	if (setupGame(info))
		startupGame();
	else
		startupPentagramMenu();

	// Unset the console auto paint, since we have finished initing
	con->SetAutoPaint(0);

	//	pout << "Paint Initial display" << Std::endl;
	paint();
}

void Ultima8Engine::startupGame() {
	con->SetAutoPaint(conAutoPaint);

	pout  << Std::endl << "-- Initializing Game: " << _gameInfo->name << " --" << Std::endl;

	GraphicSysInit();

	// Generic Commands
	con->AddConsoleCommand("Ultima8Engine::saveGame", ConCmd_saveGame);
	con->AddConsoleCommand("Ultima8Engine::loadGame", ConCmd_loadGame);
	con->AddConsoleCommand("Ultima8Engine::newGame", ConCmd_newGame);
#ifdef DEBUG
	con->AddConsoleCommand("Pathfinder::visualDebug",
		Pathfinder::ConCmd_visualDebug);
#endif

	// U8 Game commands
	con->AddConsoleCommand("MainActor::teleport", MainActor::ConCmd_teleport);
	con->AddConsoleCommand("MainActor::mark", MainActor::ConCmd_mark);
	con->AddConsoleCommand("MainActor::recall", MainActor::ConCmd_recall);
	con->AddConsoleCommand("MainActor::listmarks", MainActor::ConCmd_listmarks);
	con->AddConsoleCommand("Cheat::maxstats", MainActor::ConCmd_maxstats);
	con->AddConsoleCommand("Cheat::heal", MainActor::ConCmd_heal);
	con->AddConsoleCommand("Cheat::toggleInvincibility", MainActor::ConCmd_toggleInvincibility);
	con->AddConsoleCommand("Cheat::toggle", Ultima8Engine::ConCmd_toggleCheatMode);
	con->AddConsoleCommand("MainActor::name", MainActor::ConCmd_name);
	con->AddConsoleCommand("MovieGump::play", MovieGump::ConCmd_play);
	con->AddConsoleCommand("MusicProcess::playMusic", MusicProcess::ConCmd_playMusic);
	con->AddConsoleCommand("InverterProcess::invertScreen",
		InverterProcess::ConCmd_invertScreen);
	con->AddConsoleCommand("FastAreaVisGump::toggle",
		FastAreaVisGump::ConCmd_toggle);
	con->AddConsoleCommand("MiniMapGump::toggle",
		MiniMapGump::ConCmd_toggle);
	con->AddConsoleCommand("MainActor::useBackpack",
		MainActor::ConCmd_useBackpack);
	con->AddConsoleCommand("MainActor::useInventory",
		MainActor::ConCmd_useInventory);
	con->AddConsoleCommand("MainActor::useRecall",
		MainActor::ConCmd_useRecall);
	con->AddConsoleCommand("MainActor::useBedroll",
		MainActor::ConCmd_useBedroll);
	con->AddConsoleCommand("MainActor::useKeyring",
		MainActor::ConCmd_useKeyring);
	con->AddConsoleCommand("MainActor::toggleCombat",
		MainActor::ConCmd_toggleCombat);

	_gameData = new GameData(_gameInfo);

	Std::string bindingsfile;
	if (GAME_IS_U8) {
		bindingsfile = "@data/u8bindings.ini";
	} else if (GAME_IS_REMORSE) {
		bindingsfile = "@data/remorsebindings.ini";
	}
	if (!bindingsfile.empty()) {
		// system-wide config
		if (_configFileMan->readConfigFile(bindingsfile,
			"bindings", true))
			con->Printf(MM_INFO, "%s... Ok\n", bindingsfile.c_str());
		else
			con->Printf(MM_MINOR_WARN, "%s... Failed\n", bindingsfile.c_str());
	}

	_hidManager->loadBindings();

	if (GAME_IS_U8) {
		_ucMachine = new UCMachine(U8Intrinsics, 256);
	} else if (GAME_IS_REMORSE) {
		_ucMachine = new UCMachine(RemorseIntrinsics, 308);
	} else {
		CANT_HAPPEN_MSG("Invalid _game type.");
	}

	_inBetweenFrame = 0;
	_lerpFactor = 256;

	// Initialize _world
	_world = new World();
	_world->initMaps();

	_game = Game::createGame(getGameInfo());

	_settingMan->setDefault("ttf", false);
	_settingMan->get("ttf", _ttfOverrides);

	_settingMan->setDefault("_frameSkip", false);
	_settingMan->get("_frameSkip", _frameSkip);

	_settingMan->setDefault("_frameLimit", true);
	_settingMan->get("_frameLimit", _frameLimit);

	_settingMan->setDefault("_interpolate", true);
	_settingMan->get("_interpolate", _interpolate);

	_settingMan->setDefault("cheat", false);
	_settingMan->get("cheat", _cheatsEnabled);

	_game->loadFiles();
	_gameData->setupFontOverrides();

	// Unset the console auto paint (can't have it from here on)
	con->SetAutoPaint(0);

	// Create Midi Driver for Ultima 8
	if (getGameInfo()->type == GameInfo::GAME_U8)
		_audioMixer->openMidiOutput();

	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	newGame(saveSlot);

	_consoleGump->HideConsole();

	pout << "-- Game Initialized --" << Std::endl << Std::endl;
}

void Ultima8Engine::startupPentagramMenu() {
	con->SetAutoPaint(conAutoPaint);

	pout << Std::endl << "-- Initializing Pentagram Menu -- " << Std::endl;

	setupGame(getGameInfo("pentagram"));
	assert(_gameInfo);

	GraphicSysInit();

	// Unset the console auto paint, since we have finished initing
	con->SetAutoPaint(0);
	_consoleGump->HideConsole();

	Rect dims;
	_desktopGump->GetDims(dims);

	Gump *menugump = new PentagramMenuGump(0, 0, dims.w, dims.h);
	menugump->InitGump(0, true);
}

void Ultima8Engine::shutdown() {
	shutdownGame(false);
}

void Ultima8Engine::shutdownGame(bool reloading) {
	pout << "-- Shutting down Game -- " << Std::endl;

	// Save config here....

	_textModes.clear();

	// reset mouse cursor
	_mouse->popAllCursors();
	_mouse->pushMouseCursor();

	if (_audioMixer) {
		_audioMixer->closeMidiOutput();
		_audioMixer->reset();
	}

	FORGET_OBJECT(_world);
	_objectManager->reset();
	FORGET_OBJECT(_ucMachine);
	_kernel->reset();
	_paletteManager->reset();
	_fontManager->resetGameFonts();

	FORGET_OBJECT(_game);
	FORGET_OBJECT(_gameData);

	_desktopGump = 0;
	_consoleGump = 0;
	_gameMapGump = 0;
	_scalerGump = 0;
	_inverterGump = 0;

	_timeOffset = -(int32)Kernel::get_instance()->getFrameNum();
	_saveCount = 0;
	_hasCheated = false;

	// Generic Game
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_saveGame);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_loadGame);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_newGame);
#ifdef DEBUG
	con->RemoveConsoleCommand(Pathfinder::ConCmd_visualDebug);
#endif

	// U8 Only kind of
	con->RemoveConsoleCommand(MainActor::ConCmd_teleport);
	con->RemoveConsoleCommand(MainActor::ConCmd_mark);
	con->RemoveConsoleCommand(MainActor::ConCmd_recall);
	con->RemoveConsoleCommand(MainActor::ConCmd_listmarks);
	con->RemoveConsoleCommand(MainActor::ConCmd_maxstats);
	con->RemoveConsoleCommand(MainActor::ConCmd_heal);
	con->RemoveConsoleCommand(MainActor::ConCmd_toggleInvincibility);
	con->RemoveConsoleCommand(Ultima8Engine::ConCmd_toggleCheatMode);
	con->RemoveConsoleCommand(MainActor::ConCmd_name);
	con->RemoveConsoleCommand(MovieGump::ConCmd_play);
	con->RemoveConsoleCommand(MusicProcess::ConCmd_playMusic);
	con->RemoveConsoleCommand(InverterProcess::ConCmd_invertScreen);
	con->RemoveConsoleCommand(FastAreaVisGump::ConCmd_toggle);
	con->RemoveConsoleCommand(MiniMapGump::ConCmd_toggle);
	con->RemoveConsoleCommand(MainActor::ConCmd_useBackpack);
	con->RemoveConsoleCommand(MainActor::ConCmd_useInventory);
	con->RemoveConsoleCommand(MainActor::ConCmd_useRecall);
	con->RemoveConsoleCommand(MainActor::ConCmd_useBedroll);
	con->RemoveConsoleCommand(MainActor::ConCmd_useKeyring);
	con->RemoveConsoleCommand(MainActor::ConCmd_toggleCombat);

	// Kill Game
	CoreApp::killGame();

	pout << "-- Game Shutdown -- " << Std::endl;

	if (reloading) {
		Rect dims;
		_screen->GetSurfaceDims(dims);

		con->Print(MM_INFO, "Creating Desktop...\n");
		_desktopGump = new DesktopGump(0, 0, dims.w, dims.h);
		_desktopGump->InitGump(0);
		_desktopGump->MakeFocus();

		con->Print(MM_INFO, "Creating _scalerGump...\n");
		_scalerGump = new ScalerGump(0, 0, dims.w, dims.h);
		_scalerGump->InitGump(0);

		Rect scaled_dims;
		_scalerGump->GetDims(scaled_dims);

		con->Print(MM_INFO, "Creating Graphics Console...\n");
		_consoleGump = new ConsoleGump(0, 0, dims.w, dims.h);
		_consoleGump->InitGump(0);
		_consoleGump->HideConsole();

		con->Print(MM_INFO, "Creating Inverter...\n");
		_inverterGump = new InverterGump(0, 0, scaled_dims.w, scaled_dims.h);
		_inverterGump->InitGump(0);

		enterTextMode(_consoleGump);
	}
}

void Ultima8Engine::changeGame(istring newgame) {
	_changeGameName = newgame;
}

void Ultima8Engine::menuInitMinimal(istring gamename) {
	// Only if in the pentagram menu
	if (_gameInfo->name != "pentagram") return;
	GameInfo *info = getGameInfo(gamename);
	if (!info) info = getGameInfo("pentagram");
	assert(info);

	pout  << Std::endl << "-- Loading minimal _game data for: " << info->name << " --" << Std::endl;

	FORGET_OBJECT(_game);
	FORGET_OBJECT(_gameData);


	setupGamePaths(info);

	if (info->name == "pentagram") return;

	_gameData = new GameData(info);
	_game = Game::createGame(info);

	_game->loadFiles();
	_gameData->setupFontOverrides();

	pout << "-- Finished loading minimal--" << Std::endl << Std::endl;
}

void Ultima8Engine::DeclareArgs() {
	// parent's arguments first
	CoreApp::DeclareArgs();

	// anything else?
}

void Ultima8Engine::runGame() {
	_isRunning = true;

	int32 next_ticks = g_system->getMillis() * 3;  // Next time is right now!

	Common::Event event;
	while (_isRunning) {
		_inBetweenFrame = true;  // Will get set false if it's not an _inBetweenFrame

		if (!_frameLimit) {
			_kernel->runProcesses();
			_desktopGump->run();
			_inBetweenFrame = false;
			next_ticks = _animationRate + g_system->getMillis() * 3;
			_lerpFactor = 256;
		} else {
			int32 ticks = g_system->getMillis() * 3;
			int32 diff = next_ticks - ticks;

			while (diff < 0) {
				next_ticks += _animationRate;
				_kernel->runProcesses();
				_desktopGump->run();
#if 0
				perr << "--------------------------------------" << Std::endl;
				perr << "NEW FRAME" << Std::endl;
				perr << "--------------------------------------" << Std::endl;
#endif
				_inBetweenFrame = false;

				ticks = g_system->getMillis() * 3;

				// If frame skipping is off, we will only recalc next
				// ticks IF the frames are taking up 'way' too much time.
				if (!_frameSkip && diff <= -_animationRate * 2) next_ticks = _animationRate + ticks;

				diff = next_ticks - ticks;
				if (!_frameSkip) break;
			}

			// Calculate the lerp_factor
			_lerpFactor = ((_animationRate - diff) * 256) / _animationRate;
			//pout << "_lerpFactor: " << _lerpFactor << " framenum: " << framenum << Std::endl;
			if (!_interpolate || _kernel->isPaused() || _lerpFactor > 256)
				_lerpFactor = 256;
		}

		// get & handle all events in queue
		while (_isRunning && _events->pollEvent(event)) {
			handleEvent(event);
		}
		handleDelayedEvents();

		// Paint Screen
		paint();

		if (!_changeGameName.empty()) {
			pout << "Changing Game to: " << _changeGameName << Std::endl;

			GameInfo *info = getGameInfo(_changeGameName);

			if (info) {
				shutdownGame();

				_changeGameName.clear();

				if (setupGame(info))
					startupGame();
				else
					startupPentagramMenu();
			} else {
				perr << "Game '" << _changeGameName << "' not found" << Std::endl;
				_changeGameName.clear();
			}
		}

		if (!_errorMessage.empty()) {
			MessageBoxGump::Show(_errorTitle, _errorMessage, 0xFF8F3030);
			_errorTitle.clear();
			_errorMessage.clear();
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

// Paint the _screen
void Ultima8Engine::paint() {
	static long prev = 0;
	static long t = 0;
	static long tdiff = 0;
	static long tpaint = 0;
	long now = g_system->getMillis();

	if (!_screen) // need to worry if the graphics system has been started. Need nicer way.
		return;

	if (prev != 0)
		tdiff += now - prev;
	prev = now;
	++t;

	_painting = true;

	// Begin _painting
	_screen->BeginPainting();

	// We need to get the dims
	Rect dims;
	_screen->GetSurfaceDims(dims);

	tpaint -= g_system->getMillis();
	_desktopGump->Paint(_screen, _lerpFactor, false);
	tpaint += g_system->getMillis();

	// Draw the mouse
	_mouse->paint();

	if (_drawRenderStats) {
		static long diff = 0;
		static long fps = 0;
		static long paint = 0;
		char buf[256] = { '\0' };
		FixedWidthFont *confont = con->GetConFont();
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
		_screen->PrintTextFixed(confont, buf, dims.w - char_w * strlen(buf), v_offset);
		v_offset += confont->height;

		snprintf(buf, 255, "Paint Gumps %li ms ", paint);
		_screen->PrintTextFixed(confont, buf, dims.w - char_w * strlen(buf), v_offset);
		v_offset += confont->height;

		snprintf(buf, 255, "t %02d:%02d gh %i ", I_getTimeInMinutes(0, 0), I_getTimeInSeconds(0, 0) % 60, I_getTimeInGameHours(0, 0));
		_screen->PrintTextFixed(confont, buf, dims.w - char_w * strlen(buf), v_offset);
		v_offset += confont->height;
	}

	// End _painting
	_screen->EndPainting();

	_painting = false;
}

void Ultima8Engine::GraphicSysInit() {
	_settingMan->setDefault("_fullScreen", false);
	_settingMan->setDefault("width", SCREEN_WIDTH);
	_settingMan->setDefault("height", SCREEN_HEIGHT);
	_settingMan->setDefault("bpp", 32);

	bool new_fullscreen;
	int width, height, bpp;
	_settingMan->get("_fullScreen", new_fullscreen);
	_settingMan->get("width", width);
	_settingMan->get("height", height);
	_settingMan->get("bpp", bpp);

#ifdef UNDER_CE
	width = 240;
	height = 320;
#endif

#if 0
	// store values in user's config file
	_settingMan->set("width", width);
	_settingMan->set("height", height);
	_settingMan->set("bpp", bpp);
	_settingMan->set("_fullScreen", new_fullscreen);
#endif

	if (_screen) {
		Rect old_dims;
		_screen->GetSurfaceDims(old_dims);
		if (new_fullscreen == _fullScreen && width == old_dims.w && height == old_dims.h) return;
		bpp = RenderSurface::format.s_bpp;

		delete _screen;
	}
	_screen = 0;

	_fullScreen = new_fullscreen;

	// Set Screen Resolution
	con->Printf(MM_INFO, "Setting Video Mode %dx%dx%d...\n", width, height, bpp);

	RenderSurface *new_screen = RenderSurface::SetVideoMode(width, height, bpp);

	if (!new_screen) {
		perr << "Unable to set new video mode. Trying 640x480x32" << Std::endl;
		new_screen = RenderSurface::SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
	}

	if (!new_screen) {
		error("Unable to set video mode");
	}

	if (_desktopGump) {
		_paletteManager->RenderSurfaceChanged(new_screen);
		static_cast<DesktopGump *>(_desktopGump)->RenderSurfaceChanged(new_screen);
		_screen = new_screen;
		paint();
		return;
	}

	// setup normal mouse cursor
	con->Print(MM_INFO, "Loading Default Mouse Cursor...\n");
	_mouse->setup();

	Std::string alt_confont;
	bool confont_loaded = false;

	if (_settingMan->get("console_font", alt_confont)) {
		con->Print(MM_INFO, "Alternate console font found...\n");
		confont_loaded = LoadConsoleFont(alt_confont);
	}

	if (!confont_loaded) {
		con->Print(MM_INFO, "Loading default console font...\n");
		if (!LoadConsoleFont("@data/fixedfont.ini")) {
			error("Failed to load console font. Exiting");
		}
	}

	_desktopGump = new DesktopGump(0, 0, width, height);
	_desktopGump->InitGump(0);
	_desktopGump->MakeFocus();

	_scalerGump = new ScalerGump(0, 0, width, height);
	_scalerGump->InitGump(0);

	_consoleGump = new ConsoleGump(0, 0, width, height);
	_consoleGump->InitGump(0);

	Rect scaled_dims;
	_scalerGump->GetDims(scaled_dims);

	_inverterGump = new InverterGump(0, 0, scaled_dims.w, scaled_dims.h);
	_inverterGump->InitGump(0);

	_screen = new_screen;

	// Show the splash _screen immediately now that the _screen has been set up
	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	if (saveSlot == -1)
		showSplashScreen();

	bool ttf_antialiasing = true;
	_settingMan->setDefault("ttf_antialiasing", true);
	_settingMan->get("ttf_antialiasing", ttf_antialiasing);

	_fontManager = new FontManager(ttf_antialiasing);
	_paletteManager = new PaletteManager(new_screen);

	// TODO: assign names to these fontnumbers somehow
	_fontManager->loadTTFont(0, "Vera.ttf", 18, 0xFFFFFF, 0);
	_fontManager->loadTTFont(1, "VeraBd.ttf", 12, 0xFFFFFF, 0);
	// GameWidget's version number information:
	_fontManager->loadTTFont(2, "Vera.ttf", 8, 0xA0A0A0, 0);

	bool faded_modal = true;
	_settingMan->setDefault("fadedModal", faded_modal);
	_settingMan->get("fadedModal", faded_modal);
	DesktopGump::SetFadedModal(faded_modal);

	paint();
}

void Ultima8Engine::changeVideoMode(int width, int height, int new_fullscreen) {
	if (new_fullscreen == -2) _settingMan->set("_fullScreen", !_fullScreen);
	else if (new_fullscreen == 0) _settingMan->set("_fullScreen", false);
	else if (new_fullscreen == 1) _settingMan->set("_fullScreen", true);

	if (width > 0) _settingMan->set("width", width);
	if (height > 0) _settingMan->set("height", height);

	GraphicSysInit();
}

bool Ultima8Engine::LoadConsoleFont(Std::string confontini) {
	// try to load the file
	con->Printf(MM_INFO, "Loading console font config: %s... ", confontini.c_str());
	if (_configFileMan->readConfigFile(confontini, "confont", true))
		pout << "Ok" << Std::endl;
	else {
		pout << "Failed" << Std::endl;
		return false;
	}

	FixedWidthFont *confont = FixedWidthFont::Create("confont");

	if (!confont) {
		perr << "Failed to load Console Font." << Std::endl;
		return false;
	}

	con->SetConFont(confont);

	return true;
}

void Ultima8Engine::enterTextMode(Gump *gump) {
	uint16 key;
	for (key = 0; key < HID_LAST; ++key) {
		if (_down[key]) {
			_down[key] = false;
			_lastDown[key] = false;
			_hidManager->handleEvent((HID_Key)key, HID_EVENT_RELEASE);
		}
	}

	if (!_textModes.empty()) {
		_textModes.remove(gump->getObjId());
	} else {
#ifdef TODO
		SDL_EnableUNICODE(1);
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#endif
	}
	_textModes.push_front(gump->getObjId());
}

void Ultima8Engine::leaveTextMode(Gump *gump) {
	if (_textModes.empty()) return;
	_textModes.remove(gump->getObjId());
	if (_textModes.empty()) {
#ifdef TODO
		SDL_EnableUNICODE(0);
		SDL_EnableKeyRepeat(0, 0);
#endif
	}
}

void Ultima8Engine::handleEvent(const Common::Event &event) {
	uint32 now = g_system->getMillis();
	HID_Key key = HID_LAST;
	uint16 evn = HID_EVENT_LAST;
	bool handled = false;

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		key = HID_translateSDLKey(event.kbd.keycode);
		evn = HID_translateSDLKeyFlags(event.kbd.flags);
		break;
	case Common::EVENT_KEYUP:
		// Any system keys not in the bindings can be handled here
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
		_isRunning = false;
		break;

	default:
		break;
	}

	if (_mouse->dragging() == Mouse::DRAG_NOT && evn == HID_EVENT_DEPRESS) {
		if (_hidManager->handleEvent(key, HID_EVENT_PREEMPT))
			return;
	}

	// Text mode input. A few hacks here
	if (!_textModes.empty()) {
		Gump *gump = 0;

		while (!_textModes.empty()) {
			gump = p_dynamic_cast<Gump *>(_objectManager->getObject(_textModes.front()));
			if (gump) break;

			_textModes.pop_front();
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
		Shared::MouseButton button = Shared::BUTTON_LEFT;
		if (event.type == Common::EVENT_RBUTTONDOWN)
			button = Shared::BUTTON_RIGHT;
		else if (event.type == Common::EVENT_MBUTTONDOWN)
			button = Shared::BUTTON_MIDDLE;

		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		if (_mouse->buttonDown(button))
			handled = true;
		break;
	}

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_MBUTTONUP:
	case Common::EVENT_RBUTTONUP: {
		Shared::MouseButton button = Shared::BUTTON_LEFT;
		if (event.type == Common::EVENT_RBUTTONUP)
			button = Shared::BUTTON_RIGHT;
		else if (event.type == Common::EVENT_MBUTTONUP)
			button = Shared::BUTTON_MIDDLE;

		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		if (_mouse->buttonUp(button))
			handled = true;
		break;
	}

	case Common::EVENT_KEYDOWN:
		if (_mouse->dragging() != Mouse::DRAG_NOT)
			break;

		// Any special key handling goes here
		if ((event.kbd.keycode == Common::KEYCODE_x || event.kbd.keycode == Common::KEYCODE_x) &&
			(event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META)) != 0)
			ForceQuit();
		break;

	default:
		break;
	}

	if (_mouse->dragging() == Mouse::DRAG_NOT && !handled) {
		if (_hidManager->handleEvent(key, evn))
			handled = true;
		if (evn == HID_EVENT_DEPRESS) {
			_down[key] = true;
			if (now - _lastDown[key] < DOUBLE_CLICK_TIMEOUT &&
				_lastDown[key] != 0) {
				if (_hidManager->handleEvent(key, HID_EVENT_DOUBLE))
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
			_hidManager->handleEvent((HID_Key)key, HID_EVENT_CLICK);
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
	ods->write4(_saveCount);
	ods->write4(getGameTimeInSeconds());

	uint8 c = (_hasCheated ? 1 : 0);
	ods->write1(c);

	// write _game-specific info
	_game->writeSaveInfo(ods);
}

bool Ultima8Engine::canSaveGameStateCurrently(bool isAutosave) {
	if (_desktopGump->FindGump<ModalGump>())
		// Can't save when a modal gump is open
		return false;

	// Don't allow saving when avatar is dead.
	MainActor *av = getMainActor();
	if (!av || (av->getActorFlags() & Actor::ACT_DEAD))
		return false;

	return true;
}

bool Ultima8Engine::saveGame(int slot, const Std::string &desc, bool ignore_modals) {
	// Don't allow saving with Modals open
	if (!ignore_modals && _desktopGump->FindGump<ModalGump>()) {
		pout << "Can't save: modal gump open." << Std::endl;
		return false;
	}

	// Don't allow saving when avatar is dead.
	// (Avatar is flagged dead by usecode when you finish the _game as well.)
	MainActor *av = getMainActor();
	if (!av || (av->getActorFlags() & Actor::ACT_DEAD)) {
		pout << "Can't save: _game over." << Std::endl;
		return false;
	}

	_settingMan->set("lastSave", slot);

	return saveGameState(slot, desc).getCode() == Common::kNoError;
}

Common::Error Ultima8Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	// Hack - don't save mouse over status for gumps
	Gump *gump = _mouse->getMouseOverGump();
	if (gump)
		gump->OnMouseLeft();

	_saveCount++;

	SavegameWriter *sgw = new SavegameWriter(stream);

	// We'll make it 2KB initially
	OAutoBufferDataSource buf(2048);

	_gameInfo->save(&buf);
	sgw->writeFile("GAME", &buf);
	buf.clear();

	writeSaveInfo(&buf);
	sgw->writeFile("INFO", &buf);
	buf.clear();

	_kernel->save(&buf);
	sgw->writeFile("KERNEL", &buf);
	buf.clear();

	_objectManager->save(&buf);
	sgw->writeFile("OBJECTS", &buf);
	buf.clear();

	_world->save(&buf);
	sgw->writeFile("WORLD", &buf);
	buf.clear();

	_world->saveMaps(&buf);
	sgw->writeFile("MAPS", &buf);
	buf.clear();

	_world->getCurrentMap()->save(&buf);
	sgw->writeFile("CURRENTMAP", &buf);
	buf.clear();

	_ucMachine->saveStrings(&buf);
	sgw->writeFile("UCSTRINGS", &buf);
	buf.clear();

	_ucMachine->saveGlobals(&buf);
	sgw->writeFile("UCGLOBALS", &buf);
	buf.clear();

	_ucMachine->saveLists(&buf);
	sgw->writeFile("UCLISTS", &buf);
	buf.clear();

	save(&buf);
	sgw->writeFile("APP", &buf);
	buf.clear();

	sgw->finish();

	delete sgw;

	// Restore mouse over
	if (gump) gump->OnMouseOver();

	pout << "Done" << Std::endl;

	return Common::kNoError;
}

void Ultima8Engine::resetEngine() {
	con->Print(MM_INFO, "-- Resetting Engine --\n");

	// kill music
	if (_audioMixer) _audioMixer->reset();

	// now, reset everything (order matters)
	_world->reset();
	_ucMachine->reset();
	// ObjectManager, Kernel have to be last, because they kill
	// all processes/objects
	_objectManager->reset();
	_kernel->reset();
	_paletteManager->resetTransforms();

	// Reset thet gumps
	_desktopGump = 0;
	_consoleGump = 0;
	_gameMapGump = 0;
	_scalerGump = 0;
	_inverterGump = 0;

	_textModes.clear();

	// reset mouse cursor
	_mouse->popAllCursors();
	_mouse->pushMouseCursor();

	// FIXME: This breaks loading processes if this process gets an ID
	//        also present in a savegame.
	// _kernel->addProcess(new JoystickCursorProcess(JOY1, 0, 1));

	_timeOffset = -(int32)Kernel::get_instance()->getFrameNum();
	_inversion = 0;
	_saveCount = 0;
	_hasCheated = false;

	con->Print(MM_INFO, "-- Engine Reset --\n");
}

void Ultima8Engine::setupCoreGumps() {
	con->Print(MM_INFO, "Setting up core _game gumps...\n");

	Rect dims;
	_screen->GetSurfaceDims(dims);

	con->Print(MM_INFO, "Creating Desktop...\n");
	_desktopGump = new DesktopGump(0, 0, dims.w, dims.h);
	_desktopGump->InitGump(0);
	_desktopGump->MakeFocus();

	con->Print(MM_INFO, "Creating _scalerGump...\n");
	_scalerGump = new ScalerGump(0, 0, dims.w, dims.h);
	_scalerGump->InitGump(0);

	Rect scaled_dims;
	_scalerGump->GetDims(scaled_dims);

	con->Print(MM_INFO, "Creating Graphics Console...\n");
	_consoleGump = new ConsoleGump(0, 0, dims.w, dims.h);
	_consoleGump->InitGump(0);
	_consoleGump->HideConsole();

	con->Print(MM_INFO, "Creating Inverter...\n");
	_inverterGump = new InverterGump(0, 0, scaled_dims.w, scaled_dims.h);
	_inverterGump->InitGump(0);

	con->Print(MM_INFO, "Creating GameMapGump...\n");
	_gameMapGump = new GameMapGump(0, 0, scaled_dims.w, scaled_dims.h);
	_gameMapGump->InitGump(0);


	// TODO: clean this up
	assert(_desktopGump->getObjId() == 256);
	assert(_scalerGump->getObjId() == 257);
	assert(_consoleGump->getObjId() == 258);
	assert(_inverterGump->getObjId() == 259);
	assert(_gameMapGump->getObjId() == 260);


	for (uint16 i = 261; i < 384; ++i)
		_objectManager->reserveObjId(i);
}

bool Ultima8Engine::newGame(int saveSlot) {
	con->Print(MM_INFO, "Starting New Game...\n");

	resetEngine();

	setupCoreGumps();

	_game->startGame();

	con->Print(MM_INFO, "Create Camera...\n");
	CameraProcess::SetCameraProcess(new CameraProcess(1)); // Follow Avatar

	con->Print(MM_INFO, "Create persistent Processes...\n");
	_avatarMoverProcess = new AvatarMoverProcess();
	_kernel->addProcess(_avatarMoverProcess);

	_kernel->addProcess(new HealProcess());

	_kernel->addProcess(new SchedulerProcess());

	if (_audioMixer) _audioMixer->createProcesses();

	//	av->teleport(40, 16240, 15240, 64); // central Tenebrae
	//	av->teleport(3, 11391, 1727, 64); // docks, near gate
	//	av->teleport(39, 16240, 15240, 64); // West Tenebrae
	//	av->teleport(41, 12000, 15000, 64); // East Tenebrae
	//	av->teleport(8, 14462, 15178, 48); // before entrance to Mythran's house
	//	av->teleport(40, 13102,9474,48); // entrance to Mordea's throne room
	//	av->teleport(54, 14783,5959,8); // shrine of the Ancient Ones; Hanoi
	//	av->teleport(5, 5104,22464,48); // East road (tenebrae end)

	_game->startInitialUsecode(saveSlot);

	_settingMan->set("lastSave", saveSlot);

	return true;
}

void Ultima8Engine::syncSoundSettings() {
	UltimaEngine::syncSoundSettings();

	// Update music volume
	AudioMixer *audioMixer = AudioMixer::get_instance();
	MidiPlayer *midiPlayer = audioMixer ? audioMixer->getMidiPlayer() : nullptr;
	if (midiPlayer)
		midiPlayer->setVolume(_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
}

Common::Error Ultima8Engine::loadGameStream(Common::SeekableReadStream *stream) {
	SavegameReader *sg = new SavegameReader(stream);
	SavegameReader::State state = sg->isValid();
	if (state == SavegameReader::SAVE_CORRUPT) {
		Error("Invalid or corrupt savegame", "Error Loading savegame");
		delete sg;
		_settingMan->set("lastSave", "");
		return Common::kReadingFailed;
	}

	if (state != SavegameReader::SAVE_VALID) {
		Error("Unsupported savegame version", "Error Loading savegame");
		delete sg;
		_settingMan->set("lastSave", "");
		return Common::kReadingFailed;
	}

	IDataSource *ds;
	GameInfo saveinfo;
	ds = sg->getDataSource("GAME");
	uint32 version = sg->getVersion();
	bool ok = saveinfo.load(ds, version);

	if (!ok) {
		Error("Invalid or corrupt savegame: missing GameInfo", "Error Loading savegame");
		delete sg;
		return Common::kReadingFailed;
	}

	if (!_gameInfo->match(saveinfo)) {
		Std::string message = "Game mismatch\n";
		message += "Running _game: " + _gameInfo->getPrintDetails()  + "\n";
		message += "Savegame    : " + saveinfo.getPrintDetails();

#ifdef DEBUG
		bool ignore;
		_settingMan->setDefault("ignore_savegame_mismatch", false);
		_settingMan->get("ignore_savegame_mismatch", ignore);

		if (!ignore) {
			Error(message, "Error Loading savegame " + filename);
			delete sg;
			return false;
		}
		perr << message << Std::endl;
#else
		_settingMan->set("lastSave", "");
		Error(message, "Error Loading savegame");
		return Common::kReadingFailed;
#endif
	}

	resetEngine();

	setupCoreGumps();

	// and load everything back (order matters)
	bool totalok = true;

	Std::string message;

	// UCSTRINGS, UCGLOBALS, UCLISTS don't depend on anything else,
	// so load these first
	ds = sg->getDataSource("UCSTRINGS");
	ok = _ucMachine->loadStrings(ds, version);
	totalok &= ok;
	perr << "UCSTRINGS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "UCSTRINGS: failed\n";
	delete ds;

	ds = sg->getDataSource("UCGLOBALS");
	ok = _ucMachine->loadGlobals(ds, version);
	totalok &= ok;
	perr << "UCGLOBALS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "UCGLOBALS: failed\n";
	delete ds;

	ds = sg->getDataSource("UCLISTS");
	ok = _ucMachine->loadLists(ds, version);
	totalok &= ok;
	perr << "UCLISTS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "UCLISTS: failed\n";
	delete ds;

	// KERNEL must be before OBJECTS, for the egghatcher
	// KERNEL must be before APP, for the _avatarMoverProcess
	ds = sg->getDataSource("KERNEL");
	ok = _kernel->load(ds, version);
	totalok &= ok;
	perr << "KERNEL: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "KERNEL: failed\n";
	delete ds;

	ds = sg->getDataSource("APP");
	ok = load(ds, version);
	totalok &= ok;
	perr << "APP: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "APP: failed\n";
	delete ds;

	// WORLD must be before OBJECTS, for the egghatcher
	ds = sg->getDataSource("WORLD");
	ok = _world->load(ds, version);
	totalok &= ok;
	perr << "WORLD: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "WORLD: failed\n";
	delete ds;

	ds = sg->getDataSource("CURRENTMAP");
	ok = _world->getCurrentMap()->load(ds, version);
	totalok &= ok;
	perr << "CURRENTMAP: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "CURRENTMAP: failed\n";
	delete ds;

	ds = sg->getDataSource("OBJECTS");
	ok = _objectManager->load(ds, version);
	totalok &= ok;
	perr << "OBJECTS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "OBJECTS: failed\n";
	delete ds;

	ds = sg->getDataSource("MAPS");
	ok = _world->loadMaps(ds, version);
	totalok &= ok;
	perr << "MAPS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "MAPS: failed\n";
	delete ds;

	if (!totalok) {
		Error(message, "Error Loading savegame", true);
		delete sg;
		return Common::kReadingFailed;
	}

	pout << "Done" << Std::endl;

	_settingMan->set("lastSave", -1);

	delete sg;
	return Common::kNoError;
}

void Ultima8Engine::Error(Std::string message, Std::string title, bool exit_to_menu) {
	if (title.empty()) title = exit_to_menu ? "Fatal Game Error" : "Error";

	perr << title << ": " << message << Std::endl;

	_errorMessage = message;
	_errorTitle = title;

	if (exit_to_menu) {
		_changeGameName = "pentagram";
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

	assert(_desktopGump);

	if (gump->IsOfType<ShapeViewerGump>() || gump->IsOfType<MiniMapGump>() ||
		gump->IsOfType<ConsoleGump>() || gump->IsOfType<ScalerGump>() ||
		gump->IsOfType<PentagramMenuGump>() || gump->IsOfType<MessageBoxGump>()// ||
		//(_ttfOverrides && (gump->IsOfType<BarkGump>() ||
		//                gump->IsOfType<AskGump>()))
		) {
		//		pout << "adding to desktopgump: "; gump->dumpInfo();
		_desktopGump->AddChild(gump);
	} else if (gump->IsOfType<GameMapGump>()) {
		//		pout << "adding to invertergump: "; gump->dumpInfo();
		_inverterGump->AddChild(gump);
	} else if (gump->IsOfType<InverterGump>()) {
		//		pout << "adding to _scalerGump: "; gump->dumpInfo();
		_scalerGump->AddChild(gump);
	} else if (gump->IsOfType<DesktopGump>()) {
	} else {
		//		pout << "adding to _scalerGump: "; gump->dumpInfo();
		_scalerGump->AddChild(gump);
	}
}

uint32 Ultima8Engine::getGameTimeInSeconds() {
	// 1 second per every 30 frames
	return (Kernel::get_instance()->getFrameNum() + _timeOffset) / 30; // constant!
}


void Ultima8Engine::save(ODataSource *ods) {
	uint8 s = (_avatarInStasis ? 1 : 0);
	ods->write1(s);

	int32 absoluteTime = Kernel::get_instance()->getFrameNum() + _timeOffset;
	ods->write4(static_cast<uint32>(absoluteTime));
	ods->write2(_avatarMoverProcess->getPid());

	Palette *pal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	for (int i = 0; i < 12; i++) ods->write2(pal->matrix[i]);
	ods->write2(pal->transform);

	ods->write2(static_cast<uint16>(_inversion));

	ods->write4(_saveCount);

	uint8 c = (_hasCheated ? 1 : 0);
	ods->write1(c);
}

bool Ultima8Engine::load(IDataSource *ids, uint32 version) {
	_avatarInStasis = (ids->read1() != 0);

	// no gump should be moused over after load
	_mouse->resetMouseOverGump();

	int32 absoluteTime = static_cast<int32>(ids->read4());
	_timeOffset = absoluteTime - Kernel::get_instance()->getFrameNum();

	uint16 amppid = ids->read2();
	_avatarMoverProcess = p_dynamic_cast<AvatarMoverProcess *>(Kernel::get_instance()->getProcess(amppid));

	int16 matrix[12];
	for (int i = 0; i < 12; i++)
		matrix[i] = ids->read2();

	PaletteManager::get_instance()->transformPalette(PaletteManager::Pal_Game, matrix);
	Palette *pal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	pal->transform = static_cast<PalTransforms>(ids->read2());

	_inversion = ids->read2();

	_saveCount = ids->read4();

	_hasCheated = (ids->read1() != 0);

	return true;
}

//
// Console Commands
//

void Ultima8Engine::ConCmd_saveGame(const Console::ArgvType &argv) {
	if (argv.size() == 2) {
		// Save a _game with the given name into the quicksave slot
		Ultima8Engine::get_instance()->saveGame(1, argv[1]);
	} else {
		Ultima8Engine::get_instance()->saveGameDialog();
	}
}

void Ultima8Engine::ConCmd_loadGame(const Console::ArgvType &argv) {
	if (argv.size() == 2) {
		// Load a _game from the quicksave slot. The second parameter is ignored,
		// it just needs to be present to differentiate from showing the GUI load dialog
		Ultima8Engine::get_instance()->loadGameState(1);
	} else {
		Ultima8Engine::get_instance()->loadGameDialog();
	}
}

void Ultima8Engine::ConCmd_newGame(const Console::ArgvType &argv) {
	Ultima8Engine::get_instance()->newGame();
}

void Ultima8Engine::ConCmd_quit(const Console::ArgvType &argv) {
	Ultima8Engine::get_instance()->_isRunning = false;
}

void Ultima8Engine::ConCmd_drawRenderStats(const Console::ArgvType &argv) {
	if (argv.size() == 1) {
		pout << "Ultima8Engine::_drawRenderStats = " << Ultima8Engine::get_instance()->_drawRenderStats << Std::endl;
	} else {
		Ultima8Engine::get_instance()->_drawRenderStats = Std::strtol(argv[1].c_str(), 0, 0) != 0;
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
		pout << "Current _game is: " << Ultima8Engine::get_instance()->_gameInfo->name << Std::endl;
	} else {
		Ultima8Engine::get_instance()->changeGame(argv[1]);
	}
}

void Ultima8Engine::ConCmd_listGames(const Console::ArgvType &argv) {
	Ultima8Engine *app = Ultima8Engine::get_instance();
	Std::vector<istring> games;
	games = app->_settingMan->listGames();
	Std::vector<istring>::iterator iter;
	for (iter = games.begin(); iter != games.end(); ++iter) {
		istring _game = *iter;
		GameInfo *info = app->getGameInfo(_game);
		con->Printf(MM_INFO, "%s: ", _game.c_str());
		if (info) {
			Std::string details = info->getPrintDetails();
			con->Print(MM_INFO, details.c_str());
		} else {
			con->Print(MM_INFO, "(unknown)");
		}
		con->Print(MM_INFO, "\n");
	}
}

void Ultima8Engine::ConCmd_setVideoMode(const Console::ArgvType &argv) {
	int _fullScreen = -1;

	//if (argv.size() == 4) {
	//  if (argv[3] == "_fullScreen") _fullScreen = 1;
	//  else _fullScreen = 0;
	//} else
	if (argv.size() != 3) {
		//pout << "Usage: Ultima8Engine::setVidMode width height [_fullScreen/windowed]" << Std::endl;
		pout << "Usage: Ultima8Engine::setVidMode width height" << Std::endl;
		return;
	}

	Ultima8Engine::get_instance()->changeVideoMode(strtol(argv[1].c_str(), 0, 0), strtol(argv[2].c_str(), 0, 0), _fullScreen);
}

void Ultima8Engine::ConCmd_toggleAvatarInStasis(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->toggleAvatarInStasis();
	pout << "_avatarInStasis = " << g->isAvatarInStasis() << Std::endl;
}

void Ultima8Engine::ConCmd_togglePaintEditorItems(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->togglePaintEditorItems();
	pout << "_paintEditorItems = " << g->isPaintEditorItems() << Std::endl;
}

void Ultima8Engine::ConCmd_toggleShowTouchingItems(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->toggleShowTouchingItems();
	pout << "ShowTouchingItems = " << g->isShowTouchingItems() << Std::endl;
}

void Ultima8Engine::ConCmd_closeItemGumps(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->getDesktopGump()->CloseItemDependents();
}

void Ultima8Engine::ConCmd_toggleCheatMode(const Console::ArgvType &argv) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->setCheatMode(!g->areCheatsEnabled());
	pout << "Cheats = " << g->areCheatsEnabled() << Std::endl;
}

void Ultima8Engine::ConCmd_memberVar(const Console::ArgvType &argv) {
	if (argv.size() == 1) {
		pout << "Usage: Ultima8Engine::memberVar <member> [newvalue] [updateini]" << Std::endl;
		return;
	}

	Ultima8Engine *g = Ultima8Engine::get_instance();

	// Set the pointer to the correct type
	bool *b = 0;
	int *i = 0;
	Std::string *str = 0;
	istring *istr = 0;

	// ini entry name if supported
	const char *ini = 0;

	if (argv[1] == "_frameLimit") {
		b = &g->_frameLimit;
		ini = "_frameLimit";
	} else if (argv[1] == "_frameSkip") {
		b = &g->_frameSkip;
		ini = "_frameSkip";
	} else if (argv[1] == "_interpolate") {
		b = &g->_interpolate;
		ini = "_interpolate";
	} else {
		pout << "Unknown member: " << argv[1] << Std::endl;
		return;
	}

	// Set the value
	if (argv.size() >= 3) {
		if (b) *b = (argv[2] == "yes" || argv[2] == "true");
		else if (istr) *istr = argv[2];
		else if (i) *i = Std::strtol(argv[2].c_str(), 0, 0);
		else if (str) *str = argv[2];

		// Set config value
		if (argv.size() >= 4 && ini && *ini && (argv[3] == "yes" || argv[3] == "true")) {
			if (b) g->_settingMan->set(ini, *b);
			else if (istr) g->_settingMan->set(ini, *istr);
			else if (i) g->_settingMan->set(ini, *i);
			else if (str) g->_settingMan->set(ini, *str);
		}
	}

	// Print the value
	pout << "Ultima8Engine::" << argv[1] << " = ";
	if (b) pout << ((*b) ? "true" : "false");
	else if (istr) pout << *istr;
	else if (i) pout << *i;
	else if (str) pout << *str;
	pout << Std::endl;

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
	if (get_instance()->_avatarInStasis)
		return 1;
	else
		return 0;
}

uint32 Ultima8Engine::I_getTimeInGameHours(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	// 900 seconds per _game hour
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

	// 1 _game hour per every 27000 frames
	int32   absolute = newhour * 27000;
	get_instance()->_timeOffset = absolute - Kernel::get_instance()->getFrameNum();

	return 0;
}

uint32 Ultima8Engine::I_closeItemGumps(const uint8 *args, unsigned int /*argsize*/) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->getDesktopGump()->CloseItemDependents();

	return 0;
}

bool Ultima8Engine::isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) {
	folder = "ultima8";
	majorVersion = 1;
	minorVersion = 0;
	return true;
}

Graphics::Screen *Ultima8Engine::getScreen() const {
	Graphics::Screen *scr = dynamic_cast<Graphics::Screen *>(_screen->getRawSurface());
	assert(scr);
	return scr;
}

void Ultima8Engine::showSplashScreen() {
	Image::PNGDecoder png;
	Common::File f;

	// Get splash _screen image
	if (!f.open("data/pentagram.png") || !png.loadStream(f))
		return;

	// Blit the splash image to the _screen
	Graphics::Screen *scr = Ultima8Engine::get_instance()->getScreen();
	const Graphics::Surface *srcSurface = png.getSurface();

	scr->transBlitFrom(*srcSurface, Common::Rect(0, 0, srcSurface->w, srcSurface->h),
		Common::Rect(0, 0, scr->w, scr->h));
	scr->update();

	// Pause to allow the image to be seen
	g_system->delayMillis(2000);
}

} // End of namespace Ultima8
} // End of namespace Ultima
