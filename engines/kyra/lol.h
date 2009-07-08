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
	uint8 defaultFaceFrame;
	uint8 screamSfx;
	const uint16 *defaultModifiers;
	uint16 itemsMight[8];
	uint16 protectionAgainstItems[8];
	uint16 itemProtection;
	int16 hitPointsCur;
	uint16 hitPointsMax;
	int16 magicPointsCur;
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
	int8 skillModifiers[3];
	int32 experiencePts[3];
	uint8 characterUpdateEvents[5];
	uint8 characterUpdateDelay[5];
};

struct SpellProperty {
	uint16 spellNameCode;
	uint16 mpRequired[4];
	uint16 field_a;
	uint16 field_c;
	uint16 hpRequired[4];
	uint16 field_16;
	uint16 field_18;
	uint16 flags;
};

struct LevelBlockProperty {
	uint8 walls[4];
	uint16 assignedObjects;
	uint16 drawObjects;
	uint8 direction;
	uint8 flags;
};

struct MonsterProperty {
	uint8 shapeIndex;
	uint8 maxWidth;
	uint16 fightingStats[9];
	uint16 itemsMight[8];
	uint16 protectionAgainstItems[8];
	uint16 itemProtection;
	uint16 hitPoints;
	uint8 speedTotalWaitTicks;
	uint8 skillLevel;
	uint16 flags;
	uint16 unk5;
	uint16 numDistAttacks;
	uint16 numDistWeapons;
	uint16 distWeapons[3];
	uint8 attackSkillChance;
	uint8 attackSkillType;
	uint8 defenseSkillChance;
	uint8 defenseSkillType;
	uint8 sounds[3];
};

struct MonsterInPlay {
	uint16 nextAssignedObject;
	uint16 nextDrawObject;
	uint8 flyingHeight;
	uint16 block;
	uint16 x;
	uint16 y;
	int8 shiftStep;
	uint16 destX;
	uint16 destY;
	uint8 destDirection;
	int8 hitOffsX;
	int8 hitOffsY;
	uint8 currentSubFrame;

	uint8 mode;
	int8 fightCurTick;
	uint8 id;
	uint8 direction;
	uint8 facing;
	uint16 flags;
	uint16 damageReceived;
	//uint8 field_1C;
	int16 hitPoints;
	uint8 speedTick;
	uint8 type;
	MonsterProperty *properties;
	uint8 numDistAttacks;
	uint8 curDistWeapon;
	int8 distAttackTick;
	uint16 assignedItems;
	uint8 field_2A[4];
};

struct ItemInPlay {
	uint16 nextAssignedObject;
	uint16 nextDrawObject;
	uint8 flyingHeight;
	uint16 block;
	uint16 x;
	uint16 y;
	int8 level;
	uint16 itemPropertyIndex;
	uint16 shpCurFrame_flg;
	uint8 destDirection;
	int8 hitOffsX;
	int8 hitOffsY;
	uint8 currentSubFrame;
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
	int16 shapeX[10];
	int16 shapeY[10];
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

struct OpenDoorState {
	uint16 block;
	int8 wall;
	int8 state;
};

struct ActiveSpell {
	uint8 spell;
	const SpellProperty *p;
	uint8 charNum;
	uint8 level;
	uint8 target;
};

struct FlyingObject {
	uint8 enable;
	uint8 objectType;
	uint16 attackerId;
	uint16 item;
	uint16 x;
	uint16 y;
	uint8 flyingHeight;
	uint8 direction;
	uint8 distance;
	int8 field_D;
	uint8 c;
	uint8 flags;
	uint8 wallFlags;
};

struct FlyingObjectShape {
	uint8 shapeFront;
	uint8 shapeBack;
	uint8 shapeLeft;
	uint8 drawFlags;
	uint8 flipFlags;
};

struct LevelTempData {
	uint8 *wallsXorData;
	uint8 *flags;
	MonsterInPlay *monsters;
	FlyingObject *flyingObjects;
	uint8 monsterDifficulty;
};

struct MapLegendData {
	uint8 shapeIndex;
	bool enable;
	int8 x;
	uint16 stringId;
};

struct LightningProperty {
	uint8 lastFrame;
	uint8 frameDiv;
	int16 sfxId;
};

struct FireballState {
	FireballState(int i) {
		active = true;
		destX = 200;
		destY = 60;
		tblIndex = ((i * 50) % 255) + 200;
		progress = 1000;
		step = 10;
		finalize = false;
		finProgress = 0;
	};

	bool active;
	int16 destX;
	int16 destY;
	uint16 tblIndex;
	int32 progress;
	uint8 step;
	bool finalize;
	uint8 finProgress;
};

struct MistOfDoomAnimData {
	uint8 part1First;
	uint8 part1Last;
	uint8 part2First;
	uint8 part2Last;
	uint8 sound;
};

class LoLEngine : public KyraEngine_v1 {
friend class GUI_LoL;
friend class TextDisplayer_LoL;
friend class TIMInterpreter_LoL;
friend class Debugger_LoL;
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

	void registerDefaultSettings();
	void writeSettings();
	void readSettings();

	// options
	int _monsterDifficulty;
	bool _smoothScrollingEnabled;
	bool _floatingCursorsEnabled;

	// main loop
	void runLoop();
	void update();
	void updateEnvironmentalSfx(int soundId);

	// mouse
	void setMouseCursorToIcon(int icon);
	void setMouseCursorToItemInHand();
	uint8 *getItemIconShapePtr(int index);
	bool posWithinRect(int mouseX, int mouseY, int x1, int y1, int x2, int y2);

	void checkFloatingPointerRegions();
	int _floatingCursorControl;
	int _currentFloatingCursor;

	// intro + character selection
	int processPrologue();
	void setupPrologueData(bool load);

	void showIntro();

	struct CharacterPrev {
		const char *name;
		int x, y;
		int attrib[3];
	};

	static const CharacterPrev _charPreviews[];

	WSAMovie_v2 *_chargenWSA;
	static const uint8 _chargenFrameTableTalkie[];
	static const uint8 _chargenFrameTableFloppy[];
	const uint8 *_chargenFrameTable;
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

	void showStarcraftLogo();

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

	// outro
	void showOutro(int character, bool maxDifficulty);
	void setupEpilogueData(bool load);

	void showCredits();
	void processCredits(char *text, int dimState, int page, int delay);
	void loadOutroShapes(int file, uint8 **storage);

	uint8 _outroShapeTable[256];

	// TODO: Consider moving these tables to kyra.dat
	static const char * const _outroShapeFileTable[];
	static const uint8 _outroFrameTable[];

	static const int16 _outroRightMonsterPos[];
	static const int16 _outroLeftMonsterPos[];
	static const int16 _outroRightDoorPos[];
	static const int16 _outroLeftDoorPos[];

	static const int _outroMonsterScaleTableX[];
	static const int _outroMonsterScaleTableY[];

	// timers
	void setupTimers();
	void enableTimer(int id);
	void enableSysTimer(int sysTimer);
	void disableSysTimer(int sysTimer);

	void timerProcessDoors(int timerNum);
	void timerProcessMonsters(int timerNum);
	void timerSpecialCharacterUpdate(int timerNum);
	void timerProcessFlyingObjects(int timerNum);
	void timerRunSceneAnimScript(int timerNum);
	void timerRegeneratePoints(int timerNum);
	void timerUpdatePortraitAnimations(int skipUpdate);
	void timerUpdateLampState(int timerNum);
	void timerFadeMessageText(int timerNum);

	static const uint8 _clock2Timers[];
	static const uint8 _numClock2Timers;
	int _timer3Para;

	// sound
	int convertVolumeToMixer(int value);
	int convertVolumeFromMixer(int value);

	void loadTalkFile(int index);
	void snd_playVoiceFile(int track) {}
	bool snd_playCharacterSpeech(int id, int8 speaker, int);
	int snd_updateCharacterSpeech();
	void snd_stopSpeech(bool setFlag);
	void snd_playSoundEffect(int track, int volume);
	void snd_processEnvironmentalSoundEffect(int soundId, int block);
	void snd_queueEnvironmentalSoundEffect(int soundId, int block);
	void snd_playQueuedEffects();
	void snd_loadSoundFile(int track);
	int snd_playTrack(int track);
	int snd_stopMusic();

	int _lastSpeechId;
	int _lastSpeaker;
	uint32 _activeVoiceFileTotalTime;
	int _lastSfxTrack;
	int _lastMusicTrack;
	int _curMusicFileIndex;
	char _curMusicFileExt;
	int _environmentSfx;
	int _environmentSfxVol;
	int _envSfxDistThreshold;
	bool _envSfxUseQueue;
	int _envSfxNumTracksInQueue;
	uint16 _envSfxQueuedTracks[10];
	uint16 _envSfxQueuedBlocks[10];
	int _nextSpeechId;
	int _nextSpeaker;
	Common::List<Audio::AudioStream*> _speechList;

	int _curTlkFile;

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

	AudioDataStruct _soundData[3];

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
	void gui_highlightSelectedSpell(bool mode);
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
	void gui_specialSceneSuspendControls(int controlMode);
	void gui_specialSceneRestoreControls(int restoreLamp);

	bool _weaponsDisabled;
	int _lastButtonShape;
	uint32 _buttonPressTimer;
	int _selectedCharacter;
	int _compassDirection;
	int _compassStep;
	int _compassDirectionIndex;
	uint32 _compassTimer;
	int _charInventoryUnk;

	const CompassDef *_compassDefs;
	int _compassDefsSize;

	void gui_updateInput();
	void gui_triggerEvent(int eventType);
	void removeInputTop();
	void gui_enableDefaultPlayfieldButtons();
	void gui_enableSequenceButtons(int x, int y, int w, int h, int enableFlags);
	void gui_specialSceneRestoreButtons();
	void gui_enableCharInventoryButtons(int charNum);

	void gui_resetButtonList();
	void gui_initButtonsFromList(const int16 *list);
	void gui_setFaceFramesControlButtons(int index, int xOffs);
	void gui_initCharInventorySpecialButtons(int charNum);
	void gui_initMagicScrollButtons();
	void gui_initMagicSubmenu(int charNum);
	void gui_initButton(int index, int x = -1, int y = -1, int val = -1);
	void gui_notifyButtonListChanged();

	Common::Array<Button::Callback> _buttonCallbacks;
	Button *_activeButtons;
	Button _activeButtonData[70];
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
	int clickedSceneDropItem(Button *button);
	int clickedScenePickupItem(Button *button);
	int clickedInventorySlot(Button *button);
	int clickedInventoryScroll(Button *button);
	int clickedWall(Button *button);
	int clickedSequenceWindow(Button *button);
	int clickedScroll(Button *button);
	int clickedSpellTargetCharacter(Button *button);
	int clickedSpellTargetScene(Button *button);
	int clickedSceneThrowItem(Button *button);
	int clickedOptions(Button *button);
	int clickedRestParty(Button *button);
	int clickedMoneyBox(Button *button);
	int clickedCompass(Button *button);
	int clickedAutomap(Button *button);
	int clickedLamp(Button *button);
	int clickedStatusIcon(Button *button);

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
	int characterSays(int track, int charId, bool redraw);
	int playCharacterScriptChat(int charId, int mode, int unk1, char *str, EMCState *script, const uint16 *paramList, int16 paramIndex);

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
	int16 _visibleBlockIndex[18];
	int16 _globalScriptVars[24];

	// emc opcode
	int olol_setWallType(EMCState *script);
	int olol_getWallType(EMCState *script);
	int olol_drawScene(EMCState *script);
	int olol_getRand(EMCState *script);
	int olol_moveParty(EMCState *script);
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
	int olol_placeMoveLevelItem(EMCState *script);
	int olol_createLevelItem(EMCState *script);
	int olol_getItemPara(EMCState *script);
	int olol_getCharacterStat(EMCState *script);
	int olol_setCharacterStat(EMCState *script);
	int olol_loadLevelShapes(EMCState *script);
	int olol_closeLevelShapeFile(EMCState *script);
	int olol_loadDoorShapes(EMCState *script);
	int olol_initAnimStruct(EMCState *script);
	int olol_playAnimationPart(EMCState *script);
	int olol_freeAnimStruct(EMCState *script);
	int olol_getDirection(EMCState *script);
	int olol_characterSurpriseFeedback(EMCState *script);
	int olol_setMusicTrack(EMCState *script);
	int olol_setSequenceButtons(EMCState *script);
	int olol_setDefaultButtonState(EMCState *script);
	int olol_checkRectForMousePointer(EMCState *script);
	int olol_clearDialogueField(EMCState *script);
	int olol_setupBackgroundAnimationPart(EMCState *script);
	int olol_startBackgroundAnimation(EMCState *script);
	int olol_fadeToBlack(EMCState *script);
	int olol_fadePalette(EMCState *script);
	int olol_loadBitmap(EMCState *script);
	int olol_stopBackgroundAnimation(EMCState *script);
	int olol_getGlobalScriptVar(EMCState *script);
	int olol_setGlobalScriptVar(EMCState *script);
	int olol_getGlobalVar(EMCState *script);
	int olol_setGlobalVar(EMCState *script);
	int olol_triggerDoorSwitch(EMCState *script);
	int olol_checkEquippedItemScriptFlags(EMCState *script);
	int olol_setDoorState(EMCState *script);
	int olol_updateBlockAnimations(EMCState *script);
	int olol_mapShapeToBlock(EMCState *script);
	int olol_resetBlockShapeAssignment(EMCState *script);
	int olol_copyRegion(EMCState *script);
	int olol_initMonster(EMCState *script);
	int olol_fadeClearSceneWindow(EMCState *script);
	int olol_fadeSequencePalette(EMCState *script);
	int olol_redrawPlayfield(EMCState *script);
	int olol_loadNewLevel(EMCState *script);
	int olol_getNearestMonsterFromCharacter(EMCState *script);
	int olol_dummy0(EMCState *script);
	int olol_loadMonsterProperties(EMCState *script);
	int olol_battleHitSkillTest(EMCState *script);
	int olol_inflictDamage(EMCState *script);
	int olol_moveMonster(EMCState *script);
	int olol_dialogueBox(EMCState *script);
	int olol_giveTakeMoney(EMCState *script);
	int olol_checkMoney(EMCState *script);
	int olol_setScriptTimer(EMCState *script);
	int olol_createHandItem(EMCState *script);
	int olol_playAttackSound(EMCState *script);
	int olol_characterJoinsParty(EMCState *script);
	int olol_giveItem(EMCState *script);
	int olol_loadTimScript(EMCState *script);
	int olol_runTimScript(EMCState *script);
	int olol_releaseTimScript(EMCState *script);
	int olol_initSceneWindowDialogue(EMCState *script);
	int olol_restoreAfterSceneWindowDialogue(EMCState *script);
	int olol_getItemInHand(EMCState *script);
	int olol_checkMagic(EMCState *script);
	int olol_giveItemToMonster(EMCState *script);
	int olol_loadLangFile(EMCState *script);
	int olol_playSoundEffect(EMCState *script);
	int olol_processDialogue(EMCState *script);
	int olol_stopTimScript(EMCState *script);
	int olol_getWallFlags(EMCState *script);
	int olol_changeMonsterStat(EMCState *script);
	int olol_getMonsterStat(EMCState *script);
	int olol_releaseMonsterShapes(EMCState *script);
	int olol_playCharacterScriptChat(EMCState *script);
	int olol_playEnvironmentalSfx(EMCState *script);
	int olol_update(EMCState *script);
	int olol_healCharacter(EMCState *script);
	int olol_drawExitButton(EMCState *script);
	int olol_loadSoundFile(EMCState *script);
	int olol_playMusicTrack(EMCState *script);
	int olol_countBlockItems(EMCState *script);
	int olol_characterSkillTest(EMCState *script);
	int olol_countAllMonsters(EMCState *script);
	int olol_playEndSequence(EMCState *script);
	int olol_stopCharacterSpeech(EMCState *script);
	int olol_setPaletteBrightness(EMCState *script);
	int olol_calcInflictableDamage(EMCState *script);
	int olol_getInflictedDamage(EMCState *script);
	int olol_checkForCertainPartyMember(EMCState *script);
	int olol_printMessage(EMCState *script);
	int olol_deleteLevelItem(EMCState *script);
	int olol_calcInflictableDamagePerItem(EMCState *script);
	int olol_distanceAttack(EMCState *script);
	int olol_removeCharacterEffects(EMCState *script);
	int olol_checkInventoryFull(EMCState *script);
	int olol_objectLeavesLevel(EMCState *script);
	int olol_addSpellToScroll(EMCState *script);
	int olol_playDialogueText(EMCState *script);
	int olol_playDialogueTalkText(EMCState *script);
	int olol_checkMonsterTypeHostility(EMCState *script);
	int olol_setNextFunc(EMCState *script);
	int olol_dummy1(EMCState *script);
	int olol_suspendMonster(EMCState *script);
	int olol_triggerEventOnMouseButtonClick(EMCState *script);
	int olol_printWindowText(EMCState *script);
	int olol_countSpecificMonsters(EMCState *script);
	int olol_updateBlockAnimations2(EMCState *script);
	int olol_checkPartyForItemType(EMCState *script);
	int olol_blockDoor(EMCState *script);
	int olol_resetTimDialogueState(EMCState *script);
	int olol_getItemOnPos(EMCState *script);
	int olol_removeLevelItem(EMCState *script);
	int olol_savePage5(EMCState *script);
	int olol_restorePage5(EMCState *script);
	int olol_initDialogueSequence(EMCState *script);
	int olol_restoreAfterDialogueSequence(EMCState *script);
	int olol_setSpecialSceneButtons(EMCState *script);
	int olol_restoreButtonsAfterSpecialScene(EMCState *script);
	int olol_prepareSpecialScene(EMCState *script);
	int olol_restoreAfterSpecialScene(EMCState *script);
	int olol_assignCustomSfx(EMCState *script);
	int olol_findAssignedMonster(EMCState *script);
	int olol_checkBlockForMonster(EMCState *script);
	int olol_transformRegion(EMCState *script);
	int olol_calcCoordinatesAddDirectionOffset(EMCState *script);
	int olol_resetPortraitsAndDisableSysTimer(EMCState *script);
	int olol_enableSysTimer(EMCState *script);
	int olol_checkNeedSceneRestore(EMCState *script);
	int olol_getNextActiveCharacter(EMCState *script);
	int olol_paralyzePoisonCharacter(EMCState *script);
	int olol_drawCharPortrait(EMCState *script);
	int olol_removeInventoryItem(EMCState *script);
	int olol_getAnimationLastPart(EMCState *script);
	int olol_assignSpecialGuiShape(EMCState *script);
	int olol_findInventoryItem(EMCState *script);
	int olol_restoreFadePalette(EMCState *script);
	int olol_drinkBezelCup(EMCState *script);
	int olol_changeItemTypeOrFlag(EMCState *script);
	int olol_placeInventoryItemInHand(EMCState *script);
	int olol_castSpell(EMCState *script);
	int olol_pitDrop(EMCState *script);
	int olol_increaseSkill(EMCState *script);
	int olol_paletteFlash(EMCState *script);
	int olol_restoreMagicShroud(EMCState *script);
	int olol_disableControls(EMCState *script);
	int olol_enableControls(EMCState *script);
	int olol_shakeScene(EMCState *script);
	int olol_gasExplosion(EMCState *script);
	int olol_calcNewBlockPosition(EMCState *script);
	int olol_fadeScene(EMCState *script);
	int olol_updateDrawPage2(EMCState *script);
	int olol_setMouseCursor(EMCState *script);
	int olol_characterSays(EMCState *script);
	int olol_queueSpeech(EMCState *script);
	int olol_getItemPrice(EMCState *script);
	int olol_getLanguage(EMCState *script);

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

	Common::Array<const TIMOpcode*> _timOutroOpcodes;
	int tlol_fadeInScene(const TIM *tim, const uint16 *param);
	int tlol_unusedResourceFunc(const TIM *tim, const uint16 *param);
	int tlol_fadeInPalette(const TIM *tim, const uint16 *param);
	int tlol_fadeSoundOut(const TIM *tim, const uint16 *param);
	int tlol_displayAnimFrame(const TIM *tim, const uint16 *param);
	int tlol_delayForChat(const TIM *tim, const uint16 *param);
	int tlol_fadeOutSound(const TIM *tim, const uint16 *param);

	Common::Array<const TIMOpcode*> _timIngameOpcodes;
	int tlol_initSceneWindowDialogue(const TIM *tim, const uint16 *param);
	int tlol_restoreAfterSceneWindowDialogue(const TIM *tim, const uint16 *param);
	int tlol_giveItem(const TIM *tim, const uint16 *param);
	int tlol_setPartyPosition(const TIM *tim, const uint16 *param);
	int tlol_fadeClearWindow(const TIM *tim, const uint16 *param);
	int tlol_copyRegion(const TIM *tim, const uint16 *param);
	int tlol_characterChat(const TIM *tim, const uint16 *param);
	int tlol_drawScene(const TIM *tim, const uint16 *param);
	int tlol_update(const TIM *tim, const uint16 *param);
	int tlol_clearTextField(const TIM *tim, const uint16 *param);
	int tlol_loadSoundFile(const TIM *tim, const uint16 *param);
	int tlol_playMusicTrack(const TIM *tim, const uint16 *param);
	int tlol_playDialogueTalkText(const TIM *tim, const uint16 *param);
	int tlol_playSoundEffect(const TIM *tim, const uint16 *param);
	int tlol_startBackgroundAnimation(const TIM *tim, const uint16 *param);
	int tlol_stopBackgroundAnimation(const TIM *tim, const uint16 *param);

	// translation
	int _lang;

	uint8 *_landsFile;
	uint8 *_levelLangFile;

	int _lastUsedStringBuffer;
	char _stringBuffer[5][512];	// TODO: The original used a size of 512, it looks a bit large.
								// Maybe we can someday reduce the size.
	char *getLangString(uint16 id);
	uint8 *getTableEntry(uint8 *buffer, uint16 id);
	void decodeSjis(const char *src, char *dst);

	static const char * const _languageExt[];

	// graphics
	void setupScreenDims();
	void initSceneWindowDialogue(int controlMode);
	void restoreAfterSceneWindowDialogue(int redraw);
	void initDialogueSequence(int controlMode, int pageNum);
	void restoreAfterDialogueSequence(int controlMode);
	void resetPortraitsAndDisableSysTimer();
	void toggleSelectedCharacterFrame(bool mode);
	void fadeText();
	void transformRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage);
	void setPaletteBrightness(const Palette &srcPal, int brightness, int modifier);
	void generateBrightnessPalette(const Palette &src, Palette &dst, int brightness, int modifier);
	void generateFlashPalette(const Palette &src, Palette &dst, int colorFlags);
	void updateSequenceBackgroundAnimations();

	bool _dialogueField;
	uint8 **_itemIconShapes;
	int _numItemIconShapes;
	uint8 **_itemShapes;
	int _numItemShapes;
	uint8 **_gameShapes;
	int _numGameShapes;
	uint8 **_thrownShapes;
	int _numThrownShapes;
	uint8 **_effectShapes;
	int _numEffectShapes;

	const int8 *_gameShapeMap;
	int _gameShapeMapSize;

	uint8 *_characterFaceShapes[40][3];

	// characters
	bool addCharacter(int id);
	void setTemporaryFaceFrame(int charNum, int frame, int updateDelay, int redraw);
	void setTemporaryFaceFrameForAllCharacters(int frame, int updateDelay, int redraw);
	void setCharacterUpdateEvent(int charNum, int updateType, int updateDelay, int overwrite);
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

	void setCharacterMagicOrHitPoints(int charNum, int type, int points, int mode);
	void increaseExperience(int charNum, int skill, uint32 points);
	void increaseCharacterHitpoints(int charNum, int points, bool ignoreDeath);

	LoLCharacter *_characters;
	uint16 _activeCharsXpos[3];
	int _updateFlags;
	int _updateCharNum;
	int _updatePortraitSpeechAnimDuration;
	int _portraitSpeechAnimMode;
	int _updateCharV3;
	int _textColorFlag;
	bool _fadeText;
	int _needSceneRestore;
	uint32 _palUpdateTimer;
	uint32 _updatePortraitNext;

	int _loadLevelFlag;
	int _hasTempDataFlags;
	int _activeMagicMenu;
	uint16 _scriptCharacterCycle;
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

	int _lampEffect;
	int _brightness;
	int _lampOilStatus;
	uint32 _lampStatusTimer;
	bool _lampStatusSuspended;

	// level
	void loadLevel(int index);
	void addLevelItems();
	void loadLevelWallData(int index, bool mapShapes);
	void assignBlockObject(uint16 *cmzItemIndex, uint16 item);
	int assignLevelShapes(int index);
	uint8 *getLevelShapes(int index);
	void restoreBlockTempData(int index);
	void restoreTempDataAdjustMonsterStrength(int index);
	void loadCmzFile(const char *file);
	void loadLevelShpDat(const char *shpFile, const char *datFile, bool flag);
	void loadLevelGraphics(const char *file, int specialColor, int weight, int vcnLen, int vmpLen, const char *palFile);

	void resetItems(int flag);
	void disableMonsters();
	void resetBlockProperties();
	bool testWallFlag(int block, int direction, int flag);
	bool testWallInvisibility(int block, int direction);

	void drawScene(int pageNum);

	void generateBlockDrawingBuffer();
	void generateVmpTileData(int16 startBlockX, uint8 startBlockY, uint8 wllVmpIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY);
	void generateVmpTileDataFlipped(int16 startBlockX, uint8 startBlockY, uint8 wllVmpIndex, int16 vmpOffset, uint8 numBlocksX, uint8 numBlocksY);
	bool hasWall(int index);
	void assignVisibleBlocks(int block, int direction);

	void drawVcnBlocks();
	void drawSceneShapes();
	void setLevelShapesDim(int index, int16 &x1, int16 &x2, int dim);
	void scaleLevelShapesDim(int index, int16 &y1, int16 &y2, int dim);
	void drawLevelModifyScreenDim(int dim, int16 x1, int16 y1, int16 x2, int16 y2);
	void drawDecorations(int index);
	void drawBlockEffects(int index, int type);
	void drawSpecialGuiShape(int pageNum);
	void setWallType(int block, int wall, int val);
	void updateDrawPage2();

	void prepareSpecialScene(int fieldType, int hasDialogue, int suspendGui, int allowSceneUpdate, int controlMode, int fadeFlag);
	int restoreAfterSpecialScene(int fadeFlag, int redrawPlayField, int releaseTimScripts, int sceneUpdateMode);

	void setSequenceButtons(int x, int y, int w, int h, int enableFlags);
	void setSpecialSceneButtons(int x, int y, int w, int h, int enableFlags);
	void setDefaultButtonState();

	void updateCompass();

	void moveParty(uint16 direction, int unk1, int unk2, int buttonShape);
	bool checkBlockPassability(uint16 block, uint16 direction);
	void notifyBlockNotPassable(int scrollFlag);

	uint16 calcNewBlockPosition(uint16 curBlock, uint16 direction);
	uint16 calcBlockIndex(uint16 x, uint16 y);
	void calcCoordinates(uint16 &x, uint16 &y, int block, uint16 xOffs, uint16 yOffs);
	void calcCoordinatesForSingleCharacter(int charNum, uint16 &x, uint16 &y);
	void calcCoordinatesAddDirectionOffset(uint16 &x, uint16 &y, int direction);

	int clickedWallShape(uint16 block, uint16 direction);
	int clickedLeverOn(uint16 block, uint16 direction);
	int clickedLeverOff(uint16 block, uint16 direction);
	int clickedWallOnlyScript(uint16 block);
	int clickedDoorSwitch(uint16 block, uint16 direction);
	int clickedNiche(uint16 block, uint16 direction);

	bool clickedShape(int shapeIndex);
	void processDoorSwitch(uint16 block, int unk);
	void openCloseDoor(uint16 block, int openClose);
	void completeDoorOperations();

	void movePartySmoothScrollBlocked(int speed);
	void movePartySmoothScrollUp(int speed);
	void movePartySmoothScrollDown(int speed);
	void movePartySmoothScrollLeft(int speed);
	void movePartySmoothScrollRight(int speed);
	void movePartySmoothScrollTurnLeft(int speed);
	void movePartySmoothScrollTurnRight(int speed);

	void pitDropScroll(int numSteps);

	void shakeScene(int duration, int width, int height, int restore);
	void processGasExplosion(int soundId);

	int smoothScrollDrawSpecialGuiShape(int pageNum);

	OpenDoorState _openDoorState[3];
	int _blockDoor;

	int _smoothScrollModeNormal;

	const uint8 *_scrollXTop;
	int _scrollXTopSize;
	const uint8 *_scrollYTop;
	int _scrollYTopSize;
	const uint8 *_scrollXBottom;
	int _scrollXBottomSize;
	const uint8 *_scrollYBottom;
	int _scrollYBottomSize;

	int _nextScriptFunc;
	uint8 _currentLevel;
	int _sceneDefaultUpdate;
	int _lvlBlockIndex;
	int _lvlShapeIndex;
	bool _partyAwake;
	uint8 *_vcnBlocks;
	uint8 *_vcnShift;
	uint8 *_vcnExpTable;
	uint16 *_vmpPtr;
	uint16 *_blockDrawingBuffer;
	uint8 *_sceneWindowBuffer;
	LevelShapeProperty *_levelShapeProperties;
	uint8 **_levelShapes;

	uint8 *_specialGuiShape;
	uint16 _specialGuiShapeX;
	uint16 _specialGuiShapeY;
	uint16 _specialGuiShapeMirrorFlag;

	char _lastSuppFile[12];
	char _lastOverridePalFile[12];
	char *_lastOverridePalFilePtr;
	int _lastSpecialColor;
	int _lastSpecialColorWeight;

	int _sceneDrawVarDown;
	int _sceneDrawVarRight;
	int _sceneDrawVarLeft;
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
	LevelBlockProperty *_visibleBlocks[18];

	uint16 _partyPosX;
	uint16 _partyPosY;

	Common::SeekableReadStream *_lvlShpFileHandle;
	uint16 _lvlShpNum;
	uint16 _levelFileDataSize;
	LevelShapeProperty *_levelFileData;

	uint8 *_doorShapes[2];
	int _shpDmX;
	int _shpDmY;
	uint16 _dmScaleW;
	uint16 _dmScaleH;

	int _lastMouseRegion;
	int _seqWindowX1, _seqWindowY1,	_seqWindowX2, _seqWindowY2, _seqTrigger;
	int _spsWindowX, _spsWindowY,	_spsWindowW, _spsWindowH;

	uint8 *_tempBuffer5120;

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
	void takeCredits(int credits, int redraw);
	int makeItem(int itemType, int curFrame, int flags);
	void placeMoveLevelItem(int itemIndex, int level, int block, int xOffs, int yOffs, int flyingHeight);
	bool addItemToInventory(int itemIndex);
	bool testUnkItemFlags(int itemIndex);
	void deleteItem(int itemIndex);
	ItemInPlay *findObject(uint16 index);
	void runItemScript(int charNum, int item, int sub, int next, int reg4);
	void setHandItem(uint16 itemIndex);
	bool itemEquipped(int charNum, uint16 itemType);

	void setItemPosition(int item, uint16 x, uint16 y, int flyingHeight, int b);
	void removeLevelItem(int item, int block);
	bool launchObject(int objectType, int item, int startX, int startY, int flyingHeight, int direction, int, int attackerId, int c);
	void endObjectFlight(FlyingObject *t, int x, int y, int objectOnNextBlock);
	void processObjectFlight(FlyingObject *t, int x, int y);
	void updateObjectFlightPosition(FlyingObject *t);
	void objectFlightProcessHits(FlyingObject *t, int x, int y, int objectOnNextBlock);
	void updateFlyingObject(FlyingObject *t);

	void assignItemToBlock(uint16 *assignedBlockObjects, int id);
	int checkDrawObjectSpace(int itemX, int itemY, int partyX, int partyY);
	int checkSceneForItems(uint16 *blockDrawObjects, int colour);

	uint8 _moneyColumnHeight[5];
	uint16 _credits;

	ItemInPlay *_itemsInPlay;
	ItemProperty *_itemProperties;

	int _itemInHand;
	uint16 _inventory[48];
	int _inventoryCurItem;
	int _currentControlMode;
	int _specialSceneFlag;
	int _lastCharInventory;
	uint16 _charStatusFlags[3];
	int _emcLastItem;

	FlyingObject *_flyingObjects;

	EMCData _itemScript;

	const uint8 *_charInvIndex;
	int _charInvIndexSize;
	const uint8 *_charInvDefs;
	int _charInvDefsSize;
	const uint16 *_inventorySlotDesc;
	int _inventorySlotDescSize;
	const uint16 *_itemCost;
	int _itemCostSize;
	const uint8 *_stashSetupData;
	int _stashSetupDataSize;
	const int8 *_sceneItemOffs;
	int _sceneItemOffsSize;
	const FlyingObjectShape *_flyingItemShapes;
	int _flyingItemShapesSize;

	// monsters
	void loadMonsterShapes(const char *file, int monsterIndex, int b);
	void releaseMonsterShapes(int monsterIndex);
	int deleteMonstersFromBlock(int block);
	void setMonsterMode(MonsterInPlay *monster, int mode);
	bool updateMonsterAdjustBlocks(MonsterInPlay *monster);
	void placeMonster(MonsterInPlay *monster, uint16 x, uint16 y);
	int calcMonsterDirection(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void setMonsterDirection(MonsterInPlay *monster, int dir);
	void monsterDropItems(MonsterInPlay *monster);
	void removeAssignedObjectFromBlock(LevelBlockProperty *l, int id);
	void removeDrawObjectFromBlock(LevelBlockProperty *l, int id);
	void assignMonsterToBlock(uint16 *assignedBlockObjects, int id);
	void giveItemToMonster(MonsterInPlay *monster, uint16 item);
	int checkBlockBeforeObjectPlacement(uint16 x, uint16 y, uint16 objectWidth, uint16 testFlag, uint16 wallFlag);
	int checkBlockForWallsAndSufficientSpace(int block, int x, int y, int objectWidth, int testFlag, int wallFlag);
	int calcMonsterSkillLevel(int id, int a);
	int checkBlockOccupiedByParty(int x, int y, int testFlag);
	const uint16 *getCharacterOrMonsterStats(int id);
	uint16 *getCharacterOrMonsterItemsMight(int id);
	uint16 *getCharacterOrMonsterProtectionAgainstItems(int id);

	void drawBlockObjects(int blockArrayIndex);
	void drawMonster(uint16 id);
	int getMonsterCurFrame(MonsterInPlay *m, uint16 dirFlags);
	void reassignDrawObjects(uint16 direction, uint16 itemIndex, LevelBlockProperty *l, bool flag);
	void redrawSceneItem();
	int calcItemMonsterPosition(ItemInPlay *i, uint16 direction);
	void calcSpriteRelPosition(uint16 x1, uint16 y1, int &x2, int &y2, uint16 direction);
	void drawDoor(uint8 *shape, uint8 *table, int index, int unk2, int w, int h, int flags);
	void drawDoorOrMonsterShape(uint8 *shape, uint8 *table, int x, int y, int flags, const uint8 *ovl);
	uint8 *drawItemOrMonster(uint8 *shape, uint8 *ovl, int x, int y, int fineX, int fineY, int flags, int tblValue, bool vflip);
	int calcDrawingLayerParameters(int srcX, int srcY, int &x2, int &y2, uint16 &w, uint16 &h, uint8 *shape, int vflip);

	void updateMonster(MonsterInPlay *monster);
	void moveMonster(MonsterInPlay *monster);
	void walkMonster(MonsterInPlay *monster);
	bool chasePartyWithDistanceAttacks(MonsterInPlay *monster);
	void chasePartyWithCloseAttacks(MonsterInPlay *monster);
	int walkMonsterCalcNextStep(MonsterInPlay *monster);
	int getMonsterDistance(uint16 block1, uint16 block2);
	int checkForPossibleDistanceAttack(uint16 monsterBlock, int direction, int distance, uint16 curBlock);
	int walkMonsterCheckDest(int x, int y, MonsterInPlay *monster, int unk);
	void getNextStepCoords(int16 monsterX, int16 monsterY, int &newX, int &newY, uint16 direction);
	void rearrangeAttackingMonster(MonsterInPlay *monster);
	void moveStrayingMonster(MonsterInPlay *monster);
	void killMonster(MonsterInPlay *monster);

	MonsterInPlay *_monsters;
	MonsterProperty *_monsterProperties;
	uint8 **_monsterShapes;
	uint8 **_monsterPalettes;
	uint8 **_monsterShapesEx;
	uint8 _monsterAnimType[3];
	uint16 _monsterCurBlock;
	int _objectLastDirection;
	int _monsterCountUnk;
	int _monsterShiftAlt;

	const uint16 *_monsterModifiers;
	int _monsterModifiersSize;
	const int8 *_monsterShiftOffs;
	int _monsterShiftOffsSize;
	const uint8 *_monsterDirFlags;
	int _monsterDirFlagsSize;
	const uint8 *_monsterScaleX;
	int _monsterScaleXSize;
	const uint8 *_monsterScaleY;
	int _monsterScaleYSize;
	const uint16 *_monsterScaleWH;
	int _monsterScaleWHSize;

	// misc
	void delay(uint32 millis, bool doUpdate = false, bool isMainLoop = false);
	int generateRandomNumber(int min, int max);

	uint8 _compassBroken;
	uint8 _drainMagic;
	uint16 _globalScriptVars2[8];

	uint8 *_pageBuffer1;
	uint8 *_pageBuffer2;
	
	// spells
	typedef Common::Functor1Mem<ActiveSpell*, int, LoLEngine> SpellProc;
	Common::Array<const SpellProc*> _spellProcs;
	typedef void (LoLEngine::*SpellProcCallback)(WSAMovie_v2*, int, int);

	int castSpell(int charNum, int spellType, int spellLevel);

	int castSpark(ActiveSpell *a);
	int castHeal(ActiveSpell *a);
	int castIce(ActiveSpell *a);
	int castFireball(ActiveSpell *a);
	int castHandOfFate(ActiveSpell *a);
	int castMistOfDoom(ActiveSpell *a);
	int castLightning(ActiveSpell *a);
	int castFog(ActiveSpell *a);
	int castSwarm(ActiveSpell *a);
	int castVaelansCube(ActiveSpell *a);
	int castGuardian(ActiveSpell *a);
	int castHealOnSingleCharacter(ActiveSpell *a);

	int processMagicSpark(int charNum, int spellLevel);
	int processMagicHealSelectTarget();
	int processMagicHeal(int charNum, int spellLevel);
	int processMagicIce(int charNum, int spellLevel);
	int processMagicFireball(int charNum, int spellLevel);
	int processMagicHandOfFate(int spellLevel);
	int processMagicMistOfDoom(int charNum, int spellLevel);
	int processMagicLightning(int charNum, int spellLevel);
	int processMagicFog();
	int processMagicSwarm(int charNum, int damage);
	int processMagicVaelansCube();
	int processMagicGuardian(int charNum);

	void callbackProcessMagicSwarm(WSAMovie_v2 *mov, int x, int y);
	void callbackProcessMagicLightning(WSAMovie_v2 *mov, int x, int y);

	void drinkBezelCup(int a, int charNum);

	void addSpellToScroll(int spell, int charNum);
	void transferSpellToScollAnimation(int charNum, int spell, int slot);

	void playSpellAnimation(WSAMovie_v2 *mov, int firstFrame, int lastFrame, int frameDelay, int x, int y, SpellProcCallback callback, uint8 *pal1, uint8 *pal2, int fadeDelay, bool restoreScreen);
	int checkMagic(int charNum, int spellNum, int spellLevel);
	int getSpellTargetBlock(int currentBlock, int direction, int maxDistance, uint16 &targetBlock);
	void inflictMagicalDamage(int target, int attacker, int damage, int index, int hitType);
	void inflictMagicalDamageForBlock(int block, int attacker, int damage, int index);

	ActiveSpell _activeSpell;
	int8 _availableSpells[7];
	int _selectedSpell;
	const SpellProperty *_spellProperties;
	int _spellPropertiesSize;
	int _subMenuIndex;

	LightningProperty *_lightningProps;
	int16 _lightningCurSfx;
	int16 _lightningDiv;
	int16 _lightningFirstSfx;
	int16 _lightningSfxFrame;

	uint8 *_healOverlay;
	uint8 _swarmSpellStatus;

	uint8 **_fireballShapes;
	int _numFireballShapes;
	uint8 **_healShapes;
	int _numHealShapes;
	uint8 **_healiShapes;
	int _numHealiShapes;

	static const MistOfDoomAnimData _mistAnimData[];

	const uint8 *_updateSpellBookCoords;
	int _updateSpellBookCoordsSize;
	const uint8 *_updateSpellBookAnimData;
	int _updateSpellBookAnimDataSize;
	const uint8 *_healShapeFrames;
	int _healShapeFramesSize;
	const int16 *_fireBallCoords;
	int _fireBallCoordsSize;

	// fight
	int battleHitSkillTest(int16 attacker, int16 target, int skill);
	int calcInflictableDamage(int16 attacker, int16 target, int hitType);
	int inflictDamage(uint16 target, int damage, uint16 attacker, int skill, int flags);
	void characterHitpointsZero(int16 charNum, int a);
	void removeCharacterEffects(LoLCharacter *c, int first, int last);
	int calcInflictableDamagePerItem(int16 attacker, int16 target, uint16 itemMight, int index, int hitType);
	void checkForPartyDeath();

	void applyMonsterAttackSkill(MonsterInPlay *monster, int16 target, int16 damage);
	void applyMonsterDefenseSkill(MonsterInPlay *monster, int16 attacker, int flags, int skill, int damage);
	int removeCharacterItem(int charNum, int itemFlags);
	int paralyzePoisonCharacter(int charNum, int typeFlag, int immunityFlags, int hitChance, int redraw);
	void paralyzePoisonAllCharacters(int typeFlag, int immunityFlags, int hitChance);
	void stunCharacter(int charNum);
	void restoreSwampPalette();

	void distObj1Sub(int a, int b, int c, int d);
	void launchMagicViper();

	void breakIceWall(uint8 *pal1, uint8 *pal2);

	uint16 getNearestMonsterFromCharacter(int charNum);
	uint16 getNearestMonsterFromCharacterForBlock(uint16 block, int charNum);
	uint16 getNearestMonsterFromPos(int x, int y);
	uint16 getNearestPartyMemberFromPos(int x, int y);

	int _partyDamageFlags;

	// magic atlas
	void displayAutomap();
	void updateAutoMap(uint16 block);
	bool updateAutoMapIntern(uint16 block, uint16 x, uint16 y, int16 xOffs, int16 yOffs);
	void loadMapLegendData(int level);
	void drawMapPage(int pageNum);
	bool automapProcessButtons(int inputFlag);
	void automapBackButton();
	void automapForwardButton();
	void redrawMapCursor();
	void drawMapBlockWall(uint16 block, uint8 wall, int x, int y, int direction);
	void drawMapShape(uint8 wall, int x, int y, int direction);
	int mapGetStartPosX();
	int mapGetStartPosY();
	void mapIncludeLegendData(int type);
	void printMapText(uint16 stringId, int x, int y);
	void printMapExitButtonText();

	uint8 _currentMapLevel;
	uint8 *_mapOverlay;
	const uint8 **_automapShapes;
	const uint16 *_autoMapStrings;
	int _autoMapStringsSize;
	MapLegendData *_defaultLegendData;
	uint8 *_mapCursorOverlay;
	uint8 _automapTopLeftX;
	uint8 _automapTopLeftY;
	static const int8 _mapCoords[12][4];
	bool _mapUpdateNeeded;

	// unneeded
	void setWalkspeed(uint8) {}
	void removeHandItem() {}
	bool lineIsPassable(int, int) { return false; }

	// save
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *saveName, const Graphics::Surface *thumbnail);

	void generateTempData();
	LevelTempData *_lvlTempData[29];
};

} // end of namespace Kyra

#endif

#endif // ENABLE_LOL

