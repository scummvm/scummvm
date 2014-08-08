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

#ifndef ACCESS_SCRIPTS_H
#define ACCESS_SCRIPTS_H

#include "common/scummsys.h"

namespace Access {

class AccessEngine;
class Scripts;

typedef void(Scripts::*ScriptMethodPtr)(const byte *&pScript);

class Scripts {
protected:
	AccessEngine *_vm;

	virtual void executeCommand(int commandIndex, const byte *&pScript);
	void CMDENDOBJECT(const byte *&pScript);
	void CMDJUMPLOOK(const byte *&pScript);
	void CMDJUMPHELP(const byte *&pScript);
	void CMDJUMPGET(const byte *&pScript);
	void CMDJUMPMOVE(const byte *&pScript);
	void CMDJUMPUSE(const byte *&pScript);
	void CMDJUMPTALK(const byte *&pScript);
	void CMDNULL(const byte *&pScript);
	void CMDPRINT(const byte *&pScript);
	void CMDRETPOS(const byte *&pScript);
	void CMDANIM(const byte *&pScript);
	void CMDSETFLAG(const byte *&pScript);
	void CMDCHECKFLAG(const byte *&pScript);
	
	/**
	 * Jump to another script
	 */
	void cmdGoto(const byte *&pScript);
	
	void CMDSETINV(const byte *&pScript);
	void CMDCHECKINV(const byte *&pScript);
	void CMDSETTEX(const byte *&pScript);
	void CMDNEWROOM(const byte *&pScript);
	void CMDCONVERSE(const byte *&pScript);
	void CMDCHECKFRAME(const byte *&pScript);
	void CMDCHECKANIM(const byte *&pScript);
	void CMDSND(const byte *&pScript);
	void CMDRETNEG(const byte *&pScript);
	void CMDCHECKLOC(const byte *&pScript);
	void CMDSETANIM(const byte *&pScript);
	void CMDDISPINV(const byte *&pScript);
	void CMDSETTIMER(const byte *&pScript);
	void CMDCHECKTIMER(const byte *&pScript);
	void CMDSETTRAVEL(const byte *&pScript);
	void CMDSETVID(const byte *&pScript);
	void CMDPLAYVID(const byte *&pScript);
	void CMDPLOTIMAGE(const byte *&pScript);
	void CMDSETDISPLAY(const byte *&pScript);
	void CMDSETBUFFER(const byte *&pScript);
	void CMDSETSCROLL(const byte *&pScript);
	void CMDSAVERECT(const byte *&pScript);
	void CMDSETBUFVID(const byte *&pScript);
	void CMDPLAYBUFVID(const byte *&pScript);
	void CMDREMOVELAST(const byte *&pScript);
	void CMDSPECIAL(const byte *&pScript);
	void CMDSETCYCLE(const byte *&pScript);
	void CMDCYCLE(const byte *&pScript);
	void CMDCHARSPEAK(const byte *&pScript);
	void CMDTEXSPEAK(const byte *&pScript);
	void CMDTEXCHOICE(const byte *&pScript);
	void CMDWAIT(const byte *&pScript);
	void CMDSETCONPOS(const byte *&pScript);
	void CMDCHECKVFRAME(const byte *&pScript);
	void CMDJUMPCHOICE(const byte *&pScript);
	void CMDRETURNCHOICE(const byte *&pScript);
	void CMDCLEARBLOCK(const byte *&pScript);
	void CMDLOADSOUND(const byte *&pScript);
	void CMDFREESOUND(const byte *&pScript);
	void CMDSETVIDSND(const byte *&pScript);
	void CMDPLAYVIDSND(const byte *&pScript);
	void CMDPUSHLOCATION(const byte *&pScript);
	void CMDPLAYEROFF(const byte *&pScript);
	void CMDPLAYERON(const byte *&pScript);
	void CMDDEAD(const byte *&pScript);
	void CMDFADEOUT(const byte *&pScript);
	void CMDENDVID(const byte *&pScript);
	void CMDHELP(const byte *&pScript);
	void CMDCYCLEBACK(const byte *&pScript);
	void CMDCHAPTER(const byte *&pScript);
	void CMDSETHELP(const byte *&pScript);
	void CMDCENTERPANEL(const byte *&pScript);
	void CMDMAINPANEL(const byte *&pScript);
	void CMDRETFLASH(const byte *&pScript);
public:
	const byte *_script;
	const byte *_scriptLoc;
	int _sequence;
	bool _endFlag;
	int _returnCode;
	int _scriptCommand;
public:
	Scripts(AccessEngine *vm);

	virtual ~Scripts();

	void freeScriptData();

	const byte *searchForSequence();
	
	int executeScript();
};

} // End of namespace Access

#endif /* ACCESS_SCRIPTS_H */
