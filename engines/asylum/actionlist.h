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
	uint32 numLines; // Only set on the first line of each script
	uint32 opcode;
	int param1;
	int param2;
	int param3;
	int param4;
	int param5;
	int param6;
	int param7;
	int param8;
	int param9;

} ScriptEntry;

typedef struct Script {
	ScriptEntry commands[MAX_ACTION_COMMANDS];
	uint32        field_1BAC;
	uint32        field_1BB0;
	uint32        counter;
} Script;

typedef struct ScriptQueueEntry {
	int actionListIndex;
	//int actionListItemIndex;
	int actorIndex;
	//int field_C;
	//int field_10;

} ScriptQueueEntry;

/*
typedef struct ScriptQueue {
	ScriptQueueEntry entries[10];
	int count;
	int field_CC;

} ScriptQueue;
*/

class ActionList {
public:
	ActionList(Common::SeekableReadStream *stream, Scene *scene);
	virtual ~ActionList();

	uint32 size;
	uint32 numEntries;

	Common::Array<Script> entries;

	// FIXME
	// Made all the internal control variables public and removed the getter/setter
	// pairs for simplicity. This should be refactored later, once the function mapping
	// is cleaned up properly
	int  currentLine;
	int  currentLoops;
	int  delayedSceneIndex;
	int  delayedVideoIndex;
	bool allowInput;
	int  lineIncrement;
	bool done;
	bool waitCycle;

	/** .text:00402120
	 * Process the current script
	 */
	int process();
	/** .text:00401020
	 * Reset the _scripts entries to their default values
	 */
	void resetQueue();
	/** .text:00401050
	 * Initialize the script element at actionIndex to
	 * the actor at actorIndex
	 */
	void queueScript(int actionIndex, int actorIndex);
	/** .text:00401100
	 * Update the queued scripts
	 */
	//void updateQueue(int queueIndex);
	/**
	 * Toggle the action queue processing flag
	 */
	void setActionFlag(bool value) { _actionFlag = value; }


	void processActionListSub02(Script* script, ScriptEntry* command,  int a4);
	void enableActorSub(int actorIndex, int condition);

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
int kReturn0(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetGameFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearGameFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int kToggleGameFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpIfGameFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int kHideCursor(Script *script, ScriptEntry *cmd, Scene *scn);
int kShowCursor(Script *script, ScriptEntry *cmd, Scene *scn);
int kPlayAnimation(Script *script, ScriptEntry *cmd, Scene *scn);
int kMoveScenePosition(Script *script, ScriptEntry *cmd, Scene *scn);
int kHideActor(Script *script, ScriptEntry *cmd, Scene *scn);
int kShowActor(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetActorStats(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetSceneMotionStat(Script *script, ScriptEntry *cmd, Scene *scn);
int kDisableActor(Script *script, ScriptEntry *cmd, Scene *scn);
int kEnableActor(Script *script, ScriptEntry *cmd, Scene *scn);
int kEnableBarriers(Script *script, ScriptEntry *cmd, Scene *scn);
int kReturn(Script *script, ScriptEntry *cmd, Scene *scn);
int kDestroyBarrier(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk12_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk13_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk14_JMP_WALK_ACTOR(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk15(Script *script, ScriptEntry *cmd, Scene *scn);
int kResetAnimation(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearFlag1Bit0(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk18_PLAY_SND(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpIfFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearFlag2Bit0(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpIfFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearFlag2Bit2(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpIfFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearFlag2Bit1(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk22(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk23(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk24(Script *script, ScriptEntry *cmd, Scene *scn);
int kRunEncounter(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpIfFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearFlag2Bit4(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetActorField638(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpIfActorField638(Script *script, ScriptEntry *cmd, Scene *scn);
int kChangeScene(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk2C_ActorSub(Script *script, ScriptEntry *cmd, Scene *scn);
int kPlayMovie(Script *script, ScriptEntry *cmd, Scene *scn);
int kStopAllBarriersSounds(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetActionFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearActionFlag(Script *script, ScriptEntry *cmd, Scene *scn);
int kResetSceneRect(Script *script, ScriptEntry *cmd, Scene *scn);
int kChangeMusicById(Script *script, ScriptEntry *cmd, Scene *scn);
int kStopMusic(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk34_Status(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk35(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk36(Script *script, ScriptEntry *cmd, Scene *scn);
int kRunBlowUpPuzzle(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpIfFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearFlag2Bit3(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk3B_PALETTE_MOD(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk3C_CMP_VAL(Script *script, ScriptEntry *cmd, Scene *scn);
int kWaitUntilFramePlayed(Script *script, ScriptEntry *cmd, Scene *scn);
int kUpdateWideScreen(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk3F(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk40_SOUND(Script *script, ScriptEntry *cmd, Scene *scn);
int kPlaySpeech(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk42(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk43(Script *script, ScriptEntry *cmd, Scene *scn);
int kPaletteFade(Script *script, ScriptEntry *cmd, Scene *scn);
int kStartPaletteFadeThread(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk46(Script *script, ScriptEntry *cmd, Scene *scn);
int kActorFaceObject(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk48_MATTE_01(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk49_MATTE_90(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpIfSoundPlaying(Script *script, ScriptEntry *cmd, Scene *scn);
int kChangePlayerCharacterIndex(Script *script, ScriptEntry *cmd, Scene *scn);
int kChangeActorField40(Script *script, ScriptEntry *cmd, Scene *scn);
int kStopSound(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk4E_RANDOM_COMMAND(Script *script, ScriptEntry *cmd, Scene *scn);
int kClearScreen(Script *script, ScriptEntry *cmd, Scene *scn);
int kQuit(Script *script, ScriptEntry *cmd, Scene *scn);
int kJumpBarrierFrame(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk52(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk53(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk54_SET_ACTIONLIST_6EC(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk55(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk56(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetResourcePalette(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetBarrierFrameIdxFlaged(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk59(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk5A(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk5B(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk5C(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk5D(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk5E(Script *script, ScriptEntry *cmd, Scene *scn);
int kSetBarrierLastFrameIdx(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk60_SET_OR_CLR_ACTIONAREA_FLAG(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk61(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk62_SHOW_OPTIONS_SCREEN(Script *script, ScriptEntry *cmd, Scene *scn);
int k_unk63(Script *script, ScriptEntry *cmd, Scene *scn);

} // end of namespace Asylum

#endif
