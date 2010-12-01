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

#ifndef MYST_SCRIPTS_INTRO_H
#define MYST_SCRIPTS_INTRO_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
struct MystScriptEntry;

class MystScriptParser_Intro : public MystScriptParser {
public:
	MystScriptParser_Intro(MohawkEngine_Myst *vm);
	~MystScriptParser_Intro();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();

	DECLARE_OPCODE(o_useLinkBook);

	DECLARE_OPCODE(o_playIntroMovies);
	DECLARE_OPCODE(opcode_201);

	DECLARE_OPCODE(opcode_300);
};

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
