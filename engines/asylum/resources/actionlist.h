/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_ACTIONLIST_H
#define ASYLUM_ACTIONLIST_H

#include "asylum/views/scene.h"

#include "common/array.h"
#include "common/func.h"
#include "common/stack.h"
#include "common/stream.h"

namespace Asylum {

#define MAX_ACTION_COMMANDS 161

#define DECLARE_OPCODE(name) \
	void k##name(ScriptEntry *cmd)

#define IMPLEMENT_OPCODE(name) \
	void ActionList::k##name(ScriptEntry *cmd)

#define ADD_OPCODE(name) { \
	Opcode *func = new Opcode(#name, new Common::Functor1Mem<ScriptEntry *, void, ActionList>(this, &ActionList::k##name)); \
	_actions.push_back(func); \
}

#define GET_ACTOR() \
	Actor *actor = (cmd->param1 == -1) ? _scene->getActor() : &_scene->worldstats()->actors[cmd->param1];

class Scene;

class ActionList {
public:
	ActionList(Common::SeekableReadStream *stream, Scene *scene);
	virtual ~ActionList();

	/**
	 * Process the current script
	 */
	bool process();

	/**
	 * Initialize the script element at actionIndex to
	 * the actor at actorIndex
	 */
	void queueScript(int32 actionIndex, int32 actorIndex);

	// Accessors
	bool doesAllowInput() { return _allowInput; }
	int32 getDelayedVideoIndex() const { return _delayedVideoIndex; }
	void setDelayedVideoIndex(int32 val) { _delayedVideoIndex = val; }
	int32 getDelayedSceneIndex() const { return _delayedSceneIndex; }
	void setDelayedSceneIndex(int32 val) { _delayedSceneIndex = val; }

private:
	typedef struct ScriptEntry {
		int32 numLines; // Only set on the first line of each script
		int32 opcode;
		int32 param1;
		int32 param2;
		int32 param3;
		int32 param4;
		int32 param5;
		int32 param6;
		int32 param7;
		int32 param8;
		int32 param9;

	} ScriptEntry;

	typedef struct Script {
		ScriptEntry commands[MAX_ACTION_COMMANDS];
		int32       field_1BAC;
		int32       field_1BB0;
		int32       counter;
	} Script;

	typedef struct ScriptQueueEntry {
		int32 actionListIndex;
		int32 actorIndex;
	} ScriptQueueEntry;

	typedef Common::Functor1<ScriptEntry *, void> ActionFunctor;

	struct Opcode {
		const char *name;
		ActionFunctor *func;

		Opcode(const char* funcName, ActionFunctor *function) {
			name = funcName;
			func = function;
		}

		~Opcode() {
			delete func;
		}
	};

	Scene *_scene;

	// Script data
	Common::Array<Script> _entries;
	Common::Stack<ScriptQueueEntry> _scripts;
	Common::Array<Opcode *> _actions;

	bool              _skipProcessing;
	bool              _allowInput;
	int32             _currentLine;
	int32             _currentLoops;
	Script           *_currentScript;
	ScriptQueueEntry  _currentQueueEntry;
	int32             _delayedSceneIndex;
	int32             _delayedVideoIndex;
	bool              _done;
	int32             _lineIncrement;
	bool              _waitCycle;

	/**
	 * Loads the script entries
	 *
	 * @param stream the script data stream
	 */
	void load(Common::SeekableReadStream *stream);

	// Opcode functions
	DECLARE_OPCODE(Return);
	DECLARE_OPCODE(SetGameFlag);
	DECLARE_OPCODE(ClearGameFlag);
	DECLARE_OPCODE(ToggleGameFlag);
	DECLARE_OPCODE(JumpIfGameFlag);
	DECLARE_OPCODE(HideCursor);
	DECLARE_OPCODE(ShowCursor);
	DECLARE_OPCODE(PlayAnimation);
	DECLARE_OPCODE(MoveScenePosition);
	DECLARE_OPCODE(HideActor);
	DECLARE_OPCODE(ShowActor);
	DECLARE_OPCODE(SetActorPosition);
	DECLARE_OPCODE(SetSceneMotionStatus);
	DECLARE_OPCODE(DisableActor);
	DECLARE_OPCODE(EnableActor);
	DECLARE_OPCODE(EnableBarriers);
	DECLARE_OPCODE(DestroyBarrier);
	DECLARE_OPCODE(_unk12_JMP_WALK_ACTOR);
	DECLARE_OPCODE(_unk13_JMP_WALK_ACTOR);
	DECLARE_OPCODE(_unk14_JMP_WALK_ACTOR);
	DECLARE_OPCODE(_unk15);
	DECLARE_OPCODE(ResetAnimation);
	DECLARE_OPCODE(ClearFlag1Bit0);
	DECLARE_OPCODE(_unk18_PLAY_SND);
	DECLARE_OPCODE(JumpIfFlag2Bit0);
	DECLARE_OPCODE(SetFlag2Bit0);
	DECLARE_OPCODE(ClearFlag2Bit0);
	DECLARE_OPCODE(JumpIfFlag2Bit2);
	DECLARE_OPCODE(SetFlag2Bit2);
	DECLARE_OPCODE(ClearFlag2Bit2);
	DECLARE_OPCODE(JumpIfFlag2Bit1);
	DECLARE_OPCODE(SetFlag2Bit1);
	DECLARE_OPCODE(ClearFlag2Bit1);
	DECLARE_OPCODE(_unk22);
	DECLARE_OPCODE(_unk23);
	DECLARE_OPCODE(_unk24);
	DECLARE_OPCODE(RunEncounter);
	DECLARE_OPCODE(JumpIfFlag2Bit4);
	DECLARE_OPCODE(SetFlag2Bit4);
	DECLARE_OPCODE(ClearFlag2Bit4);
	DECLARE_OPCODE(SetActorField638);
	DECLARE_OPCODE(JumpIfActorField638);
	DECLARE_OPCODE(ChangeScene);
	DECLARE_OPCODE(_unk2C_ActorSub);
	DECLARE_OPCODE(PlayMovie);
	DECLARE_OPCODE(StopAllBarriersSounds);
	DECLARE_OPCODE(StopProcessing);
	DECLARE_OPCODE(ResumeProcessing);
	DECLARE_OPCODE(ResetSceneRect);
	DECLARE_OPCODE(ChangeMusicById);
	DECLARE_OPCODE(StopMusic);
	DECLARE_OPCODE(_unk34_Status);
	DECLARE_OPCODE(_unk35);
	DECLARE_OPCODE(_unk36);
	DECLARE_OPCODE(RunBlowUpPuzzle);
	DECLARE_OPCODE(JumpIfFlag2Bit3);
	DECLARE_OPCODE(SetFlag2Bit3);
	DECLARE_OPCODE(ClearFlag2Bit3);
	DECLARE_OPCODE(_unk3B_PALETTE_MOD);
	DECLARE_OPCODE(_unk3C_CMP_VAL);
	DECLARE_OPCODE(WaitUntilFramePlayed);
	DECLARE_OPCODE(UpdateWideScreen);
	DECLARE_OPCODE(_unk3F);
	DECLARE_OPCODE(_unk40_SOUND);
	DECLARE_OPCODE(PlaySpeech);
	DECLARE_OPCODE(_unk42);
	DECLARE_OPCODE(_unk43);
	DECLARE_OPCODE(PaletteFade);
	DECLARE_OPCODE(StartPaletteFadeThread);
	DECLARE_OPCODE(_unk46);
	DECLARE_OPCODE(ActorFaceObject);
	DECLARE_OPCODE(_unk48_MATTE_01);
	DECLARE_OPCODE(_unk49_MATTE_90);
	DECLARE_OPCODE(JumpIfSoundPlaying);
	DECLARE_OPCODE(ChangePlayerCharacterIndex);
	DECLARE_OPCODE(ChangeActorStatus);
	DECLARE_OPCODE(StopSound);
	DECLARE_OPCODE(_unk4E_RANDOM_COMMAND);
	DECLARE_OPCODE(ClearScreen);
	DECLARE_OPCODE(Quit);
	DECLARE_OPCODE(JumpBarrierFrame);
	DECLARE_OPCODE(_unk52);
	DECLARE_OPCODE(_unk53);
	DECLARE_OPCODE(_unk54_SET_ACTIONLIST_6EC);
	DECLARE_OPCODE(_unk55);
	DECLARE_OPCODE(_unk56);
	DECLARE_OPCODE(SetResourcePalette);
	DECLARE_OPCODE(SetBarrierFrameIdxFlaged);
	DECLARE_OPCODE(_unk59);
	DECLARE_OPCODE(_unk5A);
	DECLARE_OPCODE(_unk5B);
	DECLARE_OPCODE(_unk5C);
	DECLARE_OPCODE(_unk5D);
	DECLARE_OPCODE(ClearActorField970);
	DECLARE_OPCODE(SetBarrierLastFrameIdx);
	DECLARE_OPCODE(_unk60_SET_OR_CLR_ACTIONAREA_FLAG);
	DECLARE_OPCODE(_unk61);
	DECLARE_OPCODE(_unk62_SHOW_OPTIONS_SCREEN);
	DECLARE_OPCODE(_unk63);

}; // end of class ActionList

} // end of namespace Asylum

#endif // ASYLUM_ACTIONLIST_H
