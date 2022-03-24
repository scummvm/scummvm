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

#ifndef AGS_ENGINE_MAIN_CONFIG_H
#define AGS_ENGINE_MAIN_CONFIG_H

#include "ags/engine/main/graphics_mode.h"
#include "ags/shared/util/ini_util.h"

namespace AGS3 {

using AGS::Shared::String;
using AGS::Shared::ConfigTree;

// Set up default config settings
void config_defaults();
// Find and default configuration file (usually located in the game installation directory)
String find_default_cfg_file();
// Find all-games user configuration file
String find_user_global_cfg_file();
// Find and game-specific user configuration file (located into writable user directory)
String find_user_cfg_file();
// Apply overriding values from the external config (e.g. for mobile ports)
void override_config_ext(ConfigTree &cfg);
// Setup game using final config tree
void apply_config(const ConfigTree &cfg);
// Fixup game setup parameters
void post_config();

void save_config_file();

FrameScaleDef parse_scaling_option(const String &option, FrameScaleDef def_value = kFrame_Undefined);
String make_scaling_option(FrameScaleDef scale_def);
uint32_t convert_scaling_to_fp(int scale_factor);
int convert_fp_to_scaling(uint32_t scaling);

bool INIreaditem(const ConfigTree &cfg, const String &sectn, const String &item, String &value);
int INIreadint(const ConfigTree &cfg, const String &sectn, const String &item, int def_value = 0);
float INIreadfloat(const ConfigTree &cfg, const String &sectn, const String &item, float def_value = 0.f);
String INIreadstring(const ConfigTree &cfg, const String &sectn, const String &item, const String &def_value = "");
void INIwriteint(ConfigTree &cfg, const String &sectn, const String &item, int value);
void INIwritestring(ConfigTree &cfg, const String &sectn, const String &item, const String &value);

} // namespace AGS3

#endif
