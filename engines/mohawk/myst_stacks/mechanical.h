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

#ifndef MYST_SCRIPTS_MECHANICAL_H
#define MYST_SCRIPTS_MECHANICAL_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
struct MystScriptEntry;

class MystScriptParser_Mechanical : public MystScriptParser {
public:
	MystScriptParser_Mechanical(MohawkEngine_Myst *vm);
	~MystScriptParser_Mechanical();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	void opcode_200_run();
	void opcode_200_disable();
	void opcode_201_run();
	void opcode_201_disable();
	void opcode_202_run();
	void opcode_202_disable();
	void opcode_203_run();
	void opcode_203_disable();
	void opcode_204_run();
	void opcode_204_disable();
	void opcode_205_run();
	void opcode_205_disable();
	void opcode_206_run();
	void opcode_206_disable();
	void opcode_209_run();
	void opcode_209_disable();

	DECLARE_OPCODE(opcode_104);
	DECLARE_OPCODE(opcode_105);
	DECLARE_OPCODE(opcode_121);
	DECLARE_OPCODE(opcode_122);
	DECLARE_OPCODE(opcode_123);
	DECLARE_OPCODE(opcode_124);
	DECLARE_OPCODE(opcode_125);
	DECLARE_OPCODE(opcode_126);
	DECLARE_OPCODE(opcode_127);
	DECLARE_OPCODE(opcode_128);
	DECLARE_OPCODE(opcode_129);
	DECLARE_OPCODE(opcode_130);
	DECLARE_OPCODE(opcode_131);
	DECLARE_OPCODE(opcode_132);

	DECLARE_OPCODE(opcode_200);
	DECLARE_OPCODE(opcode_201);
	DECLARE_OPCODE(opcode_202);
	DECLARE_OPCODE(opcode_203);
	DECLARE_OPCODE(opcode_204);
	DECLARE_OPCODE(opcode_205);
	DECLARE_OPCODE(opcode_206);
	DECLARE_OPCODE(opcode_209);

	DECLARE_OPCODE(opcode_300);

	MystGameState::Mechanical &_state;
};

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
