/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VCRUISE_SCRIPT_H
#define VCRUISE_SCRIPT_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/types.h"

namespace Common {

class ReadStream;

} // End of namespace Common

namespace VCruise {

struct ScreenScriptSet;
struct RoomScriptSet;
struct ScriptSet;
struct ITextPreprocessor;

namespace ScriptOps {

enum ScriptOp {
	kInvalid,

	kNumber,

	kRotate,
	kAngle,
	kAngleGGet,
	kSpeed,
	kSAnimL,
	kChangeL,
	kAnimR,
	kAnimF,
	kAnimN,
	kAnimG,
	kAnimS,
	kAnim,
	kStatic,
	kVarLoad,
	kVarStore,
	kVarAddAndStore,
	kVarGlobalLoad,
	kVarGlobalStore,
	kItemCheck,
	kItemHaveSpace,
	kItemRemove,
	kItemHighlightSet,
	kItemAdd,
	kItemClear,
	kSetCursor,
	kSetRoom,
	kLMB,
	kLMB1,
	kSoundS1,
	kSoundS2,
	kSoundS3,
	kSoundL1,
	kSoundL2,
	kSoundL3,
	k3DSoundS2,
	k3DSoundL2,
	k3DSoundL3,
	kStopAL,
	kRange,
	kAddXSound,
	kClrXSound,
	kStopSndLA,
	kStopSndLO,
	kMusic,
	kMusicVolRamp,
	kParm0,
	kParm1,
	kParm2,
	kParm3,
	kParmG,
	kSParmX,
	kSAnimX,
	kVolumeDn2,
	kVolumeDn3,
	kVolumeDn4,
	kVolumeUp3,
	kRandom,
	kDrop,
	kDup,
	kSwap,
	kSay1,
	kSay2,
	kSay3,
	kSay3Get,
	kSetTimer,
	kGetTimer,
	kDelay,
	kLoSet,
	kLoGet,
	kHiSet,
	kHiGet,

	kNot,
	kAnd,
	kOr,
	kSub,
	kAdd,
	kNegate,
	kCmpEq,
	kCmpLt,
	kCmpGt,

	kBitLoad,
	kBitSet0,
	kBitSet1,

	kDisc1,
	kDisc2,
	kDisc3,

	kGoto,

	kEscOn,
	kEscOff,
	kEscGet,
	kBackStart,
	kSaveAs,
	kSave0,
	kExit,

	kAnimName,
	kValueName,
	kVarName,
	kSoundName,
	kCursorName,
	kDubbing,

	kCheckValue,	// Check if stack top is equal to arg.  If it is, pop the argument, otherwise leave it on the stack and skip the next instruction.
	kJump,			// Offset instruction index by arg.

	// Schizm ops
	kCallFunction,
	kMusicStop,
	kMusicPlayScore,
	kScoreAlways,
	kScoreNormal,
	kSndPlay,
	kSndPlayEx,
	kSndPlay3D,
	kSndPlaying,
	kSndWait,
	kSndHalt,
	kSndToBack,
	kSndStop,
	kSndStopAll,
	kSndAddRandom,
	kSndClearRandom,
	kVolumeAdd,
	kVolumeChange,
	kAnimVolume,
	kAnimChange,
	kScreenName,
	kExtractByte,
	kInsertByte,
	kString,
	kCmpNE,
	kCmpLE,
	kCmpGE,
	kReturn,
	kSpeech,
	kSpeechEx,
	kSpeechTest,
	kSay,
	kRandomInclusive,
	kHeroOut,
	kHeroGetPos,
	kHeroSetPos,
	kHeroGet,
	kGarbage,
	kGetRoom,
	kBitAnd,
	kBitOr,
	kAngleGet,
	kIsCDVersion,
	kIsDVDVersion,
	kDisc,
	kHidePanel,
	kRotateUpdate,
	kMul,
	kDiv,
	kMod,
	kCyfraGet,	// Cyfra = digit?
	kPuzzleInit,
	kPuzzleCanPress,
	kPuzzleDoMove1,
	kPuzzleDoMove2,
	kPuzzleDone,
	kPuzzleWhoWon,
	kFn,
	kItemHighlightSetTrue,

	kNumOps,
};

} // End of namespace ScriptOps

struct Instruction {
	Instruction();
	explicit Instruction(ScriptOps::ScriptOp paramOp);
	Instruction(ScriptOps::ScriptOp paramOp, int32 paramArg);

	ScriptOps::ScriptOp op;
	int32 arg;
};

struct Script {
	Common::Array<Instruction> instrs;
};

typedef Common::HashMap<uint, Common::SharedPtr<Script> > ScriptMap_t;
typedef Common::HashMap<uint, Common::SharedPtr<ScreenScriptSet> > ScreenScriptSetMap_t;
typedef Common::HashMap<uint, Common::SharedPtr<RoomScriptSet> > RoomScriptSetMap_t;
typedef Common::HashMap<Common::String, uint> ScreenNameMap_t;

struct ScreenScriptSet {
	Common::SharedPtr<Script> entryScript;
	ScriptMap_t interactionScripts;
};

struct RoomScriptSet {
	ScreenScriptSetMap_t screenScripts;
	ScreenNameMap_t screenNames;
};

struct ScriptSet {
	ScriptSet();

	RoomScriptSetMap_t roomScripts;

	Common::Array<Common::SharedPtr<Script> > functions;
	Common::Array<Common::String> functionNames;
	Common::Array<Common::String> strings;
};

struct FunctionDef {
	Common::String fnName;
	Common::SharedPtr<Script> func;
};

// Global state is required for Schizm because its preprocessor defines exist across files.
// For example, volPortWaves is set in Room01 but used in Room03 and Room20
struct IScriptCompilerGlobalState {
	virtual ~IScriptCompilerGlobalState();

	virtual void define(const Common::String &key, uint roomNumber, int32 value) = 0;
	virtual bool getDefine(const Common::String &str, uint &outRoomNumber, int32 &outValue) const = 0;

	virtual uint getFunctionIndex(const Common::String &fnName) = 0;
	virtual void setFunction(uint fnIndex, const Common::SharedPtr<Script> &fn) = 0;

	virtual uint getNumFunctions() const = 0;
	virtual void dumpFunctionNames(Common::Array<Common::String> &fnNames) const = 0;
	virtual Common::SharedPtr<Script> getFunction(uint fnIndex) const = 0;
};

Common::SharedPtr<IScriptCompilerGlobalState> createScriptCompilerGlobalState();
Common::SharedPtr<ScriptSet> compileReahLogicFile(Common::ReadStream &stream, uint streamSize, const Common::String &blamePath);
void compileSchizmLogicFile(ScriptSet &scriptSet, uint loadAsRoom, uint fileRoom, Common::ReadStream &stream, uint streamSize, const Common::String &blamePath, IScriptCompilerGlobalState *gs);
bool checkSchizmLogicForDuplicatedRoom(Common::ReadStream &stream, uint streamSize);

}

#endif
