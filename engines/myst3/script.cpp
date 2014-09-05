/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/myst3.h"
#include "engines/myst3/script.h"
#include "engines/myst3/hotspot.h"
#include "engines/myst3/state.h"
#include "engines/myst3/cursor.h"
#include "engines/myst3/inventory.h"
#include "engines/myst3/puzzles.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/ambient.h"

#include "common/events.h"

namespace Myst3 {

Script::Script(Myst3Engine *vm):
		_vm(vm) {
	_puzzles = new Puzzles(_vm);

#define OP_0(op, x) _commands.push_back(Command(op, &Script::x, #x, 0))
#define OP_1(op, x, type1) _commands.push_back(Command(op, &Script::x, #x, 1, type1))
#define OP_2(op, x, type1, type2) _commands.push_back(Command(op, &Script::x, #x, 2, type1, type2))
#define OP_3(op, x, type1, type2, type3) _commands.push_back(Command(op, &Script::x, #x, 3, type1, type2, type3))
#define OP_4(op, x, type1, type2, type3, type4) _commands.push_back(Command(op, &Script::x, #x, 4, type1, type2, type3, type4))
#define OP_5(op, x, type1, type2, type3, type4, type5) _commands.push_back(Command(op, &Script::x, #x, 5, type1, type2, type3, type4, type5))

	// TODO: Implement these remaining opcodes
	// 5: I'm pretty sure it's useless
	// 144: drawTransition
	// 247: quit

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
	OP_5( 16, spotItemAddMenu,				kValue,		kCondition,		kValue,		kValue, 	kValue	); // Six args
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
	OP_1( 27, movieRemove,					kEvalValue													);
	OP_0( 28, movieRemoveAll																			);
	OP_1( 29, movieSetLooping,				kValue														);
	OP_1( 30, movieSetNotLooping,			kValue														);
	OP_1( 31, waterEffectSetSpeed,			kValue														);
	OP_1( 32, waterEffectSetAttenuation,	kValue														);
	OP_2( 33, waterEffectSetWave,			kValue,		kValue											);
	OP_2( 34, shakeEffectSet,				kEvalValue,	kEvalValue										);
	OP_2( 35, sunspotAdd,					kValue,		kValue											);
	OP_3( 36, sunspotAddIntensity,			kValue,		kValue,		kValue								);
	OP_4( 37, sunspotAddVarIntensity,		kValue,		kValue,		kValue,		kVar					);
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
	OP_1( 48, inventoryAddSaavChapter,		kVar														);
	OP_1( 49, varSetZero,					kVar														);
	OP_1( 50, varSetOne,					kVar														);
	OP_1( 51, varSetTwo,					kVar														);
	OP_1( 52, varSetOneHundred,				kVar														);
	OP_2( 53, varSetValue,					kVar,		kValue											);
	OP_1( 54, varToggle,					kVar														);
	OP_1( 55, varSetOneIfNotZero,				kVar														);
	OP_1( 56, varOpposite,					kVar														);
	OP_1( 57, varAbsolute,					kVar														);
	OP_1( 58, varDereference,				kVar														);
	OP_1( 59, varReferenceSetZero,			kVar														);
	OP_2( 60, varReferenceSetValue,			kVar,		kValue											);
	OP_3( 61, varRandRange,					kVar,		kValue,		kValue								);
	OP_5( 62, polarToRectSimple,			kVar,		kVar,		kValue,		kValue, 	kValue		); // Seven args
	OP_5( 63, polarToRect,					kVar,		kVar,		kValue,		kValue, 	kValue		); // Ten args
	OP_4( 64, varSetDistanceToZone,			kVar,		kValue,		kValue,		kValue					);
	OP_4( 65, varSetMinDistanceToZone,		kVar,		kValue,		kValue,		kValue					);
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
	OP_5( 96, varCrossMultiplication,		kVar,		kValue,		kValue,		kValue,		kValue		);
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
	OP_2(123, ifHeadingInRange,				kValue,		kValue											);
	OP_2(124, ifPitchInRange,				kValue,		kValue											);
	OP_4(125, ifHeadingPitchInRect,			kValue,		kValue,		kValue,		kValue					);
	OP_4(126, ifMouseIsInRect,				kValue,		kValue,		kValue,		kValue					);
	OP_5(127, leverDrag,					kValue,		kValue,		kValue,		kValue, 	kVar		); // Six args
	OP_5(130, leverDragXY,					kVar, 		kVar,		kValue,		kValue,		kValue		);
	OP_5(131, itemDrag,						kVar, 		kValue,		kValue,		kValue,		kVar		);
	OP_2(132, leverDragPositions,			kVar, 		kValue											); // Variable args
	OP_5(134, runScriptWhileDragging,		kVar, 		kVar,		kValue,		kValue, 	kVar		); // Eight args
	OP_3(135, chooseNextNode,				kCondition, kValue,		kValue								);
	OP_2(136, goToNodeTransition,			kValue,		kValue											);
	OP_1(137, goToNodeTrans2,				kValue														);
	OP_1(138, goToNodeTrans1,				kValue														);
	OP_2(139, goToRoomNode,					kValue,		kValue											);
	OP_1(140, zipToNode,					kValue														);
	OP_2(141, zipToRoomNode,				kValue,		kValue											);
	OP_0(144, drawTransition																			);
	OP_0(145, reloadNode																				);
	OP_0(146, redrawFrame																				);
	OP_1(147, moviePlay, 					kEvalValue													);
	OP_1(148, moviePlaySynchronized,		kEvalValue													);
	OP_1(149, moviePlayFullFrame,			kEvalValue													);
	OP_1(150, moviePlayFullFrameTrans,		kEvalValue													);
	OP_2(151, moviePlayChangeNode,			kEvalValue,	kEvalValue										);
	OP_2(152, moviePlayChangeNodeTrans,		kEvalValue,	kEvalValue										);
	OP_2(153, lookAt,						kValue, 	kValue											);
	OP_3(154, lookAtInXFrames,				kValue, 	kValue,		kValue								);
	OP_1(155, lookAtMovieStart,				kEvalValue													);
	OP_2(156, lookAtMovieStartInXFrames,	kEvalValue,	kValue											);
	OP_4(157, cameraLimitMovement,			kValue,		kValue,		kValue,		kValue					);
	OP_0(158, cameraFreeMovement																		);
	OP_2(159, cameraLookAt,					kValue,		kValue											);
	OP_1(160, cameraLookAtVar,				kVar														);
	OP_1(161, cameraGetLookAt,				kVar														);
	OP_1(162, lookAtMovieStartImmediate,	kEvalValue													);
	OP_1(163, cameraSetFOV,					kEvalValue													);
	OP_1(164, changeNode,					kValue														);
	OP_2(165, changeNodeRoom,				kValue,		kValue											);
	OP_3(166, changeNodeRoomAge,			kValue,		kValue,		kValue								);
	OP_0(168, uselessOpcode																				);
	OP_1(169, drawXFrames,					kValue														);
	OP_1(171, drawWhileCond,				kCondition													);
	OP_1(172, whileStart,					kCondition													);
	OP_0(173, whileEnd																					);
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
	OP_1(187, runScript,					kEvalValue													);
	OP_2(188, runScriptWithVar,				kEvalValue, kValue											);
	OP_1(189, runCommonScript,				kValue														);
	OP_2(190, runCommonScriptWithVar,		kEvalValue, kValue											);
	OP_1(194, runPuzzle1,					kValue														);
	OP_2(195, runPuzzle2,					kValue,		kValue											);
	OP_3(196, runPuzzle3,					kValue,		kValue,		kValue								);
	OP_4(197, runPuzzle4,					kValue,		kValue,		kValue,		kValue					);
	OP_3(198, ambientLoadNode,				kValue,		kValue,		kValue								);
	OP_1(199, ambientReloadCurrentNode,		kEvalValue													);
	OP_2(200, ambientPlayCurrentNode,		kValue,		kValue											);
	OP_0(201, ambientApply																				);
	OP_1(202, ambientApplyWithFadeDelay,	kEvalValue													);
	OP_0(203, soundPlayBadClick																			);
	OP_5(204, soundPlayBlocking,			kEvalValue,	kEvalValue,	kEvalValue,	kEvalValue,	kValue		);
	OP_1(205, soundPlay,					kEvalValue													);
	OP_2(206, soundPlayVolume,				kEvalValue,	kEvalValue										);
	OP_3(207, soundPlayVolumeDirection,		kEvalValue,	kEvalValue,	kEvalValue							);
	OP_4(208, soundPlayVolumeDirectionAtt,	kEvalValue,	kEvalValue,	kEvalValue,	kEvalValue				);
	OP_1(209, soundStopEffect,				kEvalValue													);
	OP_2(210, soundFadeOutEffect,			kEvalValue,	kEvalValue										);
	OP_1(212, soundPlayLooping,				kEvalValue													);
	OP_5(213, soundPlayFadeInOut,			kEvalValue,	kEvalValue,	kEvalValue,	kEvalValue,	kEvalValue	);
	OP_5(214, soundChooseNext,				kVar,		kValue,		kValue,		kEvalValue,	kEvalValue	);
	OP_5(215, soundRandomizeNext,			kVar,		kValue,		kValue,		kEvalValue,	kEvalValue	);
	OP_5(216, soundChooseNextAfterOther,	kVar,		kValue,		kValue,		kEvalValue,	kEvalValue	); // Seven args
	OP_5(217, soundRandomizeNextAfterOther,	kVar,		kValue,		kValue,		kEvalValue,	kEvalValue	); // Seven args
	OP_1(218, ambientSetFadeOutDelay,		kValue														);
	OP_2(219, ambientAddSound1,				kEvalValue,	kEvalValue										);
	OP_3(220, ambientAddSound2,				kEvalValue,	kEvalValue,	kValue								);
	OP_3(222, ambientAddSound3,				kEvalValue,	kEvalValue,	kValue								);
	OP_4(223, ambientAddSound4,				kEvalValue,	kEvalValue,	kValue,		kValue					);
	OP_3(224, ambientAddSound5,				kEvalValue,	kEvalValue,	kEvalValue							);
	OP_2(225, ambientSetCue1,				kValue,		kEvalValue										);
	OP_3(226, ambientSetCue2,				kValue,		kEvalValue,	kValue								);
	OP_4(227, ambientSetCue3,				kValue,		kEvalValue,	kValue, kValue						);
	OP_2(228, ambientSetCue4,				kValue,		kEvalValue										);
	OP_1(229, runAmbientScriptNode,			kEvalValue													);
	OP_4(230, runAmbientScriptNodeRoomAge,	kEvalValue,	kEvalValue,	kEvalValue,	kEvalValue				);
	OP_1(231, runSoundScriptNode,			kEvalValue													);
	OP_2(232, runSoundScriptNodeRoom,		kEvalValue,	kEvalValue										);
	OP_3(233, runSoundScriptNodeRoomAge,	kEvalValue,	kEvalValue,	kEvalValue							);
	OP_1(234, soundStopMusic,				kEvalValue													);
	OP_1(235, movieSetStartupSound,			kEvalValue													);
	OP_2(236, movieSetStartupSoundVolume,	kEvalValue,	kEvalValue										);
	OP_3(237, movieSetStartupSoundVolumeH,	kEvalValue,	kEvalValue,	kEvalValue							);
	OP_0(239, drawOneFrame																				);
	OP_0(240, cursorHide																				);
	OP_0(241, cursorShow																				);
	OP_1(242, cursorSet,					kValue														);
	OP_0(243, cursorLock																				);
	OP_0(244, cursorUnlock																				);
	OP_1(248, dialogOpen,					kEvalValue													);
	OP_0(249, newGame																					);

	if (_vm->getPlatform() == Common::kPlatformXbox) {
		// The Xbox version inserted two new opcodes, one at position
		// 27, the other at position 77, shifting all the other opcodes
		shiftCommands(77, 1);
		OP_3(77, varDecrementMinLooping,	kVar,		kValue,		kValue								);

		shiftCommands(27, 1);
		// TODO: Add Xbox opcode 27
	}

#undef OP_0
#undef OP_1
#undef OP_2
#undef OP_3
#undef OP_4
#undef OP_5
}

Script::~Script() {
	delete _puzzles;
}

bool Script::run(const Common::Array<Opcode> *script) {
	debugC(kDebugScript, "Script start %p", (const void *) script);

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

	debugC(kDebugScript, "Script stop %p ", (const void *) script);

	return c.result;
}

const Script::Command &Script::findCommand(uint16 op) {
	for (uint16 i = 0; i < _commands.size(); i++)
		if (_commands[i].op == op)
			return _commands[i];

	// Return the invalid opcode if not found
	return findCommand(0);
}

const Script::Command &Script::findCommandByProc(CommandProc proc) {
	for (uint16 i = 0; i < _commands.size(); i++)
		if (_commands[i].proc == proc)
			return _commands[i];

	// Return the invalid opcode if not found
	return findCommand(0);
}

void Script::shiftCommands(uint16 base, int32 value) {
	for (uint16 i = 0; i < _commands.size(); i++)
		if (_commands[i].op >= base)
			_commands[i].op += value;
}

void Script::runOp(Context &c, const Opcode &op) {
	const Script::Command &cmd = findCommand(op.op);

	if (cmd.op != 0)
		(this->*(cmd.proc))(c, op);
	else
		debugC(kDebugScript, "Trying to run invalid opcode %d", op.op);
}

void Script::runSingleOp(const Opcode &op) {
	Context c;
	runOp(c, op);
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
		return _vm->_state->describeVar(value);
	case kValue:
		return Common::String::format("%d", value);
	case kEvalValue:
		if (value > 0)
			return Common::String::format("%d", value);
		else
			return _vm->_state->describeVar(-value);
	case kCondition:
		return _vm->_state->describeCondition(value);
	case kUnknown:
	default:
		return Common::String::format("unk%d", value);
	}
}

void Script::badOpcode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Invalid opcode", cmd.op);

	error("Trying to run invalid opcode %d", cmd.op);
}

void Script::uselessOpcode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Useless opcode", cmd.op);

	// List of useless opcodes

	// 167 and 168 form a pair. 168 resets what 167 sets up.
	// Since 167 is never used, 168 is marked as useless.
}


void Script::nodeCubeInit(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node cube init %d", cmd.op, cmd.args[0]);

	uint16 nodeId = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadNodeCubeFaces(nodeId);
}

void Script::nodeCubeInitIndex(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node cube init indexed %d",
			cmd.op, cmd.args[0]);

	uint16 var = _vm->_state->getVar(cmd.args[0]);

	if (var >= cmd.args.size() - 1)
		error("Opcode %d, invalid index %d", cmd.op, var);

	uint16 value = cmd.args[var + 1];

	uint16 nodeId = _vm->_state->valueOrVarValue(value);
	_vm->loadNodeCubeFaces(nodeId);
}

void Script::nodeFrameInit(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node frame init %d", cmd.op, cmd.args[0]);

	uint16 nodeId = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadNodeFrame(nodeId);
}

void Script::nodeFrameInitCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node frame init condition %d ? %d : %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint16 value;
	if (_vm->_state->evaluate(cmd.args[0]))
		value = cmd.args[1];
	else
		value = cmd.args[2];

	uint16 nodeId = _vm->_state->valueOrVarValue(value);
	_vm->loadNodeFrame(nodeId);
}

void Script::nodeFrameInitIndex(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node frame init indexed %d",
			cmd.op, cmd.args[0]);

	uint16 var = _vm->_state->getVar(cmd.args[0]);

	if (var >= cmd.args.size() - 1)
		error("Opcode %d, invalid index %d", cmd.op, var);

	uint16 value = cmd.args[var + 1];

	uint16 nodeId = _vm->_state->valueOrVarValue(value);
	_vm->loadNodeFrame(nodeId);
}

void Script::nodeMenuInit(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node menu init %d", cmd.op, cmd.args[0]);

	uint16 nodeId = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadNodeMenu(nodeId);
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

void Script::spotItemAddMenu(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add menu spotitem %d with condition %d", cmd.op, cmd.args[0], cmd.args[1]);

	Common::Rect rect = Common::Rect(cmd.args[4], cmd.args[5]);
	rect.translate(cmd.args[2], cmd.args[3]);

	_vm->addMenuSpotItem(cmd.args[0], cmd.args[1], rect);
}

void Script::movieInitLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Init movie %d, looping", cmd.op, cmd.args[0]);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, 1, false, true);
}

void Script::movieInitCondLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Init movie %d with condition %d, looping", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], false, true);
}

void Script::movieInitCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Init movie %d with condition %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], true, false);
}

void Script::movieInitPreloadLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d, looping", cmd.op, cmd.args[0]);

	_vm->_state->setMoviePreloadToMemory(true);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, 1, false, true);
}

void Script::movieInitCondPreloadLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with condition %d, looping", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setMoviePreloadToMemory(true);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], false, true);
}

void Script::movieInitCondPreload(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with condition %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setMoviePreloadToMemory(true);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], true, false);
}

void Script::movieInitFrameVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Init movie %d with next frame var %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setMovieScriptDriven(true);
	_vm->_state->setMovieNextFrameGetVar(cmd.args[1]);

	uint32 condition = _vm->_state->getMovieOverrideCondition();
	_vm->_state->setMovieOverrideCondition(0);

	if (!condition)
		condition = 1;

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, condition, false, true);
}

void Script::movieInitFrameVarPreload(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with next frame var %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setMoviePreloadToMemory(true);
	_vm->_state->setMovieScriptDriven(true);
	_vm->_state->setMovieNextFrameGetVar(cmd.args[1]);

	uint32 condition = _vm->_state->getMovieOverrideCondition();
	_vm->_state->setMovieOverrideCondition(0);

	if (!condition)
		condition = 1;

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, condition, false, true);
}

void Script::movieInitOverrridePosition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with condition %d and position U %d V %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	_vm->_state->setMoviePreloadToMemory(true);
	_vm->_state->setMovieScriptDriven(true);
	_vm->_state->setMovieOverridePosition(true);
	_vm->_state->setMovieOverridePosU(cmd.args[2]);
	_vm->_state->setMovieOverridePosV(cmd.args[3]);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], false, true);
}

void Script::movieInitScriptedPosition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with position U-var %d V-var %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	_vm->_state->setMoviePreloadToMemory(true);
	_vm->_state->setMovieScriptDriven(true);
	_vm->_state->setMovieUVar(cmd.args[1]);
	_vm->_state->setMovieVVar(cmd.args[2]);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, 1, false, true);
}

void Script::movieRemove(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Remove movie %d ",
			cmd.op, cmd.args[0]);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->removeMovie(movieid);
}

void Script::movieRemoveAll(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Remove all movies",
			cmd.op);

	_vm->removeMovie(0);
}

void Script::movieSetLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set movie %d to loop",
			cmd.op, cmd.args[0]);

	_vm->setMovieLooping(cmd.args[0], true);
}

void Script::movieSetNotLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set movie %d not to loop",
			cmd.op, cmd.args[0]);

	_vm->setMovieLooping(cmd.args[0], false);
}

void Script::waterEffectSetSpeed(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set water effect speed to %d",
			cmd.op, cmd.args[0]);

	_vm->_state->setWaterEffectSpeed(cmd.args[0]);
}

void Script::waterEffectSetAttenuation(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set water effect attenuation to %d",
			cmd.op, cmd.args[0]);

	_vm->_state->setWaterEffectAttenuation(cmd.args[0]);
}

void Script::waterEffectSetWave(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set water effect frequency to %d and amplitude to %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setWaterEffectFrequency(cmd.args[0]);
	_vm->_state->setWaterEffectAmpl(cmd.args[1]);
}

void Script::shakeEffectSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set shake effect amplitude to %d and period to %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	uint16 ampl = _vm->_state->valueOrVarValue(cmd.args[0]);
	uint16 period = _vm->_state->valueOrVarValue(cmd.args[1]);

	_vm->_state->setShakeEffectAmpl(ampl);
	_vm->_state->setShakeEffectFramePeriod(period);
}

void Script::sunspotAdd(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = _vm->_state->getSunspotIntensity();
	uint16 color = _vm->_state->getSunspotColor();
	uint16 radius = _vm->_state->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddIntensity(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = _vm->_state->getSunspotColor();
	uint16 radius = _vm->_state->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddVarIntensity(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = _vm->_state->getSunspotColor();
	uint16 radius = _vm->_state->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, cmd.args[3], true, radius);
}

void Script::sunspotAddIntensityColor(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = cmd.args[3];
	uint16 radius = _vm->_state->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddVarIntensityColor(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = cmd.args[3];
	uint16 radius = _vm->_state->getSunspotRadius();

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, cmd.args[4], true, radius);
}

void Script::sunspotAddIntensityRadius(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = _vm->_state->getSunspotColor();
	uint16 radius = cmd.args[3];

	_vm->addSunSpot(cmd.args[0], cmd.args[1], intensity, color, 1, false, radius);
}

void Script::sunspotAddVarIntensityRadius(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 intensity = cmd.args[2];
	uint16 color = _vm->_state->getSunspotColor();
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

void Script::inventoryAddSaavChapter(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Get new Saavedro chapter %d", cmd.op, cmd.args[0]);

	_vm->_inventory->addSaavedroChapter(cmd.args[0]);
}

void Script::varSetZero(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 0", cmd.op, cmd.args[0]);

	_vm->_state->setVar(cmd.args[0], 0);
}

void Script::varSetOne(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 1", cmd.op, cmd.args[0]);

	_vm->_state->setVar(cmd.args[0], 1);
}

void Script::varSetTwo(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 2", cmd.op, cmd.args[0]);

	_vm->_state->setVar(cmd.args[0], 2);
}

void Script::varSetOneHundred(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 100", cmd.op, cmd.args[0]);

	_vm->_state->setVar(cmd.args[0], 100);
}

void Script::varSetValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setVar(cmd.args[0], cmd.args[1]);
}

void Script::varToggle(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Toggle var %d", cmd.op, cmd.args[0]);

	_vm->_state->setVar(cmd.args[0], _vm->_state->getVar(cmd.args[0]) == 0);
}

void Script::varSetOneIfNotZero(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var %d to one if not zero", cmd.op, cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	if (value)
		_vm->_state->setVar(cmd.args[0], 1);
}

void Script::varOpposite(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Take the opposite of var %d", cmd.op, cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	_vm->_state->setVar(cmd.args[0], -value);
}

void Script::varAbsolute(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Take the absolute value of var %d", cmd.op, cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	_vm->_state->setVar(cmd.args[0], abs(value));
}

void Script::varDereference(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Dereference var %d", cmd.op, cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	_vm->_state->setVar(cmd.args[0], _vm->_state->getVar(value));
}

void Script::varReferenceSetZero(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set to zero the var referenced by var %d", cmd.op, cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	if (!value)
		return;

	_vm->_state->setVar(value, 0);
}

void Script::varReferenceSetValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set to %d the var referenced by var %d", cmd.op, cmd.args[1], cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	if (!value)
		return;

	_vm->_state->setVar(value, cmd.args[1]);
}

void Script::varRandRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Randomize var %d value between %d and %d", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value;

	if (cmd.args[2] - cmd.args[1] > 0)
		value = _vm->_rnd->getRandomNumberRng(cmd.args[1], cmd.args[2]);
	else
		value = cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::polarToRectSimple(Context &c, const Opcode &cmd)	{
	debugC(kDebugScript, "Opcode %d: Polar to rect transformation for angle in var %d", cmd.op, cmd.args[5]);

	int32 angleDeg = _vm->_state->getVar(cmd.args[5]);
	float angleRad = 2 * LOCAL_PI / cmd.args[6] * angleDeg;
	float angleSin = sin(angleRad);
	float angleCos = cos(angleRad);

	int32 offsetX = cmd.args[2];
	int32 offsetY = cmd.args[3];

	float radius;
	if (cmd.args[4] >= 0)
		radius = cmd.args[4] - 0.1;
	else
		radius = cmd.args[4] * -0.1;

	int32 posX = (int32)(offsetX + radius * angleSin);
	int32 posY = (int32)(offsetY - radius * angleCos);

	_vm->_state->setVar(cmd.args[0], posX);
	_vm->_state->setVar(cmd.args[1], posY);
}

void Script::polarToRect(Context &c, const Opcode &cmd)	{
	debugC(kDebugScript, "Opcode %d: Complex polar to rect transformation for angle in var %d", cmd.op, cmd.args[8]);

	int32 angleDeg = _vm->_state->getVar(cmd.args[8]);
	float angleRad = 2 * LOCAL_PI / cmd.args[9] * angleDeg;
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

	int32 posX = (int32)(offsetX + (radiusX - 0.1f) * angleSin);
	int32 posY = (int32)(offsetY - (radiusY - 0.1f) * angleCos);

	_vm->_state->setVar(cmd.args[0], posX);
	_vm->_state->setVar(cmd.args[1], posY);
}

void Script::varSetDistanceToZone(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var %d to distance to point %d %d", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	float heading = _vm->_state->getLookAtHeading();
	float pitch = _vm->_state->getLookAtPitch();
	int16 distance = (int16)(100 * _vm->_scene->distanceToZone(cmd.args[2], cmd.args[1], cmd.args[3], heading, pitch));

	_vm->_state->setVar(cmd.args[0], distance);
}

void Script::varSetMinDistanceToZone(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var %d to distance to point %d %d if lower", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	float heading = _vm->_state->getLookAtHeading();
	float pitch = _vm->_state->getLookAtPitch();
	int16 distance = (int16)(100 * _vm->_scene->distanceToZone(cmd.args[2], cmd.args[1], cmd.args[3], heading, pitch));
	if (distance < _vm->_state->getVar(cmd.args[0]))
		_vm->_state->setVar(cmd.args[0], distance);
}

void Script::varRemoveBits(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Remove bits %d from var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_state->getVar(cmd.args[0]);

	value &= ~cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varToggleBits(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Toggle bits %d from var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_state->getVar(cmd.args[0]);

	value ^= cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varCopy(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Copy var %d to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setVar(cmd.args[1], _vm->_state->getVar(cmd.args[0]));
}

void Script::varSetBitsFromVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set bits from var %d on var %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_state->getVar(cmd.args[1]);

	value |= _vm->_state->getVar(cmd.args[0]);

	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varSetBits(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set bits %d on var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_state->getVar(cmd.args[0]);

	value |= cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varApplyMask(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Apply mask %d on var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_state->getVar(cmd.args[0]);

	value &= cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varSwap(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Swap var %d and var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	_vm->_state->setVar(cmd.args[0], _vm->_state->getVar(cmd.args[1]));
	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varIncrement(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d", cmd.op, cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value++;

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varIncrementMax(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d with max value %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value++;

	if (value > cmd.args[1])
		value = cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varIncrementMaxLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d in range [%d, %d]",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value++;

	if (value > cmd.args[2])
		value = cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varAddValueMaxLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add %d to var %d in range [%d, %d]",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	int32 value = _vm->_state->getVar(cmd.args[1]);

	value += cmd.args[0];

	if (value > cmd.args[3])
		value = cmd.args[2];

	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varDecrement(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Decrement var %d", cmd.op, cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value--;

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varDecrementMin(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Decrement var %d with min value %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value--;

	if (value < cmd.args[1])
		value = cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varAddValueMax(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add value %d to var %d with max value %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[1]);

	value += cmd.args[0];

	if (value > cmd.args[2])
		value = cmd.args[2];

	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varSubValueMin(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Substract value %d from var %d with min value %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[1]);

	value -= cmd.args[0];

	if (value < cmd.args[2])
		value = cmd.args[2];

	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varZeroRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set vars from %d to %d to zero", cmd.op, cmd.args[0], cmd.args[1]);

	if (cmd.args[0] > cmd.args[1])
		error("Opcode %d, Incorrect range, %d -> %d", cmd.op, cmd.args[0], cmd.args[1]);

	for (int16 i = cmd.args[0]; i <= cmd.args[1]; i++)
		_vm->_state->setVar(i, 0);
}

void Script::varCopyRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Copy vars from %d to %d, length: %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	if (cmd.args[2] <= 0)
		return;

	for (int16 i = 0; i < cmd.args[2]; i++)
		_vm->_state->setVar(cmd.args[1] + i, _vm->_state->getVar(cmd.args[0] + i));
}

void Script::varSetRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set vars from %d to %d to val %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	if (cmd.args[0] > cmd.args[1])
		error("Opcode %d, Incorrect range, %d -> %d", cmd.op, cmd.args[0], cmd.args[1]);

	for (int16 i = cmd.args[0]; i <= cmd.args[1]; i++)
		_vm->_state->setVar(i, cmd.args[2]);
}

void Script::varIncrementMaxTen(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d max 10", cmd.op, cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value++;

	if (value == 10)
		value = 1;

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varAddValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add value %d to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[1]);
	value += cmd.args[0];
	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varArrayAddValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add value %d to array base var %d item var %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[1] + _vm->_state->getVar(cmd.args[2]));
	value += cmd.args[0];
	_vm->_state->setVar(cmd.args[1] + _vm->_state->getVar(cmd.args[2]), value);
}

void Script::varAddVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add var %d value to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[1]);
	value += _vm->_state->getVar(cmd.args[0]);
	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varSubValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Substract value %d to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[1]);
	value -= cmd.args[0];
	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varSubVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Substract var %d value to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[1]);
	value -= _vm->_state->getVar(cmd.args[0]);
	_vm->_state->setVar(cmd.args[1], value);
}

void Script::varModValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Apply modulo %d to var %d", cmd.op, cmd.args[1], cmd.args[0]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	value %= cmd.args[1];
	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varMultValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Multiply var %d by value %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	value *= cmd.args[1];
	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varMultVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Multiply var %d by var %d value", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	value *= _vm->_state->getVar(cmd.args[1]);
	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varDivValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Divide var %d by value %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	value /= cmd.args[1];
	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varDivVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Divide var %d by var %d value", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	value /= _vm->_state->getVar(cmd.args[1]);
	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varCrossMultiplication(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Cross multiply var %d from range %d %d to range %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	if (value == 0)
		return;

	int32 temp = abs(value) - cmd.args[1];
	temp *= (cmd.args[4] - cmd.args[3]) / (cmd.args[2] - cmd.args[1]);
	temp += cmd.args[3];

	_vm->_state->setVar(cmd.args[0], value > 0 ? temp : -temp);
}

void Script::varMinValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var %d to min between %d and var value", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	if (value > cmd.args[1])
		value = cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varClipValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Clip var %d value between %d and %d", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value = CLIP<int32>(value, cmd.args[1], cmd.args[2]);

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varClipChangeBound(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Clip var %d value between %d and %d changing bounds", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	if (value < cmd.args[1])
		value = cmd.args[2];

	if (value > cmd.args[2])
		value = cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varAbsoluteSubValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Take absolute value of var %d and substract %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value = abs(value) - cmd.args[1];

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varAbsoluteSubVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Take absolute value of var %d and substract var %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value = abs(value) - _vm->_state->getVar(cmd.args[1]);

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::varRatioToPercents(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Convert var %d to percents (max value %d, tare weight %d)",
			cmd.op, cmd.args[0], cmd.args[2], cmd.args[1]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value = 100 * (cmd.args[2] - abs(value - cmd.args[1])) / cmd.args[2];
	value = MAX<int32>(0, value);

	_vm->_state->setVar(cmd.args[0], value);
}


void Script::varRotateValue3(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Var take next value, var %d values %d %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	if (value == cmd.args[1]) {
		value = cmd.args[2];
	} else if (value == cmd.args[2]) {
		value = cmd.args[3];
	} else {
		value = cmd.args[1];
	}

	_vm->_state->setVar(cmd.args[0], value);
}

void Script::ifElse(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Else", cmd.op);

	c.result = true;
	c.endScript = true;
}

void Script::goToElse(Context &c) {
	const Command &elseCommand = findCommandByProc(&Script::ifElse);

	// Go to next command until an else statement is met
	do {
		c.op++;
	} while (c.op != c.script->end() && c.op->op != elseCommand.op);
}

void Script::ifCondition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If condition %d", cmd.op, cmd.args[0]);

	if (_vm->_state->evaluate(cmd.args[0]))
		return;

	goToElse(c);
}

void Script::ifCond1AndCond2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If cond %d and cond %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->evaluate(cmd.args[0])
			&& _vm->_state->evaluate(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifCond1OrCond2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If cond %d or cond %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->evaluate(cmd.args[0])
			|| _vm->_state->evaluate(cmd.args[1]))
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
		result |= _vm->_state->getVar(var) != 0;
		var++;
	} while (var <= end);

	if (result)
		return;

	goToElse(c);
}

void Script::ifVarEqualsValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d equals value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) == cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVarNotEqualsValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not equals value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) != cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVar1EqualsVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d equals var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) == _vm->_state->getVar(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVar1NotEqualsVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not equals var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) != _vm->_state->getVar(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVarSupEqValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d >= value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) >= cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVarInfEqValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d <= value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) <= cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVarInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d in range %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	if(value >= cmd.args[1] && value <= cmd.args[2])
		return;

	goToElse(c);
}

void Script::ifVarNotInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not in range %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[0]);
	if(value < cmd.args[1] || value > cmd.args[2])
		return;

	goToElse(c);
}

void Script::ifVar1SupEqVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d >= var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) >= _vm->_state->getVar(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVar1SupVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d > var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) > _vm->_state->getVar(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVar1InfEqVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d <= var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_state->getVar(cmd.args[0]) <= _vm->_state->getVar(cmd.args[1]))
		return;

	goToElse(c);
}

void Script::ifVarHasAllBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == val %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[1]);

	if ((_vm->_state->getVar(cmd.args[0]) & cmd.args[1]) == cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifVarHasNoBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == 0",
			cmd.op, cmd.args[0], cmd.args[1]);

	if ((_vm->_state->getVar(cmd.args[0]) & cmd.args[1]) == 0)
		return;

	goToElse(c);
}

void Script::ifVarHasSomeBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == val %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	if ((_vm->_state->getVar(cmd.args[0]) & cmd.args[1]) == cmd.args[2])
		return;

	goToElse(c);
}

void Script::ifHeadingInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If heading in range %d -> %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	float heading = _vm->_state->getLookAtHeading();

	if (cmd.args[1] > cmd.args[0]) {
		// If heading in range
		if (heading > cmd.args[0] && heading < cmd.args[1]) {
			return;
		}
	} else {
		// If heading *not* in range
		if (heading > cmd.args[0] || heading < cmd.args[1]) {
			return;
		}
	}

	goToElse(c);
}

void Script::ifPitchInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If pitch in range %d -> %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	float pitch = _vm->_state->getLookAtPitch();

	// If pitch in range
	if (pitch > cmd.args[0] && pitch < cmd.args[1])
		return;

	goToElse(c);
}

void Script::ifHeadingPitchInRect(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If heading in range %d -> %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	float heading = _vm->_state->getLookAtHeading();
	float pitch = _vm->_state->getLookAtPitch();

	// If pitch in range
	if (pitch <= cmd.args[0] || pitch >= cmd.args[1]) {
		goToElse(c);
		return;
	}

	if (cmd.args[3] > cmd.args[2]) {
		// If heading in range
		if (heading > cmd.args[2] && heading < cmd.args[3])
			return;
	} else {
		// If heading *not* in range
		if (heading > cmd.args[2] || heading < cmd.args[3])
			return;
	}

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

void Script::leverDrag(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Drag lever for var %d with script %d", cmd.op, cmd.args[4], cmd.args[6]);

	int16 minPosX = cmd.args[0];
	int16 minPosY = cmd.args[1];
	int16 maxPosX = cmd.args[2];
	int16 maxPosY = cmd.args[3];
	int16 var = cmd.args[4];
	int16 numPositions = cmd.args[5];
	int16 script = cmd.args[6];
	int16 topOffset = _vm->_state->getViewType() != kMenu ? 30 : 0;

	_vm->_cursor->changeCursor(2);

	bool mousePressed = true;
	while (true) {
		float ratioPosition = 0.0;
		// Compute the distance to the minimum lever point
		// and divide it by the lever movement amplitude
		if (_vm->_state->getViewType() == kCube) {
			float pitch, heading;
			_vm->_cursor->getDirection(pitch, heading);

			float amplitude = sqrt(Math::square(maxPosX - minPosX) + Math::square(maxPosY - minPosY));
			float distanceToMin = sqrt(Math::square(pitch - minPosX) + Math::square(heading - minPosY));
			float distanceToMax = sqrt(Math::square(pitch - maxPosX) + Math::square(heading - maxPosY));

			ratioPosition = distanceToMax < amplitude ? distanceToMin / amplitude : 0.0;
		} else {
			Common::Point mouse = _vm->_cursor->getPosition();
			int16 amplitude;
			int16 pixelPosition;

			if (minPosX == maxPosX) {
				// Vertical slider
				amplitude = maxPosY - minPosY;
				pixelPosition = mouse.y - minPosY - topOffset;
			} else {
				// Horizontal slider
				amplitude = maxPosX - minPosX;
				pixelPosition = mouse.x - minPosX;
			}

			ratioPosition = pixelPosition / (float) amplitude;
		}

		int16 position = (int16)(ratioPosition * (numPositions + 1));
		position = CLIP<int16>(position, 1, numPositions);

		if (_vm->_state->getDragLeverLimited()) {
			int16 minPosition = _vm->_state->getDragLeverLimitMin();
			int16 maxPosition = _vm->_state->getDragLeverLimitMax();
			position = CLIP(position, minPosition, maxPosition);
		}

		// Set new lever position
		_vm->_state->setVar(var, position);

		// Draw a frame
		_vm->processInput(true);
		_vm->drawFrame();

		mousePressed = _vm->getEventManager()->getButtonState() & Common::EventManager::LBUTTON;
		_vm->_state->setDragEnded(!mousePressed);

		if (_vm->_state->getDragLeverSpeed()) {
			debugC(kDebugScript, "Interaction with var 58 is missing in opcode 127.");
			return;
		}

		if (script) {
			_vm->_state->setVar(var, position);
			_vm->runScriptsFromNode(abs(script));
		}

		if (!mousePressed)
			break;
	}

	_vm->_state->setDragLeverLimited(0);
	_vm->_state->setDragLeverSpeed(0);
}

void Script::leverDragPositions(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Drag lever for var %d with script %d", cmd.op, cmd.args[0], cmd.args[1]);

	int16 var = cmd.args[0];
	int16 script = cmd.args[1];
	uint16 numPositions = (cmd.args.size() - 3) / 3;

	if (cmd.args[2 + numPositions * 3] != -1)
		error("leverDragPositions no end marker found");

	_vm->_cursor->changeCursor(2);

	bool mousePressed = true;
	while (true) {
		float pitch, heading;
		_vm->_cursor->getDirection(pitch, heading);

		float minDistance = 180.0;
		uint position = 0;

		// Find the lever position where the distance between the lever
		// and the mouse is minimal, by trying every possible position.
		for (uint i = 0; i < numPositions; i++) {
			float posPitch = cmd.args[2 + i * 3 + 0] * 0.1;
			float posHeading = cmd.args[2 + i * 3 + 1] * 0.1;

			// Distance between the mouse and the lever
			float distance = sqrt(Math::square(pitch - posPitch) + Math::square(heading - posHeading));

			if (distance < minDistance) {
				minDistance = distance;
				position = cmd.args[2 + i * 3 + 2];
			}
		}

		// Set new lever position
		_vm->_state->setVar(var, position);

		// Draw a frame
		_vm->processInput(true);
		_vm->drawFrame();

		mousePressed = _vm->inputValidatePressed();
		_vm->_state->setDragEnded(!mousePressed);

		if (_vm->_state->getDragLeverSpeed()) {
			debugC(kDebugScript, "Interaction with var 58 is missing in opcode 132.");
			return;
		}

		if (script) {
			_vm->_state->setVar(var, position);
			_vm->runScriptsFromNode(abs(script));
		}

		if (!mousePressed)
			break;
	}

	_vm->_state->setDragLeverSpeed(0);
}

void Script::leverDragXY(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Drag 2D lever and update X (var %d) and Y (var %d) coordinates, while running script %d", cmd.op, cmd.args[0], cmd.args[1], cmd.args[4]);

	uint16 varX = cmd.args[0];
	uint16 varY = cmd.args[1];
	uint16 scale = cmd.args[2];
	uint16 maxLeverPosition = cmd.args[3];
	uint16 script = _vm->_state->valueOrVarValue(cmd.args[4]);

	Common::Point mouseInit = _vm->_cursor->getPosition();

	_vm->_cursor->changeCursor(2);

	bool mousePressed = true;
	do {
		Common::Point mouse = _vm->_cursor->getPosition();
		int16 distanceX = (mouseInit.x - mouse.x) / scale;
		int16 distanceY = (mouseInit.y - mouse.y) / scale;

		distanceX = CLIP<int16>(distanceX, -maxLeverPosition, maxLeverPosition);
		distanceY = CLIP<int16>(distanceY, -maxLeverPosition, maxLeverPosition);

		// Set lever position variables
		_vm->_state->setVar(varX, distanceX);
		_vm->_state->setVar(varY, distanceY);

		// Draw a frame
		_vm->processInput(true);
		_vm->drawFrame();

		mousePressed = _vm->getEventManager()->getButtonState() & Common::EventManager::LBUTTON;
		_vm->_state->setDragEnded(!mousePressed);

		// Run script
		if (script)
			_vm->runScriptsFromNode(script);
	} while (mousePressed);
}

void Script::itemDrag(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Drag item %d", cmd.op, cmd.args[4]);
	_vm->dragItem(cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);
}

void Script::runScriptWhileDragging(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: While dragging lever, run script %d", cmd.op, cmd.args[7]);

	uint16 script = _vm->_state->valueOrVarValue(cmd.args[7]);
	uint16 maxDistance = cmd.args[6];
	uint16 maxLeverPosition = cmd.args[5];
	int16 lastLeverPosition = _vm->_state->getVar(cmd.args[4]);
	int16 leverHeight = cmd.args[3];
	int16 leverWidth = cmd.args[2];
	int16 topOffset = _vm->_state->getViewType() != kMenu ? 30 : 0;

	_vm->_cursor->changeCursor(2);

	bool mousePressed = true;
	do {
		mousePressed = _vm->getEventManager()->getButtonState() & Common::EventManager::LBUTTON;
		_vm->_state->setDragEnded(!mousePressed);

		_vm->processInput(true);
		_vm->drawFrame();

		// Distance between the mouse and the lever
		Common::Point mouse = _vm->_cursor->getPosition();
		int16 distanceX = mouse.x - leverWidth / 2 - _vm->_state->getVar(cmd.args[0]);
		int16 distanceY = mouse.y - leverHeight / 2 - _vm->_state->getVar(cmd.args[1]) - topOffset;
		float distance = sqrt((float) distanceX * distanceX + distanceY * distanceY);

		uint16 bestPosition = lastLeverPosition;
		if (distance > maxDistance) {
			_vm->_state->setDragLeverPositionChanged(false);
		} else {
			// Find the lever position where the distance between the lever
			// and the mouse is minimal, by trying every possible position.
			float minDistance = 1000;
			for (uint i = 0; i < maxLeverPosition; i++) {
				_vm->_state->setDragPositionFound(false);

				_vm->_state->setVar(cmd.args[4], i);
				_vm->runScriptsFromNode(script);

				mouse = _vm->_cursor->getPosition();
				distanceX = mouse.x - leverWidth / 2 - _vm->_state->getVar(cmd.args[0]);
				distanceY = mouse.y - leverHeight / 2 - _vm->_state->getVar(cmd.args[1]) - topOffset;
				distance = sqrt((float) distanceX * distanceX + distanceY * distanceY);

				if (distance < minDistance) {
					minDistance = distance;
					bestPosition = i;
				}
			}
			_vm->_state->setDragLeverPositionChanged(bestPosition != lastLeverPosition);
		}

		// Set the lever position to the best position
		_vm->_state->setDragPositionFound(true);
		_vm->_state->setVar(cmd.args[4], bestPosition);

		_vm->runScriptsFromNode(script);
		_vm->processInput(true);
		_vm->drawFrame();
	} while (mousePressed);
}

void Script::chooseNextNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Choose next node using condition %d", cmd.op, cmd.args[0]);

	if (_vm->_state->evaluate(cmd.args[0]))
		_vm->_state->setLocationNextNode(cmd.args[1]);
	else
		_vm->_state->setLocationNextNode(cmd.args[2]);
}

void Script::goToNodeTransition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d with transition %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->goToNode(cmd.args[0], static_cast<TransitionType>(cmd.args[1]));
}

void Script::goToNodeTrans2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d", cmd.op, cmd.args[0]);

	_vm->goToNode(cmd.args[0], kTransitionNone);
}

void Script::goToNodeTrans1(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d", cmd.op, cmd.args[0]);

	_vm->goToNode(cmd.args[0], kTransitionFade);
}

void Script::goToRoomNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to room %d, node %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setLocationNextRoom(cmd.args[0]);
	_vm->_state->setLocationNextNode(cmd.args[1]);

	_vm->goToNode(0, kTransitionFade);
}

void Script::zipToNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Zip to node %d", cmd.op, cmd.args[0]);

	_vm->goToNode(cmd.args[0], kTransitionZip);
}

void Script::zipToRoomNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Zip to room %d, node %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setLocationNextRoom(cmd.args[0]);
	_vm->_state->setLocationNextNode(cmd.args[1]);

	_vm->goToNode(0, kTransitionZip);
}

void Script::drawTransition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Draw transition", cmd.op);

	_vm->drawTransition(kTransitionFade);
}

void Script::reloadNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Reload current node", cmd.op);

	_vm->loadNode(0);
	_vm->drawFrame();
}

void Script::redrawFrame(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Reload current node", cmd.op);

	_vm->drawFrame();
}

void Script::moviePlay(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play movie %d", cmd.op, cmd.args[0]);

	_vm->playSimpleMovie(_vm->_state->valueOrVarValue(cmd.args[0]));
}

void Script::moviePlaySynchronized(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play movie %d, synchronized with framerate", cmd.op, cmd.args[0]);

	_vm->_state->setMovieSynchronized(1);
	_vm->playSimpleMovie(_vm->_state->valueOrVarValue(cmd.args[0]));
}

void Script::cameraLimitMovement(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Limit camera movement in a rect", cmd.op);

	_vm->_state->limitCubeCamera(cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);
}

void Script::cameraFreeMovement(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Free camera movement from rect", cmd.op);

	_vm->_state->freeCubeCamera();
}

void Script::cameraLookAt(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Camera look at %d %d", cmd.op, cmd.args[0], cmd.args[1]);

	float pitch = cmd.args[0];
	float heading = cmd.args[1];
	_vm->_state->lookAt(pitch, heading);
}

void Script::cameraLookAtVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Camera look at value of var %d", cmd.op, cmd.args[0]);

	float pitch = _vm->_state->getVar(cmd.args[0]) / 1000.0;
	float heading = _vm->_state->getVar(cmd.args[0] + 1) / 1000.0;
	_vm->_state->lookAt(pitch, heading);
}

void Script::cameraGetLookAt(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Save camera look at to var %d", cmd.op, cmd.args[0]);

	float pitch = _vm->_state->getLookAtPitch() * 1000.0;
	float heading = _vm->_state->getLookAtHeading() * 1000.0;

	_vm->_state->setVar(cmd.args[0],(int32) pitch);
	_vm->_state->setVar(cmd.args[0] + 1, (int32)heading);
}

void Script::lookAtMovieStartImmediate(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Look at movie %d start", cmd.op, cmd.args[0]);

	uint16 movieId = _vm->_state->valueOrVarValue(cmd.args[0]);

	float startPitch, startHeading;
	_vm->getMovieLookAt(movieId, true, startPitch, startHeading);
	_vm->_state->lookAt(startPitch, startHeading);
}

void Script::cameraSetFOV(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set camera fov %d", cmd.op, cmd.args[0]);

	int32 fov = _vm->_state->valueOrVarValue(cmd.args[0]);

	_vm->_state->setLookAtFOV(fov);
}

void Script::changeNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d", cmd.op, cmd.args[0]);

	_vm->loadNode(cmd.args[0]);
}

void Script::changeNodeRoom(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d room %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->loadNode(cmd.args[1], cmd.args[0]);
}

void Script::changeNodeRoomAge(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d room %d age %d", cmd.op, cmd.args[2], cmd.args[1], cmd.args[0]);

	_vm->loadNode(cmd.args[2], cmd.args[1], cmd.args[0]);
}

void Script::drawXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Draw %d frames", cmd.op, cmd.args[0]);

	uint32 endFrame = _vm->_state->getFrameCount() + cmd.args[0];

	while (_vm->_state->getFrameCount() < endFrame) {
		_vm->processInput(true);
		_vm->drawFrame();
	}
}

void Script::drawWhileCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: While condition %d, draw", cmd.op, cmd.args[0]);

	while (_vm->_state->evaluate(cmd.args[0]) && !_vm->inputEscapePressed()) {
		_vm->processInput(true);
		_vm->drawFrame();
	}
}

void Script::whileStart(Context &c, const Opcode &cmd) {
	const Command &whileEndCommand = findCommandByProc(&Script::whileEnd);

	c.whileStart = c.op - 1;

	// Check the while condition
	if (!_vm->_state->evaluate(cmd.args[0])) {
		// Condition is false, go to the next opcode after the end of the while loop
		do {
			c.op++;
		} while (c.op != c.script->end() && c.op->op != whileEndCommand.op);
	}

	_vm->processInput(true);
	_vm->drawFrame();
}

void Script::whileEnd(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: End of while condition", cmd.op);

	// Go to while start
	c.op = c.whileStart;
}

void Script::runScriptWhileCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: While condition %d, run script %d", cmd.op, cmd.args[0], cmd.args[1]);

	while (_vm->_state->evaluate(cmd.args[0])) {
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

	uint nextScript = _vm->_state->getFrameCount() + firstStep;

	while (_vm->_state->evaluate(cmd.args[0])) {

		if (_vm->_state->getFrameCount() >= nextScript) {
			nextScript = _vm->_state->getFrameCount() + step;

			_vm->runScriptsFromNode(cmd.args[1]);
		}

		_vm->processInput(true);
		_vm->drawFrame();
	}

	_vm->processInput(true);
	_vm->drawFrame();
}

void Script::moviePlayFullFrame(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play movie %d", cmd.op, cmd.args[0]);

	uint16 movieId = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->_cursor->setVisible(false);
	_vm->playMovieFullFrame(movieId);
	_vm->_cursor->setVisible(true);
}

void Script::moviePlayFullFrameTrans(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play movie %d with transition", cmd.op, cmd.args[0]);

	uint16 movieId = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->_cursor->setVisible(false);
	_vm->playMovieFullFrame(movieId);
	_vm->_cursor->setVisible(true);

	_vm->drawTransition(kTransitionFade);
}

void Script::moviePlayChangeNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play movie %d, go to node %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint16 nodeId = _vm->_state->valueOrVarValue(cmd.args[0]);
	uint16 movieId = _vm->_state->valueOrVarValue(cmd.args[1]);
	_vm->_cursor->setVisible(false);
	_vm->playMovieGoToNode(movieId, nodeId);
	_vm->_cursor->setVisible(true);
}

void Script::moviePlayChangeNodeTrans(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play movie %d, go to node %d with transition", cmd.op, cmd.args[1], cmd.args[0]);

	uint16 nodeId = _vm->_state->valueOrVarValue(cmd.args[0]);
	uint16 movieId = _vm->_state->valueOrVarValue(cmd.args[1]);
	_vm->_cursor->setVisible(false);
	_vm->playMovieGoToNode(movieId, nodeId);
	_vm->_cursor->setVisible(true);

	_vm->drawTransition(kTransitionFade);
}

void Script::lookAt(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Look at %d, %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->animateDirectionChange(cmd.args[0], cmd.args[1], 0);
}

void Script::lookAtInXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Look at %d, %d in %d frames", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	_vm->animateDirectionChange(cmd.args[0], cmd.args[1], cmd.args[2]);
}

void Script::lookAtMovieStart(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Look at movie %d start", cmd.op, cmd.args[0]);

	uint16 movieId = _vm->_state->valueOrVarValue(cmd.args[0]);

	float startPitch, startHeading;
	_vm->getMovieLookAt(movieId, true, startPitch, startHeading);
	_vm->animateDirectionChange(startPitch, startHeading, 0);
}

void Script::lookAtMovieStartInXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Look at movie %d start in %d frames", cmd.op, cmd.args[0], cmd.args[1]);

	uint16 movieId = _vm->_state->valueOrVarValue(cmd.args[0]);

	float startPitch, startHeading;
	_vm->getMovieLookAt(movieId, true, startPitch, startHeading);
	_vm->animateDirectionChange(startPitch, startHeading, cmd.args[1]);
}

void Script::runScriptForVarDrawFramesHelper(uint16 var, int32 startValue, int32 endValue, uint16 script, int32 numFrames) {
	if (numFrames < 0) {
		numFrames = -numFrames;
		uint startFrame = _vm->_state->getFrameCount();
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

					_vm->_state->setVar(var, varValue);

					if (script) {
						_vm->runScriptsFromNode(script);
					}
				}

				_vm->processInput(true);
				_vm->drawFrame();
				currentFrame = _vm->_state->getFrameCount();

				if (currentFrame > endFrame)
					break;
			}
		}

		_vm->_state->setVar(var, endValue);
	} else {
		int currentValue = startValue;
		uint endFrame = 0;

		bool positiveDirection = endValue > startValue;

		while (1) {
			if ((positiveDirection && (currentValue > endValue))
					|| (!positiveDirection && (currentValue < endValue)))
				break;

			_vm->_state->setVar(var, currentValue);

			if (script)
				_vm->runScriptsFromNode(script);

			for (uint i = _vm->_state->getFrameCount(); i < endFrame; i = _vm->_state->getFrameCount())
				_vm->drawFrame();

			endFrame = _vm->_state->getFrameCount() + numFrames;

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

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);
}

void Script::runScriptForVarStartVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to %d, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), cmd.args[2], cmd.args[3], 0);
}

void Script::runScriptForVarStartVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to %d, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), cmd.args[2], cmd.args[3], cmd.args[4]);
}

void Script::runScriptForVarEndVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to var %d value, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], _vm->_state->getVar(cmd.args[2]), cmd.args[3], 0);
}

void Script::runScriptForVarEndVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], _vm->_state->getVar(cmd.args[2]), cmd.args[3], cmd.args[4]);
}

void Script::runScriptForVarStartEndVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), _vm->_state->getVar(cmd.args[2]), cmd.args[3], 0);
}

void Script::runScriptForVarStartEndVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), _vm->_state->getVar(cmd.args[2]), cmd.args[3], cmd.args[4]);
}

void Script::drawFramesForVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d, every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawFramesHelper(cmd.args[0], cmd.args[1], cmd.args[2], 0, -cmd.args[3]);
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

	runScriptForVarDrawFramesHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), _vm->_state->getVar(cmd.args[2]), 0, numFrames);
}

void Script::runScript(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run scripts from node %d", cmd.op, cmd.args[0]);

	uint16 node = _vm->_state->valueOrVarValue(cmd.args[0]);

	_vm->runScriptsFromNode(node, _vm->_state->getLocationRoom());
}

void Script::runScriptWithVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run scripts from node %d with var %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setVar(26, cmd.args[1]);
	uint16 node = _vm->_state->valueOrVarValue(cmd.args[0]);

	_vm->runScriptsFromNode(node, _vm->_state->getLocationRoom());
}

void Script::runCommonScript(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run common script %d", cmd.op, cmd.args[0]);

	_vm->runScriptsFromNode(cmd.args[0], 101, 1);
}

void Script::runCommonScriptWithVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run common script %d with var %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setVar(26, cmd.args[1]);

	_vm->runScriptsFromNode(cmd.args[0], 101, 1);
}

void Script::runPuzzle1(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run puzzle helper %d", cmd.op, cmd.args[0]);

	_puzzles->run(cmd.args[0]);
}

void Script::runPuzzle2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run puzzle helper %d", cmd.op, cmd.args[0]);

	_puzzles->run(cmd.args[0], cmd.args[1]);
}

void Script::runPuzzle3(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run puzzle helper %d", cmd.op, cmd.args[0]);

	_puzzles->run(cmd.args[0], cmd.args[1], cmd.args[2]);
}

void Script::runPuzzle4(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run puzzle helper %d", cmd.op, cmd.args[0]);

	_puzzles->run(cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);
}

void Script::ambientLoadNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Load ambient sounds from node %d %d %d", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	_vm->_ambient->loadNode(cmd.args[2], cmd.args[1], cmd.args[0]);
}

void Script::ambientReloadCurrentNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Reload ambient sounds from current node with fade out delay : %d", cmd.op, cmd.args[0]);

	_vm->_ambient->loadNode(0, 0, 0);
	_vm->_ambient->applySounds(_vm->_state->valueOrVarValue(cmd.args[0]));
}

void Script::ambientPlayCurrentNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play ambient sounds from current node %d %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_ambient->playCurrentNode(cmd.args[0], cmd.args[1]);
}

void Script::ambientApply(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Apply loadad ambient sounds", cmd.op);

	_vm->_ambient->applySounds(1);
}

void Script::ambientApplyWithFadeDelay(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Apply loadad ambient sounds with fade out delay : %d", cmd.op, cmd.args[0]);

	_vm->_ambient->applySounds(_vm->_state->valueOrVarValue(cmd.args[0]));
}

void Script::soundPlayBadClick(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play bad click sound", cmd.op);

	_vm->_sound->playEffect(697, 5);
}

void Script::soundPlayBlocking(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play skippable sound %d", cmd.op, cmd.args[0]);

	int16 soundId = cmd.args[0];
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 heading = _vm->_state->valueOrVarValue(cmd.args[2]);
	int32 att = _vm->_state->valueOrVarValue(cmd.args[3]);
	bool nonBlocking = _vm->_state->valueOrVarValue(cmd.args[4]);
	_vm->_sound->playEffect(soundId, volume, heading, att);

	if (nonBlocking || !_vm->_sound->isPlaying(soundId)) {
		return;
	}

	while (_vm->_sound->isPlaying(soundId) && !_vm->inputEscapePressed()) {
		_vm->processInput(true);
		_vm->drawFrame();
	}
}

void Script::soundPlay(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play sound %d", cmd.op, cmd.args[0]);

	_vm->_sound->playEffect(cmd.args[0], 100);
}

void Script::soundPlayVolume(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play sound %d at volume %d", cmd.op, cmd.args[0], cmd.args[1]);

	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	_vm->_sound->playEffect(cmd.args[0], volume);
}

void Script::soundPlayVolumeDirection(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play sound %d at volume %d in direction %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 heading = _vm->_state->valueOrVarValue(cmd.args[2]);
	_vm->_sound->playEffect(cmd.args[0], volume, heading, 85);
}

void Script::soundPlayVolumeDirectionAtt(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play sound %d at volume %d in direction %d with attenuation %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 heading = _vm->_state->valueOrVarValue(cmd.args[2]);
	int32 att = _vm->_state->valueOrVarValue(cmd.args[3]);
	_vm->_sound->playEffect(cmd.args[0], volume, heading, att);
}

void Script::soundStopEffect(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Stop sound effect %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);

	_vm->_sound->stopEffect(id, 0);
}

void Script::soundFadeOutEffect(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Stop sound effect %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 fadeDuration = _vm->_state->valueOrVarValue(cmd.args[1]);

	_vm->_sound->playEffect(id, fadeDuration);
}

void Script::soundPlayLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play sound effect looping %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);

	_vm->_sound->playEffectLooping(id, 100);
}

void Script::soundPlayFadeInOut(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Play sound effect fade in fade out %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 fadeInDuration = _vm->_state->valueOrVarValue(cmd.args[2]);

	int32 playDuration;
	if (cmd.args[3] == -1) {
		playDuration = 108000;
	} else {
		playDuration = _vm->_state->valueOrVarValue(cmd.args[3]);
	}

	int32 fadeOutDuration = _vm->_state->valueOrVarValue(cmd.args[4]);

	_vm->_sound->playEffectFadeInOut(id, volume, 0, 0, fadeInDuration, playDuration, fadeOutDuration);
}

void Script::soundChooseNext(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Setup next sound with control var %d", cmd.op, cmd.args[0]);

	int16 controlVar = cmd.args[0];
	int16 startSoundId = cmd.args[1];
	int16 soundCount = cmd.args[2];
	int32 soundMinDelay = _vm->_state->valueOrVarValue(cmd.args[3]);
	int32 soundMaxDelay = _vm->_state->valueOrVarValue(cmd.args[4]);

	_vm->_sound->setupNextSound(kNext, controlVar, startSoundId, soundCount, soundMinDelay, soundMaxDelay);
}

void Script::soundRandomizeNext(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Setup next sound with control var %d", cmd.op, cmd.args[0]);

	int16 controlVar = cmd.args[0];
	int16 startSoundId = cmd.args[1];
	int16 soundCount = cmd.args[2];
	int32 soundMinDelay = _vm->_state->valueOrVarValue(cmd.args[3]);
	int32 soundMaxDelay = _vm->_state->valueOrVarValue(cmd.args[4]);

	_vm->_sound->setupNextSound(kRandom, controlVar, startSoundId, soundCount, soundMinDelay, soundMaxDelay);
}

void Script::soundChooseNextAfterOther(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Setup next sound with control var %d", cmd.op, cmd.args[0]);

	int16 controlVar = cmd.args[0];
	int16 startSoundId = cmd.args[1];
	int16 soundCount = cmd.args[2];
	int32 soundMinDelay = _vm->_state->valueOrVarValue(cmd.args[3]);
	int32 soundMaxDelay = _vm->_state->valueOrVarValue(cmd.args[4]);

	int32 controlSoundId = _vm->_state->valueOrVarValue(cmd.args[5]);
	int32 controlSoundMaxPosition = _vm->_state->valueOrVarValue(cmd.args[6]);

	_vm->_sound->setupNextSound(kNextIfOtherStarting, controlVar, startSoundId, soundCount, soundMinDelay, soundMaxDelay, controlSoundId, controlSoundMaxPosition);
}

void Script::soundRandomizeNextAfterOther(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Setup next sound with control var %d", cmd.op, cmd.args[0]);

	int16 controlVar = cmd.args[0];
	int16 startSoundId = cmd.args[1];
	int16 soundCount = cmd.args[2];
	int32 soundMinDelay = _vm->_state->valueOrVarValue(cmd.args[3]);
	int32 soundMaxDelay = _vm->_state->valueOrVarValue(cmd.args[4]);

	int32 controlSoundId = _vm->_state->valueOrVarValue(cmd.args[5]);
	int32 controlSoundMaxPosition = _vm->_state->valueOrVarValue(cmd.args[6]);

	_vm->_sound->setupNextSound(kRandomIfOtherStarting, controlVar, startSoundId, soundCount, soundMinDelay, soundMaxDelay, controlSoundId, controlSoundMaxPosition);
}

void Script::ambientSetFadeOutDelay(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set fade out delay : %d", cmd.op, cmd.args[0]);

	_vm->_state->setAmbiantPreviousFadeOutDelay(cmd.args[0]);
}

void Script::ambientAddSound1(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add ambient sound %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);

	_vm->_ambient->addSound(id, volume, 0, 0, 0, 0);
}

void Script::ambientAddSound2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add ambient sound %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 fadeOutDelay = cmd.args[2];

	_vm->_ambient->addSound(id, volume, 0, 0, 0, fadeOutDelay);
}

void Script::ambientAddSound3(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add ambient sound %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 heading = cmd.args[2];

	_vm->_ambient->addSound(id, volume, heading, 85, 0, 0);
}

void Script::ambientAddSound4(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add ambient sound %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 heading = cmd.args[2];
	int32 angle = cmd.args[3];

	_vm->_ambient->addSound(id, volume, heading, angle, 0, 0);
}

void Script::ambientAddSound5(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add ambient sound %d", cmd.op, cmd.args[0]);

	int32 id = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 u1 = _vm->_state->valueOrVarValue(cmd.args[2]);

	_vm->_ambient->addSound(id, volume, 0, 0, u1, 0);
}

void Script::ambientSetCue1(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set ambient cue %d", cmd.op, cmd.args[0]);

	int32 id = cmd.args[0];
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);

	_vm->_ambient->setCueSheet(id, volume, 0, 0);
}

void Script::ambientSetCue2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set ambient cue %d", cmd.op, cmd.args[0]);

	int32 id = cmd.args[0];
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 heading = cmd.args[2];

	_vm->_ambient->setCueSheet(id, volume, heading, 85);
}

void Script::ambientSetCue3(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set ambient cue %d", cmd.op, cmd.args[0]);

	int32 id = cmd.args[0];
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 heading = cmd.args[2];
	int32 angle = cmd.args[3];

	_vm->_ambient->setCueSheet(id, volume, heading, angle);
}

void Script::ambientSetCue4(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set ambient cue %d", cmd.op, cmd.args[0]);

	int32 id = cmd.args[0];
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);

	_vm->_ambient->setCueSheet(id, volume, 32766, 85);
}

void Script::runAmbientScriptNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run ambient script for node %d",
			cmd.op, cmd.args[0]);

	int32 node = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->runAmbientScripts(node);
}

void Script::runAmbientScriptNodeRoomAge(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run sound script for node %d, room %d, age %d",
			cmd.op, cmd.args[2], cmd.args[1], cmd.args[0]);

	int32 node = _vm->_state->valueOrVarValue(cmd.args[2]);
	_vm->_ambient->_scriptRoom = _vm->_state->valueOrVarValue(cmd.args[1]);
	_vm->_ambient->_scriptAge = _vm->_state->valueOrVarValue(cmd.args[0]);

	_vm->runAmbientScripts(node);
	_vm->_ambient->scaleVolume(_vm->_state->valueOrVarValue(cmd.args[3]));
}

void Script::runSoundScriptNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run sound script for node %d",
			cmd.op, cmd.args[0]);

	int32 node = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->runBackgroundSoundScriptsFromNode(node);
}

void Script::runSoundScriptNodeRoom(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run sound script for node %d, room %d",
			cmd.op, cmd.args[1], cmd.args[0]);

	int32 node = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 room = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->runBackgroundSoundScriptsFromNode(node, room);
}

void Script::runSoundScriptNodeRoomAge(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run sound script for node %d, room %d, age %d",
			cmd.op, cmd.args[2], cmd.args[1], cmd.args[0]);

	int32 node = _vm->_state->valueOrVarValue(cmd.args[2]);
	int32 room = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 age = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->runBackgroundSoundScriptsFromNode(node, room, age);
}

void Script::soundStopMusic(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Stop music", cmd.op);

	int32 fadeOutDuration = _vm->_state->valueOrVarValue(cmd.args[0]);

	_vm->_sound->stopMusic(fadeOutDuration);
}

void Script::movieSetStartupSound(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set movie startup sound %d", cmd.op, cmd.args[0]);

	int32 soundId = _vm->_state->valueOrVarValue(cmd.args[0]);

	_vm->_state->setMovieStartSoundId(soundId);
	_vm->_state->setMovieStartSoundVolume(100);
	_vm->_state->setMovieStartSoundHeading(0);
	_vm->_state->setMovieStartSoundAttenuation(0);
}

void Script::movieSetStartupSoundVolume(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set movie startup sound %d", cmd.op, cmd.args[0]);

	int32 soundId = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);

	_vm->_state->setMovieStartSoundId(soundId);
	_vm->_state->setMovieStartSoundVolume(volume);
	_vm->_state->setMovieStartSoundHeading(0);
	_vm->_state->setMovieStartSoundAttenuation(0);
}

void Script::movieSetStartupSoundVolumeH(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set movie startup sound %d", cmd.op, cmd.args[0]);

	int32 soundId = _vm->_state->valueOrVarValue(cmd.args[0]);
	int32 volume = _vm->_state->valueOrVarValue(cmd.args[1]);
	int32 heading = _vm->_state->valueOrVarValue(cmd.args[2]);

	_vm->_state->setMovieStartSoundId(soundId);
	_vm->_state->setMovieStartSoundVolume(volume);
	_vm->_state->setMovieStartSoundHeading(heading);
	_vm->_state->setMovieStartSoundAttenuation(0);
}

void Script::drawOneFrame(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Draw one frame", cmd.op);

	_vm->processInput(true);
	_vm->drawFrame();
}

void Script::cursorHide(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Hide cursor", cmd.op);

	_vm->_cursor->setVisible(false);
}

void Script::cursorShow(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Show cursor", cmd.op);

	_vm->_cursor->setVisible(true);
}

void Script::cursorSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set cursor %d", cmd.op, cmd.args[0]);

	_vm->_cursor->changeCursor(cmd.args[0]);
}

void Script::cursorLock(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Lock cursor", cmd.op);

	_vm->_state->setCursorLocked(true);
}

void Script::cursorUnlock(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Unlock cursor", cmd.op);

	_vm->_state->setCursorLocked(false);
}

void Script::dialogOpen(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Open dialog %d", cmd.op, cmd.args[0]);

	uint16 dialog = _vm->_state->valueOrVarValue(cmd.args[0]);
	int16 result = _vm->openDialog(dialog);
	_vm->_state->setDialogResult(result + 1);
}

void Script::newGame(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: New game", cmd.op);

	_vm->_state->newGame();
	_vm->_inventory->reset();
}

} // End of namespace Myst3
