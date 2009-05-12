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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

 // Console module header file

#ifndef SCI_CONSOLE_H
#define SCI_CONSOLE_H

#include "gui/debugger.h"
#include "sci/scicore/sciconsole.h"

namespace Sci {

class SciEngine;

class Console : public GUI::Debugger {
public:
	Console(SciEngine *vm);
	virtual ~Console();

	void con_hook_command(ConCommand command, const char *name, const char *param, const char *description);

private:
	bool cmdGetVersion(int argc, const char **argv);
	bool cmdSelectors(int argc, const char **argv);
	bool cmdKernelNames(int argc, const char **argv);
	bool cmdSuffixes(int argc, const char **argv);
	bool cmdWords(int argc, const char **argv);
	bool cmdMan(int argc, const char **argv);

private:
	SciEngine *_vm;
};

} // End of namespace Sci

#endif
