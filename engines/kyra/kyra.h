/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#ifndef KYRA_H
#define KYRA_H

#include "base/engine.h"
#include "common/rect.h"

namespace Kyra {

class Movie;
class Sound;
class SeqPlayer;
class Resource;
class PAKFile;
class Screen;
class Sprites;
class ScriptHelper;
class Debugger;
class ScreenAnimator;
class TextDisplayer;
class KyraEngine;
class StaticResource;

struct ScriptState;
struct ScriptData;

enum {
	GF_FLOPPY	= 1 <<  0,
	GF_TALKIE	= 1 <<  1,
	GF_AUDIOCD	= 1 <<  2,  // FM-Towns versions seems to use audio CD
	GF_DEMO		= 1 <<  3,
	GF_ENGLISH	= 1 <<  4,
	GF_FRENCH	= 1 <<  5,
	GF_GERMAN	= 1 <<  6,
	GF_SPANISH	= 1 <<  7,
	GF_ITALIAN	= 1 <<  8,
	// other languages here
	GF_LNGUNK	= 1 << 16,	// also used for multi language in kyra3
	GF_AMIGA	= 1 << 17	// this is no special version flag yet!
};

enum {
	GI_KYRA1 = 0,
	GI_KYRA2 = 1,
	GI_KYRA3 = 2
};

// TODO: this is just the start of makeing the debug output of the kyra engine a bit more useable
// in the future we maybe merge some flags  and/or create new ones
enum kDebugLevels {
	kDebugLevelScriptFuncs = 1 << 0,		// prints debug output of cmd_* functions
	kDebugLevelScript = 1 << 1,				// prints debug output of "ScriptHelper" functions
	kDebugLevelSprites = 1 << 2,			// prints debug output of "Sprites" functions
	kDebugLevelScreen = 1 << 3,				// prints debug output of "Screen" functions
	kDebugLevelSound = 1 << 4,				// prints debug output of "Sound" functions
	kDebugLevelAnimator = 1 << 5,			// prints debug output of "ScreenAnimator" functions
	kDebugLevelMain = 1 << 6,				// prints debug output of common "KyraEngine*" functions && "TextDisplayer" functions
	kDebugLevelGUI = 1 << 7,				// prints debug output of "KyraEngine*" gui functions
	kDebugLevelSequence = 1 << 8,			// prints debug output of "SeqPlayer" functions
	kDebugLevelMovie = 1 << 9				// prints debug output of movie specific funtions
};

struct Character {
	uint16 sceneId;
	uint8 height;
	uint8 facing;
	uint16 currentAnimFrame;
	uint8 inventoryItems[10];
	int16 x1, y1, x2, y2;
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
	uint8 needInit[12];
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

struct BeadState {
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	int16 dstX;
	int16 dstY;
	int16 width2;
	int16 unk8;
	int16 unk9;
	int16 tableIndex;
};

struct Timer {
	uint8 active;
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

struct MenuItem {
	bool enabled;
	uint16 field_1;
	uint8 field_3;
	const char *itemString;
	int16 x;
	int16 field_9;
	uint16 y;
	uint16 width;
	uint16 height;
	uint8 textColor;
	uint8 highlightColor;
	int16 field_12;
	uint8 field_13;
	uint8 bgcolor;
	uint8 color1;
	uint8 color2;
	int (KyraEngine::*callback)(Button*);
	int16 field_1b;
	const char *labelString;
	uint16 labelX;
	uint8 labelY;
	uint8 field_24;
	uint32 field_25;
};

struct Menu {
	int16 x;
	int16 y;
	uint16 width;
	uint16 height;
	uint8 bgcolor;
	uint8 color1;
	uint8 color2;
	const char *menuName;
	uint8 textColor;
	int16 field_10;
	uint16 field_12;
	uint16 highlightedItem;
	uint8 nrOfItems;
	int16 scrollUpBtnX;
	int16 scrollUpBtnY;
	int16 scrollDownBtnX;
	int16 scrollDownBtnY;
	MenuItem item[6];
};

struct KeyboardEvent {
	bool pending;
	uint32 repeat;
	uint8 ascii;
};

class KyraEngine : public Engine {
	friend class MusicPlayer;
	friend class Debugger;
	friend class ScreenAnimator;
public:

	enum {
		MUSIC_INTRO = 0
	};

	KyraEngine(OSystem *system);
	~KyraEngine();

	virtual int setupGameFlags() = 0;
	
	void errorString(const char *buf_input, char *buf_output);

	Resource *resource() { return _res; }
	Screen *screen() { return _screen; }
	ScreenAnimator *animator() { return _animator; }
	TextDisplayer *text() { return _text; }
	Sound *sound() { return _sound; }
	StaticResource *staticres() { return _staticres; }
	uint32 tickLength() const { return _tickLength; }
	virtual Movie *createWSAMovie();

	uint8 game() const { return _game; }
	uint32 features() const { return _features; }

	uint8 **shapes() { return _shapes; }
	Character *currentCharacter() { return _currentCharacter; }
	Character *characterList() { return _characterList; }
	uint16 brandonStatus() { return _brandonStatusBit; }

	int _paletteChanged;
	Common::RandomSource _rnd;
	int16 _northExitHeight;

	typedef void (KyraEngine::*IntroProc)();
	typedef int (KyraEngine::*OpcodeProc)(ScriptState *script);

	const char * const*seqWSATable() { return _seq_WSATable; }
	const char * const*seqCPSTable() { return _seq_CPSTable; }
	const char * const*seqCOLTable() { return _seq_COLTable; }
	const char * const*seqTextsTable() { return _seq_textsTable; }
	
	const uint8 * const*palTable1() { return &_specialPalettes[0]; }
	const uint8 * const*palTable2() { return &_specialPalettes[29]; }

	bool seq_skipSequence() const;
	void delayUntil(uint32 timestamp, bool updateGameTimers = false, bool update = false, bool isMainLoop = false);
	void delay(uint32 millis, bool update = false, bool isMainLoop = false);
	void quitGame();
	void loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData);

	void snd_playTheme(int file, int track = 0);
	void snd_playVoiceFile(int id);
	void snd_voiceWaitForFinish(bool ingame = true);
	void snd_playSoundEffect(int track);
	void snd_playWanderScoreViaMap(int command, int restart);

	void drawSentenceCommand(const char *sentence, int unk1);
	void updateSentenceCommand(const char *str1, const char *str2, int unk1);
	void updateTextFade();

	void updateGameTimers();
	void clearNextEventTickCount();
	void setTimerCountdown(uint8 timer, int32 countdown);
	void setTimerDelay(uint8 timer, int32 countdown);
	int16 getTimerDelay(uint8 timer);
	void enableTimer(uint8 timer);
	void disableTimer(uint8 timer);

	void delayWithTicks(int ticks);
	
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
	int cmd_vocUnload(ScriptState *script);
	int cmd_vocLoad(ScriptState *script);

protected:

	virtual int go();
	virtual int init();

	void startup();
	void mainLoop();
	int initCharacterChat(int8 charNum);
	int8 getChatPartnerNum();
	void backupChatPartnerAnimFrame(int8 charNum);
	void restoreChatPartnerAnimFrame(int8 charNum);
	void endCharacterChat(int8 charNum, int16 arg_4);
	void waitForChatToFinish(int16 chatDuration, const char *str, uint8 charNum);
	void characterSays(const char *chatStr, int8 charNum, int8 chatDuration);

	void setCharactersPositions(int character);
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
	void seq_playDrinkPotionAnim(int item, int unk2, int flags);
	int seq_playEnd();
	void seq_brandonToStone();
	void seq_playEnding();
	void seq_playCredits();
	void updateKyragemFading();
	
	static OpcodeProc _opcodeTable[];
	static const int _opcodeTableSize;
	
	void waitForEvent();
	void loadPalette(const char *filename, uint8 *palData);
	void loadMouseShapes();
	void loadCharacterShapes();
	void loadSpecialEffectShapes();
	void loadItems();
	void loadButtonShapes();
	void initMainButtonList();
	void loadMainScreen(int page = 3);
	void setCharactersInDefaultScene();
	void setupPanPages();
	void freePanPages();
	void closeFinalWsa();
	int handleMalcolmFlag();
	int handleBeadState();
	void initBeadState(int x, int y, int x2, int y2, int unk1, BeadState *ptr);
	int processBead(int x, int y, int &x2, int &y2, BeadState *ptr);
	
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
	void setWalkspeed(uint8 newSpeed);

	int buttonInventoryCallback(Button *caller);
	int buttonAmuletCallback(Button *caller);
	int buttonMenuCallback(Button *caller);
	int drawBoxCallback(Button *button);
	int drawShadedBoxCallback(Button *button);
	void calcCoords(Menu &menu);
	void initMenu(Menu &menu);
	void setGUILabels();
	
	Button *initButton(Button *list, Button *newButton);
	void processButtonList(Button *list);
	void processButton(Button *button);
	void processMenuButton(Button *button);
	void processAllMenuButtons();

	const char *getSavegameFilename(int num);
	void setupSavegames(Menu &menu, int num);
	int getNextSavegameSlot();

	int gui_resumeGame(Button *button);
	int gui_loadGameMenu(Button *button);
	int gui_saveGameMenu(Button *button);
	int gui_gameControlsMenu(Button *button);
	int gui_quitPlaying(Button *button);
	int gui_quitConfirmYes(Button *button);
	int gui_quitConfirmNo(Button *button);
	int gui_loadGame(Button *button);
	int gui_saveGame(Button *button);
	int gui_savegameConfirm(Button *button);
	int gui_cancelSubMenu(Button *button);
	int gui_scrollUp(Button *button);
	int gui_scrollDown(Button *button);
	int gui_controlsChangeMusic(Button *button);
	int gui_controlsChangeSounds(Button *button);
	int gui_controlsChangeWalk(Button *button);
	int gui_controlsChangeText(Button *button);
	int gui_controlsChangeVoice(Button *button);

	bool gui_quitConfirm(const char *str);
	void gui_getInput();
	void gui_redrawText(Menu menu);
	void gui_redrawHighlight(Menu menu);
	void gui_processHighlights(Menu &menu);
	void gui_updateSavegameString();
	void gui_redrawTextfield();
	void gui_fadePalette();
	void gui_restorePalette();
	void gui_setupControls(Menu &menu);

	uint8 _game;
	bool _quitFlag;
	bool _skipFlag;
	bool _skipIntroFlag;
	bool _abortIntroFlag;
	bool _menuDirectlyToLoad;
	bool _abortWalkFlag;
	bool _abortWalkFlag2;
	bool _mousePressFlag;
	int8 _mouseWheel;
	uint8 _flagsTable[53];
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
	int _beadStateVar;
	int _unkAmuletVar;
		
	int _malcolmFlag;
	int _endSequenceSkipFlag;
	int _endSequenceNeedLoading;
	int _unkEndSeqVar2;
	uint8 *_endSequenceBackUpRect;
	int _unkEndSeqVar4;
	int _unkEndSeqVar5;
	int _lastDisplayedPanPage;
	uint8 *_panPagesTable[20];
	Movie *_finalA, *_finalB, *_finalC;
	
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
	uint8 _brandonStatusBit0x02Flag;
	uint8 _brandonStatusBit0x20Flag;
	uint8 _brandonPoisonFlagsGFX[256];
	uint8 _deathHandler;
	int16 _brandonInvFlag;
	uint8 _poisonDeathCounter;
	int _brandonPosX;
	int _brandonPosY;

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

	uint8 _configTextspeed;
	uint8 _configWalkspeed;
	bool _configMusic;
	bool _configSounds;
	uint8 _configVoice;

	int _curMusicTheme;
	int _newMusicTheme;
	int16 _lastMusicCommand;

	Resource *_res;
	Screen *_screen;
	ScreenAnimator *_animator;
	Sound *_sound;
	SeqPlayer *_seq;
	Sprites *_sprites;
	TextDisplayer *_text;
	ScriptHelper *_scriptInterpreter;
	Debugger *_debugger;
	StaticResource *_staticres;
	Common::SaveFileManager *_saveFileMan;

	ScriptState *_scriptMain;
	
	ScriptState *_npcScript;
	ScriptData *_npcScriptData;
	
	ScriptState *_scriptClick;
	ScriptData *_scriptClickData;
	
	Character *_characterList;
	Character *_currentCharacter;
	
	Button *_buttonList;
	Button *_menuButtonList;
	bool _displayMenu;
	bool _menuRestoreScreen;
	bool _displaySubMenu;
	bool _cancelSubMenu;
	uint8 _toplevelMenu;
	int _savegameOffset;
	int _gameToLoad;
	char _savegameName[31];
	const char *_specialSavegameString;
	KeyboardEvent _keyboardEvent;

	struct KyragemState {
		uint16 nextOperation;
		uint16 rOffset;
		uint16 gOffset;
		uint16 bOffset;
		uint32 timerCount;
	} _kyragemFadingState;

	// TODO: get rid of all variables having pointers to the static resources if possible
	// i.e. let them directly use the _staticres functions
	void initStaticResource();

	const uint8 *_seq_Forest;
	const uint8 *_seq_KallakWriting;
	const uint8 *_seq_KyrandiaLogo;
	const uint8 *_seq_KallakMalcolm;
	const uint8 *_seq_MalcolmTree;
	const uint8 *_seq_WestwoodLogo;
	const uint8 *_seq_Demo1;
	const uint8 *_seq_Demo2;
	const uint8 *_seq_Demo3;
	const uint8 *_seq_Demo4;
	const uint8 *_seq_Reunion;
	
	const char * const*_seq_WSATable;
	const char * const*_seq_CPSTable;
	const char * const*_seq_COLTable;
	const char * const*_seq_textsTable;
	
	int _seq_WSATable_Size;
	int _seq_CPSTable_Size;
	int _seq_COLTable_Size;
	int _seq_textsTable_Size;
	
	const char * const*_itemList;
	const char * const*_takenList;
	const char * const*_placedList;
	const char * const*_droppedList;
	const char * const*_noDropList;
	const char * const*_putDownFirst;
	const char * const*_waitForAmulet;
	const char * const*_blackJewel;
	const char * const*_poisonGone;
	const char * const*_healingTip;
	const char * const*_thePoison;
	const char * const*_fluteString;
	const char * const*_wispJewelStrings;
	const char * const*_magicJewelString;
	const char * const*_flaskFull;
	const char * const*_fullFlask;
	const char * const*_veryClever;
	const char * const*_homeString;
	const char * const*_newGameString;
	
	const char *_voiceTextString;
	const char *_textSpeedString;
	const char *_onString;
	const char *_offString;
		
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
	int _veryClever_Size;
	int _homeString_Size;
	int _newGameString_Size;
	
	const char * const*_characterImageTable;
	int _characterImageTableSize;

	const char * const*_guiStrings;
	int _guiStringsSize;

	const char * const*_configStrings;
	int _configStringsSize;
	
	Shape *_defaultShapeTable;
	int _defaultShapeTableSize;
	
	const Shape *_healingShapeTable;
	int  _healingShapeTableSize;
	const Shape *_healingShape2Table;
	int  _healingShape2TableSize;
	
	const Shape *_posionDeathShapeTable;
	int _posionDeathShapeTableSize;
	
	const Shape *_fluteAnimShapeTable;
	int _fluteAnimShapeTableSize;
	
	const Shape *_winterScrollTable;
	int _winterScrollTableSize;
	const Shape *_winterScroll1Table;
	int _winterScroll1TableSize;
	const Shape *_winterScroll2Table;
	int _winterScroll2TableSize;
	
	const Shape *_drinkAnimationTable;
	int _drinkAnimationTableSize;
	
	const Shape *_brandonToWispTable;
	int _brandonToWispTableSize;
	
	const Shape *_magicAnimationTable;
	int _magicAnimationTableSize;
	
	const Shape *_brandonStoneTable;
	int _brandonStoneTableSize;
	
	Room *_roomTable;
	int _roomTableSize;
	const char * const*_roomFilenameTable;
	int _roomFilenameTableSize;
	
	const uint8 *_amuleteAnim;
	
	const uint8 * const*_specialPalettes;

	Timer _timers[34];
	uint32 _timerNextRun;	
	static const char *_musicFiles[];
	static const int _musicFilesCount;
	
	static const int8 _charXPosTable[];
	static const int8 _addXPosTable[];
	static const int8 _charYPosTable[];
	static const int8 _addYPosTable[];

	// positions of the inventory
	static const uint16 _itemPosX[];
	static const uint8 _itemPosY[];
	
	static Button _buttonData[];
	static Button *_buttonDataListPtr[];
	static Button _menuButtonData[];
	static Button _scrollUpButton;
	static Button _scrollDownButton;

	bool _haveScrollButtons;

	static Menu _menu[];

	static const uint8 _magicMouseItemStartFrame[];
	static const uint8 _magicMouseItemEndFrame[];
	static const uint8 _magicMouseItemStartFrame2[];
	static const uint8 _magicMouseItemEndFrame2[];

	static const uint16 _amuletX[];
	static const uint16 _amuletY[];
	static const uint16 _amuletX2[];
	static const uint16 _amuletY2[];
};

class KyraEngine_v1 : public KyraEngine {
public:
	KyraEngine_v1(OSystem *system);
	~KyraEngine_v1();

	int setupGameFlags();
};

class KyraEngine_v2 : public KyraEngine {
public:
	KyraEngine_v2(OSystem *system);
	~KyraEngine_v2();

	int setupGameFlags() { return 0; }
	
	int go();
};

// maybe subclass KyraEngine_v2 later
class KyraEngine_v3 : public KyraEngine {
public:
	KyraEngine_v3(OSystem *system);
	~KyraEngine_v3();
	
	Movie *createWSAMovie();

	int setupGameFlags() { return 0; }
	
	int go();
};

} // End of namespace Kyra

#endif
