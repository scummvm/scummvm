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
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_SNAIL_H
#define CGE2_SNAIL_H

#include "cge2/cge2.h"

namespace CGE2 {

#define kCommandFrameRate  80
#define kCommandFrameDelay (1000 / kCommandFrameRate)
#define kNoByte            -1 // Recheck this! We have no proof for it's original value.


enum CommandType {
	kCmdCom0 = 128,
	kCmdNop,     // NOP                       :: do nothing
	kCmdUse,     // USE <spr> <cav>|<lab>     :: hint for using
	kCmdPause,   // PAUSE -1 <dly>            :: delay <dly>/72 seconds
	kCmdInf,     // INF -1 <ref>              :: show text referrenced by <ref>
	kCmdCave,    // CAVE -1 <cav>             :: go to board <cav>
	kCmdSetX,    // SETX <x> <idx>            :: set sprite shift in x axis
	kCmdSetY,    // SETX <y> <idx>            :: set sprite shift in y axis
	kCmdSetZ,    // SETX <z> <idx>            :: set sprite shift in z axis
	kCmdAdd,     // ADD <idx1> <idx2>         :: sum vectors
	kCmdFlash,   // FLASH -1 0|1              :: lighten whole image (on/off)
	kCmdCycle,   // CYCLE <cnt>               :: rotate <cnt> colors from 1
	kCmdClear,   // CLEAR -1 0                :: clear kCmdAIL queue
	kCmdMouse,   // MOUSE -1 0|1              :: enable mouse (on/off)
	kCmdMap,     // MAP  0|1 0                :: temporarily turn off map for hero
	kCmdMidi,    // MIDI -1 <midi>            :: play MIDI referenced by <midi> (-1 = off)

	kCmdSpr,

	kCmdWait,    // WAIT <spr> <seq>|-1       :: wait for SEQ <seq> (-1 = freeze)
	kCmdHide,    // HIDE <spr> 0|1            :: visibility of sprite
	kCmdRoom,    // ROOM <hero> 0|1           :: additional room in pocket (no/yes)
	kCmdSay,     // SAY <spr> <ref>           :: say text referenced by <ref>
	kCmdSound,   // SOUND <spr> <ref>         :: play sound effect referenced by <ref>
	kCmdKill,    // KILL <spr> 0              :: remove sprite
	kCmdRSeq,    // RSEQ <spr> <nr>           :: relative jump SEQ <nr> lines
	kCmdSeq,     // SEQ <spr> <seq>           :: jump to certain SEQ
	kCmdSend,    // SEND <spr> <cav>          :: move sprite to board <cav>
	kCmdSwap,    // SWAP <spr1> spr2>         :: sprite exchange
	kCmdKeep,    // KEEP <spr> <seq>          :: take sprite into pocket and jump to <seq>
	kCmdGive,    // GIVE <spr> <seq>          :: remove sprite from pocket and jump to <seq>
	kCmdGetPos,  // GETPOS <spr> <idx>        :: take sprite's position
	kCmdGoto,    // GOTO <spr> <idx>          :: move sprite to position
	kCmdPort,    // PORT <spr> 0|1            :: clear/set "takeability" of sprite
	kCmdNext,    // NEXT <spr> <nr>           :: jump to <nr> - NEAR or TAKE
	kCmdNNext,   // NNEXT <spr> <nr>          :: jump to <nr> - NEAR
	kCmdMTNext,  // MTNEXT <spr> <nr>         :: jump to <nr> - TAKE
	kCmdFTNext,  // FTNEXT <spr> <nr>         :: jump to <nr> - TAKE
	kCmdRNNext,  // RNNEXT <spr> <nr>         :: relative jump to <nr> - NEAR
	kCmdRMTNext, // RMTNEXT <spr> <nr>        :: relative jump to <nr> - TAKE
	kCmdRFTNext, // RFTNEXT <spr> <nr>        :: relative jump to <nr> - TAKE
	kCmdRMNear,  // RMNEAR <spr> 0            :: remove NEAR list
	kCmdRMMTake, // RMMTAKE <spr> 0           :: remove TAKE list
	kCmdRMFTake, // RMFTAKE <spr> 0           :: remove TAKE list
	kCmdSetRef,  // SETREF <spr> <ref>        :: change reference of sprite <spr> to <ref>
	kCmdWalk,    // WALKTO <hero> <ref>|<point> :: go close to the sprite or point
	kCmdReach,   // REACH <hero> <ref>|<m>    :: reach the sprite or point with <m> method
	kCmdCover,   // COVER <sp1> <sp2>         :: cover sprite <sp1> with sprite <sp2>
	kCmdUncover, // UNCOVER <sp1> <sp2>       :: restore the state before COVER

	kCmdExec,
	kCmdGhost
};

class CommandHandler {
public:
	struct Command {
		CommandType _commandType;
		byte _lab;
		int _ref;
		int _val;
		void *_spritePtr;
		CallbackType _cbType;
	} *_commandList;
	static const char *_commandText[];
	bool _talkEnable;

	CommandHandler(CGE2Engine *vm, bool turbo);
	~CommandHandler();
	void runCommand();
	void addCommand(CommandType com, int ref, int val, void *ptr);
	void addCallback(CommandType com, int ref, int val, CallbackType cbType);
	void insertCommand(CommandType com, int ref, int val, void *ptr);
	bool idle();
	void clear();
	int getComId(const char *com);
	const char *getComStr(CommandType cmdType);
private:
	CGE2Engine *_vm;
	bool _turbo;
	uint8 _head;
	uint8 _tail;
	bool _textDelay;
	uint32 _timerExpiry; // "pause" in the original.
};

} // End of namespace CGE2

#endif
