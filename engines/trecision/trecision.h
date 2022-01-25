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

#ifndef TRECISION_TRECISION_H
#define TRECISION_TRECISION_H

#include "common/str-array.h"
#include "common/keyboard.h"
#include "common/str.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "graphics/surface.h"

#include "trecision/defines.h"
#include "trecision/fastfile.h"
#include "trecision/struct.h"
#include "trecision/scheduler.h"

namespace Trecision {
class AnimManager;
class DialogManager;
class GraphicsManager;
class LogicManager;
class SoundManager;
class Actor;
class Renderer3D;
class PathFinding3D;
class TextManager;
class Scheduler;
class AnimTypeManager;

// Saved game versions
// Version history:
// - 102: Original PC full version
// - 103: Original PC demo version
// - 110: First ScummVM version
#define SAVE_VERSION_ORIGINAL_MIN 102
#define SAVE_VERSION_ORIGINAL_MAX 109
#define SAVE_VERSION_SCUMMVM_MIN 110
#define SAVE_VERSION_SCUMMVM 110

#define MAXROOMS         	  100            // Game rooms
#define MAXOBJ          	  1400           // Game objects
#define MAXINVENTORY    	  150            // Inventory Items
#define MAXSAVEFILE		12

enum TrecisionGameId {
	GID_ArkOfTime = 0,
	GID_NightLong = 1
};

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

typedef Common::List<Common::Rect>::iterator DirtyRectsIterator;

struct ElevatorAction {
	uint16 dialog;
	uint16 choice;
	uint16 action;
	uint16 newRoom;
};

struct ObjectGraphics {
	uint16 *buf;
	uint8 *mask;
};

class TrecisionEngine : public Engine {
	void initMain();
	void loadAll();
	void loadSaveSlots(Common::StringArray &saveNames);
	void eventLoop();

	// Inventory
	void refreshInventory(uint8 startIcon, uint8 startLine);
	void moveInventoryLeft();
	void moveInventoryRight();
	void syncInventory(Common::Serializer &ser);
	void rollInventory(uint8 status);
	void doScrollInventory(Common::Point pos);
	void endUseWith();

	// Script
	void endScript();
	void evalScript();
	void processScriptFrame();
	void doAction();
	void doMouse();
	void processMouseMovement();
	void doCharacter();
	void doIdle();
	void doRoomIn(uint16 curObj);
	void doRoomOut(uint16 curObj);
	void doMouseExamine(uint16 curObj);
	void doMouseOperate(uint16 curObj);
	void doMouseTake(uint16 curObj);
	void doUseWith();
	void doScreenUseWithScreen();
	void doInvExamine();
	void doInvOperate();
	void doScript();
	void processCurrentMessage();

	// Utils
	char *getNextSentence();
	uint16 getKey();
	void processTime();
	void processMouse();
	static bool isBetween(int a, int x, int b);

	// Others
	bool canPlayerInteract();

	// Objects
	void readObj(Common::SeekableReadStream *stream);
	void readObject(Common::SeekableReadStream *stream, uint16 objIndex, uint16 objectId);

	TrecisionGameId _gameId;

	char *_textArea;
	uint16 _curScriptFrame[10];
	char *_textPtr;

	uint16 _curAscii;
	bool _keybInput;
	bool _gamePaused;
	uint8 _curStack;

	Common::List<SSortTable> _sortTableReplay;

public:
	TrecisionEngine(OSystem *syst, const ADGameDescription *desc);
	~TrecisionEngine() override;

	// ScummVM
	Common::Error run() override;
	TrecisionGameId getGameId() const { return _gameId; }
	bool isDemo() const { return _gameDescription->flags & ADGF_DEMO; }
	bool isAmiga() const { return _gameDescription->platform == Common::kPlatformAmiga; }
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return canPlayerInteract() && _curRoom != kRoomIntro; }
	bool canSaveGameStateCurrently() override { return canPlayerInteract() && _curRoom != kRoomIntro; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	bool syncGameStream(Common::Serializer &ser);

	// Data files
	Common::SeekableReadStreamEndian *readEndian(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::YES);
	void read3D(const Common::String &c);

	// Inventory
	void setInventoryStart(uint8 startIcon, uint8 startLine);
	void showInventoryName(uint16 obj, bool showhide);
	void showIconName();
	uint8 whatIcon(Common::Point pos);
	int8 iconPos(uint8 icon);
	void removeIcon(uint8 icon);
	void addIcon(uint8 icon);
	void replaceIcon(uint8 oldIcon, uint8 newIcon);
	void openInventory();
	void closeInventory();
	void closeInventoryImmediately();
	void useItem();
	void examineItem();
	void clearUseWith();

	// Script
	void playScript(uint16 id);
	bool quitPrompt();
	void demoOver();
	void startCharacterAction(uint16 action, uint16 newRoom, uint8 newPos, uint16 sent);
	void doMouseTalk(uint16 curObj);
	void changeRoom(uint16 room, uint16 action = 0, byte position = 0);

	// Utils
	uint16 textLength(const Common::String &text, uint16 begin = 0, uint16 end = 0);
	Common::KeyCode waitKey();
	void waitDelay(uint32 val);
	void freeKey();
	uint32 readTime();
	bool checkMask(Common::Point pos);
	float sinCosAngle(float sinus, float cosinus);
	float dist2D(float x1, float y1, float x2, float y2);
	float dist3D(float x1, float y1, float z1, float x2, float y2, float z2);
	static bool isGameArea(Common::Point pos);
	static bool isInventoryArea(Common::Point pos);
	static bool isIconArea(Common::Point pos);
	int getRoomObjectIndex(uint16 objectId);
	int floatComp(float f1, float f2) const;

	// Others
	void checkSystem();
	bool dataSave();
	bool dataLoad();
	void reEvent();

	// Objects
	void setObjectVisible(uint16 objectId, bool visible);
	void refreshObject(uint16 objectId);
	bool isObjectVisible(uint16 objectId);
	void setObjectAnim(uint16 objectId, uint16 animId);
	void redrawRoom();
	void readLoc();
	Common::SeekableReadStreamEndian *getLocStream();
	void tendIn();
	void readExtraObj2C();
	void readPositionerSnapshots();

	// Data files
	byte *readData(const Common::String &fileName);

	const ADGameDescription *_gameDescription;

	Graphics::Surface _thumbnail;

	uint16 _curRoom;
	uint16 _oldRoom;
	SRoom _room[MAXROOMS];

	Common::List<SSortTable> _sortTable;

	uint16 _curObj;
	SObject _obj[MAXOBJ];

	SDText _drawText;

	// Inventory
	uint16 _curInventory;
	SInvObject _inventoryObj[MAXINVENTORY];
	Common::Array<byte> _inventory;
	Common::Array<byte> _cyberInventory;
	uint8 _iconBase;
	uint8 _inventoryStatus;
	uint8 _lightIcon;
	uint8 _inventoryRefreshStartIcon;
	uint8 _inventoryRefreshStartLine;
	int16 _inventoryCounter;
	bool  _flagInventoryLocked;
	uint8 _inventorySpeedIndex;
	uint32 _inventoryScrollTime;
	uint16 _lastInv;
	uint16 _lastObj;

	bool _fastWalk;

	// Use With
	uint16 _useWith[2];
	bool _useWithInv[2];

	// Messages
	const char *_objName[MAXOBJNAME];
	const char *_sentence[MAXSENTENCE];
	const char *_sysText[MAXSYSTEXT];

	// Message system
	Message *_curMessage;
	// Snake management
	Message _snake52;

	uint32 _nextRefresh;

	Common::Point _mousePos;
	bool _mouseMoved, _mouseLeftBtn, _mouseRightBtn;
	Common::KeyCode _curKey;

	bool _flagScriptActive;
	SScriptFrame _scriptFrame[MAXSCRIPTFRAME];
	uint16 _scriptFirstFrame[MAXSCRIPT];
	
	AnimManager *_animMgr;
	GraphicsManager *_graphicsMgr;
	DialogManager *_dialogMgr;
	LogicManager *_logicMgr;
	SoundManager *_soundMgr;
	Renderer3D *_renderer;
	PathFinding3D *_pathFind;
	TextManager *_textMgr;
	Scheduler *_scheduler;
	AnimTypeManager *_animTypeMgr;

	Actor *_actor;

	// Data files
	FastFile _dataFile;	// nldata.cd0

	bool _flagDialogActive;
	bool _flagDialogMenuActive;
	bool _flagSkipTalk;
	bool _flagPaintCharacter;
	bool _flagShowCharacter;
	bool _flagSomeoneSpeaks;
	bool _flagCharacterSpeak;
	bool _flagUseWithStarted;
	bool _flagNoPaintScreen;
	bool _flagWaitRegen;

	ObjectGraphics _objectGraphics[MAXOBJINROOM];

	uint32 _curTime;
	uint32 _characterSpeakTime;

	int _cx, _cy;

	uint8 _textStatus;

	uint32 _pauseStartTime;
};

uint8 static const defActionLen[hLAST + 1] = {
	/* STAND */ 1,
	/* PARTE */ 1,
	/* WALK  */ 10,
	/* END   */ 1,
	/* STOP0 */ 3,
	/* STOP1 */ 4,
	/* STOP2 */ 3,
	/* STOP3 */ 2,
	/* STOP4 */ 3,
	/* STOP5 */ 4,
	/* STOP6 */ 3,
	/* STOP7 */ 3,
	/* STOP8 */ 2,
	/* STOP9 */ 3,
	/* WALKI */ 12,
	/* BOH   */ 9,
	/* UGG   */ 41,
	/* UTT   */ 35,
	/* WALKO */ 12,
	/* LAST  */ 15
};

} // End of namespace Trecision

#endif
