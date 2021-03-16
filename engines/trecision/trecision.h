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

#include "graphics/pixelformat.h"

#include "trecision/nl/message.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"

namespace Trecision {
class GraphicsManager;
class AnimManager;

class TrecisionEngine : public Engine {
private:
	void initMain();
	void initMessageSystem();
	void initNames();
	void initInventory();
	
public:
	TrecisionEngine(OSystem *syst);
	~TrecisionEngine() override;

	Common::Error run() override;
	void EventLoop();
	void refreshInventory(uint8 StartIcon, uint8 StartLine);
	void moveInventoryLeft();
	void moveInventoryRight();
	void RegenInv(uint8 StartIcon, uint8 StartLine);

	uint16 _curRoom;
	uint16 _oldRoom;
	uint16 _curInventory;
	int32 _curSortTableNum;
	uint16 _curObj;
	uint16 CurScriptFrame[10];

	SRoom _room[MAXROOMS];
	SObject _obj[MAXOBJ];
	SInvObject _inventoryObj[MAXINVENTORY];
	uint8 _actionLen[MAXACTION];

	SDText sdt, osdt;
	int16 _limits[50][4];
	uint16 _limitsNum;
	int _actorLimit;

	// Inventory
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

	// Snake management
	Message _snake52;
	Message *_curMessage;

	MessageQueue _gameQueue;
	MessageQueue _animQueue;
	MessageQueue _characterQueue;

	uint16 _newData[260];
	uint32 _newData2[260];
	uint16 *_video2;

	uint32 NextRefresh;

	int CurKey, CurAscii;
	int16 wmx, wmy;
	bool wmleft, wmright;
	int16 omx, omy;
	bool KeybInput;

	bool _gamePaused = false;

	GraphicsManager *_graphicsMgr;
	AnimManager *_animMgr;
};

extern TrecisionEngine *g_vm;

} // End of namespace Trecision

#endif
