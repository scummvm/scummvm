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
 * $URL$
 * $Id$
 *
 */

#ifndef KYRA_KYRA_V2_H
#define KYRA_KYRA_V2_H

#include "kyra/kyra.h"
#include "kyra/script.h"
#include "kyra/screen_v2.h"

#include "common/list.h"

namespace Kyra {

enum kSequences {
	kSequenceVirgin = 0,
	kSequenceWestwood,
	kSequenceTitle,
	kSequenceOverview,
	kSequenceLibrary,
	kSequenceHand,
	kSequencePoint,
	kSequenceZanfaun,

	kSequenceFunters,
	kSequenceFerb,
	kSequenceFish,
	kSequenceFheep,
	kSequenceFarmer,
	kSequenceFuards,
	kSequenceFirates,
	kSequenceFrash,

	kSequenceArraySize
};

enum kNestedSequences {
	kSequenceFiggle = 0,
	kSequenceOver1,
	kSequenceOver2,
	kSequenceForest,
	kSequenceDragon,
	kSequenceDarm,
	kSequenceLibrary2,
	kSequenceLibrary3,
	kSequenceMarco,
	kSequenceHand1a,
	kSequenceHand1b,
	kSequenceHand1c,
	kSequenceHand2,
	kSequenceHand3,
	kSequenceHand4
};

enum kSequencesDemo {
	kSequenceDemoVirgin = 0,
	kSequenceDemoWestwood,
	kSequenceDemoTitle,
	kSequenceDemoHill,
	kSequenceDemoOuthome,
	kSequenceDemoWharf,
	kSequenceDemoDinob,
	kSequenceDemoFisher
};

enum kNestedSequencesDemo {
	kSequenceDemoWharf2 = 0,
	kSequenceDemoDinob2,
	kSequenceDemoWater,
	kSequenceDemoBail,
	kSequenceDemoDig
};

class WSAMovieV2;
class KyraEngine_v2;
class TextDisplayer_v2;
class Debugger_v2;

typedef int (KyraEngine_v2::*Seqproc)(WSAMovieV2*, int, int, int);

struct ActiveWSA {
	int16 flags;
	WSAMovieV2 *movie;
	uint16 startFrame;
	uint16 endFrame;
	uint16 frameDelay;
	Seqproc callback;
	uint32 nextFrame;
	uint16 currentFrame;
	uint16 lastFrame;
	uint16 x;
	uint16 y;
	const uint16 *control;
	uint16 startupCommand;
	uint16 finalCommand;
};

struct ActiveText {
	uint16 strIndex;
	uint16 x;
	uint16 y;
	int duration;
	uint16 width;
	uint32 startTime;
	int16 textcolor;
};

struct Sequence {
	uint16 flags;
	const char * wsaFile;
	const char * cpsFile;
	uint8 startupCommand;
	uint8 finalCommand;
	int16 stringIndex1;
	int16 stringIndex2;
	uint16 startFrame;
	uint16 numFrames;
	uint16 frameDelay;
	uint16 xPos;
	uint16 yPos;
	Seqproc callback;
	uint16 duration;
};

struct NestedSequence {
	uint16 flags;
	const char * wsaFile;
	uint16 startframe;
	uint16 endFrame;
	uint16 frameDelay;
	Seqproc callback;
	uint16 x;
	uint16 y;
	const uint16 * wsaControl;
	uint16 startupCommand;
	uint16 finalCommand;
};

enum kMusicDataID {
	kMusicIntro = 0,
	kMusicIngame,
	kMusicFinale
};

class KyraEngine_v2 : public KyraEngine {
friend class Debugger_v2;
friend class TextDisplayer_v2;
public:
	KyraEngine_v2(OSystem *system, const GameFlags &flags);
	~KyraEngine_v2();

	virtual Screen *screen() { return _screen; }
	Screen_v2 *screen_v2() { return _screen; }
	int language() const { return _lang; }

	virtual Movie *createWSAMovie();
protected:
	// Main menu code, also used for Kyra 3
	static const char *_mainMenuStrings[];

	virtual void gui_initMainMenu() {}
	int gui_handleMainMenu();
	virtual void gui_updateMainMenuAnimation();
	void gui_drawMainMenu(const char *const *strings, int select);
	void gui_drawMainBox(int x, int y, int w, int h, int fill);
	bool gui_mainMenuGetInput();

	void gui_printString(const char *string, int x, int y, int col1, int col2, int flags, ...);

	// intro/outro
	void seq_playSequences(int startSeq, int endSeq = -1);

	int seq_introWestwood(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introTitle(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introOverview(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introLibrary(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introHand(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introPoint(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introZanfaun(WSAMovieV2 *wsaObj, int x, int y, int frm);

	int seq_introOver1(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introOver2(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introForest(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introDragon(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introDarm(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introLibrary2(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introMarco(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introHand1a(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introHand1b(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introHand1c(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introHand2(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_introHand3(WSAMovieV2 *wsaObj, int x, int y, int frm);

	int seq_finaleFunters(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_finaleFerb(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_finaleFish(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_finaleFheep(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_finaleFarmer(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_finaleFuards(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_finaleFirates(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_finaleFrash(WSAMovieV2 *wsaObj, int x, int y, int frm);

	int seq_finaleFiggle(WSAMovieV2 *wsaObj, int x, int y, int frm);

	int seq_demoVirgin(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoWestwood(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoTitle(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoHill(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoOuthome(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoWharf(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoDinob(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoFisher(WSAMovieV2 *wsaObj, int x, int y, int frm);

	int seq_demoWharf2(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoDinob2(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoWater(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoBail(WSAMovieV2 *wsaObj, int x, int y, int frm);
	int seq_demoDig(WSAMovieV2 *wsaObj, int x, int y, int frm);

	void seq_sequenceCommand(int command);
	void seq_loadNestedSequence(int wsaNum, int seqNum);
	void seq_nestedSequenceFrame(int command, int wsaNum);
	void seq_animatedSubFrame(int srcPage, int dstPage, int delaytime,
		int steps, int x, int y, int w, int h, int openClose, int directionFlags);
	bool seq_processNextSubFrame(int wsaNum);
	void seq_resetActiveWSA(int wsaNum);
	void seq_unloadWSA(int wsaNum);
	void seq_processWSAs();
	void seq_cmpFadeFrame(const char *cmpFile);
	void seq_playTalkText(uint8 chatNum);
	void seq_resetAllTextEntries();
	uint32 seq_activeTextsTimeLeft();
	void seq_waitForTextsTimeout();
	int seq_setTextEntry(uint16 strIndex, uint16 posX, uint16 posY, int duration, uint16 width);
	void seq_processText();
	char *seq_preprocessString(const char *str, int width);
	void seq_printCreditsString(uint16 strIndex, int x, int y, const uint8 *colorMap, uint8 textcolor);
	void seq_playWsaSyncDialogue(uint16 strIndex, uint16 vocIndex, int textColor, int x, int y, int width,
		WSAMovieV2 * wsa, int firstframe, int lastframe, int wsaXpos, int wsaYpos);
	void seq_finaleActorScreen();
	void seq_displayScrollText(uint8 *data, const ScreenDim *d, int tempPage1, int tempPage2, int speed, int step, Screen::FontId fid1, Screen::FontId fid2, const uint8 *shapeData = 0, const char *const *specialData = 0);
	void seq_scrollPage();
	void seq_showStarcraftLogo();

	void seq_init();
	void seq_uninit();

	int init();
	int go();

	Screen_v2 *_screen;
	TextDisplayer_v2 *_text;
	Debugger_v2 *_debugger;

	uint8 *_mouseSHPBuf;

	static const int8 _dosTrackMap[];
	static const int _dosTrackMapSize;

	const AudioDataStruct *_soundData;

protected:
	// game initialization
	void startup();
	void runLoop();
	void cleanup();

	void registerDefaultSettings();

	// TODO: get rid of all variables having pointers to the static resources if possible
	// i.e. let them directly use the _staticres functions
	void initStaticResource();

	void setupTimers();
	void setupOpcodeTable();

	void loadMouseShapes();
	void loadItemShapes();

	// run
	void update();
	void updateWithText();

	Functor0Mem<void, KyraEngine_v2> _updateFunctor;

	void updateMouse();

	void dinoRide();

	struct Button;
	int checkInput(Button *buttonList, bool mainLoop = false);
	void removeInputTop();
	void handleInput(int x, int y);
	bool handleInputUnkSub(int x, int y);

	int inputSceneChange(int x, int y, int unk1, int unk2);

	// - Input
	void updateInput();

	int _mouseX, _mouseY;
	int _mouseState;

	struct Event {
		Common::Event event;
		bool causedSkip;

		Event() : event(), causedSkip(false) {}
		Event(Common::Event e) : event(e), causedSkip(false) {}
		Event(Common::Event e, bool skip) : event(e), causedSkip(skip) {}

		operator Common::Event() const { return event; }
	};
	Common::List<Event> _eventList;

	bool skipFlag() const;
	void resetSkipFlag(bool removeEvent = true);

	// gfx/animation specific
	uint8 *_gamePlayBuffer;
	void restorePage3();

	uint8 *_screenBuffer;
	bool _inventorySaved;
	void backUpPage0();
	void restorePage0();

	uint8 *_gfxBackUpRect;

	void backUpGfxRect24x24(int x, int y);
	void restoreGfxRect24x24(int x, int y);
	void backUpGfxRect32x32(int x, int y);
	void restoreGfxRect32x32(int x, int y);

	uint8 *getShapePtr(int index) { return _defaultShapeTable[index]; }
	uint8 *_defaultShapeTable[250];
	uint8 *_sceneShapeTable[50];

	WSAMovieV2 *_wsaSlots[10];

	void freeSceneShapePtrs();

	struct ShapeDesc {
		uint8 unk0, unk1, unk2, unk3, unk4;
		uint16 width, height;
		int16 xAdd, yAdd;
	};

	ShapeDesc *_shapeDescTable;

	struct SceneAnim {
		uint16 flags;
		int16 x, y;
		int16 x2, y2;
		int16 width, height;
		uint16 unkE;
		uint16 specialSize;
		uint16 unk12;
		int16 shapeIndex;
		uint16 wsaFlag;
		char filename[14];
	};

	SceneAnim _sceneAnims[10];
	WSAMovieV2 *_sceneAnimMovie[10];
	bool _specialSceneScriptState[10];
	bool _specialSceneScriptStateBackup[10];
	ScriptState _sceneSpecialScripts[10];
	uint32 _sceneSpecialScriptsTimer[10];
	int _lastProcessedSceneScript;
	bool _specialSceneScriptRunFlag;

	void updateSpecialSceneScripts();
	void freeSceneAnims();

	int _loadedZTable;
	void loadZShapes(int shapes);
	void loadInventoryShapes();

	void resetScaleTable();
	void setScaleTableItem(int item, int data);
	int getScale(int x, int y);
	uint16 _scaleTable[15];

	void setDrawLayerTableEntry(int entry, int data);
	int getDrawLayer(int x, int y);
	int _drawLayerTable[15];

	int _layerFlagTable[16]; // seems to indicate layers where items get destroyed when dropped to (TODO: check this!)

	char _newShapeFilename[13];
	int _newShapeLastEntry;
	int _newShapeWidth, _newShapeHeight;
	int _newShapeXAdd, _newShapeYAdd;
	int _newShapeFlag;
	uint8 *_newShapeFiledata;
	int _newShapeCount;
	int _newShapeAnimFrame;
	int _newShapeDelay;

	int initNewShapes(uint8 *filedata);
	void processNewShapes(int unk1, int unk2);
	void resetNewShapes(int count, uint8 *filedata);

	// animator
	struct AnimObj {
		uint16 index;
		uint16 type;
		uint16 enabled;
		uint16 needRefresh;
		uint16 unk8;
		uint16 animFlags;
		uint16 flags;
		int16 xPos1, yPos1;
		uint8 *shapePtr;
		uint16 shapeIndex1;
		uint16 animNum;
		uint16 shapeIndex3;
		uint16 shapeIndex2;
		uint16 unk1E;
		uint8 unk20;
		uint8 unk21;
		uint8 unk22;
		uint8 unk23;
		int16 xPos2, yPos2;
		int16 xPos3, yPos3;
		int16 width, height;
		int16 width2, height2;
		AnimObj *nextObject;
	};

	AnimObj _animObjects[42];
	void clearAnimObjects();

	AnimObj *_animList;
	bool _drawNoShapeFlag;
	AnimObj *initAnimList(AnimObj *list, AnimObj *entry);
	AnimObj *addToAnimListSorted(AnimObj *list, AnimObj *entry);
	AnimObj *deleteAnimListEntry(AnimObj *list, AnimObj *entry);

	void drawAnimObjects();
	void drawSceneAnimObject(AnimObj *obj, int x, int y, int drawLayer);
	void drawCharacterAnimObject(AnimObj *obj, int x, int y, int drawLayer);

	void refreshAnimObjects(int force);
	void refreshAnimObjectsIfNeed();
	void updateItemAnimations();

	void flagAnimObjsUnk8();
	void flagAnimObjsForRefresh();

	void updateCharFacing();
	void updateCharacterAnim(int);
	void updateSceneAnim(int anim, int newFrame);

	void addItemToAnimList(int item);
	void deleteItemAnimEntry(int item);

	int _animObj0Width, _animObj0Height;
	void setCharacterAnimDim(int w, int h);
	void resetCharacterAnimDim();

	// scene
	struct SceneDesc {
		char filename[10];
		uint16 exit1, exit2, exit3, exit4;
		uint8 flags;
		uint8 sound;
	};

	SceneDesc *_sceneList;
	int _sceneListSize;
	uint16 _currentScene;

	const char *_sceneCommentString;
	uint16 _sceneExit1, _sceneExit2, _sceneExit3, _sceneExit4;
	int _sceneEnterX1, _sceneEnterY1, _sceneEnterX2, _sceneEnterY2,
		_sceneEnterX3, _sceneEnterY3, _sceneEnterX4, _sceneEnterY4;
	int _specialExitCount;
	uint16 _specialExitTable[25];
	bool checkSpecialSceneExit(int num, int x, int y);
	uint8 _scenePal[688];
	bool _overwriteSceneFacing;

	void enterNewScene(uint16 newScene, int facing, int unk1, int unk2, int unk3);
	void enterNewSceneUnk1(int facing, int unk1, int unk2);
	void enterNewSceneUnk2(int unk1);
	void unloadScene();

	void loadScenePal();
	void loadSceneMsc();

	void fadeScenePal(int srcIndex, int delay);

	void startSceneScript(int unk1);
	void runSceneScript2();
	void runSceneScript4(int unk1);
	void runSceneScript6();
	void runSceneScript7();

	void initSceneAnims(int unk1);
	void initSceneScreen(int unk1);

	int trySceneChange(int *moveTable, int unk1, int updateChar);
	int checkSceneChange();

	// pathfinder
	int _movFacingTable[600];
	int findWay(int curX, int curY, int dstX, int dstY, int *moveTable, int moveTableSize);
	bool lineIsPassable(int x, int y);
	bool directLinePassable(int x, int y, int toX, int toY);

	int pathfinderInitPositionTable(int *moveTable);
	int pathfinderAddToPositionTable(int index, int v1, int v2);
	int pathfinderInitPositionIndexTable(int tableLen, int x, int y);
	int pathfinderAddToPositionIndexTable(int index, int v);
	void pathfinderFinializePath(int *moveTable, int unk1, int x, int y, int moveTableSize);

	int _pathfinderPositionTable[400];
	int _pathfinderPositionIndexTable[200];

	// item
	uint8 _itemHtDat[176];

	struct Item {
		uint16 id;
		uint16 sceneId;
		int16 x;
		uint8 y;
		uint16 unk7;
	};
	Item *_itemList;

	uint16 _hiddenItems[20];

	int findFreeItem();
	int countAllItems();
	int findItem(uint16 sceneId, uint16 id);
	int checkItemCollision(int x, int y);
	void resetItemList();
	void updateWaterFlasks();

	int _itemInHand;
	int _handItemSet;

	bool dropItem(int unk1, uint16 item, int x, int y, int unk2);
	bool processItemDrop(uint16 sceneId, uint16 item, int x, int y, int unk1, int unk2);
	void itemDropDown(int startX, int startY, int dstX, int dstY, int itemSlot, uint16 item);
	void exchangeMouseItem(int itemPos);
	bool pickUpItem(int x, int y);

	bool isDropable(int x, int y);

	static const byte _itemStringMap[];
	static const int _itemStringMapSize;

	void setMouseCursor(uint16 item);
	void setHandItem(uint16 item);
	void removeHandItem();

	static const int16 _flaskTable[];
	bool itemIsFlask(int item);

	// inventory
	static const int _inventoryX[];
	static const int _inventoryY[];
	static const uint16 _itemMagicTable[];

	int getInventoryItemSlot(uint16 item);
	void removeItemFromInventory(int slot);
	bool checkInventoryItemExchange(uint16 item, int slot);
	void drawInventoryShape(int page, uint16 item, int slot);
	void clearInventorySlot(int slot, int page);
	void redrawInventory(int page);
	void scrollInventoryWheel();
	int findFreeVisibleInventorySlot();

	struct ItemAnimData {
		int16 itemIndex;
		uint8 numFrames;
		uint8 curFrame;
		uint32 nextFrame;
		const uint8 *frames;
	} _itemAnimData[15];

	int _nextAnimItem;

	// gui
	void loadButtonShapes();
	uint8 *_buttonShapes[19];

	struct Button {
		Button *nextButton;
		uint16 index;
		uint16 unk6;
		uint16 unk8;
		byte data0Val1;
		byte data1Val1;
		byte data2Val1;
		// XXX
		uint16 flags;
		uint8 *shapePtr0;
		uint8 *shapePtr1;
		uint8 *shapePtr2;
		uint16 dimTableIndex;
		int16 x;
		int16 y;
		int16 width;
		int16 height;
		uint8 data0Val2;
		uint8 data0Val3;
		uint8 data1Val2;
		uint8 data1Val3;
		uint8 data2Val2;
		uint8 data2Val3;
		// XXX
		uint16 flags2;
		typedef int (KyraEngine_v2::*ButtonCallback)(KyraEngine_v2::Button*);
		ButtonCallback buttonCallback;
		// XXX
	};

	bool _buttonListChanged;
	Button *_buttonList;
	Button *_backUpButtonList;
	Button *_unknownButtonList;

	void initMainButtonList();

	void processButton(Button *button);
	Button *addButtonToList(Button *list, Button *newButton);
	int processButtonList(Button *button, uint16 inputFlag);

	int scrollInventory(Button *button);
	int buttonInventory(Button *button);
	int bookButton(Button *button);
	int cauldronButton(Button *button);
	int cauldronClearButton(Button *button);

	// book
	static const int _bookPageYOffset[];
	static const byte _bookTextColorMap[];

	int _bookMaxPage;
	int _bookNewPage;
	int _bookCurPage;
	int _bookBkgd;
	bool _bookShown;

	void loadBookBkgd();
	void showBookPage();
	void bookLoop();

	void bookDecodeText(uint8 *text);
	void bookPrintText(int dstPage, const uint8 *text, int x, int y, uint8 color);

	int bookPrevPage(Button *button);
	int bookNextPage(Button *button);
	int bookClose(Button *button);

	// cauldron
	uint8 _cauldronState;
	int16 _cauldronUseCount;
	int16 _cauldronTable[25];
	int16 _cauldronStateTables[23][7];

	static const int16 _cauldronProtectedItems[];
	static const int16 _cauldronBowlTable[];
	static const int16 _cauldronMagicTable[];
	static const int16 _cauldronMagicTableScene77[];
	static const uint8 _cauldronStateTable[];

	void resetCauldronStateTable(int idx);
	bool addToCauldronStateTable(int data, int idx);

	void setCauldronState(uint8 state, bool paletteFade);
	void clearCauldronTable();
	void addFrontCauldronTable(int item);
	void cauldronItemAnim(int item);
	void cauldronRndPaletteFade();
	bool updateCauldron();
	void listItemsInCauldron();

	// localization
	void loadCCodeBuffer(const char *file);
	void loadOptionsBuffer(const char *file);
	void loadChapterBuffer(int chapter);
	uint8 *_optionsBuffer;
	uint8 *_cCodeBuffer;

	uint8 *_chapterBuffer;
	int _currentChapter;
	int _newChapterFile;

	const uint8 *getTableEntry(const uint8 *buffer, int id);
	const char *getTableString(int id, const uint8 *buffer, int decode);
	const char *getChapterString(int id);
	int decodeString1(const char *src, char *dst);
	void decodeString2(const char *src, char *dst);

	void changeFileExtension(char *buffer);

	// - Just used in French version
	int getItemCommandStringDrop(uint16 item);
	int getItemCommandStringPickUp(uint16 item);
	int getItemCommandStringInv(uint16 item);
	// -

	char _internStringBuf[200];
	static const char *_languageExtension[];
	static const char *_scriptLangExt[];

	// character
	struct Character {
		uint16 sceneId;
		uint16 dlgIndex;
		uint8 height;
		uint8 facing;
		uint16 animFrame;
		uint8 unk8;
		uint8 unk9;
		uint8 unkA;
		uint16 inventory[20];
		int16 x1, y1;
		int16 x2, y2;
	};

	int8 _deathHandler;
	Character _mainCharacter;
	bool _useCharPal;
	int _charPalEntry;
	uint8 _charPalTable[16];
	void updateCharPal(int unk1);
	void setCharPalEntry(int entry, int value);

	void moveCharacter(int facing, int x, int y);
	int updateCharPos(int *table);
	void updateCharPosWithUpdate();
	void updateCharAnimFrame(int num, int *table);

	int checkCharCollision(int x, int y);

	int _mainCharX, _mainCharY;
	int _charScaleX, _charScaleY;

	static const int _characterFrameTable[];

	// text
	void showMessageFromCCode(int id, int16 palIndex, int);
	void showMessage(const char *string, int16 palIndex);
	void showChapterMessage(int id, int16 palIndex);

	void updateCommandLineEx(int str1, int str2, int16 palIndex);

	const char *_shownMessage;

	byte _messagePal[3];
	int _msgUnk1;

	// chat
	int _vocHigh;

	const char *_chatText;
	int _chatObject;
	bool _chatIsNote;
	uint32 _chatEndTime;
	int _chatVocHigh, _chatVocLow;

	ScriptData _chatScriptData;
	ScriptState _chatScriptState;

	int chatGetType(const char *text);
	int chatCalcDuration(const char *text);

	void objectChat(const char *text, int object, int vocHigh = -1, int vocLow = -1);
	void objectChatInit(const char *text, int object, int vocHigh = -1, int vocLow = -1);
	void objectChatPrintText(const char *text, int object);
	void objectChatProcess(const char *script);
	void objectChatWaitToFinish();

	void startDialogue(int dlgIndex);

	void zanthSceneStartupChat();
	void zanthRandomIdleChat();
	void updateDlgBuffer();
	void loadDlgHeader(int &csEntry, int &vocH, int &scIndex1, int &scIndex2);
	void processDialogue(int dlgOffset, int vocH = 0, int csEntry = 0);
	void npcChatSequence(const char *str, int objectId, int vocHigh = -1, int vocLow = -1);
	void setNewDlgIndex(int dlgIndex);

	int _npcTalkChpIndex;
	int _npcTalkDlgIndex;
	uint8 _newSceneDlgState[32];
	int8 **_conversationState;
	uint8 *_dlgBuffer;

	// Talk object handling
	void initTalkObject(int index);
	void deinitTalkObject(int index);

	struct TalkObject {
		char filename[13];
		int8 scriptId;
		int16 x, y;
		int8 color;
	};
	TalkObject *_talkObjectList;

	struct TalkSections {
		uint8 *STATim;
		uint8 *TLKTim;
		uint8 *ENDTim;
	};
	TalkSections _currentTalkSections;

	char _TLKFilename[13];
	bool _objectChatFinished;

	// tim sequence
	void tim_setupOpcodes();
	uint8 *tim_loadFile(const char *filename, uint8 *buffer, int32 bufferSize);
	void tim_releaseBuffer(uint8 *buffer);
	void tim_processSequence(uint8 *timBuffer, int loop);
	void tim_playFullSequence(const char *filename);

	int tim_o_dummy_r0(uint8 *ptr);
	int tim_o_dummy_r1(uint8 *ptr);
	int tim_o_clearCmds2(uint8 *ptr);
	int tim_o_abort(uint8 *ptr);
	int tim_o_selectcurrentCommandSet(uint8 *ptr);
	int tim_o_deleteBuffer(uint8 *ptr);
	int tim_o_refreshTimers(uint8 *ptr);
	int tim_o_execSubOpcode(uint8 *ptr);
	int tim_o_initActiveSub(uint8 *ptr);
	int tim_o_resetActiveSub(uint8 *ptr);
	int tim_o_printTalkText(uint8 *ptr);
	int tim_o_updateSceneAnim(uint8 *ptr);
	int tim_o_resetChat(uint8 *ptr);
	int tim_o_playSoundEffect(uint8 *ptr);

	typedef int (KyraEngine_v2::*TimOpc)(uint8 *ptr);
	const TimOpc * _timOpcodes;

	struct TIMHeader {
		uint16 deleteBufferFlag;
		int16 unkFlag;
		int16 unkFlag2;
		int16 cmdsOffset;
		int16 unkOffset2;
		int16 AVTLOffset;
		int16 TEXTOffset;
	};

	struct Cmds {
		uint8 *dataPtr;
		uint32 unk_2;
		uint32 timer1;
		uint32 timer2;
		uint8 *backupPtr;
		uint8 *AVTLSubChunk;
	};

	struct TIMBuffers {
		uint8 *AVTLChunk;
		uint8 *TEXTChunk;
		uint8 *offsUnkFlag2;
		uint8 *offsUnkFlag;
		int16 currentEntry;
		int16 unk_12;
		Cmds *currentCommandSet;
		uint8 *unkCmds;
	};
	TIMBuffers _TIMBuffers;

	const char *_timChatText;
	int _timChatObject;

	// sound
	int _oldTalkFile;
	int _currentTalkFile;
	void openTalkFile(int newFile);
	int _lastSfxTrack;

	virtual void snd_playVoiceFile(int id);
	void snd_loadSoundFile(int id);

	void playVoice(int high, int low);
	void snd_playSoundEffect(int track);

	// timer
	void timerFadeOutMessage(int);
	void timerCauldronAnimation(int);
	void timerFunc4(int);
	void timerFunc5(int);
	void timerBurnZanthia(int);

	void setTimer1DelaySecs(int secs);

	uint32 _nextIdleAnim;
	int _lastIdleScript;

	void setNextIdleAnimTimer();
	void showIdleAnim();
	void runIdleScript(int script);

	void setWalkspeed(uint8 speed);

	// delay
	void delay(uint32 millis, bool updateGame = false, bool isMainLoop = false);

	// ingame static sequence handling
	void seq_makeBookOrCauldronAppear(int type);
	void seq_makeBookAppear();

	struct InventoryWsa {
		int x, y, x2, y2, w, h;
		int page;
		int curFrame, lastFrame, specialFrame;
		int sfx;
		int delay;
		bool running;
		uint32 timer;
		WSAMovieV2 *wsa;
	} _invWsa;

	// TODO: move inside KyraEngine_v2::InventoryWsa?
	void loadInvWsa(const char *filename, int run, int delay, int page, int sfx, int sFrame, int flags);
	void closeInvWsa();

	void updateInvWsa();
	void displayInvWsaLastFrame();

	// opcodes
	int o2_setCharacterFacingRefresh(ScriptState *script);
	int o2_setCharacterPos(ScriptState *script);
	int o2_defineObject(ScriptState *script);
	int o2_refreshCharacter(ScriptState *script);
	int o2_getCharacterX(ScriptState *script);
	int o2_getCharacterY(ScriptState *script);
	int o2_getCharacterFacing(ScriptState *script);
	int o2_getCharacterScene(ScriptState *script);
	int o2_setSceneComment(ScriptState *script);
	int o2_setCharacterAnimFrame(ScriptState *script);
	int o2_setCharacterFacing(ScriptState *script);
	int o2_trySceneChange(ScriptState *script);
	int o2_moveCharacter(ScriptState *script);
	int o2_customCharacterChat(ScriptState *script);
	int o2_soundFadeOut(ScriptState *script);
	int o2_showChapterMessage(ScriptState *script);
	int o2_restoreTalkTextMessageBkgd(ScriptState *script);
	int o2_wsaClose(ScriptState *script);
	int o2_meanWhileScene(ScriptState *script);
	int o2_backUpScreen(ScriptState *script);
	int o2_restoreScreen(ScriptState *script);
	int o2_displayWsaFrame(ScriptState *script);
	int o2_displayWsaSequentialFramesLooping(ScriptState *script);
	int o2_wsaOpen(ScriptState *script);
	int o2_displayWsaSequentialFrames(ScriptState *script);
	int o2_displayWsaSequence(ScriptState *script);
	int o2_addItemToInventory(ScriptState *script);
	int o2_drawShape(ScriptState *script);	
	int o2_addItemToCurScene(ScriptState *script);
	int o2_checkForItem(ScriptState *script);
	int o2_removeItemSlotFromInventory(ScriptState *script);
	int o2_defineItem(ScriptState *script);
	int o2_removeItemFromInventory(ScriptState *script);
	int o2_countItemInInventory(ScriptState *script);
	int o2_queryGameFlag(ScriptState *script);
	int o2_resetGameFlag(ScriptState *script);
	int o2_setGameFlag(ScriptState *script);
	int o2_setHandItem(ScriptState *script);
	int o2_removeHandItem(ScriptState *script);
	int o2_handItemSet(ScriptState *script);
	int o2_hideMouse(ScriptState *script);
	int o2_addSpecialExit(ScriptState *script);
	int o2_setMousePos(ScriptState *script);
	int o2_showMouse(ScriptState *script);
	int o2_wipeDownMouseItem(ScriptState *script);
	//int o2_playSoundEffect(ScriptState *script);
	int o2_delaySecs(ScriptState *script);
	int o2_delay(ScriptState *script);
	int o2_setScaleTableItem(ScriptState *script);
	int o2_setDrawLayerTableItem(ScriptState *script);
	int o2_setCharPalEntry(ScriptState *script);
	int o2_loadZShapes(ScriptState *script);
	int o2_drawSceneShape(ScriptState *script);
	int o2_drawSceneShapeOnPage(ScriptState *script);
	int o2_disableAnimObject(ScriptState *script);
	int o2_enableAnimObject(ScriptState *script);
	int o2_loadPalette384(ScriptState *script);
	int o2_setPalette384(ScriptState *script);
	int o2_restoreBackBuffer(ScriptState *script);
	int o2_backUpInventoryGfx(ScriptState *script);
	int o2_disableSceneAnim(ScriptState *script);
	int o2_enableSceneAnim(ScriptState *script);
	int o2_restoreInventoryGfx(ScriptState *script);
	int o2_update(ScriptState *script);
	int o2_fadeScenePal(ScriptState *script);
	int o2_enterNewSceneEx(ScriptState *script);
	int o2_switchScene(ScriptState *script);
	int o2_getShapeFlag1(ScriptState *script);
	int o2_setPathfinderFlag(ScriptState *script);
	int o2_setLayerFlag(ScriptState *script);
	int o2_setZanthiaPos(ScriptState *script);
	int o2_loadMusicTrack(ScriptState *script);
	int o2_playWanderScoreViaMap(ScriptState *script);
	int o2_playSoundEffect(ScriptState *script);
	int o2_setSceneAnimPos(ScriptState *script);
	int o2_blockInRegion(ScriptState *script);
	int o2_blockOutRegion(ScriptState *script);
	int o2_setCauldronState(ScriptState *script);
	int o2_getRand(ScriptState *script);
	int o2_setDeathHandlerFlag(ScriptState *script);
	int o2_setDrawNoShapeFlag(ScriptState *script);
	int o2_showLetter(ScriptState *script);
	int o2_fillRect(ScriptState *script);
	int o2_encodeShape(ScriptState *script);
	int o2_defineRoomEntrance(ScriptState *script);
	int o2_runTemporaryScript(ScriptState *script);
	int o2_setSpecialSceneScriptRunTime(ScriptState *script);
	int o2_defineSceneAnim(ScriptState *script);
	int o2_updateSceneAnim(ScriptState *script);
	int o2_addToSceneAnimPosAndUpdate(ScriptState *script);
	int o2_useItemOnMainChar(ScriptState *script);
	int o2_startDialogue(ScriptState *script);
	int o2_zanthRandomChat(ScriptState *script);
	int o2_setupDialogue(ScriptState *script);
	int o2_getDlgIndex(ScriptState *script);
	int o2_defineRoom(ScriptState *script);
	int o2_addCauldronStateTableEntry(ScriptState *script);
	int o2_setCountDown(ScriptState *script);
	int o2_getCountDown(ScriptState *script);
	int o2_pressColorKey(ScriptState *script);
	int o2_objectChat(ScriptState *script);
	int o2_chapterChange(ScriptState *script);
	int o2_getColorCodeFlag1(ScriptState *script);
	int o2_setColorCodeFlag1(ScriptState *script);
	int o2_getColorCodeFlag2(ScriptState *script);
	int o2_setColorCodeFlag2(ScriptState *script);
	int o2_getColorCodeValue(ScriptState *script);
	int o2_setColorCodeValue(ScriptState *script);
	int o2_countItemInstances(ScriptState *script);
	int o2_initObject(ScriptState *script);
	int o2_npcChat(ScriptState *script);
	int o2_deinitObject(ScriptState *script);
	int o2_playTimSequence(ScriptState *script);
	int o2_makeBookOrCauldronAppear(ScriptState *script);
	int o2_setSpecialSceneScriptState(ScriptState *script);
	int o2_clearSpecialSceneScriptState(ScriptState *script);
	int o2_querySpecialSceneScriptState(ScriptState *script);
	int o2_resetInputColorCode(ScriptState *script);
	int o2_setHiddenItemsEntry(ScriptState *script);
	int o2_getHiddenItemsEntry(ScriptState *script);
	int o2_mushroomEffect(ScriptState *script);
	int o2_customChat(ScriptState *script);
	int o2_customChatFinish(ScriptState *script);
	int o2_setupSceneAnimation(ScriptState *script);
	int o2_stopSceneAnimation(ScriptState *script);
	int o2_disableTimer(ScriptState *script);
	int o2_enableTimer(ScriptState *script);
	int o2_setTimerCountdown(ScriptState *script);
	int o2_processPaletteIndex(ScriptState *script);
	int o2_updateTwoSceneAnims(ScriptState *script);
	int o2_getBoolFromStack(ScriptState *script);
	int o2_setVocHigh(ScriptState *script);
	int o2_getVocHigh(ScriptState *script);
	int o2_zanthiaChat(ScriptState *script);
	int o2_isVoiceEnabled(ScriptState *script);
	int o2_isVoicePlaying(ScriptState *script);
	int o2_stopVoicePlaying(ScriptState *script);
	int o2_getGameLanguage(ScriptState *script);
	int o2_dummy(ScriptState *script);

	// opcodes temporary
	// TODO: rename it from temporary to something more appropriate
	int o2t_defineNewShapes(ScriptState *script);
	int o2t_setCurrentFrame(ScriptState *script);
	int o2t_playSoundEffect(ScriptState *script);
	int o2t_fadeScenePal(ScriptState *script);
	int o2t_setShapeFlag(ScriptState *script);

	// script
	void runStartScript(int script, int unk1);
	void loadNPCScript();

	bool _noScriptEnter;

	ScriptData _npcScriptData;

	ScriptData _sceneScriptData;
	ScriptState _sceneScriptState;

	ScriptData _temporaryScriptData;
	ScriptState _temporaryScriptState;
	bool _temporaryScriptExecBit;
	Common::Array<const Opcode*> _opcodesTemporary;

	void runTemporaryScript(const char *filename, int unk1, int unk2, int newShapes, int shapeUnload);

	// pathfinder
	int _pathfinderFlag;

	uint8 *_unkBuf500Bytes;
	uint8 *_unkBuf200kByte;
	bool _chatAltFlag;
	int _unk3, _unk4, _unk5;
	bool _unkSceneScreenFlag1;
	bool _unkHandleSceneChangeFlag;

	// sequence player
	ActiveWSA *_activeWSA;
	ActiveText *_activeText;

	const char *const *_sequencePakList;
	int _sequencePakListSize;
	const char *const *_ingamePakList;
	int _ingamePakListSize;

	const char *const *_musicFileListIntro;
	int _musicFileListIntroSize;
	const char *const *_musicFileListFinale;
	int _musicFileListFinaleSize;
	const char *const *_musicFileListIngame;
	int _musicFileListIngameSize;
	const uint8 *_cdaTrackTableIntro;
	int _cdaTrackTableIntroSize;
	const uint8 *_cdaTrackTableIngame;
	int _cdaTrackTableIngameSize;
	const uint8 *_cdaTrackTableFinale;
	int _cdaTrackTableFinaleSize;
	const char *const *_sequenceSoundList;
	int _sequenceSoundListSize;
	const char *const *_ingameSoundList;
	int _ingameSoundListSize;
	const uint16 *_ingameSoundIndex;
	int _ingameSoundIndexSize;
	const char *const *_sequenceStrings;
	int _sequenceStringsSize;
	const uint16 *_ingameTalkObjIndex;
	int _ingameTalkObjIndexSize;
	const char *const *_ingameTimJpStr;
	int _ingameTimJpStrSize;
	const uint8 *_itemAnimTable;
	uint8 *_demoShapeDefs;
	int _sequenceStringsDuration[33];

	static const uint8 _seqTextColorPresets[];
	char *_seqProcessedString;
	WSAMovieV2 *_seqWsa;

	bool _abortIntroFlag;
	int _menuChoice;

	uint32 _seqFrameDelay;
	uint32 _seqStartTime;
	uint32 _seqEndTime;
	int _seqFrameCounter;
	int _seqScrollTextCounter;
	int _seqWsaCurrentFrame;
	bool _seqSpecialFlag;
	bool _seqSubframePlaying;
	uint8 _seqTextColor[2];
	uint8 _seqTextColorMap[16];

	Sequence *_sequences;
	NestedSequence *_nSequences;

	// color code related vars
	int _colorCodeFlag1;
	int _colorCodeFlag2;
	uint8 _presetColorCode[7];
	uint8 _inputColorCode[7];
	uint32 _scriptCountDown;
	int _dbgPass;

	// save/load specific
	uint32 saveGameID() const { return 'HOFS'; }

	void saveGame(const char *fileName, const char *saveName);
	void loadGame(const char *fileName);
};

} // end of namespace Kyra

#endif




