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

#ifdef ENABLE_LOL

#ifndef KYRA_LOL_H
#define KYRA_LOL_H

#include "kyra/kyra_v1.h"
#include "kyra/script_tim.h"
#include "kyra/script.h"
#include "kyra/sound.h"
#include "kyra/gui_lol.h"
#include "kyra/text_lol.h"

#include "common/list.h"

namespace Kyra {

class Screen_LoL;
class WSAMovie_v2;
struct Button;

struct LoLCharacter {
	uint16 flags;
	char name[11];
	uint8 raceClassSex;
	int16 id;
	uint8 curFaceFrame;
	uint8 nextFaceFrame;
	uint8 field_12;
	const uint16 *defaultModifiers;
	uint16 itemsMight[8];
	uint16 field_27[2];
	uint8 field_2B;
	uint16 field_2C;
	uint16 field_2E;
	uint16 field_30;
	uint16 field_32;
	uint16 field_34;
	uint8 field_36;
	uint16 itemsProtection;
	uint16 hitPointsCur;
	uint16 hitPointsMax;
	uint16 magicPointsCur;
	uint16 magicPointsMax;
	uint8 field_41;
	uint16 damageSuffered;
	uint16 weaponHit;
	uint16 totalMightModifier;
	uint16 totalProtectionModifier;
	uint16 might;
	uint16 protection;
	int16 nextAnimUpdateCountdown;
	uint16 items[11];
	uint8 skillLevels[3];
	uint8 skillModifiers[3];
	int32 experiencePts[3];
	uint8 arrayUnk2[5];
	uint8 arrayUnk1[5];
};

struct SpellProperty {
	uint16 field_0;
	uint16 unkArr[4];
	uint16 field_A;
	uint16 field_C;
	uint16 field_E;
	uint16 spellNameCode;
	uint16 mpRequired[4];
	uint16 field_1A;
};

struct LevelBlockProperty {
	uint8 walls[4];
	uint16 itemMonsterIndex;
	uint16 field_6;
	uint8 direction;
	uint8 flags;
};

struct MonsterProperty {
	uint8 shapeIndex;
	uint8 maxWidth;
	uint16 field2[2];
	uint16 protection;
	uint16 unk[6];
	uint16 *pos;
	uint16 unk2[8];
	uint16 unk3[8];
	uint16 itemProtection;
	uint16 might;
	uint8 b;
	uint16 flags;
	uint16 unk5;
	uint16 unk6[5];
	uint8 unk7[4];
	uint8 sounds[3];
};

struct MonsterInPlay {
	uint16 next;
	uint16 unk2;
	uint8 unk4;
	uint16 blockPropertyIndex;
	uint16 x;
	uint16 y;
	int8 shiftStep;
	uint16 destX;
	uint16 destY;
	uint8 destDirection;
	uint8 anon8;
	uint8 anonh;
	uint8 anon9;

	uint8 mode;
	uint8 field_15;
	uint8 id;
	uint8 direction;
	uint8 facing;
	uint16 flags;
	uint8 field_1B;
	uint8 field_1C;
	int16 might;
	uint8 field_1F;
	uint8 type;
	MonsterProperty *properties;
	uint8 field_25;
	uint8 field_26;
	uint8 field_27;
	uint16 assignedItems;
	uint8 field_2A[4];
};

struct ItemInPlay {
	uint16 next;
	uint16 unk2;
	uint8 unk4;
	uint16 blockPropertyIndex;
	uint16 x;
	uint16 y;
	int8 level;
	uint16 itemPropertyIndex;
	uint16 shpCurFrame_flg;
	uint8 destDirection;
	uint8 anon8;
	uint8 anonh;
	uint8 anon9;
};

struct ItemProperty {
	uint16 nameStringId;
	uint8 shpIndex;
	uint16 flags;
	uint16 type;
	uint8 itemScriptFunc;
	int8 might;
	uint8 skill;
	uint8 protection;
	uint16 unkB;
	uint8 unkD;
};

struct LevelShapeProperty {
	uint16 shapeIndex[10];
	uint8 scaleFlag[10];
	uint16 shapeX[10];
	uint16 shapeY[10];
	int8 next;
	uint8 flags;
};

struct CompassDef {
	uint8 shapeIndex;
	int8 x;
	int8 y;
	uint8 flags;
};

struct ButtonDef {
	uint16 buttonflags;
	uint16 keyCode;
	uint16 keyCode2;
	int16 x;
	int16 y;
	uint16 w;
	uint16 h;
	uint16 index;
	uint16 screenDim;
};

class LoLEngine : public KyraEngine_v1 {
friend class GUI_LoL;
friend class TextDisplayer_LoL;
friend class TIMInterpreter_LoL;
public:
	LoLEngine(OSystem *system, const GameFlags &flags);
	~LoLEngine();

	Screen *screen();
	GUI *gui() const;
private:
	Screen_LoL *_screen;
	GUI_LoL *_gui;

	TIMInterpreter *_tim;

	Common::Error init();
	Common::Error go();

	// initialization
	void initStaticResource();
	void preInit();

	void loadItemIconShapes();
	int mainMenu();

	void startup();
	void startupNew();

	// main loop
	void runLoop();
	void update();
	void updateEnvironmentalSfx(int soundId);

	// mouse
	void setMouseCursorToIcon(int icon);
	void setMouseCursorToItemInHand();
	uint8 *getItemIconShapePtr(int index);
	bool posWithinRect(int mouseX, int mouseY, int x1, int y1, int x2, int y2);

	int _floatingMouseArrowControl;

	// intro
	void setupPrologueData(bool load);

	void showIntro();

	struct CharacterPrev {
		const char *name;
		int x, y;
		int attrib[3];
	};

	static const CharacterPrev _charPreviews[];

	WSAMovie_v2 *_chargenWSA;
	static const uint8 _chargenFrameTable[];
	int chooseCharacter();

	void kingSelectionIntro();
	void kingSelectionReminder();
	void kingSelectionOutro();
	void processCharacterSelection();
	void updateSelectionAnims();
	int selectionCharInfo(int character);
	void selectionCharInfoIntro(char *file);

	int getCharSelection();
	int selectionCharAccept();

	int _charSelection;
	int _charSelectionInfoResult;

	uint32 _selectionAnimTimers[4];
	uint8 _selectionAnimFrames[4];
	static const uint8 _selectionAnimIndexTable[];

	static const uint16 _selectionPosTable[];

	static const uint8 _selectionChar1IdxTable[];
	static const uint8 _selectionChar2IdxTable[];
	static const uint8 _selectionChar3IdxTable[];
	static const uint8 _selectionChar4IdxTable[];

	static const uint8 _reminderChar1IdxTable[];
	static const uint8 _reminderChar2IdxTable[];
	static const uint8 _reminderChar3IdxTable[];
	static const uint8 _reminderChar4IdxTable[];

	static const uint8 _charInfoFrameTable[];

	// timers
	void setupTimers();
	void enableTimer(int id);
	void enableSysTimer(int sysTimer);
	void disableSysTimer(int sysTimer);

	void timerProcessDoors(int timerNum);
	void timerProcessMonsters(int timerNum);
	void timerSub3(int timerNum);
	void timerSub4(int timerNum);
	void timerSub5(int timerNum);
	void timerSub6(int timerNum);
	void timerUpdatePortraitAnimations(int skipUpdate);
	void timerUpdateLampState(int timerNum);
	void timerFadeMessageText(int timerNum);

	static const uint8 _clock2Timers[];
	static const uint8 _numClock2Timers;

	// sound
	void loadTalkFile(int index);
	void snd_playVoiceFile(int track) {}
	bool snd_playCharacterSpeech(int id, int8 speaker, int);
	int snd_characterSpeaking();
	void snd_stopSpeech(bool setFlag);
	void snd_playSoundEffect(int track, int volume);
	void snd_processEnvironmentalSoundEffect(int soundId, int block);
	void snd_loadSoundFile(int track);
	int snd_playTrack(int track);
	int snd_stopMusic();

	int _lastSpeechId;
	int _lastSpeaker;
	char _activeVoiceFile[13];
	int _lastSfxTrack;
	int _lastMusicTrack;
	int _curMusicFileIndex;
	char _curMusicFileExt;
	int _environmentSfx;
	int _environmentSfxVol;
	int _environmentSfxDistThreshold;

	int _curTlkFile;
	int _speechFlag;

	char **_ingameSoundList;
	int _ingameSoundListSize;

	const uint8 *_musicTrackMap;
	int _musicTrackMapSize;
	const uint16 *_ingameSoundIndex;
	int _ingameSoundIndexSize;
	const uint8 *_ingameGMSoundIndex;
	int _ingameGMSoundIndexSize;
	const uint8 *_ingameMT32SoundIndex;
	int _ingameMT32SoundIndexSize;
	/*const uint8 *_ingameADLSoundIndex;
	int _ingameADLSoundIndexSize;*/

	// gui
	void gui_drawPlayField();
	void gui_drawScene(int pageNum);
	void gui_drawAllCharPortraitsWithStats();
	void gui_drawCharPortraitWithStats(int charNum);
	void gui_drawBox(int x, int y, int w, int h, int frameColor1, int frameColor2, int fillColor);
	void gui_drawCharFaceShape(int charNum, int x, int y, int pageNum);
	void gui_highlightPortraitFrame(int charNum);
	void gui_drawLiveMagicBar(int x, int y, int curPoints, int unk, int maxPoints, int w, int h, int col1, int col2, int flag);
	void gui_drawMoneyBox(int pageNum);
	void gui_drawInventory();
	void gui_drawInventoryItem(int index);
	void gui_drawCompass();
	void gui_drawScroll();
	void gui_highlightSelectedSpell(int unk);
	void gui_displayCharInventory(int charNum);
	void gui_printCharInventoryStats(int charNum);
	void gui_printCharacterStats(int index, int redraw, int value);
	void gui_changeCharacterStats(int charNum);
	void gui_drawCharInventoryItem(int itemIndex);
	void gui_drawBarGraph(int x, int y, int w, int h, int32 curVal, int32 maxVal, int col1, int col2);

	int gui_enableControls();
	int gui_disableControls(int controlMode);
	void gui_toggleButtonDisplayMode(int shapeIndex, int mode);
	void gui_toggleFightButtons(bool disable);
	void gui_prepareForSequence(int x, int y, int w, int h, int buttonFlags);

	bool _weaponsDisabled;
	int _lastButtonShape;
	uint32 _buttonPressTimer;
	int _selectedCharacter;
	int _compassDirection;
	int _compassUnk;
	int _compassDirectionIndex;
	int _charInventoryUnk;

	const CompassDef *_compassDefs;
	int _compassDefsSize;

	void gui_updateInput();
	void gui_triggerEvent(int eventType);
	void gui_enableDefaultPlayfieldButtons();
	void gui_enableSequenceButtons(int x, int y, int w, int h, int enableFlags);
	void gui_enableCharInventoryButtons(int charNum);

	void gui_resetButtonList();
	void gui_initButtonsFromList(const int16 *list);
	void gui_initCharacterControlButtons(int index, int xOffs);
	void gui_initCharInventorySpecialButtons(int charNum);
	void gui_initMagicScrollButtons();
	void gui_initMagicSubmenu(int charNum);
	void gui_initButton(int index, int x = -1, int y = -1, int val = -1);
	void gui_notifyButtonListChanged() { if (_gui) _gui->_buttonListChanged = true; }
	void assignButtonCallback(Button *button, int index);

	Button *_activeButtons;
	ButtonDef _sceneWindowButton;
	bool _preserveEvents;

	int clickedUpArrow(Button *button);
	int clickedDownArrow(Button *button);
	int clickedLeftArrow(Button *button);
	int clickedRightArrow(Button *button);
	int clickedTurnLeftArrow(Button *button);
	int clickedTurnRightArrow(Button *button);
	int clickedAttackButton(Button *button);
	int clickedMagicButton(Button *button);
	int clickedMagicSubmenu(Button *button);
	int clickedScreen(Button *button);
	int clickedPortraitLeft(Button *button);
	int clickedLiveMagicBarsLeft(Button *button);
	int clickedPortraitEtcRight(Button *button);
	int clickedCharInventorySlot(Button *button);
	int clickedExitCharInventory(Button *button);
	int clickedUnk16(Button *button);
	int clickedScenePickupItem(Button *button);
	int clickedInventorySlot(Button *button);
	int clickedInventoryScroll(Button *button);
	int clickedWall(Button *button);
	int clickedSequenceWindow(Button *button);
	int clickedScroll(Button *button);
	int clickedUnk23(Button *button);
	int clickedUnk24(Button *button);
	int clickedSceneDropItem(Button *button);
	int clickedOptions(Button *button);
	int clickedRestParty(Button *button);
	int clickedMoneyBox(Button *button);
	int clickedCompass(Button *button);
	int clickedAutomap(Button *button);
	int clickedLamp(Button *button);
	int clickedUnk32(Button *button);

	const ButtonDef *_buttonData;
	int _buttonDataSize;
	const int16 *_buttonList1;
	int _buttonList1Size;
	const int16 *_buttonList2;
	int _buttonList2Size;
	const int16 *_buttonList3;
	int _buttonList3Size;
	const int16 *_buttonList4;
	int _buttonList4Size;
	const int16 *_buttonList5;
	int _buttonList5Size;
	const int16 *_buttonList6;
	int _buttonList6Size;
	const int16 *_buttonList7;
	int _buttonList7Size;
	const int16 *_buttonList8;
	int _buttonList8Size;

	// text
	bool characterSays(int track, int charId, bool redraw);
	int playCharacterScriptChat(int charId, int y, int unk1, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);

	TextDisplayer_LoL *_txt;

	// emc scripts
	void runInitScript(const char *filename, int optionalFunc);
	void runInfScript(const char *filename);
	void runLevelScript(int block, int sub);
	void runLevelScriptCustom(int block, int sub, int charNum, int item, int reg3, int reg4);
	bool checkSceneUpdateNeed(int func);

	EMCData _scriptData;
	bool _suspendScript;
	uint16 _scriptDirection;
	uint16 _currentDirection;
	uint16 _currentBlock;
	bool _sceneUpdateRequired;
	int16 _currentBlockPropertyIndex[18];
	uint16 _gameFlags[16];
	uint16 _unkEMC46[16];

	// emc opcode
	int olol_drawScene(EMCState *script);
	int olol_delay(EMCState *script);
	int olol_setGameFlag(EMCState *script);
	int olol_testGameFlag(EMCState *script);
	int olol_loadLevelGraphics(EMCState *script);
	int olol_loadCmzFile(EMCState *script);
	int olol_loadMonsterShapes(EMCState *script);
	int olol_deleteHandItem(EMCState *script);
	int olol_allocItemPropertiesBuffer(EMCState *script);
	int olol_setItemProperty(EMCState *script);
	int olol_makeItem(EMCState *script);
	int olol_getItemPara(EMCState *script);
	int olol_getCharacterStat(EMCState *script);
	int olol_setCharacterStat(EMCState *script);
	int olol_loadLevelShapes(EMCState *script);
	int olol_closeLevelShapeFile(EMCState *script);
	int olol_loadDoorShapes(EMCState *script);
	int olol_initAnimStruct(EMCState *script);
	int olol_freeAnimStruct(EMCState *script);
	int olol_getDirection(EMCState *script);
	int olol_setMusicTrack(EMCState *script);
	int olol_clearDialogueField(EMCState *script);
	int olol_getUnkArrayVal(EMCState *script);
	int olol_setUnkArrayVal(EMCState *script);
	int olol_getGlobalVar(EMCState *script);
	int olol_setGlobalVar(EMCState *script);
	int olol_triggerDoorSwitch(EMCState *script);
	int olol_mapShapeToBlock(EMCState *script);
	int olol_resetBlockShapeAssignment(EMCState *script);
	int olol_initMonster(EMCState *script);
	int olol_loadMonsterProperties(EMCState *script);
	int olol_moveMonster(EMCState *script);
	int olol_setScriptTimer(EMCState *script);
	int olol_loadTimScript(EMCState *script);
	int olol_runTimScript(EMCState *script);
	int olol_releaseTimScript(EMCState *script);
	int olol_initDialogueSequence(EMCState *script);
	int olol_restoreSceneAfterDialogueSequence(EMCState *script);
	int olol_getItemInHand(EMCState *script);
	int olol_giveItemToMonster(EMCState *script);
	int olol_loadLangFile(EMCState *script);
	int olol_playSoundEffect(EMCState *script);
	int olol_stopTimScript(EMCState *script);
	int olol_getWallFlags(EMCState *script);
	int olol_playCharacterScriptChat(EMCState *script);
	int olol_loadSoundFile(EMCState *script);
	int olol_stopCharacterSpeech(EMCState *script);
	int olol_setPaletteBrightness(EMCState *script);
	int olol_printMessage(EMCState *script);
	int olol_playDialogueTalkText(EMCState *script);
	int olol_checkForMonsterMode1(EMCState *script);
	int olol_setNextFunc(EMCState *script);
	int olol_setDoorState(EMCState *script);
	int olol_assignCustomSfx(EMCState *script);
	int olol_resetPortraitsArea(EMCState *script);
	int olol_enableSysTimer(EMCState *script);

	// tim scripts
	TIM *_activeTim[10];

	// tim opcode
	void setupOpcodeTable();

	Common::Array<const TIMOpcode*> _timIntroOpcodes;
	int tlol_setupPaletteFade(const TIM *tim, const uint16 *param);
	int tlol_loadPalette(const TIM *tim, const uint16 *param);
	int tlol_setupPaletteFadeEx(const TIM *tim, const uint16 *param);
	int tlol_processWsaFrame(const TIM *tim, const uint16 *param);
	int tlol_displayText(const TIM *tim, const uint16 *param);

	int tlol_initDialogueSequence(const TIM *tim, const uint16 *param);
	int tlol_restoreSceneAfterDialogueSequence(const TIM *tim, const uint16 *param);
	int tlol_giveItem(const TIM *tim, const uint16 *param);
	int tlol_setPartyPosition(const TIM *tim, const uint16 *param);
	int tlol_fadeClearWindow(const TIM *tim, const uint16 *param);
	int tlol_update(const TIM *tim, const uint16 *param);
	int tlol_loadSoundFile(const TIM *tim, const uint16 *param);
	int tlol_playMusicTrack(const TIM *tim, const uint16 *param);	
	int tlol_playDialogueTalkText(const TIM *tim, const uint16 *param);
	int tlol_playSoundEffect(const TIM *tim, const uint16 *param);

	Common::Array<const TIMOpcode*> _timIngameOpcodes;

	// translation
	int _lang;

	uint8 *_landsFile;
	uint8 *_levelLangFile;

	int _lastUsedStringBuffer;
	char _stringBuffer[5][512];	// TODO: The original used a size of 512, it looks a bit large.
								// Maybe we can someday reduce the size.
	char *getLangString(uint16 id);
	uint8 *getTableEntry(uint8 *buffer, uint16 id);

	static const char * const _languageExt[];

	// graphics
	void setupScreenDims();
	void initDialogueSequence(int controlMode);
	void restoreSceneAfterDialogueSequence(int redraw);
	void resetPortraitsArea();
	void toggleSelectedCharacterFrame(bool mode);
	void fadeText();
	void setPaletteBrightness(uint8 *palette, int brightness, int modifier);
	void generateBrightnessPalette(uint8 *src, uint8 *dst, int brightness, int modifier);
	void updateWsaAnimations();

	uint8 **_itemIconShapes;
	int _numItemIconShapes;
	uint8 **_itemShapes;
	int _numItemShapes;
	uint8 **_gameShapes;
	int _numGameShapes;
	uint8 **_thrownShapes;
	int _numThrownShapes;
	uint8 **_iceShapes;
	int _numIceShapes;
	uint8 **_fireballShapes;
	int _numFireballShapes;

	const int8 *_gameShapeMap;
	int _gameShapeMapSize;

	uint8 *_characterFaceShapes[40][3];

	// characters
	bool addCharacter(int id);
	void initCharacter(int charNum, int firstFaceFrame, int unk2, int redraw);
	void initCharacterUnkSub(int charNum, int unk1, int unk2, int unk3);
	int countActiveCharacters();
	void loadCharFaceShapes(int charNum, int id);
	void calcCharPortraitXpos();

	void updatePortraitSpeechAnim();
	void updatePortraits();
	void initTextFading(int textType, int clearField);
	void setCharFaceFrame(int charNum, int frameNum);
	void faceFrameRefresh(int charNum);

	void recalcCharacterStats(int charNum);
	int calculateCharacterStats(int charNum, int index);
	int calculateProtection(int index);

	LoLCharacter *_characters;
	uint16 _activeCharsXpos[3];
	int _updateFlags;
	int _updateCharNum;
	int _updatePortraitSpeechAnimDuration;
	int _portraitSpeechAnimMode;
	int _updateCharV3;
	int _textColourFlag;
	bool _fadeText;
	int _hideInventory;
	uint32 _palUpdateTimer;
	uint32 _updatePortraitNext;

	int _loadLevelFlag;
	int _levelFlagUnk;
	int _unkCharNum;
	int _charStatsTemp[5];

	const LoLCharacter *_charDefaults;
	int _charDefaultsSize;

	const uint16 *_charDefsMan;
	int _charDefsManSize;
	const uint16 *_charDefsWoman;
	int _charDefsWomanSize;
	const uint16 *_charDefsKieran;
	int _charDefsKieranSize;
	const uint16 *_charDefsAkshel;
	int _charDefsAkshelSize;
	const int32 *_expRequirements;
	int _expRequirementsSize;

	// lamp
	void resetLampStatus();
	void setLampMode(bool lampOn);
	void updateLampStatus();

	int _lampOilStatus;
	int _brightness;
	int _lampStatusUnk;
	uint32 _lampStatusTimer;
	bool _lampStatusSuspended;

	// level
	void loadLevel(int index);
	void addLevelItems();
	void loadLevelWLL(int index, bool mapShapes);
	void moveItemToBlock(uint16 *cmzItemIndex, uint16 item);
	int assignLevelShapes(int index);
	uint8 *getLevelShapes(int index);
	void loadLevelCmzFile(int index);
	void loadCMZ_Sub(int index1, int index2);
	void loadCmzFile(const char *file);
	void loadLevelShpDat(const char *shpFile, const char *datFile, bool flag);
	void loadLevelGraphics(const char *file, int specialColor, int weight, int vcnLen, int vmpLen, const char *palFile);

	void resetItems(int flag);
	void resetLvlBuffer();
	void resetBlockProperties();
	bool testWallFlag(int block, int direction, int flag);
	bool testWallInvisibility(int block, int direction);

	void drawScene(int pageNum);

	void generateBlockDrawingBuffer(int block, int direction);
	void generateBlockDrawingBufferF0(int16 wllOffset, uint8 wllIndex, uint8 wllVmpIndex, int16 vmpOffset, uint8 len, uint8 numEntries);
	void generateBlockDrawingBufferF1(int16 wllOffset, uint8 wllIndex, uint8 wllVmpIndex, int16 vmpOffset, uint8 len, uint8 numEntries);
	bool hasWall(int index);
	void assignBlockCaps(int block, int direction);

	void drawVcnBlocks(uint8 *vcnBlocks, uint16 *blockDrawingBuffer, uint8 *vcnShift, int pageNum);
	void drawSceneShapes();
	void setLevelShapesDim(int index, int16 &x1, int16 &x2, int dim);
	void scaleLevelShapesDim(int index, int16 &y1, int16 &y2, int dim);
	void drawLevelModifyScreenDim(int dim, int16 x1, int16 y1, int16 x2, int16 y2);
	void drawDecorations(int index);
	void drawIceShapes(int index, int iceShapeIndex);
	void drawScriptShapes(int pageNum);
	void updateSceneWindow();

	void setSequenceGui(int x, int y, int w, int h, int enableFlags);
	void restoreDefaultGui();

	void updateCompass();

	void moveParty(uint16 direction, int unk1, int unk2, int buttonShape);
	bool checkBlockPassability(uint16 block, uint16 direction);
	void notifyBlockNotPassable(int scrollFlag);

	uint16 calcNewBlockPosition(uint16 curBlock, uint16 direction);
	uint16 calcBlockIndex(uint16 x, uint16 y);
	void calcCoordinates(uint16 & x, uint16 & y, int block, uint16 xOffs, uint16 yOffs);

	int clickedWallShape(uint16 block, uint16 direction);
	int clicked2(uint16 block, uint16 direction);
	int clicked3(uint16 block, uint16 direction);
	int clickedWallOnlyScript(uint16 block);
	int clickedDoorSwitch(uint16 block, uint16 direction);
	int clicked6(uint16 block, uint16 direction);

	bool clickedShape(int shapeIndex);
	void processDoorSwitch(uint16 block, int unk);
	void openCloseDoor(uint16 block, int openClose);

	void movePartySmoothScrollBlocked(int speed);
	void movePartySmoothScrollUp(int speed);
	void movePartySmoothScrollDown(int speed);
	void movePartySmoothScrollLeft(int speed);
	void movePartySmoothScrollRight(int speed);
	void movePartySmoothScrollTurnLeft(int speed);
	void movePartySmoothScrollTurnRight(int speed);

	int smoothScrollDrawSpecialShape(int pageNum);
	void setLF2(int block);

	struct OpenDoorState {
		uint16 block;
		int8 field_2;
		int8 state;
	};

	OpenDoorState _openDoorState[3];
	int _emcDoorState;

	uint8 *_scrollSceneBuffer;
	uint32 _smoothScrollTimer;
	int _smoothScrollModeNormal;

	const uint8 *_scrollXTop;
	int _scrollXTopSize;
	const uint8 *_scrollYTop;
	int _scrollYTopSize;
	const uint8 *_scrollXBottom;
	int _scrollXBottomSize;
	const uint8 *_scrollYBottom;
	int _scrollYBottomSize;

	int _unkFlag;
	int _nextScriptFunc;
	uint8 _currentLevel;
	int _sceneDefaultUpdate;
	int _lvlBlockIndex;
	int _lvlShapeIndex;
	bool _unkDrawLevelBool;
	uint8 *_vcnBlocks;
	uint8 *_vcnShift;
	uint8 *_vcnExpTable;
	uint16 *_vmpPtr;
	uint16 *_blockDrawingBuffer;
	uint8 *_sceneWindowBuffer;
	LevelShapeProperty *_levelShapeProperties;
	uint8 **_levelShapes;
	uint8 *_scriptAssignedLevelShape;

	char _lastSuppFile[12];
	char _lastOverridePalFile[12];
	char *_lastOverridePalFilePtr;
	int _lastSpecialColor;
	int _lastSpecialColorWeight;

	int _sceneDrawVar1;
	int _sceneDrawVar2;
	int _sceneDrawVar3;
	int _wllProcessFlag;

	uint8 *_trueLightTable2;
	uint8 *_trueLightTable1;

	int _loadSuppFilesFlag;

	uint8 *_wllVmpMap;
	int8 *_wllShapeMap;
	uint8 *_wllBuffer3;
	uint8 *_wllBuffer4;
	uint8 *_wllWallFlags;

	int16 *_lvlShapeTop;
	int16 *_lvlShapeBottom;
	int16 *_lvlShapeLeftRight;

	LevelBlockProperty *_levelBlockProperties;
	LevelBlockProperty *_curBlockCaps[18];

	uint16 _partyPosX;
	uint16 _partyPosY;

	Common::SeekableReadStream *_lvlShpFileHandle;
	uint16 _lvlShpNum;
	uint16 _levelFileDataSize;
	LevelShapeProperty *_levelFileData;

	uint8 *_doorShapes[2];
	int16 _shpDmX;
	int16 _shpDmY;
	int16 _dmScaleW;
	int16 _dmScaleH;

	int _lastMouseRegion;
	//int _preSeq_X1, _preSeq_Y1,	_preSeq_X2, _preSeq_Y2;
	uint8 _unkGameFlag;

	uint8 *_tempBuffer5120;
	uint8 *_tmpData136;

	const char *const * _levelDatList;
	int _levelDatListSize;
	const char *const * _levelShpList;
	int _levelShpListSize;

	const int8 *_dscUnk1;
	int _dscUnk1Size;
	const int8 *_dscShapeIndex;
	int _dscShapeIndexSize;
	const uint8 *_dscOvlMap;
	int _dscOvlMapSize;
	const uint16 *_dscShapeScaleW;
	int _dscShapeScaleWSize;
	const uint16 *_dscShapeScaleH;
	int _dscShapeScaleHSize;
	const int16 *_dscShapeX;
	int _dscShapeXSize;
	const int8 *_dscShapeY;
	int _dscShapeYSize;
	const uint8 *_dscTileIndex;
	int _dscTileIndexSize;
	const uint8 *_dscUnk2;
	int _dscUnk2Size;
	const uint8 *_dscDoorShpIndex;
	int _dscDoorShpIndexSize;
	const int8 *_dscDim1;
	int _dscDim1Size;
	const int8 *_dscDim2;
	int _dscDim2Size;
	const uint8 *_dscBlockMap;
	int _dscBlockMapSize;
	const uint8 *_dscDimMap;
	int _dscDimMapSize;
	const uint16 *_dscDoorMonsterScaleTable;
	int _dscDoorMonsterScaleTableSize;
	const uint16 *_dscDoor4;
	int _dscDoor4Size;
	const uint8 *_dscShapeOvlIndex;
	int _dscShapeOvlIndexSize;
	const int8 *_dscBlockIndex;
	int _dscBlockIndexSize;
	const uint8 *_dscDoor1;
	int _dscDoor1Size;
	const int16 *_dscDoorMonsterX;
	int _dscDoorMonsterXSize;
	const int16 *_dscDoorMonsterY;
	int _dscDoorMonsterYSize;

	int _sceneDrawPage1;
	int _sceneDrawPage2;

	// items
	void giveCredits(int credits, int redraw);
	int makeItem(int itemIndex, int curFrame, int flags);
	bool addItemToInventory(int itemIndex);
	bool testUnkItemFlags(int itemIndex);
	void deleteItem(int itemIndex);
	ItemInPlay *findItem(uint16 index);
	void runItemScript(int charNum, int item, int reg0, int reg3, int reg4);
	void setHandItem(uint16 itemIndex);
	void clickSceneSub1();
	int checkMonsterSpace(int itemX, int itemY, int partyX, int partyY);
	int checkSceneForItems(LevelBlockProperty *block, int pos);
	void foundItemSub(int item, int block);

	uint8 _moneyColumnHeight[5];
	uint16 _credits;

	ItemInPlay *_itemsInPlay;
	ItemProperty *_itemProperties;

	int _itemInHand;
	uint16 _inventory[48];
	int _inventoryCurItem;
	int _hideControls;
	int _lastCharInventory;

	const uint8 *_charInvIndex;
	int _charInvIndexSize;
	const int8 *_charInvDefs;
	int _charInvDefsSize;

	EMCData _itemScript;

	const uint16 *_inventorySlotDesc;
	int _inventorySlotDescSize;

	// monsters
	void loadMonsterShapes(const char *file, int monsterIndex, int b);
	void releaseMonsterShapes(int monsterIndex);
	int placeMonstersUnk(int block);
	void setMonsterMode(MonsterInPlay *monster, int a);
	void placeMonster(MonsterInPlay *monster, uint16 x, uint16 y);
	int calcMonsterDirection(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void setMonsterDirection(MonsterInPlay *monster, int dir);
	void cmzS3(MonsterInPlay *monster);
	void removeItemOrMonsterFromBlock(uint16 *blockItemIndex, int id);
	void assignItemOrMonsterToBlock(uint16 *blockItemIndex, int id);
	void giveItemToMonster(MonsterInPlay *monster, uint16 item);
	int checkBlockBeforeMonsterPlacement(int x, int y, int monsterWidth, int testFlag, int wallFlag);
	int calcMonsterSkillLevel(int id, int a);
	int checkBlockForWallsAndSufficientSpace(int block, int x, int y, int monsterWidth, int testFlag, int wallFlag);
	bool checkBlockOccupiedByParty(int x, int y, int testFlag);
	const uint16 *getCharacterOrMonsterStats(int id);
	void drawMonstersAndItems(int block);
	void drawMonster(uint16 id);
	int getMonsterCurFrame(MonsterInPlay *m, uint16 dirFlags);
	void recalcItemMonsterPositions(uint16 direction, uint16 itemIndex, LevelBlockProperty *l, bool flag);
	int calcItemMonsterPosition(ItemInPlay *i, uint16 direction);
	void recalcSpritePosition(uint16 partyX, uint16 partyY, int &itemX, int &itemY, uint16 direction);
	void drawDoor(uint8 *shape, uint8 *table, int index, int unk2, int w, int h, int flags);
	void drawDoorOrMonsterShape(uint8 *shape, uint8 *table, int x, int y, int flags, const uint8 *ovl);
	uint8 *drawItemOrMonster(uint8 *shape, uint8 *ovl, int x, int y, int w, int h, int flags, int tblValue, bool flip);
	int calcDrawingLayerParameters(int srcX, int srcY, int16 &x2, int16 &y2, int16 &w, int16 &h, uint8 *shape, int flip);

	void updateMonster(MonsterInPlay *monster);
	void moveMonster(MonsterInPlay *monster);
	void walkMonster(MonsterInPlay *monster);
	int walkMonsterCalcNextStep(MonsterInPlay *monster);
	int getMonsterDistance(uint16 block1, uint16 block2);
	int walkMonster_s3(uint16 monsterBlock, int direction, int distance, uint16 curBlock);
	int walkMonsterCheckDest(int x, int y, MonsterInPlay *monster, int unk);
	void walkMonsterGetNextStepCoords(int16 monsterX, int16 monsterY, int &newX, int &newY, uint16 unk);

	MonsterInPlay *_monsters;
	MonsterProperty *_monsterProperties;
	uint8 **_monsterShapes;
	uint8 **_monsterPalettes;
	uint8 **_monsterShapesEx;
	uint8 _monsterUnk[3];
	uint16 _monsterCurBlock;
	int _monsterLastWalkDirection;
	int _monsterCountUnk;
	int _monsterShiftAlt;

	const uint16 *_monsterModifiers;
	int _monsterModifiersSize;
	const int8 *_monsterShiftOffs;
	int _monsterShiftOffsSize;
	const uint8 *_monsterDirFlags;
	int _monsterDirFlagsSize;
	const int8 *_monsterScaleX;
	int _monsterScaleXSize;
	const int8 *_monsterScaleY;
	int _monsterScaleYSize;
	const uint16 *_monsterScaleWH;
	int _monsterScaleWHSize;

	// misc
	void delay(uint32 millis, bool cUpdate = false, bool isMainLoop = false);

	uint8 _unkBt1;
	uint8 _unkBt2;
	uint16 _unkWordArraySize8[8];

	uint8 *_pageBuffer1;
	uint8 *_pageBuffer2;

	// spells
	bool notEnoughMagic(int charNum, int spellNum, int spellLevel);

	int8 _availableSpells[7];
	int _selectedSpell;
	const SpellProperty *_spellProperties;
	int _spellPropertiesSize;
	int _subMenuIndex;

	// unneeded
	void setWalkspeed(uint8) {}
	void removeHandItem() {}
	bool lineIsPassable(int, int) { return false; }

	// save
	Common::Error loadGameState(int slot) { return Common::kNoError; }
	Common::Error saveGameState(int slot, const char *saveName, const Graphics::Surface *thumbnail) { return Common::kNoError; }
};

} // end of namespace Kyra

#endif

#endif // ENABLE_LOL

