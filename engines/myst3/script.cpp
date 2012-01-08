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

#include "engines/myst3/myst3.h"
#include "engines/myst3/script.h"
#include "engines/myst3/hotspot.h"
#include "engines/myst3/variables.h"
#include "engines/myst3/cursor.h"
#include "engines/myst3/inventory.h"

namespace Myst3 {

Script::Script(Myst3Engine *vm):
	_vm(vm) {

#define OP_0(op, x) _commands.push_back(Command(op, &Script::x, #x, 0))
#define OP_1(op, x, type1) _commands.push_back(Command(op, &Script::x, #x, 1, type1))
#define OP_2(op, x, type1, type2) _commands.push_back(Command(op, &Script::x, #x, 2, type1, type2))
#define OP_3(op, x, type1, type2, type3) _commands.push_back(Command(op, &Script::x, #x, 3, type1, type2, type3))
#define OP_4(op, x, type1, type2, type3, type4) _commands.push_back(Command(op, &Script::x, #x, 4, type1, type2, type3, type4))
#define OP_5(op, x, type1, type2, type3, type4, type5) _commands.push_back(Command(op, &Script::x, #x, 5, type1, type2, type3, type4, type5))

	OP_0(  0, badOpcode																					);
	OP_1(  4, nodeCubeInit, 				kEvalValue													);
	OP_5(  6, nodeCubeInitIndex, 			kVar, 		kEvalValue,	kEvalValue,	kEvalValue,	kEvalValue	);
	OP_1(  7, nodeFrameInit, 				kEvalValue													);
	OP_3(  8, nodeFrameInitCond, 			kCondition,	kEvalValue,	kEvalValue							);
	OP_5(  9, nodeFrameInitIndex,			kVar,		kEvalValue,	kEvalValue,	kEvalValue,	kEvalValue	);
	OP_1( 10, nodeMenuInit, 				kEvalValue													);
	OP_0( 11, stopWholeScript																			);
	OP_1( 13, spotItemAdd,					kValue														);
	OP_2( 14, spotItemAddCond,				kValue,		kCondition										);
	OP_2( 15, spotItemAddCondFade,			kValue,		kCondition										);
	OP_1( 17, movieInitLooping, 			kEvalValue													);
	OP_2( 18, movieInitCondLooping,			kEvalValue,	kCondition										);
	OP_2( 19, movieInitCond,				kEvalValue,	kCondition										);
	OP_1( 20, movieInitPreloadLooping,		kEvalValue													);
	OP_2( 21, movieInitCondPreloadLooping,	kEvalValue,	kCondition										);
	OP_2( 22, movieInitCondPreload, 		kEvalValue,	kCondition										);
	OP_2( 23, movieInitFrameVar, 			kEvalValue,	kVar											);
	OP_2( 24, movieInitFrameVarPreload,		kEvalValue,	kVar											);
	OP_4( 25, movieInitOverrridePosition,	kEvalValue,	kCondition,	kValue,		kValue					);
	OP_3( 26, movieInitScriptedPosition,	kEvalValue,	kVar,		kVar								);
	OP_2( 35, sunspotAdd,					kValue,		kValue											);
	OP_3( 36, sunspotAddIntensity,			kValue,		kValue,		kValue								);
	OP_4( 37, sunspotAddVarIntensity,		kValue,		kValue,		kValue,		kVar								);
	OP_4( 38, sunspotAddIntensityColor,		kValue,		kValue,		kValue,		kValue					);
	OP_5( 39, sunspotAddVarIntensityColor,	kValue,		kValue,		kValue,		kValue, 	kVar		);
	OP_4( 40, sunspotAddIntensityRadius,	kValue,		kValue,		kValue,		kValue					);
	OP_5( 41, sunspotAddVarIntensityRadius,	kValue,		kValue,		kValue,		kVar, 		kValue		);
	OP_5( 42, sunspotAddIntColorRadius,		kValue,		kValue,		kValue,		kValue, 	kValue		);
	OP_5( 43, sunspotAddVarIntColorRadius,	kValue,		kValue,		kValue,		kValue, 	kVar		); // Six args
	OP_2( 44, inventoryAddFront,			kVar,		kValue											);
	OP_2( 45, inventoryAddBack,				kVar,		kValue											);
	OP_1( 46, inventoryRemove,				kVar														);
	OP_0( 47, inventoryReset																			);
	OP_1( 49, varSetZero,					kVar														);
	OP_1( 50, varSetOne,					kVar														);
	OP_1( 51, varSetTwo,					kVar														);
	OP_1( 52, varSetOneHundred,				kVar														);
	OP_2( 53, varSetValue,					kVar,		kValue											);
	OP_1( 54, varToggle,					kVar														);
	OP_1( 55, varSetOneIfZero,				kVar														);
	OP_3( 61, varRandRange,					kVar,		kValue,		kValue								);
	OP_5( 63, polarToRect,					kVar,		kVar,		kValue,		kValue, 	kValue		); // Ten args
	OP_2( 67, varRemoveBits,				kVar,		kValue											);
	OP_2( 68, varToggleBits,				kVar,		kValue											);
	OP_2( 69, varCopy,						kVar,		kVar											);
	OP_2( 70, varSetBitsFromVar,			kVar,		kVar											);
	OP_2( 71, varSetBits,					kVar,		kValue											);
	OP_2( 72, varApplyMask,					kVar,		kValue											);
	OP_2( 73, varSwap,						kVar,		kVar											);
	OP_1( 74, varIncrement,					kVar														);
	OP_2( 75, varIncrementMax,				kVar,		kValue											);
	OP_3( 76, varIncrementMaxLooping,		kVar,		kValue,		kValue								);
	OP_4( 77, varAddValueMaxLooping,		kValue,		kVar,		kValue,		kValue					);
	OP_1( 78, varDecrement,					kVar														);
	OP_2( 79, varDecrementMin,				kVar,		kValue											);
	OP_3( 80, varAddValueMax,				kValue,		kVar,		kValue								);
	OP_3( 81, varSubValueMin,				kValue,		kVar,		kValue								);
	OP_2( 82, varZeroRange,					kVar,		kVar											);
	OP_3( 83, varCopyRange,					kVar,		kVar,		kValue								);
	OP_3( 84, varSetRange,					kVar,		kVar,		kValue								);
	OP_1( 85, varIncrementMaxTen,			kVar														);
	OP_2( 86, varAddValue,					kValue,		kVar											);
	OP_3( 87, varArrayAddValue, 			kValue, 	kVar, 		kVar								);
	OP_2( 88, varAddVarValue,				kVar,		kVar											);
	OP_2( 89, varSubValue,					kValue,		kVar											);
	OP_2( 90, varSubVarValue,				kVar,		kVar											);
	OP_2( 91, varModValue,					kVar,		kValue											);
	OP_2( 92, varMultValue,					kVar,		kValue											);
	OP_2( 93, varMultVarValue,				kVar,		kVar											);
	OP_2( 94, varDivValue,					kVar,		kValue											);
	OP_2( 95, varDivVarValue,				kVar,		kVar											);
	OP_2( 97, varMinValue,					kVar,		kValue											);
	OP_3( 98, varClipValue,					kVar,		kValue,		kValue								);
	OP_3( 99, varClipChangeBound,			kVar,		kValue,		kValue								);
	OP_2(100, varAbsoluteSubValue,			kVar,		kValue											);
	OP_2(101, varAbsoluteSubVar,			kVar,		kVar											);
	OP_3(102, varRatioToPercents,			kVar,		kValue,		kValue								);
	OP_4(103, varRotateValue3,				kVar,		kValue,		kValue, 	kValue					);
	OP_0(104, ifElse																					);
	OP_1(105, ifCondition, 					kCondition													);
	OP_2(106, ifCond1AndCond2, 				kCondition,	kCondition										);
	OP_2(107, ifCond1OrCond2, 				kCondition,	kCondition										);
	OP_2(108, ifOneVarSetInRange,			kVar,		kVar											);
	OP_2(109, ifVarEqualsValue,				kVar,		kValue											);
	OP_2(110, ifVarNotEqualsValue,			kVar,		kValue											);
	OP_2(111, ifVar1EqualsVar2,				kVar,		kVar											);
	OP_2(112, ifVar1NotEqualsVar2,			kVar,		kVar											);
	OP_2(113, ifVarSupEqValue,				kVar,		kValue											);
	OP_2(114, ifVarInfEqValue,				kVar,		kValue											);
	OP_3(115, ifVarInRange,					kVar,		kValue,		kValue								);
	OP_3(116, ifVarNotInRange,				kVar,		kValue,		kValue								);
	OP_2(117, ifVar1SupEqVar2,				kVar,		kVar											);
	OP_2(118, ifVar1SupVar2,				kVar,		kVar											);
	OP_2(119, ifVar1InfEqVar2,				kVar,		kVar											);
	OP_2(120, ifVarHasAllBitsSet,			kVar,		kValue											);
	OP_2(121, ifVarHasNoBitsSet,			kVar,		kValue											);
	OP_3(122, ifVarHasSomeBitsSet,			kVar,		kValue,		kValue								);
	OP_4(126, ifMouseIsInRect,				kValue,		kValue,		kValue,		kValue					);
	OP_3(135, chooseNextNode,				kCondition, kValue,		kValue								);
	OP_2(136, goToNodeTransition,			kValue,		kValue											);
	OP_1(137, goToNodeTrans2,				kValue														);
	OP_1(138, goToNodeTrans1,				kValue														);
	OP_2(139, goToRoomNode,					kValue,		kValue											);
	OP_1(140, zipToNode,					kValue														);
	OP_2(141, zipToRoomNode,				kValue,		kValue											);
	OP_1(147, moviePlay, 					kEvalValue													);
	OP_1(148, moviePlaySynchronized,		kEvalValue													);
	OP_2(174, runScriptWhileCond,			kCondition,	kValue											);
	OP_3(175, runScriptWhileCondEachXFrames,kCondition,	kValue,		kValue								);
	OP_4(176, runScriptForVar,				kVar,		kValue,		kValue,		kValue					);
	OP_5(177, runScriptForVarEachXFrames,	kVar,		kValue,		kValue,		kValue,		kValue		);
	OP_4(178, runScriptForVarStartVar,		kVar,		kVar,		kValue,		kValue					);
	OP_5(179, runScriptForVarStartVarEachXFrames, kVar,	kVar,		kValue,		kValue,		kValue		);
	OP_4(180, runScriptForVarEndVar,		kVar,		kValue,		kVar,		kValue					);
	OP_5(181, runScriptForVarEndVarEachXFrames,	kVar,	kValue,		kVar,		kValue,		kValue		);
	OP_4(182, runScriptForVarStartEndVar,	kVar,		kVar,		kVar,		kValue					);
	OP_5(183, runScriptForVarStartEndVarEachXFrames, kVar, kVar,	kVar,		kValue,		kValue		);
	OP_4(184, drawFramesForVar,				kVar,		kValue,		kValue,		kValue					);
	OP_3(185, drawFramesForVarEachTwoFrames,			kVar,		kValue,		kValue					);
	OP_3(186, drawFramesForVarStartEndVarEachTwoFrames, kVar, 		kVar,		kVar					);
	OP_1(187, runScript,					kValue														);

#undef OP_0
#undef OP_1
#undef OP_2
#undef OP_3
#undef OP_4
#undef OP_5
}

Script::~Script() {
}

bool Script::run(const Common::Array<Opcode> *script) {
	debugC(kDebugScript, "Script start %p", (void *) script);

	Context c;
	c.result = true;
	c.endScript = false;
	c.script = script;
	c.op = script->begin();

	while (c.op != script->end()) {
		runOp(c, *c.op);

		if (c.endScript || c.op == script->end())
			break;

		c.op++;
	}

	debugC(kDebugScript, "Script stop %p ", (void *) script);

	return c.result;
}

const Script::Command &Script::findCommand(uint16 op) {
	for (uint16 i = 0; i < _commands.size(); i++)
		if (_commands[i].op == op)
			return _commands[i];

	// Return the invalid opcode if not found
	return findCommand(0);
}

void Script::runOp(Context &c, const Opcode &op) {
	const Script::Command &cmd = findCommand(op.op);

	if (cmd.op != 0)
		(this->*(cmd.proc))(c, op);
	else
		warning("Trying to run invalid opcode %d", op.op);
}

const Common::String Script::describeCommand(uint16 op) {
	const Script::Command &cmd = findCommand(op);

	if (cmd.op != 0)
		return Common::String::format("%d, %s", cmd.op, cmd.desc);
	else
		return Common::String::format("%d", op);
}

const Common::String Script::describeOpcode(const Opcode &opcode) {
	const Script::Command &cmd = findCommand(opcode.op);

	Common::String d = Common::String::format("    op %s ( ",
			describeCommand(opcode.op).c_str());

	for(uint k = 0; k < opcode.args.size(); k++) {
		if (cmd.op != 0 && k < 5)
			d += describeArgument(cmd.argType[k], opcode.args[k]) + " ";
		else
			d += Common::String::format("%d ", opcode.args[k]);
	}

	d += ")\n";

	return d;
}

const Common::String Script::describeArgument(ArgumentType type, int16 value) {
	switch (type) {
	case kVar:
		return _vm->_vars->describeVar(value);
	case kValue:
		return Common::String::format("%d", value);
	case kEvalValue:
		if (value > 0)
			return Common::String::format("%d", value);
		else
			return _vm->_vars->describeVar(-value);
	case kCondition:
		return _vm->_vars->describeCondition(value);
	case kUnknown:
	default:
		return Common::String::format("unk%d", value);
	}
}

void Script::badOpcode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Invalid opcode", cmd.op);

	error("Trying to run invalid opcode %d", cmd.op);
}

void Script::nodeCubeInit(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node cube init %d", cmd.op, cmd.args[0]);

	uint16 nodeId = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadNodeCubeFaces(nodeId);
	// TODO: Load rects
}

void Script::nodeCubeInitIndex(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node cube init indexed %d",
			cmd.op, cmd.args[0]);

	uint16 var = _vm->_vars->get(cmd.args[0]);

	if (var >= cmd.args.size() - 1)
		error("Opcode %d, invalid index %d", cmd.op, var);

	uint16 value = cmd.args[var + 1];

	uint16 nodeId = _vm->_vars->valueOrVarValue(value);
	_vm->loadNodeCubeFaces(nodeId);
	// TODO: Load rects
}

void Script::nodeFrameInit(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node frame init %d", cmd.op, cmd.args[0]);

	uint16 nodeId = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadNodeFrame(nodeId);
	// TODO: Load rects
}

void Script::nodeFrameInitCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node frame init condition %d ? %d : %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint16 value;
	if (_vm->_vars->evaluate(cmd.args[0]))
		value = cmd.args[1];
	else
		value = cmd.args[2];

	uint16 nodeId = _vm->_vars->valueOrVarValue(value);
	_vm->loadNodeFrame(nodeId);
	// TODO: Load rects
}

void Script::nodeFrameInitIndex(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node frame init indexed %d",
			cmd.op, cmd.args[0]);

	uint16 var = _vm->_vars->get(cmd.args[0]);

	if (var >= cmd.args.size() - 1)
		error("Opcode %d, invalid index %d", cmd.op, var);

	uint16 value = cmd.args[var + 1];

	uint16 nodeId = _vm->_vars->valueOrVarValue(value);
	_vm->loadNodeFrame(nodeId);
	// TODO: Load rects
}

void Script::nodeMenuInit(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node menu init %d", cmd.op, cmd.args[0]);

	uint16 nodeId = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadNodeMenu(nodeId);
	// TODO: Load rects
}

void Script::stopWholeScript(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Stop whole script", cmd.op);

	c.result = false;
	c.endScript = true;
}

void Script::spotItemAdd(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Draw spotitem %d", cmd.op, cmd.args[0]);

	_vm->addSpotItem(cmd.args[0], 1, false);
}

void Script::spotItemAddCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add spotitem %d with condition %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->addSpotItem(cmd.args[0], cmd.args[1], false);
}

void Script::spotItemAddCondFade(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add fading spotitem %d for var %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->addSpotItem(cmd.args[0], cmd.args[1], true);
}

void Script::movieInitLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Init movie %d, looping", cmd.op, cmd.args[0]);

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, 1, false, true);
}

void Script::movieInitCondLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Init movie %d with condition %d, looping", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], false, true);
}

void Script::movieInitCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Init movie %d with condition %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], true, false);
}

void Script::movieInitPreloadLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d, looping", cmd.op, cmd.args[0]);

	_vm->_vars->setMoviePreloadToMemory(true);

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, 1, false, true);
}

void Script::movieInitCondPreloadLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with condition %d, looping", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->setMoviePreloadToMemory(true);

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], false, true);
}

void Script::movieInitCondPreload(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with condition %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->setMoviePreloadToMemory(true);

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], true, false);
}

void Script::movieInitFrameVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Init movie %d with next frame var %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->setMovieScriptDriven(true);
	_vm->_vars->setMovieNextFrameGetVar(cmd.args[1]);

	uint32 condition = _vm->_vars->getMovieOverrideCondition();
	_vm->_vars->setMovieOverrideCondition(0);

	if (!condition)
		condition = 1;

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, condition, false, true);
}

void Script::movieInitFrameVarPreload(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with next frame var %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->setMoviePreloadToMemory(true);
	_vm->_vars->setMovieScriptDriven(true);
	_vm->_vars->setMovieNextFrameGetVar(cmd.args[1]);

	uint32 condition = _vm->_vars->getMovieOverrideCondition();
	_vm->_vars->setMovieOverrideCondition(0);

	if (!condition)
		condition = 1;

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, condition, false, true);
}

void Script::movieInitOverrridePosition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with condition %d and position U %d V %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	_vm->_vars->setMoviePreloadToMemory(true);
	_vm->_vars->setMovieScriptDriven(true);
	_vm->_vars->setMovieOverridePosition(true);
	_vm->_vars->setMovieOverridePosU(cmd.args[2]);
	_vm->_vars->setMovieOverridePosV(cmd.args[3]);

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], false, true);
}

void Script::movieInitScriptedPosition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with position U-var %d V-var %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	_vm->_vars->setMoviePreloadToMemory(true);
	_vm->_vars->setMovieScriptDriven(true);
	_vm->_vars->setMovieUVar(cmd.args[1]);
	_vm->_vars->setMovieUVar(cmd.args[2]);

	uint16 movieid = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, 1, false, true);
}

void Script::sunspotAdd(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = _vm->_vars->getSunspotIntensity();
	uint16 color = _vm->_vars->getSunspotColor();
	uint16 radius = _vm->_vars->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddIntensity(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = _vm->_vars->getSunspotColor();
	uint16 radius = _vm->_vars->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddVarIntensity(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = _vm->_vars->getSunspotColor();
	uint16 radius = _vm->_vars->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, cmd.args[3], true, radius);
}

void Script::sunspotAddIntensityColor(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = cmd.args[3];
	uint16 radius = _vm->_vars->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddVarIntensityColor(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = cmd.args[3];
	uint16 radius = _vm->_vars->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, cmd.args[4], true, radius);
}

void Script::sunspotAddIntensityRadius(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = _vm->_vars->getSunspotColor();
	uint16 radius = cmd.args[3];

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddVarIntensityRadius(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = _vm->_vars->getSunspotColor();
	uint16 radius = cmd.args[4];

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, cmd.args[3], true, radius);
}

void Script::sunspotAddIntColorRadius(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = cmd.args[3];
	uint16 radius = cmd.args[4];

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddVarIntColorRadius(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = cmd.args[3];
	uint16 radius = cmd.args[5];

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, cmd.args[4], true, radius);
}

void Script::inventoryAddFront(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Inventory add item %d at front", cmd.op, cmd.args[0]);

	_vm->_inventory->addItem(cmd.args[0], false);
}

void Script::inventoryAddBack(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Inventory add item %d at back", cmd.op, cmd.args[0]);

	_vm->_inventory->addItem(cmd.args[0], true);
}

void Script::inventoryRemove(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Inventory remove item %d", cmd.op, cmd.args[0]);

	_vm->_inventory->removeItem(cmd.args[0]);
}

void Script::inventoryReset(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Reset inventory", cmd.op);

	_vm->_inventory->reset();
}

void Script::varSetZero(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 0", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], 0);
}

void Script::varSetOne(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 1", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], 1);
}

void Script::varSetTwo(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 2", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], 2);
}

void Script::varSetOneHundred(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 100", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], 100);
}

void Script::varSetValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->set(cmd.args[0], cmd.args[1]);
}

void Script::varToggle(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Toggle var %d", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], _vm->_vars->get(cmd.args[0]) == 0);
}

void Script::varSetOneIfZero(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var %d to one if zero", cmd.op, cmd.args[0]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	if (!value)
		_vm->_vars->set(cmd.args[0], 1);
}

void Script::varRandRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Randomize var %d value between %d and %d", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value;

	if (cmd.args[2] - cmd.args[1] > 0)
		value = _vm->_rnd->getRandomNumberRng(cmd.args[1], cmd.args[2]);
	else
		value = cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::polarToRect(Context &c, const Opcode &cmd)	{
	debugC(kDebugScript, "Opcode %d: Complex polar to rect transformation for angle in var %d", cmd.op, cmd.args[8]);

	int32 angleDeg = _vm->_vars->get(cmd.args[8]);
	float angleRad = 2 * M_PI / (cmd.args[9] * angleDeg);
	float angleSin = sin(angleRad);
	float angleCos = cos(angleRad);

	float radiusX;
	float radiusY;
	if (angleSin < 0)
		radiusX = cmd.args[4];
	else
		radiusX = cmd.args[5];
	if (angleCos > 0)
		radiusY = cmd.args[6];
	else
		radiusY = cmd.args[7];

	int32 offsetX = cmd.args[2];
	int32 offsetY = cmd.args[3];

	int32 posX = offsetX + (radiusX - 0.1f) * angleSin;
	int32 posY = offsetY - (radiusY - 0.1f) * angleCos;

	_vm->_vars->set(cmd.args[0], posX);
	_vm->_vars->set(cmd.args[1], posY);
}

void Script::varRemoveBits(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Remove bits %d from var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_vars->get(cmd.args[0]);

	value &= ~cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varToggleBits(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Toggle bits %d from var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_vars->get(cmd.args[0]);

	value ^= cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varCopy(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Copy var %d to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->set(cmd.args[1], _vm->_vars->get(cmd.args[0]));
}

void Script::varSetBitsFromVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set bits from var %d on var %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[1]);

	value |= _vm->_vars->get(cmd.args[0]);

	_vm->_vars->set(cmd.args[1], value);
}

void Script::varSetBits(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set bits %d on var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_vars->get(cmd.args[0]);

	value |= cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varApplyMask(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Apply mask %d on var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_vars->get(cmd.args[0]);

	value &= cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varSwap(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Swap var %d and var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	_vm->_vars->set(cmd.args[0], _vm->_vars->get(cmd.args[1]));
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varIncrement(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d", cmd.op, cmd.args[0]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value++;

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varIncrementMax(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d with max value %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value++;

	if (value > cmd.args[1])
		value = cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varIncrementMaxLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d in range [%d, %d]",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value++;

	if (value > cmd.args[2])
		value = cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varAddValueMaxLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add %d to var %d in range [%d, %d]",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	int32 value = _vm->_vars->get(cmd.args[1]);

	value += cmd.args[0];

	if (value > cmd.args[3])
		value = cmd.args[2];

	_vm->_vars->set(cmd.args[1], value);
}

void Script::varDecrement(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Decrement var %d", cmd.op, cmd.args[0]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value--;

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varDecrementMin(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Decrement var %d with min value %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value--;

	if (value < cmd.args[1])
		value = cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varAddValueMax(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add value %d to var %d with max value %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_vars->get(cmd.args[1]);

	value += cmd.args[0];

	if (value > cmd.args[2])
		value = cmd.args[2];

	_vm->_vars->set(cmd.args[1], value);
}

void Script::varSubValueMin(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Substract value %d from var %d with min value %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_vars->get(cmd.args[1]);

	value -= cmd.args[0];

	if (value < cmd.args[2])
		value = cmd.args[2];

	_vm->_vars->set(cmd.args[1], value);
}

void Script::varZeroRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set vars from %d to %d to zero", cmd.op, cmd.args[0], cmd.args[1]);

	if (cmd.args[0] > cmd.args[1])
		error("Opcode %d, Incorrect range, %d -> %d", cmd.op, cmd.args[0], cmd.args[1]);

	for (uint i = cmd.args[0]; i <= cmd.args[1]; i++)
		_vm->_vars->set(i, 0);
}

void Script::varCopyRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Copy vars from %d to %d, length: %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	if (cmd.args[2] <= 0)
		return;

	for (uint i = 0; i < cmd.args[2]; i++)
		_vm->_vars->set(cmd.args[1] + i, _vm->_vars->get(cmd.args[0] + i));
}

void Script::varSetRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set vars from %d to %d to val %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	if (cmd.args[0] > cmd.args[1])
		error("Opcode %d, Incorrect range, %d -> %d", cmd.op, cmd.args[0], cmd.args[1]);

	for (uint i = cmd.args[0]; i <= cmd.args[1]; i++)
		_vm->_vars->set(i, cmd.args[2]);
}

void Script::varIncrementMaxTen(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d max 10", cmd.op, cmd.args[0]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value++;

	if (value == 10)
		value = 1;

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varAddValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add value %d to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[1]);
	value += cmd.args[0];
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varArrayAddValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add value %d to array base var %d item var %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_vars->get(cmd.args[1] + _vm->_vars->get(cmd.args[2]));
	value += cmd.args[0];
	_vm->_vars->set(cmd.args[1] + _vm->_vars->get(cmd.args[2]), value);
}

void Script::varAddVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add var %d value to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[1]);
	value += _vm->_vars->get(cmd.args[0]);
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varSubValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Substract value %d to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[1]);
	value -= cmd.args[0];
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varSubVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Substract var %d value to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[1]);
	value -= _vm->_vars->get(cmd.args[0]);
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varModValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Apply modulo %d to var %d", cmd.op, cmd.args[1], cmd.args[0]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	value %= cmd.args[1];
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varMultValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Multiply var %d by value %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	value *= cmd.args[1];
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varMultVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Multiply var %d by var %d value", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	value *= _vm->_vars->get(cmd.args[1]);
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varDivValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Divide var %d by value %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	value /= cmd.args[1];
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varDivVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Divide var %d by var %d value", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	value /= _vm->_vars->get(cmd.args[1]);
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varMinValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var %d to min between %d and var value", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	if (value > cmd.args[1])
		value = cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varClipValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Clip var %d value between %d and %d", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value = CLIP<int32>(value, cmd.args[1], cmd.args[2]);

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varClipChangeBound(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Clip var %d value between %d and %d changing bounds", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	if (value < cmd.args[1])
		value = cmd.args[2];

	if (value > cmd.args[2])
		value = cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varAbsoluteSubValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Take absolute value of var %d and substract %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value = abs(value) - cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varAbsoluteSubVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Take absolute value of var %d and substract var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value = abs(value) - _vm->_vars->get(cmd.args[1]);

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varRatioToPercents(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Convert var %d to percents (max value %d, tare weight %d)",
			cmd.op, cmd.args[0], cmd.args[2], cmd.args[1]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	value = 100 * (cmd.args[2] - abs(value - cmd.args[1])) / cmd.args[2];
	value = MAX(0, value);

	_vm->_vars->set(cmd.args[0], value);
}


void Script::varRotateValue3(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Var take next value, var %d values %d %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	int32 value = _vm->_vars->get(cmd.args[0]);

	if (value == cmd.args[1]) {
		value = cmd.args[2];
	} else if (value == cmd.args[2]) {
		value = cmd.args[3];
	} else {
		value = cmd.args[1];
	}

	_vm->_vars->set(cmd.args[0], value);
}

void Script::ifElse(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Else", cmd.op);

	c.result = true;
	c.endScript = true;
}

void Script::goToElse(Context &c) {

	// Go to next command until an else statement is met
	do {
		c.op++;
	} while (c.op != c.script->end()
			&& c.op->op != 104);
}

void Script::ifCondition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If condition %d", cmd.op, cmd.args[0]);

	if (_vm->_vars->evaluate(cmd.args[0]))
		return;

	goToElse(c);
}

void Script::ifCond1AndCond2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If cond %d and cond %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->evaluate(cmd.args[0])
			&& _vm->_vars->evaluate(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifCond1OrCond2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If cond %d or cond %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->evaluate(cmd.args[0])
			|| _vm->_vars->evaluate(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifOneVarSetInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If one var set int range %d %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	uint16 var = cmd.args[0];
	uint16 end = cmd.args[1];

	if (end > var) {
		goToElse(c);
		return;
	}

	bool result = false;

	do {
		result |= _vm->_vars->get(var);
		var++;
	} while (var <= end);

	if (result)
		return;

	goToElse(c);
}

void Script::ifVarEqualsValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d equals value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) == cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVarNotEqualsValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not equals value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) != cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVar1EqualsVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d equals var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) == _vm->_vars->get(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVar1NotEqualsVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not equals var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) != _vm->_vars->get(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVarSupEqValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d >= value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) >= cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVarInfEqValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d <= value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) <= cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVarInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d in range %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	if(value >= cmd.args[1] && value <= cmd.args[2])
		return;

	goToElse(c);
}

void Script::ifVarNotInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not in range %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_vars->get(cmd.args[0]);
	if(value < cmd.args[1] && value > cmd.args[2])
		return;

	goToElse(c);
}

void Script::ifVar1SupEqVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d >= var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) >= _vm->_vars->get(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVar1SupVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d > var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) > _vm->_vars->get(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVar1InfEqVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d <= var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) <= _vm->_vars->get(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVarHasAllBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == val %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[1]);

	if ((_vm->_vars->get(cmd.args[0]) & cmd.args[1]) == cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVarHasNoBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == 0",
			cmd.op, cmd.args[0], cmd.args[1]);

	if ((_vm->_vars->get(cmd.args[0]) & cmd.args[1]) == 0)
		return;

	goToElse(c);
}

void Script::ifVarHasSomeBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == val %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	if ((_vm->_vars->get(cmd.args[0]) & cmd.args[1]) == cmd.args[2])
		return;

	goToElse(c);
}

void Script::ifMouseIsInRect(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If mouse in rect l%d t%d w%d h%d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	Common::Rect r = Common::Rect(cmd.args[2], cmd.args[3]);
	r.translate(cmd.args[0], cmd.args[1]);

	if (r.contains(_vm->_cursor->getPosition()))
		return;

	goToElse(c);
}

void Script::chooseNextNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Choose next node using condition %d", cmd.op, cmd.args[0]);

	if (_vm->_vars->evaluate(cmd.args[0]))
		_vm->_vars->setLocationNextNode(cmd.args[1]);
	else
		_vm->_vars->setLocationNextNode(cmd.args[2]);
}

void Script::goToNodeTransition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d with transition %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->goToNode(cmd.args[0], cmd.args[1]);
}

void Script::goToNodeTrans2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d", cmd.op, cmd.args[0]);

	_vm->goToNode(cmd.args[0], 2);
}

void Script::goToNodeTrans1(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d", cmd.op, cmd.args[0]);

	_vm->goToNode(cmd.args[0], 1);
}

void Script::goToRoomNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to room %d, node %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->setLocationNextRoom(cmd.args[0]);
	_vm->_vars->setLocationNextNode(cmd.args[1]);

	_vm->goToNode(0, 1);
}

void Script::zipToNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Zip to node %d", cmd.op, cmd.args[0]);

	_vm->goToNode(cmd.args[0], 3);
}

void Script::zipToRoomNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Zip to room %d, node %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->setLocationNextRoom(cmd.args[0]);
	_vm->_vars->setLocationNextNode(cmd.args[1]);

	_vm->goToNode(0, 3);
}

void Script::moviePlay(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play movie %d", cmd.op, cmd.args[0]);

	_vm->playSimpleMovie(_vm->_vars->valueOrVarValue(cmd.args[0]));
}

void Script::moviePlaySynchronized(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play movie %d, synchronized with framerate", cmd.op, cmd.args[0]);

	_vm->_vars->setMovieSynchronized(1);
	_vm->playSimpleMovie(_vm->_vars->valueOrVarValue(cmd.args[0]));
}

void Script::runScriptWhileCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: While condition %d, run script %d", cmd.op, cmd.args[0], cmd.args[1]);

	while (_vm->_vars->evaluate(cmd.args[0])) {
		_vm->runScriptsFromNode(cmd.args[1]);
		_vm->processInput(true);
		_vm->drawFrame();
	}

	_vm->processInput(true);
	_vm->drawFrame();
}

void Script::runScriptWhileCondEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: While condition %d, run script %d each %d frames", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint step = cmd.args[2] % 100;

	uint firstStep = cmd.args[2];
	if (firstStep > 100)
		firstStep /= 100;

	uint nextScript = _vm->getFrameCount() + firstStep;

	while (_vm->_vars->evaluate(cmd.args[0])) {

		if (_vm->getFrameCount() >= nextScript) {
			nextScript = _vm->getFrameCount() + step;

			_vm->runScriptsFromNode(cmd.args[1]);
		}

		_vm->processInput(true);
		_vm->drawFrame();
	}

	_vm->processInput(true);
	_vm->drawFrame();
}

void Script::runScriptForVarDrawFramesHelper(uint16 var, int32 startValue, int32 endValue, uint16 script, int32 numFrames) {
	if (numFrames < 0) {
		numFrames = -numFrames;
		uint startFrame = _vm->getFrameCount();
		uint currentFrame = startFrame;
		uint endFrame = startFrame + numFrames;
		uint numValues = abs(endValue - startValue);

		if (startFrame < endFrame) {
			int currentValue = -9999;
			while (1) {
				int nextValue = numValues * (currentFrame - startFrame) / numFrames;
				if (currentValue != nextValue) {
					currentValue = nextValue;

					int16 varValue;
					if (endValue > startValue)
						varValue = startValue + currentValue;
					else
						varValue = startValue - currentValue;

					_vm->_vars->set(var, varValue);

					if (script) {
						_vm->runScriptsFromNode(script);
					}
				}

				_vm->processInput(true);
				_vm->drawFrame();
				currentFrame = _vm->getFrameCount();

				if (currentFrame > endFrame)
					break;
			}
		}

		_vm->_vars->set(var, endValue);
	} else {
		int currentValue = startValue;
		uint endFrame = 0;

		bool positiveDirection = endValue > startValue;

		while (1) {
			if ((positiveDirection && (currentValue >= endValue))
					|| (!positiveDirection && (currentValue <= startValue)))
				break;

			_vm->_vars->set(var, currentValue);

			if (script)
				_vm->runScriptsFromNode(script);

			for (uint i = _vm->getFrameCount(); i < endFrame; i = _vm->getFrameCount())
				_vm->drawFrame();

			endFrame = _vm->getFrameCount() + numFrames;

			currentValue += positiveDirection ? 1 : -1;
		}
	}
}

void Script::runScriptForVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], 0);
}

void Script::runScriptForVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], (int16) cmd.args[4]);
}

void Script::runScriptForVarStartVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to %d, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_vars->get(cmd.args[1]), cmd.args[2], cmd.args[3], 0);
}

void Script::runScriptForVarStartVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to %d, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_vars->get(cmd.args[1]), cmd.args[2], cmd.args[3], (int16) cmd.args[4]);
}

void Script::runScriptForVarEndVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to var %d value, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], _vm->_vars->get(cmd.args[2]), cmd.args[3], 0);
}

void Script::runScriptForVarEndVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], _vm->_vars->get(cmd.args[2]), cmd.args[3], (int16) cmd.args[4]);
}

void Script::runScriptForVarStartEndVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_vars->get(cmd.args[1]), _vm->_vars->get(cmd.args[2]), cmd.args[3], 0);
}

void Script::runScriptForVarStartEndVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_vars->get(cmd.args[1]), _vm->_vars->get(cmd.args[2]), cmd.args[3], (int16) cmd.args[4]);
}

void Script::drawFramesForVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d, every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], cmd.args[2], 0, -((int16) cmd.args[3]));
}

void Script::drawFramesForVarEachTwoFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d draw 2 frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint numFrames = 2 * (-1 - abs(cmd.args[2] - cmd.args[1]));

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], cmd.args[2], 0, numFrames);
}

void Script::drawFramesForVarStartEndVarEachTwoFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value draw 2 frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint numFrames = 2 * (-1 - abs(cmd.args[2] - cmd.args[1]));

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_vars->get(cmd.args[1]), _vm->_vars->get(cmd.args[2]), 0, numFrames);
}

void Script::runScript(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run scripts from node %d", cmd.op, cmd.args[0]);

	_vm->runScriptsFromNode(cmd.args[0], _vm->_vars->getLocationRoom());
}

} /* namespace Myst3 */
