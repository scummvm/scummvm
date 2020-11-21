//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

//
// Game configuration
//
#include <ctype.h> // toupper

#include "core/platform.h"
#include "ac/gamesetup.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_translation.h"
#include "ac/path_helper.h"
#include "ac/spritecache.h"
#include "ac/system.h"
#include "debug/debugger.h"
#include "debug/debug_log.h"
#include "main/mainheader.h"
#include "main/config.h"
#include "platform/base/agsplatformdriver.h"
#include "util/directory.h"
#include "util/ini_util.h"
#include "util/textstreamreader.h"
#include "util/path.h"
#include "util/string_utils.h"
#include "media/audio/audio_system.h"


using namespace AGS::Common;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern GameSetup usetup;
extern SpriteCache spriteset;
extern int force_window;
extern GameState play;

// Filename of the default config file, the one found in the game installation
const String DefaultConfigFileName = "acsetup.cfg";

// Replace the filename part of complete path WASGV with INIFIL
// TODO: get rid of this and use proper lib path function instead
void INIgetdirec(char *wasgv, const char *inifil) {
    int u = strlen(wasgv) - 1;

    for (u = strlen(wasgv) - 1; u >= 0; u--) {
        if ((wasgv[u] == '\\') || (wasgv[u] == '/')) {
            memcpy(&wasgv[u + 1], inifil, strlen(inifil) + 1);
            break;
        }
    }

    if (u <= 0) {
        // no slashes - either the path is just "f:acwin.exe"
        if (strchr(wasgv, ':') != nullptr)
            memcpy(strchr(wasgv, ':') + 1, inifil, strlen(inifil) + 1);
        // or it's just "acwin.exe" (unlikely)
        else
            strcpy(wasgv, inifil);
    }

}

bool INIreaditem(const ConfigTree &cfg, const String &sectn, const String &item, String &value)
{
    ConfigNode sec_it = cfg.find(sectn);
    if (sec_it != cfg.end())
    {
        StrStrOIter item_it = sec_it->second.find(item);
        if (item_it != sec_it->second.end())
        {
            value = item_it->second;
            return true;
        }
    }
    return false;
}

int INIreadint(const ConfigTree &cfg, const String &sectn, const String &item, int def_value)
{
    String str;
    if (!INIreaditem(cfg, sectn, item, str))
        return def_value;

    return atoi(str);
}

float INIreadfloat(const ConfigTree &cfg, const String &sectn, const String &item, float def_value)
{
    String str;
    if (!INIreaditem(cfg, sectn, item, str))
        return def_value;

    return atof(str);
}

String INIreadstring(const ConfigTree &cfg, const String &sectn, const String &item, const String &def_value)
{
    String str;
    if (!INIreaditem(cfg, sectn, item, str))
        return def_value;
    return str;
}

void INIwriteint(ConfigTree &cfg, const String &sectn, const String &item, int value)
{
    cfg[sectn][item] = StrUtil::IntToString(value);
}

void INIwritestring(ConfigTree &cfg, const String &sectn, const String &item, const String &value)
{
    cfg[sectn][item] = value;
}

void parse_scaling_option(const String &scaling_option, FrameScaleDefinition &scale_def, int &scale_factor)
{
    const char *game_scale_options[kNumFrameScaleDef - 1] = { "max_round", "stretch", "proportional" };
    scale_def = kFrame_IntScale;
    for (int i = 0; i < kNumFrameScaleDef - 1; ++i)
    {
        if (scaling_option.CompareNoCase(game_scale_options[i]) == 0)
        {
            scale_def = (FrameScaleDefinition)(i + 1);
            break;
        }
    }

    if (scale_def == kFrame_IntScale)
        scale_factor = StrUtil::StringToInt(scaling_option);
    else
        scale_factor = 0;
}

void parse_scaling_option(const String &scaling_option, GameFrameSetup &frame_setup)
{
    parse_scaling_option(scaling_option, frame_setup.ScaleDef, frame_setup.ScaleFactor);
}

// Parses legacy filter ID and converts it into current scaling options
bool parse_legacy_frame_config(const String &scaling_option, String &filter_id, GameFrameSetup &frame)
{
    struct
    {
        String LegacyName;
        String CurrentName;
        int    Scaling;
    } legacy_filters[6] = { {"none", "none", -1}, {"max", "StdScale", 0}, {"StdScale", "StdScale", -1},
                           {"AAx", "Linear", -1}, {"Hq2x", "Hqx", 2}, {"Hq3x", "Hqx", 3} };

    for (int i = 0; i < 6; i++)
    {
        if (scaling_option.CompareLeftNoCase(legacy_filters[i].LegacyName) == 0)
        {
            filter_id = legacy_filters[i].CurrentName;
            frame.ScaleDef = legacy_filters[i].Scaling == 0 ? kFrame_MaxRound : kFrame_IntScale;
            frame.ScaleFactor = legacy_filters[i].Scaling >= 0 ? legacy_filters[i].Scaling :
                scaling_option.Mid(legacy_filters[i].LegacyName.GetLength()).ToInt();
            return true;
        }
    }
    return false;
}

String make_scaling_option(FrameScaleDefinition scale_def, int scale_factor)
{
    switch (scale_def)
    {
    case kFrame_MaxRound:
        return "max_round";
    case kFrame_MaxStretch:
        return "stretch";
    case kFrame_MaxProportional:
        return "proportional";
    }
    return String::FromFormat("%d", scale_factor);
}

String make_scaling_option(const GameFrameSetup &frame_setup)
{
    return make_scaling_option(frame_setup.ScaleDef, frame_setup.ScaleFactor);
}

uint32_t convert_scaling_to_fp(int scale_factor)
{
    if (scale_factor >= 0)
        return scale_factor <<= kShift;
    else
        return kUnit / abs(scale_factor);
}

int convert_fp_to_scaling(uint32_t scaling)
{
    if (scaling == 0)
        return 0;
    return scaling >= kUnit ? (scaling >> kShift) : -kUnit / (int32_t)scaling;
}

AlIDStr AlIDToChars(int al_id)
{
    if (al_id == 0)
        return AlIDStr {{ 'N', 'O', 'N', 'E', '\0' }};
    else if (al_id == -1)
        return AlIDStr {{ 'A', 'U', 'T', 'O', '\0' }};
    else
        return AlIDStr {{
            static_cast<char>((al_id >> 24) & 0xFF),
            static_cast<char>((al_id >> 16) & 0xFF),
            static_cast<char>((al_id >> 8) & 0xFF),
            static_cast<char>((al_id) & 0xFF),
            '\0'
        }};
}

AlIDStr AlIDToChars(const String &s)
{
    AlIDStr id_str;
    size_t i = 0;
    for (; i < s.GetLength(); ++i)
        id_str.s[i] = toupper(s[i]);
    for (; i < 4; ++i)
        id_str.s[i] = ' ';
    id_str.s[4] = 0;
    return id_str;
}

int StringToAlID(const char *cstr)
{
    return (int)(AL_ID(cstr[0u], cstr[1u], cstr[2u], cstr[3u]));
}

// Parses a config string which may hold plain driver's ID or 4-char ID packed
// as a 32-bit integer.
int parse_driverid(const String &id)
{
    int asint;
    if (StrUtil::StringToInt(id, asint, 0) == StrUtil::kNoError)
        return asint;
    if (id.GetLength() > 4)
        return -1; // autodetect
    if (id.CompareNoCase("AUTO") == 0)
        return -1; // autodetect
    if (id.CompareNoCase("NONE") == 0)
        return 0; // no driver
    return StringToAlID(AlIDToChars(id).s);
}

// Reads driver ID from config, where it may be represented as string or number
int read_driverid(const ConfigTree &cfg, const String &sectn, const String &item, int def_value)
{
    String s = INIreadstring(cfg, sectn, item);
    if (s.IsEmpty())
        return def_value;
    return parse_driverid(s);
}

void write_driverid(ConfigTree &cfg, const String &sectn, const String &item, int value)
{
    INIwritestring(cfg, sectn, item, AlIDToChars(value).s);
}

void graphics_mode_get_defaults(bool windowed, ScreenSizeSetup &scsz_setup, GameFrameSetup &frame_setup)
{
    scsz_setup.Size = Size();
    if (windowed)
    {
        // For the windowed we define mode by the scaled game.
        scsz_setup.SizeDef = kScreenDef_ByGameScaling;
        scsz_setup.MatchDeviceRatio = false;
        frame_setup = usetup.Screen.WinGameFrame;
    }
    else
    {
        // For the fullscreen we set current desktop resolution, which
        // corresponds to most comfortable fullscreen mode for the driver.
        scsz_setup.SizeDef = kScreenDef_MaxDisplay;
        scsz_setup.MatchDeviceRatio = true;
        frame_setup = usetup.Screen.FsGameFrame;
    }
}

String find_default_cfg_file(const char *alt_cfg_file)
{
    // Try current directory for config first; else try exe dir
    String filename = String::FromFormat("%s/%s", Directory::GetCurrentDirectory().GetCStr(), DefaultConfigFileName.GetCStr());
    if (!Common::File::TestReadFile(filename))
    {
        char conffilebuf[512];
        strcpy(conffilebuf, alt_cfg_file);
        fix_filename_case(conffilebuf);
        fix_filename_slashes(conffilebuf);
        INIgetdirec(conffilebuf, DefaultConfigFileName);
        filename = conffilebuf;
    }
    return filename;
}

String find_user_global_cfg_file()
{
    String parent_dir = PathOrCurDir(platform->GetUserGlobalConfigDirectory());
    return String::FromFormat("%s/%s", parent_dir.GetCStr(), DefaultConfigFileName.GetCStr());
}

String find_user_cfg_file()
{
    String parent_dir = MakeSpecialSubDir(PathOrCurDir(platform->GetUserConfigDirectory()));
    return String::FromFormat("%s/%s", parent_dir.GetCStr(), DefaultConfigFileName.GetCStr());
}

void config_defaults()
{
#if AGS_PLATFORM_OS_WINDOWS
    usetup.Screen.DriverID = "D3D9";
#else
    usetup.Screen.DriverID = "OGL";
#endif
#if AGS_PLATFORM_OS_WINDOWS
    usetup.digicard = DIGI_DIRECTAMX(0);
#endif
    usetup.midicard = MIDI_AUTODETECT;
    usetup.translation = "";
}

void read_game_data_location(const ConfigTree &cfg)
{
    usetup.data_files_dir = INIreadstring(cfg, "misc", "datadir", usetup.data_files_dir);
    if (!usetup.data_files_dir.IsEmpty())
    {
        // strip any trailing slash
        // TODO: move this to Path namespace later
        AGS::Common::Path::FixupPath(usetup.data_files_dir);
#if AGS_PLATFORM_OS_WINDOWS
        // if the path is just x:\ don't strip the slash
        if (!(usetup.data_files_dir.GetLength() == 3 && usetup.data_files_dir[1u] == ':'))
        {
            usetup.data_files_dir.TrimRight('/');
        }
#else
        usetup.data_files_dir.TrimRight('/');
#endif
    }
    usetup.main_data_filename = INIreadstring(cfg, "misc", "datafile", usetup.main_data_filename);
}

void read_legacy_audio_config(const ConfigTree &cfg)
{
#if AGS_PLATFORM_OS_WINDOWS
    int idx = INIreadint(cfg, "sound", "digiwinindx", -1);
    if (idx == 0)
        idx = DIGI_DIRECTAMX(0);
    else if (idx == 1)
        idx = DIGI_WAVOUTID(0);
    else if (idx == 2)
        idx = DIGI_NONE;
    else if (idx == 3)
        idx = DIGI_DIRECTX(0);
    else
        idx = DIGI_AUTODETECT;
    usetup.digicard = idx;

    idx = INIreadint(cfg, "sound", "midiwinindx", -1);
    if (idx == 1)
        idx = MIDI_NONE;
    else if (idx == 2)
        idx = MIDI_WIN32MAPPER;
    else
        idx = MIDI_AUTODETECT;
    usetup.midicard = idx;
#endif
}

void read_legacy_graphics_config(const ConfigTree &cfg)
{
    usetup.Screen.DisplayMode.Windowed = INIreadint(cfg, "misc", "windowed") > 0;
    usetup.Screen.DriverID = INIreadstring(cfg, "misc", "gfxdriver", usetup.Screen.DriverID);

    {
        String legacy_filter = INIreadstring(cfg, "misc", "gfxfilter");
        if (!legacy_filter.IsEmpty())
        {
            // NOTE: legacy scaling config is applied only to windowed setting
            if (usetup.Screen.DisplayMode.Windowed)
                usetup.Screen.DisplayMode.ScreenSize.SizeDef = kScreenDef_ByGameScaling;
            parse_legacy_frame_config(legacy_filter, usetup.Screen.Filter.ID, usetup.Screen.WinGameFrame);

            // AGS 3.2.1 and 3.3.0 aspect ratio preferences
            if (!usetup.Screen.DisplayMode.Windowed)
            {
                usetup.Screen.DisplayMode.ScreenSize.MatchDeviceRatio =
                    (INIreadint(cfg, "misc", "sideborders") > 0 || INIreadint(cfg, "misc", "forceletterbox") > 0 ||
                     INIreadint(cfg, "misc", "prefer_sideborders") > 0 || INIreadint(cfg, "misc", "prefer_letterbox") > 0);
            }
        }

        // AGS 3.4.0 - 3.4.1-rc uniform scaling option
        String uniform_frame_scale = INIreadstring(cfg, "graphics", "game_scale");
        if (!uniform_frame_scale.IsEmpty())
        {
            GameFrameSetup frame_setup;
            parse_scaling_option(uniform_frame_scale, frame_setup);
            usetup.Screen.FsGameFrame = frame_setup;
            usetup.Screen.WinGameFrame = frame_setup;
        }
    }

    usetup.Screen.DisplayMode.RefreshRate = INIreadint(cfg, "misc", "refresh");
}

// Variables used for mobile port configs
extern int psp_gfx_renderer;
extern int psp_gfx_scaling;
extern int psp_gfx_super_sampling;
extern int psp_gfx_smoothing;
extern int psp_gfx_smooth_sprites;
extern int psp_audio_enabled;
extern int psp_midi_enabled;
extern char psp_translation[];

void override_config_ext(ConfigTree &cfg)
{
    // Mobile ports always run in fullscreen mode
#if AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS
    INIwriteint(cfg, "graphics", "windowed", 0);
#endif

    // psp_gfx_renderer - rendering mode
    //    * 0 - software renderer
    //    * 1 - hardware, render to screen
    //    * 2 - hardware, render to texture
    if (psp_gfx_renderer == 0)
    {
        INIwritestring(cfg, "graphics", "driver", "Software");
        INIwriteint(cfg, "graphics", "render_at_screenres", 1);
    }
    else
    {
        INIwritestring(cfg, "graphics", "driver", "OGL");
        INIwriteint(cfg, "graphics", "render_at_screenres", psp_gfx_renderer == 1);
    }

    // psp_gfx_scaling - scaling style:
    //    * 0 - no scaling
    //    * 1 - stretch and preserve aspect ratio
    //    * 2 - stretch to whole screen
    if (psp_gfx_scaling == 0)
        INIwritestring(cfg, "graphics", "game_scale_fs", "1");
    else if (psp_gfx_scaling == 1)
        INIwritestring(cfg, "graphics", "game_scale_fs", "proportional");
    else
        INIwritestring(cfg, "graphics", "game_scale_fs", "stretch");

    // psp_gfx_smoothing - scaling filter:
    //    * 0 - nearest-neighbour
    //    * 1 - linear
    if (psp_gfx_smoothing == 0)
        INIwritestring(cfg, "graphics", "filter", "StdScale");
    else
        INIwritestring(cfg, "graphics", "filter", "Linear");

    // psp_gfx_super_sampling - enable super sampling
    //    * 0 - x1
    //    * 1 - x2
    if (psp_gfx_renderer == 2)
        INIwriteint(cfg, "graphics", "supersampling", psp_gfx_super_sampling + 1);
    else
        INIwriteint(cfg, "graphics", "supersampling", 0);

    INIwriteint(cfg, "misc", "antialias", psp_gfx_smooth_sprites != 0);
    INIwritestring(cfg, "language", "translation", psp_translation);
}

void apply_config(const ConfigTree &cfg)
{
    {
        // Legacy settings has to be translated into new options;
        // they must be read first, to let newer options override them, if ones are present
        read_legacy_audio_config(cfg);
        if (psp_audio_enabled)
        {
            usetup.digicard = read_driverid(cfg, "sound", "digiid", usetup.digicard);
            if (psp_midi_enabled)
                usetup.midicard = read_driverid(cfg, "sound", "midiid", usetup.midicard);
            else
                usetup.midicard = MIDI_NONE;
        }
        else
        {
            usetup.digicard = DIGI_NONE;
            usetup.midicard = MIDI_NONE;
        }

        psp_audio_multithreaded = INIreadint(cfg, "sound", "threaded", psp_audio_multithreaded);

        // Legacy graphics settings has to be translated into new options;
        // they must be read first, to let newer options override them, if ones are present
        read_legacy_graphics_config(cfg);

        // Graphics mode
        usetup.Screen.DriverID = INIreadstring(cfg, "graphics", "driver", usetup.Screen.DriverID);

        usetup.Screen.DisplayMode.Windowed = INIreadint(cfg, "graphics", "windowed") > 0;
        const char *screen_sz_def_options[kNumScreenDef] = { "explicit", "scaling", "max" };
        usetup.Screen.DisplayMode.ScreenSize.SizeDef = usetup.Screen.DisplayMode.Windowed ? kScreenDef_ByGameScaling : kScreenDef_MaxDisplay;
        String screen_sz_def_str = INIreadstring(cfg, "graphics", "screen_def");
        for (int i = 0; i < kNumScreenDef; ++i)
        {
            if (screen_sz_def_str.CompareNoCase(screen_sz_def_options[i]) == 0)
            {
                usetup.Screen.DisplayMode.ScreenSize.SizeDef = (ScreenSizeDefinition)i;
                break;
            }
        }

        usetup.Screen.DisplayMode.ScreenSize.Size = Size(INIreadint(cfg, "graphics", "screen_width"),
                                                        INIreadint(cfg, "graphics", "screen_height"));
        usetup.Screen.DisplayMode.ScreenSize.MatchDeviceRatio = INIreadint(cfg, "graphics", "match_device_ratio", 1) != 0;
        // TODO: move to config overrides (replace values during config load)
#if AGS_PLATFORM_OS_MACOS
        usetup.Screen.Filter.ID = "none";
#else
        usetup.Screen.Filter.ID = INIreadstring(cfg, "graphics", "filter", "StdScale");
        parse_scaling_option(INIreadstring(cfg, "graphics", "game_scale_fs", "proportional"), usetup.Screen.FsGameFrame);
        parse_scaling_option(INIreadstring(cfg, "graphics", "game_scale_win", "max_round"), usetup.Screen.WinGameFrame);
#endif

        usetup.Screen.DisplayMode.RefreshRate = INIreadint(cfg, "graphics", "refresh");
        usetup.Screen.DisplayMode.VSync = INIreadint(cfg, "graphics", "vsync") > 0;
        usetup.RenderAtScreenRes = INIreadint(cfg, "graphics", "render_at_screenres") > 0;
        usetup.Supersampling = INIreadint(cfg, "graphics", "supersampling", 1);

        usetup.enable_antialiasing = INIreadint(cfg, "misc", "antialias") > 0;

        // This option is backwards (usevox is 0 if no_speech_pack)
        usetup.no_speech_pack = INIreadint(cfg, "sound", "usespeech", 1) == 0;

        usetup.user_data_dir = INIreadstring(cfg, "misc", "user_data_dir");
        usetup.shared_data_dir = INIreadstring(cfg, "misc", "shared_data_dir");

        usetup.translation = INIreadstring(cfg, "language", "translation");

        int cache_size_kb = INIreadint(cfg, "misc", "cachemax", DEFAULTCACHESIZE_KB);
        if (cache_size_kb > 0)
            spriteset.SetMaxCacheSize((size_t)cache_size_kb * 1024);

        usetup.mouse_auto_lock = INIreadint(cfg, "mouse", "auto_lock") > 0;

        usetup.mouse_speed = INIreadfloat(cfg, "mouse", "speed", 1.f);
        if (usetup.mouse_speed <= 0.f)
            usetup.mouse_speed = 1.f;
        const char *mouse_ctrl_options[kNumMouseCtrlOptions] = { "never", "fullscreen", "always" };
        String mouse_str = INIreadstring(cfg, "mouse", "control_when", "fullscreen");
        for (int i = 0; i < kNumMouseCtrlOptions; ++i)
        {
            if (mouse_str.CompareNoCase(mouse_ctrl_options[i]) == 0)
            {
                usetup.mouse_ctrl_when = (MouseControlWhen)i;
                break;
            }
        }
        usetup.mouse_ctrl_enabled = INIreadint(cfg, "mouse", "control_enabled", 1) > 0;
        const char *mouse_speed_options[kNumMouseSpeedDefs] = { "absolute", "current_display" };
        mouse_str = INIreadstring(cfg, "mouse", "speed_def", "current_display");
        for (int i = 0; i < kNumMouseSpeedDefs; ++i)
        {
            if (mouse_str.CompareNoCase(mouse_speed_options[i]) == 0)
            {
                usetup.mouse_speed_def = (MouseSpeedDef)i;
                break;
            }
        }

        usetup.override_multitasking = INIreadint(cfg, "override", "multitasking", -1);
        String override_os = INIreadstring(cfg, "override", "os");
        usetup.override_script_os = -1;
        if (override_os.CompareNoCase("dos") == 0)
        {
            usetup.override_script_os = eOS_DOS;
        }
        else if (override_os.CompareNoCase("win") == 0)
        {
            usetup.override_script_os = eOS_Win;
        }
        else if (override_os.CompareNoCase("linux") == 0)
        {
            usetup.override_script_os = eOS_Linux;
        }
        else if (override_os.CompareNoCase("mac") == 0)
        {
            usetup.override_script_os = eOS_Mac;
        }
        usetup.override_upscale = INIreadint(cfg, "override", "upscale") > 0;
    }

    // Apply logging configuration
    apply_debug_config(cfg);
}

void post_config()
{
    if (usetup.Screen.DriverID.IsEmpty() || usetup.Screen.DriverID.CompareNoCase("DX5") == 0)
        usetup.Screen.DriverID = "Software";

    // FIXME: this correction is needed at the moment because graphics driver
    // implementation requires some filter to be created anyway
    usetup.Screen.Filter.UserRequest = usetup.Screen.Filter.ID;
    if (usetup.Screen.Filter.ID.IsEmpty() || usetup.Screen.Filter.ID.CompareNoCase("none") == 0)
    {
        usetup.Screen.Filter.ID = "StdScale";
    }

    if (!usetup.Screen.FsGameFrame.IsValid())
        usetup.Screen.FsGameFrame = GameFrameSetup(kFrame_MaxProportional);
    if (!usetup.Screen.WinGameFrame.IsValid())
        usetup.Screen.WinGameFrame = GameFrameSetup(kFrame_MaxRound);
    
    // TODO: helper functions to remove slash in paths (or distinct path type)
    if (usetup.user_data_dir.GetLast() == '/' || usetup.user_data_dir.GetLast() == '\\')
        usetup.user_data_dir.ClipRight(1);
    if (usetup.shared_data_dir.GetLast() == '/' || usetup.shared_data_dir.GetLast() == '\\')
        usetup.shared_data_dir.ClipRight(1);
}

void save_config_file()
{
    ConfigTree cfg;

    // Last display mode
    // TODO: force_window check is a temporary workaround (see comment below)
    if (force_window == 0)
    {
        bool is_windowed = System_GetWindowed() != 0;
        cfg["graphics"]["windowed"] = String::FromFormat("%d", is_windowed ? 1 : 0);
        // TODO: this is a hack, necessary because the original config system was designed when
        // switching mode at runtime was not considered a possibility.
        // Normally, two changes need to be done here:
        // * the display setup needs to be reviewed and simplified a bit.
        // * perhaps there should be two saved setups for fullscreen and windowed saved in memory
        // (like ActiveDisplaySetting is saved currently), to know how the window size is defined
        // in each modes (by explicit width/height values or from game scaling).
        // This specifically *must* be done if there will be script API for modifying fullscreen
        // resolution, or size of the window could be changed any way at runtime.
        if (is_windowed != usetup.Screen.DisplayMode.Windowed)
        {
            if (is_windowed)
                cfg["graphics"]["screen_def"] = "scaling";
            else
                cfg["graphics"]["screen_def"] = "max";
        }
    }

    // Other game options that could be changed at runtime
    if (game.options[OPT_RENDERATSCREENRES] == kRenderAtScreenRes_UserDefined)
        cfg["graphics"]["render_at_screenres"] = String::FromFormat("%d", usetup.RenderAtScreenRes ? 1 : 0);
    cfg["mouse"]["control_enabled"] = String::FromFormat("%d", usetup.mouse_ctrl_enabled ? 1 : 0);
    cfg["mouse"]["speed"] = String::FromFormat("%f", Mouse::GetSpeed());
    cfg["language"]["translation"] = usetup.translation;

    String cfg_file = find_user_cfg_file();
    if (!cfg_file.IsEmpty())
        IniUtil::Merge(cfg_file, cfg);
}
