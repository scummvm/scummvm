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

typedef void(Scripts::*ScriptMethodPtr)();

class Scripts {
protected:
	AccessEngine *_vm;

	virtual void executeCommand(int commandIndex);
	void CMDENDOBJECT();
	void CMDJUMPLOOK();
	void CMDJUMPHELP();
	void CMDJUMPGET();
	void CMDJUMPMOVE();
	void CMDJUMPUSE();
	void CMDJUMPTALK();
	void CMDNULL();
	void CMDPRINT();
	void CMDRETPOS();
	void CMDANIM();
	void CMDSETFLAG();
	void CMDCHECKFLAG();
	void CMDGOTO();
	void CMDSETINV();
	void CMDCHECKINV();
	void CMDSETTEX();
	void CMDNEWROOM();
	void CMDCONVERSE();
	void CMDCHECKFRAME();
	void CMDCHECKANIM();
	void CMDSND();
	void CMDRETNEG();
	void CMDCHECKLOC();
	void CMDSETANIM();
	void CMDDISPINV();
	void CMDSETTIMER();
	void CMDCHECKTIMER();
	void CMDSETTRAVEL();
	void CMDSETVID();
	void CMDPLAYVID();
	void CMDPLOTIMAGE();
	void CMDSETDISPLAY();
	void CMDSETBUFFER();
	void CMDSETSCROLL();
	void CMDSAVERECT();
	void CMDSETBUFVID();
	void CMDPLAYBUFVID();
	void CMDREMOVELAST();
	void CMDSPECIAL();
	void CMDSETCYCLE();
	void CMDCYCLE();
	void CMDCHARSPEAK();
	void CMDTEXSPEAK();
	void CMDTEXCHOICE();
	void CMDWAIT();
	void CMDSETCONPOS();
	void CMDCHECKVFRAME();
	void CMDJUMPCHOICE();
	void CMDRETURNCHOICE();
	void CMDCLEARBLOCK();
	void CMDLOADSOUND();
	void CMDFREESOUND();
	void CMDSETVIDSND();
	void CMDPLAYVIDSND();
	void CMDPUSHLOCATION();
	void CMDPLAYEROFF();
	void CMDPLAYERON();
	void CMDDEAD();
	void CMDFADEOUT();
	void CMDENDVID();
	void CMDHELP();
	void CMDCYCLEBACK();
	void CMDCHAPTER();
	void CMDSETHELP();
	void CMDCENTERPANEL();
	void CMDMAINPANEL();
	void CMDRETFLASH();
public:
	byte *_script;
	byte *_scriptLoc;
	int _sequence;
	bool _endFlag;
	int _returnCode;
	int _scriptCommand;
public:
	Scripts(AccessEngine *vm);

	virtual ~Scripts();

	void freeScriptData();

	void searchForSequence();
	
	int executeScript();
};

} // End of namespace Access

#endif /* ACCESS_SCRIPTS_H */
