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
	uint32 unkField3[12];	// maybe pointer to shape of the item
};

struct AnimObject {
	uint8 index;
	uint32 active;
	uint32 refreshFlag;
	uint32 bkgdChangeFlag;
	uint32 unk1;
	uint32 flags;
	int16 drawY;
	uint8 *sceneAnimPtr;
	uint16 animFrameNumber;
	uint8 *background;
	uint16 rectSize;
	int16 x1, y1;
	int16 x2, y2;
	uint16 width;
	uint16 height;
	uint16 width2;
	uint16 height2;
	AnimObject *nextAnimObject;
};

struct Rect {
	int x, y;
	int x2, y2;
};

struct TalkCoords {
	uint16 y, x, w;
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

struct WSAMovieV1;

class MusicPlayer;
class SeqPlayer;
class Resource;
class PAKFile;
class Screen;
class Sprites;
struct ScriptState;
struct ScriptData;
class ScriptHelper;

class KyraEngine : public Engine {
	friend class MusicPlayer;
public:

	enum {
		TALK_SUBSTRING_LEN = 80,
		TALK_SUBSTRING_NUM = 3
	};
	
	enum {
		MUSIC_INTRO = 0
	};

	KyraEngine(GameDetector *detector, OSystem *system);
	~KyraEngine();
	
	void errorString(const char *buf_input, char *buf_output);

	Resource *resource() { return _res; }
	Screen *screen() { return _screen; }
	MusicPlayer *midi() { return _midi; }

	uint8 game() const { return _game; }
	uint32 features() const { return _features; }
	SceneExits sceneExits() const { return _sceneExits; }
	// ugly hack used by the dat loader
	SceneExits &sceneExits() { return _sceneExits; }
	
	Common::RandomSource _rnd;
	int16 _northExitHeight;

	typedef void (KyraEngine::*IntroProc)();
	typedef int (KyraEngine::*OpcodeProc)(ScriptState *script);

	const char **seqWSATable() { return (const char **)_seq_WSATable; }
	const char **seqCPSTable() { return (const char **)_seq_CPSTable; }
	const char **seqCOLTable() { return (const char **)_seq_COLTable; }
	const char **seqTextsTable() { return (const char **)_seq_textsTable; }

	bool seq_skipSequence() const;
	
	void loadBitmap(const char *filename, int tempPage, int dstPage, uint8 *palData);

	void snd_playTheme(int file, int track = 0);
	void snd_playTrack(int track);
	void snd_playVoiceFile(int id);
	bool snd_voicePlaying();

	void printTalkTextMessage(const char *text, int x, int y, uint8 color, int srcPage, int dstPage);
	void restoreTalkTextMessageBkgd(int srcPage, int dstPage);

	WSAMovieV1 *wsa_open(const char *filename, int offscreenDecode, uint8 *palBuf);
	void wsa_close(WSAMovieV1 *wsa);
	uint16 wsa_getNumFrames(WSAMovieV1 *wsa) const;
	void wsa_play(WSAMovieV1 *wsa, int frameNum, int x, int y, int pageNum);

	void waitTicks(int ticks);
	
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
	int cmd_CopyWSARegion(ScriptState *script);
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
	int cmd_Poison_Brandon_And_Remaps(ScriptState *script);
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
	void setTalkCoords(uint16 y);
	int getCenterStringX(const char *str, int x1, int x2);
	int getCharLength(const char *str, int len);
	int dropCRIntoString(char *str, int offs);
	char *preprocessString(const char *str);
	int buildMessageSubstrings(const char *str);
	int getWidestLineWidth(int linesCount);
	void calcWidestLineBounds(int &x1, int &x2, int w, int cx);
	void printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2);
	void setCharacterDefaultFrame(int character);
	void setCharactersPositions(int character);
	void setCharactersHeight();
	int setGameFlag(int flag);
	int queryGameFlag(int flag);
	int resetGameFlag(int flag);
	
	void enterNewScene(int sceneId, int facing, int unk1, int unk2, int brandonAlive);
	void moveCharacterToPos(int character, int facing, int xpos, int ypos);
	void setCharacterPositionWithUpdate(int character);
	int setCharacterPosition(int character, int *facingTable);
	void setCharacterPositionHelper(int character, int *facingTable);
	int getOppositeFacingDirection(int dir);
	void loadSceneMSC();
	void blockInRegion(int x, int y, int width, int height);
	void blockOutRegion(int x, int y, int width, int height);
	void startSceneScript(int brandonAlive);
	void initSceneData(int facing, int unk1, int brandonAlive);
	void clearNoDropRects();
	void addToNoDropRects(int x, int y, int w, int h);
	byte findFreeItemInScene(int scene);
	byte findItemAtPos(int x, int y);
	void placeItemInGenericMapScene(int item, int index);
	void initSceneObjectList(int brandonAlive);
	void restoreAllObjectBackgrounds();
	void preserveAnyChangedBackgrounds();
	void preserveOrRestoreBackground(AnimObject *obj, bool restore);
	void prepDrawAllObjects();
	void copyChangedObjectsForward(int refreshFlag);
	void updateAllObjectShapes();
	void animRefreshNPC(int character);
	int findDuplicateItemShape(int shape);
	int16 fetchAnimWidth(const uint8 *shape, int16 mult);
	int8 fetchAnimHeight(const uint8 *shape, int8 mult);
	int findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize);
	int findSubPath(int x, int y, int toX, int toY, int *moveTable, int start, int end);
	int getFacingFromPointToPoint(int x, int y, int toX, int toY);
	void changePosTowardsFacing(int &x, int &y, int facing);
	bool lineIsPassable(int x, int y);
	int getMoveTableSize(int *moveTable);
	int handleSceneChange(int xpos, int ypos, int unk1, int frameReset);
	int processSceneChange(int *table, int unk1, int frameReset);
	int changeScene(int facing);
	
	AnimObject *objectRemoveQueue(AnimObject *queue, AnimObject *rem);
	AnimObject *objectAddHead(AnimObject *queue, AnimObject *head);
	AnimObject *objectQueue(AnimObject *queue, AnimObject *add);
	
	void seq_demo();
	void seq_intro();
	void seq_introLogos();
	void seq_introStory();
	void seq_introMalcolmTree();
	void seq_introKallakWriting();
	void seq_introKallakMalcolm();

	void wsa_processFrame(WSAMovieV1 *wsa, int frameNum, uint8 *dst);

	void snd_startTrack();
	void snd_haltTrack();
	void snd_setSoundEffectFile(int file);
	void snd_playSoundEffect(int track);
	
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
	
	void delay(uint32 millis);
	void loadPalette(const char *filename, uint8 *palData);
	void loadMouseShapes();
	void loadCharacterShapes();
	void loadSpecialEffectShapes();
	void loadItems();
	void loadMainScreen();
	void setCharactersInDefaultScene();
	void resetBrandonPosionFlags();
	void initAnimStateList();

	uint8 _game;
	bool _fastMode;
	bool _quitFlag;
	bool _skipIntroFlag;
	bool _abortIntroFlag;
	char _talkBuffer[300];
	char _talkSubstrings[TALK_SUBSTRING_LEN * TALK_SUBSTRING_NUM];
	TalkCoords _talkCoords;
	uint16 _talkMessageY;
	uint16 _talkMessageH;
	bool _talkMessagePrinted;
	uint8 _flagsTable[53];
	uint8 *_shapes[377];
	uint16 _gameSpeed;
	uint32 _features;
	int _mouseX, _mouseY;
	bool _needMouseUpdate;
	
	WSAMovieV1 *_wsaObjects[10];
	uint16 _entranceMouseCursorTracks[8];
	uint16 _walkBlockNorth;
	uint16 _walkBlockEast;
	uint16 _walkBlockSouth;
	uint16 _walkBlockWest;
	
	int32 _scaleMode;
	int16 _scaleTable[145];
	
	Rect _noDropRects[11];
	
	uint16 _birthstoneGemTable[4];
	uint8 _idolGemsTable[3];
	
	uint16 _marbleVaseItem;
	
	uint16 _brandonStatusBit;
	uint8 _unkBrandonPoisonFlags[256];	// this seem not to be posion flags, it is used for drawing once
	int _brandonPosX;
	int _brandonPosY;
	int16 _brandonScaleX;
	int16 _brandonScaleY;
	int _brandonDrawFrame;
	
	int8 *_sceneAnimTable[50];
	
	Item _itemTable[145];
	
	uint16 *_exitListPtr;
	uint16 _exitList[11];
	SceneExits _sceneExits;
	uint16 _currentRoom;
	uint8 *_maskBuffer;
	
	int _sceneChangeState;
	int _loopFlag2;
	
	int _pathfinderFlag;
	int _pathfinderFlag2;
	int _lastFindWayRet;
	int *_movFacingTable;
	
	AnimObject *_objectQueue;
	AnimObject *_animStates;
	AnimObject *_charactersAnimState;
	AnimObject *_animObjects;
	AnimObject *_animItems;
	
	int _curMusicTheme;
	int _newMusicTheme;
	AudioStream *_currentVocFile;
	Audio::SoundHandle _vocHandle;

	Resource *_res;
	Screen *_screen;
	MusicPlayer *_midi;
	SeqPlayer *_seq;
	Sprites *_sprites;
	ScriptHelper *_scriptInterpreter;
	
	ScriptState *_scriptMain;
	ScriptData *_npcScriptData;
	
	ScriptState *_scriptClick;	// TODO: rename to a better name
	ScriptData *_scriptClickData;
	
	Character *_characterList;
	Character *_currentCharacter;
	
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
	
	char **_characterImageTable;
	int _characterImageTableSize;
	
	Shape *_defaultShapeTable;
	int _defaultShapeTableSize;
	
	Room *_roomTable;
	int _roomTableSize;	
	char **_roomFilenameTable;
	int _roomFilenameTableSize;
	
	static const char *_xmidiFiles[];
	static const int _xmidiFilesCount;
	
	static const int8 _charXPosTable[];
	static const int8 _addXPosTable[];
	static const int8 _charYPosTable[];
	static const int8 _addYPosTable[];
};

} // End of namespace Kyra

#endif
