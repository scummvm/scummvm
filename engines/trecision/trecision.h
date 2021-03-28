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

#ifndef TRECISION_TRECISION_H
#define TRECISION_TRECISION_H

#include "engines/engine.h"
#include "common/keyboard.h"
#include "graphics/pixelformat.h"

#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"

namespace Trecision {
class AnimManager;
class GraphicsManager;
class LogicManager;

class TrecisionEngine : public Engine {
private:
	void initMain();
	void initMessageSystem();
	void initNames();
	void LoadAll();

public:
	TrecisionEngine(OSystem *syst);
	~TrecisionEngine() override;

	Common::Error run() override;
	void eventLoop();

	// Inventory
	void refreshInventory(uint8 StartIcon, uint8 StartLine);
	void moveInventoryLeft();
	void moveInventoryRight();
	void setInventoryStart(uint8 StartIcon, uint8 StartLine);
	void doInventory();
	void showInventoryName(uint16 obj, bool showhide);
	uint8 whatIcon(uint16 mx);
	uint8 iconPos(uint8 icon);
	void removeIcon(uint8 icon);
	void addIcon(uint8 icon);
	void replaceIcon(uint8 oldIcon, uint8 newIcon);
	void doInventoryUseWithInventory();
	void doInventoryUseWithScreen();

	// Script
	void EndScript();
	void PlayScript(uint16 i);
	void EvalScript();

	// Utils
	char *getNextSentence();
	void setRoom(uint16 r, bool b);

	// Text
	void addText(uint16 x, uint16 y, const char *sign, uint16 tcol, uint16 scol);
	void clearText();
	void drawString();
	void doClearText();
	void redrawString();

	// Others
	void checkSystem();


	uint16 _curRoom;
	uint16 _oldRoom;
	SRoom _room[MAXROOMS];

	int32 _curSortTableNum;
	uint16 _curScriptFrame[10];

	uint16 _curObj;
	SObject _obj[MAXOBJ];

	uint8 _actionLen[MAXACTION];

	char *_textPtr;
	SDText _sdText, _oldSdText;
	int16 _limits[50][4];
	uint16 _limitsNum;
	int _actorLimit;

	// Inventory
	uint16 _curInventory;
	SInvObject _inventoryObj[MAXINVENTORY];
	uint8 _inventory[MAXICON];
	uint8 _inventorySize;
	uint8 _cyberInventory[MAXICON];
	uint8 _cyberInventorySize;
	uint8 _iconBase;
	uint8 _inventoryStatus;
	uint8 _lightIcon;
	uint8 _inventoryRefreshStartIcon;
	uint8 _inventoryRefreshStartLine;
	uint16 _lastCurInventory;
	uint16 _lastLightIcon;
	int16 _inventoryCounter;
	bool  _flagInventoryLocked;
	int16 _inventorySpeed[8];
	uint8 _inventorySpeedIndex;
	uint32 _inventoryScrollTime;
	uint16 lastinv;
	uint16 lastobj;

	bool _fastWalk;
	bool _fastWalkLocked;
	bool _mouseONOFF;

	// Use With
	uint16 _useWith[2];
	bool _useWithInv[2];

	// Messages
	const char *_objName[MAXOBJNAME];
	const char *_sentence[MAXSENTENCE];
	const char *_sysText[MAXSYSTEXT];

	// Message system
	Message _gameMsg[MAXMESSAGE];
	Message _characterMsg[MAXMESSAGE];
	Message _animMsg[MAXMESSAGE];
	Message _idleMsg;
	Message *_curMessage;
	MessageQueue _gameQueue;
	MessageQueue _animQueue;
	MessageQueue _characterQueue;
	// Snake management
	Message _snake52;

	uint16 _newData[260];
	uint32 _newData2[260];
	uint16 *_video2;

	uint32 _nextRefresh;

	int16 _mouseX, _mouseY;
	int16 _oldMouseX, _oldMouseY;
	bool _mouseLeftBtn, _mouseRightBtn;
	Common::KeyCode _curKey;
	uint16 _curAscii;
	bool _keybInput;

	bool _gamePaused = false;
	// CloseUp12 and 13
	uint16 _closeUpObj;

	bool _flagMouseEnabled;

	bool _flagscriptactive;
	SScriptFrame _scriptFrame[MAXSCRIPTFRAME];
	SScript _script[MAXSCRIPT];
	uint8 _curStack;

	// Text
	int16 _textStackTop;
	StackText _textStack[MAXTEXTSTACK];

	// SlotMachine41
	uint16 _slotMachine41Counter;

	//Dialogs
	DialogChoice _choice[MAXCHOICE];

	bool _flagScreenRefreshed;

	// special management
	uint16 _wheel;
	uint16 _wheelPos[3];

	AnimManager *_animMgr;
	GraphicsManager *_graphicsMgr;
	LogicManager *_logicMgr;

	uint8 *Font;
	uint16 *Arrows;
	uint8 *TextureArea;
	uint16 *Icone;
};

extern TrecisionEngine *g_vm;

} // End of namespace Trecision

#endif
