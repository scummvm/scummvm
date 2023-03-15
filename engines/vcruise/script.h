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
	kStopAL,
	kRange,
	kAddXSound,
	kClrXSound,
	kStopSndLA,
	kStopSndLO,
	kMusic,
	kMusicUp,
	kMusicDn,
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
	kSay1,
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

	kAnimName,
	kValueName,
	kVarName,
	kSoundName,
	kCursorName,

	kCheckValue,	// Check if stack top is equal to arg.  If it is, pop the argument, otherwise leave it on the stack and skip the next instruction.
	kJump,			// Offset instruction index by arg.

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

struct ScreenScriptSet {
	Common::SharedPtr<Script> entryScript;
	ScriptMap_t interactionScripts;
};

struct RoomScriptSet {
	ScreenScriptSetMap_t screenScripts;
};

struct ScriptSet {
	RoomScriptSetMap_t roomScripts;
	Common::Array<Common::String> strings;
};

Common::SharedPtr<ScriptSet> compileLogicFile(Common::ReadStream &stream, uint streamSize, const Common::String &blamePath);

}

#endif
