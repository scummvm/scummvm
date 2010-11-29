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

#ifndef MYST_SCRIPTS_MYST_H
#define MYST_SCRIPTS_MYST_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
struct MystScriptEntry;

class MystScriptParser_Myst : public MystScriptParser {
public:
	MystScriptParser_Myst(MohawkEngine_Myst *vm);
	~MystScriptParser_Myst();

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
	void opcode_209_run();
	void opcode_209_disable();
	void opcode_210_run();
	void opcode_210_disable();
	void opcode_211_run();
	void opcode_211_disable();
	void opcode_212_run();
	void opcode_212_disable();

	DECLARE_OPCODE(opcode_100);
	DECLARE_OPCODE(opcode_101);
	DECLARE_OPCODE(opcode_102);
	DECLARE_OPCODE(opcode_103);
	DECLARE_OPCODE(opcode_104);
	DECLARE_OPCODE(opcode_105);
	DECLARE_OPCODE(opcode_106);
	DECLARE_OPCODE(opcode_107);
	DECLARE_OPCODE(opcode_108);
	DECLARE_OPCODE(opcode_109);
	DECLARE_OPCODE(opcode_110);
	DECLARE_OPCODE(opcode_111);
	DECLARE_OPCODE(opcode_112);
	DECLARE_OPCODE(opcode_113);
	DECLARE_OPCODE(opcode_114);
	DECLARE_OPCODE(opcode_115);
	DECLARE_OPCODE(opcode_116);
	DECLARE_OPCODE(opcode_117);
	DECLARE_OPCODE(opcode_118);
	DECLARE_OPCODE(opcode_119);
	DECLARE_OPCODE(opcode_120);
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
	DECLARE_OPCODE(opcode_133);
	DECLARE_OPCODE(opcode_147);
	DECLARE_OPCODE(opcode_164);
	DECLARE_OPCODE(opcode_169);
	DECLARE_OPCODE(opcode_181);
	DECLARE_OPCODE(opcode_182);
	DECLARE_OPCODE(opcode_183);
	DECLARE_OPCODE(opcode_184);
	DECLARE_OPCODE(opcode_185);
	DECLARE_OPCODE(opcode_196);
	DECLARE_OPCODE(opcode_197);
	DECLARE_OPCODE(opcode_198);
	DECLARE_OPCODE(opcode_199);

	DECLARE_OPCODE(opcode_200);
	DECLARE_OPCODE(opcode_201);
	DECLARE_OPCODE(opcode_202);
	DECLARE_OPCODE(opcode_203);
	DECLARE_OPCODE(opcode_204);
	DECLARE_OPCODE(opcode_205);
	DECLARE_OPCODE(opcode_206);
	DECLARE_OPCODE(opcode_207);
	DECLARE_OPCODE(opcode_208);
	DECLARE_OPCODE(opcode_209);
	DECLARE_OPCODE(opcode_210);
	DECLARE_OPCODE(opcode_211);
	DECLARE_OPCODE(opcode_212);
	DECLARE_OPCODE(opcode_213);
	DECLARE_OPCODE(opcode_214);
	DECLARE_OPCODE(opcode_215);
	DECLARE_OPCODE(opcode_216);
	DECLARE_OPCODE(opcode_217);
	DECLARE_OPCODE(opcode_218);
	DECLARE_OPCODE(opcode_219);
	DECLARE_OPCODE(opcode_220);
	DECLARE_OPCODE(opcode_221);
	DECLARE_OPCODE(opcode_222);
	DECLARE_OPCODE(opcode_298);
	DECLARE_OPCODE(opcode_299);

	DECLARE_OPCODE(opcode_300);
	DECLARE_OPCODE(opcode_301);
	DECLARE_OPCODE(opcode_302);
	DECLARE_OPCODE(opcode_303);
	DECLARE_OPCODE(opcode_304);
	DECLARE_OPCODE(opcode_305);
	DECLARE_OPCODE(opcode_306);
	DECLARE_OPCODE(opcode_307);
	DECLARE_OPCODE(opcode_308);
	DECLARE_OPCODE(opcode_309);
	DECLARE_OPCODE(opcode_312);
};

}

#undef DECLARE_OPCODE

#endif
