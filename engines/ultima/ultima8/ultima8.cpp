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

 // TODO: !! a lot of these includes are just for some hacks... clean up sometime
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/filesys/file_system.h"
#include "ultima/ultima8/conf/setting_manager.h"
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/games/start_u8_process.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/kernel/memory_manager.h"
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
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/gumps/inverter_gump.h"
#include "ultima/ultima8/gumps/scaler_gump.h"
#include "ultima/ultima8/gumps/fast_area_vis_gump.h"
#include "ultima/ultima8/gumps/minimap_gump.h"
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/gumps/menu_gump.h"

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
#include "ultima/ultima8/meta_engine.h"

namespace Ultima {
namespace Ultima8 {

using Std::string;

// a bit of a hack to prevent having to write a load function for
// every process
template<class T>
struct ProcessLoader {
	static Process *load(IDataSource *ids, uint32 version) {
		T *p = new T();
		bool ok = p->loadData(ids, version);
		if (!ok) {
			delete p;
			p = nullptr;
		}
		return p;
	}
};

DEFINE_RUNTIME_CLASSTYPE_CODE(Ultima8Engine, CoreApp)

Ultima8Engine::Ultima8Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Shared::UltimaEngine(syst, gameDesc), CoreApp(gameDesc), _saveCount(0), _game(nullptr),
		_kernel(nullptr), _objectManager(nullptr), _mouse(nullptr), _ucMachine(nullptr),
		_screen(nullptr), _fontManager(nullptr), _paletteManager(nullptr), _gameData(nullptr),
		_world(nullptr), _desktopGump(nullptr), _gameMapGump(nullptr), _avatarMoverProcess(nullptr),
		_frameSkip(false), _frameLimit(true), _interpolate(true), _animationRate(100),
		_avatarInStasis(false), _paintEditorItems(false), _inversion(0), _painting(false),
		_showTouching(false), _timeOffset(0), _hasCheated(false), _cheatsEnabled(false),
		_ttfOverrides(false), _audioMixer(0) {
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

	return true;
}

void Ultima8Engine::deinitialize() {
}

void Ultima8Engine::startup() {
	setDebugger(new Debugger());
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
	_kernel->addProcessLoader("AmbushProcess",
		ProcessLoader<AmbushProcess>::load);

	_objectManager = new ObjectManager();
	_mouse = new Mouse();

	GraphicSysInit();

	// Audio Mixer
	_audioMixer = new AudioMixer(_mixer);

	pout << "-- Pentagram Initialized -- " << Std::endl << Std::endl;

	// We Attempt to startup _game
	setupGameList();
	GameInfo *info = getDefaultGame();
	if (setupGame(info))
		startupGame();
	else
		// Couldn't setup the game, should never happen?
		CANT_HAPPEN_MSG("default game failed to initialize");

	paint();
}

void Ultima8Engine::startupGame() {
	pout  << Std::endl << "-- Initializing Game: " << _gameInfo->_name << " --" << Std::endl;

	GraphicSysInit();

	_gameData = new GameData(_gameInfo);

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

	_settingMan->setDefault("frameSkip", false);
	_settingMan->get("frameSkip", _frameSkip);

	_settingMan->setDefault("frameLimit", true);
	_settingMan->get("frameLimit", _frameLimit);

	_settingMan->setDefault("interpolate", true);
	_settingMan->get("interpolate", _interpolate);

	_settingMan->setDefault("cheat", false);
	_settingMan->get("cheat", _cheatsEnabled);

	_game->loadFiles();
	_gameData->setupFontOverrides();

	// Create Midi Driver for Ultima 8
	if (getGameInfo()->_type == GameInfo::GAME_U8)
		_audioMixer->openMidiOutput();

	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	if (saveSlot == -1)
		_settingMan->get("lastSave", saveSlot);

	newGame(saveSlot);

	pout << "-- Game Initialized --" << Std::endl << Std::endl;
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

	FORGET_OBJECT(_world);
	_objectManager->reset();
	FORGET_OBJECT(_ucMachine);
	_kernel->reset();
	_paletteManager->reset();
	_fontManager->resetGameFonts();

	FORGET_OBJECT(_game);
	FORGET_OBJECT(_gameData);

	if (_audioMixer) {
		_audioMixer->closeMidiOutput();
		_audioMixer->reset();
	}

	_desktopGump = nullptr;
	_gameMapGump = nullptr;
	_scalerGump = nullptr;
	_inverterGump = nullptr;

	_timeOffset = -(int32)Kernel::get_instance()->getFrameNum();
	_saveCount = 0;
	_hasCheated = false;

	// Kill Game
	CoreApp::killGame();

	pout << "-- Game Shutdown -- " << Std::endl;

	if (reloading) {
		Rect dims;
		_screen->GetSurfaceDims(dims);

		debugN(MM_INFO, "Creating Desktop...\n");
		_desktopGump = new DesktopGump(0, 0, dims.w, dims.h);
		_desktopGump->InitGump(0);
		_desktopGump->MakeFocus();

		debugN(MM_INFO, "Creating _scalerGump...\n");
		_scalerGump = new ScalerGump(0, 0, dims.w, dims.h);
		_scalerGump->InitGump(0);

		Rect scaled_dims;
		_scalerGump->GetDims(scaled_dims);

		debugN(MM_INFO, "Creating Inverter...\n");
		_inverterGump = new InverterGump(0, 0, scaled_dims.w, scaled_dims.h);
		_inverterGump->InitGump(0);
	}
}

void Ultima8Engine::changeGame(istring newgame) {
	_changeGameName = newgame;
}

void Ultima8Engine::menuInitMinimal(istring gamename) {
	// Only if in the pentagram menu
	if (_gameInfo->_name != "pentagram") return;
	GameInfo *info = getGameInfo(gamename);
	if (!info) info = getGameInfo("pentagram");
	assert(info);

	pout  << Std::endl << "-- Loading minimal _game data for: " << info->_name << " --" << Std::endl;

	FORGET_OBJECT(_game);
	FORGET_OBJECT(_gameData);


	setupGamePaths(info);

	if (info->_name == "pentagram") return;

	_gameData = new GameData(info);
	_game = Game::createGame(info);

	_game->loadFiles();
	_gameData->setupFontOverrides();

	pout << "-- Finished loading minimal--" << Std::endl << Std::endl;
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
					CANT_HAPPEN_MSG("Failed to start up game with valid info.");
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

	// End _painting
	_screen->EndPainting();

	_painting = false;
}

void Ultima8Engine::GraphicSysInit() {
	_settingMan->setDefault("width", DEFAULT_SCREEN_WIDTH);
	_settingMan->setDefault("height", DEFAULT_SCREEN_HEIGHT);
	_settingMan->setDefault("bpp", 16);

	int width, height, bpp;
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
#endif

	if (_screen) {
		Rect old_dims;
		_screen->GetSurfaceDims(old_dims);
		if (width == old_dims.w && height == old_dims.h)
			return;
		bpp = RenderSurface::_format.s_bpp;

		delete _screen;
	}
	_screen = nullptr;

	// Set Screen Resolution
	debugN(MM_INFO, "Setting Video Mode %dx%dx%d...\n", width, height, bpp);

	RenderSurface *new_screen = RenderSurface::SetVideoMode(width, height, bpp);

	if (!new_screen) {
		perr << Common::String::format("Unable to set new video mode. Trying %dx%dx32", width, height) << Std::endl;
		new_screen = RenderSurface::SetVideoMode(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 32);
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
	debugN(MM_INFO, "Loading Default Mouse Cursor...\n");
	_mouse->setup();

	Std::string alt_confont;
	bool confont_loaded = false;

	if (_settingMan->get("console_font", alt_confont)) {
		debugN(MM_INFO, "Alternate console font found...\n");
		confont_loaded = LoadConsoleFont(alt_confont);
	}

	if (!confont_loaded) {
		debugN(MM_INFO, "Loading default console font...\n");
		if (!LoadConsoleFont("@data/fixedfont.ini")) {
			error("Failed to load console font. Exiting");
		}
	}

	_desktopGump = new DesktopGump(0, 0, width, height);
	_desktopGump->InitGump(0);
	_desktopGump->MakeFocus();

	_scalerGump = new ScalerGump(0, 0, width, height);
	_scalerGump->InitGump(0);

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

void Ultima8Engine::changeVideoMode(int width, int height) {
	if (width > 0) _settingMan->set("width", width);
	if (height > 0) _settingMan->set("height", height);

	GraphicSysInit();
}

bool Ultima8Engine::LoadConsoleFont(Std::string confontini) {
	// try to load the file
	debugN(MM_INFO, "Loading console font config: %s... ", confontini.c_str());
	if (_configFileMan->readConfigFile(confontini, "confont", true))
		pout << "Ok" << Std::endl;
	else {
		pout << "Failed" << Std::endl;
		return false;
	}

	return true;
}

void Ultima8Engine::enterTextMode(Gump *gump) {
	uint16 key;
	for (key = 0; key < HID_LAST; ++key) {
		if (_down[key]) {
			_down[key] = false;
			_lastDown[key] = false;
		}
	}

	if (!_textModes.empty()) {
		_textModes.remove(gump->getObjId());
	}
	_textModes.push_front(gump->getObjId());
}

void Ultima8Engine::leaveTextMode(Gump *gump) {
	if (!_textModes.empty())
		_textModes.remove(gump->getObjId());
}

void Ultima8Engine::handleEvent(const Common::Event &event) {
	uint32 now = g_system->getMillis();
	HID_Key key = HID_LAST;
	uint16 evn = HID_EVENT_LAST;
	bool handled = false;

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		key = HID_translateKey(event.kbd.keycode);
		evn = HID_translateKeyFlags(event.kbd.flags);
		break;
	case Common::EVENT_KEYUP:
		// Any system keys not in the bindings can be handled here
		break;

	case Common::EVENT_LBUTTONDOWN:
		key = HID_translateMouseButton(1);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_LBUTTONUP:
		key = HID_translateMouseButton(1);
		evn = HID_EVENT_RELEASE;
		break;
	case Common::EVENT_RBUTTONDOWN:
		key = HID_translateMouseButton(2);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_RBUTTONUP:
		key = HID_translateMouseButton(2);
		evn = HID_EVENT_RELEASE;
		break;
	case Common::EVENT_MBUTTONDOWN:
		key = HID_translateMouseButton(3);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_MBUTTONUP:
		key = HID_translateMouseButton(3);
		evn = HID_EVENT_RELEASE;
		break;

	case Common::EVENT_JOYBUTTON_DOWN:
		key = HID_translateJoystickButton(event.joystick.button + 1);
		evn = HID_EVENT_DEPRESS;
		break;
	case Common::EVENT_JOYBUTTON_UP:
		key = HID_translateJoystickButton(event.joystick.button + 1);
		evn = HID_EVENT_DEPRESS;
		break;

	case Common::EVENT_MOUSEMOVE:
		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		break;

	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		MetaEngine::pressAction((KeybindingAction)event.customType);
		return;

	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		MetaEngine::releaseAction((KeybindingAction)event.customType);
		return;

	case Common::EVENT_QUIT:
		_isRunning = false;
		break;

	default:
		break;
	}

	// Text mode input. A few hacks here
	Gump *gump = nullptr;

	if (!_textModes.empty()) {
		while (!_textModes.empty()) {
			gump = p_dynamic_cast<Gump *>(_objectManager->getObject(_textModes.front()));
			if (gump)
				break;

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
		if ((event.kbd.keycode == Common::KEYCODE_q || event.kbd.keycode == Common::KEYCODE_x) &&
			(event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META)) != 0)
			ForceQuit();
		break;

	default:
		break;
	}

	if (_mouse->dragging() == Mouse::DRAG_NOT && !handled) {
		if (evn == HID_EVENT_DEPRESS) {
			_down[key] = true;
			if (now - _lastDown[key] < DOUBLE_CLICK_TIMEOUT && _lastDown[key] != 0) {
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
	if (_desktopGump->FindGump<ModalGump>() || _avatarInStasis)
		// Can't save when a modal gump is open, or avatar in statsis  during cutscenes
		return false;

	if (_kernel->getRunningProcess() && _kernel->getRunningProcess()->IsOfType(StartU8Process::ClassType))
		// Don't save while starting up.
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

	return saveGameState(slot, desc).getCode() == Common::kNoError;
}

Common::Error Ultima8Engine::loadGameState(int slot) {
	Common::Error result = Shared::UltimaEngine::loadGameState(slot);
	if (result.getCode() == Common::kNoError)
		_settingMan->set("lastSave", slot);
	else
		_settingMan->set("lastSave", "");

	return result;
}

Common::Error Ultima8Engine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::Error result = Shared::UltimaEngine::saveGameState(slot, desc, isAutosave);;

	if (!isAutosave) {
		if (result.getCode() == Common::kNoError)
			_settingMan->set("lastSave", slot);
		else
			_settingMan->set("lastSave", "");
	}

	return result;
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
	debugN(MM_INFO, "-- Resetting Engine --\n");

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
	_desktopGump = nullptr;
	_gameMapGump = nullptr;
	_scalerGump = nullptr;
	_inverterGump = nullptr;

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

	debugN(MM_INFO, "-- Engine Reset --\n");
}

void Ultima8Engine::setupCoreGumps() {
	debugN(MM_INFO, "Setting up core _game gumps...\n");

	Rect dims;
	_screen->GetSurfaceDims(dims);

	debugN(MM_INFO, "Creating Desktop...\n");
	_desktopGump = new DesktopGump(0, 0, dims.w, dims.h);
	_desktopGump->InitGump(0);
	_desktopGump->MakeFocus();

	debugN(MM_INFO, "Creating _scalerGump...\n");
	_scalerGump = new ScalerGump(0, 0, dims.w, dims.h);
	_scalerGump->InitGump(0);

	Rect scaled_dims;
	_scalerGump->GetDims(scaled_dims);

	debugN(MM_INFO, "Creating Inverter...\n");
	_inverterGump = new InverterGump(0, 0, scaled_dims.w, scaled_dims.h);
	_inverterGump->InitGump(0);

	debugN(MM_INFO, "Creating GameMapGump...\n");
	_gameMapGump = new GameMapGump(0, 0, scaled_dims.w, scaled_dims.h);
	_gameMapGump->InitGump(0);


	// TODO: clean this up
	assert(_desktopGump->getObjId() == 256);
	assert(_scalerGump->getObjId() == 257);
	assert(_inverterGump->getObjId() == 258);
	assert(_gameMapGump->getObjId() == 259);


	for (uint16 i = 261; i < 384; ++i)
		_objectManager->reserveObjId(i);
}

bool Ultima8Engine::newGame(int saveSlot) {
	debugN(MM_INFO, "Starting New Game...\n");

	// First validate we still have a save file for the slot
	if (saveSlot != -1) {
		SaveStateDescriptor desc = getMetaEngine().querySaveMetaInfos(_targetName.c_str(), saveSlot);
		if (desc.getSaveSlot() != saveSlot)
			saveSlot = -1;
	}

	resetEngine();

	setupCoreGumps();

	_game->startGame();

	debugN(MM_INFO, "Create Camera...\n");
	CameraProcess::SetCameraProcess(new CameraProcess(1)); // Follow Avatar

	debugN(MM_INFO, "Create persistent Processes...\n");
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

	if (saveSlot == -1)
		_settingMan->set("lastSave", "");

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
		return Common::kReadingFailed;
	}

	if (state != SavegameReader::SAVE_VALID) {
		Error("Unsupported savegame version", "Error Loading savegame");
		delete sg;
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
			error("%s", message.c_str());
		}
		perr << message << Std::endl;
#else
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
	pout << "UCSTRINGS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "UCSTRINGS: failed\n";
	delete ds;

	ds = sg->getDataSource("UCGLOBALS");
	ok = _ucMachine->loadGlobals(ds, version);
	totalok &= ok;
	pout << "UCGLOBALS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "UCGLOBALS: failed\n";
	delete ds;

	ds = sg->getDataSource("UCLISTS");
	ok = _ucMachine->loadLists(ds, version);
	totalok &= ok;
	pout << "UCLISTS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "UCLISTS: failed\n";
	delete ds;

	// KERNEL must be before OBJECTS, for the egghatcher
	// KERNEL must be before APP, for the _avatarMoverProcess
	ds = sg->getDataSource("KERNEL");
	ok = _kernel->load(ds, version);
	totalok &= ok;
	pout << "KERNEL: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "KERNEL: failed\n";
	delete ds;

	ds = sg->getDataSource("APP");
	ok = load(ds, version);
	totalok &= ok;
	pout << "APP: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "APP: failed\n";
	delete ds;

	// WORLD must be before OBJECTS, for the egghatcher
	ds = sg->getDataSource("WORLD");
	ok = _world->load(ds, version);
	totalok &= ok;
	pout << "WORLD: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "WORLD: failed\n";
	delete ds;

	ds = sg->getDataSource("CURRENTMAP");
	ok = _world->getCurrentMap()->load(ds, version);
	totalok &= ok;
	pout << "CURRENTMAP: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "CURRENTMAP: failed\n";
	delete ds;

	ds = sg->getDataSource("OBJECTS");
	ok = _objectManager->load(ds, version);
	totalok &= ok;
	pout << "OBJECTS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "OBJECTS: failed\n";
	delete ds;

	ds = sg->getDataSource("MAPS");
	ok = _world->loadMaps(ds, version);
	totalok &= ok;
	pout << "MAPS: " << (ok ? "ok" : "failed") << Std::endl;
	if (!ok) message += "MAPS: failed\n";
	delete ds;

	if (!totalok) {
		Error(message, "Error Loading savegame", true);
		delete sg;
		return Common::kReadingFailed;
	}

	pout << "Done" << Std::endl;

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
		gump->IsOfType<ScalerGump>() || gump->IsOfType<MessageBoxGump>()// ||
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
	for (int i = 0; i < 12; i++) ods->write2(pal->_matrix[i]);
	ods->write2(pal->_transform);

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
	pal->_transform = static_cast<PalTransforms>(ids->read2());

	_inversion = ids->read2();

	_saveCount = ids->read4();

	_hasCheated = (ids->read1() != 0);

	return true;
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

Gump *Ultima8Engine::getMenuGump() const {
	if (_textModes.empty())
		return nullptr;

	return p_dynamic_cast<Gump *>(_objectManager->getObject(_textModes.front()));
}

} // End of namespace Ultima8
} // End of namespace Ultima
