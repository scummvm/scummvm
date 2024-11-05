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

#include "common/file.h"
#include "common/rational.h"
#include "common/translation.h"
#include "common/compression/unzip.h"
#include "gui/saveload.h"
#include "image/png.h"
#include "engines/dialogs.h"
#include "engines/util.h"

 // TODO: !! a lot of these includes are just for some hacks... clean up sometime
#include "ultima/ultima8/conf/config_file_manager.h"
#include "ultima/ultima8/kernel/object_manager.h"
#include "ultima/ultima8/games/start_u8_process.h"
#include "ultima/ultima8/games/start_crusader_process.h"
#include "ultima/ultima8/gfx/fonts/font_manager.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/filesys/savegame.h"
#include "ultima/ultima8/gumps/game_map_gump.h"
#include "ultima/ultima8/gumps/inverter_gump.h"
#include "ultima/ultima8/gumps/minimap_gump.h"
#include "ultima/ultima8/gumps/cru_status_gump.h"
#include "ultima/ultima8/gumps/movie_gump.h"
#include "ultima/ultima8/gumps/weasel_gump.h"

// For gump positioning... perhaps shouldn't do it this way....
#include "ultima/ultima8/gumps/message_box_gump.h"
#include "ultima/ultima8/gumps/keypad_gump.h"
#include "ultima/ultima8/gumps/computer_gump.h"
#include "ultima/ultima8/world/actors/quick_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/battery_charger_process.h"
#include "ultima/ultima8/world/actors/cru_healer_process.h"
#include "ultima/ultima8/world/actors/targeted_anim_process.h"
#include "ultima/ultima8/usecode/u8_intrinsics.h"
#include "ultima/ultima8/usecode/remorse_intrinsics.h"
#include "ultima/ultima8/usecode/regret_intrinsics.h"

#include "ultima/ultima8/gfx/cycle_process.h"
#include "ultima/ultima8/world/actors/scheduler_process.h"
#include "ultima/ultima8/world/egg_hatcher_process.h" // for a hack
#include "ultima/ultima8/usecode/uc_process.h" // more hacking
#include "ultima/ultima8/world/actors/actor_bark_notify_process.h" // guess
#include "ultima/ultima8/kernel/delay_process.h"
#include "ultima/ultima8/world/actors/avatar_gravity_process.h"
#include "ultima/ultima8/world/actors/teleport_to_egg_process.h"
#include "ultima/ultima8/world/item_selection_process.h"
#include "ultima/ultima8/world/split_item_process.h"
#include "ultima/ultima8/world/target_reticle_process.h"
#include "ultima/ultima8/world/snap_process.h"
#include "ultima/ultima8/world/crosshair_process.h"
#include "ultima/ultima8/world/actors/pathfinder_process.h"
#include "ultima/ultima8/world/actors/u8_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/cru_avatar_mover_process.h"
#include "ultima/ultima8/world/actors/cru_pathfinder_process.h"
#include "ultima/ultima8/world/actors/resurrection_process.h"
#include "ultima/ultima8/world/actors/clear_feign_death_process.h"
#include "ultima/ultima8/world/actors/loiter_process.h"
#include "ultima/ultima8/world/actors/avatar_death_process.h"
#include "ultima/ultima8/world/actors/surrender_process.h"
#include "ultima/ultima8/world/actors/combat_process.h"
#include "ultima/ultima8/world/actors/guard_process.h"
#include "ultima/ultima8/world/actors/attack_process.h"
#include "ultima/ultima8/world/actors/auto_firer_process.h"
#include "ultima/ultima8/world/actors/pace_process.h"
#include "ultima/ultima8/world/actors/rolling_thunder_process.h"
#include "ultima/ultima8/world/bobo_boomer_process.h"
#include "ultima/ultima8/world/super_sprite_process.h"
#include "ultima/ultima8/world/destroy_item_process.h"
#include "ultima/ultima8/world/actors/ambush_process.h"
#include "ultima/ultima8/audio/audio_mixer.h"
#include "ultima/ultima8/audio/u8_music_process.h"
#include "ultima/ultima8/audio/cru_music_process.h"
#include "ultima/ultima8/audio/midi_player.h"
#include "ultima/ultima8/gumps/shape_viewer_gump.h"
#include "ultima/ultima8/metaengine.h"

//#define PAINT_TIMING 1

#define GAME_FRAME_TIME 50

namespace Ultima {
namespace Ultima8 {

using Std::string;

// a bit of a hack to prevent having to write a load function for
// every process
template<class T>
struct ProcessLoader {
	static Process *load(Common::ReadStream *rs, uint32 version) {
		T *p = new T();
		bool ok = p->loadData(rs, version);
		if (!ok) {
			delete p;
			p = nullptr;
		}
		return p;
	}
};

inline bool HasPreventSaveFlag(const Gump *g) { return g->hasFlags(Gump::FLAG_PREVENT_SAVE); }

Ultima8Engine *Ultima8Engine::_instance = nullptr;

Ultima8Engine::Ultima8Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Ultima8"),
		_isRunning(false),  _gameInfo(nullptr),
		_configFileMan(nullptr), _saveCount(0), _game(nullptr), _lastError(Common::kNoError),
		_kernel(nullptr), _objectManager(nullptr), _mouse(nullptr), _ucMachine(nullptr),
		_screen(nullptr), _fontManager(nullptr), _paletteManager(nullptr), _gameData(nullptr),
		_world(nullptr), _desktopGump(nullptr), _gameMapGump(nullptr), _avatarMoverProcess(nullptr),
		_frameSkip(false), _frameLimit(true), _interpolate(true), _animationRate(100),
		_avatarInStasis(false), _cruStasis(false), _paintEditorItems(false), _inversion(0),
		_showTouching(false), _timeOffset(0), _hasCheated(false), _cheatsEnabled(false),
		_fontOverride(false), _fontAntialiasing(false), _audioMixer(0), _inverterGump(nullptr),
	    _lerpFactor(256), _inBetweenFrame(false), _crusaderTeleporting(false), _moveKeyFrame(0),
		_highRes(false), _priorFrameCounterTime(0) {
	_instance = this;
}

Ultima8Engine::~Ultima8Engine() {
	delete _kernel;
	delete _objectManager;
	delete _audioMixer;
	delete _ucMachine;
	delete _paletteManager;
	delete _mouse;
	delete _gameData;
	delete _world;
	delete _fontManager;
	delete _screen;
	delete _configFileMan;
	delete _gameInfo;

	_instance = nullptr;
}

Common::Error Ultima8Engine::run() {
	Common::Error result = initialize();
	if (result.getCode() == Common::kNoError) {
		result = runGame();
		deinitialize();
	}

	return result;
}

void Ultima8Engine::initializePath(const Common::FSNode& gamePath) {
	Engine::initializePath(gamePath);

	// Crusader: No Regret (DOS/German)
	SearchMan.addSubDirectoryMatching(gamePath, "data", 0, 4);
}

Common::Error Ultima8Engine::initialize() {
	debug(1, "-- Initializing Pentagram --");

	// Call syncSoundSettings to get default volumes set
	syncSoundSettings();

	// Try and set up the data archive
	Common::Archive *archive = Common::makeZipArchive("ultima8.dat");
	if (!archive) {
		GUIErrorMessageFormat(_("Unable to locate the '%s' engine data file."), "ultima8.dat");
		return Common::kPathDoesNotExist;
	}
	SearchMan.add("ultima8.dat", archive);

	setDebugger(new Debugger());

	_gameInfo = nullptr;
	_configFileMan = new ConfigFileManager();
	_fontManager = new FontManager();
	_kernel = new Kernel();

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
	_kernel->addProcessLoader("AvatarMoverProcess", // parent class for backward compatibility
							  ProcessLoader<U8AvatarMoverProcess>::load);
	_kernel->addProcessLoader("U8AvatarMoverProcess",
							  ProcessLoader<U8AvatarMoverProcess>::load);
	_kernel->addProcessLoader("CruAvatarMoverProcess",
							  ProcessLoader<CruAvatarMoverProcess>::load);
	_kernel->addProcessLoader("QuickAvatarMoverProcess",
							  ProcessLoader<QuickAvatarMoverProcess>::load);
	_kernel->addProcessLoader("PathfinderProcess",
							  ProcessLoader<PathfinderProcess>::load);
	_kernel->addProcessLoader("CruPathfinderProcess",
							  ProcessLoader<CruPathfinderProcess>::load);
	_kernel->addProcessLoader("SpriteProcess",
							  ProcessLoader<SpriteProcess>::load);
	_kernel->addProcessLoader("CameraProcess",
							  ProcessLoader<CameraProcess>::load);
	_kernel->addProcessLoader("MusicProcess", // parent class name for save game backwards-compatibility.
							  ProcessLoader<U8MusicProcess>::load);
	_kernel->addProcessLoader("U8MusicProcess",
							  ProcessLoader<U8MusicProcess>::load);
	_kernel->addProcessLoader("RemorseMusicProcess", // name was changed, keep this for backward-compatibility.
							  ProcessLoader<CruMusicProcess>::load);
	_kernel->addProcessLoader("CruMusicProcess",
							  ProcessLoader<CruMusicProcess>::load);
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
	_kernel->addProcessLoader("TargetReticleProcess",
							  ProcessLoader<TargetReticleProcess>::load);
	_kernel->addProcessLoader("SurrenderProcess",
							  ProcessLoader<SurrenderProcess>::load);
	_kernel->addProcessLoader("CruHealerProcess",
							  ProcessLoader<CruHealerProcess>::load);
	_kernel->addProcessLoader("BatteryChargerProcess",
							  ProcessLoader<BatteryChargerProcess>::load);
	_kernel->addProcessLoader("CycleProcess",
							  ProcessLoader<CycleProcess>::load);
	_kernel->addProcessLoader("GuardProcess",
							  ProcessLoader<GuardProcess>::load);
	_kernel->addProcessLoader("SnapProcess",
							  ProcessLoader<SnapProcess>::load);
	_kernel->addProcessLoader("CrosshairProcess",
							  ProcessLoader<CrosshairProcess>::load);
	_kernel->addProcessLoader("ItemSelectionProcess",
							  ProcessLoader<ItemSelectionProcess>::load);
	_kernel->addProcessLoader("PaceProcess",
							  ProcessLoader<PaceProcess>::load);
	_kernel->addProcessLoader("SuperSpriteProcess",
							  ProcessLoader<SuperSpriteProcess>::load);
	_kernel->addProcessLoader("AttackProcess",
							  ProcessLoader<AttackProcess>::load);
	_kernel->addProcessLoader("AutoFirerProcess",
							  ProcessLoader<AutoFirerProcess>::load);
	_kernel->addProcessLoader("BoboBoomerProcess",
							  ProcessLoader<BoboBoomerProcess>::load);
	_kernel->addProcessLoader("RollingThunderProcess",
							  ProcessLoader<RollingThunderProcess>::load);

	_objectManager = new ObjectManager();
	_mouse = new Mouse();

	// Audio Mixer
	_audioMixer = new AudioMixer(_mixer);

	debug(1, "-- Pentagram Initialized -- ");

	if (setupGame()) {
		Common::Error result = startupGame();
		if (result.getCode() != Common::kNoError)
			return result;
	} else {
		// Couldn't setup the game, should never happen?
		warning("game failed to initialize");
	}
	paint();
	return Common::kNoError;
}

void Ultima8Engine::deinitialize() {
	debug(1, "-- Shutting down Game -- ");

	// Save config here....

	// reset mouse cursor
	_mouse->popAllCursors();
	_mouse->pushMouseCursor(Mouse::MOUSE_NORMAL);

	delete _world;
	_world = nullptr;

	_objectManager->reset();

	delete _ucMachine;
	_ucMachine = nullptr;

	_kernel->reset();
	_paletteManager->reset();
	_fontManager->resetGameFonts();

	delete _game;
	_game = nullptr;

	delete _gameData;
	_gameData = nullptr;

	if (_audioMixer) {
		_audioMixer->closeMidiOutput();
		_audioMixer->reset();
		delete _audioMixer;
		_audioMixer = nullptr;
	}

	_desktopGump = nullptr;
	_gameMapGump = nullptr;
	_inverterGump = nullptr;

	_timeOffset = -(int32)Kernel::get_instance()->getFrameNum();
	_saveCount = 0;
	_hasCheated = false;

	_configFileMan->clearRoot("bindings");
	_configFileMan->clearRoot("language");
	_configFileMan->clearRoot("weapons");
	_configFileMan->clearRoot("armour");
	_configFileMan->clearRoot("monsters");
	_configFileMan->clearRoot("game");
	_gameInfo = nullptr;

	debug(1, "-- Game Shutdown -- ");
}

void Ultima8Engine::pauseEngineIntern(bool pause) {
	if (_mixer)
		_mixer->pauseAll(pause);
	if (_audioMixer) {
		MidiPlayer *midiPlayer = _audioMixer->getMidiPlayer();
		if (midiPlayer)
			midiPlayer->pause(pause);
	}

	// This will normally be non-null except in the case of
	// a fatal error on startup (eg missing files)
	if (_avatarMoverProcess)
		_avatarMoverProcess->resetMovementFlags();
}

bool Ultima8Engine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsSubtitleOptions) ||
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsChangingOptionsDuringRuntime);
}

Common::Language Ultima8Engine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool Ultima8Engine::setupGame() {
	GameInfo *info = new GameInfo;
	info->_name = _gameDescription->desc.gameId;
	info->_type = GameInfo::GAME_UNKNOWN;
	info->version = 0;
	info->_language = GameInfo::GAMELANG_UNKNOWN;
	info->_ucOffVariant = GameInfo::GAME_UC_DEFAULT;

	if (info->_name == "ultima8")
		info->_type = GameInfo::GAME_U8;
	else if (info->_name == "remorse")
		info->_type = GameInfo::GAME_REMORSE;
	else if (info->_name == "regret")
		info->_type = GameInfo::GAME_REGRET;

	if (info->_type == GameInfo::GAME_REMORSE) {
		switch (_gameDescription->desc.flags & ADGF_USECODE_MASK) {
		case ADGF_USECODE_DEMO:
			info->_ucOffVariant = GameInfo::GAME_UC_DEMO;
			break;
		case ADGF_USECODE_ORIG:
			info->_ucOffVariant = GameInfo::GAME_UC_ORIG;
			break;
		case ADGF_USECODE_ES:
			info->_ucOffVariant = GameInfo::GAME_UC_REM_ES;
			break;
		case ADGF_USECODE_FR:
			info->_ucOffVariant = GameInfo::GAME_UC_REM_FR;
			break;
		case ADGF_USECODE_JA:
			info->_ucOffVariant = GameInfo::GAME_UC_REM_JA;
			break;
		default:
			break;
		}
	} else if (info->_type == GameInfo::GAME_REGRET) {
		switch (_gameDescription->desc.flags & ADGF_USECODE_MASK) {
		case ADGF_USECODE_DEMO:
			info->_ucOffVariant = GameInfo::GAME_UC_DEMO;
			break;
		case ADGF_USECODE_ORIG:
			info->_ucOffVariant = GameInfo::GAME_UC_ORIG;
			break;
		case ADGF_USECODE_DE:
			info->_ucOffVariant = GameInfo::GAME_UC_REG_DE;
			break;
		default:
			break;
		}
	}

	switch (_gameDescription->desc.language) {
	case Common::EN_ANY:
		info->_language = GameInfo::GAMELANG_ENGLISH;
		break;
	case Common::FR_FRA:
		info->_language = GameInfo::GAMELANG_FRENCH;
		break;
	case Common::DE_DEU:
		info->_language = GameInfo::GAMELANG_GERMAN;
		break;
	case Common::ES_ESP:
		info->_language = GameInfo::GAMELANG_SPANISH;
		break;
	case Common::JA_JPN:
		info->_language = GameInfo::GAMELANG_JAPANESE;
		break;
	default:
		error("Unknown language");
		break;
	}

	if (info->_type == GameInfo::GAME_UNKNOWN) {
		warning("%s: unknown, skipping", info->_name.c_str());
		return false;
	}

	// output detected game info
	Std::string details = info->getPrintDetails();
	debug(1, "%s: %s", info->_name.c_str(), details.c_str());

	_gameInfo = info;
	return true;
}

Common::Error Ultima8Engine::startupGame() {
	debug(1, "-- Initializing Game: %s --", _gameInfo->_name.c_str());

	if (ConfMan.hasKey("usehighres")) {
		_highRes = ConfMan.getBool("usehighres");
	}

	if (GAME_IS_U8) {
		ConfMan.registerDefault("width", _highRes ? U8_HIRES_SCREEN_WIDTH : U8_DEFAULT_SCREEN_WIDTH);
		ConfMan.registerDefault("height", _highRes ? U8_HIRES_SCREEN_HEIGHT : U8_DEFAULT_SCREEN_HEIGHT);
	} else {
		ConfMan.registerDefault("width", _highRes ? CRUSADER_HIRES_SCREEN_WIDTH : CRUSADER_DEFAULT_SCREEN_WIDTH);
		ConfMan.registerDefault("height", _highRes ? CRUSADER_HIRES_SCREEN_HEIGHT : CRUSADER_DEFAULT_SCREEN_HEIGHT);
	}

	int width = ConfMan.getInt("width");
	int height = ConfMan.getInt("height");
	changeVideoMode(width, height);

	// Show the splash screen immediately now that the screen has been set up
	_mouse->setMouseCursor(Mouse::MOUSE_NONE);
	showSplashScreen();

	_gameData = new GameData(_gameInfo);

	if (_gameInfo->_type == GameInfo::GAME_U8) {
		_ucMachine = new UCMachine(U8Intrinsics, ARRAYSIZE(U8Intrinsics));
	} else if (_gameInfo->_type == GameInfo::GAME_REMORSE) {
		switch (_gameInfo->_ucOffVariant) {
		case GameInfo::GAME_UC_DEMO:
			_ucMachine = new UCMachine(RemorseDemoIntrinsics, ARRAYSIZE(RemorseDemoIntrinsics));
			break;
		case GameInfo::GAME_UC_REM_ES:
			_ucMachine = new UCMachine(RemorseEsIntrinsics, ARRAYSIZE(RemorseEsIntrinsics));
			break;
		case GameInfo::GAME_UC_REM_FR:
			_ucMachine = new UCMachine(RemorseFrIntrinsics, ARRAYSIZE(RemorseFrIntrinsics));
			break;
		case GameInfo::GAME_UC_REM_JA:
			warning("TODO: Create Remorse JA intrinsic list");
			_ucMachine = new UCMachine(RemorseIntrinsics, ARRAYSIZE(RemorseIntrinsics));
			break;
		case GameInfo::GAME_UC_ORIG:
			warning("TODO: Create Remorse original version intrinsic list");
			_ucMachine = new UCMachine(RemorseIntrinsics, ARRAYSIZE(RemorseIntrinsics));
			break;
		default:
			_ucMachine = new UCMachine(RemorseIntrinsics, ARRAYSIZE(RemorseIntrinsics));
			break;
		}
	} else if (_gameInfo->_type == GameInfo::GAME_REGRET) {
		switch (_gameInfo->_ucOffVariant) {
		case GameInfo::GAME_UC_DEMO:
			_ucMachine = new UCMachine(RegretDemoIntrinsics, ARRAYSIZE(RegretDemoIntrinsics));
			break;
		case GameInfo::GAME_UC_REG_DE:
			_ucMachine = new UCMachine(RegretDeIntrinsics, ARRAYSIZE(RegretDeIntrinsics));
			break;
		case GameInfo::GAME_UC_ORIG: // 1.06 is the original CD release too?
		default:
			_ucMachine = new UCMachine(RegretIntrinsics, ARRAYSIZE(RegretIntrinsics));
			break;
		}
	} else {
		warning("Invalid game type.");
	}

	_inBetweenFrame = false;
	_lerpFactor = 256;

	// Initialize _world
	_world = new World();
	_world->initMaps();

	_game = Game::createGame(getGameInfo());

	ConfMan.registerDefault("font_override", false);
	ConfMan.registerDefault("font_antialiasing", true);
	ConfMan.registerDefault("frameSkip", false);
	ConfMan.registerDefault("frameLimit", true);
	// Position interpolation looks nice on U8, but causes Crusader to look janky.
	ConfMan.registerDefault("interpolate", _gameInfo->_type == GameInfo::GAME_U8);
	ConfMan.registerDefault("cheat", false);

	bool loaded = _game->loadFiles();
	if (!loaded)
		return Common::kNoGameDataFoundError;

	applyGameSettings();

	// Create Midi Driver for Ultima 8
	if (getGameInfo()->_type == GameInfo::GAME_U8)
		_audioMixer->openMidiOutput();

	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	if (saveSlot == -1 && ConfMan.hasKey("lastSave"))
		saveSlot = ConfMan.getInt("lastSave");

	if (!newGame(saveSlot))
		return Common::kNoGameDataFoundError;

	debug(1, "-- Game Initialized --");
	return Common::kNoError;
}

//
// To time the frames, we use "fast" ticks which come 3000 times a second.
//
static uint32 _fastTicksNow() {
	return g_system->getMillis() * 3;
}

Common::Error Ultima8Engine::runGame() {
	_isRunning = true;

	int32 next_ticks = _fastTicksNow();  // Next time is right now!

	Common::Event event;
	while (_isRunning) {
		_inBetweenFrame = true;  // Will get set false if it's not an _inBetweenFrame

		if (!_frameLimit) {
			for (unsigned int tick = 0; tick < Kernel::TICKS_PER_FRAME; tick++) {
				_kernel->runProcesses();
				_desktopGump->run();
			}
			_inBetweenFrame = false;
			next_ticks = _animationRate + _fastTicksNow();
			_lerpFactor = 256;
		} else {
			int32 ticks = _fastTicksNow();
			int32 diff = next_ticks - ticks;

			while (diff < 0) {
				next_ticks += _animationRate;
				for (unsigned int tick = 0; tick < Kernel::TICKS_PER_FRAME; tick++) {
					_kernel->runProcesses();
					_desktopGump->run();
				}
#if 0
				debug(1, "--- NEW FRAME ---");
#endif
				_inBetweenFrame = false;

				ticks = _fastTicksNow();

				// If frame skipping is off, we will only recalc next
				// ticks IF the frames are taking up 'way' too much time.
				if (!_frameSkip && diff <= -_animationRate * 2) next_ticks = _animationRate + ticks;

				diff = next_ticks - ticks;
				if (!_frameSkip) break;
			}

			// Calculate the lerp_factor
			_lerpFactor = ((_animationRate - diff) * 256) / _animationRate;
			if (!_interpolate || _kernel->isPaused() || _lerpFactor > 256)
				_lerpFactor = 256;
		}

		// get & handle all events in queue
		while (_isRunning && pollEvent(event)) {
			handleEvent(event);
		}
		handleDelayedEvents();

		// Update the mouse
		_mouse->update();

		// Paint Screen
		paint();

		if (_lastError.getCode() != Common::kNoError) {
			return _lastError;
		}

		// Do a delay
		g_system->delayMillis(5);
	}
	return Common::kNoError;
}

// Paint the _screen
void Ultima8Engine::paint() {
#ifdef PAINT_TIMING
	static long prev = 0;
	static long t = 0;
	static long tdiff = 0;
	static long tpaint = 0;
	long now = g_system->getMillis();

	if (prev != 0)
		tdiff += now - prev;
	prev = now;
	++t;
#endif

	if (!_screen) // need to worry if the graphics system has been started. Need nicer way.
		return;

	// Begin _painting
	_screen->BeginPainting();

#ifdef PAINT_TIMING
	tpaint -= g_system->getMillis();
#endif

	Rect r;
	_screen->GetSurfaceDims(r);
	if (_highRes)
		_screen->fill32(TEX32_PACK_RGB(0, 0, 0), r);

#ifdef DEBUG
	// Fill the screen with an annoying color so we can see fast area bugs
	_screen->fill32(TEX32_PACK_RGB(0x10, 0xFF, 0x10), r);
#endif

	_desktopGump->Paint(_screen, _lerpFactor, false);
#ifdef PAINT_TIMING
	tpaint += g_system->getMillis();

	if (t % 150 == 0) { // every ~5 seconds
		debug("Ultima8Engine: Paint average %.03f millis", (float)tpaint / t);
	}
#endif

	// End _painting
	_screen->EndPainting();

	Graphics::Screen *screen = getScreen();
	if (screen)
		screen->update();
}

void Ultima8Engine::changeVideoMode(int width, int height) {
	if (_screen) {
		Rect old_dims;
		_screen->GetSurfaceDims(old_dims);
		if (width == old_dims.width() && height == old_dims.height())
			return;

		delete _screen;
	}

	// Set Screen Resolution
	debug(1, "Setting Video Mode %dx%d...", width, height);

	Common::List<Graphics::PixelFormat> tryModes = g_system->getSupportedFormats();
	for (Common::List<Graphics::PixelFormat>::iterator g = tryModes.begin(); g != tryModes.end(); ++g) {
		if (g->bytesPerPixel != 2 && g->bytesPerPixel != 4) {
			g = tryModes.reverse_erase(g);
		}
	}

	initGraphics(width, height, tryModes);

	Graphics::PixelFormat format = g_system->getScreenFormat();
	if (format.bytesPerPixel != 2 && format.bytesPerPixel != 4) {
		error("Only 16 bit and 32 bit video modes supported");
	}

	// Set up blitting surface
	Graphics::ManagedSurface *surface = new Graphics::Screen(width, height, format);
	_screen = new RenderSurface(surface);

	if (!_paletteManager) {
		_paletteManager = new PaletteManager(format);
	} else {
		_paletteManager->PixelFormatChanged(format);
	}

	if (!_desktopGump) {
		_desktopGump = new DesktopGump(0, 0, width, height);
		_desktopGump->InitGump(0);
		_desktopGump->MakeFocus();
	} else {
		_desktopGump->SetDims(Rect(0, 0, width, height));
		_desktopGump->RenderSurfaceChanged();
	}

	paint();
}

void Ultima8Engine::handleEvent(const Common::Event &event) {
	// Handle the fact that we can get 2 modals stacking.
	// We want the focussed one preferably.
	Gump *modal = dynamic_cast<ModalGump *>(_desktopGump->GetFocusChild());
	if (!modal)
		modal = _desktopGump->FindGump<ModalGump>();
	if (modal) {
		_avatarMoverProcess->resetMovementFlags();
	}

	Common::Keymapper *const keymapper = _eventMan->getKeymapper();
	keymapper->setEnabledKeymapType(modal ? Common::Keymap::kKeymapTypeGui : Common::Keymap::kKeymapTypeGame);

	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		if (modal) {
			// Paste from Clip-Board on Ctrl-V - Note this should be a flag of some sort
			if (event.kbd.keycode == Common::KEYCODE_v && (event.kbd.flags & Common::KBD_CTRL)) {
				if (!g_system->hasTextInClipboard())
					return;

				Common::String text = g_system->getTextFromClipboard();

				// Only read the first line of text
				while (!text.empty() && text.firstChar() >= ' ')
					modal->OnTextInput(text.firstChar());

				return;
			}

			if (event.kbd.ascii >= ' ' &&
				event.kbd.ascii <= 255 &&
				!(event.kbd.ascii >= 0x7F && // control chars
					event.kbd.ascii <= 0x9F)) {
				modal->OnTextInput(event.kbd.ascii);
			}

			modal->OnKeyDown(event.kbd.keycode, event.kbd.flags);
		}
		break;
	case Common::EVENT_KEYUP:
		if (modal) {
			modal->OnKeyUp(event.kbd.keycode);
		}
		break;

	case Common::EVENT_MOUSEMOVE:
		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		break;

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN: {
		Mouse::MouseButton button = Mouse::BUTTON_LEFT;
		if (event.type == Common::EVENT_RBUTTONDOWN)
			button = Mouse::BUTTON_RIGHT;
		else if (event.type == Common::EVENT_MBUTTONDOWN)
			button = Mouse::BUTTON_MIDDLE;

		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		_mouse->buttonDown(button);
		break;
	}

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_MBUTTONUP:
	case Common::EVENT_RBUTTONUP: {
		Mouse::MouseButton button = Mouse::BUTTON_LEFT;
		if (event.type == Common::EVENT_RBUTTONUP)
			button = Mouse::BUTTON_RIGHT;
		else if (event.type == Common::EVENT_MBUTTONUP)
			button = Mouse::BUTTON_MIDDLE;

		_mouse->setMouseCoords(event.mouse.x, event.mouse.y);
		_mouse->buttonUp(button);
		break;
	}

	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		handleActionDown((KeybindingAction)event.customType);
		break;

	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		handleActionUp((KeybindingAction)event.customType);
		break;

	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		_isRunning = false;
		break;

	default:
		break;
	}
}

void Ultima8Engine::handleDelayedEvents() {
	//uint32 now = g_system->getMillis();

	_mouse->handleDelayedEvents();
}

void Ultima8Engine::handleActionDown(KeybindingAction action) {
	if (!isAvatarInStasis()) {
		if (_avatarMoverProcess && _avatarMoverProcess->onActionDown(action)) {
			moveKeyEvent();
			return;
		}
	}

	Common::String methodName = MetaEngine::getMethod(action, true);
	if (!methodName.empty())
		g_debugger->executeCommand(methodName);
}

void Ultima8Engine::handleActionUp(KeybindingAction action) {
	if (_avatarMoverProcess && _avatarMoverProcess->onActionUp(action)) {
		moveKeyEvent();
		return;
	}

	Common::String methodName = MetaEngine::getMethod(action, false);
	if (!methodName.empty())
		g_debugger->executeCommand(methodName);
}

void Ultima8Engine::writeSaveInfo(Common::WriteStream *ws) {
	TimeDate timeInfo;
	g_system->getTimeAndDate(timeInfo);

	ws->writeUint16LE(static_cast<uint16>(timeInfo.tm_year + 1900));
	ws->writeByte(static_cast<uint8>(timeInfo.tm_mon + 1));
	ws->writeByte(static_cast<uint8>(timeInfo.tm_mday));
	ws->writeByte(static_cast<uint8>(timeInfo.tm_hour));
	ws->writeByte(static_cast<uint8>(timeInfo.tm_min));
	ws->writeByte(static_cast<uint8>(timeInfo.tm_sec));
	ws->writeUint32LE(_saveCount);
	ws->writeUint32LE(getGameTimeInSeconds());

	uint8 c = (_hasCheated ? 1 : 0);
	ws->writeByte(c);

	// write _game-specific info
	_game->writeSaveInfo(ws);
}

bool Ultima8Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	// Can't save when avatar in stasis during cutscenes
	if (_avatarInStasis || _cruStasis)
		return false;

	// Check for gumps that prevent saving
	if (_desktopGump->FindGump(&HasPreventSaveFlag, true))
	{
		return false;
	}

	// Check for processes that prevent saving
	if (!_kernel->canSave()) {
		return false;
	}

	// Don't allow saving when avatar is dead.
	MainActor *av = getMainActor();
	if (!av || av->hasActorFlags(Actor::ACT_DEAD))
		return false;

	return true;
}

Common::Error Ultima8Engine::loadGameState(int slot) {
	Common::Error result = Engine::loadGameState(slot);
	if (result.getCode() == Common::kNoError)
		ConfMan.setInt("lastSave", slot);
	else
		ConfMan.setInt("lastSave", -1);

	ConfMan.flushToDisk();

	return result;
}

Common::Error Ultima8Engine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::Error result = Engine::saveGameState(slot, desc, isAutosave);

	if (!isAutosave) {
		if (result.getCode() == Common::kNoError)
			ConfMan.setInt("lastSave", slot);
		else
			ConfMan.setInt("lastSave", -1);
	}

	ConfMan.flushToDisk();

	return result;
}

Common::Error Ultima8Engine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	// Hack - don't save mouse over status for gumps
	Gump *gump = _mouse->getMouseOverGump();
	if (gump)
		gump->onMouseLeft();

	Gump *modalGump = _desktopGump->FindGump<ModalGump>();
	if (modalGump)
		modalGump->HideGump();

	_mouse->pushMouseCursor(Mouse::MOUSE_WAIT);

	// Redraw to indicate busy and for save thumbnail
	paint();

	if (modalGump)
		modalGump->UnhideGump();

	_saveCount++;

	SavegameWriter *sgw = new SavegameWriter(stream);
	Common::MemoryWriteStreamDynamic buf(DisposeAfterUse::YES);

	_gameInfo->save(&buf);
	sgw->writeFile("GAME", &buf);
	buf.seek(0);

	writeSaveInfo(&buf);
	sgw->writeFile("INFO", &buf);
	buf.seek(0);

	_kernel->save(&buf);
	sgw->writeFile("KERNEL", &buf);
	buf.seek(0);

	_objectManager->save(&buf);
	sgw->writeFile("OBJECTS", &buf);
	buf.seek(0);

	_world->save(&buf);
	sgw->writeFile("WORLD", &buf);
	buf.seek(0);

	_world->saveMaps(&buf);
	sgw->writeFile("MAPS", &buf);
	buf.seek(0);

	_world->getCurrentMap()->save(&buf);
	sgw->writeFile("CURRENTMAP", &buf);
	buf.seek(0);

	_ucMachine->saveStrings(&buf);
	sgw->writeFile("UCSTRINGS", &buf);
	buf.seek(0);

	_ucMachine->saveGlobals(&buf);
	sgw->writeFile("UCGLOBALS", &buf);
	buf.seek(0);

	_ucMachine->saveLists(&buf);
	sgw->writeFile("UCLISTS", &buf);
	buf.seek(0);

	save(&buf);
	sgw->writeFile("APP", &buf);
	buf.seek(0);

	sgw->finish();

	delete sgw;

	// Restore mouse over
	if (gump) gump->onMouseOver();

	debug(1, "Done");

	_mouse->popMouseCursor();

	return Common::kNoError;
}

void Ultima8Engine::resetEngine() {
	debug(1, "-- Resetting Engine --");

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
	_inverterGump = nullptr;

	// reset mouse cursor
	_mouse->popAllCursors();
	_mouse->pushMouseCursor(Mouse::MOUSE_NORMAL);

	_timeOffset = -(int32)Kernel::get_instance()->getFrameNum();
	_inversion = 0;
	_saveCount = 0;
	_hasCheated = false;

	debug(1, "-- Engine Reset --");
}

void Ultima8Engine::setupCoreGumps() {
	debug(1, "Setting up core game gumps...");

	Rect dims;
	_screen->GetSurfaceDims(dims);

	debug(1, "Creating Desktop...");
	_desktopGump = new DesktopGump(0, 0, dims.width(), dims.height());
	_desktopGump->InitGump(0);
	_desktopGump->MakeFocus();

	ConfMan.registerDefault("fadedModal", true);
	bool faded_modal = ConfMan.getBool("fadedModal");
	DesktopGump::SetFadedModal(faded_modal);

	if (GAME_IS_U8) {
		debug(1, "Creating Inverter...");
		_inverterGump = new InverterGump(0, 0, dims.width(), dims.height());
		_inverterGump->InitGump(0);
	}
	debug(1, "Creating GameMapGump...");
	_gameMapGump = new GameMapGump(0, 0, dims.width(), dims.height());
	_gameMapGump->InitGump(0);

	// TODO: clean this up
	if (GAME_IS_U8) {
		assert(_desktopGump->getObjId() == 256);
		assert(_inverterGump->getObjId() == 257);
		assert(_gameMapGump->getObjId() == 258);
	}

	for (uint16 i = 259; i < 384; ++i)
		_objectManager->reserveObjId(i);
}

bool Ultima8Engine::newGame(int saveSlot) {
	debug(1, "Starting New Game (slot %d)... ", saveSlot);

	resetEngine();

	setupCoreGumps();

	if (!_game->startGame())
		return false;

	debug(1, "Create Camera...");
	CameraProcess::SetCameraProcess(new CameraProcess(kMainActorId));

	debug(1, "Create persistent Processes...");
	if (GAME_IS_U8)
		_avatarMoverProcess = new U8AvatarMoverProcess();
	else
		_avatarMoverProcess = new CruAvatarMoverProcess();
	_kernel->addProcess(_avatarMoverProcess);

	if (GAME_IS_U8)
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

	if (GAME_IS_CRUSADER) {
		_kernel->addProcess(new TargetReticleProcess());
		_kernel->addProcess(new ItemSelectionProcess());
		_kernel->addProcess(new CrosshairProcess());
		_kernel->addProcess(new CycleProcess());
		_kernel->addProcess(new SnapProcess());
	}

	_game->startInitialUsecode(saveSlot);

	if (saveSlot == -1)
		ConfMan.setInt("lastSave", -1);

	return true;
}

void Ultima8Engine::syncSoundSettings() {
	Engine::syncSoundSettings();

	// Update music volume
	AudioMixer *audioMixer = AudioMixer::get_instance();
	MidiPlayer *midiPlayer = audioMixer ? audioMixer->getMidiPlayer() : nullptr;
	if (midiPlayer)
		midiPlayer->syncSoundSettings();
}

void Ultima8Engine::applyGameSettings() {
	Engine::applyGameSettings();

	bool fontOverride = ConfMan.getBool("font_override");
	bool fontAntialiasing = ConfMan.getBool("font_antialiasing");

	if (_fontOverride != fontOverride || _fontAntialiasing != fontAntialiasing) {
		_fontOverride = fontOverride;
		_fontAntialiasing = fontAntialiasing;

		_fontManager->resetGameFonts();

		// TODO: assign names to these fontnumbers somehow
		_fontManager->loadTTFont(0, "Vera.ttf", 18, 0xFFFFFF, 0);
		_fontManager->loadTTFont(1, "VeraBd.ttf", 12, 0xFFFFFF, 0);
		// GameWidget's version number information:
		_fontManager->loadTTFont(2, "Vera.ttf", 8, 0xA0A0A0, 0);

		_gameData->setupFontOverrides();
	}

	_frameSkip = ConfMan.getBool("frameSkip");
	_frameLimit = ConfMan.getBool("frameLimit");
	_interpolate = ConfMan.getBool("interpolate");
	_cheatsEnabled = ConfMan.getBool("cheat");
}

void Ultima8Engine::openConfigDialog() {
	GUI::ConfigDialog dlg;
	dlg.runModal();

	g_system->applyBackendSettings();
	applyGameSettings();
	syncSoundSettings();
}

Common::Error Ultima8Engine::loadGameStream(Common::SeekableReadStream *stream) {
	SavegameReader *sg = new SavegameReader(stream);
	SavegameReader::State state = sg->isValid();
	if (state == SavegameReader::SAVE_CORRUPT) {
		delete sg;
		return Common::Error(Common::kReadingFailed, "Invalid or corrupt savegame");
	}

	if (state != SavegameReader::SAVE_VALID) {
		delete sg;
		return Common::Error(Common::kReadingFailed, "Unsupported savegame version");
	}

	_mouse->pushMouseCursor(Mouse::MOUSE_WAIT);

	// Redraw to indicate busy
	paint();

	Common::SeekableReadStream *ds;
	GameInfo saveinfo;
	ds = sg->getDataSource("GAME");
	uint32 version = sg->getVersion();
	bool ok = saveinfo.load(ds, version);

	if (!ok) {
		delete sg;
		return Common::Error(Common::kReadingFailed, "Invalid or corrupt savegame: missing GameInfo");
	}

	if (!_gameInfo->match(saveinfo, true)) {
		Std::string message = "Game mismatch\n";
		message += "Running _game: " + _gameInfo->getPrintDetails()  + "\n";
		message += "Savegame    : " + saveinfo.getPrintDetails();

#ifdef DEBUG
		ConfMan.registerDefault("ignore_savegame_mismatch", true);
		bool ignore = ConfMan.getBool("ignore_savegame_mismatch");

		if (!ignore) {
			error("%s", message.c_str());
		}
		debug(1, "%s", message.c_str());
#else
		delete sg;
		return Common::Error(Common::kReadingFailed, message);
#endif
	}

	resetEngine();

	setupCoreGumps();

	// and load everything back (order matters)
	// for each entry, check that we read exactly the number of bytes
	// expected - anything else suggests a corrupt save (or a bug)
	bool totalok = true;

	Std::string message;

	// UCSTRINGS, UCGLOBALS, UCLISTS don't depend on anything else,
	// so load these first
	ds = sg->getDataSource("UCSTRINGS");
	ok = _ucMachine->loadStrings(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "UCSTRINGS: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "UCSTRINGS: failed\n";
	delete ds;

	ds = sg->getDataSource("UCGLOBALS");
	ok = _ucMachine->loadGlobals(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "UCGLOBALS: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "UCGLOBALS: failed\n";
	delete ds;

	ds = sg->getDataSource("UCLISTS");
	ok = _ucMachine->loadLists(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "UCLISTS: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "UCLISTS: failed\n";
	delete ds;

	// KERNEL must be before OBJECTS, for the egghatcher
	// KERNEL must be before APP, for the _avatarMoverProcess
	ds = sg->getDataSource("KERNEL");
	ok = _kernel->load(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "KERNEL: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "KERNEL: failed\n";
	delete ds;

	ds = sg->getDataSource("APP");
	ok = load(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "APP: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "APP: failed\n";
	delete ds;

	// WORLD must be before OBJECTS, for the egghatcher
	ds = sg->getDataSource("WORLD");
	ok = _world->load(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "WORLD: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "WORLD: failed\n";
	delete ds;

	ds = sg->getDataSource("CURRENTMAP");
	ok = _world->getCurrentMap()->load(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "CURRENTMAP: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "CURRENTMAP: failed\n";
	delete ds;

	ds = sg->getDataSource("OBJECTS");
	ok = _objectManager->load(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "OBJECTS: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "OBJECTS: failed\n";
	delete ds;

	ds = sg->getDataSource("MAPS");
	ok = _world->loadMaps(ds, version);
	ok &= (ds->pos() == ds->size() && !ds->eos());
	totalok &= ok;
	debug(1, "MAPS: %s", (ok ? "ok" : "failed"));
	if (!ok) message += "MAPS: failed\n";
	delete ds;

	// Reset mouse cursor
	_mouse->popAllCursors();
	_mouse->pushMouseCursor(Mouse::MOUSE_NORMAL);

	/*
	// In case of bugs, ensure persistent processes are around?
	if (!TargetReticleProcess::get_instance())
		_kernel->addProcess(new TargetReticleProcess());
	if (!ItemSelectionProcess::get_instance())
		_kernel->addProcess(new ItemSelectionProcess());
	if (!CrosshairProcess::get_instance())
		_kernel->addProcess(new CrosshairProcess());
	if (!CycleProcess::get_instance())
		_kernel->addProcess(new CycleProcess());
	if (!SnapProcess::get_instance())
		_kernel->addProcess(new SnapProcess());
	 */

	if (!totalok) {
		delete sg;
		return Common::Error(Common::kReadingFailed, message);
	}

	debug(1, "Done");

	delete sg;
	return Common::kNoError;
}

void Ultima8Engine::setError(Common::Error &error) {
	_lastError = error;
}

Gump *Ultima8Engine::getGump(uint16 gumpid) {
	return dynamic_cast<Gump *>(ObjectManager::get_instance()->
		getObject(gumpid));
}

void Ultima8Engine::addGump(Gump *gump) {
	// TODO: At some point, this will have to _properly_ choose to
	// which 'layer' to add the gump: inverted, scaled or neither.

	assert(_desktopGump);

	if (dynamic_cast<ShapeViewerGump *>(gump) || dynamic_cast<MiniMapGump *>(gump) ||
		dynamic_cast<MessageBoxGump *>(gump)// ||
		//(_fontOverrides && (dynamic_cast<BarkGump *>(gump) ||
		//                dynamic_cast<AskGump *>(gump)))
		) {
		_desktopGump->AddChild(gump);
	} else if (dynamic_cast<GameMapGump *>(gump)) {
		if (GAME_IS_U8)
			_inverterGump->AddChild(gump);
		else
			_desktopGump->AddChild(gump);
	} else if (dynamic_cast<InverterGump *>(gump)) {
		_desktopGump->AddChild(gump);
	} else if (dynamic_cast<DesktopGump *>(gump)) {
	} else {
		_desktopGump->AddChild(gump);
	}
}

uint32 Ultima8Engine::getGameTimeInSeconds() {
	// 1 second per every 30 frames
	return (Kernel::get_instance()->getFrameNum() + _timeOffset) / Kernel::FRAMES_PER_SECOND; // constant!
}

void Ultima8Engine::moveKeyEvent() {
	_moveKeyFrame = Kernel::get_instance()->getFrameNum();
}

bool Ultima8Engine::moveKeyDownRecently() {
	uint32 nowframe = Kernel::get_instance()->getFrameNum();
	return (nowframe - _moveKeyFrame) < 2 * Kernel::FRAMES_PER_SECOND;
}

void Ultima8Engine::save(Common::WriteStream *ws) {
	uint8 s = (_avatarInStasis ? 1 : 0);
	ws->writeByte(s);

	if (GAME_IS_CRUSADER) {
		uint8 f = (_crusaderTeleporting ? 1 : 0);
		ws->writeByte(f);
	}

	int32 absoluteTime = Kernel::get_instance()->getFrameNum() + _timeOffset;
	ws->writeUint32LE(static_cast<uint32>(absoluteTime));
	ws->writeUint16LE(_avatarMoverProcess->getPid());

	PaletteManager::get_instance()->saveTransforms(*ws);

	ws->writeUint16LE(static_cast<uint16>(_inversion));

	ws->writeUint32LE(_saveCount);

	uint8 c = (_hasCheated ? 1 : 0);
	ws->writeByte(c);
}

bool Ultima8Engine::load(Common::ReadStream *rs, uint32 version) {
	_avatarInStasis = (rs->readByte() != 0);

	if (GAME_IS_CRUSADER) {
		_crusaderTeleporting = (rs->readByte() != 0);
		_cruStasis = false;
	}

	// no gump should be moused over after load
	_mouse->resetMouseOverGump();

	int32 absoluteTime = static_cast<int32>(rs->readUint32LE());
	_timeOffset = absoluteTime - Kernel::get_instance()->getFrameNum();

	uint16 amppid = rs->readUint16LE();
	_avatarMoverProcess = dynamic_cast<AvatarMoverProcess *>(Kernel::get_instance()->getProcess(amppid));

	if (!PaletteManager::get_instance()->loadTransforms(*rs))
		return false;

	_inversion = rs->readUint16LE();

	_saveCount = rs->readUint32LE();

	_hasCheated = (rs->readByte() != 0);

	// Integrity checks
	if (!_avatarMoverProcess) {
		warning("No AvatarMoverProcess.  Corrupt savegame?");
		return false;
	}
	if (_saveCount > 1024*1024) {
		warning("Improbable savecount %d.  Corrupt savegame?", _saveCount);
		return false;
	}

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
	return Kernel::get_instance()->getTickNum();
}

uint32 Ultima8Engine::I_setAvatarInStasis(const uint8 *args, unsigned int argsize) {
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

uint32 Ultima8Engine::I_setCruStasis(const uint8 *args, unsigned int argsize) {
	get_instance()->setCruStasis(true);
	return 0;
}

uint32 Ultima8Engine::I_clrCruStasis(const uint8 *args, unsigned int argsize) {
	get_instance()->setCruStasis(false);
	return 0;
}

uint32 Ultima8Engine::I_getTimeInGameHours(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	// 900 seconds per _game hour
	return get_instance()->getGameTimeInSeconds() / 900;
}

uint32 Ultima8Engine::I_getCrusaderTeleporting(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	return get_instance()->isCrusaderTeleporting() ? 1 : 0;
}

uint32 Ultima8Engine::I_setCrusaderTeleporting(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	get_instance()->setCrusaderTeleporting(true);
	return 0;
}

uint32 Ultima8Engine::I_clrCrusaderTeleporting(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	get_instance()->setCrusaderTeleporting(false);
	return 0;
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
	int32 absolute = newhour * 27000;
	get_instance()->_timeOffset = absolute - Kernel::get_instance()->getFrameNum();

	return 0;
}

uint32 Ultima8Engine::I_closeItemGumps(const uint8 *args, unsigned int /*argsize*/) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	g->getDesktopGump()->CloseItemDependents();

	return 0;
}

uint32 Ultima8Engine::I_moveKeyDownRecently(const uint8 *args, unsigned int /*argsize*/) {
	Ultima8Engine *g = Ultima8Engine::get_instance();
	return g->moveKeyDownRecently() ? 1 : 0;
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
	if (!f.open("pentagram.png") || !png.loadStream(f))
		return;

	// Blit the splash image to the _screen
	Graphics::Screen *scr = Ultima8Engine::get_instance()->getScreen();
	const Graphics::Surface *srcSurface = png.getSurface();

	Common::Rect dest(0, 0, scr->w, scr->h);

	// Splash screen is expected to be 640x480.
	// If the window has a different aspect ratio or corrected aspect ratio,
	// then scale to appropriate size and center.
	frac_t aspectRatio = Common::Rational(scr->w, scr->h).toFrac();
	if (aspectRatio != Common::Rational(320, 240).toFrac() &&
		aspectRatio != Common::Rational(320, 200).toFrac()) {
		Common::Rational scaleFactorX(scr->w, srcSurface->w);
		Common::Rational scaleFactorY(scr->h, srcSurface->h);
		Common::Rational scale = scaleFactorX < scaleFactorY ? scaleFactorX : scaleFactorY;

		dest.setWidth((srcSurface->w * scale).toInt());
		dest.setHeight((srcSurface->h * scale).toInt());
		dest.moveTo((scr->w - dest.width()) / 2, (scr->h - dest.height()) / 2);
	}

	scr->transBlitFrom(*srcSurface, Common::Rect(0, 0, srcSurface->w, srcSurface->h), dest);
	scr->update();
	// Handle a single event to get the splash screen shown
	Common::Event event;
	pollEvent(event);
}

bool Ultima8Engine::pollEvent(Common::Event &event) {
	uint32 timer = g_system->getMillis();

	if (timer >= (_priorFrameCounterTime + GAME_FRAME_TIME)) {
		// Time to build up next game frame
		_priorFrameCounterTime = timer;

		// Render anything pending for the screen
		Graphics::Screen *screen = getScreen();
		if (screen)
			screen->update();
	}

	// Event handling
	return g_system->getEventManager()->pollEvent(event);
}

} // End of namespace Ultima8
} // End of namespace Ultima
