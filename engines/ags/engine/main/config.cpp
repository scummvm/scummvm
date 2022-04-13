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
// Game configuration
//
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/system.h"
#include "ags/shared/core/platform.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/engine/main/config.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/ini_util.h"
#include "ags/shared/util/text_stream_reader.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_utils.h"
#include "common/config-manager.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

// Filename of the default config file, the one found in the game installation
const char *DefaultConfigFileName = "acsetup.cfg";

bool INIreaditem(const ConfigTree &cfg, const String &sectn, const String &item, String &value) {
	ConfigNode sec_it = cfg.find(sectn);
	if (sec_it != cfg.end()) {
		StrStrOIter item_it = sec_it->_value.find(item);
		if (item_it != sec_it->_value.end()) {
			value = item_it->_value;
			return true;
		}
	}
	return false;
}

int INIreadint(const ConfigTree &cfg, const String &sectn, const String &item, int def_value) {
	String str;
	if (!INIreaditem(cfg, sectn, item, str))
		return def_value;

	return atoi(str.GetCStr());
}

float INIreadfloat(const ConfigTree &cfg, const String &sectn, const String &item, float def_value) {
	String str;
	if (!INIreaditem(cfg, sectn, item, str))
		return def_value;

	return atof(str.GetCStr());
}

String INIreadstring(const ConfigTree &cfg, const String &sectn, const String &item, const String &def_value) {
	String str;
	if (!INIreaditem(cfg, sectn, item, str))
		return def_value;
	return str;
}

void INIwriteint(ConfigTree &cfg, const String &sectn, const String &item, int value) {
	cfg[sectn][item] = StrUtil::IntToString(value);
}

void INIwritestring(ConfigTree &cfg, const String &sectn, const String &item, const String &value) {
	cfg[sectn][item] = value;
}

WindowSetup parse_window_mode(const String &option, bool as_windowed, WindowSetup def_value) {
	// "full_window" option means pseudo fullscreen ("borderless fullscreen window")
	if (!as_windowed && (option.CompareNoCase("full_window") == 0))
		return WindowSetup(kWnd_FullDesktop);
	// Check supported options for explicit resolution or scale factor,
	// in which case we'll use either a resizing window or a REAL fullscreen mode
	const WindowMode exp_wmode = as_windowed ? kWnd_Windowed : kWnd_Fullscreen;
	// Note that for "desktop" we return "default" for windowed, this will result
	// in refering to the  desktop size but resizing in accordance to the scaling style
	if (option.CompareNoCase("desktop") == 0)
		return as_windowed ? WindowSetup(exp_wmode) : WindowSetup(get_desktop_size(), exp_wmode);
	// "Native" means using game resolution as a window size
	if (option.CompareNoCase("native") == 0)
		return WindowSetup(_GP(game).GetGameRes(), exp_wmode);
	// Try parse an explicit resolution type or game scale factor --
	size_t at = option.FindChar('x');
	if (at == 0) { // try parse as a scale (xN)
		int scale = StrUtil::StringToInt(option.Mid(1));
		if (scale > 0) return WindowSetup(scale, exp_wmode);
	} else if (at != (size_t)(-1)) { // else try parse as a "width x height"
		Size sz = Size(StrUtil::StringToInt(option.Mid(0, at)),
			StrUtil::StringToInt(option.Mid(at + 1)));
		if (!sz.IsNull()) return WindowSetup(sz, exp_wmode);
	}
	// In case of "default" option, or any format mistake, return the default
	return def_value;
}

// Legacy screen size definition
enum ScreenSizeDefinition {
	kScreenDef_Undefined = -1,
	kScreenDef_Explicit,        // define by width & height
	kScreenDef_ByGameScaling,   // define by game scale factor
	kScreenDef_MaxDisplay,      // set to maximal supported (desktop/device screen size)
	kNumScreenDef
};

static ScreenSizeDefinition parse_legacy_screendef(const String &option) {
	const char *screen_sz_def_options[kNumScreenDef] = { "explicit", "scaling", "max" };
	for (int i = 0; i < kNumScreenDef; ++i) {
		if (option.CompareNoCase(screen_sz_def_options[i]) == 0) {
			return (ScreenSizeDefinition)i;
		}
	}
	return kScreenDef_Undefined;
}

FrameScaleDef parse_scaling_option(const String &option, FrameScaleDef def_value) {
	if (option.CompareNoCase("round") == 0 || option.CompareNoCase("max_round") == 0)
		return kFrame_Round;
	if (option.CompareNoCase("stretch") == 0)
		return kFrame_Stretch;
	if (option.CompareNoCase("proportional") == 0)
		return kFrame_Proportional;
	return def_value;
}

static FrameScaleDef parse_legacy_scaling_option(const String &option, int &scale) {
	FrameScaleDef frame = parse_scaling_option(option, kFrame_Undefined);
	if (frame == kFrame_Undefined) {
		scale = StrUtil::StringToInt(option);
		return scale > 0 ? kFrame_Round : kFrame_Undefined;
	}
	return frame;
}

// Parses legacy filter ID and converts it into current scaling options
bool parse_legacy_frame_config(const String &scaling_option, String &filter_id,
	FrameScaleDef &frame, int &scale_factor) {
	struct {
		String LegacyName;
		String CurrentName;
		int    Scaling;
	} legacy_filters[6] = { {"none", "none", -1}, {"max", "StdScale", 0}, {"StdScale", "StdScale", -1},
						   {"AAx", "Linear", -1}, {"Hq2x", "Hqx", 2}, {"Hq3x", "Hqx", 3} };

	for (int i = 0; i < 6; i++) {
		if (scaling_option.CompareLeftNoCase(legacy_filters[i].LegacyName) == 0) {
			filter_id = legacy_filters[i].CurrentName;
			frame = kFrame_Round;
			scale_factor = legacy_filters[i].Scaling >= 0 ? legacy_filters[i].Scaling :
				scaling_option.Mid(legacy_filters[i].LegacyName.GetLength()).ToInt();
			return true;
		}
	}
	return false;
}

String make_scaling_option(FrameScaleDef scale_def) {
	switch (scale_def) {
	case kFrame_Stretch:
		return "stretch";
	case kFrame_Proportional:
		return "proportional";
	default:
		return "round";
	}
}

uint32_t convert_scaling_to_fp(int scale_factor) {
	if (scale_factor >= 0)
		return scale_factor <<= kShift;
	else
		return kUnit / abs(scale_factor);
}

int convert_fp_to_scaling(uint32_t scaling) {
	if (scaling == 0)
		return 0;
	return scaling >= kUnit ? (scaling >> kShift) : -kUnit / (int32_t)scaling;
}

String find_default_cfg_file() {
	return Path::ConcatPaths(_GP(usetup).startup_dir, DefaultConfigFileName);
}

String find_user_global_cfg_file() {
	return Path::ConcatPaths(GetGlobalUserConfigDir().FullDir, DefaultConfigFileName);
}

String find_user_cfg_file() {
	return Path::ConcatPaths(GetGameUserConfigDir().FullDir, DefaultConfigFileName);
}

void config_defaults() {
#if AGS_PLATFORM_OS_WINDOWS
	_GP(usetup).Screen.DriverID = "D3D9";
#else
	_GP(usetup).Screen.DriverID = "OGL";
#endif
	// Defaults for the window style are max resizing window and "fullscreen desktop"
	_GP(usetup).Screen.FsSetup = WindowSetup(kWnd_FullDesktop);
	_GP(usetup).Screen.WinSetup = WindowSetup(kWnd_Windowed);
	_GP(usetup).audio_backend = 1;
	_GP(usetup).translation = "";
}

static void read_legacy_graphics_config(const ConfigTree &cfg) {
	// Pre-3.* game resolution setup
	int default_res = INIreadint(cfg, "misc", "defaultres", 0);
	int screen_res = INIreadint(cfg, "misc", "screenres", 0);
	if ((default_res == kGameResolution_320x200 ||
		default_res == kGameResolution_320x240) && screen_res > 0) {
		_GP(usetup).override_upscale = true; // run low-res game in high-res mode
	}

	_GP(usetup).Screen.Windowed = INIreadint(cfg, "misc", "windowed") > 0;
	_GP(usetup).Screen.DriverID = INIreadstring(cfg, "misc", "gfxdriver", _GP(usetup).Screen.DriverID);

	// Window setup: style and size definition, game frame style
	{
		String legacy_filter = INIreadstring(cfg, "misc", "gfxfilter");
		if (!legacy_filter.IsEmpty()) {
			// Legacy scaling config is applied only to windowed setting
			int scale_factor = 0;
			parse_legacy_frame_config(legacy_filter, _GP(usetup).Screen.Filter.ID, _GP(usetup).Screen.WinGameFrame,
				scale_factor);
			if (scale_factor > 0)
				_GP(usetup).Screen.WinSetup = WindowSetup(scale_factor);

			// AGS 3.2.1 and 3.3.0 aspect ratio preferences for fullscreen
			if (!_GP(usetup).Screen.Windowed) {
				bool allow_borders =
					(INIreadint(cfg, "misc", "sideborders") > 0 || INIreadint(cfg, "misc", "forceletterbox") > 0 ||
						INIreadint(cfg, "misc", "prefer_sideborders") > 0 || INIreadint(cfg, "misc", "prefer_letterbox") > 0);
				_GP(usetup).Screen.FsGameFrame = allow_borders ? kFrame_Proportional : kFrame_Stretch;
			}
		}

		// AGS 3.4.0 - 3.4.1-rc uniform scaling option
		String uniform_frame_scale = INIreadstring(cfg, "graphics", "game_scale");
		if (!uniform_frame_scale.IsEmpty()) {
			int src_scale = 1;
			FrameScaleDef frame = parse_legacy_scaling_option(uniform_frame_scale, src_scale);
			_GP(usetup).Screen.FsGameFrame = frame;
			_GP(usetup).Screen.WinGameFrame = frame;
		}

		// AGS 3.5.* gfx mode with screen definition
		const bool is_windowed = INIreadint(cfg, "graphics", "windowed") != 0;
		WindowSetup &ws = is_windowed ? _GP(usetup).Screen.WinSetup : _GP(usetup).Screen.FsSetup;
		const WindowMode wm = is_windowed ? kWnd_Windowed : kWnd_Fullscreen;
		ScreenSizeDefinition scr_def = parse_legacy_screendef(INIreadstring(cfg, "graphics", "screen_def"));
		switch (scr_def) {
		case kScreenDef_Explicit:
		{
			Size sz(
				INIreadint(cfg, "graphics", "screen_width"),
				INIreadint(cfg, "graphics", "screen_height"));
			ws = WindowSetup(sz, wm);
		}
		break;
		case kScreenDef_ByGameScaling:
		{
			int src_scale;
			is_windowed ?
				parse_legacy_scaling_option(INIreadstring(cfg, "graphics", "game_scale_win"), src_scale) :
				parse_legacy_scaling_option(INIreadstring(cfg, "graphics", "game_scale_fs"), src_scale);
			ws = WindowSetup(src_scale, wm);
		}
		break;
		case kScreenDef_MaxDisplay:
			ws = is_windowed ? WindowSetup() : WindowSetup(kWnd_FullDesktop);
			break;
		default:
			break;
		}
	}

	_GP(usetup).Screen.Params.RefreshRate = INIreadint(cfg, "misc", "refresh");
}


void override_config_ext(ConfigTree &cfg) {
	// Mobile ports always run in fullscreen mode
#if AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS
	INIwriteint(cfg, "graphics", "windowed", 0);
#endif

	// psp_gfx_renderer - rendering mode
	//    * 0 - software renderer
	//    * 1 - hardware, render to screen
	//    * 2 - hardware, render to texture
	if (_G(psp_gfx_renderer) == 0) {
		INIwritestring(cfg, "graphics", "driver", "Software");
		INIwriteint(cfg, "graphics", "render_at_screenres", 1);
	} else {
		INIwritestring(cfg, "graphics", "driver", "OGL");
		INIwriteint(cfg, "graphics", "render_at_screenres", _G(psp_gfx_renderer) == 1);
	}

	// psp_gfx_scaling - scaling style:
	//    * 0 - no scaling
	//    * 1 - stretch and preserve aspect ratio
	//    * 2 - stretch to whole screen
	if (_G(psp_gfx_scaling) == 0)
		INIwritestring(cfg, "graphics", "game_scale_fs", "1");
	else if (_G(psp_gfx_scaling) == 1)
		INIwritestring(cfg, "graphics", "game_scale_fs", "proportional");
	else
		INIwritestring(cfg, "graphics", "game_scale_fs", "stretch");

	// psp_gfx_smoothing - scaling filter:
	//    * 0 - nearest-neighbour
	//    * 1 - linear
	if (_G(psp_gfx_smoothing) == 0)
		INIwritestring(cfg, "graphics", "filter", "StdScale");
	else
		INIwritestring(cfg, "graphics", "filter", "Linear");

	// psp_gfx_super_sampling - enable super sampling
	//    * 0 - x1
	//    * 1 - x2
	if (_G(psp_gfx_renderer) == 2)
		INIwriteint(cfg, "graphics", "supersampling", _G(psp_gfx_super_sampling) + 1);
	else
		INIwriteint(cfg, "graphics", "supersampling", 0);

	// psp_gfx_rotation - scaling style:
	//    * 0 - unlocked, let the user rotate as wished.
	//    * 1 - portrait
	//    * 2 - landscape
	INIwriteint(cfg, "graphics", "rotation", _G(psp_rotation));

#if AGS_PLATFORM_OS_ANDROID
	// config_mouse_control_mode - enable relative mouse mode
	//    * 1 - relative mouse touch controls
	//    * 0 - direct touch mouse control
	INIwriteint(cfg, "mouse", "control_enabled", config_mouse_control_mode);
#endif

	INIwriteint(cfg, "misc", "antialias", _G(psp_gfx_smooth_sprites) != 0);
	INIwritestring(cfg, "language", "translation", _G(psp_translation));
	INIwriteint(cfg, "misc", "clear_cache_on_room_change", _G(psp_clear_cache_on_room_change) != 0);
}

void apply_config(const ConfigTree &cfg) {
	{
		_GP(usetup).audio_backend = INIreadint(cfg, "sound", "enabled", _GP(usetup).audio_backend);

		// Legacy graphics settings has to be translated into new options;
		// they must be read first, to let newer options override them, if ones are present
		read_legacy_graphics_config(cfg);

		// Graphics mode
		_GP(usetup).Screen.DriverID = INIreadstring(cfg, "graphics", "driver", _GP(usetup).Screen.DriverID);
		_GP(usetup).Screen.Windowed = INIreadint(cfg, "graphics", "windowed", _GP(usetup).Screen.Windowed ? 1 : 0) > 0;
		_GP(usetup).Screen.FsSetup =
			parse_window_mode(INIreadstring(cfg, "graphics", "fullscreen", "default"), false, _GP(usetup).Screen.FsSetup);
		_GP(usetup).Screen.WinSetup =
			parse_window_mode(INIreadstring(cfg, "graphics", "window", "default"), true, _GP(usetup).Screen.WinSetup);

		// TODO: move to config overrides (replace values during config load)
#if AGS_PLATFORM_OS_MACOS
		_GP(usetup).Screen.Filter.ID = "none";
#else
		_GP(usetup).Screen.Filter.ID = INIreadstring(cfg, "graphics", "filter", "StdScale");
		_GP(usetup).Screen.FsGameFrame =
			parse_scaling_option(INIreadstring(cfg, "graphics", "game_scale_fs", "proportional"), _GP(usetup).Screen.FsGameFrame);
		_GP(usetup).Screen.WinGameFrame =
			parse_scaling_option(INIreadstring(cfg, "graphics", "game_scale_win", "round"), _GP(usetup).Screen.WinGameFrame);
#endif

		_GP(usetup).Screen.Params.RefreshRate = INIreadint(cfg, "graphics", "refresh");
		_GP(usetup).Screen.Params.VSync = INIreadint(cfg, "graphics", "vsync") > 0;
		_GP(usetup).RenderAtScreenRes = INIreadint(cfg, "graphics", "render_at_screenres") > 0;
		_GP(usetup).Supersampling = INIreadint(cfg, "graphics", "supersampling", 1);
#ifdef TODO
		_GP(usetup).rotation = (ScreenRotation)INIreadint(cfg, "graphics", "rotation", _GP(usetup).rotation);
		String rotation_str = INIreadstring(cfg, "graphics", "rotation", "unlocked");
		_GP(usetup).rotation = StrUtil::ParseEnum<ScreenRotation>(
			rotation_str, CstrArr<kNumScreenRotationOptions>{ "unlocked", "portrait", "landscape" },
			_GP(usetup).rotation);
#endif
		_GP(usetup).enable_antialiasing = INIreadint(cfg, "misc", "antialias") > 0;

		// This option is backwards (usevox is 0 if no_speech_pack)
		_GP(usetup).no_speech_pack = INIreadint(cfg, "sound", "usespeech", 1) == 0;

		_GP(usetup).clear_cache_on_room_change = INIreadint(cfg, "misc", "clear_cache_on_room_change", _GP(usetup).clear_cache_on_room_change);
		_GP(usetup).user_data_dir = INIreadstring(cfg, "misc", "user_data_dir");
		_GP(usetup).shared_data_dir = INIreadstring(cfg, "misc", "shared_data_dir");

		_GP(usetup).translation = INIreadstring(cfg, "language", "translation");

		int cache_size_kb = INIreadint(cfg, "misc", "cachemax", DEFAULTCACHESIZE_KB);
		if (cache_size_kb > 0)
			_GP(spriteset).SetMaxCacheSize((size_t)cache_size_kb * 1024);

		_GP(usetup).mouse_auto_lock = INIreadint(cfg, "mouse", "auto_lock") > 0;

		_GP(usetup).mouse_speed = INIreadfloat(cfg, "mouse", "speed", 1.f);
		if (_GP(usetup).mouse_speed <= 0.f)
			_GP(usetup).mouse_speed = 1.f;
		const char *mouse_ctrl_options[kNumMouseCtrlOptions] = { "never", "fullscreen", "always" };
		String mouse_str = INIreadstring(cfg, "mouse", "control_when", "fullscreen");
		for (int i = 0; i < kNumMouseCtrlOptions; ++i) {
			if (mouse_str.CompareNoCase(mouse_ctrl_options[i]) == 0) {
				_GP(usetup).mouse_ctrl_when = (MouseControlWhen)i;
				break;
			}
		}
		_GP(usetup).mouse_ctrl_enabled = INIreadint(cfg, "mouse", "control_enabled", _GP(usetup).mouse_ctrl_enabled) > 0;
		const char *mouse_speed_options[kNumMouseSpeedDefs] = { "absolute", "current_display" };
		mouse_str = INIreadstring(cfg, "mouse", "speed_def", "current_display");
		for (int i = 0; i < kNumMouseSpeedDefs; ++i) {
			if (mouse_str.CompareNoCase(mouse_speed_options[i]) == 0) {
				_GP(usetup).mouse_speed_def = (MouseSpeedDef)i;
				break;
			}
		}

		_GP(usetup).override_multitasking = INIreadint(cfg, "override", "multitasking", -1);
		String override_os = INIreadstring(cfg, "override", "os");
		_GP(usetup).override_script_os = -1;
		if (override_os.CompareNoCase("dos") == 0) {
			_GP(usetup).override_script_os = eOS_DOS;
		} else if (override_os.CompareNoCase("win") == 0) {
			_GP(usetup).override_script_os = eOS_Win;
		} else if (override_os.CompareNoCase("linux") == 0) {
			_GP(usetup).override_script_os = eOS_Linux;
		} else if (override_os.CompareNoCase("mac") == 0) {
			_GP(usetup).override_script_os = eOS_Mac;
		}
		_GP(usetup).override_upscale = INIreadint(cfg, "override", "upscale", _GP(usetup).override_upscale) > 0;
	}

	// Apply logging configuration
	apply_debug_config(cfg);
}

void post_config() {
	if (_GP(usetup).Screen.DriverID.IsEmpty() || _GP(usetup).Screen.DriverID.CompareNoCase("DX5") == 0)
		_GP(usetup).Screen.DriverID = "Software";

	// FIXME: this correction is needed at the moment because graphics driver
	// implementation requires some filter to be created anyway
	_GP(usetup).Screen.Filter.UserRequest = _GP(usetup).Screen.Filter.ID;
	if (_GP(usetup).Screen.Filter.ID.IsEmpty() || _GP(usetup).Screen.Filter.ID.CompareNoCase("none") == 0) {
		_GP(usetup).Screen.Filter.ID = "StdScale";
	}
}

void save_config_file() {
	// ScummVM doesn't write out any configuration changes
}

} // namespace AGS3
