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
#include "common/stream.h"

#include "asylum/scene.h"

namespace Asylum {

#define MAX_ACTION_COMMANDS 161

class Scene;

typedef struct ActionCommand {
	uint32 numLines;	//	Only set on the first line of each script
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

} ActionCommand;

typedef struct ActionDefinitions {
	ActionCommand commands[MAX_ACTION_COMMANDS];
	uint32        field_1BAC;
	uint32        field_1BB0;
	uint32        counter;
} ActionDefinitions;

class ActionList {
public:
	ActionList(Common::SeekableReadStream *stream, Scene *scene);
	virtual ~ActionList();

	uint32 size;
	uint32 numEntries;

	Common::Array<ActionDefinitions> entries;

	// FIXME
	// Made all the internal control variables public and removed the getter/setter
	// pairs for simplicity. This should be refactored later, once the function mapping
	// is cleaned up properly
	int  currentLine;
	int  currentLoops;
	int  delayedSceneIndex;
	int  delayedVideoIndex;
	bool allowInput;
	bool processing;
	int  lineIncrement;
	bool done;
	bool waitCycle;

	void setScriptByIndex(uint32 index);
	ActionDefinitions* getScript() {
		return _currentScript;
	}

	/** .text:00402120
	 * Process the current script
	 */
	int  process();

	void processActionListSub02(ActionDefinitions* script, ActionCommand* command,  int a4);
	void enableActorSub(int actorIndex, int condition);

private:
	Scene *_scene;
	ActionDefinitions *_currentScript;

	void load(Common::SeekableReadStream *stream);

};

// opcode functions
int kReturn0(ActionCommand *cmd, Scene *scn);
int kSetGameFlag(ActionCommand *cmd, Scene *scn);
int kClearGameFlag(ActionCommand *cmd, Scene *scn);
int kToggleGameFlag(ActionCommand *cmd, Scene *scn);
int kJumpIfGameFlag(ActionCommand *cmd, Scene *scn);
int kHideCursor(ActionCommand *cmd, Scene *scn);
int kShowCursor(ActionCommand *cmd, Scene *scn);
int kPlayAnimation(ActionCommand *cmd, Scene *scn);
int kMoveScenePosition(ActionCommand *cmd, Scene *scn);
int kHideActor(ActionCommand *cmd, Scene *scn);
int kShowActor(ActionCommand *cmd, Scene *scn);
int kSetActorStats(ActionCommand *cmd, Scene *scn);
int kSetSceneMotionStat(ActionCommand *cmd, Scene *scn);
int kDisableActor(ActionCommand *cmd, Scene *scn);
int kEnableActor(ActionCommand *cmd, Scene *scn);
int kEnableBarriers(ActionCommand *cmd, Scene *scn);
int kReturn(ActionCommand *cmd, Scene *scn);
int kDestroyBarrier(ActionCommand *cmd, Scene *scn);
int k_unk12_JMP_WALK_ACTOR(ActionCommand *cmd, Scene *scn);
int k_unk13_JMP_WALK_ACTOR(ActionCommand *cmd, Scene *scn);
int k_unk14_JMP_WALK_ACTOR(ActionCommand *cmd, Scene *scn);
int k_unk15(ActionCommand *cmd, Scene *scn);
int kResetAnimation(ActionCommand *cmd, Scene *scn);
int kClearFlag1Bit0(ActionCommand *cmd, Scene *scn);
int k_unk18_PLAY_SND(ActionCommand *cmd, Scene *scn);
int kJumpIfFlag2Bit0(ActionCommand *cmd, Scene *scn);
int kSetFlag2Bit0(ActionCommand *cmd, Scene *scn);
int kClearFlag2Bit0(ActionCommand *cmd, Scene *scn);
int kJumpIfFlag2Bit2(ActionCommand *cmd, Scene *scn);
int kSetFlag2Bit2(ActionCommand *cmd, Scene *scn);
int kClearFlag2Bit2(ActionCommand *cmd, Scene *scn);
int kJumpIfFlag2Bit1(ActionCommand *cmd, Scene *scn);
int kSetFlag2Bit1(ActionCommand *cmd, Scene *scn);
int kClearFlag2Bit1(ActionCommand *cmd, Scene *scn);
int k_unk22(ActionCommand *cmd, Scene *scn);
int k_unk23(ActionCommand *cmd, Scene *scn);
int k_unk24(ActionCommand *cmd, Scene *scn);
int kRunEncounter(ActionCommand *cmd, Scene *scn);
int kJumpIfFlag2Bit4(ActionCommand *cmd, Scene *scn);
int kSetFlag2Bit4(ActionCommand *cmd, Scene *scn);
int kClearFlag2Bit4(ActionCommand *cmd, Scene *scn);
int kSetActorField638(ActionCommand *cmd, Scene *scn);
int kJumpIfActorField638(ActionCommand *cmd, Scene *scn);
int kChangeScene(ActionCommand *cmd, Scene *scn);
int k_unk2C_ActorSub(ActionCommand *cmd, Scene *scn);
int kPlayMovie(ActionCommand *cmd, Scene *scn);
int kStopAllBarriersSounds(ActionCommand *cmd, Scene *scn);
int kSetActionFlag(ActionCommand *cmd, Scene *scn);
int kClearActionFlag(ActionCommand *cmd, Scene *scn);
int kResetSceneRect(ActionCommand *cmd, Scene *scn);
int kChangeMusicById(ActionCommand *cmd, Scene *scn);
int kStopMusic(ActionCommand *cmd, Scene *scn);
int k_unk34_Status(ActionCommand *cmd, Scene *scn);
int k_unk35(ActionCommand *cmd, Scene *scn);
int k_unk36(ActionCommand *cmd, Scene *scn);
int kRunBlowUpPuzzle(ActionCommand *cmd, Scene *scn);
int kJumpIfFlag2Bit3(ActionCommand *cmd, Scene *scn);
int kSetFlag2Bit3(ActionCommand *cmd, Scene *scn);
int kClearFlag2Bit3(ActionCommand *cmd, Scene *scn);
int k_unk3B_PALETTE_MOD(ActionCommand *cmd, Scene *scn);
int k_unk3C_CMP_VAL(ActionCommand *cmd, Scene *scn);
int kWaitUntilFramePlayed(ActionCommand *cmd, Scene *scn);
int kUpdateWideScreen(ActionCommand *cmd, Scene *scn);
int k_unk3F(ActionCommand *cmd, Scene *scn);
int k_unk40_SOUND(ActionCommand *cmd, Scene *scn);
int kPlaySpeech(ActionCommand *cmd, Scene *scn);
int k_unk42(ActionCommand *cmd, Scene *scn);
int k_unk43(ActionCommand *cmd, Scene *scn);
int kPaletteFade(ActionCommand *cmd, Scene *scn);
int kStartPaletteFadeThread(ActionCommand *cmd, Scene *scn);
int k_unk46(ActionCommand *cmd, Scene *scn);
int kActorFaceObject(ActionCommand *cmd, Scene *scn);
int k_unk48_MATTE_01(ActionCommand *cmd, Scene *scn);
int k_unk49_MATTE_90(ActionCommand *cmd, Scene *scn);
int kJumpIfSoundPlaying(ActionCommand *cmd, Scene *scn);
int kChangePlayerCharacterIndex(ActionCommand *cmd, Scene *scn);
int kChangeActorField40(ActionCommand *cmd, Scene *scn);
int kStopSound(ActionCommand *cmd, Scene *scn);
int k_unk4E_RANDOM_COMMAND(ActionCommand *cmd, Scene *scn);
int kClearScreen(ActionCommand *cmd, Scene *scn);
int kQuit(ActionCommand *cmd, Scene *scn);
int kJumpBarrierFrame(ActionCommand *cmd, Scene *scn);
int k_unk52(ActionCommand *cmd, Scene *scn);
int k_unk53(ActionCommand *cmd, Scene *scn);
int k_unk54_SET_ACTIONLIST_6EC(ActionCommand *cmd, Scene *scn);
int k_unk55(ActionCommand *cmd, Scene *scn);
int k_unk56(ActionCommand *cmd, Scene *scn);
int kSetResourcePalette(ActionCommand *cmd, Scene *scn);
int kSetBarrierFrameIdxFlaged(ActionCommand *cmd, Scene *scn);
int k_unk59(ActionCommand *cmd, Scene *scn);
int k_unk5A(ActionCommand *cmd, Scene *scn);
int k_unk5B(ActionCommand *cmd, Scene *scn);
int k_unk5C(ActionCommand *cmd, Scene *scn);
int k_unk5D(ActionCommand *cmd, Scene *scn);
int k_unk5E(ActionCommand *cmd, Scene *scn);
int kSetBarrierLastFrameIdx(ActionCommand *cmd, Scene *scn);
int k_unk60_SET_OR_CLR_ACTIONAREA_FLAG(ActionCommand *cmd, Scene *scn);
int k_unk61(ActionCommand *cmd, Scene *scn);
int k_unk62_SHOW_OPTIONS_SCREEN(ActionCommand *cmd, Scene *scn);
int k_unk63(ActionCommand *cmd, Scene *scn);

} // end of namespace Asylum

#endif
