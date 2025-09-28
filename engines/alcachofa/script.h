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

#ifndef ALCACHOFA_SCRIPT_H
#define ALCACHOFA_SCRIPT_H

#include "alcachofa/common.h"

#include "common/hashmap.h"
#include "common/span.h"
#include "common/stream.h"
#include "common/system.h"

namespace Alcachofa {

class Process;

// the ScriptOp and ScriptKernelTask enums represent the *implemented* order
// the specific Game instance maps the version-specific op codes to our order

enum class ScriptOp {
	Nop,
	Dup,
	PushAddr,
	PushDynAddr,
	PushValue,
	Deref,
	Pop1,
	PopN,
	Store,
	LoadString,
	ScriptCall,
	KernelCall,
	JumpIfFalse,
	JumpIfTrue,
	Jump,
	Negate,
	BooleanNot,
	Mul,
	Add,
	Sub,
	Less,
	Greater,
	LessEquals,
	GreaterEquals,
	Equals,
	NotEquals,
	BitAnd,
	BitOr,
	ReturnValue,
	ReturnVoid,
	Crash
};

enum class ScriptKernelTask {
	Nop = 0,
	PlayVideo,
	PlaySound,
	PlayMusic,
	StopMusic,
	WaitForMusicToEnd,
	ShowCenterBottomText,
	StopAndTurn,
	StopAndTurnMe,
	ChangeCharacter,
	SayText,
	Go,
	Put,
	ChangeCharacterRoom,
	KillProcesses,
	LerpCharacterLodBias,
	On,
	Off,
	Pickup,
	CharacterPickup,
	Drop,
	CharacterDrop,
	Delay,
	HadNoMousePressFor,
	Fork,
	Animate,
	AnimateCharacter,
	AnimateTalking,
	ChangeRoom,
	ToggleRoomFloor,
	SetDialogLineReturn,
	DialogMenu,
	ClearInventory,
	FadeType0,
	FadeType1,
	LerpWorldLodBias,
	FadeType2,
	SetActiveTextureSet,
	SetMaxCamSpeedFactor,
	WaitCamStopping,
	CamFollow,
	CamShake,
	LerpCamXY,
	LerpCamZ,
	LerpCamScale,
	LerpCamToObjectWithScale,
	LerpCamToObjectResettingZ,
	LerpCamRotation,
	FadeIn,
	FadeOut,
	FadeIn2,
	FadeOut2,
	LerpCamXYZ,
	LerpCamToObjectKeepingZ,

	SheriffTakesCharacter, ///< some special-case V1 tasks, unknown yet
	ChangeDoor,
	Disguise
};

enum class ScriptFlags {
	None = 0,
	AllowMissing = (1 << 0),
	IsBackground = (1 << 1)
};
inline ScriptFlags operator | (ScriptFlags a, ScriptFlags b) {
	return (ScriptFlags)(((uint)a) | ((uint)b));
}
inline bool operator & (ScriptFlags a, ScriptFlags b) {
	return ((uint)a) & ((uint)b);
}

struct ScriptInstruction {
	ScriptInstruction(Common::ReadStream &stream);

	int32 _op; ///< int32 because it still has to be mapped using a game-specific translation table
	int32 _arg;
};

class Script {
public:
	Script();

	void syncGame(Common::Serializer &s);
	int32 variable(const char *name) const;
	int32 &variable(const char *name);
	Process *createProcess(
		MainCharacterKind character,
		const Common::String &procedure,
		ScriptFlags flags = ScriptFlags::None);
	Process *createProcess(
		MainCharacterKind character,
		const Common::String &behavior,
		const Common::String &action,
		ScriptFlags flags = ScriptFlags::None);
	bool hasProcedure(const Common::String &behavior, const Common::String &action) const;
	bool hasProcedure(const Common::String &procedure) const;

	using VariableNameIterator = Common::HashMap<Common::String, uint32>::const_iterator;
	inline VariableNameIterator beginVariables() const { return _variableNames.begin(); }
	inline VariableNameIterator endVariables() const { return _variableNames.end(); }
	inline bool hasVariable(const char *name) const { return _variableNames.contains(name); }

	void setScriptTimer(bool reset);
private:
	friend struct ScriptTask;
	friend struct ScriptTimerTask;
	Common::HashMap<Common::String, uint32> _variableNames;
	Common::HashMap<Common::String, uint32> _procedures;
	Common::Array<ScriptInstruction> _instructions;
	Common::Array<int32> _variables;
	Common::SpanOwner<Common::Span<char>> _strings;
	uint32 _scriptTimer = 0;
};

}

#endif // ALCACHOFA_SCRIPT_H
