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
#include "common/memstream.h"

namespace Access {

class AccessEngine;
class Scripts;

typedef void(Scripts::*ScriptMethodPtr)();

class Scripts {
private:
	const byte *_rawData;
	Common::MemoryReadStream *_data;
protected:
	AccessEngine *_vm;

	virtual void executeCommand(int commandIndex);
	void CMDOBJECT();
	void CMDENDOBJECT();
	void cmdJumpLook();
	void cmdJumpHelp();
	void cmdJumpGet();
	void cmdJumpMove();
	void cmdJumpUse();
	void cmdJumpTalk();
	void cmdNull();
	void CMDPRINT();
	void cmdRetPos();
	void cmdAnim();
	void cmdSetFlag();
	void cmdCheckFlag();
	
	/**
	 * Jump to another script
	 */
	void cmdGoto();
	
	void cmdSetInventory();
	void cmdCheckInventory();
	void CMDSETTEX();
	void CMDNEWROOM();
	void CMDCONVERSE();
	void cmdCheckFrame();
	void cmdCheckAnim();
	void cmdSnd();
	void cmdRetNeg();
	void cmdCheckLoc();
	void cmdSetAnim();
	void CMDDISPINV();
	void CMDSETTIMER();
	void CMDCHECKTIMER();
	void cmdSetTravel();
	void CMDSETVID();
	void CMDPLAYVID();
	void cmdPlotImage();
	void cmdSetDisplay();
	void cmdSetBuffer();
	void cmdSetScroll();
	void CMDSAVERECT();
	void CMDSETBUFVID();
	void CMDPLAYBUFVID();
	void cmdRemoveLast();
	void CMDSPECIAL();
	void CMDSETCYCLE();
	void CMDCYCLE();
	void CMDCHARSPEAK();
	void CMDTEXSPEAK();
	void CMDTEXCHOICE();
	void CMDWAIT();
	void CMDSETCONPOS();
	void CMDCHECKVFRAME();
	void cmdJumpChoice();
	void cmdReturnChoice();
	void CMDCLEARBLOCK();
	void CMDLOADSOUND();
	void CMDFREESOUND();
	void CMDSETVIDSND();
	void CMDPLAYVIDSND();
	void CMDPUSHLOCATION();
	void cmdPlayerOff();
	void cmdPlayerOn();
	void CMDDEAD();
	void cmdFadeOut();
	void CMDENDVID();
	void CMDHELP();
	void CMDCYCLEBACK();
	void CMDCHAPTER();
	void CMDSETHELP();
	void CMDCENTERPANEL();
	void cmdMainPanel();
	void CMDRETFLASH();
public:
	int _sequence;
	bool _endFlag;
	int _returnCode;
	int _scriptCommand;
	int _choice;
	int32 _choiceStart;
public:
	Scripts(AccessEngine *vm);

	virtual ~Scripts();

	void setScript(const byte *data, int size);

	void freeScriptData();

	void searchForSequence();
	
	int executeScript();
};

} // End of namespace Access

#endif /* ACCESS_SCRIPTS_H */
