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

#include "common/textconsole.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

int16 global[GLOBAL_LIST_SIZE];
int global_list_size = GLOBAL_LIST_SIZE;

char global_release_name[] = { "ScummVM" };
char global_release_version[] = { "ScummVM" };
char global_release_date[] = { "ScummVM" };
char global_release_copyright[] = { "ScummVM" };


void global_init_code() {
	g_engine->global_init_code();
}

void global_daemon_code() {
	g_engine->global_daemon_code();
}

void global_pre_parser_code() {
	g_engine->global_pre_parser_code();
}

void global_parser_code() {
	g_engine->global_parser_code();
}

void global_error_code() {
	g_engine->global_error_code();
}

void global_room_init() {
	g_engine->global_room_init();
}

void global_verb_filter() {
	g_engine->global_verb_filter();
}

void global_sound_driver() {
	g_engine->global_sound_driver();
}

void global_section_constructor() {
	g_engine->global_section_constructor();
}

void global_write_config_file() {
	write_config_file();
}

} // namespace MADSV2
} // namespace MADS
