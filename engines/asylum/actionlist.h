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

#ifndef ASYLUM_ACTIONLIST_H_
#define ASYLUM_ACTIONLIST_H_

#include "common/array.h"
#include "common/stack.h"
#include "common/stream.h"

#include "asylum/scene.h"

namespace Asylum {

#define MAX_ACTION_COMMANDS 161

class Scene;

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
	int32        field_1BAC;
	int32        field_1BB0;
	int32        counter;
} Script;

typedef struct ScriptQueueEntry {
	int32 actionListIndex;
	//int32 actionListItemIndex;
	int32 actorIndex;
	//int32 field_C;
	//int32 field_10;

} ScriptQueueEntry;

/*
typedef struct ScriptQueue {
	ScriptQueueEntry entries[10];
	int32 count;
	int32 field_CC;

} ScriptQueue;
*/

class ActionList {
public:
	ActionList(Common::SeekableReadStream *stream, Scene *scene);
	virtual ~ActionList();

	int32 size;
	int32 numEntries;

	Common::Array<Script> entries;

	// FIXME
	// Made all the internal control variables public and removed the getter/setter
	// pairs for simplicity. This should be refactored later, once the function mapping
	// is cleaned up properly
	int32  currentLine;
	int32  currentLoops;
	int32  delayedSceneIndex;
	int32  delayedVideoIndex;
	bool allowInput;
	int32  lineIncrement;
	bool done;
	bool waitCycle;

	/** .text:00402120
	 * Process the current script
	 */
	int32 process();
	/** .text:00401020
	 * Reset the _scripts entries to their default values
	 */
	void resetQueue();
	/** .text:00401050
	 * Initialize the script element at actionIndex to
	 * the actor at actorIndex
	 */
	void queueScript(int32 actionIndex, int32 actorIndex);
	/** .text:00401100
	 * Update the queued scripts
	 */
	//void updateQueue(int32 queueIndex);
	/**
	 * Toggle the action queue processing flag
	 */
	void setActionFlag(bool value) { _actionFlag = value; }


	void processActionListSub02(Script* script, ScriptEntry* command,  int32 a4);
	void enableActorSub(int32 actorIndex, int32 condition);

private:
	Scene *_scene;
	bool  _actionFlag;
	//ScriptQueue _scripts;
	Common::Stack<ScriptQueueEntry> _scripts;
	Script *_currentScript;
	ScriptQueueEntry _currentQueueEntry;
	void load(Common::SeekableReadStream *stream);
};

// opcode functions
int32 kReturn0(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetGameFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearGameFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kToggleGameFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpIfGameFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kHideCursor(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kShowCursor(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kPlayAnimation(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kMoveScenePosition(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kHideActor(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kShowActor(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetActorStats(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetSceneMotionStat(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kDisableActor(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kEnableActor(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kEnableBarriers(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kReturn(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kDestroyBarrier(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk12_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk13_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk14_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk15(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kResetAnimation(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearFlag1Bit0(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk18_PLAY_SND(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpIfFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpIfFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpIfFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk22(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk23(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk24(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kRunEncounter(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpIfFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetActorField638(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpIfActorField638(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kChangeScene(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk2C_ActorSub(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kPlayMovie(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kStopAllBarriersSounds(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetActionFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearActionFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kResetSceneRect(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kChangeMusicById(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kStopMusic(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk34_Status(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk35(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk36(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kRunBlowUpPuzzle(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpIfFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk3B_PALETTE_MOD(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk3C_CMP_VAL(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kWaitUntilFramePlayed(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kUpdateWideScreen(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk3F(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk40_SOUND(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kPlaySpeech(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk42(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk43(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kPaletteFade(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kStartPaletteFadeThread(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk46(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kActorFaceObject(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk48_MATTE_01(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk49_MATTE_90(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpIfSoundPlaying(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kChangePlayerCharacterIndex(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kChangeActorField40(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kStopSound(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk4E_RANDOM_COMMAND(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kClearScreen(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kQuit(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kJumpBarrierFrame(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk52(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk53(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk54_SET_ACTIONLIST_6EC(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk55(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk56(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetResourcePalette(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetBarrierFrameIdxFlaged(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk59(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk5A(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk5B(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk5C(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk5D(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk5E(Script *script, ScriptEntry *cmd, Scene *scn);
int32 kSetBarrierLastFrameIdx(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk60_SET_OR_CLR_ACTIONAREA_FLAG(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk61(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk62_SHOW_OPTIONS_SCREEN(Script *script, ScriptEntry *cmd, Scene *scn);
int32 k_unk63(Script *script, ScriptEntry *cmd, Scene *scn);

} // end of namespace Asylum

#endif
