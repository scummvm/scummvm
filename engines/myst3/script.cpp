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

#include "engines/myst3/script.h"

#include "engines/myst3/ambient.h"
#include "engines/myst3/cursor.h"
#include "engines/myst3/database.h"
#include "engines/myst3/hotspot.h"
#include "engines/myst3/inventory.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/puzzles.h"
#include "engines/myst3/scene.h"
#include "engines/myst3/sound.h"
#include "engines/myst3/state.h"

#include "common/events.h"

namespace Myst3 {

Script::Script(Myst3Engine *vm):
		_vm(vm) {
	_puzzles = new Puzzles(_vm);

#define OP(op, x, s) _commands.push_back(Command(op, &Script::x, #x, s))

	// TODO: Implement these remaining opcodes
	// 5: I'm pretty sure it's useless
	// 247: quit

	OP(  0, badOpcode,                                ""      );
	OP(  4, nodeCubeInit,                             "e"     );
	OP(  6, nodeCubeInitIndex,                        "veeee" );
	OP(  7, nodeFrameInit,                            "e"     );
	OP(  8, nodeFrameInitCond,                        "cee"   );
	OP(  9, nodeFrameInitIndex,                       "veeee" );
	OP( 10, nodeMenuInit,                             "e"     );
	OP( 11, stopWholeScript,                          ""      );
	OP( 13, spotItemAdd,                              "i"     );
	OP( 14, spotItemAddCond,                          "ic"    );
	OP( 15, spotItemAddCondFade,                      "ic"    );
	OP( 16, spotItemAddMenu,                          "iciii" ); // Six args
	OP( 17, movieInitLooping,                         "e"     );
	OP( 18, movieInitCondLooping,                     "ec"    );
	OP( 19, movieInitCond,                            "ec"    );
	OP( 20, movieInitPreloadLooping,                  "e"     );
	OP( 21, movieInitCondPreloadLooping,              "ec"    );
	OP( 22, movieInitCondPreload,                     "ec"    );
	OP( 23, movieInitFrameVar,                        "ev"    );
	OP( 24, movieInitFrameVarPreload,                 "ev"    );
	OP( 25, movieInitOverridePosition,                "ecii"  );
	OP( 26, movieInitScriptedPosition,                "evv"   );
	OP( 27, movieRemove,                              "e"     );
	OP( 28, movieRemoveAll,                           ""      );
	OP( 29, movieSetLooping,                          "i"     );
	OP( 30, movieSetNotLooping,                       "i"     );
	OP( 31, waterEffectSetSpeed,                      "i"     );
	OP( 32, waterEffectSetAttenuation,                "i"     );
	OP( 33, waterEffectSetWave,                       "ii"    );
	OP( 34, shakeEffectSet,                           "ee"    );
	OP( 35, sunspotAdd,                               "ii"    );
	OP( 36, sunspotAddIntensity,                      "iii"   );
	OP( 37, sunspotAddVarIntensity,                   "iiiv"  );
	OP( 38, sunspotAddIntensityColor,                 "iiii"  );
	OP( 39, sunspotAddVarIntensityColor,              "iiiiv" );
	OP( 40, sunspotAddIntensityRadius,                "iiii"  );
	OP( 41, sunspotAddVarIntensityRadius,             "iiivi" );
	OP( 42, sunspotAddIntColorRadius,                 "iiiii" );
	OP( 43, sunspotAddVarIntColorRadius,              "iiiiv" ); // Six args
	OP( 44, inventoryAddFront,                        "vi"    );
	OP( 45, inventoryAddBack,                         "vi"    );
	OP( 46, inventoryRemove,                          "v"     );
	OP( 47, inventoryReset,                           ""      );
	OP( 48, inventoryAddSaavChapter,                  "v"     );
	OP( 49, varSetZero,                               "v"     );
	OP( 50, varSetOne,                                "v"     );
	OP( 51, varSetTwo,                                "v"     );
	OP( 52, varSetOneHundred,                         "v"     );
	OP( 53, varSetValue,                              "vi"    );
	OP( 54, varToggle,                                "v"     );
	OP( 55, varSetOneIfNotZero,                       "v"     );
	OP( 56, varOpposite,                              "v"     );
	OP( 57, varAbsolute,                              "v"     );
	OP( 58, varDereference,                           "v"     );
	OP( 59, varReferenceSetZero,                      "v"     );
	OP( 60, varReferenceSetValue,                     "vi"    );
	OP( 61, varRandRange,                             "vii"   );
	OP( 62, polarToRectSimple,                        "vviii" ); // Seven args
	OP( 63, polarToRect,                              "vviii" ); // Ten args
	OP( 64, varSetDistanceToZone,                     "viii"  );
	OP( 65, varSetMinDistanceToZone,                  "viii"  );
	OP( 67, varRemoveBits,                            "vi"    );
	OP( 68, varToggleBits,                            "vi"    );
	OP( 69, varCopy,                                  "vv"    );
	OP( 70, varSetBitsFromVar,                        "vv"    );
	OP( 71, varSetBits,                               "vi"    );
	OP( 72, varApplyMask,                             "vi"    );
	OP( 73, varSwap,                                  "vv"    );
	OP( 74, varIncrement,                             "v"     );
	OP( 75, varIncrementMax,                          "vi"    );
	OP( 76, varIncrementMaxLooping,                   "vii"   );
	OP( 77, varAddValueMaxLooping,                    "ivii"  );
	OP( 78, varDecrement,                             "v"     );
	OP( 79, varDecrementMin,                          "vi"    );
	OP( 80, varAddValueMax,                           "ivi"   );
	OP( 81, varSubValueMin,                           "ivi"   );
	OP( 82, varZeroRange,                             "vv"    );
	OP( 83, varCopyRange,                             "vvi"   );
	OP( 84, varSetRange,                              "vvi"   );
	OP( 85, varIncrementMaxTen,                       "v"     );
	OP( 86, varAddValue,                              "iv"    );
	OP( 87, varArrayAddValue,                         "ivv"   );
	OP( 88, varAddVarValue,                           "vv"    );
	OP( 89, varSubValue,                              "iv"    );
	OP( 90, varSubVarValue,                           "vv"    );
	OP( 91, varModValue,                              "vi"    );
	OP( 92, varMultValue,                             "vi"    );
	OP( 93, varMultVarValue,                          "vv"    );
	OP( 94, varDivValue,                              "vi"    );
	OP( 95, varDivVarValue,                           "vv"    );
	OP( 96, varCrossMultiplication,                   "viiii" );
	OP( 97, varMinValue,                              "vi"    );
	OP( 98, varClipValue,                             "vii"   );
	OP( 99, varClipChangeBound,                       "vii"   );
	OP(100, varAbsoluteSubValue,                      "vi"    );
	OP(101, varAbsoluteSubVar,                        "vv"    );
	OP(102, varRatioToPercents,                       "vii"   );
	OP(103, varRotateValue3,                          "viii"  );
	OP(104, ifElse,                                   ""      );
	OP(105, ifCondition,                              "c"     );
	OP(106, ifCond1AndCond2,                          "cc"    );
	OP(107, ifCond1OrCond2,                           "cc"    );
	OP(108, ifOneVarSetInRange,                       "vv"    );
	OP(109, ifVarEqualsValue,                         "vi"    );
	OP(110, ifVarNotEqualsValue,                      "vi"    );
	OP(111, ifVar1EqualsVar2,                         "vv"    );
	OP(112, ifVar1NotEqualsVar2,                      "vv"    );
	OP(113, ifVarSupEqValue,                          "vi"    );
	OP(114, ifVarInfEqValue,                          "vi"    );
	OP(115, ifVarInRange,                             "vii"   );
	OP(116, ifVarNotInRange,                          "vii"   );
	OP(117, ifVar1SupEqVar2,                          "vv"    );
	OP(118, ifVar1SupVar2,                            "vv"    );
	OP(119, ifVar1InfEqVar2,                          "vv"    );
	OP(120, ifVarHasAllBitsSet,                       "vi"    );
	OP(121, ifVarHasNoBitsSet,                        "vi"    );
	OP(122, ifVarHasSomeBitsSet,                      "vii"   );
	OP(123, ifHeadingInRange,                         "ii"    );
	OP(124, ifPitchInRange,                           "ii"    );
	OP(125, ifHeadingPitchInRect,                     "iiii"  );
	OP(126, ifMouseIsInRect,                          "iiii"  );
	OP(127, leverDrag,                                "iiiiv" ); // Six args
	OP(130, leverDragXY,                              "vviii" );
	OP(131, itemDrag,                                 "viiiv" );
	OP(132, leverDragPositions,                       "vi"    ); // Variable args
	OP(134, runScriptWhileDragging,                   "vviiv" ); // Eight args
	OP(135, chooseNextNode,                           "cii"   );
	OP(136, goToNodeTransition,                       "ii"    );
	OP(137, goToNodeTrans2,                           "i"     );
	OP(138, goToNodeTrans1,                           "i"     );
	OP(139, goToRoomNode,                             "ii"    );
	OP(140, zipToNode,                                "i"     );
	OP(141, zipToRoomNode,                            "ii"    );
	OP(144, drawTransition,                           ""      );
	OP(145, reloadNode,                               ""      );
	OP(146, redrawFrame,                              ""      );
	OP(147, moviePlay,                                "e"     );
	OP(148, moviePlaySynchronized,                    "e"     );
	OP(149, moviePlayFullFrame,                       "e"     );
	OP(150, moviePlayFullFrameTrans,                  "e"     );
	OP(151, moviePlayChangeNode,                      "ee"    );
	OP(152, moviePlayChangeNodeTrans,                 "ee"    );
	OP(153, lookAt,                                   "ii"    );
	OP(154, lookAtInXFrames,                          "iii"   );
	OP(155, lookAtMovieStart,                         "e"     );
	OP(156, lookAtMovieStartInXFrames,                "ei"    );
	OP(157, cameraLimitMovement,                      "iiii"  );
	OP(158, cameraFreeMovement,                       ""      );
	OP(159, cameraLookAt,                             "ii"    );
	OP(160, cameraLookAtVar,                          "v"     );
	OP(161, cameraGetLookAt,                          "v"     );
	OP(162, lookAtMovieStartImmediate,                "e"     );
	OP(163, cameraSetFOV,                             "e"     );
	OP(164, changeNode,                               "i"     );
	OP(165, changeNodeRoom,                           "ii"    );
	OP(166, changeNodeRoomAge,                        "iii"   );
	OP(168, uselessOpcode,                            ""      );
	OP(169, drawXTicks,                               "i"     );
	OP(171, drawWhileCond,                            "c"     );
	OP(172, whileStart,                               "c"     );
	OP(173, whileEnd,                                 ""      );
	OP(174, runScriptWhileCond,                       "ci"    );
	OP(175, runScriptWhileCondEachXFrames,            "cii"   );
	OP(176, runScriptForVar,                          "viii"  );
	OP(177, runScriptForVarEachXFrames,               "viiii" );
	OP(178, runScriptForVarStartVar,                  "vvii"  );
	OP(179, runScriptForVarStartVarEachXFrames,       "vviii" );
	OP(180, runScriptForVarEndVar,                    "vivi"  );
	OP(181, runScriptForVarEndVarEachXFrames,         "vivii" );
	OP(182, runScriptForVarStartEndVar,               "vvvi"  );
	OP(183, runScriptForVarStartEndVarEachXFrames,    "vvvii" );
	OP(184, drawFramesForVar,                         "viii"  );
	OP(185, drawFramesForVarEachTwoFrames,            "vii"   );
	OP(186, drawFramesForVarStartEndVarEachTwoFrames, "vvv"   );
	OP(187, runScript,                                "e"     );
	OP(188, runScriptWithVar,                         "ei"    );
	OP(189, runCommonScript,                          "i"     );
	OP(190, runCommonScriptWithVar,                   "ei"    );
	OP(194, runPuzzle1,                               "i"     );
	OP(195, runPuzzle2,                               "ii"    );
	OP(196, runPuzzle3,                               "iii"   );
	OP(197, runPuzzle4,                               "iiii"  );
	OP(198, ambientLoadNode,                          "iii"   );
	OP(199, ambientReloadCurrentNode,                 "e"     );
	OP(200, ambientPlayCurrentNode,                   "ii"    );
	OP(201, ambientApply,                             ""      );
	OP(202, ambientApplyWithFadeDelay,                "e"     );
	OP(203, soundPlayBadClick,                        ""      );
	OP(204, soundPlayBlocking,                        "eeeei" );
	OP(205, soundPlay,                                "e"     );
	OP(206, soundPlayVolume,                          "ee"    );
	OP(207, soundPlayVolumeDirection,                 "eee"   );
	OP(208, soundPlayVolumeDirectionAtt,              "eeee"  );
	OP(209, soundStopEffect,                          "e"     );
	OP(210, soundFadeOutEffect,                       "ee"    );
	OP(212, soundPlayLooping,                         "e"     );
	OP(213, soundPlayFadeInOut,                       "eeeee" );
	OP(214, soundChooseNext,                          "viiee" );
	OP(215, soundRandomizeNext,                       "viiee" );
	OP(216, soundChooseNextAfterOther,                "viiee" ); // Seven args
	OP(217, soundRandomizeNextAfterOther,             "viiee" ); // Seven args
	OP(218, ambientSetFadeOutDelay,                   "i"     );
	OP(219, ambientAddSound1,                         "ee"    );
	OP(220, ambientAddSound2,                         "eei"   );
	OP(222, ambientAddSound3,                         "eei"   );
	OP(223, ambientAddSound4,                         "eeii"  );
	OP(224, ambientAddSound5,                         "eee"   );
	OP(225, ambientSetCue1,                           "ie"    );
	OP(226, ambientSetCue2,                           "iei"   );
	OP(227, ambientSetCue3,                           "ieii"  );
	OP(228, ambientSetCue4,                           "ie"    );
	OP(229, runAmbientScriptNode,                     "e"     );
	OP(230, runAmbientScriptNodeRoomAge,              "eeee"  );
	OP(231, runSoundScriptNode,                       "e"     );
	OP(232, runSoundScriptNodeRoom,                   "ee"    );
	OP(233, runSoundScriptNodeRoomAge,                "eee"   );
	OP(234, soundStopMusic,                           "e"     );
	OP(235, movieSetStartupSound,                     "e"     );
	OP(236, movieSetStartupSoundVolume,               "ee"    );
	OP(237, movieSetStartupSoundVolumeH,              "eee"   );
	OP(239, drawOneFrame,                             ""      );
	OP(240, cursorHide,                               ""      );
	OP(241, cursorShow,                               ""      );
	OP(242, cursorSet,                                "i"     );
	OP(243, cursorLock,                               ""      );
	OP(244, cursorUnlock,                             ""      );
	OP(248, dialogOpen,                               "e"     );
	OP(249, newGame,                                  ""      );

	if (_vm->getPlatform() == Common::kPlatformXbox) {
		// The Xbox version inserted two new opcodes, one at position
		// 27, the other at position 77, shifting all the other opcodes
		shiftCommands(77, 1);
		OP(77, varDecrementMinLooping,	              "vii"   );

		shiftCommands(27, 1);
		OP(27, movieInitCondScriptedPosition,         "ecvv"  );
	}

#undef OP
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

	while (c.op != script->end() && !_vm->shouldQuit()) {
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
		if (cmd.op != 0 && k < strlen(cmd.signature))
			d += describeArgument(cmd.signature[k], opcode.args[k]) + " ";
		else
			d += Common::String::format("%d ", opcode.args[k]);
	}

	d += ")\n";

	return d;
}

const Common::String Script::describeArgument(char type, int16 value) {
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

void Script::movieInitOverridePosition(Context &c, const Opcode &cmd) {
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

void Script::movieInitCondScriptedPosition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Preload movie %d with condition %d, position U-var %d V-var %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	_vm->_state->setMoviePreloadToMemory(true);
	_vm->_state->setMovieScriptDriven(true);
	_vm->_state->setMovieUVar(cmd.args[2]);
	_vm->_state->setMovieVVar(cmd.args[3]);

	uint16 movieid = _vm->_state->valueOrVarValue(cmd.args[0]);
	_vm->loadMovie(movieid, cmd.args[1], false, true);
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
	_vm->_state->setShakeEffectTickPeriod(period);
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
	float angleRad = 2 * (float)M_PI / cmd.args[6] * angleDeg;
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
	float angleRad = 2 * (float)M_PI / cmd.args[9] * angleDeg;
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
	if (distance >= _vm->_state->getVar(cmd.args[0]))
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

void Script::varDecrementMinLooping(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Decrement var %d in range [%d, %d]",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	int32 value = _vm->_state->getVar(cmd.args[0]);

	value--;

	if (value < cmd.args[1])
		value = cmd.args[2];

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

	if (var > end) {
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

	Common::Point mouse = _vm->_cursor->getPosition(false);
	mouse = _vm->_scene->scalePoint(mouse);

	if (r.contains(mouse))
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

	_vm->_cursor->changeCursor(2);

	int16 previousPosition = -1;
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
			Common::Point mouse = _vm->_cursor->getPosition(false);
			mouse = _vm->_scene->scalePoint(mouse);
			int16 amplitude;
			int16 pixelPosition;

			if (minPosX == maxPosX) {
				// Vertical slider
				amplitude = maxPosY - minPosY;
				pixelPosition = mouse.y - minPosY;
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
		_vm->processInput(false);
		_vm->drawFrame();

		bool mousePressed = (_vm->getEventManager()->getButtonState() & Common::EventManager::LBUTTON) != 0;
		_vm->_state->setDragEnded(!mousePressed);

		if (_vm->_state->getDragLeverSpeed()) {
			debugC(kDebugScript, "Interaction with var 58 is missing in opcode 127.");
			return;
		}

		if (script && (position != previousPosition || !mousePressed)) {
			_vm->_state->setVar(var, position);
			_vm->runScriptsFromNode(abs(script));
		}

		if (script > 0) {
			// In this case the script is executed only if the lever position changed.
			// Otherwise it is executed every frame
			previousPosition = position;
		}

		if (!mousePressed || _vm->shouldQuit())
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

	int16 previousPosition = -1;
	while (true) {
		float pitch, heading;
		_vm->_cursor->getDirection(pitch, heading);

		float minDistance = 180.0;
		int16 position = 0;

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
		_vm->processInput(false);
		_vm->drawFrame();

		bool mousePressed = _vm->inputValidatePressed();
		_vm->_state->setDragEnded(!mousePressed);

		if (_vm->_state->getDragLeverSpeed()) {
			debugC(kDebugScript, "Interaction with var 58 is missing in opcode 132.");
			return;
		}

		if (script && (position != previousPosition || !mousePressed)) {
			_vm->_state->setVar(var, position);
			_vm->runScriptsFromNode(abs(script));
		}

		if (script > 0) {
			// In this case the script is executed only if the lever position changed.
			// Otherwise it is executed every frame
			previousPosition = position;
		}

		if (!mousePressed || _vm->shouldQuit())
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

	Common::Point mouseInit = _vm->_cursor->getPosition(false);
	mouseInit = _vm->_scene->scalePoint(mouseInit);

	_vm->_cursor->changeCursor(2);

	bool mousePressed = true;
	do {
		Common::Point mouse = _vm->_cursor->getPosition(false);
		mouse = _vm->_scene->scalePoint(mouse);
		int16 distanceX = (mouseInit.x - mouse.x) / scale;
		int16 distanceY = (mouseInit.y - mouse.y) / scale;

		distanceX = CLIP<int16>(distanceX, -maxLeverPosition, maxLeverPosition);
		distanceY = CLIP<int16>(distanceY, -maxLeverPosition, maxLeverPosition);

		// Set lever position variables
		_vm->_state->setVar(varX, distanceX);
		_vm->_state->setVar(varY, distanceY);

		// Draw a frame
		_vm->processInput(false);
		_vm->drawFrame();

		mousePressed = _vm->getEventManager()->getButtonState() & Common::EventManager::LBUTTON;
		_vm->_state->setDragEnded(!mousePressed);

		// Run script
		if (script)
			_vm->runScriptsFromNode(script);
	} while (mousePressed && !_vm->shouldQuit());
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

	_vm->_cursor->changeCursor(2);

	bool dragWithDirectionKeys = _vm->_state->hasVarDragWithDirectionKeys()
			&& _vm->_state->getDragWithDirectionKeys();

	bool dragging = true;
	do {
		dragging = _vm->getEventManager()->getButtonState() & Common::EventManager::LBUTTON;
		dragging |= _vm->_state->hasVarGamePadActionPressed() && _vm->_state->getGamePadActionPressed();
		_vm->_state->setDragEnded(!dragging);

		_vm->processInput(false);
		_vm->drawFrame();

		if (!dragWithDirectionKeys) {
			// Distance between the mouse and the lever
			Common::Point mouse = _vm->_cursor->getPosition(false);
			mouse = _vm->_scene->scalePoint(mouse);
			int16 distanceX = mouse.x - leverWidth / 2 - _vm->_state->getVar(cmd.args[0]);
			int16 distanceY = mouse.y - leverHeight / 2 - _vm->_state->getVar(cmd.args[1]);
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

					mouse = _vm->_cursor->getPosition(false);
					mouse = _vm->_scene->scalePoint(mouse);
					distanceX = mouse.x - leverWidth / 2 - _vm->_state->getVar(cmd.args[0]);
					distanceY = mouse.y - leverHeight / 2 - _vm->_state->getVar(cmd.args[1]);
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
		} else {
			uint16 previousPosition = _vm->_state->getVar(cmd.args[4]);
			uint16 position = previousPosition;

			if (_vm->_state->getGamePadLeftPressed()) {
				position--;
			} else if (_vm->_state->getGamePadRightPressed()) {
				position++;
			}

			position = CLIP<int16>(position, 0, maxLeverPosition);
			_vm->_state->setVar(cmd.args[4], position);
			_vm->_state->setDragLeverPositionChanged(position != previousPosition);
		}

		_vm->runScriptsFromNode(script);
		_vm->processInput(false);
		_vm->drawFrame();
	} while (dragging && !_vm->shouldQuit());

	if (dragWithDirectionKeys) {
		_vm->_state->setDragWithDirectionKeys(false);
	}

	_vm->_state->setDragPositionFound(false);
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

void Script::drawXTicks(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Draw %d ticks", cmd.op, cmd.args[0]);

	uint32 endTick = _vm->_state->getTickCount() + cmd.args[0];

	while (_vm->_state->getTickCount() < endTick && !_vm->shouldQuit()) {
		_vm->processInput(false);
		_vm->drawFrame();
	}
}

void Script::drawWhileCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: While condition %d, draw", cmd.op, cmd.args[0]);

	while (_vm->_state->evaluate(cmd.args[0]) && !_vm->inputEscapePressed() && !_vm->shouldQuit()) {
		_vm->processInput(false);
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

	_vm->processInput(false);
	_vm->drawFrame();
}

void Script::whileEnd(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: End of while condition", cmd.op);

	// Go to while start
	c.op = c.whileStart;
}

void Script::runScriptWhileCond(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: While condition %d, run script %d", cmd.op, cmd.args[0], cmd.args[1]);

	while (_vm->_state->evaluate(cmd.args[0]) && !_vm->shouldQuit()) {
		_vm->runScriptsFromNode(cmd.args[1]);
		_vm->processInput(false);
		_vm->drawFrame();
	}

	_vm->processInput(false);
	_vm->drawFrame();
}

void Script::runScriptWhileCondEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: While condition %d, run script %d each %d frames", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint step = cmd.args[2] % 100;

	uint firstStep = cmd.args[2];
	if (firstStep > 100)
		firstStep /= 100;

	uint nextScript = _vm->_state->getTickCount() + firstStep;

	while (_vm->_state->evaluate(cmd.args[0]) && !_vm->shouldQuit()) {

		if (_vm->_state->getTickCount() >= nextScript) {
			nextScript = _vm->_state->getTickCount() + step;

			_vm->runScriptsFromNode(cmd.args[1]);
		}

		_vm->processInput(false);
		_vm->drawFrame();
	}

	_vm->processInput(false);
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

void Script::runScriptForVarDrawTicksHelper(uint16 var, int32 startValue, int32 endValue, uint16 script, int32 numTicks) {
	if (numTicks < 0) {
		numTicks = -numTicks;
		uint startTick = _vm->_state->getTickCount();
		uint currentTick = startTick;
		uint endTick = startTick + numTicks;
		uint numValues = abs(endValue - startValue);

		if (startTick < endTick) {
			int currentValue = -9999;
			while (1) {
				int nextValue = numValues * (currentTick - startTick) / numTicks;
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

				_vm->processInput(false);
				_vm->drawFrame();
				currentTick = _vm->_state->getTickCount();

				if (currentTick > endTick)
					break;
			}
		}

		_vm->_state->setVar(var, endValue);
	} else {
		int currentValue = startValue;
		uint endTick = 0;

		bool positiveDirection = endValue > startValue;

		while (1) {
			if ((positiveDirection && (currentValue > endValue))
					|| (!positiveDirection && (currentValue < endValue)))
				break;

			_vm->_state->setVar(var, currentValue);

			if (script)
				_vm->runScriptsFromNode(script);

			for (uint i = _vm->_state->getTickCount(); i < endTick; i = _vm->_state->getTickCount()) {
				_vm->processInput(false);
				_vm->drawFrame();
			}

			endTick = _vm->_state->getTickCount() + numTicks;

			currentValue += positiveDirection ? 1 : -1;
		}
	}
}

void Script::runScriptForVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawTicksHelper(cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], 0);
}

void Script::runScriptForVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawTicksHelper(cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);
}

void Script::runScriptForVarStartVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to %d, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawTicksHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), cmd.args[2], cmd.args[3], 0);
}

void Script::runScriptForVarStartVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to %d, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawTicksHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), cmd.args[2], cmd.args[3], cmd.args[4]);
}

void Script::runScriptForVarEndVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to var %d value, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawTicksHelper(cmd.args[0], cmd.args[1], _vm->_state->getVar(cmd.args[2]), cmd.args[3], 0);
}

void Script::runScriptForVarEndVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawTicksHelper(cmd.args[0], cmd.args[1], _vm->_state->getVar(cmd.args[2]), cmd.args[3], cmd.args[4]);
}

void Script::runScriptForVarStartEndVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawTicksHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), _vm->_state->getVar(cmd.args[2]),
	                               cmd.args[3], 0);
}

void Script::runScriptForVarStartEndVarEachXFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value, run script %d every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3], cmd.args[4]);

	runScriptForVarDrawTicksHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), _vm->_state->getVar(cmd.args[2]),
	                               cmd.args[3], cmd.args[4]);
}

void Script::drawFramesForVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d, every %d frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	runScriptForVarDrawTicksHelper(cmd.args[0], cmd.args[1], cmd.args[2], 0, -cmd.args[3]);
}

void Script::drawFramesForVarEachTwoFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from %d to %d draw 2 frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint numFrames = 2 * (-1 - abs(cmd.args[2] - cmd.args[1]));

	runScriptForVarDrawTicksHelper(cmd.args[0], cmd.args[1], cmd.args[2], 0, numFrames);
}

void Script::drawFramesForVarStartEndVarEachTwoFrames(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: For var %d from var %d value to var %d value draw 2 frames",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint numFrames = 2 * (-1 - abs(cmd.args[2] - cmd.args[1]));

	runScriptForVarDrawTicksHelper(cmd.args[0], _vm->_state->getVar(cmd.args[1]), _vm->_state->getVar(cmd.args[2]), 0,
	                               numFrames);
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

	_vm->runScriptsFromNode(cmd.args[0], kRoomShared, 1);
}

void Script::runCommonScriptWithVar(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run common script %d with var %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_state->setVar(26, cmd.args[1]);

	_vm->runScriptsFromNode(cmd.args[0], kRoomShared, 1);
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

	while (_vm->_sound->isPlaying(soundId) && !_vm->inputEscapePressed() && !_vm->shouldQuit()) {
		_vm->processInput(false);
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

	_vm->_sound->stopEffect(id, fadeDuration);
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

	_vm->processInput(false);
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
	_vm->_state->setDialogResult(result);
}

void Script::newGame(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: New game", cmd.op);

	_vm->_state->newGame();
	_vm->_inventory->reset();
}

} // End of namespace Myst3
