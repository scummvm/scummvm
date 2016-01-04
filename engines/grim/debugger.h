/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_DEBUGGER_H
#define GRIM_DEBUGGER_H

#include "common/debug.h"

#include "gui/debugger.h"

namespace Grim {

class Debugger : public GUI::Debugger {
public:
	Debugger();
	virtual ~Debugger();

	bool cmd_checkFiles(int argc, const char **argv);
	bool cmd_lua_do(int argc, const char **argv);
	bool cmd_jump(int argc, const char **argv);
	bool cmd_set_renderer(int argc, const char **argv);
	bool cmd_save(int argc, const char **argv);
	bool cmd_load(int argc, const char **argv);
};

}

#endif
