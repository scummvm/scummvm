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
 */

#include "common/scummsys.h"
#include "access/access.h"
#include "access/scripts.h"

#define SCRIPT_START_BYTE 0xE0

namespace Access {

Scripts::Scripts(AccessEngine *vm) : _vm(vm) {
	_script = nullptr;
	_scriptLoc = nullptr;
	_sequence = 0;
	_endFlag = false;
	_returnCode = 0;
}

Scripts::~Scripts() {
	freeScriptData();
}

void Scripts::freeScriptData() {
	delete[] _script;
	_script = nullptr;
}

void Scripts::searchForSequence() {
	assert(_script);
	byte *pSrc = _script;
	int sequenceId;
	do {
		while (*pSrc++ != SCRIPT_START_BYTE) ;
		sequenceId = READ_LE_UINT16(pSrc);
		pSrc += 2;
	} while (sequenceId != _sequence);

	_scriptLoc = pSrc;
}

int Scripts::executeScript() {
	assert(_scriptLoc);
	_endFlag = 0;
	_returnCode = 0;

	do {
		byte *pSrc = _scriptLoc;
		for (pSrc = _scriptLoc; *pSrc == SCRIPT_START_BYTE; pSrc += 3) ;	
		_scriptCommand = *pSrc++;

		executeCommand(_scriptCommand - 0x80);
		_scriptLoc = pSrc;
	} while (!_endFlag);

	return _returnCode;
}

void Scripts::executeCommand(int commandIndex) {
	static const ScriptMethodPtr COMMAND_LIST[] = {
		&Scripts::CMDENDOBJECT, &Scripts::CMDJUMPLOOK, &Scripts::CMDJUMPHELP, &Scripts::CMDJUMPGET, &Scripts::CMDJUMPMOVE,
		&Scripts::CMDJUMPUSE, &Scripts::CMDJUMPTALK, &Scripts::CMDNULL, &Scripts::CMDPRINT, &Scripts::CMDRETPOS, &Scripts::CMDANIM,
		&Scripts::CMDSETFLAG, &Scripts::CMDCHECKFLAG, &Scripts::CMDGOTO, &Scripts::CMDSETINV, &Scripts::CMDSETINV,
		&Scripts::CMDCHECKINV, &Scripts::CMDSETTEX, &Scripts::CMDNEWROOM, &Scripts::CMDCONVERSE, &Scripts::CMDCHECKFRAME,
		&Scripts::CMDCHECKANIM, &Scripts::CMDSND, &Scripts::CMDRETNEG, &Scripts::CMDRETPOS, &Scripts::CMDCHECKLOC, &Scripts::CMDSETANIM,
		&Scripts::CMDDISPINV, &Scripts::CMDSETTIMER, &Scripts::CMDSETTIMER, &Scripts::CMDCHECKTIMER, &Scripts::CMDSETTRAVEL,
		&Scripts::CMDSETTRAVEL, &Scripts::CMDSETVID, &Scripts::CMDPLAYVID, &Scripts::CMDPLOTIMAGE, &Scripts::CMDSETDISPLAY,
		&Scripts::CMDSETBUFFER, &Scripts::CMDSETSCROLL, &Scripts::CMDSAVERECT, &Scripts::CMDSAVERECT, &Scripts::CMDSETBUFVID,
		&Scripts::CMDPLAYBUFVID, &Scripts::CMDREMOVELAST, &Scripts::CMDSPECIAL, &Scripts::CMDSPECIAL, &Scripts::CMDSPECIAL,
		&Scripts::CMDSETCYCLE, &Scripts::CMDCYCLE, &Scripts::CMDCHARSPEAK, &Scripts::CMDTEXSPEAK, &Scripts::CMDTEXCHOICE,
		&Scripts::CMDWAIT, &Scripts::CMDSETCONPOS, &Scripts::CMDCHECKVFRAME, &Scripts::CMDJUMPCHOICE, &Scripts::CMDRETURNCHOICE,
		&Scripts::CMDCLEARBLOCK, &Scripts::CMDLOADSOUND, &Scripts::CMDFREESOUND, &Scripts::CMDSETVIDSND, &Scripts::CMDPLAYVIDSND,
		&Scripts::CMDPUSHLOCATION, &Scripts::CMDPUSHLOCATION, &Scripts::CMDPUSHLOCATION, &Scripts::CMDPUSHLOCATION,
		&Scripts::CMDPUSHLOCATION, &Scripts::CMDPLAYEROFF, &Scripts::CMDPLAYERON, &Scripts::CMDDEAD, &Scripts::CMDFADEOUT,
		&Scripts::CMDENDVID, &Scripts::CMDHELP, &Scripts::CMDCYCLEBACK, &Scripts::CMDCHAPTER, &Scripts::CMDSETHELP, &Scripts::CMDCENTERPANEL,
		&Scripts::CMDMAINPANEL, &Scripts::CMDRETFLASH
	};

	(this->*COMMAND_LIST[commandIndex])();
}

void Scripts::CMDENDOBJECT() { }
void Scripts::CMDJUMPLOOK() { }
void Scripts::CMDJUMPHELP() { }
void Scripts::CMDJUMPGET() { }
void Scripts::CMDJUMPMOVE() { }
void Scripts::CMDJUMPUSE() { }
void Scripts::CMDJUMPTALK() { }
void Scripts::CMDNULL() { }
void Scripts::CMDPRINT() { }
void Scripts::CMDRETPOS() { }
void Scripts::CMDANIM() { }
void Scripts::CMDSETFLAG() { }
void Scripts::CMDCHECKFLAG() { }
void Scripts::CMDGOTO() { }
void Scripts::CMDSETINV() { }
void Scripts::CMDCHECKINV() { }
void Scripts::CMDSETTEX() { }
void Scripts::CMDNEWROOM() { }
void Scripts::CMDCONVERSE() { }
void Scripts::CMDCHECKFRAME() { }
void Scripts::CMDCHECKANIM() { }
void Scripts::CMDSND() { }
void Scripts::CMDRETNEG() { }
void Scripts::CMDCHECKLOC() { }
void Scripts::CMDSETANIM() { }
void Scripts::CMDDISPINV() { }
void Scripts::CMDSETTIMER() { }
void Scripts::CMDCHECKTIMER() { }
void Scripts::CMDSETTRAVEL() { }
void Scripts::CMDSETVID() { }
void Scripts::CMDPLAYVID() { }
void Scripts::CMDPLOTIMAGE() { }
void Scripts::CMDSETDISPLAY() { }
void Scripts::CMDSETBUFFER() { }
void Scripts::CMDSETSCROLL() { }
void Scripts::CMDSAVERECT() { }
void Scripts::CMDSETBUFVID() { }
void Scripts::CMDPLAYBUFVID() { }
void Scripts::CMDREMOVELAST() { }
void Scripts::CMDSPECIAL() { }
void Scripts::CMDSETCYCLE() { }
void Scripts::CMDCYCLE() { }
void Scripts::CMDCHARSPEAK() { }
void Scripts::CMDTEXSPEAK() { }
void Scripts::CMDTEXCHOICE() { }
void Scripts::CMDWAIT() { }
void Scripts::CMDSETCONPOS() { }
void Scripts::CMDCHECKVFRAME() { }
void Scripts::CMDJUMPCHOICE() { }
void Scripts::CMDRETURNCHOICE() { }
void Scripts::CMDCLEARBLOCK() { }
void Scripts::CMDLOADSOUND() { }
void Scripts::CMDFREESOUND() { }
void Scripts::CMDSETVIDSND() { }
void Scripts::CMDPLAYVIDSND() { }
void Scripts::CMDPUSHLOCATION() { }
void Scripts::CMDPLAYEROFF() { }
void Scripts::CMDPLAYERON() { }
void Scripts::CMDDEAD() { }
void Scripts::CMDFADEOUT() { }
void Scripts::CMDENDVID() { }
void Scripts::CMDHELP() { }
void Scripts::CMDCYCLEBACK() { }
void Scripts::CMDCHAPTER() { }
void Scripts::CMDSETHELP() { }
void Scripts::CMDCENTERPANEL() { }
void Scripts::CMDMAINPANEL() { }
void Scripts::CMDRETFLASH() { }


} // End of namespace Access
