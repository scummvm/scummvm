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
	void runSingleOp(const Opcode &op);

	const Common::String describeOpcode(const Opcode &opcode);

private:
	struct Context {
		bool endScript;
		bool result;
		const Common::Array<Opcode> *script;
		Common::Array<Opcode>::const_iterator op;
		Common::Array<Opcode>::const_iterator whileStart;
	};

	typedef void (Script::*CommandProc)(Context &c, const Opcode &cmd);

	enum ArgumentType {
		kUnknown   = 'u',
		kVar       = 'v',
		kValue     = 'i',
		kEvalValue = 'e',
		kCondition = 'c'
	};

	struct Command {
		Command() : op(0), proc(nullptr), desc(nullptr), signature(nullptr)  { }
		Command(uint16 o, CommandProc p, const char *d, const char *s) : op(o), proc(p), desc(d), signature(s) { }

		uint16 op;
		CommandProc proc;
		const char *desc;
		const char *signature;
	};

	Myst3Engine *_vm;
	Puzzles *_puzzles;

	Common::Array<Command> _commands;

	const Command &findCommand(uint16 op);
	const Command &findCommandByProc(CommandProc proc);
	const Common::String describeCommand(uint16 op);
	const Common::String describeArgument(char type, int16 value);

	void shiftCommands(uint16 base, int32 value);

	void runOp(Context &c, const Opcode &op);
	void goToElse(Context &c);

	void runScriptForVarDrawTicksHelper(uint16 var, int32 startValue, int32 endValue, uint16 script, int32 numTicks);

	DECLARE_OPCODE(badOpcode);
	DECLARE_OPCODE(uselessOpcode);

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
	DECLARE_OPCODE(spotItemAddMenu);
	DECLARE_OPCODE(movieInitLooping);
	DECLARE_OPCODE(movieInitCondLooping);
	DECLARE_OPCODE(movieInitCond);
	DECLARE_OPCODE(movieInitPreloadLooping);
	DECLARE_OPCODE(movieInitCondPreloadLooping);
	DECLARE_OPCODE(movieInitCondPreload);
	DECLARE_OPCODE(movieInitFrameVar);
	DECLARE_OPCODE(movieInitFrameVarPreload);
	DECLARE_OPCODE(movieInitOverridePosition);
	DECLARE_OPCODE(movieInitScriptedPosition);
	DECLARE_OPCODE(movieInitCondScriptedPosition);
	DECLARE_OPCODE(movieRemove);
	DECLARE_OPCODE(movieRemoveAll);
	DECLARE_OPCODE(movieSetLooping);
	DECLARE_OPCODE(movieSetNotLooping);
	DECLARE_OPCODE(waterEffectSetSpeed);
	DECLARE_OPCODE(waterEffectSetAttenuation);
	DECLARE_OPCODE(waterEffectSetWave);
	DECLARE_OPCODE(shakeEffectSet);
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
	DECLARE_OPCODE(inventoryAddSaavChapter);
	DECLARE_OPCODE(varSetZero);
	DECLARE_OPCODE(varSetOne);
	DECLARE_OPCODE(varSetTwo);
	DECLARE_OPCODE(varSetOneHundred);
	DECLARE_OPCODE(varSetValue);
	DECLARE_OPCODE(varToggle);
	DECLARE_OPCODE(varSetOneIfNotZero);
	DECLARE_OPCODE(varOpposite);
	DECLARE_OPCODE(varAbsolute);
	DECLARE_OPCODE(varDereference);
	DECLARE_OPCODE(varReferenceSetZero);
	DECLARE_OPCODE(varReferenceSetValue);
	DECLARE_OPCODE(varRandRange);
	DECLARE_OPCODE(polarToRectSimple);
	DECLARE_OPCODE(polarToRect);
	DECLARE_OPCODE(varSetDistanceToZone);
	DECLARE_OPCODE(varSetMinDistanceToZone);
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
	DECLARE_OPCODE(varDecrementMinLooping);
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
	DECLARE_OPCODE(varCrossMultiplication);
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
	DECLARE_OPCODE(ifHeadingInRange);
	DECLARE_OPCODE(ifPitchInRange);
	DECLARE_OPCODE(ifHeadingPitchInRect);
	DECLARE_OPCODE(ifMouseIsInRect);
	DECLARE_OPCODE(leverDrag);
	DECLARE_OPCODE(leverDragPositions);
	DECLARE_OPCODE(leverDragXY);
	DECLARE_OPCODE(itemDrag);
	DECLARE_OPCODE(runScriptWhileDragging);
	DECLARE_OPCODE(chooseNextNode);
	DECLARE_OPCODE(goToNodeTransition);
	DECLARE_OPCODE(goToNodeTrans2);
	DECLARE_OPCODE(goToNodeTrans1);
	DECLARE_OPCODE(goToRoomNode);
	DECLARE_OPCODE(zipToNode);
	DECLARE_OPCODE(zipToRoomNode);
	DECLARE_OPCODE(drawTransition);
	DECLARE_OPCODE(reloadNode);
	DECLARE_OPCODE(redrawFrame);
	DECLARE_OPCODE(moviePlay);
	DECLARE_OPCODE(moviePlaySynchronized);
	DECLARE_OPCODE(moviePlayFullFrame);
	DECLARE_OPCODE(moviePlayFullFrameTrans);
	DECLARE_OPCODE(moviePlayChangeNode);
	DECLARE_OPCODE(moviePlayChangeNodeTrans);
	DECLARE_OPCODE(lookAt);
	DECLARE_OPCODE(lookAtInXFrames);
	DECLARE_OPCODE(lookAtMovieStart);
	DECLARE_OPCODE(lookAtMovieStartInXFrames);
	DECLARE_OPCODE(cameraLimitMovement);
	DECLARE_OPCODE(cameraFreeMovement);
	DECLARE_OPCODE(cameraLookAt);
	DECLARE_OPCODE(cameraLookAtVar);
	DECLARE_OPCODE(cameraGetLookAt);
	DECLARE_OPCODE(lookAtMovieStartImmediate);
	DECLARE_OPCODE(cameraSetFOV);
	DECLARE_OPCODE(changeNode);
	DECLARE_OPCODE(changeNodeRoom);
	DECLARE_OPCODE(changeNodeRoomAge);

	DECLARE_OPCODE(drawXTicks);
	DECLARE_OPCODE(drawWhileCond);
	DECLARE_OPCODE(whileStart);
	DECLARE_OPCODE(whileEnd);
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
	DECLARE_OPCODE(runScriptWithVar);
	DECLARE_OPCODE(runCommonScript);
	DECLARE_OPCODE(runCommonScriptWithVar);
	DECLARE_OPCODE(runPuzzle1);
	DECLARE_OPCODE(runPuzzle2);
	DECLARE_OPCODE(runPuzzle3);
	DECLARE_OPCODE(runPuzzle4);
	DECLARE_OPCODE(ambientLoadNode);
	DECLARE_OPCODE(ambientReloadCurrentNode);
	DECLARE_OPCODE(ambientPlayCurrentNode);
	DECLARE_OPCODE(ambientApply);
	DECLARE_OPCODE(ambientApplyWithFadeDelay);
	DECLARE_OPCODE(soundPlayBadClick);
	DECLARE_OPCODE(soundPlayBlocking);
	DECLARE_OPCODE(soundPlay);
	DECLARE_OPCODE(soundPlayVolume);
	DECLARE_OPCODE(soundPlayVolumeDirection);
	DECLARE_OPCODE(soundPlayVolumeDirectionAtt);
	DECLARE_OPCODE(soundStopEffect);
	DECLARE_OPCODE(soundFadeOutEffect);
	DECLARE_OPCODE(soundPlayLooping);
	DECLARE_OPCODE(soundPlayFadeInOut);
	DECLARE_OPCODE(soundChooseNext);
	DECLARE_OPCODE(soundRandomizeNext);
	DECLARE_OPCODE(soundChooseNextAfterOther);
	DECLARE_OPCODE(soundRandomizeNextAfterOther);
	DECLARE_OPCODE(ambientSetFadeOutDelay);
	DECLARE_OPCODE(ambientAddSound1);
	DECLARE_OPCODE(ambientAddSound2);
	DECLARE_OPCODE(ambientAddSound3);
	DECLARE_OPCODE(ambientAddSound4);
	DECLARE_OPCODE(ambientAddSound5);
	DECLARE_OPCODE(ambientSetCue1);
	DECLARE_OPCODE(ambientSetCue2);
	DECLARE_OPCODE(ambientSetCue3);
	DECLARE_OPCODE(ambientSetCue4);
	DECLARE_OPCODE(runAmbientScriptNode);
	DECLARE_OPCODE(runAmbientScriptNodeRoomAge);
	DECLARE_OPCODE(runSoundScriptNode);
	DECLARE_OPCODE(runSoundScriptNodeRoom);
	DECLARE_OPCODE(runSoundScriptNodeRoomAge);
	DECLARE_OPCODE(soundStopMusic);
	DECLARE_OPCODE(movieSetStartupSound);
	DECLARE_OPCODE(movieSetStartupSoundVolume);
	DECLARE_OPCODE(movieSetStartupSoundVolumeH);
	DECLARE_OPCODE(drawOneFrame);
	DECLARE_OPCODE(cursorHide);
	DECLARE_OPCODE(cursorShow);
	DECLARE_OPCODE(cursorSet);
	DECLARE_OPCODE(cursorLock);
	DECLARE_OPCODE(cursorUnlock);
	DECLARE_OPCODE(dialogOpen);
	DECLARE_OPCODE(newGame);
};

} // End of namespace Myst3

#endif // SCRIPT_H_
