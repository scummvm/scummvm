/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#define INIT_ROOM_SCRIPT 1000
#define ROOM_SCRIPT 2000

typedef void(Scripts::*ScriptMethodPtr)();

class Scripts : public Manager {
	friend class Debugger;

private:
	int _specialFunction;

	void clearWatch();
	void printWatch();

protected:
	Resource *_resource;
	Common::SeekableReadStream *_data;
	ScriptMethodPtr COMMAND_LIST[100];

	virtual bool executeSpecial(int commandIndex, int param1, int param2) = 0;
	virtual void executeCommand(int commandIndex);

	void charLoop();

	void cmdObject();
	void cmdEndObject();
	void cmdEndObject_v3();
	void cmdJumpLook();
	void cmdJumpOpen();
	void cmdJumpHelp();
	void cmdJumpGet();
	void cmdJumpMove();
	void cmdJumpUse();
	void cmdJumpUse_v3();
	void cmdJumpTalk();
	void cmdJumpTalk_v3();
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
	void cmdSetPlayerCoords();
	void cmdNewRoom();
	void cmdConverse();
	void cmdCheckFrame();
	void cmdCheckAnim();
	void cmdSnd();
	void cmdRetNeg();
	void cmdBD();
	void cmdCheckLoc();
	void cmdSetAnim();
	void cmdDispInv_v1();
	void cmdDispInv_v2();
	void cmdSetAbout();
	void cmdSetTimer();
	void cmdCheckTimer();
	void cmdJumpGoto();
	void cmdJumpGoto_v3();
	void cmdSetTravel();
	void cmdSetVideo_v1();
	void cmdSetVideo_v3();
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
	void cmdDispAbout_v3();
	void cmdPushLocation();
	void cmdPushLocation_v1();
	void cmdCheckTravel();
	void cmdBlock();
	void cmdPlayerOff();
	void cmdPlayerOn();
	void cmdDead();
	void cmdFadeOut();
	void cmdFadeOut_v3();
	void cmdEndVideo();
	void cmdHelp_v1();
	//void cmdHelp_v2(); // only in AmazonScripts
	void cmdCycleBack();
	void cmdSetHelp();
	void cmdDoTravel_Noct();
	void cmdDigitalPlay();
	void cmdFillSound();
	void cmdPlayVid1();
	void cmdCharWait();
	void cmdUndoText();
	void cmdResetAnim();
	void cmdWalkTo();
	void cmdWalkCheck();
	void cmdSoundEnd();
	void cmdFadeWhite();
	void cmdGotoFrame();
	void cmdPlayerScale();
	void cmdRestoreBlock();
	void cmdCopyScnBuf();
	void cmdStilWalkTo();
	void cmdStilWalkCheck();
	void cmdStilOff();
	void cmdStilOn();
	void cmdReturnExit();
	void cmdSetCoords();
	void cmdSetStilCoords();
	void cmdSetPlayerDir();
	void cmdSetStilDir();
	void cmdStilScale();
	void cmdLockInterface();
	void cmdUnlockInterface();
	void cmdCharSpeak_v3();
	void cmdPlayerSpeak();
	void cmdPlayerChoice();

public:
	int _sequence;
	bool _endFlag;
	int _returnCode;
	int _scriptCommand;
	int _choice;
	int32 _choiceStart;
	Common::Point _charsOrg, _texsOrg;

	int _continuenceType;  // Only used in Noctropolis
	bool _continuenceFlag; // Only set in Noctropolis (always false otherwise). See also room->_conFlag which is used in earlier games.

public:
	Scripts(AccessEngine *vm);

	virtual ~Scripts();

	void setOpcodes();
	void setOpcodes_v2();
	void setOpcodes_v3();

	void setScript(Resource *data, bool restartFlag = false);

	void freeScriptData();

	virtual void searchForSequence();

	int executeScript();

	void findNull();
	void doCmdPrint_v1(const Common::String &msg);


	/**
	 * Print a given message to the screen in a bubble box
	 */
	void printString(const Common::String &msg, Common::Point pt = Common::Point(-1, -1));

	// Script commands that need to be public
	void cmdFreeSound();
	void cmdRetPos();
	void converse1(int val);
};

} // End of namespace Access

#endif /* ACCESS_SCRIPTS_H */
