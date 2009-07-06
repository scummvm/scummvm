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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_INTERPERTER_H_
#define ASYLUM_INTERPERTER_H_

#include "asylum/scene.h"

namespace Asylum {


class Interpreter {
public:
    Interpreter(Scene *scene);
    ~Interpreter();
    
    void processActionLists();
    bool isProcessing() { return _processing; }
    
private:
    Scene * _scene;
    int _currentLine;
    int _currentScriptIndex;
    int _currentLoops;
    bool _processing;
    
	friend class Console;
};

enum opcodes {
    kReturn0                = 0x00,
    kSetGameFlag,
    kClearGameFlag,
    kToogleGameFlag,
    kJumpIfGameFlag,
    kHideCursor,
    kShowCursor,
    kPlayAnimation,
    kMoveScenePosition,
    kHideActor,
    kShowActor,
    kSetActorStats,
    k_unk0C_SET_SCENE_FIELD88,
    kDisableActor,
    kEnableActor,
    kEnableBarriers,
    kReturn,                // 0x10
    kDestroyObject,
    k_unk12_JMP_WALK_ACTOR,
    k_unk13_JMP_WALK_ACTOR,
    k_unk14_JMP_WALK_ACTOR,
    k_unk15,
    kResetAnimation,
    kClearFlag1Bit0,
    k_unk18_PLAY_SND,
    kJumpIfFlag2Bit0,
    kSetFlag2Bit0,
    kClearFlag2Bit0,
    kJumpIfFlag2Bit2,
    kSetFlag2Bit2,
    kClearFlag2Bit2,
    kJumpIfFlag2Bit1,
    kSetFlag2Bit1,          // 0x20
    kClearFlag2Bit1,
    k_unk22,
    k_unk23,
    k_unk24,
    kRunEncounter,
    kJumpIfFlag2Bit4,
    kSetFlag2Bit4,
    kClearFlag2Bit4,
    kSetActorField638,
    kJumpIfActorField638,
    kChangeScene,
    k_unk2C_ActorSub,
    kPlayMovie,
    kStopAllObjectsSounds,
    kSetActionFlag01,
    kClearActionFlag01,     // 0x30
    kResetSceneRect,
    kChangeMusicById,
    kStopMusic,
    k_unk34_Status,
    k_unk35,
    k_unk36,
    kRunMinigame,
    kJumpIfFlag2Bit3,
    kSetFlag2Bit3,
    kClearFlag2Bit3,
    k_unk3B_PALETTE_MOD,
    k_unk3C_CMP_VAL,
    kWaitUntilFramePlayed,
    kUpdateMatteBars,
    k_unk3F,
    k_unk40_SOUND,          // 0x40
    kPlaySpeech,
    k_unk42,
    k_unk43,
    kPaletteFade,
    kStartPaletteFadeThread,
    k_unk46,
    kActorFaceObject,
    k_unk48_MATTE_01,
    k_unk49_MATTE_90,
    kJumpIfSoundPlaying,
    kChangePlayerCharacterIndex,
    kChangeActorField40,
    kStopSound,
    k_unk4E_RANDOM_COMMAND,
    kDrawGame,
    kQuit,                  // 0x50
    kJumpObjectFrame,
    k_unk52,
    k_unk53,
    k_unk54_SET_ACTIONLIST_6EC,
    k_unk55,
    k_unk56,
    k_unk57,
    k_unk58,
    k_unk59,
    k_unk5A,
    k_unk5B,
    k_unk5C,
    k_unk5D,
    k_unk5E,
    k_unk5F,
    k_unk60_SET_OR_CLR_ACTIONAREA_FLAG,     // 0x60
    k_unk61,
    k_unk62_SHOW_OPTIONS_SCREEN
};

} // end of namespace Asylum

#endif
