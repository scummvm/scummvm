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

//
// Game configuration
//
//include <ctype.h> // toupper
#include "ags/shared/core/platform.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/main/main_header.h"
#include "ags/engine/main/config.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/ini_util.h"
#include "ags/shared/util/text_stream_reader.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_utils.h"
#include "ags/engine/media/audio/audio_system.h"
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

ScreenSizeDefinition parse_screendef(const String &option, ScreenSizeDefinition def_value) {
	const char *screen_sz_def_options[kNumScreenDef] = { "explicit", "scaling", "max" };
	for (int i = 0; i < kNumScreenDef; ++i) {
		if (option.CompareNoCase(screen_sz_def_options[i]) == 0) {
			return (ScreenSizeDefinition)i;
		}
	}
	return def_value;
}

void parse_scaling_option(const String &scaling_option, FrameScaleDefinition &scale_def, int &scale_factor) {
	const char *game_scale_options[kNumFrameScaleDef - 1] = { "max_round", "stretch", "proportional" };
	scale_def = kFrame_IntScale;
	for (int i = 0; i < kNumFrameScaleDef - 1; ++i) {
		if (scaling_option.CompareNoCase(game_scale_options[i]) == 0) {
			scale_def = (FrameScaleDefinition)(i + 1);
			break;
		}
	}

	if (scale_def == kFrame_IntScale)
		scale_factor = StrUtil::StringToInt(scaling_option);
	else
		scale_factor = 0;
}

void parse_scaling_option(const String &scaling_option, GameFrameSetup &frame_setup) {
	parse_scaling_option(scaling_option, frame_setup.ScaleDef, frame_setup.ScaleFactor);
}

// Parses legacy filter ID and converts it into current scaling options
bool parse_legacy_frame_config(const String &scaling_option, String &filter_id, GameFrameSetup &frame) {
	struct {
		String LegacyName;
		String CurrentName;
		int    Scaling;
	} legacy_filters[6] = { {"none", "none", -1}, {"max", "StdScale", 0}, {"StdScale", "StdScale", -1},
		{"AAx", "Linear", -1}, {"Hq2x", "Hqx", 2}, {"Hq3x", "Hqx", 3}
	};

	for (int i = 0; i < 6; i++) {
		if (scaling_option.CompareLeftNoCase(legacy_filters[i].LegacyName) == 0) {
			filter_id = legacy_filters[i].CurrentName;
			frame.ScaleDef = legacy_filters[i].Scaling == 0 ? kFrame_MaxRound : kFrame_IntScale;
			frame.ScaleFactor = legacy_filters[i].Scaling >= 0 ? legacy_filters[i].Scaling :
			                    scaling_option.Mid(legacy_filters[i].LegacyName.GetLength()).ToInt();
			return true;
		}
	}
	return false;
}

String make_scaling_option(FrameScaleDefinition scale_def, int scale_factor) {
	switch (scale_def) {
	case kFrame_MaxRound:
		return "max_round";
	case kFrame_MaxStretch:
		return "stretch";
	case kFrame_MaxProportional:
		return "proportional";
	default:
		break;
	}
	return String::FromFormat("%d", scale_factor);
}

String make_scaling_option(const GameFrameSetup &frame_setup) {
	return make_scaling_option(frame_setup.ScaleDef, frame_setup.ScaleFactor);
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

void graphics_mode_get_defaults(bool windowed, ScreenSizeSetup &scsz_setup, GameFrameSetup &frame_setup) {
	scsz_setup.Size = Size();
	if (windowed) {
		// For the windowed we define mode by the scaled _GP(game).
		scsz_setup.SizeDef = kScreenDef_ByGameScaling;
		scsz_setup.MatchDeviceRatio = false;
		frame_setup = _GP(usetup).Screen.WinGameFrame;
	} else {
		// For the fullscreen we set current desktop resolution, which
		// corresponds to most comfortable fullscreen mode for the driver.
		scsz_setup.SizeDef = kScreenDef_MaxDisplay;
		scsz_setup.MatchDeviceRatio = true;
		frame_setup = _GP(usetup).Screen.FsGameFrame;
	}
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
	_GP(usetup).audio_backend = 1;
	_GP(usetup).translation = "";
}

void read_legacy_graphics_config(const ConfigTree &cfg) {
	// Pre-3.* game resolution setup
	int default_res = INIreadint(cfg, "misc", "defaultres", 0);
	int screen_res = INIreadint(cfg, "misc", "screenres", 0);
	if ((default_res == kGameResolution_320x200 ||
	        default_res == kGameResolution_320x240) && screen_res > 0) {
		_GP(usetup).override_upscale = true; // run low-res game in high-res mode
	}

	_GP(usetup).Screen.DisplayMode.Windowed = true; // INIreadint(cfg, "misc", "windowed") > 0;
	_GP(usetup).Screen.DriverID = INIreadstring(cfg, "misc", "gfxdriver", _GP(usetup).Screen.DriverID);

	{
		String legacy_filter = INIreadstring(cfg, "misc", "gfxfilter");
		if (!legacy_filter.IsEmpty()) {
			// NOTE: legacy scaling config is applied only to windowed setting
			if (_GP(usetup).Screen.DisplayMode.Windowed)
				_GP(usetup).Screen.DisplayMode.ScreenSize.SizeDef = kScreenDef_ByGameScaling;
			parse_legacy_frame_config(legacy_filter, _GP(usetup).Screen.Filter.ID, _GP(usetup).Screen.WinGameFrame);

			// AGS 3.2.1 and 3.3.0 aspect ratio preferences
			if (!_GP(usetup).Screen.DisplayMode.Windowed) {
				_GP(usetup).Screen.DisplayMode.ScreenSize.MatchDeviceRatio =
				    (INIreadint(cfg, "misc", "sideborders") > 0 || INIreadint(cfg, "misc", "forceletterbox") > 0 ||
				     INIreadint(cfg, "misc", "prefer_sideborders") > 0 || INIreadint(cfg, "misc", "prefer_letterbox") > 0);
			}
		}

		// AGS 3.4.0 - 3.4.1-rc uniform scaling option
		String uniform_frame_scale = INIreadstring(cfg, "graphics", "game_scale");
		if (!uniform_frame_scale.IsEmpty()) {
			GameFrameSetup frame_setup;
			parse_scaling_option(uniform_frame_scale, frame_setup);
			_GP(usetup).Screen.FsGameFrame = frame_setup;
			_GP(usetup).Screen.WinGameFrame = frame_setup;
		}
	}

	_GP(usetup).Screen.DisplayMode.RefreshRate = INIreadint(cfg, "misc", "refresh");
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

#if AGS_PLATFORM_OS_ANDROID
	// config_mouse_control_mode - enable relative mouse mode
	//    * 1 - relative mouse touch controls
	//    * 0 - direct touch mouse control
	INIwriteint(cfg, "mouse", "control_enabled", config_mouse_control_mode);
#endif

	INIwriteint(cfg, "misc", "antialias", _G(psp_gfx_smooth_sprites) != 0);
	INIwritestring(cfg, "language", "translation", _G(psp_translation));
}

void apply_config(const ConfigTree &cfg) {
	{
		_GP(usetup).audio_backend = INIreadint(cfg, "sound", "enabled", _GP(usetup).audio_backend);

		// Legacy graphics settings has to be translated into new options;
		// they must be read first, to let newer options override them, if ones are present
		read_legacy_graphics_config(cfg);

		// Graphics mode
		_GP(usetup).Screen.DriverID = INIreadstring(cfg, "graphics", "driver", _GP(usetup).Screen.DriverID);

		_GP(usetup).Screen.DisplayMode.Windowed = true; // INIreadint(cfg, "graphics", "windowed") > 0;
		_GP(usetup).Screen.DisplayMode.ScreenSize.SizeDef = _GP(usetup).Screen.DisplayMode.Windowed ? kScreenDef_ByGameScaling : kScreenDef_MaxDisplay;
		_GP(usetup).Screen.DisplayMode.ScreenSize.SizeDef = parse_screendef(INIreadstring(cfg, "graphics", "screen_def"),
		        _GP(usetup).Screen.DisplayMode.ScreenSize.SizeDef);

		_GP(usetup).Screen.DisplayMode.ScreenSize.Size = Size(INIreadint(cfg, "graphics", "screen_width"),
		        INIreadint(cfg, "graphics", "screen_height"));
		_GP(usetup).Screen.DisplayMode.ScreenSize.MatchDeviceRatio = INIreadint(cfg, "graphics", "match_device_ratio", 1) != 0;
		// TODO: move to config overrides (replace values during config load)
#if AGS_PLATFORM_OS_MACOS
		_GP(usetup).Screen.Filter.ID = "none";
#else
		_GP(usetup).Screen.Filter.ID = INIreadstring(cfg, "graphics", "filter", "StdScale");
		parse_scaling_option(INIreadstring(cfg, "graphics", "game_scale_fs", "proportional"), _GP(usetup).Screen.FsGameFrame);
		parse_scaling_option(INIreadstring(cfg, "graphics", "game_scale_win", "max_round"), _GP(usetup).Screen.WinGameFrame);
#endif

		_GP(usetup).Screen.DisplayMode.RefreshRate = INIreadint(cfg, "graphics", "refresh");
		_GP(usetup).Screen.DisplayMode.VSync = INIreadint(cfg, "graphics", "vsync") > 0;
		_GP(usetup).RenderAtScreenRes = INIreadint(cfg, "graphics", "render_at_screenres") > 0;
		_GP(usetup).Supersampling = INIreadint(cfg, "graphics", "supersampling", 1);

		_GP(usetup).enable_antialiasing = INIreadint(cfg, "misc", "antialias") > 0;

		// This option is backwards (usevox is 0 if no_speech_pack)
		_GP(usetup).no_speech_pack = INIreadint(cfg, "sound", "usespeech", 1) == 0;

		_GP(usetup).user_data_dir = INIreadstring(cfg, "misc", "user_data_dir");
		_GP(usetup).shared_data_dir = INIreadstring(cfg, "misc", "shared_data_dir");

		Common::String translation;
		if (ConfMan.getActiveDomain()->tryGetVal("translation", translation) && !translation.empty())
			_GP(usetup).translation = translation;

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

	if (!_GP(usetup).Screen.FsGameFrame.IsValid())
		_GP(usetup).Screen.FsGameFrame = GameFrameSetup(kFrame_MaxProportional);
	if (!_GP(usetup).Screen.WinGameFrame.IsValid())
		_GP(usetup).Screen.WinGameFrame = GameFrameSetup(kFrame_MaxRound);

	_GP(usetup).user_data_dir = Path::MakePathNoSlash(_GP(usetup).user_data_dir);
	_GP(usetup).shared_data_dir = Path::MakePathNoSlash(_GP(usetup).shared_data_dir);
}

void save_config_file() {
	// ScummVM doesn't write out any configuration changes
}

} // namespace AGS3
