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

#ifndef AGS_ENGINE_MAIN_MAIN_H
#define AGS_ENGINE_MAIN_MAIN_H

#include "ags/shared/core/platform.h"
#include "ags/shared/util/version.h"
#include "ags/shared/util/ini_util.h"

namespace AGS3 {

using AGS::Shared::ConfigTree;
using AGS::Shared::String;

extern void main_init(int argc, const char *argv[]);

extern int main_process_cmdline(ConfigTree &cfg, int argc, const char *argv[]);

extern String get_engine_string();

extern void main_print_help();

extern void main_set_gamedir(int argc, const char *argv[]);

} // namespace AGS3

#endif
