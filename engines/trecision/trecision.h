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
#include "common/serializer.h"
#include "graphics/surface.h"

#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/struct.h"
#include "trecision/fastfile.h"
#include "trecision/video.h"

namespace Trecision {
class AnimManager;
class GraphicsManager;
class LogicManager;

// Temporary define for the new video code
// TODOs:
// - Fix occasional palette glitches after seeking
#define USE_NEW_VIDEO_CODE 0

#define USE_DIRTY_RECTS 0
#define SAVE_VERSION_ORIGINAL 102
#define SAVE_VERSION_SCUMMVM 103

enum TrecisionMessageIds {
	kMessageSavePosition = 9,
	kMessageEmptySpot = 10,
	kMessageLoadPosition = 11,
	kMessageConfirmExit = 13,
	kMessageDemoOver = 17,
	kMessageError = 19,
	kMessageUse = 23,
	kMessageWith = 24,
	kMessageGoto = 25,
	kMessageGoto2 = 26
};

class TrecisionEngine : public Engine {
	void initMain();
	void initMessageSystem();
	void openDataFiles();
	void initNames();
	void LoadAll();

public:
	TrecisionEngine(OSystem *syst);
	~TrecisionEngine() override;

	// ScummVM
	Common::Error run() override;
	void eventLoop();
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return true; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	bool syncGameStream(Common::Serializer &ser);

	// Data files
	byte *readData(Common::String fileName);
	uint16 *readData16(Common::String fileName, int &size);

	// Inventory
	void refreshInventory(uint8 startIcon, uint8 startLine);
	void moveInventoryLeft();
	void moveInventoryRight();
	void setInventoryStart(uint8 startIcon, uint8 startLine);
	void doInventory();
	void showInventoryName(uint16 obj, bool showhide);
	uint8 whatIcon(uint16 mx);
	uint8 iconPos(uint8 icon);
	void removeIcon(uint8 icon);
	void addIcon(uint8 icon);
	void replaceIcon(uint8 oldIcon, uint8 newIcon);
	void doInventoryUseWithInventory();
	void doInventoryUseWithScreen();
	void rollInventory(uint8 status);
	void doScrollInventory(uint16 mouseX);

	// Script
	void endScript();
	void playScript(uint16 id);
	void evalScript();

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
	void initCursor();

	Graphics::Surface _thumbnail;

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
	Common::Rect _limits[50];
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
	uint16 _lastInv;
	uint16 _lastObj;

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

	uint16 *_screenBuffer;
	uint16 *_smackImageBuffer;

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

	//Dialogs
	DialogChoice _choice[MAXCHOICE];

	bool _flagScreenRefreshed;

	AnimManager *_animMgr;
	GraphicsManager *_graphicsMgr;
	LogicManager *_logicMgr;

	uint8 *_font;
	uint16 *_arrows;
	uint8 *_textureArea;
	uint16 *_icons;
	int16 *_zBuffer;

	SActor *_actor;

	// Data files
	FastFile _dataFile;	// nldata.cd0
	FastFile _speechFile;	// nlspeech.cd0

	bool _flagDialogActive;
	bool _flagDialogMenuActive;
	bool _flagSkipTalk;
	bool _flagSkipEnable;
	bool _flagPaintCharacter;
	bool _flagShowCharacter;
	bool _flagSomeoneSpeaks;
	bool _flagCharacterSpeak;
	bool _flagUseWithStarted;
	bool FlagUseWithLocked;
	bool _flagCharacterExists;
	bool FlagWalkNoInterrupt;
	bool _flagNoPaintScreen;
	bool _flagWaitRegen;
};

extern TrecisionEngine *g_vm;

} // End of namespace Trecision

#endif
