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

#ifndef ASYLUM_SCRIPT_H
#define ASYLUM_SCRIPT_H

#include "asylum/shared.h"

#include "common/array.h"
#include "common/func.h"
#include "common/stack.h"
#include "common/stream.h"

namespace Asylum {

#define MAX_ACTION_COMMANDS 161

#define DECLARE_OPCODE(name) \
	void k##name(ScriptEntry *cmd)

#define IMPLEMENT_OPCODE(name) \
	void ScriptManager::k##name(ScriptEntry *cmd) { \
	if (!_currentScript) error("[" #name "] No current script set!"); \
	if (!cmd) error("[" #name "] Invalid command parameter!");

#define END_OPCODE }


#define ADD_OPCODE(name) { \
	Opcode *func = new Opcode(#name, new Common::Functor1Mem<ScriptEntry *, void, ScriptManager>(this, &ScriptManager::k##name)); \
	_opcodes.push_back(func); \
}

class Actor;
class AsylumEngine;
class Scene;

struct ActionArea {
	char  name[52];
	int32 id;
	int32 field01;
	int32 field02;
	int32 field_40;
	int32 field_44;
	int32 flags;
	int32 scriptIndex;
	int32 scriptIndex2;
	int32 actionType;    ///< flag (see ActionType enumeration)
	int32 flagNums[10];
	int32 field_7C;
	int32 polyIdx;
	int32 field_84;
	int32 field_88;
	ResourceId soundResourceId;
	int32 field_90;
	ResourceId paletteResourceId;
	int32 array[5];
	int32 volume;

	ActionArea() {
		memset(&name, 0, sizeof(name));
		id = 0;
		field01 = 0;
		field02 = 0;
		field_40 = 0;
		field_44 = 0;
		flags = 0;
		scriptIndex = 0;
		scriptIndex2 = 0;
		actionType = 0;
		memset(&flagNums, 0, sizeof(flagNums));
		field_7C = 0;
		polyIdx = 0;
		field_84 = 0;
		field_88 = 0;
		soundResourceId = kResourceNone;
		field_90 = 0;
		paletteResourceId = kResourceNone;
		memset(&array, 0, sizeof(array));
		volume = 0;
	}

	void load(Common::SeekableReadStream *stream);

	Common::String toString() {
		Common::String output;

		output += Common::String::format("Action %d: %s\n", id, name);
		output += Common::String::format("           flags=%d      scriptIndex=%d      scriptIndex2=%d   type=%d\n", flags, scriptIndex, actionType, polyIdx);
		output += Common::String::format("           sound=%d      polygon=%d          palette=%d        volume=%d\n", soundResourceId, scriptIndex2, paletteResourceId, volume);
		output += Common::String::format("           field01=%d    field02=%d          field40=%d        field44=%d\n", field01, field02, field_40, field_44);
		output += Common::String::format("           field7C=%d    field84=%d          field88=%d        field90=%d\n", field_7C, field_84, field_88, field_90);

		return output;
	}
};

class ScriptManager {
public:
	ScriptManager(AsylumEngine *engine);
	virtual ~ScriptManager();

	/**
	 * Loads the script entries
	 *
	 * @param stream the script data stream
	 */
	void load(Common::SeekableReadStream *stream);

	/**
	 * Process the current script
	 */
	bool process();

	/**
	 * Resets the queue and local variables
	 */
	void reset();

	/**
	 * Initialize the script element at actionIndex to
	 * the actor at actorIndex
	 */
	void queueScript(int32 scriptIndex, ActorIndex actorIndex);

	/**
	 * Query if 'scriptIndex' is in queue. 
	 *
	 * @param scriptIndex Zero-based index of the script. 
	 *
	 * @return true if in queue, false if not. 
	 */
	bool isInQueue(int32 scriptIndex);

	// Accessors
	int32 getDelayedVideoIndex() const { return _delayedVideoIndex; }
	void setDelayedVideoIndex(int32 val) { _delayedVideoIndex = val; }
	ResourcePackId getDelayedSceneIndex() const { return _delayedSceneIndex; }
	void setDelayedSceneIndex(ResourcePackId id) { _delayedSceneIndex = id; }

	bool isProcessingSkipped() { return _skipProcessing; }

private:
	enum ObjectEnableType {
		kObjectEnableType0,
		kObjectEnableType1,
		kObjectEnableType2
	};

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

		ScriptEntry() {
			numLines = 0;
			opcode = 0;
			param1 = 0;
			param2 = 0;
			param3 = 0;
			param4 = 0;
			param5 = 0;
			param6 = 0;
			param7 = 0;
			param8 = 0;
			param9 = 0;
		}
	} ScriptEntry;

	typedef struct Script {
		ScriptEntry commands[MAX_ACTION_COMMANDS];
		int32       field_1BAC;
		int32       field_1BB0;
		int32       counter;

		Script() {
			field_1BAC = 0;
			field_1BB0 = 0;
			counter = 0;
		}
	} Script;

	typedef struct ScriptQueueEntry {
		int32 scriptIndex;
		ActorIndex actorIndex;
	} ScriptQueueEntry;

	// Opcodes
	typedef Common::Functor1<ScriptEntry *, void> OpcodeFunctor;

	struct Opcode {
		const char    *name;
		OpcodeFunctor *func;

		Opcode(const char* opcodeName, OpcodeFunctor *functor) {
			name = opcodeName;
			func = functor;
		}

		~Opcode() {
			delete func;
		}
	};

	// Engine
	AsylumEngine *_vm;

	// Script queue and data
	Common::Array<Opcode *>         _opcodes;
	Common::Stack<ScriptQueueEntry> _queue;
	Common::Array<Script>           _scripts;

	bool              _skipProcessing;
	int32             _currentLine;
	int32             _currentLoops;
	Script           *_currentScript;
	ScriptQueueEntry  _currentQueueEntry;
	ResourcePackId    _delayedSceneIndex;
	int32             _delayedVideoIndex;
	bool              _done;
	bool              _exit;
	int32             _lineIncrement;
	bool              _waitCycle;

	/**
	 * Resets the queue.
	 */
	void resetQueue();

	// Opcode helper functions
	void enableObject(ScriptEntry *cmd, ObjectEnableType type);
	void setActionFlag(ScriptEntry *cmd, ActionType flag); //|
	void clearActionFlag(ScriptEntry *cmd, ActionType flag); //&
	void jumpIfActionFlag(ScriptEntry *cmd, ActionType flag);
	void setNextLine(int32 line);

	//////////////////////////////////////////////////////////////////////////
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
	DECLARE_OPCODE(EnableObjects);
	DECLARE_OPCODE(RemoveObject);
	DECLARE_OPCODE(JumpActorSpeech);
	DECLARE_OPCODE(JumpAndSetDirection);
	DECLARE_OPCODE(JumpIfActorCoordinates);
	DECLARE_OPCODE(Nop);
	DECLARE_OPCODE(ResetAnimation);
	DECLARE_OPCODE(DisableObject);
	DECLARE_OPCODE(JumpIfSoundPlayingAndPlaySound);
	DECLARE_OPCODE(JumpIfActionFind);
	DECLARE_OPCODE(SetActionFind);
	DECLARE_OPCODE(ClearActionFind);
	DECLARE_OPCODE(JumpIfActionGrab);
	DECLARE_OPCODE(SetActionGrab);
	DECLARE_OPCODE(ClearActionGrab);
	DECLARE_OPCODE(JumpIfActionTalk);
	DECLARE_OPCODE(SetActionTalk);
	DECLARE_OPCODE(ClearActionTalk);
	DECLARE_OPCODE(_unk22);
	DECLARE_OPCODE(_unk23);
	DECLARE_OPCODE(_unk24);
	DECLARE_OPCODE(RunEncounter);
	DECLARE_OPCODE(JumpIfAction16);
	DECLARE_OPCODE(SetAction16);
	DECLARE_OPCODE(ClearAction16);
	DECLARE_OPCODE(SetActorField638);
	DECLARE_OPCODE(JumpIfActorField638);
	DECLARE_OPCODE(ChangeScene);
	DECLARE_OPCODE(_unk2C_ActorSub);
	DECLARE_OPCODE(PlayMovie);
	DECLARE_OPCODE(StopAllObjectsSounds);
	DECLARE_OPCODE(StopProcessing);
	DECLARE_OPCODE(ResumeProcessing);
	DECLARE_OPCODE(ResetSceneRect);
	DECLARE_OPCODE(ChangeMusicById);
	DECLARE_OPCODE(StopMusic);
	DECLARE_OPCODE(_unk34_Status);
	DECLARE_OPCODE(SetVolume);
	DECLARE_OPCODE(Jump);
	DECLARE_OPCODE(RunBlowUpPuzzle);
	DECLARE_OPCODE(JumpIfAction8);
	DECLARE_OPCODE(SetAction8);
	DECLARE_OPCODE(ClearAction8);
	DECLARE_OPCODE(_unk3B_PALETTE_MOD);
	DECLARE_OPCODE(IncrementParam2);
	DECLARE_OPCODE(WaitUntilFramePlayed);
	DECLARE_OPCODE(UpdateWideScreen);
	DECLARE_OPCODE(JumpIfActor);
	DECLARE_OPCODE(PlaySpeechScene);
	DECLARE_OPCODE(PlaySpeech);
	DECLARE_OPCODE(PlaySpeechScene2);
	DECLARE_OPCODE(MoveScenePositionFromActor);
	DECLARE_OPCODE(PaletteFade);
	DECLARE_OPCODE(StartPaletteFadeThread);
	DECLARE_OPCODE(_unk46);
	DECLARE_OPCODE(ActorFaceObject);
	DECLARE_OPCODE(_unk48_MATTE_01);
	DECLARE_OPCODE(_unk49_MATTE_90);
	DECLARE_OPCODE(JumpIfSoundPlaying);
	DECLARE_OPCODE(ChangePlayerActorIndex);
	DECLARE_OPCODE(ChangeActorStatus);
	DECLARE_OPCODE(StopSound);
	DECLARE_OPCODE(JumpRandom);
	DECLARE_OPCODE(ClearScreen);
	DECLARE_OPCODE(Quit);
	DECLARE_OPCODE(JumpObjectFrame);
	DECLARE_OPCODE(DeleteGraphics);
	DECLARE_OPCODE(SetActorField944);
	DECLARE_OPCODE(_unk54_SET_ACTIONLIST_6EC);
	DECLARE_OPCODE(_unk55);
	DECLARE_OPCODE(_unk56);
	DECLARE_OPCODE(SetResourcePalette);
	DECLARE_OPCODE(SetObjectFrameIdxFlaged);
	DECLARE_OPCODE(_unk59);
	DECLARE_OPCODE(_unk5A);
	DECLARE_OPCODE(_unk5B);
	DECLARE_OPCODE(QueueScript);
	DECLARE_OPCODE(_unk5D);
	DECLARE_OPCODE(ClearActorFields);
	DECLARE_OPCODE(SetObjectLastFrameIdx);
	DECLARE_OPCODE(_unk60_SET_OR_CLR_ACTIONAREA_FLAG);
	DECLARE_OPCODE(_unk61);
	DECLARE_OPCODE(ShowOptionsScreen);
	DECLARE_OPCODE(_unk63);

	friend class Console;
}; // end of class ActionList

} // end of namespace Asylum

#endif // ASYLUM_SCRIPT_H
