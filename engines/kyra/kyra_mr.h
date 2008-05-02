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

#include "kyra/kyra_v2.h"
#include "kyra/screen_mr.h"
#include "kyra/script.h"

#include "common/hashmap.h"
#include "common/list.h"

namespace Kyra {

class SoundDigital;
class Screen_MR;
class MainMenu;
class WSAMovieV2;
class TextDisplayer_MR;
class Debugger_v3;
class GUI_v3;
struct Button;

class KyraEngine_MR : public KyraEngine_v2 {
friend class Debugger_v3;
friend class TextDisplayer_MR;
friend class GUI_v3;
public:
	KyraEngine_MR(OSystem *system, const GameFlags &flags);
	~KyraEngine_MR();

	Screen *screen() { return _screen; }
	Screen_v2 *screen_v2() const { return _screen; }
	SoundDigital *soundDigital() { return _soundDigital; }
	int language() const { return _lang; }

	int go();

	void playVQA(const char *name);

protected:
	// KyraEngine_v2 API
	int getFirstSpecialSceneScript() const { return 9; }

	// --
	Screen_MR *_screen;
	SoundDigital *_soundDigital;

	int init();

	void preinit();
	void startup();
	void runStartupScript(int script, int unk1);

	void setupOpcodeTable();

	// run
	bool _showOutro;
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
	int checkInput(Button *buttonList, bool mainLoop = false);

	// sound specific
private:
	void playMenuAudioFile();

	int _musicSoundChannel;
	int _fadeOutMusicChannel;
	const char *_menuAudioFile;

	static const char *_soundList[];
	static const int _soundListSize;

	void snd_playWanderScoreViaMap(int track, int force);
	void stopMusicTrack();

	int musicUpdate(int forceRestart);
	void fadeOutMusic(int ticks);

	void snd_playSoundEffect(int item, int volume);

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
	bool lineIsPassable(int x, int y);

private:
	// main menu
	static const char *_mainMenuStrings[];

	// animator
	uint8 *_gamePlayBuffer;
	void restorePage3();

	AnimObj *_animObjects;

	void clearAnimObjects();

	void animSetupPaletteEntry(AnimObj *anim);

	void drawAnimObjects();
	void drawSceneAnimObject(AnimObj *obj, int x, int y, int drawLayer);
	void drawCharacterAnimObject(AnimObj *obj, int x, int y, int drawLayer);

	void refreshAnimObjects(int force);

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
	uint16 _hiddenItems[100];

	static const uint8 _trashItemList[];
	void removeTrashItems();

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
	void setItemMouseCursor();
	void setMouseCursor(uint16 item);

	// shapes
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
	uint8 *_sceneShapes[20];

	void freeSceneShapes();

	// voice
	int _currentTalkFile;
	void openTalkFile(int file);

	// scene
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

	EMCState _sceneScriptState;
	EMCData _sceneScriptData;

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

	EMCData _chatScriptData;
	EMCState _chatScriptState;

	int chatGetType(const char *text);
	int chatCalcDuration(const char *text);

	void objectChat(const char *text, int object, int vocHigh, int vocLow);
	void objectChatInit(const char *text, int object, int vocHigh, int vocLow);
	void objectChatPrintText(const char *text, int object);
	void objectChatProcess(const char *script);
	void objectChatWaitToFinish();

	void badConscienceChat(const char *str, int vocHigh, int vocLow);
	void badConscienceChatWaitToFinish();

	void goodConscienceChat(const char *str, int vocHigh, int vocLow);
	void goodConscienceChatWaitToFinish();

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

	EMCData _dialogScriptData;
	EMCState _dialogScriptState;
	int _dialogSceneAnim;
	int _dialogSceneScript;
	int _dialogScriptFuncStart, _dialogScriptFuncProc, _dialogScriptFuncEnd;

	void dialogStartScript(int object, int funcNum);
	void dialogEndScript(int object);

	void npcChatSequence(const char *str, int object, int vocHigh, int vocLow);

	Common::Array<const Opcode *> _opcodesDialog;

	int o3d_updateAnim(EMCState *script);
	int o3d_delay(EMCState *script);

	void malcolmRandomChat();
	void runDialog(int dlgIndex, int funcNum);

	// conscience
	bool _badConscienceShown;
	int _badConscienceAnim;
	bool _badConsciencePosition;

	static const uint8 _badConscienceFrameTable[];

	void showBadConscience();
	void hideBadConscience();

	bool _goodConscienceShown;
	int _goodConscienceAnim;
	bool _goodConsciencePosition;

	static const uint8 _goodConscienceFrameTable[];

	void showGoodConscience();
	void hideGoodConscience();

	// special script code
	bool _temporaryScriptExecBit;
	bool _useFrameTable;
	
	Common::Array<const Opcode *> _opcodesTemporary;

	int o3t_defineNewShapes(EMCState *script);
	int o3t_setCurrentFrame(EMCState *script);
	int o3t_setNewShapeFlag(EMCState *script);

	EMCData _temporaryScriptData;
	EMCState _temporaryScriptState;

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

	void eelScript();

	// save/load
	void saveGame(const char *fileName, const char *saveName);
	void loadGame(const char *fileName);

	// opcodes
	int o3_getMalcolmShapes(EMCState *script);
	int o3_setCharacterPos(EMCState *script);
	int o3_defineObject(EMCState *script);
	int o3_refreshCharacter(EMCState *script);
	int o3_getCharacterX(EMCState *script);
	int o3_getCharacterY(EMCState *script);
	int o3_getCharacterFacing(EMCState *script);
	int o3_getCharacterScene(EMCState *script);
	int o3_getMalcolmsMood(EMCState *script);
	int o3_getCharacterFrameFromFacing(EMCState *script);
	int o3_setCharacterFacingOverwrite(EMCState *script);
	int o3_trySceneChange(EMCState *script);
	int o3_moveCharacter(EMCState *script);
	int o3_setCharacterFacing(EMCState *script);
	int o3_showSceneFileMessage(EMCState *script);
	int o3_setCharacterAnimFrameFromFacing(EMCState *script);
	int o3_showBadConscience(EMCState *script);
	int o3_hideBadConscience(EMCState *script);
	int o3_setInventorySlot(EMCState *script);
	int o3_getInventorySlot(EMCState *script);
	int o3_addItemToInventory(EMCState *script);
	int o3_addItemToCurScene(EMCState *script);
	int o3_objectChat(EMCState *script);
	int o3_checkForItem(EMCState *script);
	int o3_resetInventory(EMCState *script);
	int o3_defineItem(EMCState *script);
	int o3_removeInventoryItemInstances(EMCState *script);
	int o3_countInventoryItemInstances(EMCState *script);
	int o3_npcChatSequence(EMCState *script);
	int o3_queryGameFlag(EMCState *script);
	int o3_resetGameFlag(EMCState *script);
	int o3_setGameFlag(EMCState *script);
	int o3_setHandItem(EMCState *script);
	int o3_removeHandItem(EMCState *script);
	int o3_handItemSet(EMCState *script);
	int o3_hideMouse(EMCState *script);
	int o3_addSpecialExit(EMCState *script);
	int o3_setMousePos(EMCState *script);
	int o3_showMouse(EMCState *script);
	int o3_badConscienceChat(EMCState *script);
	int o3_wipeDownMouseItem(EMCState *script);
	int o3_setMalcolmsMood(EMCState *script);
	int o3_delay(EMCState *script);
	int o3_updateScore(EMCState *script);
	int o3_makeSecondChanceSave(EMCState *script);
	int o3_setSceneFilename(EMCState *script);
	int o3_removeItemsFromScene(EMCState *script);
	int o3_disguiseMalcolm(EMCState *script);
	int o3_drawSceneShape(EMCState *script);
	int o3_drawSceneShapeOnPage(EMCState *script);
	int o3_checkInRect(EMCState *script);
	int o3_updateConversations(EMCState *script);
	int o3_setSceneDim(EMCState *script);
	int o3_update(EMCState *script);
	int o3_setSceneAnimPosAndFrame(EMCState *script);
	int o3_removeItemInstances(EMCState *script);
	int o3_disableInventory(EMCState *script);
	int o3_enableInventory(EMCState *script);
	int o3_enterNewScene(EMCState *script);
	int o3_switchScene(EMCState *script);
	int o3_getShapeFlag1(EMCState *script);
	int o3_setMalcolmPos(EMCState *script);
	int o3_stopMusic(EMCState *script);
	int o3_playWanderScoreViaMap(EMCState *script);
	int o3_playSoundEffect(EMCState *script);
	int o3_getScore(EMCState *script);
	int o3_blockOutRegion(EMCState *script);
	int o3_showSceneStringsMessage(EMCState *script);
	int o3_getRand(EMCState *script);
	int o3_setDeathHandler(EMCState *script);
	int o3_showGoodConscience(EMCState *script);
	int o3_goodConscienceChat(EMCState *script);
	int o3_hideGoodConscience(EMCState *script);
	int o3_waitForConfirmationClick(EMCState *script);
	int o3_defineRoomEntrance(EMCState *script);
	int o3_runTemporaryScript(EMCState *script);
	int o3_setSpecialSceneScriptRunTime(EMCState *script);
	int o3_defineSceneAnim(EMCState *script);
	int o3_updateSceneAnim(EMCState *script);
	int o3_runActorScript(EMCState *script);
	int o3_runDialog(EMCState *script);
	int o3_malcolmRandomChat(EMCState *script);
	int o3_setDlgIndex(EMCState *script);
	int o3_getDlgIndex(EMCState *script);
	int o3_defineScene(EMCState *script);
	int o3_setConversationState(EMCState *script);
	int o3_getConversationState(EMCState *script);
	int o3_changeChapter(EMCState *script);
	int o3_countItemInstances(EMCState *script);
	int o3_dialogStartScript(EMCState *script);
	int o3_dialogEndScript(EMCState *script);
	int o3_setSpecialSceneScriptState(EMCState *script);
	int o3_clearSpecialSceneScriptState(EMCState *script);
	int o3_querySpecialSceneScriptState(EMCState *script);
	int o3_setHiddenItemsEntry(EMCState *script);
	int o3_getHiddenItemsEntry(EMCState *script);
	int o3_customChat(EMCState *script);
	int o3_customChatFinish(EMCState *script);
	int o3_setupSceneAnimObject(EMCState *script);
	int o3_removeSceneAnimObject(EMCState *script);
	int o3_disableTimer(EMCState *script);
	int o3_enableTimer(EMCState *script);
	int o3_setTimerCountdown(EMCState *script);
	int o3_setVocHigh(EMCState *script);
	int o3_getVocHigh(EMCState *script);
	int o3_dummy(EMCState *script);

	// misc
	TextDisplayer_MR *_text;
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

