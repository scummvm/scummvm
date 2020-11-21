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
//
//
//=============================================================================
#ifndef __AGS_EE_MAIN__CONFIG_H
#define __AGS_EE_MAIN__CONFIG_H

#include "main/graphics_mode.h"
#include "util/ini_util.h"

using AGS::Common::String;
using AGS::Common::ConfigTree;

// Set up default config settings
void config_defaults();
// Find and default configuration file (usually located in the game installation directory)
String find_default_cfg_file(const char *alt_cfg_file);
// Find all-games user configuration file
String find_user_global_cfg_file();
// Find and game-specific user configuration file (located into writable user directory)
String find_user_cfg_file();
// Read optional data file name and location from config
void read_game_data_location(const AGS::Common::ConfigTree &cfg);
// Apply overriding values from the external config (e.g. for mobile ports)
void override_config_ext(ConfigTree &cfg);
// Setup game using final config tree
void apply_config(const ConfigTree &cfg);
// Fixup game setup parameters
void post_config();

void save_config_file();

void parse_scaling_option(const String &scaling_option, FrameScaleDefinition &scale_def, int &scale_factor);
void parse_scaling_option(const String &scaling_option, GameFrameSetup &frame_setup);
String make_scaling_option(FrameScaleDefinition scale_def, int scale_factor = 0);
String make_scaling_option(const GameFrameSetup &frame_setup);
uint32_t convert_scaling_to_fp(int scale_factor);
int convert_fp_to_scaling(uint32_t scaling);
// Fill in setup structs with default settings for the given mode (windowed or fullscreen)
void graphics_mode_get_defaults(bool windowed, ScreenSizeSetup &scsz_setup, GameFrameSetup &frame_setup);

typedef struct { char s[5]; } AlIDStr;
// Converts Allegro driver ID type to 4-char string
AlIDStr AlIDToChars(int al_id);
AlIDStr AlIDToChars(const String &s);
// Converts C-string into Allegro's driver ID; string must be at least 4 character long
int StringToAlID(const char *cstr);
// Reads driver ID from config, where it may be represented as string or number
int read_driverid(const ConfigTree &cfg, const String &sectn, const String &item, int def_value);
// Writes driver ID to config
void write_driverid(ConfigTree &cfg, const String &sectn, const String &item, int value);


bool INIreaditem(const ConfigTree &cfg, const String &sectn, const String &item, String &value);
int INIreadint(const ConfigTree &cfg, const String &sectn, const String &item, int def_value = 0);
float INIreadfloat(const ConfigTree &cfg, const String &sectn, const String &item, float def_value = 0.f);
String INIreadstring(const ConfigTree &cfg, const String &sectn, const String &item, const String &def_value = "");
void INIwriteint(ConfigTree &cfg, const String &sectn, const String &item, int value);
void INIwritestring(ConfigTree &cfg, const String &sectn, const String &item, const String &value);
void INIwriteint(ConfigTree &cfg, const String &sectn, const String &item, int value);


#endif // __AGS_EE_MAIN__CONFIG_H
