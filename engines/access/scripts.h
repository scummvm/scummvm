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

#ifndef ACCESS_SCRIPTS_H
#define ACCESS_SCRIPTS_H

#include "common/scummsys.h"
#include "common/memstream.h"
#include "access/data.h"

namespace Access {

class AccessEngine;
class Scripts;

#define SCRIPT_START_BYTE 0xE0
#define ROOM_SCRIPT 2000

typedef void(Scripts::*ScriptMethodPtr)();

class Scripts : public Manager {
private:
	int _specialFunction;

	void clearWatch();
	void printWatch();

protected:
	Resource *_resource;
	Common::SeekableReadStream *_data;
	ScriptMethodPtr COMMAND_LIST[100];

	virtual void executeSpecial(int commandIndex, int param1, int param2) = 0;
	virtual void executeCommand(int commandIndex);

	void charLoop();

	/**
	 * Read a null terminated string from the script
	 */
	Common::String readString();

	void cmdObject();
	void cmdEndObject();
	void cmdJumpLook();
	void cmdJumpHelp();
	void cmdJumpGet();
	void cmdJumpMove();
	void cmdJumpUse();
	void cmdJumpTalk();
	void cmdNull();
	void cmdPrint_v1();
	void cmdPrint_v2();
	void cmdAnim();
	void cmdSetFlag();
	void cmdCheckFlag();

	/**
	 * Jump to another script
	 */
	void cmdGoto();

	void cmdAddScore();
	void cmdSetInventory();
	void cmdCheckInventory();
	void cmdSetTex();
	void cmdNewRoom();
	void cmdConverse();
	void cmdCheckFrame();
	void cmdCheckAnim();
	void cmdSnd();
	void cmdRetNeg();
	void cmdCheckLoc();
	void cmdSetAnim();
	void cmdDispInv_v1();
	void cmdDispInv_v2();
	void cmdSetAbout();
	void cmdSetTimer();
	void cmdCheckTimer();
	void cmdJumpGoto();
	void cmdSetTravel();
	void cmdSetVideo();
	void cmdPlayVideo();
	void cmdPlotImage();
	void cmdSetDisplay();
	void cmdSetBuffer();
	void cmdSetScroll();
	void cmdSaveRect();
	void cmdVideoEnded();
	void cmdSetBufVid();
	void cmdPlayBufVid();
	void cmdRemoveLast();
	void cmdDoTravel();
	void cmdCheckAbout();
	void cmdSpecial();
	void cmdSetCycle();
	void cmdCycle();
	void cmdCharSpeak();
	void cmdTexSpeak();
	void cmdTexChoice();
	void cmdWait();
	void cmdSetConPos();
	void cmdCheckVFrame();
	void cmdJumpChoice();
	void cmdReturnChoice();
	void cmdClearBlock();
	void cmdLoadSound();
	void cmdSetVideoSound();
	void cmdPlayVideoSound();
	void cmdPrintWatch();
	void cmdDispAbout();
	void cmdPushLocation();
	void cmdCheckTravel();
	void cmdBlock();
	void cmdPlayerOff();
	void cmdPlayerOn();
	void cmdDead();
	void cmdFadeOut();
	void cmdEndVideo();
	void cmdHelp_v1();
	void cmdHelp_v2();
	void cmdCycleBack();
	void cmdSetHelp();
public:
	int _sequence;
	bool _endFlag;
	int _returnCode;
	int _scriptCommand;
	int _choice;
	int32 _choiceStart;
	Common::Point _charsOrg, _texsOrg;

public:
	Scripts(AccessEngine *vm);

	virtual ~Scripts();

	void setOpcodes();
	void setOpcodes_v2();

	void setScript(Resource *data, bool restartFlag = false);

	void freeScriptData();

	void searchForSequence();

	int executeScript();

	void findNull();
	void doCmdPrint_v1(Common::String msg);

	/**
	 * Print a given message to the screen in a bubble box
	 */
	void printString(const Common::String &msg);

	// Script commands that need to be public
	void cmdFreeSound();
	void cmdRetPos();
	void converse1(int val);
};

} // End of namespace Access

#endif /* ACCESS_SCRIPTS_H */
