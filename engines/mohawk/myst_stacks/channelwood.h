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

#ifndef MYST_SCRIPTS_CHANNELWOOD_H
#define MYST_SCRIPTS_CHANNELWOOD_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
struct MystScriptEntry;

class MystScriptParser_Channelwood : public MystScriptParser {
public:
	MystScriptParser_Channelwood(MohawkEngine_Myst *vm);
	~MystScriptParser_Channelwood();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	void opcode_202_run();
	void opcode_202_disable();
	void opcode_203_run();
	void opcode_203_disable();

	DECLARE_OPCODE(opcode_101);
	DECLARE_OPCODE(opcode_102);
	DECLARE_OPCODE(opcode_104);
	DECLARE_OPCODE(opcode_117);
	DECLARE_OPCODE(opcode_118);
	DECLARE_OPCODE(opcode_119);
	DECLARE_OPCODE(opcode_122);
	DECLARE_OPCODE(opcode_127);
	DECLARE_OPCODE(opcode_129);

	DECLARE_OPCODE(opcode_201);
	DECLARE_OPCODE(opcode_202);
	DECLARE_OPCODE(opcode_203);

	DECLARE_OPCODE(opcode_300);
};

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
