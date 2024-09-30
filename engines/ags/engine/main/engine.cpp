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

//
// Engine initialization
//

#include "ags/shared/core/platform.h"
#include "ags/lib/allegro.h" // allegro_install and _exit
#include "ags/engine/ac/asset_helper.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/character.h"
#include "ags/engine/ac/character_extras.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/ac/gui.h"
#include "ags/engine/ac/lip_sync.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/route_finder.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/speech.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/translation.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/ac/dynobj/script_object.h"
#include "ags/engine/ac/dynobj/script_system.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/shared/font/ags_font_renderer.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gfx/image.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/gfx/gfx_driver_factory.h"
#include "ags/engine/gfx/ddb.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/game_file.h"
#include "ags/engine/main/game_start.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/engine_setup.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/main/main.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/error.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_utils.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

bool engine_init_backend() {
	set_our_eip(-199);
	_G(platform)->PreBackendInit();
	// Initialize SDL
	Debug::Printf(kDbgMsg_Info, "Initializing backend libs");
	if (sys_main_init()) {
		const char *user_hint = _G(platform)->GetBackendFailUserHint();
		_G(platform)->DisplayAlert("Unable to initialize SDL library.\n\n%s",
		                           user_hint);
		return false;
	}

	// Initialize stripped allegro library
	if (install_allegro()) {
		_G(platform)->DisplayAlert("Internal error: unable to initialize stripped Allegro 4 library.");
		return false;
	}

	_G(platform)->PostBackendInit();
	return true;
}

void winclosehook() {
	_G(want_exit) = true;
	_G(abort_engine) = true;
	_G(check_dynamic_sprites_at_exit) = 0;
	AbortGame();
}

void engine_setup_window() {
	Debug::Printf(kDbgMsg_Info, "Setting up window");

	set_our_eip(-198);
	sys_window_set_title(_GP(game).gamename.GetCStr());
	sys_window_set_icon();
	sys_evt_set_quit_callback(winclosehook);
	set_our_eip(-197);
}

// Fills map with game settings, to e.g. let setup application(s)
// display correct properties to the user
static void fill_game_properties(StringOrderMap &map) {
	map["title"] = _GP(game).gamename;
	map["guid"] = _GP(game).guid;
	map["legacy_uniqueid"] = StrUtil::IntToString(_GP(game).uniqueid);
	map["legacy_resolution"] = StrUtil::IntToString(_GP(game).GetResolutionType());
	map["legacy_letterbox"] = StrUtil::IntToString(_GP(game).options[OPT_LETTERBOX]);
	map["resolution_width"] = StrUtil::IntToString(_GP(game).GetDefaultRes().Width);
	map["resolution_height"] = StrUtil::IntToString(_GP(game).GetDefaultRes().Height);
	map["resolution_bpp"] = StrUtil::IntToString(_GP(game).GetColorDepth());
	map["render_at_screenres"] = StrUtil::IntToString(
	                                 _GP(game).options[OPT_RENDERATSCREENRES] == kRenderAtScreenRes_UserDefined ? -1 :
	                                 (_GP(game).options[OPT_RENDERATSCREENRES] == kRenderAtScreenRes_Enabled ? 1 : 0));
}

// Starts up setup application, if capable.
// Returns TRUE if should continue running the game, otherwise FALSE.
bool engine_run_setup(const ConfigTree &cfg, int &app_res) {
	app_res = EXIT_NORMAL;
#if AGS_PLATFORM_OS_WINDOWS
	{
		Debug::Printf(kDbgMsg_Info, "Running Setup");

		ConfigTree cfg_with_meta = cfg;
		fill_game_properties(cfg_with_meta["gameproperties"]);
		ConfigTree cfg_out;
		SetupReturnValue res = _G(platform)->RunSetup(cfg_with_meta, cfg_out);
		if (res != kSetup_Cancel) {
			String cfg_file = PreparePathForWriting(GetGameUserConfigDir(), DefaultConfigFileName);
			if (cfg_file.IsEmpty()) {
				_G(platform)->DisplayAlert("Unable to write into directory '%s'.\n%s",
				                           GetGameUserConfigDir().FullDir.GetCStr(), _G(platform)->GetDiskWriteAccessTroubleshootingText());
			} else if (!IniUtil::Merge(cfg_file, cfg_out)) {
				_G(platform)->DisplayAlert("Unable to write to the configuration file (error code 0x%08X).\n%s",
				                           _G(platform)->GetLastSystemError(), _G(platform)->GetDiskWriteAccessTroubleshootingText());
			}
		}
		if (res != kSetup_RunGame)
			return false;

		// TODO: investigate if the full program restart may (should) be avoided

		// Just re-reading the config file seems to cause a caching
		// problem on Win9x, so let's restart the process.
		sys_main_shutdown();
		allegro_exit();
		char quotedpath[MAX_PATH];
		snprintf(quotedpath, MAX_PATH, "\"%s\"", _G(appPath).GetCStr());
		_spawnl(_P_OVERLAY, _G(appPath), quotedpath, NULL);
	}
#endif
	return true;
}

// Scans given directory for the AGS game config. If such config exists
// and it contains data file name, then returns one.
// Otherwise returns empty string.
static String find_game_data_in_config(const String &path) {
	// First look for config
	ConfigTree cfg;
	String def_cfg_file = Path::ConcatPaths(path, DefaultConfigFileName);
	if (IniUtil::Read(def_cfg_file, cfg)) {
		String data_file = CfgReadString(cfg, "misc", "datafile");
		Debug::Printf("Found game config: %s", def_cfg_file.GetCStr());
		Debug::Printf(" Cfg: data file: %s", data_file.GetCStr());
		// Only accept if it's a relative path
		if (!data_file.IsEmpty() && Path::IsRelativePath(data_file))
			return Path::ConcatPaths(path, data_file);
	}
	return ""; // not found in config
}

// Scans for game data in several common locations.
// When it does so, it first looks for game config file, which contains
// explicit directions to game data in its settings.
// If such config is not found, it scans same location for *any* game data instead.
String search_for_game_data_file(String &was_searching_in) {
	Debug::Printf("Looking for the game data.\n Cwd: %s\n Path arg: %s",
	              Directory::GetCurrentDirectory().GetCStr(),
	              _G(cmdGameDataPath).GetCStr());
	// 1. From command line argument, which may be a directory or actual file
	if (!_G(cmdGameDataPath).IsEmpty()) {
		if (File::IsFile(_G(cmdGameDataPath)))
			return _G(cmdGameDataPath); // this path is a file
		if (!File::IsDirectory(_G(cmdGameDataPath)))
			return ""; // path is neither file nor directory
		was_searching_in = _G(cmdGameDataPath);
		Debug::Printf("Searching in (cmd arg): %s", was_searching_in.GetCStr());
		// first scan for config
		String data_path = find_game_data_in_config(_G(cmdGameDataPath));
		if (!data_path.IsEmpty())
			return data_path;
		// if not found in config, lookup for data in same dir
		return FindGameData(_G(cmdGameDataPath));
	}

	// 2. Look in other known locations
	// 2.1. Look for attachment in the running executable
	if (!_G(appPath).IsEmpty() && Shared::AssetManager::IsDataFile(_G(appPath))) {
		Debug::Printf("Found game data embedded in executable");
		was_searching_in = Path::GetDirectoryPath(_G(appPath));
		return _G(appPath);
	}

	// 2.2 Look in current working directory
	String cur_dir = Directory::GetCurrentDirectory();
	was_searching_in = cur_dir;
	Debug::Printf("Searching in (cwd): %s", was_searching_in.GetCStr());
	// first scan for config
	String data_path = find_game_data_in_config(cur_dir);
	if (!data_path.IsEmpty())
		return data_path;
	// if not found in config, lookup for data in same dir
	data_path = FindGameData(cur_dir);
	if (!data_path.IsEmpty())
		return data_path;

	// 2.3 Look in executable's directory (if it's different from current dir)
	if (Path::ComparePaths(_G(appDirectory), cur_dir) == 0)
		return ""; // no luck
	was_searching_in = _G(appDirectory);
	Debug::Printf("Searching in (exe dir): %s", was_searching_in.GetCStr());
	// first scan for config
	data_path = find_game_data_in_config(_G(appDirectory));
	if (!data_path.IsEmpty())
		return data_path;
	// if not found in config, lookup for data in same dir
	return FindGameData(_G(appDirectory));
}

void engine_init_fonts() {
	Debug::Printf(kDbgMsg_Info, "Initializing TTF renderer");

	init_font_renderer();
}

void engine_init_mouse() {
	int res = _GP(mouse).GetButtonCount();
	if (res < 0)
		Debug::Printf(kDbgMsg_Info, "Initializing mouse: failed");
	else
		Debug::Printf(kDbgMsg_Info, "Initializing mouse: number of buttons reported is %d", res);
	_GP(mouse).SetSpeed(_GP(usetup).mouse_speed);
}

void engine_locate_speech_pak() {
	init_voicepak("");
}

void engine_locate_audio_pak() {
	String music_file = _GP(game).GetAudioVOXName();
	String music_filepath = find_assetlib(music_file);
	if (!music_filepath.IsEmpty()) {
		if (_GP(AssetMgr)->AddLibrary(music_filepath) == kAssetNoError) {
			Debug::Printf(kDbgMsg_Info, "%s found and initialized.", music_file.GetCStr());
			_GP(ResPaths).AudioPak.Name = music_file;
			_GP(ResPaths).AudioPak.Path = music_filepath;
		} else {
			_G(platform)->DisplayAlert("Unable to initialize digital audio pack '%s', file could be corrupt or of unsupported format.",
				music_file.GetCStr());
		}
	} else if (!_GP(ResPaths).AudioDir2.IsEmpty() &&
			Path::ComparePaths(_GP(ResPaths).DataDir, _GP(ResPaths).AudioDir2) != 0) {
		Debug::Printf(kDbgMsg_Info, "Audio pack was not found, but explicit audio directory is defined.");
	}
}

// Assign asset locations to the AssetManager
void engine_assign_assetpaths() {
	_GP(AssetMgr)->AddLibrary(_GP(ResPaths).GamePak.Path, ",audio"); // main pack may have audio bundled too
	// The asset filters are currently a workaround for limiting search to certain locations;
	// this is both an optimization and to prevent unexpected behavior.
	// - empty filter is for regular files
	// audio - audio clips
	// voice - voice-over clips
	// NOTE: we add extra optional directories first because they should have higher priority
	// TODO: maybe change AssetManager library order to stack-like later (last added = top priority)?
	if (!_GP(ResPaths).DataDir2.IsEmpty() && Path::ComparePaths(_GP(ResPaths).DataDir2, _GP(ResPaths).DataDir) != 0)
		_GP(AssetMgr)->AddLibrary(_GP(ResPaths).DataDir2, ",audio,voice"); // dir may have anything
	if (!_GP(ResPaths).AudioDir2.IsEmpty() && Path::ComparePaths(_GP(ResPaths).AudioDir2, _GP(ResPaths).DataDir) != 0)
		_GP(AssetMgr)->AddLibrary(_GP(ResPaths).AudioDir2, "audio");
	if (!_GP(ResPaths).VoiceDir2.IsEmpty() && Path::ComparePaths(_GP(ResPaths).VoiceDir2, _GP(ResPaths).DataDir) != 0)
		_GP(AssetMgr)->AddLibrary(_GP(ResPaths).VoiceDir2, "voice");

	_GP(AssetMgr)->AddLibrary(_GP(ResPaths).DataDir, ",audio,voice"); // dir may have anything
	if (!_GP(ResPaths).AudioPak.Path.IsEmpty())
		_GP(AssetMgr)->AddLibrary(_GP(ResPaths).AudioPak.Path, "audio");
	if (!_GP(ResPaths).SpeechPak.Path.IsEmpty())
		_GP(AssetMgr)->AddLibrary(_GP(ResPaths).SpeechPak.Path, "voice");
}

void engine_init_keyboard() {
	/* do nothing */
}

void engine_init_audio() {
#if !AGS_PLATFORM_SCUMMVM
	if (usetup.audio_backend != 0) {
		Debug::Printf("Initializing audio");
		try {
			audio_core_init(); // audio core system
		} catch (std::runtime_error ex) {
			Debug::Printf(kDbgMsg_Error, "Failed to initialize audio: %s", ex.what());
			usetup.audio_backend = 0;
		}
	}
#endif

	if (!_GP(usetup).audio_enabled) {
		// all audio is disabled
		Debug::Printf(kDbgMsg_Info, "Audio is disabled");
	}
}

void engine_init_debug() {
	if (_GP(usetup).show_fps)
		_G(display_fps) = kFPS_Forced;
	if ((_G(debug_flags) & (~DBG_DEBUGMODE)) > 0) {
		_G(platform)->DisplayAlert("Engine debugging enabled.\n"
		                           "\nNOTE: You have selected to enable one or more engine debugging options.\n"
		                           "These options cause many parts of the game to behave abnormally, and you\n"
		                           "may not see the game as you are used to it. The point is to test whether\n"
		                           "the engine passes a point where it is crashing on you normally.\n"
		                           "[Debug flags enabled: 0x%02X]", _G(debug_flags));
	}
}

void engine_init_pathfinder() {
	init_pathfinder(_G(loaded_game_file_version));
}

void engine_pre_init_gfx() {
	//Debug::Printf("Initialize gfx");

	//_G(platform)->InitialiseAbufAtStartup();
}

int engine_load_game_data() {
	Debug::Printf("Load game data");
	set_our_eip(-17);
	HError err = load_game_file();
	if (!err) {
		_G(proper_exit) = 1;
		display_game_file_error(err);
		return EXIT_ERROR;
	}
	return 0;
}

// Replace special tokens inside a user path option
static void resolve_configured_path(String &option) {
	option.Replace(Shared::String("$GAMENAME$"), _GP(game).gamename);
}

// Setup paths and directories that may be affected by user configuration
void engine_init_user_directories() {
	resolve_configured_path(_GP(usetup).user_data_dir);
	resolve_configured_path(_GP(usetup).shared_data_dir);

	if (!_GP(usetup).user_conf_dir.IsEmpty())
		Debug::Printf(kDbgMsg_Info, "User config directory: %s", _GP(usetup).user_conf_dir.GetCStr());
	if (!_GP(usetup).user_data_dir.IsEmpty())
		Debug::Printf(kDbgMsg_Info, "User data directory: %s", _GP(usetup).user_data_dir.GetCStr());
	if (!_GP(usetup).shared_data_dir.IsEmpty())
		Debug::Printf(kDbgMsg_Info, "Shared data directory: %s", _GP(usetup).shared_data_dir.GetCStr());

	// Initialize default save directory early, for we'll need it to set restart point
	SetDefaultSaveDirectory();
}

#if AGS_PLATFORM_OS_ANDROID
extern char android_base_directory[256];
#endif // AGS_PLATFORM_OS_ANDROID

// TODO: remake/remove this nonsense
int check_write_access() {
#if AGS_PLATFORM_SCUMMVM
	return true;
#else

	set_our_eip(-1895);

	// The Save Game Dir is the only place that we should write to
	String svg_dir = get_save_game_directory();
	if (platform->GetDiskFreeSpaceMB(svg_dir) < 2)
		return 0;
	String tempPath = String::FromFormat("%s""tmptest.tmp", svg_dir.GetCStr());
	Stream *temp_s = Shared::File::CreateFile(tempPath);
	if (!temp_s)
		// TODO: The fallback should be done on all platforms, and there's
		// already similar procedure found in SetSaveGameDirectoryPath.
		// If Android has extra dirs to fallback to, they should be provided
		// by platform driver's method, not right here!
#if AGS_PLATFORM_OS_ANDROID
	{
		put_backslash(android_base_directory);
		tempPath.Format("%s""tmptest.tmp", android_base_directory);
		temp_s = Shared::File::CreateFile(tempPath);
		if (temp_s == NULL) return 0;
		else SetCustomSaveParent(android_base_directory);
	}
#else
		return 0;
#endif // AGS_PLATFORM_OS_ANDROID

	set_our_eip(-1896);

	temp_s->Write("just to test the drive free space", 30);
	delete temp_s;

	set_our_eip(-1897);

	if (File::DeleteFile(tempPath))
		return 0;

	return 1;
#endif
}

int engine_check_disk_space() {
	Debug::Printf(kDbgMsg_Info, "Checking for disk space");

	if (check_write_access() == 0) {
		_G(platform)->DisplayAlert("Unable to write in the savegame directory.\n%s", _G(platform)->GetDiskWriteAccessTroubleshootingText());
		_G(proper_exit) = 1;
		return EXIT_ERROR;
	}

	return 0;
}

int engine_check_font_was_loaded() {
	if (!font_first_renderer_loaded()) {
		_G(platform)->DisplayAlert("No game fonts found. At least one font is required to run the _GP(game).");
		_G(proper_exit) = 1;
		return EXIT_ERROR;
	}

	return 0;
}

// Do the preload graphic if available
void show_preload() {
	RGB temppal[256];
	Bitmap *splashsc = BitmapHelper::CreateRawBitmapOwner(load_pcx("preload.pcx", temppal));
	if (splashsc != nullptr) {
		Debug::Printf("Displaying preload image");
		if (splashsc->GetColorDepth() == 8)
			set_palette_range(temppal, 0, 255, 0);
		if (_G(gfxDriver)->UsesMemoryBackBuffer())
			_G(gfxDriver)->GetMemoryBackBuffer()->Clear();

		const Rect &view = _GP(play).GetMainViewport();
		Bitmap *tsc = BitmapHelper::CreateBitmapCopy(splashsc, _GP(game).GetColorDepth());
		if (!_G(gfxDriver)->HasAcceleratedTransform() && view.GetSize() != tsc->GetSize()) {
			Bitmap *stretched = new Bitmap(view.GetWidth(), view.GetHeight(), tsc->GetColorDepth());
			stretched->StretchBlt(tsc, RectWH(0, 0, view.GetWidth(), view.GetHeight()));
			delete tsc;
			tsc = stretched;
		}
		IDriverDependantBitmap *ddb = _G(gfxDriver)->CreateDDBFromBitmap(tsc, false, true);
		ddb->SetStretch(view.GetWidth(), view.GetHeight());
		_G(gfxDriver)->ClearDrawLists();
		_G(gfxDriver)->BeginSpriteBatch(view);
		_G(gfxDriver)->DrawSprite(0, 0, ddb);
		_G(gfxDriver)->EndSpriteBatch();
		render_to_screen();
		_G(gfxDriver)->DestroyDDB(ddb);
		delete splashsc;
		delete tsc;
		_G(platform)->Delay(500);
	}
}

int engine_init_sprites() {
	Debug::Printf(kDbgMsg_Info, "Initialize sprites");
	HError err = _GP(spriteset).InitFile(SpriteFile::DefaultSpriteFileName, SpriteFile::DefaultSpriteIndexName);
	if (!err) {
		sys_main_shutdown();
		allegro_exit();
		_G(proper_exit) = 1;
		_G(platform)->DisplayAlert("Could not load sprite set file %s\n%s",
		                           SpriteFile::DefaultSpriteFileName,
		                           err->FullMessage().GetCStr());
		return EXIT_ERROR;
	}

	if (_GP(usetup).SpriteCacheSize > 0)
		_GP(spriteset).SetMaxCacheSize(_GP(usetup).SpriteCacheSize * 1024);
	Debug::Printf("Sprite cache set: %zu KB", _GP(spriteset).GetMaxCacheSize() / 1024);
	return 0;
}

// TODO: this should not be a part of "engine_" function group,
// move this elsewhere (InitGameState?).
void engine_init_game_settings() {
	set_our_eip(-7);
	Debug::Printf("Initialize game settings");

	// Initialize randomizer
	_GP(play).randseed = g_system->getMillis();
	::AGS::g_vm->setRandomNumberSeed(_GP(play).randseed);

	if (_GP(usetup).audio_enabled) {
		_GP(play).separate_music_lib = !_GP(ResPaths).AudioPak.Name.IsEmpty();
		_GP(play).voice_avail = _GP(ResPaths).VoiceAvail;
	} else {
		_GP(play).voice_avail = false;
		_GP(play).separate_music_lib = false;
	}

	// Setup a text encoding mode depending on the game data hint
	if (_GP(game).options[OPT_GAMETEXTENCODING] == 65001) // utf-8 codepage number
		set_uformat(U_UTF8);
	else
		set_uformat(U_ASCII);

	int ee;

	for (ee = 0; ee < 256; ee++) {
		if (_GP(game).paluses[ee] != PAL_BACKGROUND)
			_G(palette)[ee] = _GP(game).defpal[ee];
	}

	for (ee = 0; ee < _GP(game).numcursors; ee++) {
		// The cursor graphics are assigned to mousecurs[] and so cannot
		// be removed from memory
		if (_GP(game).mcurs[ee].pic >= 0)
			_GP(spriteset).PrecacheSprite(_GP(game).mcurs[ee].pic);

		// just in case they typed an invalid view number in the editor
		if (_GP(game).mcurs[ee].view >= _GP(game).numviews)
			_GP(game).mcurs[ee].view = -1;

		if (_GP(game).mcurs[ee].view >= 0)
			precache_view(_GP(game).mcurs[ee].view);
	}
	// may as well preload the character gfx
	if (_G(playerchar)->view >= 0)
		precache_view(_G(playerchar)->view, 0, Character_GetDiagonalWalking(_G(playerchar)) ? 8 : 4);

	set_our_eip(-6);

	for (ee = 0; ee < MAX_ROOM_OBJECTS; ee++) {
		_G(scrObj)[ee].id = ee;
	}

	for (ee = 0; ee < _GP(game).numcharacters; ee++) {
		memset(&_GP(game).chars[ee].inv[0], 0, MAX_INV * sizeof(short));
		_GP(game).chars[ee].activeinv = -1;
		_GP(game).chars[ee].following = -1;
		_GP(game).chars[ee].followinfo = 97 | (10 << 8);
		if (_G(loaded_game_file_version) < kGameVersion_360)
			_GP(game).chars[ee].idletime = 20; // default to 20 seconds		_GP(game).chars[ee].idleleft = _GP(game).chars[ee].idletime;
		_GP(game).chars[ee].transparency = 0;
		_GP(game).chars[ee].baseline = -1;
		_GP(game).chars[ee].walkwaitcounter = 0;
		_GP(game).chars[ee].z = 0;
		_GP(charextra)[ee].xwas = INVALID_X;
		_GP(charextra)[ee].zoom = 100;
		if (_GP(game).chars[ee].view >= 0) {
			// set initial loop to 0
			_GP(game).chars[ee].loop = 0;
			// or to 1 if they don't have up/down frames
			if (_GP(views)[_GP(game).chars[ee].view].loops[0].numFrames < 1)
				_GP(game).chars[ee].loop = 1;
		}
		_GP(charextra)[ee].process_idle_this_time = 0;
		_GP(charextra)[ee].invorder_count = 0;
		_GP(charextra)[ee].slow_move_counter = 0;
		_GP(charextra)[ee].animwait = 0;
	}

	set_our_eip(-5);
	for (ee = 0; ee < _GP(game).numinvitems; ee++) {
		if (_GP(game).invinfo[ee].flags & IFLG_STARTWITH) _G(playerchar)->inv[ee] = 1;
		else _G(playerchar)->inv[ee] = 0;
	}

	//
	// TODO: following big initialization sequence could be in GameState ctor
	_GP(play).score = 0;
	_GP(play).sierra_inv_color = 7;
	// copy the value set by the editor
	if (_GP(game).options[OPT_GLOBALTALKANIMSPD] >= 0) {
		_GP(play).talkanim_speed = _GP(game).options[OPT_GLOBALTALKANIMSPD];
		_GP(game).options[OPT_GLOBALTALKANIMSPD] = 1;
	} else {
		_GP(play).talkanim_speed = -_GP(game).options[OPT_GLOBALTALKANIMSPD] - 1;
		_GP(game).options[OPT_GLOBALTALKANIMSPD] = 0;
	}
	_GP(play).inv_item_wid = 40;
	_GP(play).inv_item_hit = 22;
	_GP(play).messagetime = -1;
	_GP(play).disabled_user_interface = 0;
	_GP(play).gscript_timer = -1;
	_GP(play).debug_mode = _GP(game).options[OPT_DEBUGMODE];
	_GP(play).inv_top = 0;
	_GP(play).inv_numdisp = 0;
	_GP(play).inv_numorder = 0;
	_GP(play).text_speed = 15;
	_GP(play).text_min_display_time_ms = 1000;
	_GP(play).ignore_user_input_after_text_timeout_ms = 500;
	_GP(play).ClearIgnoreInput();
	_GP(play).lipsync_speed = 15;
	_GP(play).close_mouth_speech_time = 10;
	_GP(play).disable_antialiasing = 0;
	_GP(play).rtint_enabled = false;
	_GP(play).rtint_level = 0;
	_GP(play).rtint_light = 0;
	_GP(play).text_speed_modifier = 0;
	_GP(play).text_align = kHAlignLeft;
	// Make the default alignment to the right with right-to-left text
	if (_GP(game).options[OPT_RIGHTLEFTWRITE])
		_GP(play).text_align = kHAlignRight;

	_GP(play).speech_bubble_width = get_fixed_pixel_size(100);
	_GP(play).bg_frame = 0;
	_GP(play).bg_frame_locked = 0;
	_GP(play).bg_anim_delay = 0;
	_GP(play).anim_background_speed = 0;
	_GP(play).mouse_cursor_hidden = 0;
	_GP(play).silent_midi = 0;
	_GP(play).current_music_repeating = 0;
	_GP(play).skip_until_char_stops = -1;
	_GP(play).get_loc_name_last_time = -1;
	_GP(play).get_loc_name_save_cursor = -1;
	_GP(play).restore_cursor_mode_to = -1;
	_GP(play).restore_cursor_image_to = -1;
	_GP(play).ground_level_areas_disabled = 0;
	_GP(play).next_screen_transition = -1;
	_GP(play).temporarily_turned_off_character = -1;
	_GP(play).inv_backwards_compatibility = 0;
	_GP(play).gamma_adjustment = 100;
	_GP(play).music_queue_size = 0;
	_GP(play).shakesc_length = 0;
	_GP(play).wait_counter = 0;
	_GP(play).SetWaitSkipResult(SKIP_NONE);
	_GP(play).key_skip_wait = SKIP_NONE;
	_GP(play).cur_music_number = -1;
	_GP(play).music_repeat = 1;
	_GP(play).music_master_volume = 100 + LegacyMusicMasterVolumeAdjustment;
	_GP(play).digital_master_volume = 100;
	_GP(play).screen_flipped = 0;
	_GP(play).speech_mode = kSpeech_VoiceText;
	_GP(play).speech_skip_style = user_to_internal_skip_speech((SkipSpeechStyle)_GP(game).options[OPT_NOSKIPTEXT]);
	_GP(play).sound_volume = 255;
	_GP(play).speech_volume = 255;
	_GP(play).normal_font = 0;
	_GP(play).speech_font = 1;
	_GP(play).speech_text_shadow = 16;
	_GP(play).screen_tint = -1;
	_GP(play).bad_parsed_word[0] = 0;
	_GP(play).swap_portrait_side = 0;
	_GP(play).swap_portrait_lastchar = -1;
	_GP(play).swap_portrait_lastlastchar = -1;
	_GP(play).in_conversation = 0;
	_GP(play).skip_display = 3;
	_GP(play).no_multiloop_repeat = 0;
	_GP(play).in_cutscene = 0;
	_GP(play).fast_forward = 0;
	_GP(play).totalscore = _GP(game).totalscore;
	_GP(play).roomscript_finished = 0;
	_GP(play).no_textbg_when_voice = 0;
	_GP(play).max_dialogoption_width = get_fixed_pixel_size(180);
	_GP(play).no_hicolor_fadein = 0;
	_GP(play).bgspeech_game_speed = 0;
	_GP(play).bgspeech_stay_on_display = 0;
	_GP(play).unfactor_speech_from_textlength = 0;
	_GP(play).mp3_loop_before_end = 70;
	_GP(play).speech_music_drop = 60;
	_GP(play).room_changes = 0;
	_GP(play).check_interaction_only = 0;
	_GP(play).replay_hotkey_unused = -1;  // StartRecording: not supported.
	_GP(play).dialog_options_x = 0;
	_GP(play).dialog_options_y = 0;
	_GP(play).min_dialogoption_width = 0;
	_GP(play).disable_dialog_parser = 0;
	_GP(play).ambient_sounds_persist = 0;
	_GP(play).screen_is_faded_out = 0;
	_GP(play).player_on_region = 0;
	_GP(play).top_bar_backcolor = 8;
	_GP(play).top_bar_textcolor = 16;
	_GP(play).top_bar_bordercolor = 8;
	_GP(play).top_bar_borderwidth = 1;
	_GP(play).top_bar_ypos = 25;
	_GP(play).top_bar_font = -1;
	_GP(play).screenshot_width = 160;
	_GP(play).screenshot_height = 100;
	_GP(play).speech_text_align = kHAlignCenter;
	_GP(play).auto_use_walkto_points = 1;
	_GP(play).inventory_greys_out = 0;
	_GP(play).skip_speech_specific_key = 0;
	_GP(play).abort_key = 324;  // Alt+X
	_GP(play).fade_to_red = 0;
	_GP(play).fade_to_green = 0;
	_GP(play).fade_to_blue = 0;
	_GP(play).show_single_dialog_option = 0;
	_GP(play).keep_screen_during_instant_transition = 0;
	_GP(play).read_dialog_option_colour = -1;
	_GP(play).stop_dialog_at_end = DIALOG_NONE;
	_GP(play).speech_portrait_placement = 0;
	_GP(play).speech_portrait_x = 0;
	_GP(play).speech_portrait_y = 0;
	_GP(play).speech_display_post_time_ms = 0;
	_GP(play).dialog_options_highlight_color = DIALOG_OPTIONS_HIGHLIGHT_COLOR_DEFAULT;
	_GP(play).speech_has_voice = false;
	_GP(play).speech_voice_blocking = false;
	_GP(play).speech_in_post_state = false;
	_GP(play).complete_overlay_on = 0;
	_GP(play).text_overlay_on = 0;
	_GP(play).narrator_speech = _GP(game).playercharacter;
	_GP(play).crossfading_out_channel = 0;
	_GP(play).speech_textwindow_gui = _GP(game).options[OPT_TWCUSTOM];
	if (_GP(play).speech_textwindow_gui == 0)
		_GP(play).speech_textwindow_gui = -1;
	_GP(play).game_name = _GP(game).gamename;
	_GP(play).lastParserEntry[0] = 0;
	_GP(play).follow_change_room_timer = 150;
	for (ee = 0; ee < MAX_ROOM_BGFRAMES; ee++)
		_GP(play).raw_modified[ee] = 0;
	_GP(play).game_speed_modifier = 0;
	if (_G(debug_flags) & DBG_DEBUGMODE)
		_GP(play).debug_mode = 1;
	_GP(play).shake_screen_yoff = 0;

	GUI::Options.DisabledStyle = static_cast<GuiDisableStyle>(_GP(game).options[OPT_DISABLEOFF]);
	GUI::Options.ClipControls = _GP(game).options[OPT_CLIPGUICONTROLS] != 0;
	// Force GUI metrics recalculation, accommodating for loaded fonts
	GUI::MarkForFontUpdate(-1);

	memset(&_GP(play).walkable_areas_on[0], 1, MAX_WALK_AREAS);
	memset(&_GP(play).script_timers[0], 0, MAX_TIMERS * sizeof(int));
	memset(&_GP(play).default_audio_type_volumes[0], -1, MAX_AUDIO_TYPES * sizeof(int));

	if (!_GP(usetup).translation.IsEmpty())
		Game_ChangeTranslation(_GP(usetup).translation.GetCStr());

	update_invorder();
	_G(displayed_room) = -10;

	_G(currentcursor) = 0;
	set_our_eip(-4);
	_G(mousey) = 100; // stop icon bar popping up

	// We use same variable to read config and be used at runtime for now,
	// so update it here with regards to game design option
	_GP(usetup).RenderAtScreenRes =
	    (_GP(game).options[OPT_RENDERATSCREENRES] == kRenderAtScreenRes_UserDefined && _GP(usetup).RenderAtScreenRes) ||
	    _GP(game).options[OPT_RENDERATSCREENRES] == kRenderAtScreenRes_Enabled;
}

void engine_setup_scsystem_auxiliary() {
	// ScriptSystem::aci_version is only 10 chars long
	snprintf(_GP(scsystem).aci_version, sizeof(_GP(scsystem).aci_version), "%s", _G(EngineVersion).LongString.GetCStr());
	if (_GP(usetup).override_script_os >= 0) {
		_GP(scsystem).os = _GP(usetup).override_script_os;
	} else {
		_GP(scsystem).os = _G(platform)->GetSystemOSID();
	}
}

void engine_prepare_to_start_game() {
	Debug::Printf("Prepare to start game");

	engine_setup_scsystem_auxiliary();

	if (_GP(usetup).load_latest_save) {
#ifndef AGS_PLATFORM_SCUMMVM
		int slot = GetLastSaveSlot();
		if (slot >= 0)
			loadSaveGameOnStartup = get_save_game_path(slot);
#endif
	}
}

// Define location of the game data either using direct settings or searching
// for the available resource packs in common locations.
// Returns two paths:
// - startup_dir: this is where engine found game config and/or data;
// - data_path: full path of the main data pack;
// data_path's directory (may or not be eq to startup_dir) should be considered data directory,
// and this is where engine look for all game data.
HError define_gamedata_location_checkall(String &data_path, String &startup_dir) {
	// First try if they provided a startup option
	if (!_G(cmdGameDataPath).IsEmpty()) {
		// If not a valid path - bail out
		if (!File::IsFileOrDir(_G(cmdGameDataPath)))
			return new Error(String::FromFormat("Provided game location is not a valid path.\n Cwd: %s\n Path: %s",
			    Directory::GetCurrentDirectory().GetCStr(),
			    _G(cmdGameDataPath).GetCStr()));
		// If it's a file, then keep it and proceed
		if (File::IsFile(_G(cmdGameDataPath))) {
			Debug::Printf("Using provided game data path: %s", _G(cmdGameDataPath).GetCStr());
			startup_dir = Path::GetDirectoryPath(_G(cmdGameDataPath));
			data_path = _G(cmdGameDataPath);
			return HError::None();
		}
	}

#if AGS_SEARCH_FOR_GAME_ON_LAUNCH
	// No direct filepath provided, search in common locations.
	data_path = search_for_game_data_file(startup_dir);
	if (data_path.IsEmpty()) {
		return new Error("Engine was not able to find any compatible game data.",
		                 startup_dir.IsEmpty() ? String() : String::FromFormat("Searched in: %s", startup_dir.GetCStr()));
	}
	data_path = Path::MakeAbsolutePath(data_path);
	Debug::Printf(kDbgMsg_Info, "Located game data pak: %s", data_path.GetCStr());
	return HError::None();
#else
	// No direct filepath provided, bail out.
	return new Error("The game location was not defined by startup settings.");
#endif
}

// Define location of the game data
bool define_gamedata_location() {
	String data_path, startup_dir;
	HError err = define_gamedata_location_checkall(data_path, startup_dir);
	if (!err) {
		_G(platform)->DisplayAlert("ERROR: Unable to determine game data.\n%s", err->FullMessage().GetCStr());
		main_print_help();
		return false;
	}

	// On success: set all the necessary path and filename settings
	_GP(usetup).startup_dir = startup_dir;
	_GP(usetup).main_data_file = data_path;
	_GP(usetup).main_data_dir = Path::GetDirectoryPath(data_path);
	return true;
}

// Find and preload main game data
bool engine_init_gamedata() {
	Debug::Printf(kDbgMsg_Info, "Initializing game data");
	// First, find data location
	if (!define_gamedata_location())
		return false;

	// Try init game lib
	AssetError asset_err = _GP(AssetMgr)->AddLibrary(_GP(usetup).main_data_file);
	if (asset_err != kAssetNoError) {
		_G(platform)->DisplayAlert("ERROR: The game data is missing, is of unsupported format or corrupt.\nFile: '%s'", _GP(usetup).main_data_file.GetCStr());
		return false;
	}

	// Pre-load game name and savegame folder names from data file
	// TODO: research if that is possible to avoid this step and just
	// read the full head game data at this point. This might require
	// further changes of the order of initialization.
	HError err = preload_game_data();
	if (!err) {
		display_game_file_error(err);
		return false;
	}

	// Setup _GP(ResPaths), so that we know out main locations further
	_GP(ResPaths).GamePak.Path = _GP(usetup).main_data_file;
	_GP(ResPaths).GamePak.Name = Path::GetFilename(_GP(usetup).main_data_file);
	_GP(ResPaths).DataDir = _GP(usetup).install_dir.IsEmpty() ? _GP(usetup).main_data_dir : Path::MakeAbsolutePath(_GP(usetup).install_dir);
	_GP(ResPaths).DataDir2 = Path::MakeAbsolutePath(_GP(usetup).opt_data_dir);
	_GP(ResPaths).AudioDir2 = Path::MakeAbsolutePath(_GP(usetup).opt_audio_dir);
	_GP(ResPaths).VoiceDir2 = Path::MakeAbsolutePath(_GP(usetup).opt_voice_dir);

	Debug::Printf(kDbgMsg_Info, "Startup directory: %s", _GP(usetup).startup_dir.GetCStr());
	Debug::Printf(kDbgMsg_Info, "Data directory: %s", _GP(ResPaths).DataDir.GetCStr());
	if (!_GP(ResPaths).DataDir2.IsEmpty())
		Debug::Printf(kDbgMsg_Info, "Opt data directory: %s", _GP(ResPaths).DataDir2.GetCStr());
	if (!_GP(ResPaths).AudioDir2.IsEmpty())
		Debug::Printf(kDbgMsg_Info, "Opt audio directory: %s", _GP(ResPaths).AudioDir2.GetCStr());
	if (!_GP(ResPaths).VoiceDir2.IsEmpty())
		Debug::Printf(kDbgMsg_Info, "Opt voice-over directory: %s", _GP(ResPaths).VoiceDir2.GetCStr());
	return true;
}

void engine_read_config(ConfigTree &cfg) {
	if (!_GP(usetup).conf_path.IsEmpty()) {
		IniUtil::Read(_GP(usetup).conf_path, cfg);
		return;
	}

	// Read default configuration file
	String def_cfg_file = find_default_cfg_file();
	IniUtil::Read(def_cfg_file, cfg);

	// Disabled on Windows because people were afraid that this config could be mistakenly
	// created by some installer and screw up their games. Until any kind of solution is found.
	String user_global_cfg_file;
	// Read user global configuration file
	user_global_cfg_file = find_user_global_cfg_file();
	if (Path::ComparePaths(user_global_cfg_file, def_cfg_file) != 0)
		IniUtil::Read(user_global_cfg_file, cfg);

	// Handle directive to search for the user config inside the custom directory;
		// this option may come either from command line or default/global config.
	if (_GP(usetup).user_conf_dir.IsEmpty())
		_GP(usetup).user_conf_dir = CfgReadString(cfg, "misc", "user_conf_dir");
	if (_GP(usetup).user_conf_dir.IsEmpty()) // also try deprecated option
		_GP(usetup).user_conf_dir = CfgReadBoolInt(cfg, "misc", "localuserconf") ? "." : "";
	// Test if the file is writeable, if it is then both engine and setup
	// applications may actually use it fully as a user config, otherwise
	// fallback to default behavior.
	if (!_GP(usetup).user_conf_dir.IsEmpty()) {
		resolve_configured_path(_GP(usetup).user_conf_dir);
		if (Path::IsRelativePath(_GP(usetup).user_conf_dir))
			_GP(usetup).user_conf_dir = Path::ConcatPaths(_GP(usetup).startup_dir, _GP(usetup).user_conf_dir);
		if (!Directory::CreateDirectory(_GP(usetup).user_conf_dir) ||
			!File::TestWriteFile(Path::ConcatPaths(_GP(usetup).user_conf_dir, DefaultConfigFileName))) {
			Debug::Printf(kDbgMsg_Warn, "Write test failed at user config dir '%s', using default path.",
				_GP(usetup).user_conf_dir.GetCStr());
			_GP(usetup).user_conf_dir = "";
		}
	}

	// Handle directive to search for the user config inside the game directory;
	// this option may come either from command line or default/global config.
	_GP(usetup).local_user_conf |= CfgReadInt(cfg, "misc", "localuserconf", 0) != 0;
	if (_GP(usetup).local_user_conf) { // Test if the file is writeable, if it is then both engine and setup
	  // applications may actually use it fully as a user config, otherwise
	  // fallback to default behavior.
		_GP(usetup).local_user_conf = File::TestWriteFile(def_cfg_file);
	}

	// Read user configuration file
	String user_cfg_file = find_user_cfg_file();
	if (Path::ComparePaths(user_cfg_file, def_cfg_file) != 0 &&
	        Path::ComparePaths(user_cfg_file, user_global_cfg_file) != 0)
		IniUtil::Read(user_cfg_file, cfg);

	// Apply overriding options from platform settings
	// TODO: normally, those should be instead stored in the same config file in a uniform way
	override_config_ext(cfg);
}

// Gathers settings from all available sources into single ConfigTree
void engine_prepare_config(ConfigTree &cfg, const ConfigTree &startup_opts) {
	Debug::Printf(kDbgMsg_Info, "Setting up game configuration");
	// Read configuration files
	engine_read_config(cfg);
	// Merge startup options in
	for (const auto &sectn : startup_opts)
		for (const auto &opt : sectn._value)
			cfg[sectn._key][opt._key] = opt._value;
}

// Applies configuration to the running game
void engine_set_config(const ConfigTree cfg) {
	config_defaults();
	apply_config(cfg);
	post_config();
}

static bool print_info_needs_game(const std::set<String> &keys) {
	return keys.count("all") > 0 || keys.count("config") > 0 || keys.count("configpath") > 0 ||
	       keys.count("data") > 0 || keys.count("filepath") > 0 || keys.count("gameproperties") > 0;
}

static void engine_print_info(const std::set<String> &keys, ConfigTree *user_cfg) {
	const bool all = keys.count("all") > 0;
	ConfigTree data;
	if (all || keys.count("engine") > 0) {
		data["engine"]["name"] = get_engine_name();
		data["engine"]["version"] = get_engine_version();
	}
	if (all || keys.count("graphicdriver") > 0) {
		StringV drv;
		AGS::Engine::GetGfxDriverFactoryNames(drv);
		for (size_t i = 0; i < drv.size(); ++i) {
			data["graphicdriver"][String::FromFormat("%zu", i)] = drv[i];
		}
	}
	if (all || keys.count("configpath") > 0) {
		String def_cfg_file = find_default_cfg_file();
		String gl_cfg_file = find_user_global_cfg_file();
		String user_cfg_file = find_user_cfg_file();
		data["configpath"]["default"] = def_cfg_file;
		data["configpath"]["global"] = gl_cfg_file;
		data["configpath"]["user"] = user_cfg_file;
	}
	if ((all || keys.count("config") > 0) && user_cfg) {
		for (const auto &sectn : *user_cfg) {
			String cfg_sectn = String::FromFormat("config@%s", sectn._key.GetCStr());
			for (const auto &opt : sectn._value)
				data[cfg_sectn][opt._key] = opt._value;
		}
	}
	if (all || keys.count("data") > 0) {
		data["data"]["gamename"] = _GP(game).gamename;
		data["data"]["version"] = StrUtil::IntToString(_G(loaded_game_file_version));
		data["data"]["compiledwith"] = _GP(game).compiled_with;
		data["data"]["basepack"] = _GP(ResPaths).GamePak.Path;
	}
	if (all || keys.count("gameproperties") > 0) {
		fill_game_properties(data["gameproperties"]);
	}
	if (all || keys.count("filepath") > 0) {
		data["filepath"]["exe"] = _G(appPath);
		data["filepath"]["cwd"] = Directory::GetCurrentDirectory();
		data["filepath"]["datadir"] = Path::MakePathNoSlash(_GP(ResPaths).DataDir);
		if (!_GP(ResPaths).DataDir2.IsEmpty()) {
			data["filepath"]["datadir2"] = Path::MakePathNoSlash(_GP(ResPaths).DataDir2);
			data["filepath"]["audiodir2"] = Path::MakePathNoSlash(_GP(ResPaths).AudioDir2);
			data["filepath"]["voicedir2"] = Path::MakePathNoSlash(_GP(ResPaths).VoiceDir2);
		}
		data["filepath"]["savegamedir"] = Path::MakePathNoSlash(GetGameUserDataDir().FullDir);
		data["filepath"]["appdatadir"] = Path::MakePathNoSlash(GetGameAppDataDir().FullDir);
	}
	String full;
	IniUtil::WriteToString(full, data);
	_G(platform)->WriteStdOut("%s", full.GetCStr());
}

// TODO: this function is still a big mess, engine/system-related initialization
// is mixed with game-related data adjustments. Divide it in parts, move game
// data init into either InitGameState() or other game method as appropriate.
int initialize_engine(const ConfigTree &startup_opts) {
	_G(proper_exit) = false;

	if (_G(engine_pre_init_callback)) {
		_G(engine_pre_init_callback)();
	}

	//-----------------------------------------------------
	// Install backend
	if (!engine_init_backend())
		return EXIT_ERROR;

	//-----------------------------------------------------
	// Locate game data and assemble game config
	if (_G(justTellInfo) && !print_info_needs_game(_G(tellInfoKeys))) {
		engine_print_info(_G(tellInfoKeys), nullptr);
		return EXIT_NORMAL;
	}

	if (!engine_init_gamedata())
		return EXIT_ERROR;
	ConfigTree cfg;
	engine_prepare_config(cfg, startup_opts);
	// Test if need to run built-in setup program (where available)
	if (!_G(justTellInfo) && _G(justRunSetup)) {
		int res;
		if (!engine_run_setup(cfg, res))
			return res;
	}
	// Set up game options from user config
	engine_set_config(cfg);
	if (_G(justTellInfo)) {
		engine_print_info(_G(tellInfoKeys), &cfg);
		return EXIT_NORMAL;
	}

	set_our_eip(-190);

	//-----------------------------------------------------
	// Init auxiliary data files and other directories, initialize asset manager
	engine_init_user_directories();

	set_our_eip(-191);

	engine_locate_speech_pak();

	set_our_eip(-192);

	engine_locate_audio_pak();

	set_our_eip(-193);

	engine_assign_assetpaths();

	//-----------------------------------------------------
	// Begin setting up systems

	set_our_eip(-194);

	engine_init_fonts();

	set_our_eip(-195);

	engine_init_keyboard();

	set_our_eip(-196);

	engine_init_mouse();

	set_our_eip(-198);

	engine_init_audio();

	set_our_eip(-199);

	engine_init_debug();

	set_our_eip(-10);

	engine_init_pathfinder();

	set_game_speed(40);

	set_our_eip(-20);
	set_our_eip(-19);

	int res = engine_load_game_data();
	if (res != 0)
		return res;

	set_our_eip(-189);

	res = engine_check_disk_space();
	if (res != 0)
		return res;

	// Make sure that at least one font was loaded in the process of loading
	// the game data.
	// TODO: Fold this check into engine_load_game_data()
	res = engine_check_font_was_loaded();
	if (res != 0)
		return res;

	set_our_eip(-179);

	engine_adjust_for_rotation_settings();

	// Attempt to initialize graphics mode
	if (!engine_try_set_gfxmode_any(_GP(usetup).Screen))
		return EXIT_ERROR;

	// Configure game window after renderer was initialized
	engine_setup_window();

	SetMultitasking(_GP(usetup).multitasking);

	sys_window_show_cursor(false); // hide the system cursor

	show_preload();

	res = engine_init_sprites();
	if (res != 0)
		return res;

	engine_init_game_settings();

	engine_prepare_to_start_game();

	initialize_start_and_play_game(_G(override_start_room), _G(loadSaveGameOnStartup));

	return EXIT_NORMAL;
}

bool engine_try_set_gfxmode_any(const DisplayModeSetup &setup) {
	const DisplayMode old_dm = _G(gfxDriver) ? _G(gfxDriver)->GetDisplayMode() : DisplayMode();

	engine_shutdown_gfxmode();

	sys_renderer_set_output(_GP(usetup).software_render_driver);

	const Size init_desktop = get_desktop_size();
	bool res = graphics_mode_init_any(GraphicResolution(_GP(game).GetGameRes(), _GP(game).color_depth * 8),
		setup, ColorDepthOption(_GP(game).GetColorDepth()));

	if (res)
		engine_post_gfxmode_setup(init_desktop, old_dm);
	// Make sure that we don't receive window events queued during init
	sys_flush_events();
	return res;
}

bool engine_try_switch_windowed_gfxmode() {
	if (!_G(gfxDriver) || !_G(gfxDriver)->IsModeSet())
		return false;

	// Keep previous mode in case we need to revert back
	DisplayMode old_dm = _G(gfxDriver)->GetDisplayMode();
	FrameScaleDef old_frame = graphics_mode_get_render_frame();

	// Release engine resources that depend on display mode
	engine_pre_gfxmode_release();

	Size init_desktop = get_desktop_size();
	bool windowed = !old_dm.IsWindowed();
	ActiveDisplaySetting setting = graphics_mode_get_last_setting(windowed);
	DisplayMode last_opposite_mode = setting.Dm;
	FrameScaleDef frame = setting.Frame;

	// Apply vsync in case it has been toggled at runtime
	last_opposite_mode.Vsync = _GP(usetup).Screen.Params.VSync;

	// If there are saved parameters for given mode (fullscreen/windowed)
	// then use them, if there are not, get default setup for the new mode.
	bool res;
	if (last_opposite_mode.IsValid()) {
		res = graphics_mode_set_dm(last_opposite_mode);
	} else {
		WindowSetup ws = windowed ? _GP(usetup).Screen.WinSetup : _GP(usetup).Screen.FsSetup;
		frame = windowed ? _GP(usetup).Screen.WinGameFrame : _GP(usetup).Screen.FsGameFrame;
		res = graphics_mode_set_dm_any(_GP(game).GetGameRes(), ws, old_dm.ColorDepth,
			frame, _GP(usetup).Screen.Params);
	}

	// Apply corresponding frame render method
	if (res)
		res = graphics_mode_set_render_frame(frame);

	if (!res) {
		// If failed, try switching back to previous gfx mode
		res = graphics_mode_set_dm(old_dm) &&
			graphics_mode_set_render_frame(old_frame);
		if (!res)
			quitprintf("Failed to restore graphics mode.");
	}

	// If succeeded (with any case), update engine objects that rely on
	// active display mode.
	if (!_G(gfxDriver)->GetDisplayMode().IsRealFullscreen())
		init_desktop = get_desktop_size();
	engine_post_gfxmode_setup(init_desktop, old_dm);
	// Make sure that we don't receive window events queued during init
	sys_flush_events();
	return res;
}

void engine_on_window_changed(const Size &sz) {
	graphics_mode_on_window_changed(sz);
	on_coordinates_scaling_changed();
	invalidate_screen();
}

void engine_shutdown_gfxmode() {
	if (!_G(gfxDriver))
		return;

	engine_pre_gfxsystem_shutdown();
	graphics_mode_shutdown();
}

const char *get_engine_name() {
	return "Adventure Game Studio run-time engine";
}

const char *get_engine_version() {
	return _G(EngineVersion).LongString.GetCStr();
}

String get_engine_version_and_build() {
	const char *bit = (AGS_PLATFORM_64BIT) ? "64-bit" : "32-bit";
	const char *end = (AGS_PLATFORM_ENDIAN_LITTLE) ? "LE" : "BE";
#ifdef BUILD_STR
	return String::FromFormat("%s (Build: %s), %s %s",
							  _G(EngineVersion).LongString.GetCStr(), EngineVersion.BuildInfo.GetCStr(),
							  bit, end);
#else
	return String::FromFormat("%s, %s %s",
							  _G(EngineVersion).LongString.GetCStr(),
							  bit, end);
#endif
}

void engine_set_pre_init_callback(t_engine_pre_init_callback callback) {
	_G(engine_pre_init_callback) = callback;
}

} // namespace AGS3
