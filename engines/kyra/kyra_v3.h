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

#ifndef KYRA_KYRA_V3_H
#define KYRA_KYRA_V3_H

#include "kyra/kyra.h"
#include "kyra/screen_v3.h"
#include "kyra/script.h"

#include "common/hashmap.h"
#include "common/list.h"

namespace Kyra {

class SoundDigital;
class Screen_v3;
class MainMenu;
class WSAMovieV2;
class TextDisplayer_v3;
class Debugger_v3;
class GUI_v3;
struct Button;

class KyraEngine_v3 : public KyraEngine {
friend class Debugger_v3;
friend class TextDisplayer_v3;
friend class GUI_v3;
public:
	KyraEngine_v3(OSystem *system, const GameFlags &flags);
	~KyraEngine_v3();

	Screen *screen() { return _screen; }
	SoundDigital *soundDigital() { return _soundDigital; }
	int language() const { return _lang; }

	int go();

	void playVQA(const char *name);

	virtual Movie *createWSAMovie();
private:
	Screen_v3 *_screen;
	SoundDigital *_soundDigital;

	int init();

	void preinit();
	void startup();
	void runStartupScript(int script, int unk1);

	void setupOpcodeTable();

	// run
	bool _runFlag;
	int _deathHandler;

	void runLoop();
	void handleInput(int x, int y);
	bool _unkHandleSceneChangeFlag;
	int inputSceneChange(int x, int y, int unk1, int unk2);

	void update();
	void updateWithText();
	void updateMouse();

	void delay(uint32 millis, bool update = false, bool isMainLoop = false);

	// - Input
	void updateInput();
	int checkInput(Button *buttonList, bool mainLoop = false);
	void removeInputTop();

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

	// sound specific
private:
	void playMenuAudioFile();

	int _musicSoundChannel;
	int _fadeOutMusicChannel;
	const char *_menuAudioFile;

	static const char *_soundList[];
	static const int _soundListSize;

	void playMusicTrack(int track, int force);
	void stopMusicTrack();

	int musicUpdate(int forceRestart);
	void fadeOutMusic(int ticks);

	void playSoundEffect(int item, int volume);

	static const uint8 _sfxFileMap[];
	static const int _sfxFileMapSize;
	static const char *_sfxFileList[];
	static const int _sfxFileListSize;

	int _voiceSoundChannel;

	void playVoice(int high, int low);
	void snd_playVoiceFile(int file);
	bool snd_voiceIsPlaying();
	void snd_stopVoice();

	int _curStudioSFX;
	void playStudioSFX(const char *str);

	// gui
	GUI_v3 *_gui;

	Button *_mainButtonData;
	Button *_mainButtonList;
	bool _mainButtonListInitialized;
	void initMainButtonList(bool disable);

	bool _enableInventory;
	int buttonInventory(Button *button);
	int buttonMoodChange(Button *button);
	int buttonShowScore(Button *button);
	int buttonJesterStaff(Button *button);

	// -> main menu
	void initMainMenu();
	void uninitMainMenu();

	WSAMovieV2 *_menuAnim;
	MainMenu *_menu;

	// timer
	void setupTimers();

	void setWalkspeed(uint8);
	void setCommandLineRestoreTimer(int secs);

	void timerRestoreCommandLine(int arg);
	void timerRunSceneScript7(int arg);
	void timerFleaDeath(int arg);

	uint32 _nextIdleAnim;
	void setNextIdleAnimTimer();

	// pathfinder
	int *_moveFacingTable;
	int _pathfinderFlag;

	int findWay(int x1, int y1, int x2, int y2, int *moveTable, int moveTableSize);
	bool lineIsPassable(int x, int y);

private:
	// main menu
	static const char *_mainMenuStrings[];

	// animator
	struct AnimObj {
		uint16 index;
		uint16 type;
		bool enabled;
		bool needRefresh;
		uint16 unk8;
		uint16 flags;
		int16 xPos1, yPos1;
		uint8 *shapePtr;
		uint16 shapeIndex;
		uint16 animNum;
		uint16 shapeIndex3;
		uint16 shapeIndex2;
		int16 xPos2, yPos2;
		int16 xPos3, yPos3;
		int16 width, height;
		int16 width2, height2;
		uint16 palette;
		AnimObj *nextObject;
	};

	AnimObj *_animObjects;
	uint8 *_gamePlayBuffer;

	void clearAnimObjects();

	AnimObj *_animList;
	bool _drawNoShapeFlag;
	AnimObj *initAnimList(AnimObj *list, AnimObj *entry);
	AnimObj *addToAnimListSorted(AnimObj *list, AnimObj *entry);
	AnimObj *deleteAnimListEntry(AnimObj *list, AnimObj *entry);

	void animSetupPaletteEntry(AnimObj *anim);

	void restorePage3();

	void drawAnimObjects();
	void drawSceneAnimObject(AnimObj *obj, int x, int y, int drawLayer);
	void drawCharacterAnimObject(AnimObj *obj, int x, int y, int drawLayer);

	void refreshAnimObjects(int force);
	void refreshAnimObjectsIfNeed();

	void flagAnimObjsForRefresh();

	bool _loadingState;
	void updateCharacterAnim(int charId);

	void updateSceneAnim(int anim, int newFrame);
	void setupSceneAnimObject(int anim, uint16 flags, int x, int y, int x2, int y2, int w, int h, int unk10, int specialSize, int unk14, int shape, const char *filename);
	void removeSceneAnimObject(int anim, int refresh);

	int _charBackUpWidth2, _charBackUpHeight2;
	int _charBackUpWidth, _charBackUpHeight;

	void setCharacterAnimDim(int w, int h);
	void resetCharacterAnimDim();

	bool _nextIdleType;
	void showIdleAnim();

	void addItemToAnimList(int item);
	void deleteItemAnimEntry(int item);

	// interface
	uint8 *_interface;
	uint8 *_interfaceCommandLine;

	void loadInterfaceShapes();
	void loadInterface();

	void showMessage(const char *string, uint8 c0, uint8 c1);
	void showMessageFromCCode(int string, uint8 c0, int);
	void updateItemCommand(int item, int str, uint8 c0);

	void updateCommandLine();
	void restoreCommandLine();
	void updateCLState();

	int _commandLineY;
	const char *_shownMessage;
	bool _restoreCommandLine;
	bool _inventoryState;
	int _inventoryScrollSpeed;

	void showInventory();
	void hideInventory();

	void drawMalcolmsMoodText();
	void drawMalcolmsMoodPointer(int frame, int page);
	void drawJestersStaff(int type, int page);
	
	void drawScore(int page, int x, int y);
	void drawScoreCounting(int oldScore, int newScore, int drawOld, const int x);
	int getScoreX(const char *str);

	static const uint8 _inventoryX[];
	static const uint8 _inventoryY[];
	void redrawInventory(int page);
	void clearInventorySlot(int slot, int page);
	void drawInventorySlot(int page, int item, int slot);

	WSAMovieV2 *_invWsa;
	int _invWsaFrame;

	// localization
	uint8 *_scoreFile;
	uint8 *_cCodeFile;
	uint8 *_scenesFile;
	uint8 *_itemFile;
	uint8 *_actorFile;
	uint32 _actorFileSize;
	uint8 *_sceneStrings;

	uint8 *getTableEntry(uint8 *buffer, int id);
	void getTableEntry(Common::SeekableReadStream *stream, int id, char *dst);

	// items
	int8 *_itemBuffer1;
	int8 *_itemBuffer2;
	struct Item {
		uint16 id;
		uint16 sceneId;
		int16 x, y;
		uint16 unk8;
	};

	Item *_itemList;
	uint16 _hiddenItems[100];

	void resetItem(int index);
	void resetItemList();
	static const uint8 _trashItemList[];
	void removeTrashItems();

	int findFreeItem();
	int findItem(uint16 item, uint16 scene);
	int findItem(uint16 item);

	int countAllItems();
	
	void initItems();

	int checkItemCollision(int x, int y);

	bool dropItem(int unk1, uint16 item, int x, int y, int unk2);
	bool processItemDrop(uint16 sceneId, uint16 item, int x, int y, int unk1, int unk2);
	void itemDropDown(int startX, int startY, int dstX, int dstY, int itemSlot, uint16 item, int remove);
	void exchangeMouseItem(int itemPos, int runScript);
	bool pickUpItem(int x, int y, int runScript);

	bool isDropable(int x, int y);

	static const uint8 _itemMagicTable[];
	bool itemListMagic(int handItem, int itemSlot);
	bool itemInventoryMagic(int handItem, int invSlot);

	static const uint8 _itemStringMap[];
	static const uint _itemStringMapSize;
	static const uint8 _itemStringPickUp[];
	static const uint8 _itemStringDrop[];
	static const uint8 _itemStringInv[];

	int getItemCommandStringPickUp(uint16 item);
	int getItemCommandStringDrop(uint16 item);
	int getItemCommandStringInv(uint16 item);

	// -> hand item
	void setMouseCursor(uint16 item);

	void setHandItem(uint16 item);
	void removeHandItem();
	void setItemMouseCursor();

	int _itemInHand;
	int _handItemSet;

	// shapes
	typedef Common::HashMap<int, uint8*> ShapeMap;
	ShapeMap _gameShapes;

	void addShapeToPool(const uint8 *data, int realIndex, int shape);
	uint8 *getShapePtr(int shape) const;

	void initMouseShapes();

	int _malcolmShapes;
	void loadMalcolmShapes(int newShapes);
	void updateMalcolmShapes();

	int _malcolmShapeXOffset, _malcolmShapeYOffset;

	struct ShapeDesc {
		uint8 width, height;
		int8 xOffset, yOffset;
	};
	static const ShapeDesc _shapeDescs[];
	static const int _shapeDescsSize;

	// scene animation
	struct SceneAnim {
		uint16 flags;
		int16 x, y;
		int16 x2, y2;
		int16 width, height;
		uint16 unk10;
		uint16 specialSize;
		uint16 unk14;
		uint16 shapeIndex;
		uint16 wsaFlag;
		char filename[13];
	};

	SceneAnim *_sceneAnims;
	WSAMovieV2 *_sceneAnimMovie[16];
	uint8 *_sceneShapes[20];

	void freeSceneShapes();
	void freeSceneAnims();

	// voice
	int _currentTalkFile;
	void openTalkFile(int file);

	// scene
	struct SceneDesc {
		char filename1[10];
		char filename2[10];
		uint16 exit1, exit2, exit3, exit4;
		uint8 flags, sound;
	};

	SceneDesc *_sceneList;
	int _sceneListSize;
	uint16 _sceneExit1, _sceneExit2, _sceneExit3, _sceneExit4;
	int _sceneEnterX1, _sceneEnterY1;
	int _sceneEnterX2, _sceneEnterY2;
	int _sceneEnterX3, _sceneEnterY3;
	int _sceneEnterX4, _sceneEnterY4;

	int _specialExitCount;
	uint16 _specialExitTable[25];
	bool checkSpecialSceneExit(int index, int x, int y);

	bool _noScriptEnter;
	void enterNewScene(uint16 scene, int facing, int unk1, int unk2, int unk3);
	void enterNewSceneUnk1(int facing, int unk1, int unk2);
	void enterNewSceneUnk2(int unk1);
	int _enterNewSceneLock;

	void unloadScene();

	void loadScenePal();
	void loadSceneMsc();
	void initSceneScript(int unk1);
	void initSceneAnims(int unk1);
	void initSceneScreen(int unk1);

	int runSceneScript1(int x, int y);
	int runSceneScript2();
	bool _noStartupChat;
	void runSceneScript4(int unk1);
	void runSceneScript6();
	void runSceneScript8();

	int _sceneMinX, _sceneMaxX;
	int _maskPageMinY, _maskPageMaxY;

	ScriptState _sceneScriptState;
	ScriptData _sceneScriptData;

	bool _specialSceneScriptState[10];
	bool _specialSceneScriptStateBackup[10];
	ScriptState _sceneSpecialScripts[10];
	uint32 _sceneSpecialScriptsTimer[10];
	int _lastProcessedSceneScript;
	bool _specialSceneScriptRunFlag;

	void updateSpecialSceneScripts();

	int trySceneChange(int *moveTable, int unk1, int unk2);
	int checkSceneChange();

	int8 _sceneDatPalette[45];
	int8 _sceneDatLayerTable[15];
	struct SceneShapeDesc {
		// the original saves those variables, we don't, since
		// they are just needed on scene load
		/*int x, y;
		int w, h;*/
		int drawX, drawY;
	};
	SceneShapeDesc _sceneShapeDescs[20];

	int getDrawLayer(int x, int y);

	int getScale(int x, int y);
	int _scaleTable[15];

	bool _unkSceneScreenFlag1;

	// character
	struct Character {
		uint16 sceneId;
		uint16 dlgIndex;
		uint8 height;
		uint8 facing;
		uint16 animFrame;
		//uint8 unk8, unk9;
		byte walkspeed;
		uint16 inventory[10];
		int16 x1, y1;
		int16 x2, y2;
		int16 x3, y3;
	};

	Character _mainCharacter;
	int _mainCharX, _mainCharY;
	int _charScale;

	void moveCharacter(int facing, int x, int y);

	void updateCharPosWithUpdate();
	int updateCharPos(int *table, int force);

	uint32 _updateCharPosNextUpdate;
	static const int8 _updateCharPosXTable[];
	static const int8 _updateCharPosYTable[];

	void updateCharAnimFrame(int character, int *table);
	int8 _characterAnimTable[2];
	static const uint8 _characterFrameTable[];

	bool _overwriteSceneFacing;

	void updateCharPal(int unk1);
	int _lastCharPalLayer;
	bool _charPalUpdate;

	bool checkCharCollision(int x, int y);

	int _malcolmsMood;

	void makeCharFacingMouse();

	int findFreeInventorySlot();

	// talk object
	struct TalkObject {
		char filename[13];
		int8 sceneAnim;
		int8 sceneScript;
		int16 x, y;
		uint8 color;
		uint8 sceneId;
	};

	TalkObject *_talkObjectList;

	bool talkObjectsInCurScene();

	// chat
	int _vocHigh;

	const char *_chatText;
	int _chatObject;
	uint32 _chatEndTime;
	int _chatVocHigh, _chatVocLow;

	ScriptData _chatScriptData;
	ScriptState _chatScriptState;

	int chatGetType(const char *text);
	int chatCalcDuration(const char *text);

	void objectChat(const char *text, int object, int vocHigh, int vocLow);
	void objectChatInit(const char *text, int object, int vocHigh, int vocLow);
	void objectChatPrintText(const char *text, int object);
	void objectChatProcess(const char *script);
	void objectChatWaitToFinish();

	void badConscienceChat(const char *str, int vocHigh, int vocLow);
	void badConscienceChatWaitToFinish();

	void malcolmSceneStartupChat();

	byte _newSceneDlgState[40];
	int8 _conversationState[30][30];
	bool _chatAltFlag;
	void setDlgIndex(uint16 index);
	void updateDlgIndex();

	Common::SeekableReadStream *_cnvFile;
	Common::SeekableReadStream *_dlgBuffer;
	int _curDlgChapter, _curDlgIndex, _curDlgLang;
	void updateDlgBuffer();
	void loadDlgHeader(int &vocHighBase, int &vocHighIndex, int &index1, int &index2);

	static const uint8 _vocHighTable[];
	bool _isStartupDialog;
	void processDialog(int vocHighIndex, int vocHighBase, int funcNum);

	ScriptData _dialogScriptData;
	ScriptState _dialogScriptState;
	int _dialogSceneAnim;
	int _dialogSceneScript;
	int _dialogScriptFuncStart, _dialogScriptFuncProc, _dialogScriptFuncEnd;

	void dialogStartScript(int object, int funcNum);
	void dialogEndScript(int object);

	void npcChatSequence(const char *str, int object, int vocHigh, int vocLow);

	Common::Array<const Opcode *> _opcodesDialog;

	int o3d_updateAnim(ScriptState *script);
	int o3d_delay(ScriptState *script);

	void malcolmRandomChat();
	void runDialog(int dlgIndex, int funcNum);

	// conscience
	bool _badConscienceShown;
	int _badConscienceAnim;
	bool _badConsciencePosition;

	static const uint8 _badConscienceFrameTable[];

	void showBadConscience();
	void hideBadConscience();

	// special script code
	bool _temporaryScriptExecBit;
	bool _useFrameTable;
	
	Common::Array<const Opcode *> _opcodesTemporary;

	int o3t_defineNewShapes(ScriptState *script);
	int o3t_setCurrentFrame(ScriptState *script);
	int o3t_setNewShapeFlag(ScriptState *script);

	ScriptData _temporaryScriptData;
	ScriptState _temporaryScriptState;

	void runTemporaryScript(const char *filename, int allowSkip, int resetChar, int newShapes, int shapeUnload);

	// special shape code
	char _newShapeFilename[13];
	int _newShapeLastEntry;
	int _newShapeWidth, _newShapeHeight;
	int _newShapeXAdd, _newShapeYAdd;

	int _newShapeAnimFrame;
	int _newShapeDelay;

	int _newShapeFlag;
	uint8 *_newShapeFiledata;
	int _newShapeCount;

	int initNewShapes(uint8 *filedata);
	void processNewShapes(int allowSkip, int resetChar);
	void resetNewShapes(int count, uint8 *filedata);

	// unk
	uint8 *_costPalBuffer;
	uint8 *_screenBuffer;
	uint8 *_paletteOverlay;
	bool _useActorBuffer;

	int _currentChapter;
	void changeChapter(int newChapter, int sceneId, int malcolmShapes, int facing);

	static const uint8 _chapterLowestScene[];

	int _unk3, _unk4, _unk5;

	void loadCostPal();
	void loadShadowShape();
	void loadExtrasShapes();

	uint8 *_gfxBackUpRect;
	void backUpGfxRect32x32(int x, int y);
	void restoreGfxRect32x32(int x, int y);

	char *_stringBuffer;

	int _score;
	int _scoreMax;
	
	static const int8 _scoreTable[];
	static const int _scoreTableSize;
	int8 _scoreFlagTable[26];
	bool updateScore(int scoreId, int strId);
	void scoreIncrease(int count, const char *str);

	// save/load
	void saveGame(const char *fileName, const char *saveName);
	void loadGame(const char *fileName);

	// opcodes
	int o3_getMalcolmShapes(ScriptState *script);
	int o3_setCharacterPos(ScriptState *script);
	int o3_defineObject(ScriptState *script);
	int o3_refreshCharacter(ScriptState *script);
	int o3_getCharacterX(ScriptState *script);
	int o3_getCharacterY(ScriptState *script);
	int o3_getCharacterFacing(ScriptState *script);
	int o3_getCharacterScene(ScriptState *script);
	int o3_getMalcolmsMood(ScriptState *script);
	int o3_getCharacterFrameFromFacing(ScriptState *script);
	int o3_setCharacterFacingOverwrite(ScriptState *script);
	int o3_trySceneChange(ScriptState *script);
	int o3_moveCharacter(ScriptState *script);
	int o3_setCharacterFacing(ScriptState *script);
	int o3_showSceneFileMessage(ScriptState *script);
	int o3_setCharacterAnimFrameFromFacing(ScriptState *script);
	int o3_showBadConscience(ScriptState *script);
	int o3_hideBadConscience(ScriptState *script);
	int o3_setInventorySlot(ScriptState *script);
	int o3_getInventorySlot(ScriptState *script);
	int o3_addItemToInventory(ScriptState *script);
	int o3_addItemToCurScene(ScriptState *script);
	int o3_objectChat(ScriptState *script);
	int o3_checkForItem(ScriptState *script);
	int o3_resetInventory(ScriptState *script);
	int o3_defineItem(ScriptState *script);
	int o3_removeInventoryItemInstances(ScriptState *script);
	int o3_countInventoryItemInstances(ScriptState *script);
	int o3_npcChatSequence(ScriptState *script);
	int o3_queryGameFlag(ScriptState *script);
	int o3_resetGameFlag(ScriptState *script);
	int o3_setGameFlag(ScriptState *script);
	int o3_setHandItem(ScriptState *script);
	int o3_removeHandItem(ScriptState *script);
	int o3_handItemSet(ScriptState *script);
	int o3_hideMouse(ScriptState *script);
	int o3_addSpecialExit(ScriptState *script);
	int o3_setMousePos(ScriptState *script);
	int o3_showMouse(ScriptState *script);
	int o3_badConscienceChat(ScriptState *script);
	int o3_wipeDownMouseItem(ScriptState *script);
	int o3_setMalcolmsMood(ScriptState *script);
	int o3_delay(ScriptState *script);
	int o3_updateScore(ScriptState *script);
	int o3_makeSecondChanceSave(ScriptState *script);
	int o3_setSceneFilename(ScriptState *script);
	int o3_removeItemsFromScene(ScriptState *script);
	int o3_disguiseMalcolm(ScriptState *script);
	int o3_drawSceneShape(ScriptState *script);
	int o3_drawSceneShapeOnPage(ScriptState *script);
	int o3_checkInRect(ScriptState *script);
	int o3_updateConversations(ScriptState *script);
	int o3_setSceneDim(ScriptState *script);
	int o3_update(ScriptState *script);
	int o3_setSceneAnimPosAndFrame(ScriptState *script);
	int o3_removeItemInstances(ScriptState *script);
	int o3_disableInventory(ScriptState *script);
	int o3_enableInventory(ScriptState *script);
	int o3_enterNewScene(ScriptState *script);
	int o3_switchScene(ScriptState *script);
	int o3_setMalcolmPos(ScriptState *script);
	int o3_stopMusic(ScriptState *script);
	int o3_playMusicTrack(ScriptState *script);
	int o3_playSoundEffect(ScriptState *script);
	int o3_getScore(ScriptState *script);
	int o3_blockOutRegion(ScriptState *script);
	int o3_showSceneStringsMessage(ScriptState *script);
	int o3_getRand(ScriptState *script);
	int o3_setDeathHandler(ScriptState *script);
	int o3_waitForConfirmationClick(ScriptState *script);
	int o3_defineRoomEntrance(ScriptState *script);
	int o3_runTemporaryScript(ScriptState *script);
	int o3_setSpecialSceneScriptRunTime(ScriptState *script);
	int o3_defineSceneAnim(ScriptState *script);
	int o3_updateSceneAnim(ScriptState *script);
	int o3_runActorScript(ScriptState *script);
	int o3_runDialog(ScriptState *script);
	int o3_malcolmRandomChat(ScriptState *script);
	int o3_setDlgIndex(ScriptState *script);
	int o3_getDlgIndex(ScriptState *script);
	int o3_defineScene(ScriptState *script);
	int o3_changeChapter(ScriptState *script);
	int o3_countItemInstances(ScriptState *script);
	int o3_dialogStartScript(ScriptState *script);
	int o3_dialogEndScript(ScriptState *script);
	int o3_setSpecialSceneScriptState(ScriptState *script);
	int o3_clearSpecialSceneScriptState(ScriptState *script);
	int o3_querySpecialSceneScriptState(ScriptState *script);
	int o3_setHiddenItemsEntry(ScriptState *script);
	int o3_getHiddenItemsEntry(ScriptState *script);
	int o3_customChat(ScriptState *script);
	int o3_customChatFinish(ScriptState *script);
	int o3_setupSceneAnimObject(ScriptState *script);
	int o3_removeSceneAnimObject(ScriptState *script);
	int o3_disableTimer(ScriptState *script);
	int o3_enableTimer(ScriptState *script);
	int o3_setTimerCountdown(ScriptState *script);
	int o3_setVocHigh(ScriptState *script);
	int o3_getVocHigh(ScriptState *script);
	int o3_dummy(ScriptState *script);

	// misc
	TextDisplayer_v3 *_text;
	Debugger_v3 *_debugger;	
	bool _wsaPlayingVQA;

	// resource specific
private:
	static const char *_languageExtension[];
	static const int _languageExtensionSize;

	char *appendLanguage(char *buf, int lang, int bufSize);

	int loadLanguageFile(const char *file, uint8 *&buffer);
};

} // end of namespace Kyra

#endif

