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

#ifndef MYST_SCRIPTS_H
#define MYST_SCRIPTS_H

#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/util.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
class MystResource;

enum MystScriptType {
	kMystScriptNone,
	kMystScriptNormal,
	kMystScriptInit,
	kMystScriptExit
};

struct MystScriptEntry {
	MystScriptEntry();
	~MystScriptEntry();

	MystScriptType type;
	uint16 u0;
	uint16 opcode;
	uint16 var;
	uint16 argc;
	uint16 *argv;
	uint16 u1;
};

typedef Common::SharedPtr<Common::Array<MystScriptEntry> > MystScript;

class MystScriptParser {
public:
	MystScriptParser(MohawkEngine_Myst *vm);
	virtual ~MystScriptParser();

	void runScript(MystScript script, MystResource *invokingResource = NULL);
	void runOpcode(uint16 op, uint16 var = 0, uint16 argc = 0, uint16 *argv = NULL);
	const char *getOpcodeDesc(uint16 op);
	MystScript readScript(Common::SeekableReadStream *stream, MystScriptType type);

	virtual void disableInitOpcodes() = 0;
	virtual void runPersistentOpcodes() = 0;

	virtual uint16 getVar(uint16 var);
	virtual void toggleVar(uint16 var);
	virtual bool setVarValue(uint16 var, uint16 value);

	DECLARE_OPCODE(unknown);

	DECLARE_OPCODE(o_0_toggleVar);
	DECLARE_OPCODE(o_1_setVar);
	DECLARE_OPCODE(o_2_changeCardSwitch);
	DECLARE_OPCODE(takePage);
	DECLARE_OPCODE(opcode_4);
	DECLARE_OPCODE(o_6_changeCard);
	DECLARE_OPCODE(o_9_triggerMovie);
	DECLARE_OPCODE(o_10_toggleVarNoRedraw);
	DECLARE_OPCODE(o_14_drawAreaState);
	DECLARE_OPCODE(o_15_redrawAreaForVar);
	DECLARE_OPCODE(o_16_changeCardDirectional);
	DECLARE_OPCODE(o_17_changeCardPush);
	DECLARE_OPCODE(o_18_changeCardPop);
	DECLARE_OPCODE(o_19_enableAreas);
	DECLARE_OPCODE(o_20_disableAreas);
	DECLARE_OPCODE(o_21_directionalUpdate);
	DECLARE_OPCODE(o_23_toggleAreasActivation);
	DECLARE_OPCODE(o_24_playSound);
	DECLARE_OPCODE(o_26_stopSoundBackground);
	DECLARE_OPCODE(o_27_playSoundBlocking);
	DECLARE_OPCODE(o_28_restoreDefaultRect);
	DECLARE_OPCODE(o_29_33_blitRect);
	DECLARE_OPCODE(opcode_30);
	DECLARE_OPCODE(opcode_31);
	DECLARE_OPCODE(opcode_32);
	DECLARE_OPCODE(opcode_34);
	DECLARE_OPCODE(opcode_35);
	DECLARE_OPCODE(changeCursor);
	DECLARE_OPCODE(hideCursor);
	DECLARE_OPCODE(showCursor);
	DECLARE_OPCODE(opcode_39);
	DECLARE_OPCODE(changeStack);
	DECLARE_OPCODE(opcode_41);
	DECLARE_OPCODE(opcode_42);
	DECLARE_OPCODE(opcode_43);
	DECLARE_OPCODE(opcode_44);
	DECLARE_OPCODE(opcode_46);

	DECLARE_OPCODE(NOP);

protected:
	MohawkEngine_Myst *_vm;

	typedef void (MystScriptParser::*OpcodeProcMyst)(uint16 op, uint16 var, uint16 argc, uint16* argv);

	struct MystOpcode {
		uint16 op;
		OpcodeProcMyst proc;
		const char *desc;
	};

	const MystOpcode *_opcodes;
	void setupOpcodes();
	MystResource *_invokingResource;
	uint16 _opcodeCount;

	static const uint8 stack_map[];
	static const uint16 start_card[];

	void varUnusedCheck(uint16 op, uint16 var);


};

}

#undef DECLARE_OPCODE

#endif
