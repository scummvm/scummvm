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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef ASYLUM_SCRIPTMANAGER_H_
#define ASYLUM_SCRIPTMANAGER_H_

#include "common/singleton.h"

#include "asylum/scene.h"

namespace Asylum {

class ActionList;
struct ActionDefinitions;
struct ActionCommand;
class WorldStats;

class ScriptManager: public Common::Singleton<ScriptManager> {
public:

	int processActionList();

	void setScript(ActionDefinitions *action);
	void setScriptIndex(uint32 index);
    ActionDefinitions* getScript() { return _currentScript; }

    int checkBarrierFlags(int barrierId);
    int setBarrierNextFrame(int barrierId, int barrierFlags);

    void processActionListSub02(ActionDefinitions* script, ActionCommand* command,  int a4);
    void enableActorSub(int actorIndex, int condition);

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

private:
	friend class Common::Singleton<SingletonBaseType>;
	ScriptManager();
	~ScriptManager();

	ActionDefinitions *_currentScript;

	friend class Console;

}; // end of class ScriptManager

// opcode functions
int kReturn0(ActionCommand *cmd);
int kSetGameFlag(ActionCommand *cmd);
int kClearGameFlag(ActionCommand *cmd);
int kToggleGameFlag(ActionCommand *cmd);
int kJumpIfGameFlag(ActionCommand *cmd);
int kHideCursor(ActionCommand *cmd);
int kShowCursor(ActionCommand *cmd);
int kPlayAnimation(ActionCommand *cmd);
int kMoveScenePosition(ActionCommand *cmd);
int kHideActor(ActionCommand *cmd);
int kShowActor(ActionCommand *cmd);
int kSetActorStats(ActionCommand *cmd);
int kSetSceneMotionStat(ActionCommand *cmd);
int kDisableActor(ActionCommand *cmd);
int kEnableActor(ActionCommand *cmd);
int kEnableBarriers(ActionCommand *cmd);
int kReturn(ActionCommand *cmd);
int kDestroyBarrier(ActionCommand *cmd);
int k_unk12_JMP_WALK_ACTOR(ActionCommand *cmd);
int k_unk13_JMP_WALK_ACTOR(ActionCommand *cmd);
int k_unk14_JMP_WALK_ACTOR(ActionCommand *cmd);
int k_unk15(ActionCommand *cmd);
int kResetAnimation(ActionCommand *cmd);
int kClearFlag1Bit0(ActionCommand *cmd);
int k_unk18_PLAY_SND(ActionCommand *cmd);
int kJumpIfFlag2Bit0(ActionCommand *cmd);
int kSetFlag2Bit0(ActionCommand *cmd);
int kClearFlag2Bit0(ActionCommand *cmd);
int kJumpIfFlag2Bit2(ActionCommand *cmd);
int kSetFlag2Bit2(ActionCommand *cmd);
int kClearFlag2Bit2(ActionCommand *cmd);
int kJumpIfFlag2Bit1(ActionCommand *cmd);
int kSetFlag2Bit1(ActionCommand *cmd);
int kClearFlag2Bit1(ActionCommand *cmd);
int k_unk22(ActionCommand *cmd);
int k_unk23(ActionCommand *cmd);
int k_unk24(ActionCommand *cmd);
int kRunEncounter(ActionCommand *cmd);
int kJumpIfFlag2Bit4(ActionCommand *cmd);
int kSetFlag2Bit4(ActionCommand *cmd);
int kClearFlag2Bit4(ActionCommand *cmd);
int kSetActorField638(ActionCommand *cmd);
int kJumpIfActorField638(ActionCommand *cmd);
int kChangeScene(ActionCommand *cmd);
int k_unk2C_ActorSub(ActionCommand *cmd);
int kPlayMovie(ActionCommand *cmd);
int kStopAllBarriersSounds(ActionCommand *cmd);
int kSetActionFlag01(ActionCommand *cmd);
int kClearActionFlag01(ActionCommand *cmd);
int kResetSceneRect(ActionCommand *cmd);
int kChangeMusicById(ActionCommand *cmd);
int kStopMusic(ActionCommand *cmd);
int k_unk34_Status(ActionCommand *cmd);
int k_unk35(ActionCommand *cmd);
int k_unk36(ActionCommand *cmd);
int kRunBlowUpPuzzle(ActionCommand *cmd);
int kJumpIfFlag2Bit3(ActionCommand *cmd);
int kSetFlag2Bit3(ActionCommand *cmd);
int kClearFlag2Bit3(ActionCommand *cmd);
int k_unk3B_PALETTE_MOD(ActionCommand *cmd);
int k_unk3C_CMP_VAL(ActionCommand *cmd);
int kWaitUntilFramePlayed(ActionCommand *cmd);
int kUpdateWideScreen(ActionCommand *cmd);
int k_unk3F(ActionCommand *cmd);
int k_unk40_SOUND(ActionCommand *cmd);
int kPlaySpeech(ActionCommand *cmd);
int k_unk42(ActionCommand *cmd);
int k_unk43(ActionCommand *cmd);
int kPaletteFade(ActionCommand *cmd);
int kStartPaletteFadeThread(ActionCommand *cmd);
int k_unk46(ActionCommand *cmd);
int kActorFaceObject(ActionCommand *cmd);
int k_unk48_MATTE_01(ActionCommand *cmd);
int k_unk49_MATTE_90(ActionCommand *cmd);
int kJumpIfSoundPlaying(ActionCommand *cmd);
int kChangePlayerCharacterIndex(ActionCommand *cmd);
int kChangeActorField40(ActionCommand *cmd);
int kStopSound(ActionCommand *cmd);
int k_unk4E_RANDOM_COMMAND(ActionCommand *cmd);
int kClearScreen(ActionCommand *cmd);
int kQuit(ActionCommand *cmd);
int kJumpBarrierFrame(ActionCommand *cmd);
int k_unk52(ActionCommand *cmd);
int k_unk53(ActionCommand *cmd);
int k_unk54_SET_ACTIONLIST_6EC(ActionCommand *cmd);
int k_unk55(ActionCommand *cmd);
int k_unk56(ActionCommand *cmd);
int kSetResourcePalette(ActionCommand *cmd);
int kSetBarrierFrameIdxFlaged(ActionCommand *cmd);
int k_unk59(ActionCommand *cmd);
int k_unk5A(ActionCommand *cmd);
int k_unk5B(ActionCommand *cmd);
int k_unk5C(ActionCommand *cmd);
int k_unk5D(ActionCommand *cmd);
int k_unk5E(ActionCommand *cmd);
int kSetBarrierLastFrameIdx(ActionCommand *cmd);
int k_unk60_SET_OR_CLR_ACTIONAREA_FLAG(ActionCommand *cmd);
int k_unk61(ActionCommand *cmd);
int k_unk62_SHOW_OPTIONS_SCREEN(ActionCommand *cmd);
int k_unk63(ActionCommand *cmd);


#define ScriptMan	(::Asylum::ScriptManager::instance())

} // end of namespace Asylum

#endif
