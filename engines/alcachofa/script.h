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

#ifndef SCRIPT_H
#define SCRIPT_H

#include "common.h"

#include "common/hashmap.h"
#include "common/span.h"
#include "common/stream.h"
#include "common/system.h"

namespace Alcachofa {

class Process;

enum class ScriptOp {
	Nop,
	Dup,
	PushAddr,
	PushValue,
	Deref,
	Crash5, ///< would crash original engine by writing to read-only memory
	PopN,
	Store,
	Crash8,
	Crash9,
	LoadString,
	LoadString2, ///< exactly the same as LoadString
	Crash12,
	ScriptCall,
	KernelCall,
	JumpIfFalse,
	JumpIfTrue,
	Jump,
	Negate,
	BooleanNot,
	Mul,
	Crash21,
	Crash22,
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
	Crash33,
	Crash34,
	Crash35,
	Crash36,
	Return
};

enum class ScriptKernelTask {
	PlayVideo = 1,
	PlaySound,
	PlayMusic,
	StopMusic,
	WaitForMusicToEnd,
	ShowCenterBottomText,
	StopAndTurn,
	StopAndTurnMe,
	ChangeCharacter,
	SayText,
	Nop10,
	Go,
	Put,
	ChangeCharacterRoom,
	KillProcesses,
	LerpLodBias,
	On,
	Off,
	Pickup,
	CharacterPickup,
	Drop,
	CharacterDrop,
	Delay,
	HadNoMousePressFor,
	Nop24,
	Fork,
	Animate,
	AnimateCharacter,
	AnimateTalking,
	ChangeRoom,
	ToggleRoomFloor,
	SetDialogLineReturn,
	DialogMenu,
	ClearInventory,
	Nop34,
	FadeType0,
	FadeType1,
	SetLodBias,
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
	LerpCamToObjectKeepingZ
};

struct ScriptInstruction {
	ScriptInstruction(Common::ReadStream &stream);

	ScriptOp _op;
	int32 _arg;
};

class Script {
public:
	Script();

	int32 variable(const char *name) const;
	int32 &variable(const char *name);
	Process *createProcess(
		MainCharacterKind character,
		const Common::String &procedure,
		bool allowMissing = false);
	Process *createProcess(
		MainCharacterKind character,
		const Common::String &behavior,
		const Common::String &action,
		bool allowMissing = false);

private:
	friend struct ScriptTask;
	Common::HashMap<Common::String, uint32> _variableNames;
	Common::HashMap<Common::String, uint32> _procedures;
	Common::Array<ScriptInstruction> _instructions;
	Common::Array<int32> _variables;
	Common::SpanOwner<Common::Span<char>> _strings;
};

}

#endif // SCRIPT_H
