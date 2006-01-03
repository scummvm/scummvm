/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef KYRA_H
#define KYRA_H

#include "base/engine.h"
#include "common/rect.h"
#include "sound/mixer.h"
#include "common/file.h"

class AudioStream;

namespace Kyra {

enum {
	GF_FLOPPY	= 1 <<  0,
	GF_TALKIE	= 1 <<  1,
	GF_AUDIOCD	= 1 <<  2,  // FM-Towns versions seems to use audio CD
	GF_DEMO		= 1 <<  3,
	GF_ENGLISH	= 1 <<  4,
	GF_FRENCH	= 1 <<  5,
	GF_GERMAN	= 1 <<  6,
	GF_SPANISH	= 1 <<  7,
	// other languages here
	GF_LNGUNK	= 1 << 16
};

enum {
	GI_KYRA1 = 0
};

struct Character {
	uint16 sceneId;
	uint8 height;
	uint8 facing;
	uint16 currentAnimFrame;
	uint32 unk6;
	uint8 inventoryItems[10];
	int16 x1, y1, x2, y2;
	uint16 field_20;
	uint16 field_23;
};

struct Shape {
	uint8 imageIndex;
	int8 xOffset, yOffset;
	uint8 x, y, w, h;
};

struct Room {
	uint8 nameIndex;
	uint16 northExit;
	uint16 eastExit;
	uint16 southExit;
	uint16 westExit;
	uint8 itemsTable[12];
	uint16 itemsXPos[12];
	uint8 itemsYPos[12];
	uint32 unkField3[12];
};

struct Rect {
	int x, y;
	int x2, y2;
};

struct Item {
	uint8 unk1;
	uint8 height;
	uint8 unk2;
	uint8 unk3;
};

struct SeqLoop {
	const uint8 *ptr;
	uint16 count;
};

struct SceneExits {
	uint16 northXPos;
	uint8  northYPos;
	uint16 eastXPos;
	uint8  eastYPos;
	uint16 southXPos;
	uint8  southYPos;
	uint16 westXPos;
	uint8  westYPos;
};

class Movie;

class MusicPlayer;
class SeqPlayer;
class Resource;
class PAKFile;
class Screen;
class Sprites;
struct ScriptState;
struct ScriptData;
class ScriptHelper;
class Debugger;
class ScreenAnimator;
class TextDisplayer;
class KyraEngine;

struct Timer {
	bool active;
	int32 countdown;
	uint32 nextRun;
	void (KyraEngine::*func)(int timerNum);
};

struct Button {
	Button *nextButton;
	uint16 specialValue;
	// uint8 unk[4];
	uint8 process0;
	uint8 process1;
	uint8 process2;
	// uint8 unk
	uint16 flags;
	typedef int (KyraEngine::*ButtonCallback)(Button*);
	// using 6 pointers instead of 3 as in the orignal here (safer for use with classes)
	uint8 *process0PtrShape;
	uint8 *process1PtrShape;
	uint8 *process2PtrShape;
	ButtonCallback process0PtrCallback;
	ButtonCallback process1PtrCallback;
	ButtonCallback process2PtrCallback;
	uint16 dimTableIndex;
	uint16 x;
	uint16 y;
	uint16 width;
	uint16 height;
	// uint8 unk[8];
	uint32 flags2;
	ButtonCallback buttonCallback;
	// uint8 unk[8];
};

class KyraEngine : public Engine {
	friend class MusicPlayer;
	friend class Debugger;
	friend class ScreenAnimator;
public:

	enum {
		MUSIC_INTRO = 0
	};

	KyraEngine(GameDetector *detector, OSystem *system);
	~KyraEngine();
	
	void errorString(const char *buf_input, char *buf_output);

	Resource *resource() { return _res; }
	Screen *screen() { return _screen; }
	ScreenAnimator *animator() { return _animator; }
	TextDisplayer *text() { return _text; }
	MusicPlayer *midi() { return _midi; }
	uint32 tickLength() const { return _tickLength; }
	Movie *createWSAMovie();

	uint8 game() const { return _game; }
	uint32 features() const { return _features; }
	
	Common::RandomSource _rnd;
	int16 _northExitHeight;

	Character *_currentCharacter;
	int _paletteChanged;

	typedef void (KyraEngine::*IntroProc)();
	typedef int (KyraEngine::*OpcodeProc)(ScriptState *script);

	const char **seqWSATable() { return const_cast<const char **>(_seq_WSATable); }
	const char **seqCPSTable() { return const_cast<const char **>(_seq_CPSTable); }
	const char **seqCOLTable() { return const_cast<const char **>(_seq_COLTable); }
	const char **seqTextsTable() { return const_cast<const char **>(_seq_textsTable); }
	
	const uint8 **palTable1() { return const_cast<const uint8 **>(&_specialPalettes[0]); }
	const uint8 **palTable2() { return const_cast<const uint8 **>(&_specialPalettes[29]); }

	bool seq_skipSequence() const;
	void quitGame();
	void loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData);

	void snd_playTheme(int file, int track = 0);
	void snd_playTrack(int track);
	void snd_playVoiceFile(int id);
	bool snd_voicePlaying();
	void snd_playSoundEffect(int track);

	void drawSentenceCommand(char *sentence, int unk1);
	void updateSentenceCommand(char *str1, char *str2, int unk1);
	void updateTextFade();

	void updateGameTimers();
	void clearNextEventTickCount();
	void setTimerCountdown(uint8 timer, int32 countdown);
	void setTimerDelay(uint8 timer, int32 countdown);
	int16 getTimerDelay(uint8 timer);
	void enableTimer(uint8 timer);
	void disableTimer(uint8 timer);

	void waitTicks(int ticks);
	void delayWithTicks(int ticks);
	void animRefreshNPC(int character);
	int16 fetchAnimWidth(const uint8 *shape, int16 mult);
	int16 fetchAnimHeight(const uint8 *shape, int16 mult);
	
	void saveGame(const char *fileName, const char *saveName);
	void loadGame(const char *fileName);

	int mouseX() { return _mouseX; }
	int mouseY() { return _mouseY; }
	
	// all opcode procs (maybe that is somehow useless atm)
	int cmd_magicInMouseItem(ScriptState *script);
	int cmd_characterSays(ScriptState *script);
	int cmd_pauseTicks(ScriptState *script);
	int cmd_drawSceneAnimShape(ScriptState *script);
	int cmd_queryGameFlag(ScriptState *script);
	int cmd_setGameFlag(ScriptState *script);
	int cmd_resetGameFlag(ScriptState *script);
	int cmd_runNPCScript(ScriptState *script);
	int cmd_setSpecialExitList(ScriptState *script);
	int cmd_blockInWalkableRegion(ScriptState *script);
	int cmd_blockOutWalkableRegion(ScriptState *script);
	int cmd_walkPlayerToPoint(ScriptState *script);
	int cmd_dropItemInScene(ScriptState *script);
	int cmd_drawAnimShapeIntoScene(ScriptState *script);
	int cmd_createMouseItem(ScriptState *script);
	int cmd_savePageToDisk(ScriptState *script);
	int cmd_sceneAnimOn(ScriptState *script);
	int cmd_sceneAnimOff(ScriptState *script);
	int cmd_getElapsedSeconds(ScriptState *script);
	int cmd_mouseIsPointer(ScriptState *script);
	int cmd_destroyMouseItem(ScriptState *script);
	int cmd_runSceneAnimUntilDone(ScriptState *script);
	int cmd_fadeSpecialPalette(ScriptState *script);
	int cmd_playAdlibSound(ScriptState *script);
	int cmd_playAdlibScore(ScriptState *script);
	int cmd_phaseInSameScene(ScriptState *script);
	int cmd_setScenePhasingFlag(ScriptState *script);
	int cmd_resetScenePhasingFlag(ScriptState *script);
	int cmd_queryScenePhasingFlag(ScriptState *script);
	int cmd_sceneToDirection(ScriptState *script);
	int cmd_setBirthstoneGem(ScriptState *script);
	int cmd_placeItemInGenericMapScene(ScriptState *script);
	int cmd_setBrandonStatusBit(ScriptState *script);
	int cmd_pauseSeconds(ScriptState *script);
	int cmd_getCharactersLocation(ScriptState *script);
	int cmd_runNPCSubscript(ScriptState *script);
	int cmd_magicOutMouseItem(ScriptState *script);
	int cmd_internalAnimOn(ScriptState *script);
	int cmd_forceBrandonToNormal(ScriptState *script);
	int cmd_poisonDeathNow(ScriptState *script);
	int cmd_setScaleMode(ScriptState *script);
	int cmd_openWSAFile(ScriptState *script);
	int cmd_closeWSAFile(ScriptState *script);
	int cmd_runWSAFromBeginningToEnd(ScriptState *script);
	int cmd_displayWSAFrame(ScriptState *script);
	int cmd_enterNewScene(ScriptState *script);
	int cmd_setSpecialEnterXAndY(ScriptState *script);
	int cmd_runWSAFrames(ScriptState *script);
	int cmd_popBrandonIntoScene(ScriptState *script);
	int cmd_restoreAllObjectBackgrounds(ScriptState *script);
	int cmd_setCustomPaletteRange(ScriptState *script);
	int cmd_loadPageFromDisk(ScriptState *script);
	int cmd_customPrintTalkString(ScriptState *script);
	int cmd_restoreCustomPrintBackground(ScriptState *script);
	int cmd_hideMouse(ScriptState *script);
	int cmd_showMouse(ScriptState *script);
	int cmd_getCharacterX(ScriptState *script);
	int cmd_getCharacterY(ScriptState *script);
	int cmd_changeCharactersFacing(ScriptState *script);
	int cmd_copyWSARegion(ScriptState *script);
	int cmd_printText(ScriptState *script);
	int cmd_random(ScriptState *script);
	int cmd_loadSoundFile(ScriptState *script);
	int cmd_displayWSAFrameOnHidPage(ScriptState *script);
	int cmd_displayWSASequentialFrames(ScriptState *script);
	int cmd_drawCharacterStanding(ScriptState *script);
	int cmd_internalAnimOff(ScriptState *script);
	int cmd_changeCharactersXAndY(ScriptState *script);
	int cmd_clearSceneAnimatorBeacon(ScriptState *script);
	int cmd_querySceneAnimatorBeacon(ScriptState *script);
	int cmd_refreshSceneAnimator(ScriptState *script);
	int cmd_placeItemInOffScene(ScriptState *script);
	int cmd_wipeDownMouseItem(ScriptState *script);
	int cmd_placeCharacterInOtherScene(ScriptState *script);
	int cmd_getKey(ScriptState *script);
	int cmd_specificItemInInventory(ScriptState *script);
	int cmd_popMobileNPCIntoScene(ScriptState *script);
	int cmd_mobileCharacterInScene(ScriptState *script);
	int cmd_hideMobileCharacter(ScriptState *script);
	int cmd_unhideMobileCharacter(ScriptState *script);
	int cmd_setCharactersLocation(ScriptState *script);
	int cmd_walkCharacterToPoint(ScriptState *script);
	int cmd_specialEventDisplayBrynnsNote(ScriptState *script);
	int cmd_specialEventRemoveBrynnsNote(ScriptState *script);
	int cmd_setLogicPage(ScriptState *script);
	int cmd_fatPrint(ScriptState *script);
	int cmd_preserveAllObjectBackgrounds(ScriptState *script);
	int cmd_updateSceneAnimations(ScriptState *script);
	int cmd_sceneAnimationActive(ScriptState *script);
	int cmd_setCharactersMovementDelay(ScriptState *script);
	int cmd_getCharactersFacing(ScriptState *script);
	int cmd_bkgdScrollSceneAndMasksRight(ScriptState *script);
	int cmd_dispelMagicAnimation(ScriptState *script);
	int cmd_findBrightestFireberry(ScriptState *script);
	int cmd_setFireberryGlowPalette(ScriptState *script);
	int cmd_setDeathHandlerFlag(ScriptState *script);
	int cmd_drinkPotionAnimation(ScriptState *script);
	int cmd_makeAmuletAppear(ScriptState *script);
	int cmd_drawItemShapeIntoScene(ScriptState *script);
	int cmd_setCharactersCurrentFrame(ScriptState *script);
	int cmd_waitForConfirmationMouseClick(ScriptState *script);
	int cmd_pageFlip(ScriptState *script);
	int cmd_setSceneFile(ScriptState *script);
	int cmd_getItemInMarbleVase(ScriptState *script);
	int cmd_setItemInMarbleVase(ScriptState *script);
	int cmd_addItemToInventory(ScriptState *script);
	int cmd_intPrint(ScriptState *script);
	int cmd_shakeScreen(ScriptState *script);
	int cmd_createAmuletJewel(ScriptState *script);
	int cmd_setSceneAnimCurrXY(ScriptState *script);
	int cmd_poisonBrandonAndRemaps(ScriptState *script);
	int cmd_fillFlaskWithWater(ScriptState *script);
	int cmd_getCharactersMovementDelay(ScriptState *script);
	int cmd_getBirthstoneGem(ScriptState *script);
	int cmd_queryBrandonStatusBit(ScriptState *script);
	int cmd_playFluteAnimation(ScriptState *script);
	int cmd_playWinterScrollSequence(ScriptState *script);
	int cmd_getIdolGem(ScriptState *script);
	int cmd_setIdolGem(ScriptState *script);
	int cmd_totalItemsInScene(ScriptState *script);
	int cmd_restoreBrandonsMovementDelay(ScriptState *script);
	int cmd_setMousePos(ScriptState *script);
	int cmd_getMouseState(ScriptState *script);
	int cmd_setEntranceMouseCursorTrack(ScriptState *script);
	int cmd_itemAppearsOnGround(ScriptState *script);
	int cmd_setNoDrawShapesFlag(ScriptState *script);
	int cmd_fadeEntirePalette(ScriptState *script);
	int cmd_itemOnGroundHere(ScriptState *script);
	int cmd_queryCauldronState(ScriptState *script);
	int cmd_setCauldronState(ScriptState *script);
	int cmd_queryCrystalState(ScriptState *script);
	int cmd_setCrystalState(ScriptState *script);
	int cmd_setPaletteRange(ScriptState *script);
	int cmd_shrinkBrandonDown(ScriptState *script);
	int cmd_growBrandonUp(ScriptState *script);
	int cmd_setBrandonScaleXAndY(ScriptState *script);
	int cmd_resetScaleMode(ScriptState *script);
	int cmd_getScaleDepthTableValue(ScriptState *script);
	int cmd_setScaleDepthTableValue(ScriptState *script);
	int cmd_message(ScriptState *script);
	int cmd_checkClickOnNPC(ScriptState *script);
	int cmd_getFoyerItem(ScriptState *script);
	int cmd_setFoyerItem(ScriptState *script);
	int cmd_setNoItemDropRegion(ScriptState *script);
	int cmd_walkMalcolmOn(ScriptState *script);
	int cmd_passiveProtection(ScriptState *script);
	int cmd_setPlayingLoop(ScriptState *script);
	int cmd_brandonToStoneSequence(ScriptState *script);
	int cmd_brandonHealingSequence(ScriptState *script);
	int cmd_protectCommandLine(ScriptState *script);
	int cmd_pauseMusicSeconds(ScriptState *script);
	int cmd_resetMaskRegion(ScriptState *script);
	int cmd_setPaletteChangeFlag(ScriptState *script);
	int cmd_fillRect(ScriptState *script);
	int cmd_dummy(ScriptState *script);

protected:

	int go();
	int init(GameDetector &detector);

	void startup();
	void mainLoop();
	int initCharacterChat(int8 charNum);
	int8 getChatPartnerNum();
	void backupChatPartnerAnimFrame(int8 charNum);
	void restoreChatPartnerAnimFrame(int8 charNum);
	void endCharacterChat(int8 charNum, int16 arg_4);
	void waitForChatToFinish(int16 chatDuration, char *str, uint8 charNum);
	void characterSays(char *chatStr, int8 charNum, int8 chatDuration);

	void setCharacterDefaultFrame(int character);
	void setCharactersPositions(int character);
	void setCharactersHeight();
	int setGameFlag(int flag);
	int queryGameFlag(int flag);
	int resetGameFlag(int flag);
	
	void enterNewScene(int sceneId, int facing, int unk1, int unk2, int brandonAlive);
	void transcendScenes(int roomIndex, int roomName);
	void setSceneFile(int roomIndex, int roomName);
	void moveCharacterToPos(int character, int facing, int xpos, int ypos);
	void setCharacterPositionWithUpdate(int character);
	int setCharacterPosition(int character, int *facingTable);
	void setCharacterPositionHelper(int character, int *facingTable);
	int getOppositeFacingDirection(int dir);
	void loadSceneMSC();
	void blockInRegion(int x, int y, int width, int height);
	void blockOutRegion(int x, int y, int width, int height);
	void startSceneScript(int brandonAlive);
	void setupSceneItems();
	void initSceneData(int facing, int unk1, int brandonAlive);
	void clearNoDropRects();
	void addToNoDropRects(int x, int y, int w, int h);
	byte findFreeItemInScene(int scene);
	byte findItemAtPos(int x, int y);
	void placeItemInGenericMapScene(int item, int index);
	void initSceneObjectList(int brandonAlive);
	void initSceneScreen(int brandonAlive);
	int findDuplicateItemShape(int shape);
	int findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize);
	int findSubPath(int x, int y, int toX, int toY, int *moveTable, int start, int end);
	int getFacingFromPointToPoint(int x, int y, int toX, int toY);
	void changePosTowardsFacing(int &x, int &y, int facing);
	bool lineIsPassable(int x, int y);
	int getMoveTableSize(int *moveTable);
	int handleSceneChange(int xpos, int ypos, int unk1, int frameReset);
	int processSceneChange(int *table, int unk1, int frameReset);
	int changeScene(int facing);
	void createMouseItem(int item);
	void destroyMouseItem();
	void setMouseItem(int item);
	void wipeDownMouseItem(int xpos, int ypos);
	void rectClip(int &x, int &y, int w, int h);
	void backUpRect0(int xpos, int ypos);
	void restoreRect0(int xpos, int ypos);
	void backUpRect1(int xpos, int ypos);
	void restoreRect1(int xpos, int ypos);
	void copyBackgroundBlock(int x, int page, int flag);
	void copyBackgroundBlock2(int x);
	void makeBrandonFaceMouse();
	void setBrandonPoisonFlags(int reset);
	void resetBrandonPoisonFlags();

	void processInput(int xpos, int ypos);
	int processInputHelper(int xpos, int ypos);
	int clickEventHandler(int xpos, int ypos);
	void clickEventHandler2();
	void updateMousePointer(bool forceUpdate = false);
	bool hasClickedOnExit(int xpos, int ypos);
	int checkForNPCScriptRun(int xpos, int ypos);
	void runNpcScript(int func);
	
	int countItemsInScene(uint16 sceneId);
	int processItemDrop(uint16 sceneId, uint8 item, int x, int y, int unk1, int unk2);
	void exchangeItemWithMouseItem(uint16 sceneId, int itemIndex);
	void addItemToRoom(uint16 sceneId, uint8 item, int itemIndex, int x, int y);
	int getDrawLayer(int x, int y);
	int getDrawLayer2(int x, int y, int height);
	int checkNoDropRects(int x, int y);
	int isDropable(int x, int y);
	void itemDropDown(int x, int y, int destX, int destY, byte freeItem, int item);
	void dropItem(int unk1, int item, int x, int y, int unk2);
	void itemSpecialFX(int x, int y, int item);
	void itemSpecialFX1(int x, int y, int item);
	void itemSpecialFX2(int x, int y, int item);
	void magicOutMouseItem(int animIndex, int itemPos);
	void magicInMouseItem(int animIndex, int item, int itemPos);
	void specialMouseItemFX(int shape, int x, int y, int animIndex, int tableIndex, int loopStart, int maxLoops);
	void processSpecialMouseItemFX(int shape, int x, int y, int tableValue, int loopStart, int maxLoops);
	void updatePlayerItemsForScene();
	void redrawInventory(int page);
	
	void drawJewelPress(int jewel, int drawSpecial);
	void drawJewelsFadeOutStart();
	void drawJewelsFadeOutEnd(int jewel);
	void setupShapes123(const Shape *shapeTable, int endShape, int flags);
	void freeShapes123();
	void setBrandonAnimSeqSize(int width, int height);
	void resetBrandonAnimSeqSize();
	
	void seq_demo();
	void seq_intro();
	void seq_introLogos();
	void seq_introStory();
	void seq_introMalcolmTree();
	void seq_introKallakWriting();
	void seq_introKallakMalcolm();
	void seq_createAmuletJewel(int jewel, int page, int noSound, int drawOnly);
	void seq_brandonHealing();
	void seq_brandonHealing2();
	void seq_poisonDeathNow(int now);
	void seq_poisonDeathNowAnim();
	void seq_playFluteAnimation();
	void seq_winterScroll1();
	void seq_winterScroll2();
	void seq_makeBrandonInv();
	void seq_makeBrandonNormal();
	void seq_makeBrandonNormal2();
	void seq_makeBrandonWisp();
	void seq_dispelMagicAnimation();
	void seq_fillFlaskWithWater(int item, int type);
	void seq_playDrinkPotionAnim(int unk1, int unk2, int flags);

	void snd_startTrack();
	void snd_haltTrack();
	void snd_setSoundEffectFile(int file);
	
	static OpcodeProc _opcodeTable[];
	static const int _opcodeTableSize;
	
	enum {
		RES_ALL = 0,
		RES_INTRO = (1 << 0),
		RES_INGAME = (1 << 1)
	};
	
	void res_loadResources(int type = RES_ALL);
	void res_unloadResources(int type = RES_ALL);
	void res_loadLangTable(const char *filename, PAKFile *res, byte ***loadTo, int *size, bool nativ);
	void res_loadTable(const byte *src, byte ***loadTo, int *size);
	void res_loadRoomTable(const byte *src, Room **loadTo, int *size);
	void res_loadShapeTable(const byte *src, Shape **loadTo, int *size);
	void res_freeLangTable(char ***sting, int *size);
	
	void waitForEvent();
	void delay(uint32 millis, bool update = false);
	void loadPalette(const char *filename, uint8 *palData);
	void loadMouseShapes();
	void loadCharacterShapes();
	void loadSpecialEffectShapes();
	void loadItems();
	void loadButtonShapes();
	void initMainButtonList();
	void loadMainScreen();
	void setCharactersInDefaultScene();
	void resetBrandonPosionFlags();
	void initAnimStateList();
	
	void setTimer19();
	void setupTimers();
	void timerUpdateHeadAnims(int timerNum);
	void timerSetFlags1(int timerNum);
	void timerSetFlags2(int timerNum);
	void timerSetFlags3(int timerNum);
	void timerCheckAnimFlag1(int timerNum);
	void timerCheckAnimFlag2(int timerNum);
	void checkAmuletAnimFlags();
	void timerRedrawAmulet(int timerNum);
	void timerFadeText(int timerNum);
	void updateAnimFlag1(int timerNum);
	void updateAnimFlag2(int timerNum);
	void drawAmulet();
	void setTextFadeTimerCountdown(int16 countdown);
	
	int buttonInventoryCallback(Button *caller);
	int buttonAmuletCallback(Button *caller);
	Button *initButton(Button *list, Button *newButton);
	void processButtonList(Button *list);
	void processButton(Button *button);
	
	uint8 _game;
	bool _fastMode;
	bool _quitFlag;
	bool _skipIntroFlag;
	bool _abortIntroFlag;
	bool _abortWalkFlag;
	bool _abortWalkFlag2;
	bool _mousePressFlag;
	uint8 _flagsTable[53];
	uint8 *_unkPtr1, *_unkPtr2;
	uint8 *_hidPage, *_screenPage;
	uint8 *_shapes[377];
	uint16 _gameSpeed;
	uint16 _tickLength;
	uint32 _features;
	int _mouseX, _mouseY;
	int8 _itemInHand;
	int _mouseState;
	bool _handleInput;
	bool _changedScene;
	int _unkScreenVar1, _unkScreenVar2, _unkScreenVar3;
	int _unkAmuletVar;
	
	int _brandonAnimSeqSizeWidth;
	int _brandonAnimSeqSizeHeight;

	Movie *_movieObjects[10];

	uint16 _entranceMouseCursorTracks[8];
	uint16 _walkBlockNorth;
	uint16 _walkBlockEast;
	uint16 _walkBlockSouth;
	uint16 _walkBlockWest;
	
	int32 _scaleMode;
	int16 _scaleTable[145];
	
	Rect _noDropRects[11];
	
	int8 _birthstoneGemTable[4];
	int8 _idolGemsTable[3];
	
	int8 _marbleVaseItem;
	int8 _foyerItemTable[3];
	
	int8 _cauldronState;
	int8 _crystalState[2];

	uint16 _brandonStatusBit;
	int _brandonStatusBit0x02Flag;
	int _brandonStatusBit0x20Flag;
	uint8 _brandonPoisonFlagsGFX[256];
	uint8 _deathHandler;
	int _brandonInvFlag;
	int8 _poisonDeathCounter;
	int _brandonPosX;
	int _brandonPosY;
	int _brandonScaleX;
	int _brandonScaleY;
	int _brandonDrawFrame;

	uint16 _currentChatPartnerBackupFrame;
	uint16 _currentCharAnimFrame;
	
	int8 *_sceneAnimTable[50];
	
	Item _itemTable[145];
	int _lastProcessedItem;
	int _lastProcessedItemHeight;
	
	int16 *_exitListPtr;
	int16 _exitList[11];
	SceneExits _sceneExits;
	uint16 _currentRoom;
	int _scenePhasingFlag;
	uint8 *_maskBuffer;
	
	int _sceneChangeState;
	int _loopFlag2;
	
	int _pathfinderFlag;
	int _pathfinderFlag2;
	int _lastFindWayRet;
	int *_movFacingTable;
	
	int8 _talkingCharNum;
	int8 _charSayUnk2;
	int8 _charSayUnk3;
	int8 _currHeadShape;
	uint8 _currSentenceColor[3];
	int8 _startSentencePalIndex;
	bool _fadeText;

	uint8 _configTalkspeed;
	
	int _curMusicTheme;
	int _newMusicTheme;
	AudioStream *_currentVocFile;
	Audio::SoundHandle _vocHandle;

	Resource *_res;
	Screen *_screen;
	ScreenAnimator *_animator;
	MusicPlayer *_midi;
	SeqPlayer *_seq;
	Sprites *_sprites;
	TextDisplayer *_text;
	ScriptHelper *_scriptInterpreter;
	Debugger *_debugger;
	Common::SaveFileManager *_saveFileMan;

	ScriptState *_scriptMain;
	
	ScriptState *_npcScript;
	ScriptData *_npcScriptData;
	
	ScriptState *_scriptClick;
	ScriptData *_scriptClickData;
	
	Character *_characterList;
	
	Button *_buttonList;
	
	uint8 *_buttonShape0;
	uint8 *_buttonShape1;
	uint8 *_buttonShape2;
	uint8 *_buttonShape3;
	uint8 *_buttonShape4;
	uint8 *_buttonShape5;
	
	uint8 *_seq_Forest;
	uint8 *_seq_KallakWriting;
	uint8 *_seq_KyrandiaLogo;
	uint8 *_seq_KallakMalcolm;
	uint8 *_seq_MalcolmTree;
	uint8 *_seq_WestwoodLogo;
	uint8 *_seq_Demo1;
	uint8 *_seq_Demo2;
	uint8 *_seq_Demo3;
	uint8 *_seq_Demo4;
	
	char **_seq_WSATable;
	char **_seq_CPSTable;
	char **_seq_COLTable;
	char **_seq_textsTable;
	
	int _seq_WSATable_Size;
	int _seq_CPSTable_Size;
	int _seq_COLTable_Size;
	int _seq_textsTable_Size;
	
	char **_itemList;
	char **_takenList;
	char **_placedList;
	char **_droppedList;
	char **_noDropList;
	char **_putDownFirst;
	char **_waitForAmulet;
	char **_blackJewel;
	char **_poisonGone;
	char **_healingTip;
	char **_thePoison;
	char **_fluteString;
	char **_wispJewelStrings;
	char **_magicJewelString;
	char **_flaskFull;
	char **_fullFlask;
	
	int _itemList_Size;
	int _takenList_Size;
	int _placedList_Size;
	int _droppedList_Size;
	int _noDropList_Size;
	int _putDownFirst_Size;
	int _waitForAmulet_Size;
	int _blackJewel_Size;
	int _poisonGone_Size;
	int _healingTip_Size;
	int _thePoison_Size;
	int _fluteString_Size;
	int _wispJewelStrings_Size;
	int _magicJewelString_Size;
	int _flaskFull_Size;
	int _fullFlask_Size;
	
	char **_characterImageTable;
	int _characterImageTableSize;
	
	Shape *_defaultShapeTable;
	int _defaultShapeTableSize;
	
	Shape *_healingShapeTable;
	int  _healingShapeTableSize;
	Shape *_healingShape2Table;
	int  _healingShape2TableSize;
	
	Shape *_posionDeathShapeTable;
	int _posionDeathShapeTableSize;
	
	Shape *_fluteAnimShapeTable;
	int _fluteAnimShapeTableSize;
	
	Shape *_winterScrollTable;
	int _winterScrollTableSize;
	Shape *_winterScroll1Table;
	int _winterScroll1TableSize;
	Shape *_winterScroll2Table;
	int _winterScroll2TableSize;
	
	Shape *_drinkAnimationTable;
	int _drinkAnimationTableSize;
	
	Shape *_brandonToWispTable;
	int _brandonToWispTableSize;
	
	Shape *_magicAnimationTable;
	int _magicAnimationTableSize;
	
	Shape *_brandonStoneTable;
	int _brandonStoneTableSize;
	
	Room *_roomTable;
	int _roomTableSize;	
	char **_roomFilenameTable;
	int _roomFilenameTableSize;
	
	uint8 *_amuleteAnim;
	
	uint8 *_specialPalettes[33];

	Timer _timers[34];
	uint32 _timerNextRun;	
	static const char *_xmidiFiles[];
	static const int _xmidiFilesCount;
	
	static const int8 _charXPosTable[];
	static const int8 _addXPosTable[];
	static const int8 _charYPosTable[];
	static const int8 _addYPosTable[];

	// positions of the inventory
	static const uint16 _itemPosX[];
	static const uint8 _itemPosY[];
	
	static Button _buttonData[];
	static Button *_buttonDataListPtr[];

	static const uint8 _magicMouseItemStartFrame[];
	static const uint8 _magicMouseItemEndFrame[];
	static const uint8 _magicMouseItemStartFrame2[];
	static const uint8 _magicMouseItemEndFrame2[];

	static const uint16 _amuletX[];
	static const uint16 _amuletY[];
	static const uint16 _amuletX2[];
	static const uint16 _amuletY2[];
};

} // End of namespace Kyra

#endif
