/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

#ifndef SCRIPT_H_
#define SCRIPT_H_

#include "common/array.h"

namespace Myst3 {

class Myst3Engine;
class Puzzles;
struct Opcode;

#define DECLARE_OPCODE(x) void x(Context &c, const Opcode &cmd)

class Script {
public:
	Script(Myst3Engine *vm);
	virtual ~Script();

	bool run(const Common::Array<Opcode> *script);
	const Common::String describeOpcode(const Opcode &opcode);

private:
	struct Context {
		bool endScript;
		bool result;
		const Common::Array<Opcode> *script;
		Common::Array<Opcode>::const_iterator op;
	};

	typedef void (Script::*CommandProc)(Context &c, const Opcode &cmd);

	enum ArgumentType {
		kUnknown,
		kVar,
		kValue,
		kEvalValue,
		kCondition
	};

	struct Command {
		Command() {}
		Command(uint16 o, CommandProc p, const char *d, uint8 argc, ...) : op(o), proc(p), desc(d) {
			va_list types;

			for(int j = 0; j < 5; j++)
				argType[j] = kUnknown;

			va_start(types, argc);
			for(int j = 0; j < argc; j++)
				argType[j] = (ArgumentType) va_arg(types, int);
			va_end(types);
		}

		uint16 op;
		CommandProc proc;
		const char *desc;

		ArgumentType argType[5];
	};

	Myst3Engine *_vm;
	Puzzles *_puzzles;

	Common::Array<Command> _commands;

	const Command &findCommand(uint16 op);
	const Common::String describeCommand(uint16 op);
	const Common::String describeArgument(ArgumentType type, int16 value);

	void runOp(Context &c, const Opcode &op);
	void goToElse(Context &c);

	void runScriptForVarDrawFramesHelper(uint16 var, int32 startValue, int32 endValue, uint16 script, int32 numFrames);

	DECLARE_OPCODE(badOpcode);

	DECLARE_OPCODE(nodeCubeInit);
	DECLARE_OPCODE(nodeCubeInitIndex);
	DECLARE_OPCODE(nodeFrameInit);
	DECLARE_OPCODE(nodeFrameInitCond);
	DECLARE_OPCODE(nodeFrameInitIndex);
	DECLARE_OPCODE(nodeMenuInit);
	DECLARE_OPCODE(stopWholeScript);
	DECLARE_OPCODE(spotItemAdd);
	DECLARE_OPCODE(spotItemAddCond);
	DECLARE_OPCODE(spotItemAddCondFade);
	DECLARE_OPCODE(movieInitLooping);
	DECLARE_OPCODE(movieInitCondLooping);
	DECLARE_OPCODE(movieInitCond);
	DECLARE_OPCODE(movieInitPreloadLooping);
	DECLARE_OPCODE(movieInitCondPreloadLooping);
	DECLARE_OPCODE(movieInitCondPreload);
	DECLARE_OPCODE(movieInitFrameVar);
	DECLARE_OPCODE(movieInitFrameVarPreload);
	DECLARE_OPCODE(movieInitOverrridePosition);
	DECLARE_OPCODE(movieInitScriptedPosition);
	DECLARE_OPCODE(sunspotAdd);
	DECLARE_OPCODE(sunspotAddIntensity);
	DECLARE_OPCODE(sunspotAddVarIntensity);
	DECLARE_OPCODE(sunspotAddIntensityColor);
	DECLARE_OPCODE(sunspotAddVarIntensityColor);
	DECLARE_OPCODE(sunspotAddIntensityRadius);
	DECLARE_OPCODE(sunspotAddVarIntensityRadius);
	DECLARE_OPCODE(sunspotAddIntColorRadius);
	DECLARE_OPCODE(sunspotAddVarIntColorRadius);
	DECLARE_OPCODE(inventoryAddFront);
	DECLARE_OPCODE(inventoryAddBack);
	DECLARE_OPCODE(inventoryRemove);
	DECLARE_OPCODE(inventoryReset);
	DECLARE_OPCODE(varSetZero);
	DECLARE_OPCODE(varSetOne);
	DECLARE_OPCODE(varSetTwo);
	DECLARE_OPCODE(varSetOneHundred);
	DECLARE_OPCODE(varSetValue);
	DECLARE_OPCODE(varToggle);
	DECLARE_OPCODE(varSetOneIfZero);
	DECLARE_OPCODE(varRandRange);
	DECLARE_OPCODE(polarToRect);
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
	DECLARE_OPCODE(varClipChangeBound);
	DECLARE_OPCODE(varAbsoluteSubValue);
	DECLARE_OPCODE(varAbsoluteSubVar);
	DECLARE_OPCODE(varRatioToPercents);
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
	DECLARE_OPCODE(ifMouseIsInRect);
	DECLARE_OPCODE(chooseNextNode);
	DECLARE_OPCODE(goToNodeTransition);
	DECLARE_OPCODE(goToNodeTrans2);
	DECLARE_OPCODE(goToNodeTrans1);
	DECLARE_OPCODE(goToRoomNode);
	DECLARE_OPCODE(zipToNode);
	DECLARE_OPCODE(zipToRoomNode);
	DECLARE_OPCODE(moviePlay);
	DECLARE_OPCODE(moviePlaySynchronized);
	DECLARE_OPCODE(runScriptWhileCond);
	DECLARE_OPCODE(runScriptWhileCondEachXFrames);
	DECLARE_OPCODE(runScriptForVar);
	DECLARE_OPCODE(runScriptForVarEachXFrames);
	DECLARE_OPCODE(runScriptForVarStartVar);
	DECLARE_OPCODE(runScriptForVarStartVarEachXFrames);
	DECLARE_OPCODE(runScriptForVarEndVar);
	DECLARE_OPCODE(runScriptForVarEndVarEachXFrames);
	DECLARE_OPCODE(runScriptForVarStartEndVar);
	DECLARE_OPCODE(runScriptForVarStartEndVarEachXFrames);
	DECLARE_OPCODE(drawFramesForVar);
	DECLARE_OPCODE(drawFramesForVarEachTwoFrames);
	DECLARE_OPCODE(drawFramesForVarStartEndVarEachTwoFrames);
	DECLARE_OPCODE(runScript);
	DECLARE_OPCODE(runPuzzle1);
	DECLARE_OPCODE(runPuzzle2);
	DECLARE_OPCODE(runPuzzle3);
	DECLARE_OPCODE(runPuzzle4);

};

} /* namespace Myst3 */
#endif /* SCRIPT_H_ */
