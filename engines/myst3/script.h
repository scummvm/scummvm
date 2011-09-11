/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef SCRIPT_H_
#define SCRIPT_H_

#include "common/array.h"

namespace Myst3 {

class Myst3Engine;
struct Opcode;

#define DECLARE_OPCODE(x) void x(Context &c, const Opcode &cmd)

class Script {
public:
	Script(Myst3Engine *vm);
	virtual ~Script();

	bool run(const Common::Array<Opcode> *script);
	const Common::String describeCommand(uint16 op);

private:
	struct Context {
		bool endScript;
		bool result;
		const Common::Array<Opcode> *script;
		Common::Array<Opcode>::const_iterator op;
	};

	typedef void (Script::*CommandProc)(Context &c, const Opcode &cmd);

	struct Command {
		Command() {}
		Command(uint16 o, CommandProc p, const char *d) : op(o), proc(p), desc(d) {}

		uint16 op;
		CommandProc proc;
		const char *desc;
	};

	Myst3Engine *_vm;
	Common::Array<Command> _commands;

	void runOp(Context &c, const Opcode &op);
	void goToElse(Context &c);

	DECLARE_OPCODE(nodeCubeInit);
	DECLARE_OPCODE(nodeCubeInitIndex);
	DECLARE_OPCODE(nodeFrameInit);
	DECLARE_OPCODE(nodeFrameInitCond);
	DECLARE_OPCODE(nodeFrameInitIndex);
	DECLARE_OPCODE(stopWholeScript);
	DECLARE_OPCODE(sunspotAdd);
	DECLARE_OPCODE(varSetZero);
	DECLARE_OPCODE(varSetOne);
	DECLARE_OPCODE(varSetTwo);
	DECLARE_OPCODE(varSetOneHundred);
	DECLARE_OPCODE(varSetValue);
	DECLARE_OPCODE(varToggle);
	DECLARE_OPCODE(varSetOneIfZero);
	DECLARE_OPCODE(varRemoveBits);
	DECLARE_OPCODE(varToggleBits);
	DECLARE_OPCODE(varCopy);
	DECLARE_OPCODE(varSetBitsFromVar);
	DECLARE_OPCODE(varSetBits);
	DECLARE_OPCODE(varApplyMask);
	DECLARE_OPCODE(varSwap);
	DECLARE_OPCODE(varIncrement);
	DECLARE_OPCODE(varIncrementMax);
	DECLARE_OPCODE(varIncrementMaxLooping);
	DECLARE_OPCODE(varAddValueMaxLooping);
	DECLARE_OPCODE(varDecrement);
	DECLARE_OPCODE(varDecrementMin);
	DECLARE_OPCODE(varAddValueMax);
	DECLARE_OPCODE(varSubValueMin);
	DECLARE_OPCODE(varZeroRange);
	DECLARE_OPCODE(varCopyRange);
	DECLARE_OPCODE(varSetRange);
	DECLARE_OPCODE(varIncrementMaxTen);
	DECLARE_OPCODE(varAddValue);
	DECLARE_OPCODE(varArrayAddValue);
	DECLARE_OPCODE(varAddVarValue);
	DECLARE_OPCODE(varSubValue);
	DECLARE_OPCODE(varSubVarValue);
	DECLARE_OPCODE(varModValue);
	DECLARE_OPCODE(varMultValue);
	DECLARE_OPCODE(varMultVarValue);
	DECLARE_OPCODE(varDivValue);
	DECLARE_OPCODE(varDivVarValue);
	DECLARE_OPCODE(varMinValue);
	DECLARE_OPCODE(varClipValue);
	DECLARE_OPCODE(varRotateValue3);
	DECLARE_OPCODE(ifElse);
	DECLARE_OPCODE(ifCondition);
	DECLARE_OPCODE(ifCond1AndCond2);
	DECLARE_OPCODE(ifCond1OrCond2);
	DECLARE_OPCODE(ifOneVarSetInRange);
	DECLARE_OPCODE(ifVarEqualsValue);
	DECLARE_OPCODE(ifVarNotEqualsValue);
	DECLARE_OPCODE(ifVar1EqualsVar2);
	DECLARE_OPCODE(ifVar1NotEqualsVar2);
	DECLARE_OPCODE(ifVarSupEqValue);
	DECLARE_OPCODE(ifVarInfEqValue);
	DECLARE_OPCODE(ifVarInRange);
	DECLARE_OPCODE(ifVarNotInRange);
	DECLARE_OPCODE(ifVar1SupEqVar2);
	DECLARE_OPCODE(ifVar1SupVar2);
	DECLARE_OPCODE(ifVar1InfEqVar2);
	DECLARE_OPCODE(ifVarHasAllBitsSet);
	DECLARE_OPCODE(ifVarHasNoBitsSet);
	DECLARE_OPCODE(ifVarHasSomeBitsSet);
	DECLARE_OPCODE(goToNode);
	DECLARE_OPCODE(goToRoomNode);
	DECLARE_OPCODE(runScriptsFromNode);
};

} /* namespace Myst3 */
#endif /* SCRIPT_H_ */
