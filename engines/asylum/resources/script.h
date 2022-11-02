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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_RESOURCES_SCRIPT_H
#define ASYLUM_RESOURCES_SCRIPT_H

#include "common/array.h"
#include "common/func.h"
#include "common/serializer.h"
#include "common/stack.h"
#include "common/stream.h"

#include "asylum/shared.h"

namespace Asylum {

#define MAX_ACTION_COMMANDS 161

#define DECLARE_OPCODE(name) \
	void Op##name(ScriptEntry *cmd)

#define IMPLEMENT_OPCODE(name) \
	void ScriptManager::Op##name(ScriptEntry *cmd) { \
	if (!_currentScript) error("[" #name "] No current script set"); \
	if (!_currentQueueEntry) error("[" #name "] Invalid current queue entry"); \
	if (!cmd) error("[" #name "] Invalid command parameter");

#define END_OPCODE }


#define ADD_OPCODE(name) { \
	Opcode *func = new Opcode(#name, new Common::Functor1Mem<ScriptEntry *, void, ScriptManager>(this, &ScriptManager::Op##name)); \
	_opcodes.push_back(func); \
}

class Actor;
class AsylumEngine;
class Scene;

struct ActionArea : public Common::Serializable {
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
	uint32 polygonIndex;
	ResourceId soundResourceIdFrame;
	int32 field_88;
	ResourceId soundResourceId;
	int32 field_90;
	ResourceId paletteResourceId;
	int32 paths[5];
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
		polygonIndex = 0;
		soundResourceIdFrame = kResourceNone;
		field_88 = 0;
		soundResourceId = kResourceNone;
		field_90 = 0;
		paletteResourceId = kResourceNone;
		memset(&paths, 0, sizeof(paths));
		volume = 0;
	}

	void load(Common::SeekableReadStream *stream);

	Common::String toString() {
		Common::String output;

		output += Common::String::format("Action %d: %s\n", id, name);
		output += Common::String::format("           flags=%d      scriptIndex=%d      scriptIndex2=%d   type=%d\n", flags, scriptIndex, scriptIndex2, actionType);
		output += Common::String::format("           sound=%d      polygon=%d          palette=%d        volume=%d\n", soundResourceId, polygonIndex, paletteResourceId, volume);
		output += Common::String::format("           field01=%d    field02=%d          field40=%d        field44=%d\n", field01, field02, field_40, field_44);
		output += Common::String::format("           field7C=%d    field84=%d          field88=%d        field90=%d\n", field_7C, soundResourceIdFrame, field_88, field_90);

		return output;
	}

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);
};

class ScriptManager : public Common::Serializable {
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
	 * Fully resets script manager state (used while changing scenes)
	 */
	void resetAll();

	/**
	 * Resets the queue and local variables
	 */
	void reset(uint32 count = 0);

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
	bool isInQueue(int32 scriptIndex) const;

	/**
	 * Remove a script element from the queue
	 */
	void removeFromQueue(uint32 entryIndex);

	/**
	 * Resets the queue.
	 */
	void resetQueue();

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);
	void saveQueue(Common::Serializer &s);

private:
	enum ObjectTransparency {
		kObjectEnableType0,
		kObjectEnableType1,
		kObjectTransparencyOpaque
	};

	//////////////////////////////////////////////////////////////////////////
	// Script Queue
	//////////////////////////////////////////////////////////////////////////
	struct ScriptQueueEntry : public Common::Serializable {
		int32 scriptIndex;
		int32 currentLine;
		ActorIndex actorIndex;
		uint32 next;
		uint32 prev;

		ScriptQueueEntry() {
			reset();
		}

		void reset() {
			scriptIndex = -1;
			currentLine = 0;
			actorIndex = 0;
			next = 0;
			prev = 0;
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsSint32LE(scriptIndex);
			s.syncAsSint32LE(currentLine);
			s.syncAsUint32LE(actorIndex);
			s.syncAsUint32LE(next);
			s.syncAsUint32LE(prev);
		}
	};

	struct ScriptQueue : public Common::Serializable {
		ScriptQueueEntry entries[10];
		uint32 first;
		uint32 last;

		ScriptQueue() {
			reset();
		}

		void reset() {
			for (uint32 i = 0; i < ARRAYSIZE(entries); i++)
				entries[i].reset();

			first = 0;
			last = 0;
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			for (uint32 i = 0; i < ARRAYSIZE(entries); i++)
				entries[i].saveLoadWithSerializer(s);

			s.syncAsUint32LE(first);
			s.syncAsUint32LE(last);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// Scripts
	//////////////////////////////////////////////////////////////////////////
	struct ScriptEntry : public Common::Serializable {
		int32 numLines; // Only set on the first line of each script
		OpcodeType opcode;
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
			opcode = kOpcodeReturn;
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

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsSint32LE(numLines);
			s.syncAsSint32LE(opcode);
			s.syncAsSint32LE(param1);
			s.syncAsSint32LE(param2);
			s.syncAsSint32LE(param3);
			s.syncAsSint32LE(param4);
			s.syncAsSint32LE(param5);
			s.syncAsSint32LE(param6);
			s.syncAsSint32LE(param7);
			s.syncAsSint32LE(param8);
			s.syncAsSint32LE(param9);
		}
	};

	struct Script : public Common::Serializable {
		ScriptEntry commands[MAX_ACTION_COMMANDS];
		int32       field_1BAC;
		int32       field_1BB0;
		int32       counter;

		Script() {
			field_1BAC = 0;
			field_1BB0 = 0;
			counter = 0;
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			for (int32 i = 0; i < ARRAYSIZE(commands); i++)
				commands[i].saveLoadWithSerializer(s);

			s.syncAsSint32LE(field_1BAC);
			s.syncAsSint32LE(field_1BB0);
			s.syncAsSint32LE(counter);
		}
	};

	//////////////////////////////////////////////////////////////////////////
	// Opcodes
	//////////////////////////////////////////////////////////////////////////
	typedef Common::Functor1<ScriptEntry *, void> OpcodeFunctor;

	struct Opcode {
		const char    *name;
		OpcodeFunctor *func;

		Opcode(const char *opcodeName, OpcodeFunctor *functor) {
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
	ScriptQueue _queue;
	Common::Array<Opcode *>         _opcodes;
	Common::Array<Script>           _scripts;

	bool              _done;
	bool              _exit;
	bool              _processNextEntry;
	ScriptEntry		 *_lastProcessedCmd; // DEBUGGING
	Script           *_currentScript;
	ScriptQueueEntry *_currentQueueEntry;

	// Opcode helper functions
	void enableObject(ScriptEntry *cmd, ObjectTransparency type);
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
	DECLARE_OPCODE(AddToInventory);
	DECLARE_OPCODE(RemoveFromInventory);
	DECLARE_OPCODE(JumpIfInventoryOmits);
	DECLARE_OPCODE(RunEncounter);
	DECLARE_OPCODE(JumpIfAction16);
	DECLARE_OPCODE(SetAction16);
	DECLARE_OPCODE(ClearAction16);
	DECLARE_OPCODE(SelectInventoryItem);
	DECLARE_OPCODE(JumpIfInventoryItemNotSelected);
	DECLARE_OPCODE(ChangeScene);
	DECLARE_OPCODE(Interact);
	DECLARE_OPCODE(PlayMovie);
	DECLARE_OPCODE(StopAllObjectsSounds);
	DECLARE_OPCODE(StopProcessing);
	DECLARE_OPCODE(ResumeProcessing);
	DECLARE_OPCODE(ResetSceneRect);
	DECLARE_OPCODE(ChangeMusicById);
	DECLARE_OPCODE(StopMusic);
	DECLARE_OPCODE(IncrementParam1);
	DECLARE_OPCODE(SetVolume);
	DECLARE_OPCODE(Jump);
	DECLARE_OPCODE(RunPuzzle);
	DECLARE_OPCODE(JumpIfAction8);
	DECLARE_OPCODE(SetAction8);
	DECLARE_OPCODE(ClearAction8);
	DECLARE_OPCODE(CreatePalette);
	DECLARE_OPCODE(IncrementParam2);
	DECLARE_OPCODE(WaitUntilFramePlayed);
	DECLARE_OPCODE(UpdateWideScreen);
	DECLARE_OPCODE(JumpIfActor);
	DECLARE_OPCODE(PlaySpeechScene);
	DECLARE_OPCODE(PlaySpeech);
	DECLARE_OPCODE(PlaySpeechScene2);
	DECLARE_OPCODE(MoveScenePositionFromActor);
	DECLARE_OPCODE(PaletteFade);
	DECLARE_OPCODE(QueuePaletteFade);
	DECLARE_OPCODE(PlaySoundUpdateObject);
	DECLARE_OPCODE(ActorFaceTarget);
	DECLARE_OPCODE(HideMatteBars);
	DECLARE_OPCODE(ShowMatteBars);
	DECLARE_OPCODE(JumpIfSoundPlaying);
	DECLARE_OPCODE(ChangePlayer);
	DECLARE_OPCODE(ChangeActorStatus);
	DECLARE_OPCODE(StopSound);
	DECLARE_OPCODE(JumpRandom);
	DECLARE_OPCODE(ClearScreen);
	DECLARE_OPCODE(Quit);
	DECLARE_OPCODE(JumpObjectFrame);
	DECLARE_OPCODE(DeleteGraphics);
	DECLARE_OPCODE(SetActorField944);
	DECLARE_OPCODE(SetScriptField1BB0);
	DECLARE_OPCODE(OnScriptField1BB0);
	DECLARE_OPCODE(WalkToActor);
	DECLARE_OPCODE(SetResourcePalette);
	DECLARE_OPCODE(SetObjectFrameIndexAndFlags);
	DECLARE_OPCODE(SetObjectFlags);
	DECLARE_OPCODE(SetActorActionIndex2);
	DECLARE_OPCODE(UpdateTransparency);
	DECLARE_OPCODE(QueueScript);
	DECLARE_OPCODE(ProcessActor);
	DECLARE_OPCODE(ClearActorFields);
	DECLARE_OPCODE(SetObjectLastFrameIndex);
	DECLARE_OPCODE(SetActionAreaFlags);
	DECLARE_OPCODE(MorphActor);
	DECLARE_OPCODE(ShowMenu);
	DECLARE_OPCODE(UpdateGlobalFlags);

	friend class Console;
}; // end of class ActionList

} // end of namespace Asylum

#endif // ASYLUM_RESOURCES_SCRIPT_H
