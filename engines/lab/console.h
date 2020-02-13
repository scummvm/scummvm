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

#ifndef LAB_CONSOLE_H
#define LAB_CONSOLE_H

#include "gui/debugger.h"

namespace Lab {

class LabEngine;

class Console : public GUI::Debugger {
public:
	Console(LabEngine *vm);
	~Console(void) override;

private:
	LabEngine *_vm;

	bool Cmd_Scene(int argc, const char **argv);
	bool Cmd_DumpSceneResources(int argc, const char **argv);
	bool Cmd_FindAction(int argc, const char **argv);
};

} // End of namespace Lab
#endif
