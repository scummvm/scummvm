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
struct Button;

class KyraEngine_v3 : public KyraEngine {
friend class TextDisplayer_v3;
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

	int _curMusicTrack;

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

	// main menu
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

	// interface
	uint8 *_interface;
	uint8 *_interfaceCommandLine;

	void loadInterfaceShapes();
	void loadInterface();

	void showMessage(const char *string, uint8 c0, uint8 c1);
	void updateCommandLine();
	void restoreCommandLine();

	int _commandLineY;
	const char *_shownMessage;
	bool _restoreCommandLine;
	bool _inventoryState;

	// localization
	uint8 *_scoreFile;
	uint8 *_cCodeFile;
	uint8 *_scenesFile;
	uint8 *_itemFile;
	uint8 *_actorFile;
	uint32 _actorFileSize;
	uint8 *_sceneStrings;

	uint8 *getTableEntry(uint8 *buffer, int id);

	// items
	uint8 *_itemBuffer1;
	uint8 *_itemBuffer2;
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

	int findFreeItem();
	
	void initItems();

	int checkItemCollision(int x, int y);

	// -> hand item
	void setItemMouseCursor();

	int _itemInHand;
	int _handItemSet;

	// shapes
	typedef Common::HashMap<int, uint8*> ShapeMap;
	ShapeMap _gameShapes;

	void addShapeToPool(const uint8 *data, int realIndex, int shape);
	uint8 *getShapePtr(int shape) const { return _gameShapes[shape]; }

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
	void runSceneScript8();

	int _sceneMinX, _sceneMaxX;
	int _maskPageMinY, _maskPageMaxY;

	ScriptState _sceneScriptState;
	ScriptData _sceneScriptData;
	WSAMovieV2 *_wsaSlots[10];

	bool _specialSceneScriptState[10];
	ScriptState _sceneSpecialScripts[10];
	uint32 _sceneSpecialScriptsTimer[10];
	int _lastProcessedSceneScript;
	bool _specialSceneScriptRunFlag;

	void updateSpecialSceneScripts();

	int trySceneChange(int *moveTable, int unk1, int unk2);
	int checkSceneChange();

	int8 _sceneDatPalette[45];
	int8 _sceneDatLayerTable[15];

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
		uint32 walkspeed;
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

	// talk object
	struct TalkObject {
		char filename[13];
		int8 unkD;
		int8 unkE;
		int16 x, y;
		uint8 color;
		int8 unk14;
	};

	TalkObject *_talkObjectList;

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
	int o3t_playSoundEffect(ScriptState *script);
	int o3t_getMalcolmShapes(ScriptState *script);

	// special shape code
	char _newShapeFilename[13];
	int _newShapeLastEntry;
	int _newShapeWidth, _newShapeHeight;
	int _newShapeXAdd, _newShapeYAdd;

	int _newShapeAnimFrame;
	int _newShapeDelay;

	// unk
	uint8 *_unkBuffer1040Bytes;
	uint8 *_costPalBuffer;
	uint8 *_screenBuffer;
	uint8 *_gfxBackUpRect;
	uint8 *_paletteOverlay;
	bool _useActorBuffer;
	int _curChapter;

	int _unk3, _unk4, _unk5;

	void loadCostPal();
	void loadShadowShape();
	void loadExtrasShapes();

	// opcodes
	int o3_getMalcolmShapes(ScriptState *script);
	int o3_setCharacterPos(ScriptState *script);
	int o3_defineObject(ScriptState *script);
	int o3_refreshCharacter(ScriptState *script);
	int o3_getCharacterX(ScriptState *script);
	int o3_getCharacterY(ScriptState *script);
	int o3_showSceneFileMessage(ScriptState *script);
	int o3_showBadConscience(ScriptState *script);
	int o3_hideBadConscience(ScriptState *script);
	int o3_objectChat(ScriptState *script);
	int o3_defineItem(ScriptState *script);
	int o3_queryGameFlag(ScriptState *script);
	int o3_resetGameFlag(ScriptState *script);
	int o3_setGameFlag(ScriptState *script);
	int o3_getHandItem(ScriptState *script);
	int o3_hideMouse(ScriptState *script);
	int o3_setMousePos(ScriptState *script);
	int o3_showMouse(ScriptState *script);
	int o3_badConscienceChat(ScriptState *script);
	int o3_delay(ScriptState *script);
	int o3_setSceneFilename(ScriptState *script);
	int o3_playSoundEffect(ScriptState *script);
	int o3_getRand(ScriptState *script);
	int o3_defineRoomEntrance(ScriptState *script);
	int o3_setSpecialSceneScriptRunTime(ScriptState *script);
	int o3_defineSceneAnim(ScriptState *script);
	int o3_updateSceneAnim(ScriptState *script);
	int o3_defineScene(ScriptState *script);
	int o3_setSpecialSceneScriptState(ScriptState *script);
	int o3_clearSpecialSceneScriptState(ScriptState *script);
	int o3_querySpecialSceneScriptState(ScriptState *script);
	int o3_setHiddenItemsEntry(ScriptState *script);
	int o3_getHiddenItemsEntry(ScriptState *script);
	int o3_removeSceneAnimObject(ScriptState *script);
	int o3_setVocHigh(ScriptState *script);
	int o3_getVocHigh(ScriptState *script);
	int o3_dummy(ScriptState *script);

	// misc
	TextDisplayer_v3 *_text;
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

