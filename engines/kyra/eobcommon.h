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
 */

#ifndef KYRA_EOBCOMMON_H
#define KYRA_EOBCOMMON_H

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)
#include "kyra/loleobbase.h"
#endif // (ENABLE_EOB || ENABLE_LOL)

#ifdef ENABLE_EOB

namespace Kyra {

struct EobShapeDef {
	int16 index;
	uint8 x, y, w, h;
};

struct CreatePartyModButton {
	uint8 encodeLabelX;
	uint8 encodeLabelY;
	uint8 labelW;
	uint8 labelH;
	uint8 labelX;
	uint8 labelY;
	uint8 bodyIndex;
	uint8 destX;
	uint8 destY;
};

struct EobRect8 {
	uint8 x;
	uint8 y;
	uint8 w;
	uint8 h;
};

struct EobChargenButtonDef {
	uint8 x;
	uint8 y;
	uint8 w;
	uint8 h;
	uint8 keyCode;
};

struct EobGuiButtonDef {
	uint16 keyCode;
	uint16 keyCode2;
	uint16 flags;
	uint16 x;
	uint8 y;
	uint16 w;
	uint8 h;
	Button::Callback buttonCallback;
	uint16 arg;
};

struct EobCharacter {
	uint8 id;
	uint8 flags;
	char name[11];
	int8 strengthCur;
	int8 strengthMax;
	int8 strengthExtCur;
	int8 strengthExtMax;
	int8 intelligenceCur;
	int8 intelligenceMax;
	int8 wisdomCur;
	int8 wisdomMax;
	int8 dexterityCur;
	int8 dexterityMax;
	int8 constitutionCur;
	int8 constitutionMax;
	int8 charismaCur;
	int8 charismaMax;
	int16 hitPointsCur;
	int16 hitPointsMax;
	int8 armorClass;
	uint8 disabledSlots;
	uint8 raceSex;
	uint8 cClass;
	uint8 alignment;
	int8 portrait;
	uint8 food;
	uint8 level[3];
	uint32 experience[3];
	uint8 *faceShape;

	int8 mageSpells[80];
	int8 clericSpells[80];
	uint32 mageSpellsAvailableFlags;

	Item inventory[27];
	uint32 timers[10];
	int8 events[10];
	uint8 effectsRemainder[4];
	uint32 effectFlags;
	uint8 damageTaken;
	int8 slotStatus[5];
};

struct EobItem {
	uint8 nameUnid;
	uint8 nameId;
	uint8 flags;
	int8 icon;
	int8 type;
	int8 pos;
	int16 block;
	Item next;
	Item prev;
	uint8 level;
	int8 value;
};

struct EobItemType {
	uint16 invFlags;
	uint16 handFlags;
	int8 armorClass;
	int8 allowedClasses;
	int8 requiredHands;
	int8 dmgNumDiceS;
	int8 dmgNumPipsS;
	int8 dmgIncS;
	int8 dmgNumDiceL;
	int8 dmgNumPipsL;
	int8 dmgIncL;
	uint8 unk1;
	uint16 extraProperties;
};

struct SpriteDecoration {
	uint8 *shp;
	uint8 x;
	uint8 y;
};

struct EobMonsterProperty {
	int8 armorClass;
	int8 hitChance;
	uint8 level;
	uint8 hpDcTimes;
	uint8 hpDcPips;
	uint8 hpDcBase;
	uint8 attacksPerRound;
	struct DmgDc {
		uint8 times;
		uint8 pips;
		int8 base;
	} dmgDc[3];
	uint16 statusFlags;
	uint32 capsFlags;
	uint32 typeFlags;
	int32 experience;

	uint8 u30;
	uint8 sound1;
	uint8 sound2;
	uint8 numRemoteAttacks;
	uint8 remoteWeaponChangeMode;
	uint8 numRemoteWeapons;

	int8 remoteWeapons[5];

	uint8 tuResist;
	uint8 dmgModifierEvade;

	uint8 decorations[3];
};

struct EobMonsterInPlay {
	uint8 type;
	uint8 unit;
	uint16 block;
	uint8 pos;
	int8 dir;
	uint8 animStep;
	uint8 shpIndex;
	int8 mode;
	int8 f_9;
	int8 curAttackFrame;
	int8 spellStatusLeft;
	int16 hitPointsMax;
	int16 hitPointsCur;
	uint16 dest;
	uint16 randItem;
	uint16 fixedItem;
	uint8 flags;
	uint8 idleAnimState;
	uint8 curRemoteWeapon;
	uint8 numRemoteAttacks;
	int8 palette;
	uint8 directionChanged;
	uint8 stepsTillRemoteAttack;
	uint8 sub;
};

struct ScriptTimer {
	uint16 func;
	uint16 ticks;
	uint32 next;
};

struct EobMenuDef {
	int8 titleStrId;
	uint8 dim;
	uint8 firstButtonStrId;
	int8 numButtons;
	int8 titleCol;
};
struct EobMenuButtonDef {
	int8 labelId;
	int16 x;
	int8 y;
	uint8 width;
	uint8 height;
	int16 keyCode;
	int16 flags;
};

class EobInfProcessor;

class EobCoreEngine : public LolEobBaseEngine {
friend class TextDisplayer_Eob;
friend class GUI_Eob;
friend class EobInfProcessor;
friend class DarkmoonSequenceHelper;
friend class CharacterGenerator;
public:
	EobCoreEngine(OSystem *system, const GameFlags &flags);
	virtual ~EobCoreEngine();

	Screen *screen() { return _screen; }
	GUI *gui() const { return _gui; }

protected:
	// Startup
	virtual Common::Error init();
	Common::Error go();

	// Main Menu, Intro, Finale
	virtual int mainMenu() = 0;
	virtual void seq_playFinale() = 0;
	bool _playFinale;

	//Init, config
	void loadItemsAndDecorationsShapes();
	void releaseItemsAndDecorationsShapes();

	void initButtonData();
	void initMenus();
	void initStaticResource();
	virtual void initSpells();

	void registerDefaultSettings();
	void readSettings();
	void writeSettings();

	const uint8 **_largeItemShapes;
	const uint8 **_smallItemShapes;
	const uint8 **_thrownItemShapes;
	const int _numLargeItemShapes;
	const int _numSmallItemShapes;
	const int _numThrownItemShapes;
	const int _numItemIconShapes;

	const uint8 **_spellShapes;
	const uint8 **_firebeamShapes;
	const uint8 *_redSplatShape;
	const uint8 *_greenSplatShape;
	const uint8 **_wallOfForceShapes;
	const uint8 **_teleporterShapes;
	const uint8 **_sparkShapes;
	const uint8 *_deadCharShape;
	const uint8 *_disabledCharGrid;
	const uint8 *_blackBoxSmallGrid;
	const uint8 *_weaponSlotGrid;
	const uint8 *_blackBoxWideGrid;
	const uint8 *_lightningColumnShape;

	uint8 *_tempIconShape;
	uint8 *_itemsOverlay;

	static const uint8 _teleporterShapeDefs[];
	static const uint8 _wallOfForceShapeDefs[];

	const char *const *_mainMenuStrings;

	// Main loop
	virtual void startupNew();
	virtual void startupLoad() = 0;
	void runLoop();
	void update() { screen()->updateScreen(); }
	bool checkPartyStatus(bool handleDeath);

	bool _runFlag;
	//int _runLoopUnk2;

	// Create Party
	bool startCharacterGeneration();

	uint8 **_faceShapes;

	static const int8 _characterClassType[];
	static const uint8 _hpIncrPerLevel[];
	static const uint8 _numLevelsPerClass[];
	static const int16 _hpConstModifiers[];
	static const uint8 _charClassModUnk[];

	const uint8 *_classModifierFlags;

	// timers
	void setupTimers();
	void setCharEventTimer(int charIndex, uint32 countdown, int evnt, int updateExistingTimer);
	void deleteCharEventTimer(int charIndex, int evnt);
	void setupCharacterTimers();
	void advanceTimers(uint32 millis);

	void timerProcessMonsters(int timerNum);
	void timerSpecialCharacterUpdate(int timerNum);
	void timerProcessFlyingObjects(int timerNum);
	void timerProcessCharacterExchange(int timerNum);
	void timerUpdateTeleporters(int timerNum);
	void timerUpdateFoodStatus(int timerNum);
	void timerUpdateMonsterIdleAnim(int timerNum);

	uint8 getClock2Timer(int index) { return index < _numClock2Timers ? _clock2Timers[index] : 0; }
	uint8 getNumClock2Timers()  { return _numClock2Timers; }

	static const uint8 _clock2Timers[];
	static const uint8 _numClock2Timers;

	int32 _restPartyElapsedTime;

	// Mouse
	void setHandItem(Item itemIndex);

	// Characters
	int getDexterityArmorClassModifier(int dexterity);
	int generateCharacterHitpointsByLevel(int charIndex, int levelIndex);
	int getClassAndConstHitpointsModifier(int cclass, int constitution);
	int getCharacterClassType(int cclass, int levelIndex);
	int getModifiedHpLimits(int hpModifier, int constModifier, int level, bool mode);
	Common::String getCharStrength(int str, int strExt);
	int testCharacter(int index, int flags);
	int getNextValidCharIndex(int curCharIndex, int searchStep);

	void recalcArmorClass(int index);
	int validateWeaponSlotItem(int index, int slot);
	int getCharacterClericPaladinLevel(int index);
	int getCharacterMageLevel(int index);
	int getCharacterLevelIndex(int type, int cClass);

	int countCharactersWithSpecificItems(int16 itemType, int16 itemValue);
	int checkInventoryForItem(int character, int16 itemType, int16 itemValue);
	void modifyCharacterHitpoints(int character, int16 points);
	void neutralizePoison(int character);

	virtual void npcSequence(int npcIndex) = 0;
	void initNpc(int npcIndex);
	int npcJoinDialogue(int npcIndex, int queryJoinTextId, int confirmJoinTextId, int noJoinTextId);
	int prepareForNewPartyMember(int16 itemType, int16 itemValue);
	void dropCharacter(int charIndex);
	void removeCharacterFromParty(int charIndex);
	void exchangeCharacters(int charIndex1, int charIndex2);

	void increasePartyExperience(int16 points);
	void increaseCharacterExperience(int charIndex, int32 points);
	uint32 getRequiredExperience(int cClass, int levelIndex, int level);
	void increaseCharacterLevel(int charIndex, int levelIndex);

	void setWeaponSlotStatus(int charIndex, int mode, int slot);

	EobCharacter *_characters;
	Common::String _strenghtStr;
	int _castScrollSlot;
	int _exchangeCharacterId;

	const char *const *_levelGainStrings;
	const uint32 *_expRequirementTables[6];

	const uint8 *_constModTables[6];
	const uint8 *_constModLevelIndex;
	const uint8 *_constModDiv;
	const uint8 *_constModExt;

	const EobCharacter *_npcPreset;
	bool _partyResting;
	bool _loading;

	// Items
	void loadItemDefs();
	Item duplicateItem(Item itemIndex);
	void setItemPosition(Item *itemQueue, int block, Item item, int pos);
	void createInventoryItem(EobCharacter *c, Item itemIndex, int itemValue, int preferedInventorySlot);
	int deleteInventoryItem(int charIndex, int slot);
	void deleteBlockItem(uint16 block, int type);
	int validateInventorySlotForItem(Item item, int charIndex, int slot);
	void deletePartyItem(Item itemType, int16 itemValue);
	virtual void updateUsedCharacterHandItem(int charIndex, int slot) = 0;
	int itemUsableByCharacter(int charIndex, Item item);
	int countQueuedItems(Item itemQueue, int16 id, int16 type, int count, int includeFlyingItems);
	int getQueuedItem(Item *items, int pos, int id);
	void printFullItemName(Item item);
	void identifyQueuedItems(Item itemQueue);
	void drawItemIconShape(int pageNum, Item itemId, int x, int y);
	bool isMagicWeapon(Item itemIndex);
	bool checkInventoryForRings(int charIndex, int itemValue);
	void eatItemInHand(int charIndex);

	bool launchObject(int charIndex, Item item, uint16 startBlock, int startPos, int dir, int type);
	void launchMagicObject(int charIndex, int type, uint16 startBlock, int startPos, int dir);
	bool updateObjectFlight(EobFlyingObject *fo, int block, int pos);
	bool updateFlyingObjectHitTest(EobFlyingObject *fo, int block, int pos);
	void explodeObject(EobFlyingObject *fo, int block, Item item);
	void endObjectFlight(EobFlyingObject *fo);
	void checkFlyingObjects();

	void reloadWeaponSlot(int charIndex, int slotIndex, int itemType, int arrowOrDagger);

	EobItem *_items;
	uint16 _numItems;
	EobItemType *_itemTypes;
	char **_itemNames;
	uint16 _numItemNames;
	uint32 _partyEffectFlags;
	Item _lastUsedItem;

	const uint16 *_slotValidationFlags;
	const int8 *_projectileWeaponAmmoTypes;
	const uint8 *_wandTypes;

	EobFlyingObject *_flyingObjects;
	const uint8 *_drawObjPosIndex;
	const uint8 *_flightObjFlipIndex;
	const int8 *_flightObjShpMap;
	const int8 *_flightObjSclIndex;

	const uint8 *_expObjectTlMode;
	const uint8 *_expObjectTblIndex;
	const uint8 *_expObjectShpStart;
	const uint8 *_expObjectAnimTbl1;
	int _expObjectAnimTbl1Size;
	const uint8 *_expObjectAnimTbl2;
	int _expObjectAnimTbl2Size;
	const uint8 *_expObjectAnimTbl3;
	int _expObjectAnimTbl3Size;

	// Monsters
	void loadMonsterShapes(const char *filename, int monsterIndex, bool hasDecorations, int encodeTableIndex);
	void releaseMonsterShapes(int first, int num);
	virtual void generateMonsterPalettes(const char *file, int16 monsterIndex) {}
	virtual void loadMonsterDecoration(const char *file, int16 monsterIndex) {}
	const uint8 *loadMonsterProperties(const uint8 *data);
	const uint8 *loadActiveMonsterData(const uint8 *data, int level);
	void initMonster(int index, int unit, uint16 block, int pos, int dir, int type, int shpIndex, int mode, int i, int randItem, int fixedItem);
	void placeMonster(EobMonsterInPlay *m, uint16 block, int dir);
	virtual void replaceMonster(int b, uint16 block, int pos, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) = 0;
	void killMonster(EobMonsterInPlay *m, bool giveExperience);
	virtual bool killMonsterExtra(EobMonsterInPlay *m);
	int countSpecificMonsters(int type);
	void updateAttackingMonsterFlags();

	const int8 *getMonsterBlockPositions(uint16 block);
	int getClosestMonsterPos(int charIndex, int block);

	bool blockHasMonsters(uint16 block);
	bool isMonsterOnPos(EobMonsterInPlay *m, uint16 block, int pos, int checkPos4);
	const int16 *findBlockMonsters(uint16 block, int pos, int dir, int blockDamage, int singleTargetCheckAdjacent);

	void drawBlockObject(int flipped, int page, const uint8 *shape, int x, int y, int sd, uint8 *ovl = 0);
	void drawMonsterShape(const uint8 *shape, int x, int y, int flipped, int flags, int palIndex);
	void flashMonsterShape(EobMonsterInPlay *m);
	void updateAllMonsterShapes();
	void drawBlockItems(int index);
	void drawDoor(int index);
	virtual void drawDoorIntern(int type, int index, int x, int y, int w, int wall, int mDim, int16 y1, int16 y2) = 0;
	void drawMonsters(int index);
	void drawWallOfForce(int index);
	void drawFlyingObjects(int index);
	void drawTeleporter(int index);

	void updateMonsters(int unit);
	void updateMonsterDest(EobMonsterInPlay *m);
	void updateMonsterDest2(EobMonsterInPlay *m);
	void turnFriendlyMonstersHostile();
	int getNextMonsterDirection(int curBlock, int destBlock);
	int getNextMonsterPos(EobMonsterInPlay *m, int block);
	int findFreeMonsterPos(int block, int size);
	void updateMoveMonster(EobMonsterInPlay *m);
	bool updateMonsterTryDistanceAttack(EobMonsterInPlay *m);
	bool updateMonsterTryCloseAttack(EobMonsterInPlay *m, int block);
	void walkMonster(EobMonsterInPlay *m, int destBlock);
	bool walkMonsterNextStep(EobMonsterInPlay *m, int destBlock, int direction);
	void updateMonsterFollowPath(EobMonsterInPlay *m, int turnSteps);
	void updateMonstersStraying(EobMonsterInPlay *m, int a);
	void updateMonstersSpellStatus(EobMonsterInPlay *m);
	void setBlockMonsterDirection(int block, int dir);

	uint8 *_monsterOvl1;
	uint8 *_monsterOvl2;

	SpriteDecoration *_monsterDecorations;
	EobMonsterProperty *_monsterProps;

	EobMonsterInPlay *_monsters;

	const int8 *_monsterStepTable0;
	const int8 *_monsterStepTable1;
	const int8 *_monsterStepTable2;
	const int8 *_monsterStepTable3;
	const uint8 *_monsterCloseAttPosTable1;
	const uint8 *_monsterCloseAttPosTable2;
	const int8 *_monsterCloseAttUnkTable;
	const uint8 *_monsterCloseAttChkTable1;
	const uint8 *_monsterCloseAttChkTable2;
	const uint8 *_monsterCloseAttDstTable1;
	const uint8 *_monsterCloseAttDstTable2;

	const uint8 *_monsterProximityTable;
	const uint8 *_findBlockMonstersTable;
	const char *const *_monsterDustStrings;
	
	const uint8 *_monsterDistAttType10;
	const uint8 *_monsterDistAttSfx10;
	const uint8 *_monsterDistAttType17;
	const uint8 *_monsterDistAttSfx17;
	const char *const *_monsterSpecAttStrings;

	const int8 *_monsterFrmOffsTable1;
	const int8 *_monsterFrmOffsTable2;

	const uint16 *_encodeMonsterShpTable;
	const uint8 _teleporterWallId;

	const int8 *_monsterDirChangeTable;

	// Level
	void loadLevel(int level, int func);
	Common::String initLevelData(int func);
	void addLevelItems();
	void loadVcnData(const char *file, const char */*nextFile*/);
	void loadBlockProperties(const char *mazFile);
	void loadDecorations(const char *cpsFile, const char *decFile);
	void assignWallsAndDecorations(int wallIndex, int vmpIndex, int decDataIndex, int specialType, int flags);
	void releaseDecorations();
	void releaseDoorShapes();
	void toggleWallState(int wall, int flags);
	virtual void loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2) {}
	virtual const uint8 *loadDoorShapes(const char *filename, int doorIndex, const uint8*shapeDefs) { return (const uint8*)filename; }

	void drawScene(int refresh);
	void drawSceneShapes(int start = 0);
	void drawDecorations(int index);

	int calcNewBlockPositionAndTestPassability(uint16 curBlock, uint16 direction);
	void notifyBlockNotPassable();
	void moveParty(uint16 block);

	int clickedDoorSwitch(uint16 block, uint16 direction);
	int clickedDoorPry(uint16 block, uint16 direction);
	int clickedDoorNoPry(uint16 block, uint16 direction);
	int clickedNiche(uint16 block, uint16 direction);

	int specialWallAction(int block, int direction);

	void openDoor(int block);
	void closeDoor(int block);

	int16 _doorType[2];
	int16 _noDoorSwitch[2];

	EobRect8 *_levelDecorationRects;
	SpriteDecoration *_doorSwitches;

	int8 _currentSub;
	Common::String _curGfxFile;

	uint32 _drawSceneTimer;
	uint32 _flashShapeTimer;
	uint32 _envAudioTimer;
	uint16 _teleporterPulse;

	const int16 *_dscShapeCoords;

	const uint8 *_dscItemPosIndex;
	const int16 *_dscItemShpX;
	const uint8 *_dscItemScaleIndex;
	const uint8 *_dscItemTileIndex;
	const uint8 *_dscItemShapeMap;

	const uint8 *_dscDoorScaleOffs;
	const uint8 *_dscDoorScaleMult1;
	const uint8 *_dscDoorScaleMult2;
	const uint8 *_dscDoorScaleMult3;
	const uint8 *_dscDoorY1;

	const uint8 *_wllFlagPreset;
	int _wllFlagPresetSize;
	const uint8 *_teleporterShapeCoords;

	// Script
	void runLevelScript(int block, int flags);
	void setScriptFlag(int flag);
	bool checkScriptFlag(int flag);

	const uint8 *initScriptTimers(const uint8 *pos);
	void updateScriptTimers();

	EobInfProcessor *_inf;
	int _stepCounter;
	int _stepsUntilScriptCall;
	ScriptTimer _scriptTimers[5];
	int _scriptTimersCount;
	uint8 _scriptTimersMode;

	// Gui
	void gui_drawPlayField(bool refresh);
	void gui_restorePlayField();
	void gui_drawAllCharPortraitsWithStats();
	void gui_drawCharPortraitWithStats(int index);
	void gui_drawFaceShape(int index);
	void gui_drawWeaponSlot(int charIndex, int slot);
	void gui_drawWeaponSlotStatus(int x, int y, int status);
	void gui_drawHitpoints(int index);
	void gui_drawFoodStatusGraph(int index);
	void gui_drawHorizontalBarGraph(int x, int y, int w, int h, int32 curVal, int32 maxVal, int col1, int col2);
	void gui_drawCharPortraitStatusFrame(int index);
	void gui_drawInventoryItem(int slot, int special, int pageNum);
	void gui_drawCompass(bool force);
	void gui_drawDialogueBox();
	void gui_drawSpellbook();
	void gui_drawSpellbookScrollArrow(int x, int y, int direction);
	void gui_updateSlotAfterScrollUse();
	void gui_updateControls();
	void gui_toggleButtons();
	void gui_setPlayFieldButtons();
	void gui_setInventoryButtons();
	void gui_setStatsListButtons();
	void gui_setSwapCharacterButtons();
	void gui_setCastOnWhomButtons();
	void gui_initButton(int index, int x = -1, int y = -1, int val = -1);
	Button *gui_getButton(Button *buttonList, int index);

	int clickedInventoryNextPage(Button *button);
	int clickedPortraitRestore(Button *button);
	int clickedCharPortraitDefault(Button *button);
	int clickedCamp(Button *button);
	int clickedSceneDropPickupItem(Button *button);
	int clickedCharPortrait2(Button *button);
	int clickedWeaponSlot(Button *button);
	int clickedCharNameLabelRight(Button *button);
	int clickedInventorySlot(Button *button);
	int clickedEatItem(Button *button);
	int clickedInventoryPrevChar(Button *button);
	int clickedInventoryNextChar(Button *button);
	int clickedSpellbookTab(Button *button);
	int clickedSpellbookList(Button *button);
	int clickedCastSpellOnCharacter(Button *button);
	int clickedUpArrow(Button *button);
	int clickedDownArrow(Button *button);
	int clickedLeftArrow(Button *button);
	int clickedRightArrow(Button *button);
	int clickedTurnLeftArrow(Button *button);
	int clickedTurnRightArrow(Button *button);
	int clickedAbortCharSwitch(Button *button);
	int clickedSceneThrowItem(Button *button);
	int clickedSceneSpecial(Button *button);
	int clickedSpellbookAbort(Button *button);
	int clickedSpellbookScroll(Button *button);
	int clickedUnk(Button *button);

	void gui_processCharPortraitClick(int index);
	void gui_processWeaponSlotClickLeft(int charIndex, int slotIndex);
	void gui_processWeaponSlotClickRight(int charIndex, int slotIndex);
	void gui_processInventorySlotClick(int slot);

	static const int16 _buttonList1[];
	int _buttonList1Size;
	static const int16 _buttonList2[];
	int _buttonList2Size;
	static const int16 _buttonList3[];
	int _buttonList3Size;
	static const int16 _buttonList4[];
	int _buttonList4Size;
	static const int16 _buttonList5[];
	int _buttonList5Size;
	static const int16 _buttonList6[];
	int _buttonList6Size;
	static const int16 _buttonList7[];
	int _buttonList7Size;
	static const int16 _buttonList8[];
	int _buttonList8Size;

	const EobGuiButtonDef *_buttonDefs;
	const char *const *_characterGuiStringsHp;
	const char *const *_characterGuiStringsWp;
	const char *const *_characterGuiStringsWr;
	const char *const *_characterGuiStringsSt;
	const char *const *_characterGuiStringsIn;

	const char *const *_characterStatusStrings7;
	const char *const *_characterStatusStrings8;
	const char *const *_characterStatusStrings9;
	const char *const *_characterStatusStrings12;
	const char *const *_characterStatusStrings13;

	const uint16 *_inventorySlotsX;
	const uint8 *_inventorySlotsY;
	const uint8 **_compassShapes;
	uint8 _charExchangeSwap;
	bool _configHpBarGraphs;

	// text
	void setupDialogueButtons(int presetfirst, int numStr, const char *str1, ...);
	void initDialogueSequence();
	void restoreAfterDialogueSequence();
	void drawSequenceBitmap(const char *file, int destRect, int x1, int y1, int flags);
	int runDialogue(int dialogueTextId, int style, const char *button1, ...);

	char _dialogueLastBitmap[13];
	int _dlgUnk1;
	int _moveCounter;

	uint8 _color4;
	uint8 _color5;
	uint8 _color6;
	uint8 _color7;
	uint8 _color8;
	uint8 _color9;
	uint8 _color10;
	uint8 _color11;
	uint8 _color12;
	uint8 _color13;
	uint8 _color14;

	const char *const *_chargenStatStrings;
	const char *const *_chargenRaceSexStrings;
	const char *const *_chargenClassStrings;
	const char *const *_chargenAlignmentStrings;

	const char *const *_pryDoorStrings;
	const char *const *_warningStrings;

	const char *const *_ripItemStrings;
	const char *const *_cursedString;
	const char *const *_enchantedString;
	const char *const *_magicObjectStrings;
	const char *const *_magicObjectString5;
	const char *const *_patternSuffix;
	const char *const *_patternGrFix1;
	const char *const *_patternGrFix2;
	const char *const *_validateArmorString;
	const char *const *_validateCursedString;
	const char *const *_validateNoDropString;
	const char *const *_potionStrings;
	const char *const *_wandStrings;	
	const char *const *_itemMisuseStrings;	
	
	const char *const *_suffixStringsRings;
	const char *const *_suffixStringsPotions;
	const char *const *_suffixStringsWands;

	const char *const *_takenStrings;
	const char *const *_potionEffectStrings;

	const char *const *_yesNoStrings;
	const char *const *_npcMaxStrings;
	const char *const *_okStrings;
	const char *const *_npcJoinStrings;
	const char *const *_cancelStrings;
	const char *const *_abortStrings;

	// Rest party
	void restParty_displayWarning(const char *str);
	bool restParty_updateMonsters();
	int restParty_getCharacterWithLowestHp();
	bool restParty_checkHealSpells(int charIndex);
	bool restParty_checkSpellsToLearn();
	virtual void restParty_npc();
	virtual bool restParty_extraAbortCondition();

	// misc
	void delay(uint32 millis, bool doUpdate = false, bool isMainLoop = false);
	void displayParchment(int id);
	virtual void useHorn(int charIndex, int weaponSlot) {}
	virtual bool checkPartyStatusExtra() = 0;

	virtual void drawLightningColumn() {}
	virtual int resurrectionSelectDialogue() { return -1; }
	virtual int charSelectDialogue() { return -1; }
	virtual void characterLevelGain(int charIndex) {}

	Common::Error loadGameState(int slot);
	Common::Error saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail);

	void *generateMonsterTempData(LevelTempData *tmp);
	void restoreMonsterTempData(LevelTempData *tmp);
	void releaseMonsterTempData(LevelTempData *tmp);

	const char * const *_saveLoadStrings;

	Screen_Eob *_screen;
	GUI_Eob *_gui;

	// fight
	void useSlotWeapon(int charIndex, int slotIndex, Item item);
	int closeDistanceAttack(int charIndex, Item item);
	int thrownAttack(int charIndex, int slotIndex, Item item);
	int projectileWeaponAttack(int charIndex, Item item);

	void inflictMonsterDamage(EobMonsterInPlay *m, int damage, bool giveExperience);
	void calcAndInflictMonsterDamage(EobMonsterInPlay *m, int times, int pips, int offs, int flags, int b, int damageType);
	void calcAndInflictCharacterDamage(int charIndex, int times, int itemOrPips, int useStrModifierOrBase, int flg, int a, int damageType);
	int calcCharacterDamage(int charIndex, int times, int itemOrPips, int useStrModifierOrBase, int flg, int a, int damageType) ;
	void inflictCharacterDamage(int charIndex, int damage);

	bool characterAttackHitTest(int charIndex, int monsterIndex, int item, int attackType);
	bool monsterAttackHitTest(EobMonsterInPlay *m, int charIndex);
	bool flyingObjectMonsterHit(EobFlyingObject *fo, int monsterIndex);
	bool flyingObjectPartyHit(EobFlyingObject *fo);

	void monsterCloseAttack(EobMonsterInPlay *m);
	void monsterSpellCast(EobMonsterInPlay *m, int type);
	void statusAttack(int charIndex, int attackStatusFlags, const char *attackStatusString, int a, uint32 effectDuration, int restoreEvent, int noRefresh);

	int calcCloseDistanceMonsterDamage(EobMonsterInPlay *m, int times, int pips, int offs, int flags, int b, int damageType);
	int calcDamageModifers(int charIndex, EobMonsterInPlay *m, int item, int itemType, int useStrModifier);
	bool checkUnkConstModifiers(void *target, int hpModifier, int level, int b, int race);
	bool specialAttackConstTest(int charIndex, int b);
	int getConstModifierTableValue(int hpModifier, int level, int b);
	bool calcDamageCheckItemType(int itemType);
	int recalcDamageModifier(int damageType, int dmgModifier);
	bool tryMonsterAttackEvasion(EobMonsterInPlay *m);
	int getStrHitChanceModifier(int charIndex);
	int getStrDamageModifier(int charIndex);
	int getDexHitChanceModifier(int charIndex);
	int getMonsterAcHitChanceModifier(int charIndex, int monsterAc);
	void explodeMonster(EobMonsterInPlay *m);
	
	int _dstMonsterIndex;
	bool _preventMonsterFlash;
	int16 _foundMonstersArray[5];

	// magic
	void useMagicBookOrSymbol(int charIndex, int type);
	void useMagicScroll(int charIndex, int type, int weaponSlot);
	void usePotion(int charIndex, int weaponSlot);
	void useWand(int charIndex, int weaponSlot);

	virtual void turnUndeadAuto() {};
	virtual void turnUndeadAutoHit() {};

	void castSpell(int spell, int weaponSlot);
	void removeCharacterEffect(int spell, int charIndex, int showWarning);
	void removeAllCharacterEffects(int charIndex);
	void castOnWhomDialogue();
	void startSpell(int spell);

	void sparkEffectDefensive(int charIndex);
	void sparkEffectOffensive();
	void setSpellEventTimer(int spell, int timerBaseFactor, int timerLength, int timerLevelFactor, int updateExistingTimer);
	void sortCharacterSpellList(int charIndex);

	bool magicObjectDamageHit(EobFlyingObject *fo, int dcTimes, int dcPips, int dcOffs, int level);
	bool magicObjectStatusHit(EobMonsterInPlay *m, int type, bool tryEvade, int mod);
	bool turnUndeadHit(EobMonsterInPlay *m, int hitChance, int casterLevel);

	void printWarning(const char* str);
	void printNoEffectWarning();

	void spellCallback_start_empty() {}
	bool spellCallback_end_empty(EobFlyingObject *fo) { return true; }
	void spellCallback_start_armor();
	void spellCallback_start_burningHands();
	void spellCallback_start_detectMagic();
	bool spellCallback_end_detectMagic(EobFlyingObject *fo);
	void spellCallback_start_magicMissile();
	bool spellCallback_end_magicMissile(EobFlyingObject *fo);
	void spellCallback_start_shockingGrasp();
	bool spellCallback_end_shockingGraspFlameBlade(EobFlyingObject *fo);
	void spellCallback_start_improvedIdentify();
	void spellCallback_start_melfsAcidArrow();
	bool spellCallback_end_melfsAcidArrow(EobFlyingObject *fo);
	void spellCallback_start_dispelMagic();
	void spellCallback_start_fireball();
	bool spellCallback_end_fireball(EobFlyingObject *fo);
	void spellCallback_start_flameArrow();
	bool spellCallback_end_flameArrow(EobFlyingObject *fo);
	void spellCallback_start_holdPerson();
	bool spellCallback_end_holdPerson(EobFlyingObject *fo);
	void spellCallback_start_lightningBolt();
	bool spellCallback_end_lightningBolt(EobFlyingObject *fo);
	void spellCallback_start_vampiricTouch();
	bool spellCallback_end_vampiricTouch(EobFlyingObject *fo);
	void spellCallback_start_fear();
	void spellCallback_start_iceStorm();
	bool spellCallback_end_iceStorm(EobFlyingObject *fo);
	void spellCallback_start_removeCurse();
	void spellCallback_start_coneOfCold();
	void spellCallback_start_holdMonster();
	bool spellCallback_end_holdMonster(EobFlyingObject *fo);
	void spellCallback_start_wallOfForce();
	void spellCallback_start_disintegrate();
	void spellCallback_start_fleshToStone();
	void spellCallback_start_stoneToFlesh();
	void spellCallback_start_trueSeeing();
	bool spellCallback_end_trueSeeing(EobFlyingObject *fo);
	void spellCallback_start_slayLiving();
	void spellCallback_start_powerWordStun();
	void spellCallback_start_causeLightWounds();
	void spellCallback_start_cureLightWounds();
	void spellCallback_start_aid();
	bool spellCallback_end_aid(EobFlyingObject *fo);
	void spellCallback_start_flameBlade();
	void spellCallback_start_slowPoison();
	bool spellCallback_end_slowPoison(EobFlyingObject *fo);
	void spellCallback_start_createFood();
	void spellCallback_start_removeParalysis();
	void spellCallback_start_causeSeriousWounds();
	void spellCallback_start_cureSeriousWounds();
	void spellCallback_start_neutralizePoison();
	void spellCallback_start_causeCriticalWounds();
	void spellCallback_start_cureCriticalWounds();
	void spellCallback_start_flameStrike();
	bool spellCallback_end_flameStrike(EobFlyingObject *fo);
	void spellCallback_start_raiseDead();
	void spellCallback_start_harm();
	void spellCallback_start_heal();
	void spellCallback_start_layOnHands();
	void spellCallback_start_turnUndead();
	bool spellCallback_end_unk1Passive(EobFlyingObject *fo);
	bool spellCallback_end_unk2Passive(EobFlyingObject *fo);
	bool spellCallback_end_deathSpellPassive(EobFlyingObject *fo);
	bool spellCallback_end_disintegratePassive(EobFlyingObject *fo);
	bool spellCallback_end_causeCriticalWoundsPassive(EobFlyingObject *fo);
	bool spellCallback_end_fleshToStonePassive(EobFlyingObject *fo);

	int8 _openBookSpellLevel;
	int8 _openBookSpellSelectedItem;
	int8 _openBookSpellListOffset;
	uint8 _openBookChar;
	uint8 _openBookType;
	uint8 _openBookCharBackup;
	uint8 _openBookTypeBackup;
	uint8 _openBookCasterLevel;
	const char *const *_openBookSpellList;
	int8 *_openBookAvailableSpells;
	uint8 _activeSpellCaster;
	uint8 _activeSpellCasterPos;
	uint8 _activeSpell;
	bool _returnAfterSpellCallback;

	typedef void (EobCoreEngine::*SpellStartCallback)();
	typedef bool (EobCoreEngine::*SpellEndCallback)(EobFlyingObject *fo);

	struct EobSpell {
		const char *name;
		SpellStartCallback startCallback;
		uint16 flags;
		const uint16 *timingPara;
		SpellEndCallback endCallback;
		uint8 sound;
		uint32 effectFlags;
		uint16 damageFlags;
	};

	EobSpell *_spells;
	int _numSpells;

	const char *const *_bookNumbers;
	const char *const *_mageSpellList;
	int _mageSpellListSize;
	int _clericSpellOffset;
	const char *const *_clericSpellList;
	const char *const *_spellNames;
	const char *const *_magicStrings1;
	const char *const *_magicStrings2;
	const char *const *_magicStrings3;
	const char *const *_magicStrings4;
	const char *const *_magicStrings6;
	const char *const *_magicStrings7;
	const char *const *_magicStrings8;

	uint8 *_spellAnimBuffer;

	const uint8 *_sparkEffectDefSteps;
	const uint8 *_sparkEffectDefSubSteps;
	const uint8 *_sparkEffectDefShift;
	const uint8 *_sparkEffectDefAdd;
	const uint8 *_sparkEffectDefX;
	const uint8 *_sparkEffectDefY;
	const uint32 *_sparkEffectOfFlags1;
	const uint32 *_sparkEffectOfFlags2;
	const uint8 *_sparkEffectOfShift;
	const uint8 *_sparkEffectOfX;
	const uint8 *_sparkEffectOfY;

	const uint8 *_magicFlightObjectProperties;
	const uint8 *_turnUndeadEffect;

	// Menu
	EobMenuDef *_menuDefs;
	const EobMenuButtonDef *_menuButtonDefs;

	bool _configMouse;

	const char *const *_menuStringsMain;
	const char *const *_menuStringsSaveLoad;
	const char *const *_menuStringsOnOff;
	const char *const *_menuStringsSpells;
	const char *const *_menuStringsRest;
	const char *const *_menuStringsDrop;
	const char *const *_menuStringsExit;
	const char *const *_menuStringsStarve;
	const char *const *_menuStringsScribe;
	const char *const *_menuStringsDrop2;
	const char *const *_menuStringsHead;
	const char *const *_menuStringsPoison;
	const char *const *_menuStringsMgc;
	const char *const *_menuStringsPrefs;
	const char *const *_menuStringsRest2;
	const char *const *_menuStringsRest3;
	const char *const *_menuStringsRest4;
	const char *const *_menuStringsDefeat;
	const char *_errorSlotEmptyString;
	const char *_errorSlotNoNameString;

	const char *const *_menuStringsTransfer;
	const char *const *_menuStringsSpec;
	const char *const *_menuStringsSpellNo;
	const char *const *_menuYesNoStrings;

	const uint8 *_spellLevelsMage;
	int _spellLevelsMageSize;
	const uint8 *_spellLevelsCleric;
	int _spellLevelsClericSize;
	const uint8 *_numSpellsCleric;
	const uint8 *_numSpellsWisAdj;
	const uint8 *_numSpellsPal;
	const uint8 *_numSpellsMage;

	// sound
	void snd_playSoundEffect(int id, int volume=0xFF);
};

}	// End of namespace Kyra

#endif // ENABLE_EOB

#endif