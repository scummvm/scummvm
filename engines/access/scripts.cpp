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

const byte *Scripts::searchForSequence() {
	assert(_script);
	const byte *pSrc = _script;
	int sequenceId;
	do {
		while (*pSrc++ != SCRIPT_START_BYTE) ;
		sequenceId = READ_LE_UINT16(pSrc);
		pSrc += 2;
	} while (sequenceId != _sequence);

	_scriptLoc = pSrc;
	return pSrc;
}

int Scripts::executeScript() {
	assert(_scriptLoc);
	_endFlag = 0;
	_returnCode = 0;

	do {
		const byte *pSrc = _scriptLoc;
		for (pSrc = _scriptLoc; *pSrc == SCRIPT_START_BYTE; pSrc += 3) ;	
		_scriptCommand = *pSrc++;

		executeCommand(_scriptCommand - 0x80, pSrc);
		_scriptLoc = pSrc;
	} while (!_endFlag);

	return _returnCode;
}

void Scripts::executeCommand(int commandIndex, const byte *&pScript) {
	static const ScriptMethodPtr COMMAND_LIST[] = {
		&Scripts::CMDENDOBJECT, &Scripts::cmdJumpLook, &Scripts::cmdJumpHelp, &Scripts::cmdJumpGet, &Scripts::cmdJumpMove,
		&Scripts::cmdJumpUse, &Scripts::cmdJumpTalk, &Scripts::CMDNULL, &Scripts::CMDPRINT, &Scripts::CMDRETPOS, &Scripts::CMDANIM,
		&Scripts::cmdSetFlag, &Scripts::CMDCHECKFLAG, &Scripts::cmdGoto, &Scripts::CMDSETINV, &Scripts::CMDSETINV,
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

	(this->*COMMAND_LIST[commandIndex])(pScript);
}

void Scripts::CMDENDOBJECT(const byte *&pScript) { }

void Scripts::cmdJumpLook(const byte *&pScript) {
	if (_vm->_selectCommand == 0)
		cmdGoto(pScript);
	else
		pScript += 2;
}

void Scripts::cmdJumpHelp(const byte *&pScript) { 
	if (_vm->_selectCommand == 8)
		cmdGoto(pScript);
	else
		pScript += 2;
}

void Scripts::cmdJumpGet(const byte *&pScript) { 
	if (_vm->_selectCommand == 3)
		cmdGoto(pScript);
	else
		pScript += 2;
}

void Scripts::cmdJumpMove(const byte *&pScript) { 
	if (_vm->_selectCommand == 2)
		cmdGoto(pScript);
	else
		pScript += 2;
}

void Scripts::cmdJumpUse(const byte *&pScript) { 
	if (_vm->_selectCommand == 4)
		cmdGoto(pScript);
	else
		pScript += 2;
}

void Scripts::cmdJumpTalk(const byte *&pScript) { 
	if (_vm->_selectCommand == 6)
		cmdGoto(pScript);
	else
		pScript += 2;
}

void Scripts::CMDNULL(const byte *&pScript) { }
void Scripts::CMDPRINT(const byte *&pScript) { }
void Scripts::CMDRETPOS(const byte *&pScript) { }
void Scripts::CMDANIM(const byte *&pScript) { }

void Scripts::cmdSetFlag(const byte *&pScript) { 
	int flagNum = *pScript++;
	byte flagVal = *pScript++;
	assert(flagNum < 100);
	_vm->_flags[flagNum] = flagVal;
}

void Scripts::CMDCHECKFLAG(const byte *&pScript) { 
	int flagNum = READ_LE_UINT16(pScript);
	int flagVal = READ_LE_UINT16(pScript + 2);
	pScript += 4;
	assert(flagNum < 100);

	if (_vm->_flags[flagNum] == (flagVal & 0xff))
		cmdGoto(pScript);
	else
		pScript += 2;
}

void Scripts::cmdGoto(const byte *&pScript) { 
	_sequence = READ_LE_UINT16(pScript);
	pScript = searchForSequence();
}

void Scripts::CMDSETINV(const byte *&pScript) { }
void Scripts::CMDCHECKINV(const byte *&pScript) { }
void Scripts::CMDSETTEX(const byte *&pScript) { }
void Scripts::CMDNEWROOM(const byte *&pScript) { }
void Scripts::CMDCONVERSE(const byte *&pScript) { }
void Scripts::CMDCHECKFRAME(const byte *&pScript) { }
void Scripts::CMDCHECKANIM(const byte *&pScript) { }
void Scripts::CMDSND(const byte *&pScript) { }
void Scripts::CMDRETNEG(const byte *&pScript) { }
void Scripts::CMDCHECKLOC(const byte *&pScript) { }
void Scripts::CMDSETANIM(const byte *&pScript) { }
void Scripts::CMDDISPINV(const byte *&pScript) { }
void Scripts::CMDSETTIMER(const byte *&pScript) { }
void Scripts::CMDCHECKTIMER(const byte *&pScript) { }
void Scripts::CMDSETTRAVEL(const byte *&pScript) { }
void Scripts::CMDSETVID(const byte *&pScript) { }
void Scripts::CMDPLAYVID(const byte *&pScript) { }
void Scripts::CMDPLOTIMAGE(const byte *&pScript) { }
void Scripts::CMDSETDISPLAY(const byte *&pScript) { }
void Scripts::CMDSETBUFFER(const byte *&pScript) { }
void Scripts::CMDSETSCROLL(const byte *&pScript) { }
void Scripts::CMDSAVERECT(const byte *&pScript) { }
void Scripts::CMDSETBUFVID(const byte *&pScript) { }
void Scripts::CMDPLAYBUFVID(const byte *&pScript) { }
void Scripts::CMDREMOVELAST(const byte *&pScript) { }
void Scripts::CMDSPECIAL(const byte *&pScript) { }
void Scripts::CMDSETCYCLE(const byte *&pScript) { }
void Scripts::CMDCYCLE(const byte *&pScript) { }
void Scripts::CMDCHARSPEAK(const byte *&pScript) { }
void Scripts::CMDTEXSPEAK(const byte *&pScript) { }
void Scripts::CMDTEXCHOICE(const byte *&pScript) { }
void Scripts::CMDWAIT(const byte *&pScript) { }
void Scripts::CMDSETCONPOS(const byte *&pScript) { }
void Scripts::CMDCHECKVFRAME(const byte *&pScript) { }
void Scripts::CMDJUMPCHOICE(const byte *&pScript) { }
void Scripts::CMDRETURNCHOICE(const byte *&pScript) { }
void Scripts::CMDCLEARBLOCK(const byte *&pScript) { }
void Scripts::CMDLOADSOUND(const byte *&pScript) { }
void Scripts::CMDFREESOUND(const byte *&pScript) { }
void Scripts::CMDSETVIDSND(const byte *&pScript) { }
void Scripts::CMDPLAYVIDSND(const byte *&pScript) { }
void Scripts::CMDPUSHLOCATION(const byte *&pScript) { }
void Scripts::CMDPLAYEROFF(const byte *&pScript) { }
void Scripts::CMDPLAYERON(const byte *&pScript) { }
void Scripts::CMDDEAD(const byte *&pScript) { }
void Scripts::CMDFADEOUT(const byte *&pScript) { }
void Scripts::CMDENDVID(const byte *&pScript) { }
void Scripts::CMDHELP(const byte *&pScript) { }
void Scripts::CMDCYCLEBACK(const byte *&pScript) { }
void Scripts::CMDCHAPTER(const byte *&pScript) { }
void Scripts::CMDSETHELP(const byte *&pScript) { }
void Scripts::CMDCENTERPANEL(const byte *&pScript) { }
void Scripts::CMDMAINPANEL(const byte *&pScript) { }
void Scripts::CMDRETFLASH(const byte *&pScript) { }


} // End of namespace Access
