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
	uint16 resourceId;
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

	virtual void disablePersistentScripts() = 0;
	virtual void runPersistentScripts() = 0;

	virtual uint16 getVar(uint16 var);
	virtual void toggleVar(uint16 var);
	virtual bool setVarValue(uint16 var, uint16 value);

	void animatedUpdate(uint16 argc, uint16 *argv, uint16 delay);

	DECLARE_OPCODE(unknown);

	DECLARE_OPCODE(o_0_toggleVar);
	DECLARE_OPCODE(o_1_setVar);
	DECLARE_OPCODE(o_2_changeCardSwitch);
	DECLARE_OPCODE(o_3_takePage);
	DECLARE_OPCODE(o_4_redrawCard);
	DECLARE_OPCODE(o_6_goToDest);
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
	DECLARE_OPCODE(o_30_changeSound);
	DECLARE_OPCODE(o_31_soundPlaySwitch);
	DECLARE_OPCODE(o_32_soundResumeBackground);
	DECLARE_OPCODE(o_34_changeCard);
	DECLARE_OPCODE(o_35_drawImageChangeCard);
	DECLARE_OPCODE(o_36_changeMainCursor);
	DECLARE_OPCODE(o_37_hideCursor);
	DECLARE_OPCODE(o_38_showCursor);
	DECLARE_OPCODE(o_39_delay);
	DECLARE_OPCODE(o_40_changeStack);
	DECLARE_OPCODE(o_41_changeCardPlaySoundDirectional);
	DECLARE_OPCODE(o_42_directionalUpdatePlaySound);
	DECLARE_OPCODE(o_43_saveMainCursor);
	DECLARE_OPCODE(o_44_restoreMainCursor);
	DECLARE_OPCODE(o_46_soundWaitStop);

	DECLARE_OPCODE(NOP);

protected:
	MohawkEngine_Myst *_vm;

	typedef void (MystScriptParser::*OpcodeProcMyst)(uint16 op, uint16 var, uint16 argc, uint16* argv);

	struct MystOpcode {
		uint16 op;
		OpcodeProcMyst proc;
		const char *desc;
	};

	uint16 _opcodeCount;
	const MystOpcode *_opcodes;

	MystResource *_invokingResource;

	uint16 _savedCardId;
	uint16 _savedCursorId;
	uint16 _tempVar; // Generic temp var used by the scripts

	static const uint8 stack_map[];
	static const uint16 start_card[];

	void setupOpcodes();
	void varUnusedCheck(uint16 op, uint16 var);
};

}

#undef DECLARE_OPCODE

#endif
