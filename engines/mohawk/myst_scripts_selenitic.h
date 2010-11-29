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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/mohawk/myst_scripts.h $
 * $Id: myst_scripts.h 47541 2010-01-25 01:39:44Z lordhoto $
 *
 */

#ifndef MYST_SCRIPTS_SELENITIC_H
#define MYST_SCRIPTS_SELENITIC_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
struct MystScriptEntry;

class MystScriptParser_Selenitic : public MystScriptParser {
public:
	MystScriptParser_Selenitic(MohawkEngine_Myst *vm);
	~MystScriptParser_Selenitic();

	void disableInitOpcodes();
	void runPersistentOpcodes();

private:
	void setupOpcodes();

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

	DECLARE_OPCODE(opcode_101);
	DECLARE_OPCODE(opcode_105);
	DECLARE_OPCODE(opcode_106);
	DECLARE_OPCODE(opcode_107);
	DECLARE_OPCODE(opcode_108);
	DECLARE_OPCODE(opcode_109);
	DECLARE_OPCODE(opcode_110);
	DECLARE_OPCODE(opcode_111);
	DECLARE_OPCODE(opcode_115);

	DECLARE_OPCODE(opcode_200);
	DECLARE_OPCODE(opcode_201);
	DECLARE_OPCODE(opcode_202);
	DECLARE_OPCODE(opcode_203);
	DECLARE_OPCODE(opcode_204);
	DECLARE_OPCODE(opcode_205);
	DECLARE_OPCODE(opcode_206);
};

}

#undef DECLARE_OPCODE

#endif
