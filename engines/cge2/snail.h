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
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_SNAIL_H
#define CGE2_SNAIL_H

#include "cge2/cge2_main.h"

namespace CGE2 {

enum Action { kNear, kMTake, kFTake, kActions };

enum SNCom {
	kSNCom0 = 128,
	kSNNop,     // NOP                       :: do nothing
	kSNUse,     // USE <spr> <cav>|<lab>     :: hint for using
	kSNPause,   // PAUSE -1 <dly>            :: delay <dly>/72 seconds
	kSNInf,     // INF -1 <ref>              :: show text referrenced by <ref>
	kSNCave,    // CAVE -1 <cav>             :: go to board <cav>
	kSNSlave,   // SLAVE
	kSNFocus,   // FOCUS                     :: change active hero
	kSNSetX,    // SETX <x> <idx>            :: set sprite shift in x axis
	kSNSetY,    // SETX <y> <idx>            :: set sprite shift in y axis
	kSNSetZ,    // SETX <z> <idx>            :: set sprite shift in z axis
	kSNAdd,     // ADD <idx1> <idx2>         :: sum vectors
	kSNSub,     // SUB <idx1> <idx2>         :: subtract vectors
	kSNMul,     // MUL <idx> <nr>            :: multiply vector by number
	kSNDiv,     // DIV <idx> <nr>            :: divide vector by number
	kSNIf,      // IF
	kSNFlag,    // FLAG <nr> <val>           :: set flag <nr> to <val>
	kSNFlash,   // FLASH -1 0|1              :: lighten whole image (on/off)
	kSNLight,   // LIGHT
	kSNCycle,   // CYCLE <cnt>               :: rotate <cnt> colors from 1
	kSNClear,   // CLEAR -1 0                :: clear kSNAIL queue
	kSNTalk,    // TALK -1 0|1               :: enable speach (on/off)
	kSNMouse,   // MOUSE -1 0|1              :: enable mouse (on/off)
	kSNMap,     // MAP  0|1 0                :: temporarily turn off map for hero
	kSNCount,   // COUNT
	kSNMidi,    // MIDI -1 <midi>            :: play MIDI referenced by <midi> (-1 = off)
	kSNSetDlg,  // SETDLG 0..3 0..3          :: switch of speach mode
	kSNMskDlg,  // MSKDLG 0..3 0..3          :: switch of speach mode mask

	kSNSpr,

	kSNWait,    // WAIT <spr> <seq>|-1       :: wait for SEQ <seq> (-1 = freeze)
	kSNHide,    // HIDE <spr> 0|1            :: visibility of sprite
	kSNRoom,    // ROOM <hero> 0|1           :: additional room in pocket (no/yes)
	kSNSay,     // SAY <spr> <ref>           :: say text referenced by <ref>
	kSNSound,   // SOUND <spr> <ref>         :: play sound effect referenced by <ref>
	kSNTime,    // TIME <spr> 0              :: say current time
	kSNKill,    // KILL <spr> 0              :: remove sprite
	kSNRSeq,    // RSEQ <spr> <nr>           :: relative jump SEQ <nr> lines
	kSNSeq,     // SEQ <spr> <seq>           :: jump to certain SEQ
	kSNSend,    // SEND <spr> <cav>          :: move sprite to board <cav>
	kSNSwap,    // SWAP <spr1> spr2>         :: sprite exchange
	kSNKeep,    // KEEP <spr> <seq>          :: take sprite into pocket and jump to <seq>
	kSNGive,    // GIVE <spr> <seq>          :: remove sprite from pocket and jump to <seq>
	kSNGetPos,  // GETPOS <spr> <idx>        :: take sprite's position
	kSNGoto,    // GOTO <spr> <idx>          :: move sprite to position
	kSNMoveX,   // MOVEX <spr> <dx>          :: relative move along X axis
	kSNMoveY,   // MOVEY <spr> <dy>          :: relative move along Y axis
	kSNMoveZ,   // MOVEZ <spr> <dz>          :: relative move along Z axis
	kSNTrans,   // TRANS <spr> 0|1           :: clear/set logical transparency
	kSNPort,    // PORT <spr> 0|1            :: clear/set "takeability" of sprite
	kSNNext,    // NEXT <spr> <nr>           :: jump to <nr> - NEAR or TAKE
	kSNNNext,   // NNEXT <spr> <nr>          :: jump to <nr> - NEAR
	kSNMTNext,  // MTNEXT <spr> <nr>         :: jump to <nr> - TAKE
	kSNFTNext,  // FTNEXT <spr> <nr>         :: jump to <nr> - TAKE
	kSNRNNext,  // RNNEXT <spr> <nr>         :: relative jump to <nr> - NEAR
	kSNRMTNext, // RMTNEXT <spr> <nr>        :: relative jump to <nr> - TAKE
	kSNRFTNext, // RFTNEXT <spr> <nr>        :: relative jump to <nr> - TAKE
	kSNRMNear,  // RMNEAR <spr> 0            :: remove NEAR list
	kSNRMMTake, // RMMTAKE <spr> 0           :: remove TAKE list
	kSNRMFTake, // RMFTAKE <spr> 0           :: remove TAKE list
	kSNSetRef,  // RETREF <spr> <ref>        :: change reference of sprite <spr> to <ref> 
	kSNBackPt,  // BACKPT <spr> 0            :: paint sprite onto the background
	kSNWalk,    // WALK <hero> <ref>|<point> :: go close to the sprite or point
	kSNReach,   // REACH <hero> <ref>|<m>    :: reach the sprite or point with <m> method
	kSNCover,   // COVER <sp1> <sp2>         :: cover sprite <sp1> with sprite <sp2>
	kSNUncover, // UNCOVER <sp1> <sp2>       :: restore the state before COVER

	kSNDim,
	kSNExec,
	kSNStep,
	kSNGhost,
	
	kSNNOne = kNoByte
};

class Snail {
	static const char *comTxt[];
public:
	static int com(const char *com);
};

} // End of namespace CGE2

#endif
