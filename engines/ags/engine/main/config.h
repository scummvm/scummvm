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

#ifndef AGS_ENGINE_MAIN_CONFIG_H
#define AGS_ENGINE_MAIN_CONFIG_H

#include "main/graphics_mode.h"
#include "util/ini_util.h"

namespace AGS3 {

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

typedef struct {
	char s[5];
} AlIDStr;
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

} // namespace AGS3

#endif
