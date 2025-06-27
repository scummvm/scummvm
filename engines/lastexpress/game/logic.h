/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LASTEXPRESS_LOGIC_H
#define LASTEXPRESS_LOGIC_H

#include "lastexpress/lastexpress.h"
#include "lastexpress/shared.h"
#include "lastexpress/game/events.h"

#include "common/events.h"

namespace LastExpress {

class LastExpressEngine;

class CVCRFile;
class Action;
class Debugger;
class SaveLoad;

struct Link;
struct Node;
struct Item;
struct Door;
struct Character;
struct ConsCallParam;

#define HAND_PARAMS    Message *msg
#define CONS_PARAMS    ConsCallParam param1, ConsCallParam param2, ConsCallParam param3, ConsCallParam param4
#define CALL_PARAMS    void (LogicManager::*functionPointer)(CONS_PARAMS), CONS_PARAMS

class LogicManager {
	friend class ArchiveManager;
	friend class GoldArchiveManager;
	friend class CBeetle;
	friend class LastExpressEngine;
	friend class MemoryManager;
	friend class OtisManager;
	friend class SaveManager;
	friend class SoundManager;
	friend class VCR;
	friend class GraphicsManager;
	friend class Menu;
	friend class CFight;

public:
	LogicManager(LastExpressEngine *engine);
	~LogicManager();

	// LOGIC

	void send(int sender, int receiver, int actionId, ConsCallParam param);
	void sendAll(int character, int action, ConsCallParam param);
	void fedEx(int sender, int receiver, int action, ConsCallParam param);
	void forceJump(int character, void (LogicManager::*functionPointer)(CONS_PARAMS));
	void autoMessage(int character, int action, ConsCallParam param);
	bool doAutoMessage(Message *msg);
	void save(int character, int type, int event);
	void endGame(int type, int value, int sceneIndex, bool showScene);
	void winGame();
	void killGracePeriod();
	void fadeToBlack();
	void fadeToWhite();
	void restoreIcons();
	void dropItem(int item, int outLocation);
	void takeItem(int item);
	void giveCathItem(int item);
	void takeCathItem(int item);
	bool cathHasItem(int item);
	int findLargeItem();
	int checkCathDir();
	bool checkCathDir(int car, int position);
	bool isFemale(int character);
	bool isSingleFemale(int character);
	bool isNight();
	bool whoOutside(int character);
	bool whoOnScreen(int character);
	bool checkLoc(int character, int car);
	bool emptyComp(int car, int position);
	bool inComp(int character, int car, int position);
	bool inComp(int character);
	bool cathOutHisWindow();
	bool cathOutRebeccaWindow();
	bool inSuite(int character, int car, int position);
	bool inOffice(int character);
	bool inLowBaggage(int character);
	bool inSalon(int character);
	bool inDiningRoom(int character);
	bool inKitchen(int character);
	bool inOuterSanctum(int character);
	bool inInnerSanctum(int character);
	bool onLowPlatform(int character);
	bool onKronPlatform(int character);
	bool cathInCorridor(int car);
	bool cathFacingUp();
	bool rcClear();
	int32 absPosition(int character1, int character2);
	bool nearChar(int character1, int character2, int maxDist);
	bool nearX(int character, int x, int maxDist);
	bool whoFacingCath(int character);
	bool whoWalking(int character);
	int checkDoor(int door);
	bool preventEnterComp(int door);
	void setDoor(int door, int character, int status, int windowCursor, int handleCursor);
	void setModel(int door, int8 model);
	int getModel(int door);
	void blockView(int character, int car, int position);
	void releaseView(int character, int car, int position);
	void releaseEverything(int character);
	void release2Views(int character, int car, int pos1, int pos2);
	void block2ViewsBump4(int character, int car, int pos1, int pos2, int pos3, int pos4);
	void blockAtDoor(int character, int status);
	void releaseAtDoor(int character, int status);
	void softBlockAtDoor(int character, int status);
	void softReleaseAtDoor(int character, int status);
	int getBumpNode(int car, int position, int param);
	void bumpCath(int car, int position, int param);
	bool obstacleBetween(int character1, int character2);
	bool bumpCathTowardsCond(int door, bool playSound, bool loadScene);
	void bumpCathFx(int car, int position);
	void bumpCathRx(int car, int position);
	void bumpCathFDoor(int door);
	void bumpCathRDoor(int door);
	void bumpCathTylerComp();
	int getSmartBumpNode(int node);
	void smartBumpCath();
	void bumpCathCloseUp(int item);
	int playFight(int fightId);
	void playNIS(int nisId);
	void cleanNIS();
	int getVolume(int character);
	void queueSFX(int index, uint8 action, uint8 delay);
	void playCathExcuseMe();
	void playChrExcuseMe(int character, int receivingCharacter, int volume);
	const char *getCathJustChecking();
	const char *getCathSorryDialog();
	const char *getCathWCDialog();
	void playCondYelling(int character, int situation);
	void playDialog(int character, const char *filename, int volume, int delay);
	bool whoRunningDialog(int character);
	bool dialogRunning(const char *name);
	bool cathRunningDialog(const char *name);
	void fadeDialog(uint32 character);
	void fadeDialog(const char *name);
	void endDialog(uint32 character);
	void endDialog(const char *name);
	void claimNISLink(int character);
	int32 getSoundTicks(int character);
	int whoseBit(int character);
	bool cathWillSeeOtis(int position);
	bool walk(int character, int car, int position);
	void startCycOtis(int character, const char *filename);
	void startSeqOtis(int character, const char *filename);
	void advanceFrame(int character);
	void endGraphics(int character);
	void makeAllJump(int chapter);

	// ACTION

	int findCursor(Link *link);
	bool nodeHasItem(int item);
	void doPreFunction(int *sceneOut);
	void doPostFunction();
	void doAction(Link *link);
	void takeTyler(bool doCleanNIS, int8 bedPosition);
	void dropTyler(bool doCleanNIS);
	void takeJacket(bool doCleanNIS);
	void takeScarf(bool doCleanNIS);
	const char *getHintDialog(int character);

	// TRAIN NAVIGATION

	void loadTrain(int cd);
	void nodeStepTimer(Event *event);
	void mouseStatus();
	void getNewMnum();
	void nodeStepMouse(Event *event);
	void doF4();
	bool pointIn(int32 cursorX, int32 cursorY, Link *hotspot);
	void checkInventory(int32 flags);
	void bumpCathNode(int sceneIndex);
	void displayWaitIcon();
	void restoreEggIcon();

	// DEBUGGER COMMANDS
	int32 getGameTime();
	int32 getRealTime();
	int32 getTimeSpeed();
	Common::String translateNodeProperty(int property);
	void showCurrentTrainNode();
	void showCharacterDebugger();
	const char *getCharacterName(int index) const;
	bool isCharacterPinned(int index) const;
	void toggleCharacterPin(int index);
	void renderCharacterDetails(Character *character, int index);
	void renderCharacterGrid(bool onlyPinned, int &selectedCharacter);
	void renderCharacterList(int &selectedCharacter);
	void renderCurrentSceneDebugger();
	void renderGlobalVars();

	Common::StringArray getCharacterFunctionNames(int character);

	void showTrainMapWindow();

	bool _pinnedCharacters[40] = { false };
	const char *_characterNames[40] = {
		"Cath", "Anna", "August", "Cond1", "Cond2", "HeadWait", "Waiter1", "Waiter2",
		"Cook", "TrainM", "Tatiana", "Vassili", "Alexei", "Abbot", "Milos", "Vesna",
		"Ivo", "Salko", "Kronos", "Kahina", "Francois", "Madame", "Monsieur", "Rebecca",
		"Sophie", "Mahmud", "Yasmin", "Hadija", "Alouan", "Police", "Max", "Master",
		"Clerk", "TableA", "TableB", "TableC", "TableD", "TableE", "TableF", "Mitchell"
	};

private:
	LastExpressEngine *_engine = nullptr;

	Node *_trainData = nullptr;
	int32 _activeNode = 0;
	int8 _closeUp = 0;
	int32 _nodeReturn = 0;
	int32 _nodeReturn2 = 0;
	int32 _activeItem = 0;
	int32 _trainIndex = -1;
	int32 _numberOfScenes = 0;

	Item *_items = nullptr;
	Door *_doors = nullptr;
	int32 *_blockedViews = nullptr;
	int32 *_blockedX = nullptr;
	int32 *_softBlockedX = nullptr;
	int32 *_globals = nullptr;
	byte *_doneNIS = nullptr;

	int32 _gameTime = 0;
	int32 _timeSpeed = 1;
	int32 _realTime = 0;
	int32 _lastSavegameSessionTicks = 0;

	int _doorPositions[8] = {
		8200, 7500, 6470, 5790,
		4840, 4070, 3050, 2740 
	};

	int32 _lastTickCondYellingComp2 = 0;
	int32 _lastTickCondYellingComp3 = 0;
	int32 _lastTickCondYellingComp4 = 0;
	int32 _lastTickCondYellingCompHarem = 0;
	int32 _lastTickCondYellingCompA = 0;
	int32 _lastTickCondYellingCompB = 0;
	int32 _lastTickCondYellingCompC = 0;
	int32 _lastTickCondYellingCompD = 0;
	int32 _lastTickCondYellingCompE = 0;
	int32 _lastTickCondYellingCompF = 0;
	int32 _lastTickCondYellingCompG = 0;
	int32 _lastTickCondYellingCompH = 0;

	bool _doubleClickFlag = false;

	bool _inventoryFlag1 = false;
	bool _inventoryFlag2 = false;
	bool _isEggHighlighted = false;
	bool _isMagnifierInUse = false;
	int _highlightedItem = 0;
	int _inventoryVerticalSlot = 0;

	bool _flagBumpCathNode = false;

	int _navigationItemBrightness = 0;
	int _navigationItemBrighnessStep = 1;
	int32 _eventTicksUntilNextSavePoint = 450;
	int32 _eventTicksSinceLastDemoSavegame = 150;
	bool _actionJustPerformed = false;

	bool _mitchellFlag = false;
	Common::String _mitchellSequence = "";

	int _concertPhasesTicks[54] = {
		735, 1395, 1965, 2205, 3405, 3750, 3975, 4365, 4650,
		4770, 4995, 5085, 5430, 5685, 5850, 7515, 7620, 7785,
		7875, 8235, 8340, 8745, 8805, 8925, 8985, 9765, 9930,
		12375, 12450, 12705, 13140, 13305, 13380, 13560, 14145,
		14385, 14445, 14805, 16485, 16560, 16755, 16845, 17700,
		17865, 18645, 18720, 19410, 19500, 22020, 22185, 22590,
		22785, 23085, 23265
	};

	const char *_concertPhases[54] = {
		"201d", "201a", "201d", "201a", "201d", "201a", "201d", "201a", "201d", "201a",
		"201e", "201d", "201a", "201d", "201a", "201d", "201a", "201d", "201a", "201d",
		"201a", "201d", "201a", "201d", "201a", "201d", "201a", "201e", "201d", "201c",
		"201d", "201a", "201d", "201a", "201d", "201a", "201c", "201a", "201d", "201a",
		"201d", "201a", "201d", "201a", "201d", "201a", "201e", "201a", "201d", "201a",
		"201d", "201a", "201d", "201a"
	};

	int _nisTicks[273] = {
		0,    255,  255,  225,  195,  750,  750,  765,  765,  750,
		750,  0,    1005, 780,  300,  285,  870,  0,    120,  120,
		120,  525,  180,  210,  210,  210,  135,  945,  300,  390,
		375,  1050, 945,  495,  1230, 1425, 195,  405,  600,  945,
		510,  540,  855,  645,  0,    0,    0,    1005, 255,  0,
		255,  1320, 210,  120,  1350, 315,  315,  315,  480,  660,
		300,  1305, 300,  0,    0,    0,    0,    0,    0,    0,
		0,    0,    0,    0,    0,    105,  510,  510,  720,  120,
		465,  690,  450,  465,  1020, 600,  435,  525,  150,  390,
		225,  240,  1095, 0,    720,  1005, 840,  840,  450,  450,
		450,  450,  630,  0,    1710, 240,  240,  930,  1035, 1035,
		540,  150,  150,  90,   885,  0,    135,  1665, 285,  285,
		165,  165,  165,  0,    0,    0,    0,    300,  180,  0,
		450,  450,  450,  450,  450,  450,  450,  450,  150,  150,
		90,   90,   135,  105,  0,    150,  165,  180,  0,    165,
		165,  165,  165,  450,  450,  450,  450,  450,  450,  0,
		0,    315,  0,    0,    0,    0,    0,    0,    120,  105,
		105,  450,  450,  525,  0,    615,  180,  180,  180,  180,
		180,  300,  300,  195,  195,  195,  195,  195,  195,  135,
		1410, 0,    1920, 600,  195,  105,  165,  0,    150,  150,
		150,  180,  180,  180,  90,   90,   90,   0,    0,    0,
		390,  360,  105,  75,   75,   150,  120,  120,  120,  120,
		120,  105,  105,  105,  105,  270,  270,  270,  120,  120,
		165,  165,  165,  165,  150,  150,  150,  150,  120,  120,
		120,  120,  90,   135,  135,  135,  150,  180,  180,  150,
		150,  150,  75,   75,   120,  120,  375,  225,  225,  105,
		195,  120,  180,  135,  105,  195,  195,  240,  240,  195,
		195,  600,  600
	};

	const char *_nisNames[273] = {
		"",
		"1002",
		"1002D",
		"1003",
		"1005",
		"1006", // 5
		"1006A",
		"1008",
		"1008N",
		"1008A",
		"1008AN", // 10
		"1009",
		"1011",
		"1011A",
		"1012",
		"1013",
		"1017",  // 15
		"1017A",
		"1019",
		"1019D",
		"1020", // 20
		"1022",
		"1022A",
		"1022AD",
		"1022B",
		"1022C", // 25
		"1023",
		"1025",
		"1028",
		"1030",
		"1031", // 30
		"1032",
		"1033",
		"1034",
		"1035",
		"1037", // 35
		"1038",
		"1038A",
		"1039",
		"1040",
		"1041", // 40
		"1042",
		"1043",
		"1044",
		"1046",
		"1047", // 45
		"1047A",
		"1059",
		"1060",
		"1063",
		"1101", // 50
		"1102",
		"1103",
		"1104",
		"1105",
		"1106", // 55
		"1106A",
		"1106D",
		"1107",
		"1107A",
		"1108", // 60
		"1109",
		"1110",
		"1112",
		"1115",
		"1115A", // 65
		"1115B",
		"1115C",
		"1115D",
		"1115E",
		"1115F", // 70
		"1115G",
		"1115H",
		"1116",
		"1117",
		"1118", // 75
		"1202",
		"1202A",
		"1203",
		"1204",
		"1205", // 80
		"1206",
		"1206A",
		"1208",
		"1210",
		"1211", // 85
		"1212",
		"1213",
		"1213A",
		"1215",
		"1216", // 90
		"1219",
		"1222",
		"1223",
		"1224",
		"1225", // 95
		"1227",
		"1227A",
		"1303",
		"1303N",
		"1304", // 100
		"1304N",
		"1305",
		"1309",
		"1311",
		"1312", // 105
		"1312D",
		"1313",
		"1315",
		"1315A",
		"1401", // 110
		"1402",
		"1402B",
		"1403",
		"1404",
		"1404A", // 115
		"1405",
		"1406",
		"1501",
		"1501A",
		"1502", // 120
		"1502A",
		"1502D",
		"1503",
		"1504",
		"1505", // 125
		"1505A",
		"1506",
		"1506A",
		"1508",
		"1509", // 130
		"1509S",
		"1509A",
		"1509AS",
		"1509N",
		"1509SN", // 135
		"1509AN",
		"1509BN",
		"1511",
		"1511A",
		"1511B", // 140
		"1511BA",
		"1511C",
		"1511D",
		"1930",
		"1511E", // 145
		"1512",
		"1513",
		"1517",
		"1517A",
		"1518", // 150
		"1518A",
		"1518B",
		"1591",
		"1592",
		"1593", // 155
		"1594",
		"1595",
		"1596",
		"1601",
		"1603", // 160
		"1606B",
		"1607A",
		"1610",
		"1611",
		"1612", // 165
		"1615",
		"1619",
		"1620",
		"1621",
		"1622", // 170
		"1629",
		"1630",
		"1631",
		"1632",
		"1633", // 175
		"1634",
		"1702",
		"1702DD",
		"1702NU",
		"1702ND", // 180
		"1704",
		"1704D",
		"1705",
		"1705D",
		"1706", // 185
		"1706DD",
		"1706ND",
		"1706NU",
		"1901",
		"1902", // 190
		"1903",
		"1904",
		"1908",
		"1908A",
		"1908B", // 195
		"1908C",
		"1908CD",
		"1909A",
		"1909B",
		"1909C", // 200
		"1910A",
		"1910B",
		"1910C",
		"1911A",
		"1911B", // 205
		"1911C",
		"1912",
		"1913",
		"1917",
		"1918", // 210
		"1919",
		"1919A",
		"1920",
		"1922",
		"1923", // 215
		"8001",
		"8001A",
		"8002",
		"8002A",
		"8002B", // 220
		"8003",
		"8003A",
		"8004",
		"8004A",
		"8005", // 225
		"8005B",
		"8010",
		"8013",
		"8013A",
		"8014", // 230
		"8014A",
		"8014R",
		"8014AR",
		"8015",
		"8015A", // 235
		"8015R",
		"8015AR",
		"8017",
		"8017A",
		"8017R", // 240
		"8017AR",
		"8017N",
		"8023",
		"8023A",
		"8023M", // 245
		"8024",
		"8024A",
		"8024M",
		"8025",
		"8025A", // 250
		"8025M",
		"8027",
		"8028",
		"8029",
		"8029A", // 255
		"8031",
		"8032",
		"8032A",
		"8033",
		"8035", // 260
		"8035A",
		"8035B",
		"8035C",
		"8036",
		"8037", // 265
		"8037A",
		"8040",
		"8040A",
		"8041",
		"8041A", // 270
		"8042",
		"8042A"
	};

	/////////////////////////////////////////////
	//            CHARACTER EVENTS
	/////////////////////////////////////////////

	// MAIN ENTRY POINT
	void CONS_All(bool isFirstChapter, int character);

	// CHAPTER SELECTOR
	void CONS_Abbot(int chapter);
	void CONS_Alexei(int chapter);
	void CONS_Alouan(int chapter);
	void CONS_Anna(int chapter);
	void CONS_August(int chapter);
	void CONS_Clerk(int chapter);
	void CONS_Cond1(int chapter);
	void CONS_Cond2(int chapter);
	void CONS_Cook(int chapter);
	void CONS_Francois(int chapter);
	void CONS_Hadija(int chapter);
	void CONS_HeadWait(int chapter);
	void CONS_Ivo(int chapter);
	void CONS_Kahina(int chapter);
	void CONS_Kronos(int chapter);
	void CONS_Madame(int chapter);
	void CONS_Mahmud(int chapter);
	void CONS_Master(int chapter);
	void CONS_Max(int chapter);
	void CONS_Milos(int chapter);
	void CONS_Mitchell(int chapter);
	void CONS_Monsieur(int chapter);
	void CONS_Police(int chapter);
	void CONS_Rebecca(int chapter);
	void CONS_Salko(int chapter);
	void CONS_Sophie(int chapter);
	void CONS_TableA(int chapter);
	void CONS_TableB(int chapter);
	void CONS_TableC(int chapter);
	void CONS_TableD(int chapter);
	void CONS_TableE(int chapter);
	void CONS_TableF(int chapter);
	void CONS_Tatiana(int chapter);
	void CONS_TrainM(int chapter);
	void CONS_Vassili(int chapter);
	void CONS_Vesna(int chapter);
	void CONS_Waiter1(int chapter);
	void CONS_Waiter2(int chapter);
	void CONS_Yasmin(int chapter);

	// EVENTS
	// 
	// Each event has:
	// - A constructor
	// - An handler

	// ABBOT EVENTS

	void AbbotCall(CALL_PARAMS);

	void CONS_Abbot_DebugWalks(CONS_PARAMS);
	void HAND_Abbot_DebugWalks(HAND_PARAMS);

	void CONS_Abbot_DoSeqOtis(CONS_PARAMS);
	void HAND_Abbot_DoSeqOtis(HAND_PARAMS);

	void CONS_Abbot_DoCorrOtis(CONS_PARAMS);
	void HAND_Abbot_DoCorrOtis(HAND_PARAMS);

	void CONS_Abbot_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_Abbot_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_Abbot_FinishSeqOtis(CONS_PARAMS);
	void HAND_Abbot_FinishSeqOtis(HAND_PARAMS);

	void CONS_Abbot_DoComplexSeqOtis(CONS_PARAMS);
	void HAND_Abbot_DoComplexSeqOtis(HAND_PARAMS);

	void CONS_Abbot_DoWait(CONS_PARAMS);
	void HAND_Abbot_DoWait(HAND_PARAMS);

	void CONS_Abbot_DoWaitReal(CONS_PARAMS);
	void HAND_Abbot_DoWaitReal(HAND_PARAMS);

	void CONS_Abbot_DoDialog(CONS_PARAMS);
	void HAND_Abbot_DoDialog(HAND_PARAMS);

	void CONS_Abbot_SaveGame(CONS_PARAMS);
	void HAND_Abbot_SaveGame(HAND_PARAMS);

	void CONS_Abbot_DoWalk(CONS_PARAMS);
	void HAND_Abbot_DoWalk(HAND_PARAMS);

	void CONS_Abbot_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_Abbot_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_Abbot_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_Abbot_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_Abbot_WaitRCClear(CONS_PARAMS);
	void HAND_Abbot_WaitRCClear(HAND_PARAMS);

	void CONS_Abbot_Birth(CONS_PARAMS);
	void HAND_Abbot_Birth(HAND_PARAMS);

	void CONS_Abbot_StartPart2(CONS_PARAMS);
	void HAND_Abbot_StartPart2(HAND_PARAMS);

	void CONS_Abbot_StartPart3(CONS_PARAMS);
	void HAND_Abbot_StartPart3(HAND_PARAMS);

	void CONS_Abbot_InKitchen(CONS_PARAMS);
	void HAND_Abbot_InKitchen(HAND_PARAMS);

	void CONS_Abbot_OpenComp(CONS_PARAMS);
	void HAND_Abbot_OpenComp(HAND_PARAMS);

	void CONS_Abbot_AloneComp(CONS_PARAMS);
	void HAND_Abbot_AloneComp(HAND_PARAMS);

	void CONS_Abbot_GoLunch(CONS_PARAMS);
	void HAND_Abbot_GoLunch(HAND_PARAMS);

	void CONS_Abbot_EatingLunch(CONS_PARAMS);
	void HAND_Abbot_EatingLunch(HAND_PARAMS);

	void CONS_Abbot_LeaveLunch(CONS_PARAMS);
	void HAND_Abbot_LeaveLunch(HAND_PARAMS);

	void CONS_Abbot_ClosedComp(CONS_PARAMS);
	void HAND_Abbot_ClosedComp(HAND_PARAMS);

	void CONS_Abbot_GoSalon1(CONS_PARAMS);
	void HAND_Abbot_GoSalon1(HAND_PARAMS);

	void CONS_Abbot_InSalon(CONS_PARAMS);
	void HAND_Abbot_InSalon(HAND_PARAMS);

	void CONS_Abbot_GoComp(CONS_PARAMS);
	void HAND_Abbot_GoComp(HAND_PARAMS);

	void CONS_Abbot_OpenComp2(CONS_PARAMS);
	void HAND_Abbot_OpenComp2(HAND_PARAMS);

	void CONS_Abbot_GoWander(CONS_PARAMS);
	void HAND_Abbot_GoWander(HAND_PARAMS);

	void CONS_Abbot_GoSalon2(CONS_PARAMS);
	void HAND_Abbot_GoSalon2(HAND_PARAMS);

	void CONS_Abbot_InSalon2(CONS_PARAMS);
	void HAND_Abbot_InSalon2(HAND_PARAMS);

	void CONS_Abbot_GoComp3(CONS_PARAMS);
	void HAND_Abbot_GoComp3(HAND_PARAMS);

	void CONS_Abbot_OpenComp3(CONS_PARAMS);
	void HAND_Abbot_OpenComp3(HAND_PARAMS);

	void CONS_Abbot_GoSalon3(CONS_PARAMS);
	void HAND_Abbot_GoSalon3(HAND_PARAMS);

	void CONS_Abbot_InSalon3(CONS_PARAMS);
	void HAND_Abbot_InSalon3(HAND_PARAMS);

	void CONS_Abbot_WithAugust(CONS_PARAMS);
	void HAND_Abbot_WithAugust(HAND_PARAMS);

	void CONS_Abbot_GoComp4(CONS_PARAMS);
	void HAND_Abbot_GoComp4(HAND_PARAMS);

	void CONS_Abbot_InComp4(CONS_PARAMS);
	void HAND_Abbot_InComp4(HAND_PARAMS);

	void CONS_Abbot_StartPart4(CONS_PARAMS);
	void HAND_Abbot_StartPart4(HAND_PARAMS);

	void CONS_Abbot_DoWalkSearchingForCath(CONS_PARAMS);
	void HAND_Abbot_DoWalkSearchingForCath(HAND_PARAMS);

	void CONS_Abbot_AtDinner(CONS_PARAMS);
	void HAND_Abbot_AtDinner(HAND_PARAMS);

	void CONS_Abbot_LeaveDinner(CONS_PARAMS);
	void HAND_Abbot_LeaveDinner(HAND_PARAMS);

	void CONS_Abbot_InComp(CONS_PARAMS);
	void HAND_Abbot_InComp(HAND_PARAMS);

	void CONS_Abbot_Asleep4(CONS_PARAMS);
	void HAND_Abbot_Asleep4(HAND_PARAMS);

	void CONS_Abbot_Conferring(CONS_PARAMS);
	void HAND_Abbot_Conferring(HAND_PARAMS);

	void CONS_Abbot_GoSalon4(CONS_PARAMS);
	void HAND_Abbot_GoSalon4(HAND_PARAMS);

	void CONS_Abbot_BeforeBomb(CONS_PARAMS);
	void HAND_Abbot_BeforeBomb(HAND_PARAMS);

	void CONS_Abbot_AfterBomb(CONS_PARAMS);
	void HAND_Abbot_AfterBomb(HAND_PARAMS);

	void CONS_Abbot_CatchCath(CONS_PARAMS);
	void HAND_Abbot_CatchCath(HAND_PARAMS);

	void CONS_Abbot_StartPart5(CONS_PARAMS);
	void HAND_Abbot_StartPart5(HAND_PARAMS);

	void CONS_Abbot_Prisoner(CONS_PARAMS);
	void HAND_Abbot_Prisoner(HAND_PARAMS);

	void CONS_Abbot_Free(CONS_PARAMS);
	void HAND_Abbot_Free(HAND_PARAMS);

	void CONS_Abbot_RunningTrain(CONS_PARAMS);
	void HAND_Abbot_RunningTrain(HAND_PARAMS);

	Common::StringArray _abbotFuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"DoCorrOtis",
		"DoEnterCorrOtis",
		"FinishSeqOtis",
		"DoComplexSeqOtis",
		"DoWait",
		"DoWaitReal",
		"DoDialog",
		"SaveGame",
		"DoWalk",
		"DoJoinedSeqOtis",
		"DoBlockSeqOtis",
		"WaitRCClear",
		"Birth",
		"StartPart2",
		"StartPart3",
		"InKitchen",
		"OpenComp",
		"AloneComp",
		"GoLunch",
		"EatingLunch",
		"LeaveLunch",
		"ClosedComp",
		"GoSalon1",
		"InSalon",
		"GoComp",
		"OpenComp2",
		"GoWander",
		"GoSalon2",
		"InSalon2",
		"GoComp3",
		"OpenComp3",
		"GoSalon3",
		"InSalon3",
		"WithAugust",
		"GoComp4",
		"InComp4",
		"StartPart4",
		"DoWalkSearchingForCath",
		"AtDinner",
		"LeaveDinner",
		"InComp",
		"Asleep4",
		"Conferring",
		"GoSalon4",
		"BeforeBomb",
		"AfterBomb",
		"CatchCath",
		"StartPart5",
		"Prisoner",
		"Free",
		"RunningTrain"
	};


	// ALEXEI EVENTS

	void AlexeiCall(CALL_PARAMS);

	void CONS_Alexei_DebugWalks(CONS_PARAMS);
	void HAND_Alexei_DebugWalks(HAND_PARAMS);

	void CONS_Alexei_DoDialog(CONS_PARAMS);
	void HAND_Alexei_DoDialog(HAND_PARAMS);

	void CONS_Alexei_DoWait(CONS_PARAMS);
	void HAND_Alexei_DoWait(HAND_PARAMS);

	void CONS_Alexei_DoSeqOtis(CONS_PARAMS);
	void HAND_Alexei_DoSeqOtis(HAND_PARAMS);

	void CONS_Alexei_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_Alexei_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_Alexei_DoCorrOtis(CONS_PARAMS);
	void HAND_Alexei_DoCorrOtis(HAND_PARAMS);

	void CONS_Alexei_FinishSeqOtis(CONS_PARAMS);
	void HAND_Alexei_FinishSeqOtis(HAND_PARAMS);

	void CONS_Alexei_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_Alexei_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_Alexei_SaveGame(CONS_PARAMS);
	void HAND_Alexei_SaveGame(HAND_PARAMS);

	void CONS_Alexei_DoWalk(CONS_PARAMS);
	void HAND_Alexei_DoWalk(HAND_PARAMS);

	void CONS_Alexei_DoComplexSeqOtis(CONS_PARAMS);
	void HAND_Alexei_DoComplexSeqOtis(HAND_PARAMS);

	void CONS_Alexei_WaitRCClear(CONS_PARAMS);
	void HAND_Alexei_WaitRCClear(HAND_PARAMS);

	void CONS_Alexei_EnterComp(CONS_PARAMS);
	void HAND_Alexei_EnterComp(HAND_PARAMS);

	void CONS_Alexei_ExitComp(CONS_PARAMS);
	void HAND_Alexei_ExitComp(HAND_PARAMS);

	void CONS_Alexei_PacingAtWindow(CONS_PARAMS);
	void HAND_Alexei_PacingAtWindow(HAND_PARAMS);

	void CONS_Alexei_CompLogic(CONS_PARAMS);
	void HAND_Alexei_CompLogic(HAND_PARAMS);

	void CONS_Alexei_Birth(CONS_PARAMS);
	void HAND_Alexei_Birth(HAND_PARAMS);

	void CONS_Alexei_AtDinner(CONS_PARAMS);
	void HAND_Alexei_AtDinner(HAND_PARAMS);

	void CONS_Alexei_ReturnComp(CONS_PARAMS);
	void HAND_Alexei_ReturnComp(HAND_PARAMS);

	void CONS_Alexei_GoSalon(CONS_PARAMS);
	void HAND_Alexei_GoSalon(HAND_PARAMS);

	void CONS_Alexei_Sitting(CONS_PARAMS);
	void HAND_Alexei_Sitting(HAND_PARAMS);

	void CONS_Alexei_StandingAtWindow(CONS_PARAMS);
	void HAND_Alexei_StandingAtWindow(HAND_PARAMS);

	void CONS_Alexei_WaitingForTat(CONS_PARAMS);
	void HAND_Alexei_WaitingForTat(HAND_PARAMS);

	void CONS_Alexei_Upset(CONS_PARAMS);
	void HAND_Alexei_Upset(HAND_PARAMS);

	void CONS_Alexei_ReturnCompNight(CONS_PARAMS);
	void HAND_Alexei_ReturnCompNight(HAND_PARAMS);

	void CONS_Alexei_Asleep(CONS_PARAMS);
	void HAND_Alexei_Asleep(HAND_PARAMS);

	void CONS_Alexei_WakeUp(CONS_PARAMS);
	void HAND_Alexei_WakeUp(HAND_PARAMS);

	void CONS_Alexei_StartPart2(CONS_PARAMS);
	void HAND_Alexei_StartPart2(HAND_PARAMS);

	void CONS_Alexei_InComp2(CONS_PARAMS);
	void HAND_Alexei_InComp2(HAND_PARAMS);

	void CONS_Alexei_AtBreakfast(CONS_PARAMS);
	void HAND_Alexei_AtBreakfast(HAND_PARAMS);

	void CONS_Alexei_ReturnCompPart2(CONS_PARAMS);
	void HAND_Alexei_ReturnCompPart2(HAND_PARAMS);

	void CONS_Alexei_StartPart3(CONS_PARAMS);
	void HAND_Alexei_StartPart3(HAND_PARAMS);

	void CONS_Alexei_PlayingChess(CONS_PARAMS);
	void HAND_Alexei_PlayingChess(HAND_PARAMS);

	void CONS_Alexei_InPart3(CONS_PARAMS);
	void HAND_Alexei_InPart3(HAND_PARAMS);

	void CONS_Alexei_Pacing3(CONS_PARAMS);
	void HAND_Alexei_Pacing3(HAND_PARAMS);

	void CONS_Alexei_GoSalon3(CONS_PARAMS);
	void HAND_Alexei_GoSalon3(HAND_PARAMS);

	void CONS_Alexei_StartPart4(CONS_PARAMS);
	void HAND_Alexei_StartPart4(HAND_PARAMS);

	void CONS_Alexei_InComp4(CONS_PARAMS);
	void HAND_Alexei_InComp4(HAND_PARAMS);

	void CONS_Alexei_MeetTat(CONS_PARAMS);
	void HAND_Alexei_MeetTat(HAND_PARAMS);

	void CONS_Alexei_LeavePlat(CONS_PARAMS);
	void HAND_Alexei_LeavePlat(HAND_PARAMS);

	void CONS_Alexei_InCompAgain(CONS_PARAMS);
	void HAND_Alexei_InCompAgain(HAND_PARAMS);

	void CONS_Alexei_GoSalon4(CONS_PARAMS);
	void HAND_Alexei_GoSalon4(HAND_PARAMS);

	void CONS_Alexei_Pacing(CONS_PARAMS);
	void HAND_Alexei_Pacing(HAND_PARAMS);

	void CONS_Alexei_GoToPlatform(CONS_PARAMS);
	void HAND_Alexei_GoToPlatform(HAND_PARAMS);

	void CONS_Alexei_ReturnComp4(CONS_PARAMS);
	void HAND_Alexei_ReturnComp4(HAND_PARAMS);

	void CONS_Alexei_BombPlanB(CONS_PARAMS);
	void HAND_Alexei_BombPlanB(HAND_PARAMS);

	void CONS_Alexei_Dead(CONS_PARAMS);
	void HAND_Alexei_Dead(HAND_PARAMS);

	void CONS_Alexei_StartPart5(CONS_PARAMS);
	void HAND_Alexei_StartPart5(HAND_PARAMS);

	Common::StringArray _alexeiFuncNames = {
		"DebugWalks",
		"DoDialog",
		"DoWait",
		"DoSeqOtis",
		"DoBlockSeqOtis",
		"DoCorrOtis",
		"FinishSeqOtis",
		"DoJoinedSeqOtis",
		"SaveGame",
		"DoWalk",
		"DoComplexSeqOtis",
		"WaitRCClear",
		"EnterComp",
		"ExitComp",
		"PacingAtWindow",
		"CompLogic",
		"Birth",
		"AtDinner",
		"ReturnComp",
		"GoSalon",
		"Sitting",
		"StandingAtWindow",
		"WaitingForTat",
		"Upset",
		"ReturnCompNight",
		"Asleep",
		"WakeUp",
		"StartPart2",
		"InComp2",
		"AtBreakfast",
		"ReturnCompPart2",
		"StartPart3",
		"PlayingChess",
		"InPart3",
		"Pacing3",
		"GoSalon3",
		"StartPart4",
		"InComp4",
		"MeetTat",
		"LeavePlat",
		"InCompAgain",
		"GoSalon4",
		"Pacing",
		"GoToPlatform",
		"ReturnComp4",
		"BombPlanB",
		"Dead",
		"StartPart5"
	};

	// ALOUAN EVENTS

	void AlouanCall(CALL_PARAMS);

	void CONS_Alouan_DebugWalks(CONS_PARAMS);
	void HAND_Alouan_DebugWalks(HAND_PARAMS);

	void CONS_Alouan_DoCorrOtis(CONS_PARAMS);
	void HAND_Alouan_DoCorrOtis(HAND_PARAMS);

	void CONS_Alouan_DoDialog(CONS_PARAMS);
	void HAND_Alouan_DoDialog(HAND_PARAMS);

	void CONS_Alouan_DoWait(CONS_PARAMS);
	void HAND_Alouan_DoWait(HAND_PARAMS);

	void CONS_Alouan_DoWalk(CONS_PARAMS);
	void HAND_Alouan_DoWalk(HAND_PARAMS);

	void CONS_Alouan_PeekF(CONS_PARAMS);
	void HAND_Alouan_PeekF(HAND_PARAMS);

	void CONS_Alouan_PeekH(CONS_PARAMS);
	void HAND_Alouan_PeekH(HAND_PARAMS);

	void CONS_Alouan_GoFtoH(CONS_PARAMS);
	void HAND_Alouan_GoFtoH(HAND_PARAMS);

	void CONS_Alouan_GoHtoF(CONS_PARAMS);
	void HAND_Alouan_GoHtoF(HAND_PARAMS);

	void CONS_Alouan_Birth(CONS_PARAMS);
	void HAND_Alouan_Birth(HAND_PARAMS);

	void CONS_Alouan_Part1(CONS_PARAMS);
	void HAND_Alouan_Part1(HAND_PARAMS);

	void CONS_Alouan_Asleep(CONS_PARAMS);
	void HAND_Alouan_Asleep(HAND_PARAMS);

	void CONS_Alouan_StartPart2(CONS_PARAMS);
	void HAND_Alouan_StartPart2(HAND_PARAMS);

	void CONS_Alouan_Part2(CONS_PARAMS);
	void HAND_Alouan_Part2(HAND_PARAMS);

	void CONS_Alouan_StartPart3(CONS_PARAMS);
	void HAND_Alouan_StartPart3(HAND_PARAMS);

	void CONS_Alouan_Part3(CONS_PARAMS);
	void HAND_Alouan_Part3(HAND_PARAMS);

	void CONS_Alouan_StartPart4(CONS_PARAMS);
	void HAND_Alouan_StartPart4(HAND_PARAMS);

	void CONS_Alouan_Part4(CONS_PARAMS);
	void HAND_Alouan_Part4(HAND_PARAMS);

	void CONS_Alouan_Asleep4(CONS_PARAMS);
	void HAND_Alouan_Asleep4(HAND_PARAMS);

	void CONS_Alouan_StartPart5(CONS_PARAMS);
	void HAND_Alouan_StartPart5(HAND_PARAMS);

	void CONS_Alouan_Prisoner(CONS_PARAMS);
	void HAND_Alouan_Prisoner(HAND_PARAMS);

	void CONS_Alouan_Free(CONS_PARAMS);
	void HAND_Alouan_Free(HAND_PARAMS);

	void CONS_Alouan_Hiding(CONS_PARAMS);
	void HAND_Alouan_Hiding(HAND_PARAMS);

	void CONS_Alouan_Disappear(CONS_PARAMS);
	void HAND_Alouan_Disappear(HAND_PARAMS);

	Common::StringArray _alouanFuncNames = {
		"DebugWalks",
		"DoCorrOtis",
		"DoDialog",
		"DoWait",
		"DoWalk",
		"PeekF",
		"PeekH",
		"GoFtoH",
		"GoHtoF",
		"Birth",
		"Part1",
		"Asleep",
		"StartPart2",
		"Part2",
		"StartPart3",
		"Part3",
		"StartPart4",
		"Part4",
		"Asleep4",
		"StartPart5",
		"Prisoner",
		"Free",
		"Hiding",
		"Disappear"
	};

	// ANNA EVENTS

	void AnnaCall(CALL_PARAMS);

	void CONS_Anna_DebugWalks(CONS_PARAMS);
	void HAND_Anna_DebugWalks(HAND_PARAMS);

	void CONS_Anna_DoSeqOtis(CONS_PARAMS);
	void HAND_Anna_DoSeqOtis(HAND_PARAMS);

	void CONS_Anna_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_Anna_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_Anna_DoCorrOtis(CONS_PARAMS);
	void HAND_Anna_DoCorrOtis(HAND_PARAMS);

	void CONS_Anna_FinishSeqOtis(CONS_PARAMS);
	void HAND_Anna_FinishSeqOtis(HAND_PARAMS);

	void CONS_Anna_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_Anna_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_Anna_DoDialog(CONS_PARAMS);
	void HAND_Anna_DoDialog(HAND_PARAMS);

	void CONS_Anna_WaitRCClear(CONS_PARAMS);
	void HAND_Anna_WaitRCClear(HAND_PARAMS);

	void CONS_Anna_SaveGame(CONS_PARAMS);
	void HAND_Anna_SaveGame(HAND_PARAMS);

	void CONS_Anna_DoWalk(CONS_PARAMS);
	void HAND_Anna_DoWalk(HAND_PARAMS);

	void CONS_Anna_DoWait(CONS_PARAMS);
	void HAND_Anna_DoWait(HAND_PARAMS);

	void CONS_Anna_PracticeMusic(CONS_PARAMS);
	void HAND_Anna_PracticeMusic(HAND_PARAMS);

	void CONS_Anna_DoComplexSeqOtis(CONS_PARAMS);
	void HAND_Anna_DoComplexSeqOtis(HAND_PARAMS);

	void CONS_Anna_DoWaitReal(CONS_PARAMS);
	void HAND_Anna_DoWaitReal(HAND_PARAMS);

	void CONS_Anna_CompLogic(CONS_PARAMS);
	void HAND_Anna_CompLogic(HAND_PARAMS);

	void CONS_Anna_Birth(CONS_PARAMS);
	void HAND_Anna_Birth(HAND_PARAMS);

	void CONS_Anna_DoWalkP1(CONS_PARAMS);
	void HAND_Anna_DoWalkP1(HAND_PARAMS);

	void CONS_Anna_DiningLogic(CONS_PARAMS);
	void HAND_Anna_DiningLogic(HAND_PARAMS);

	void CONS_Anna_FleeTyler(CONS_PARAMS);
	void HAND_Anna_FleeTyler(HAND_PARAMS);

	void CONS_Anna_WaitDinner(CONS_PARAMS);
	void HAND_Anna_WaitDinner(HAND_PARAMS);

	void CONS_Anna_GoDinner(CONS_PARAMS);
	void HAND_Anna_GoDinner(HAND_PARAMS);

	void CONS_Anna_WaitHW(CONS_PARAMS);
	void HAND_Anna_WaitHW(HAND_PARAMS);

	void CONS_Anna_WaitingDinner(CONS_PARAMS);
	void HAND_Anna_WaitingDinner(HAND_PARAMS);

	void CONS_Anna_WaitingDinner2(CONS_PARAMS);
	void HAND_Anna_WaitingDinner2(HAND_PARAMS);

	void CONS_Anna_EatingDinner(CONS_PARAMS);
	void HAND_Anna_EatingDinner(HAND_PARAMS);

	void CONS_Anna_LeaveDinner(CONS_PARAMS);
	void HAND_Anna_LeaveDinner(HAND_PARAMS);

	void CONS_Anna_FreshenUp(CONS_PARAMS);
	void HAND_Anna_FreshenUp(HAND_PARAMS);

	void CONS_Anna_GoSalon(CONS_PARAMS);
	void HAND_Anna_GoSalon(HAND_PARAMS);

	void CONS_Anna_WaitAugust(CONS_PARAMS);
	void HAND_Anna_WaitAugust(HAND_PARAMS);

	void CONS_Anna_FlirtAugust(CONS_PARAMS);
	void HAND_Anna_FlirtAugust(HAND_PARAMS);

	void CONS_Anna_LeaveAugust(CONS_PARAMS);
	void HAND_Anna_LeaveAugust(HAND_PARAMS);

	void CONS_Anna_ReturnComp(CONS_PARAMS);
	void HAND_Anna_ReturnComp(HAND_PARAMS);

	void CONS_Anna_ReadyForBed(CONS_PARAMS);
	void HAND_Anna_ReadyForBed(HAND_PARAMS);

	void CONS_Anna_Asleep(CONS_PARAMS);
	void HAND_Anna_Asleep(HAND_PARAMS);

	void CONS_Anna_WakeNight(CONS_PARAMS);
	void HAND_Anna_WakeNight(HAND_PARAMS);

	void CONS_Anna_GoVassili(CONS_PARAMS);
	void HAND_Anna_GoVassili(HAND_PARAMS);

	void CONS_Anna_AtSeizure(CONS_PARAMS);
	void HAND_Anna_AtSeizure(HAND_PARAMS);

	void CONS_Anna_SpeakTatiana(CONS_PARAMS);
	void HAND_Anna_SpeakTatiana(HAND_PARAMS);

	void CONS_Anna_DoWalk1019(CONS_PARAMS);
	void HAND_Anna_DoWalk1019(HAND_PARAMS);

	void CONS_Anna_LeaveTatiana(CONS_PARAMS);
	void HAND_Anna_LeaveTatiana(HAND_PARAMS);

	void CONS_Anna_GoBackToSleep(CONS_PARAMS);
	void HAND_Anna_GoBackToSleep(HAND_PARAMS);

	void CONS_Anna_StartPart2(CONS_PARAMS);
	void HAND_Anna_StartPart2(HAND_PARAMS);

	void CONS_Anna_InPart2(CONS_PARAMS);
	void HAND_Anna_InPart2(HAND_PARAMS);

	void CONS_Anna_StartPart3(CONS_PARAMS);
	void HAND_Anna_StartPart3(HAND_PARAMS);

	void CONS_Anna_ExitComp(CONS_PARAMS);
	void HAND_Anna_ExitComp(HAND_PARAMS);

	void CONS_Anna_Practicing(CONS_PARAMS);
	void HAND_Anna_Practicing(HAND_PARAMS);

	void CONS_Anna_GoLunch(CONS_PARAMS);
	void HAND_Anna_GoLunch(HAND_PARAMS);

	void CONS_Anna_Lunch(CONS_PARAMS);
	void HAND_Anna_Lunch(HAND_PARAMS);

	void CONS_Anna_DoOtis5026J(CONS_PARAMS);
	void HAND_Anna_DoOtis5026J(HAND_PARAMS);

	void CONS_Anna_LeaveLunch(CONS_PARAMS);
	void HAND_Anna_LeaveLunch(HAND_PARAMS);

	void CONS_Anna_AfterLunch(CONS_PARAMS);
	void HAND_Anna_AfterLunch(HAND_PARAMS);

	void CONS_Anna_ReturnComp3(CONS_PARAMS);
	void HAND_Anna_ReturnComp3(HAND_PARAMS);

	void CONS_Anna_Dressing(CONS_PARAMS);
	void HAND_Anna_Dressing(HAND_PARAMS);

	void CONS_Anna_GiveMaxToCond2(CONS_PARAMS);
	void HAND_Anna_GiveMaxToCond2(HAND_PARAMS);

	void CONS_Anna_GoConcert(CONS_PARAMS);
	void HAND_Anna_GoConcert(HAND_PARAMS);

	void CONS_Anna_Concert(CONS_PARAMS);
	void HAND_Anna_Concert(HAND_PARAMS);

	void COND_Anna_LeaveConcert(CONS_PARAMS);
	void HAND_Anna_LeaveConcert(HAND_PARAMS);

	void CONS_Anna_LeaveConcertCathInComp(CONS_PARAMS);
	void HAND_Anna_LeaveConcertCathInComp(HAND_PARAMS);

	void CONS_Anna_AfterConcert(CONS_PARAMS);
	void HAND_Anna_AfterConcert(HAND_PARAMS);

	void CONS_Anna_GiveMaxBack(CONS_PARAMS);
	void HAND_Anna_GiveMaxBack(HAND_PARAMS);

	void CONS_Anna_GoBagg(CONS_PARAMS);
	void HAND_Anna_GoBagg(HAND_PARAMS);

	void CONS_Anna_InBagg(CONS_PARAMS);
	void HAND_Anna_InBagg(HAND_PARAMS);

	void CONS_Anna_DeadBagg(CONS_PARAMS);
	void HAND_Anna_DeadBagg(HAND_PARAMS);

	void CONS_Anna_BaggageFight(CONS_PARAMS);
	void HAND_Anna_BaggageFight(HAND_PARAMS);

	void CONS_Anna_PrepareVienna(CONS_PARAMS);
	void HAND_Anna_PrepareVienna(HAND_PARAMS);

	void CONS_Anna_StartPart4(CONS_PARAMS);
	void HAND_Anna_StartPart4(HAND_PARAMS);

	void CONS_Anna_Reading(CONS_PARAMS);
	void HAND_Anna_Reading(HAND_PARAMS);

	void CONS_Anna_Sulking(CONS_PARAMS);
	void HAND_Anna_Sulking(HAND_PARAMS);

	void CONS_Anna_GoSalon4(CONS_PARAMS);
	void HAND_Anna_GoSalon4(HAND_PARAMS);

	void CONS_Anna_ReturnToComp4(CONS_PARAMS);
	void HAND_Anna_ReturnToComp4(HAND_PARAMS);

	void CONS_Anna_EnterCompCathFollowsAnna(CONS_PARAMS);
	void HAND_Anna_EnterCompCathFollowsAnna(HAND_PARAMS);

	void CONS_Anna_DoWalkCathFollowsAnna(CONS_PARAMS);
	void HAND_Anna_DoWalkCathFollowsAnna(HAND_PARAMS);

	void CONS_Anna_LetDownHair(CONS_PARAMS);
	void HAND_Anna_LetDownHair(HAND_PARAMS);

	void CONS_Anna_StartPart5(CONS_PARAMS);
	void HAND_Anna_StartPart5(HAND_PARAMS);

	void CONS_Anna_TiedUp(CONS_PARAMS);
	void HAND_Anna_TiedUp(HAND_PARAMS);

	void CONS_Anna_Outside(CONS_PARAMS);
	void HAND_Anna_Outside(HAND_PARAMS);

	void CONS_Anna_ReadyToScore(CONS_PARAMS);
	void HAND_Anna_ReadyToScore(HAND_PARAMS);

	void CONS_Anna_Kidnapped(CONS_PARAMS);
	void HAND_Anna_Kidnapped(HAND_PARAMS);

	void CONS_Anna_Waiting(CONS_PARAMS);
	void HAND_Anna_Waiting(HAND_PARAMS);

	void CONS_Anna_FinalSequence(CONS_PARAMS);
	void HAND_Anna_FinalSequence(HAND_PARAMS);

	void CONS_Anna_OpenFirebird(CONS_PARAMS);
	void HAND_Anna_OpenFirebird(HAND_PARAMS);

	Common::StringArray _annaFuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"DoBlockSeqOtis",
		"DoCorrOtis",
		"FinishSeqOtis",
		"DoJoinedSeqOtis",
		"DoDialog",
		"WaitRCClear",
		"SaveGame",
		"DoWalk",
		"DoWait",
		"PracticeMusic",
		"DoComplexSeqOtis",
		"DoWaitReal",
		"CompLogic",
		"Birth",
		"DoWalkP1",
		"DiningLogic",
		"FleeTyler",
		"WaitDinner",
		"GoDinner",
		"WaitHW",
		"WaitingDinner",
		"WaitingDinner2",
		"EatingDinner",
		"LeaveDinner",
		"FreshenUp",
		"GoSalon",
		"WaitAugust",
		"FlirtAugust",
		"LeaveAugust",
		"ReturnComp",
		"ReadyForBed",
		"Asleep",
		"WakeNight",
		"GoVassili",
		"AtSeizure",
		"SpeakTatiana",
		"DoWalk1019",
		"LeaveTatiana",
		"GoBackToSleep",
		"StartPart2",
		"InPart2",
		"StartPart3",
		"ExitComp",
		"Practicing",
		"GoLunch",
		"Lunch",
		"DoOtis5026J",
		"LeaveLunch",
		"AfterLunch",
		"ReturnComp3",
		"Dressing",
		"GiveMaxToCond2",
		"GoConcert",
		"Concert",
		"LeaveConcert",
		"LeaveConcertCathInComp",
		"AfterConcert",
		"GiveMaxBack",
		"GoBagg",
		"InBagg",
		"DeadBagg",
		"BaggageFight",
		"PrepareVienna",
		"StartPart4",
		"Reading",
		"Sulking",
		"GoSalon4",
		"ReturnToComp4",
		"EnterCompCathFollowsAnna",
		"DoWalkCathFollowsAnna",
		"LetDownHair",
		"StartPart5",
		"TiedUp",
		"Outside",
		"ReadyToScore",
		"Kidnapped",
		"Waiting",
		"FinalSequence",
		"OpenFirebird"
	};

	// AUGUST EVENTS

	void AugustCall(CALL_PARAMS);

	void CONS_August_DebugWalks(CONS_PARAMS);
	void HAND_August_DebugWalks(HAND_PARAMS);

	void CONS_August_DoWait(CONS_PARAMS);
	void HAND_August_DoWait(HAND_PARAMS);

	void CONS_August_DoSeqOtis(CONS_PARAMS);
	void HAND_August_DoSeqOtis(HAND_PARAMS);

	void CONS_August_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_August_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_August_DoCorrOtis(CONS_PARAMS);
	void HAND_August_DoCorrOtis(HAND_PARAMS);

	void CONS_August_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_August_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_August_DoBriefCorrOtis(CONS_PARAMS);
	void HAND_August_DoBriefCorrOtis(HAND_PARAMS);

	void CONS_August_FinishSeqOtis(CONS_PARAMS);
	void HAND_August_FinishSeqOtis(HAND_PARAMS);

	void CONS_August_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_August_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_August_FinishJoinedSeqOtis(CONS_PARAMS);
	void HAND_August_FinishJoinedSeqOtis(HAND_PARAMS);

	void CONS_August_DoComplexSeqOtis(CONS_PARAMS);
	void HAND_August_DoComplexSeqOtis(HAND_PARAMS);

	void CONS_August_DoDialog(CONS_PARAMS);
	void HAND_August_DoDialog(HAND_PARAMS);

	void CONS_August_DoDialogFullVol(CONS_PARAMS);
	void HAND_August_DoDialogFullVol(HAND_PARAMS);

	void CONS_August_WaitRCClear(CONS_PARAMS);
	void HAND_August_WaitRCClear(HAND_PARAMS);

	void CONS_August_SaveGame(CONS_PARAMS);
	void HAND_August_SaveGame(HAND_PARAMS);

	void CONS_August_DoWalk(CONS_PARAMS);
	void HAND_August_DoWalk(HAND_PARAMS);

	void CONS_August_LookingForCath(CONS_PARAMS);
	void HAND_August_LookingForCath(HAND_PARAMS);

	void CONS_August_DoWalkSearchingForCath(CONS_PARAMS);
	void HAND_August_DoWalkSearchingForCath(HAND_PARAMS);

	void CONS_August_EnterComp(CONS_PARAMS);
	void HAND_August_EnterComp(HAND_PARAMS);

	void CONS_August_ExitComp(CONS_PARAMS);
	void HAND_August_ExitComp(HAND_PARAMS);

	void CONS_August_CompLogic(CONS_PARAMS);
	void HAND_August_CompLogic(HAND_PARAMS);

	void CONS_August_Birth(CONS_PARAMS);
	void HAND_August_Birth(HAND_PARAMS);

	void CONS_August_KnockTyler(CONS_PARAMS);
	void HAND_August_KnockTyler(HAND_PARAMS);

	void CONS_August_DoNIS1006(CONS_PARAMS);
	void HAND_August_DoNIS1006(HAND_PARAMS);

	void CONS_August_WaitTyler(CONS_PARAMS);
	void HAND_August_WaitTyler(HAND_PARAMS);

	void CONS_August_SeekTyler(CONS_PARAMS);
	void HAND_August_SeekTyler(HAND_PARAMS);

	void CONS_August_GotoDinner(CONS_PARAMS);
	void HAND_August_GotoDinner(HAND_PARAMS);

	void CONS_August_OrderDinner(CONS_PARAMS);
	void HAND_August_OrderDinner(HAND_PARAMS);

	void CONS_August_EatingDinner(CONS_PARAMS);
	void HAND_August_EatingDinner(HAND_PARAMS);

	void CONS_August_GreetAnna(CONS_PARAMS);
	void HAND_August_GreetAnna(HAND_PARAMS);

	void CONS_August_ReturnFromDinner(CONS_PARAMS);
	void HAND_August_ReturnFromDinner(HAND_PARAMS);

	void CONS_August_GoSalon(CONS_PARAMS);
	void HAND_August_GoSalon(HAND_PARAMS);

	void CONS_August_BackFromSalon(CONS_PARAMS);
	void HAND_August_BackFromSalon(HAND_PARAMS);

	void CONS_August_Asleep(CONS_PARAMS);
	void HAND_August_Asleep(HAND_PARAMS);

	void CONS_August_StartPart2(CONS_PARAMS);
	void HAND_August_StartPart2(HAND_PARAMS);

	void CONS_August_AtBreakfast(CONS_PARAMS);
	void HAND_August_AtBreakfast(HAND_PARAMS);

	void CONS_August_OpenComp(CONS_PARAMS);
	void HAND_August_OpenComp(HAND_PARAMS);

	void CONS_August_InSalon(CONS_PARAMS);
	void HAND_August_InSalon(HAND_PARAMS);

	void CONS_August_EndPart2(CONS_PARAMS);
	void HAND_August_EndPart2(HAND_PARAMS);

	void CONS_August_StartPart3(CONS_PARAMS);
	void HAND_August_StartPart3(HAND_PARAMS);

	void CONS_August_DoWalkP3A(CONS_PARAMS);
	void HAND_August_DoWalkP3A(HAND_PARAMS);

	void CONS_August_DoWalkP3B(CONS_PARAMS);
	void HAND_August_DoWalkP3B(HAND_PARAMS);

	void CONS_August_GoLunch(CONS_PARAMS);
	void HAND_August_GoLunch(HAND_PARAMS);

	void CONS_August_ReturnLunch(CONS_PARAMS);
	void HAND_August_ReturnLunch(HAND_PARAMS);

	void CONS_August_AfterLunch(CONS_PARAMS);
	void HAND_August_AfterLunch(HAND_PARAMS);

	void CONS_August_Reading(CONS_PARAMS);
	void HAND_August_Reading(HAND_PARAMS);

	void CONS_August_BathroomTrip(CONS_PARAMS);
	void HAND_August_BathroomTrip(HAND_PARAMS);

	void CONS_August_Dressing(CONS_PARAMS);
	void HAND_August_Dressing(HAND_PARAMS);

	void CONS_August_GoConcert(CONS_PARAMS);
	void HAND_August_GoConcert(HAND_PARAMS);

	void CONS_August_Concert(CONS_PARAMS);
	void HAND_August_Concert(HAND_PARAMS);

	void CONS_August_StalkAnna(CONS_PARAMS);
	void HAND_August_StalkAnna(HAND_PARAMS);

	void CONS_August_AfterConcert(CONS_PARAMS);
	void HAND_August_AfterConcert(HAND_PARAMS);

	void CONS_August_Satisfied(CONS_PARAMS);
	void HAND_August_Satisfied(HAND_PARAMS);

	void CONS_August_InSalon3(CONS_PARAMS);
	void HAND_August_InSalon3(HAND_PARAMS);

	void CONS_August_LeaveSalon(CONS_PARAMS);
	void HAND_August_LeaveSalon(HAND_PARAMS);

	void CONS_August_BeforeVienna(CONS_PARAMS);
	void HAND_August_BeforeVienna(HAND_PARAMS);

	void CONS_August_StartPart4(CONS_PARAMS);
	void HAND_August_StartPart4(HAND_PARAMS);

	void CONS_August_GoDinner(CONS_PARAMS);
	void HAND_August_GoDinner(HAND_PARAMS);

	void CONS_August_WaitingAnna(CONS_PARAMS);
	void HAND_August_WaitingAnna(HAND_PARAMS);

	void CONS_August_Dinner(CONS_PARAMS);
	void HAND_August_Dinner(HAND_PARAMS);

	void CONS_August_ReturnComp4(CONS_PARAMS);
	void HAND_August_ReturnComp4(HAND_PARAMS);

	void CONS_August_GoSalon4(CONS_PARAMS);
	void HAND_August_GoSalon4(HAND_PARAMS);

	void CONS_August_Drinking(CONS_PARAMS);
	void HAND_August_Drinking(HAND_PARAMS);

	void CONS_August_Drunk(CONS_PARAMS);
	void HAND_August_Drunk(HAND_PARAMS);

	void CONS_August_Asleep4(CONS_PARAMS);
	void HAND_August_Asleep4(HAND_PARAMS);

	void CONS_August_StartPart5(CONS_PARAMS);
	void HAND_August_StartPart5(HAND_PARAMS);

	void CONS_August_Prisoner(CONS_PARAMS);
	void HAND_August_Prisoner(HAND_PARAMS);

	void CONS_August_Hiding(CONS_PARAMS);
	void HAND_August_Hiding(HAND_PARAMS);

	void CONS_August_CutLoose(CONS_PARAMS);
	void HAND_August_CutLoose(HAND_PARAMS);

	void CONS_August_Disappear(CONS_PARAMS);
	void HAND_August_Disappear(HAND_PARAMS);

	Common::StringArray _augustFuncNames = {
		"DebugWalks",
		"DoWait",
		"DoSeqOtis",
		"DoBlockSeqOtis",
		"DoCorrOtis",
		"DoEnterCorrOtis",
		"DoBriefCorrOtis",
		"FinishSeqOtis",
		"DoJoinedSeqOtis",
		"FinishJoinedSeqOtis",
		"DoComplexSeqOtis",
		"DoDialog",
		"DoDialogFullVol",
		"WaitRCClear",
		"SaveGame",
		"DoWalk",
		"LookingForCath",
		"DoWalkSearchingForCath",
		"EnterComp",
		"ExitComp",
		"CompLogic",
		"Birth",
		"KnockTyler",
		"DoNIS1006",
		"WaitTyler",
		"SeekTyler",
		"GotoDinner",
		"OrderDinner",
		"EatingDinner",
		"GreetAnna",
		"ReturnFromDinner",
		"GoSalon",
		"BackFromSalon",
		"Asleep",
		"StartPart2",
		"AtBreakfast",
		"OpenComp",
		"InSalon",
		"EndPart2",
		"StartPart3",
		"DoWalkP3A",
		"DoWalkP3B",
		"GoLunch",
		"ReturnLunch",
		"AfterLunch",
		"Reading",
		"BathroomTrip",
		"Dressing",
		"GoConcert",
		"Concert",
		"StalkAnna",
		"AfterConcert",
		"Satisfied",
		"InSalon3",
		"LeaveSalon",
		"BeforeVienna",
		"StartPart4",
		"GoDinner",
		"WaitingAnna",
		"Dinner",
		"ReturnComp4",
		"GoSalon4",
		"Drinking",
		"Drunk",
		"Asleep4",
		"StartPart5",
		"Prisoner",
		"Hiding",
		"CutLoose",
		"Disappear"
	};

	// CLERK EVENTS

	void ClerkCall(CALL_PARAMS);

	void CONS_Clerk_SaveGame(CONS_PARAMS);
	void HAND_Clerk_SaveGame(HAND_PARAMS);

	void CONS_Clerk_Birth(CONS_PARAMS);
	void HAND_Clerk_Birth(HAND_PARAMS);

	void CONS_Clerk_StartPart2(CONS_PARAMS);
	void HAND_Clerk_StartPart2(HAND_PARAMS);

	void CONS_Clerk_StartPart3(CONS_PARAMS);
	void HAND_Clerk_StartPart3(HAND_PARAMS);

	void CONS_Clerk_StartPart4(CONS_PARAMS);
	void HAND_Clerk_StartPart4(HAND_PARAMS);

	void CONS_Clerk_StartPart5(CONS_PARAMS);
	void HAND_Clerk_StartPart5(HAND_PARAMS);

	void CONS_Clerk_DoHaremKnock(CONS_PARAMS);
	void HAND_Clerk_DoHaremKnock(HAND_PARAMS);

	void CONS_Clerk_Processing(CONS_PARAMS);
	void HAND_Clerk_Processing(HAND_PARAMS);

	Common::StringArray _clerkFuncNames = {
		"SaveGame",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5",
		"DoHaremKnock",
		"Processing"
	};


	// COND1 EVENTS

	void Cond1Call(CALL_PARAMS);

	void CONS_Cond1_DebugWalks(CONS_PARAMS);
	void HAND_Cond1_DebugWalks(HAND_PARAMS);

	void CONS_Cond1_DoSeqOtis(CONS_PARAMS);
	void HAND_Cond1_DoSeqOtis(HAND_PARAMS);

	void CONS_Cond1_DoCorrOtis(CONS_PARAMS);
	void HAND_Cond1_DoCorrOtis(HAND_PARAMS);

	void CONS_Cond1_DoBriefCorrOtis(CONS_PARAMS);
	void HAND_Cond1_DoBriefCorrOtis(HAND_PARAMS);

	void CONS_Cond1_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_Cond1_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_Cond1_FinishSeqOtis(CONS_PARAMS);
	void HAND_Cond1_FinishSeqOtis(HAND_PARAMS);

	void CONS_Cond1_DoDialog(CONS_PARAMS);
	void HAND_Cond1_DoDialog(HAND_PARAMS);

	void CONS_Cond1_DoDialogFullVol(CONS_PARAMS);
	void HAND_Cond1_DoDialogFullVol(HAND_PARAMS);

	void CONS_Cond1_SaveGame(CONS_PARAMS);
	void HAND_Cond1_SaveGame(HAND_PARAMS);

	void CONS_Cond1_DoWalk(CONS_PARAMS);
	void HAND_Cond1_DoWalk(HAND_PARAMS);

	void CONS_Cond1_DoWait(CONS_PARAMS);
	void HAND_Cond1_DoWait(HAND_PARAMS);

	void CONS_Cond1_StandAsideDialog(CONS_PARAMS);
	void HAND_Cond1_StandAsideDialog(HAND_PARAMS);

	void CONS_Cond1_Passing(CONS_PARAMS);
	void HAND_Cond1_Passing(HAND_PARAMS);

	void CONS_Cond1_Listen(CONS_PARAMS);
	void HAND_Cond1_Listen(HAND_PARAMS);

	void CONS_Cond1_AlexeiLockUnlockMyComp(CONS_PARAMS);
	void HAND_Cond1_AlexeiLockUnlockMyComp(HAND_PARAMS);

	void CONS_Cond1_AugustLockUnlockMyComp(CONS_PARAMS);
	void HAND_Cond1_AugustLockUnlockMyComp(HAND_PARAMS);

	void CONS_Cond1_SitDown(CONS_PARAMS);
	void HAND_Cond1_SitDown(HAND_PARAMS);

	void CONS_Cond1_SitDownFast(CONS_PARAMS);
	void HAND_Cond1_SitDownFast(HAND_PARAMS);

	void CONS_Cond1_GetUp(CONS_PARAMS);
	void HAND_Cond1_GetUp(HAND_PARAMS);

	void CONS_Cond1_GetUpListen(CONS_PARAMS);
	void HAND_Cond1_GetUpListen(HAND_PARAMS);

	void CONS_Cond1_MakeBed(CONS_PARAMS);
	void HAND_Cond1_MakeBed(HAND_PARAMS);

	void CONS_Cond1_MakeBedMahmud(CONS_PARAMS);
	void HAND_Cond1_MakeBedMahmud(HAND_PARAMS);

	void CONS_Cond1_MakeBedMahmud2(CONS_PARAMS);
	void HAND_Cond1_MakeBedMahmud2(HAND_PARAMS);

	void CONS_Cond1_MakeBedAugust(CONS_PARAMS);
	void HAND_Cond1_MakeBedAugust(HAND_PARAMS);

	void CONS_Cond1_MakeBedAlexei(CONS_PARAMS);
	void HAND_Cond1_MakeBedAlexei(HAND_PARAMS);

	void CONS_Cond1_SubMakeBed(CONS_PARAMS);
	void HAND_Cond1_SubMakeBed(HAND_PARAMS);

	void CONS_Cond1_EnterTyler(CONS_PARAMS);
	void HAND_Cond1_EnterTyler(HAND_PARAMS);

	void CONS_Cond1_VisitCond2(CONS_PARAMS);
	void HAND_Cond1_VisitCond2(HAND_PARAMS);

	void CONS_Cond1_VisitCond2Double(CONS_PARAMS);
	void HAND_Cond1_VisitCond2Double(HAND_PARAMS);

	void CONS_Cond1_CathBuzzing(CONS_PARAMS);
	void HAND_Cond1_CathBuzzing(HAND_PARAMS);

	void CONS_Cond1_CathRattling(CONS_PARAMS);
	void HAND_Cond1_CathRattling(HAND_PARAMS);

	void CONS_Cond1_BathroomTrip(CONS_PARAMS);
	void HAND_Cond1_BathroomTrip(HAND_PARAMS);

	void CONS_Cond1_DoPending(CONS_PARAMS);
	void HAND_Cond1_DoPending(HAND_PARAMS);

	void CONS_Cond1_Birth(CONS_PARAMS);
	void HAND_Cond1_Birth(HAND_PARAMS);

	void CONS_Cond1_DeliverAugustMessage(CONS_PARAMS);
	void HAND_Cond1_DeliverAugustMessage(HAND_PARAMS);

	void CONS_Cond1_DeliverKronosMessage(CONS_PARAMS);
	void HAND_Cond1_DeliverKronosMessage(HAND_PARAMS);

	void CONS_Cond1_VisitCond2Special(CONS_PARAMS);
	void HAND_Cond1_VisitCond2Special(HAND_PARAMS);

	void CONS_Cond1_MakeBedTyler(CONS_PARAMS);
	void HAND_Cond1_MakeBedTyler(HAND_PARAMS);

	void CONS_Cond1_MakeAllBeds(CONS_PARAMS);
	void HAND_Cond1_MakeAllBeds(HAND_PARAMS);

	void CONS_Cond1_VisitKahina(CONS_PARAMS);
	void HAND_Cond1_VisitKahina(HAND_PARAMS);

	void CONS_Cond1_StartPart1(CONS_PARAMS);
	void HAND_Cond1_StartPart1(HAND_PARAMS);

	void CONS_Cond1_Sitting(CONS_PARAMS);
	void HAND_Cond1_Sitting(HAND_PARAMS);

	void CONS_Cond1_StartPart2(CONS_PARAMS);
	void HAND_Cond1_StartPart2(HAND_PARAMS);

	void CONS_Cond1_SittingDay(CONS_PARAMS);
	void HAND_Cond1_SittingDay(HAND_PARAMS);

	void CONS_Cond1_StartPart3(CONS_PARAMS);
	void HAND_Cond1_StartPart3(HAND_PARAMS);

	void CONS_Cond1_OnDuty(CONS_PARAMS);
	void HAND_Cond1_OnDuty(HAND_PARAMS);

	void CONS_Cond1_StartPart4(CONS_PARAMS);
	void HAND_Cond1_StartPart4(HAND_PARAMS);

	void CONS_Cond1_OnDuty4(CONS_PARAMS);
	void HAND_Cond1_OnDuty4(HAND_PARAMS);

	void CONS_Cond1_MakeBeds4(CONS_PARAMS);
	void HAND_Cond1_MakeBeds4(HAND_PARAMS);

	void CONS_Cond1_StartPart5(CONS_PARAMS);
	void HAND_Cond1_StartPart5(HAND_PARAMS);

	void CONS_Cond1_Prisoner(CONS_PARAMS);
	void HAND_Cond1_Prisoner(HAND_PARAMS);

	void CONS_Cond1_ComfortMadame(CONS_PARAMS);
	void HAND_Cond1_ComfortMadame(HAND_PARAMS);

	void CONS_Cond1_HideOut(CONS_PARAMS);
	void HAND_Cond1_HideOut(HAND_PARAMS);

	void CONS_Cond1_Disappear(CONS_PARAMS);
	void HAND_Cond1_Disappear(HAND_PARAMS);

	Common::StringArray _cond1FuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"DoCorrOtis",
		"DoBriefCorrOtis",
		"DoEnterCorrOtis",
		"FinishSeqOtis",
		"DoDialog",
		"DoDialogFullVol",
		"SaveGame",
		"DoWalk",
		"DoWait",
		"StandAsideDialog",
		"Passing",
		"Listen",
		"AlexeiLockUnlockMyComp",
		"AugustLockUnlockMyComp",
		"SitDown",
		"SitDownFast",
		"GetUp",
		"GetUpListen",
		"MakeBed",
		"MakeBedMahmud",
		"MakeBedMahmud2",
		"MakeBedAugust",
		"MakeBedAlexei",
		"SubMakeBed",
		"EnterTyler",
		"VisitCond2",
		"VisitCond2Double",
		"CathBuzzing",
		"CathRattling",
		"BathroomTrip",
		"DoPending",
		"Birth",
		"DeliverAugustMessage",
		"DeliverKronosMessage",
		"VisitCond2Special",
		"MakeBedTyler",
		"MakeAllBeds",
		"VisitKahina",
		"StartPart1",
		"Sitting",
		"StartPart2",
		"SittingDay",
		"StartPart3",
		"OnDuty",
		"StartPart4",
		"OnDuty4",
		"MakeBeds4",
		"StartPart5",
		"Prisoner",
		"ComfortMadame",
		"HideOut",
		"Disappear"
	};

	// COND2 EVENTS

	void Cond2Call(CALL_PARAMS);

	void CONS_Cond2_DebugWalks(CONS_PARAMS);
	void HAND_Cond2_DebugWalks(HAND_PARAMS);

	void CONS_Cond2_DoSeqOtis(CONS_PARAMS);
	void HAND_Cond2_DoSeqOtis(HAND_PARAMS);

	void CONS_Cond2_DoCorrOtis(CONS_PARAMS);
	void HAND_Cond2_DoCorrOtis(HAND_PARAMS);

	void CONS_Cond2_FinishSeqOtis(CONS_PARAMS);
	void HAND_Cond2_FinishSeqOtis(HAND_PARAMS);

	void CONS_Cond2_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_Cond2_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_Cond2_DoDialog(CONS_PARAMS);
	void HAND_Cond2_DoDialog(HAND_PARAMS);

	void CONS_Cond2_DoDialogFullVol(CONS_PARAMS);
	void HAND_Cond2_DoDialogFullVol(HAND_PARAMS);

	void CONS_Cond2_SaveGame(CONS_PARAMS);
	void HAND_Cond2_SaveGame(HAND_PARAMS);

	void CONS_Cond2_DoWalk(CONS_PARAMS);
	void HAND_Cond2_DoWalk(HAND_PARAMS);

	void CONS_Cond2_DoWait(CONS_PARAMS);
	void HAND_Cond2_DoWait(HAND_PARAMS);

	void CONS_Cond2_DoWaitReal(CONS_PARAMS);
	void HAND_Cond2_DoWaitReal(HAND_PARAMS);

	void CONS_Cond2_StandAsideDialog(CONS_PARAMS);
	void HAND_Cond2_StandAsideDialog(HAND_PARAMS);

	void CONS_Cond2_Passing(CONS_PARAMS);
	void HAND_Cond2_Passing(HAND_PARAMS);

	void CONS_Cond2_Listen(CONS_PARAMS);
	void HAND_Cond2_Listen(HAND_PARAMS);

	void CONS_Cond2_TatianaLockUnlockMyComp(CONS_PARAMS);
	void HAND_Cond2_TatianaLockUnlockMyComp(HAND_PARAMS);

	void CONS_Cond2_GetUp(CONS_PARAMS);
	void HAND_Cond2_GetUp(HAND_PARAMS);

	void CONS_Cond2_GetUpListen(CONS_PARAMS);
	void HAND_Cond2_GetUpListen(HAND_PARAMS);

	void CONS_Cond2_SitDown(CONS_PARAMS);
	void HAND_Cond2_SitDown(HAND_PARAMS);

	void CONS_Cond2_SitDownFast(CONS_PARAMS);
	void HAND_Cond2_SitDownFast(HAND_PARAMS);

	void CONS_Cond2_MakeBed(CONS_PARAMS);
	void HAND_Cond2_MakeBed(HAND_PARAMS);

	void CONS_Cond2_MakeBedIvo(CONS_PARAMS);
	void HAND_Cond2_MakeBedIvo(HAND_PARAMS);

	void CONS_Cond2_MakeBedMilos(CONS_PARAMS);
	void HAND_Cond2_MakeBedMilos(HAND_PARAMS);

	void CONS_Cond2_TryMakeAnna(CONS_PARAMS);
	void HAND_Cond2_TryMakeAnna(HAND_PARAMS);

	void CONS_Cond2_MakeBedAnna(CONS_PARAMS);
	void HAND_Cond2_MakeBedAnna(HAND_PARAMS);

	void CONS_Cond2_MakeBedRebecca(CONS_PARAMS);
	void HAND_Cond2_MakeBedRebecca(HAND_PARAMS);

	void CONS_Cond2_MakeBedMadame(CONS_PARAMS);
	void HAND_Cond2_MakeBedMadame(HAND_PARAMS);

	void CONS_Cond2_MakeBedMonsieur(CONS_PARAMS);
	void HAND_Cond2_MakeBedMonsieur(HAND_PARAMS);

	void CONS_Cond2_MakeBedTatiana(CONS_PARAMS);
	void HAND_Cond2_MakeBedTatiana(HAND_PARAMS);

	void CONS_Cond2_MakeBedVassili(CONS_PARAMS);
	void HAND_Cond2_MakeBedVassili(HAND_PARAMS);

	void CONS_Cond2_CathBuzzing(CONS_PARAMS);
	void HAND_Cond2_CathBuzzing(HAND_PARAMS);

	void CONS_Cond2_CathRattling(CONS_PARAMS);
	void HAND_Cond2_CathRattling(HAND_PARAMS);

	void CONS_Cond2_BathroomTrip(CONS_PARAMS);
	void HAND_Cond2_BathroomTrip(HAND_PARAMS);

	void CONS_Cond2_DoPending(CONS_PARAMS);
	void HAND_Cond2_DoPending(HAND_PARAMS);

	void CONS_Cond2_TakeMaxBack(CONS_PARAMS);
	void HAND_Cond2_TakeMaxBack(HAND_PARAMS);

	void CONS_Cond2_ReturnMax(CONS_PARAMS);
	void HAND_Cond2_ReturnMax(HAND_PARAMS);

	void CONS_Cond2_Birth(CONS_PARAMS);
	void HAND_Cond2_Birth(HAND_PARAMS);

	void CONS_Cond2_RespondVassili(CONS_PARAMS);
	void HAND_Cond2_RespondVassili(HAND_PARAMS);

	void CONS_Cond2_AtSeizure(CONS_PARAMS);
	void HAND_Cond2_AtSeizure(HAND_PARAMS);

	void CONS_Cond2_AfterPolice(CONS_PARAMS);
	void HAND_Cond2_AfterPolice(HAND_PARAMS);

	void CONS_Cond2_Sitting(CONS_PARAMS);
	void HAND_Cond2_Sitting(HAND_PARAMS);

	void CONS_Cond2_MakeAllBeds(CONS_PARAMS);
	void HAND_Cond2_MakeAllBeds(HAND_PARAMS);

	void CONS_Cond2_StartPart2(CONS_PARAMS);
	void HAND_Cond2_StartPart2(HAND_PARAMS);

	void CONS_Cond2_SittingDay(CONS_PARAMS);
	void HAND_Cond2_SittingDay(HAND_PARAMS);

	void CONS_Cond2_StartPart3(CONS_PARAMS);
	void HAND_Cond2_StartPart3(HAND_PARAMS);

	void CONS_Cond2_SittingDay3(CONS_PARAMS);
	void HAND_Cond2_SittingDay3(HAND_PARAMS);

	void CONS_Cond2_AnnaTakeMax(CONS_PARAMS);
	void HAND_Cond2_AnnaTakeMax(HAND_PARAMS);

	void CONS_Cond2_AnnaLockMe(CONS_PARAMS);
	void HAND_Cond2_AnnaLockMe(HAND_PARAMS);

	void CONS_Cond2_AnnaUnlockMe(CONS_PARAMS);
	void HAND_Cond2_AnnaUnlockMe(HAND_PARAMS);

	void CONS_Cond2_InviteTatiana(CONS_PARAMS);
	void HAND_Cond2_InviteTatiana(HAND_PARAMS);

	void CONS_Cond2_InviteRebecca(CONS_PARAMS);
	void HAND_Cond2_InviteRebecca(HAND_PARAMS);

	void CONS_Cond2_TatianaComeHere(CONS_PARAMS);
	void HAND_Cond2_TatianaComeHere(HAND_PARAMS);

	void CONS_Cond2_StartPart4(CONS_PARAMS);
	void HAND_Cond2_StartPart4(HAND_PARAMS);

	void CONS_Cond2_OnDuty4(CONS_PARAMS);
	void HAND_Cond2_OnDuty4(HAND_PARAMS);

	void CONS_Cond2_WaitBombDefused(CONS_PARAMS);
	void HAND_Cond2_WaitBombDefused(HAND_PARAMS);

	void CONS_Cond2_ServiceAnna(CONS_PARAMS);
	void HAND_Cond2_ServiceAnna(HAND_PARAMS);

	void CONS_Cond2_MakeAllBeds4(CONS_PARAMS);
	void HAND_Cond2_MakeAllBeds4(HAND_PARAMS);

	void CONS_Cond2_StartPart5(CONS_PARAMS);
	void HAND_Cond2_StartPart5(HAND_PARAMS);

	void CONS_Cond2_Prisoner(CONS_PARAMS);
	void HAND_Cond2_Prisoner(HAND_PARAMS);

	void CONS_Cond2_ReturnSeat(CONS_PARAMS);
	void HAND_Cond2_ReturnSeat(HAND_PARAMS);

	void CONS_Cond2_Waiting5(CONS_PARAMS);
	void HAND_Cond2_Waiting5(HAND_PARAMS);

	void CONS_Cond2_MakeRounds(CONS_PARAMS);
	void HAND_Cond2_MakeRounds(HAND_PARAMS);

	void CONS_Cond2_InCompH(CONS_PARAMS);
	void HAND_Cond2_InCompH(HAND_PARAMS);

	void CONS_Cond2_Disappear(CONS_PARAMS);
	void HAND_Cond2_Disappear(HAND_PARAMS);

	Common::StringArray _cond2FuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"DoCorrOtis",
		"FinishSeqOtis",
		"DoEnterCorrOtis",
		"DoDialog",
		"DoDialogFullVol",
		"SaveGame",
		"DoWalk",
		"DoWait",
		"DoWaitReal",
		"StandAsideDialog",
		"Passing",
		"Listen",
		"TatianaLockUnlockMyComp",
		"GetUp",
		"GetUpListen",
		"SitDown",
		"SitDownFast",
		"MakeBed",
		"MakeBedIvo",
		"MakeBedMilos",
		"TryMakeAnna",
		"MakeBedAnna",
		"MakeBedRebecca",
		"MakeBedMadame",
		"MakeBedMonsieur",
		"MakeBedTatiana",
		"MakeBedVassili",
		"CathBuzzing",
		"CathRattling",
		"BathroomTrip",
		"DoPending",
		"TakeMaxBack",
		"ReturnMax",
		"Birth",
		"RespondVassili",
		"AtSeizure",
		"AfterPolice",
		"Sitting",
		"MakeAllBeds",
		"StartPart2",
		"SittingDay",
		"StartPart3",
		"SittingDay3",
		"AnnaTakeMax",
		"AnnaLockMe",
		"AnnaUnlockMe",
		"InviteTatiana",
		"InviteRebecca",
		"TatianaComeHere",
		"StartPart4",
		"OnDuty4",
		"WaitBombDefused",
		"ServiceAnna",
		"MakeAllBeds4",
		"StartPart5",
		"Prisoner",
		"ReturnSeat",
		"Waiting5",
		"MakeRounds",
		"InCompH",
		"Disappear"
	};

	// COOK EVENTS

	void CookCall(CALL_PARAMS);

	void CONS_Cook_DoSeqOtis(CONS_PARAMS);
	void HAND_Cook_DoSeqOtis(HAND_PARAMS);

	void CONS_Cook_DoDialog(CONS_PARAMS);
	void HAND_Cook_DoDialog(HAND_PARAMS);

	void CONS_Cook_UptrainVersion(CONS_PARAMS);
	void HAND_Cook_UptrainVersion(HAND_PARAMS);

	void CONS_Cook_DowntrainVersion(CONS_PARAMS);
	void HAND_Cook_DowntrainVersion(HAND_PARAMS);

	void CONS_Cook_Birth(CONS_PARAMS);
	void HAND_Cook_Birth(HAND_PARAMS);

	void CONS_Cook_InKitchenDinner(CONS_PARAMS);
	void HAND_Cook_InKitchenDinner(HAND_PARAMS);

	void CONS_Cook_LockUp(CONS_PARAMS);
	void HAND_Cook_LockUp(HAND_PARAMS);

	void CONS_Cook_StartPart2(CONS_PARAMS);
	void HAND_Cook_StartPart2(HAND_PARAMS);

	void CONS_Cook_InKitchenBreakfast(CONS_PARAMS);
	void HAND_Cook_InKitchenBreakfast(HAND_PARAMS);

	void CONS_Cook_StartPart3(CONS_PARAMS);
	void HAND_Cook_StartPart3(HAND_PARAMS);

	void CONS_Cook_InKitchenLunch(CONS_PARAMS);
	void HAND_Cook_InKitchenLunch(HAND_PARAMS);

	void CONS_Cook_StartPart4(CONS_PARAMS);
	void HAND_Cook_StartPart4(HAND_PARAMS);

	void CONS_Cook_InKitchenDinner2(CONS_PARAMS);
	void HAND_Cook_InKitchenDinner2(HAND_PARAMS);

	void CONS_Cook_StartPart5(CONS_PARAMS);
	void HAND_Cook_StartPart5(HAND_PARAMS);

	Common::StringArray _cookFuncNames = {
		"DoSeqOtis",
		"DoDialog",
		"UptrainVersion",
		"DowntrainVersion",
		"Birth",
		"InKitchenDinner",
		"LockUp",
		"StartPart2",
		"InKitchenBreakfast",
		"StartPart3",
		"InKitchenLunch",
		"StartPart4",
		"InKitchenDinner2",
		"StartPart5"
	};

	// FRANCOIS EVENTS

	void FrancoisCall(CALL_PARAMS);

	void CONS_Francois_DebugWalks(CONS_PARAMS);
	void HAND_Francois_DebugWalks(HAND_PARAMS);

	void CONS_Francois_DoWait(CONS_PARAMS);
	void HAND_Francois_DoWait(HAND_PARAMS);

	void CONS_Francois_DoSeqOtis(CONS_PARAMS);
	void HAND_Francois_DoSeqOtis(HAND_PARAMS);

	void CONS_Francois_DoCorrOtis(CONS_PARAMS);
	void HAND_Francois_DoCorrOtis(HAND_PARAMS);

	void CONS_Francois_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_Francois_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_Francois_DoDialog(CONS_PARAMS);
	void HAND_Francois_DoDialog(HAND_PARAMS);

	void CONS_Francois_SaveGame(CONS_PARAMS);
	void HAND_Francois_SaveGame(HAND_PARAMS);

	void CONS_Francois_DoWalk(CONS_PARAMS);
	void HAND_Francois_DoWalk(HAND_PARAMS);

	void CONS_Francois_ExitComp(CONS_PARAMS);
	void HAND_Francois_ExitComp(HAND_PARAMS);

	void CONS_Francois_EnterComp(CONS_PARAMS);
	void HAND_Francois_EnterComp(HAND_PARAMS);

	void CONS_Francois_Rampage(CONS_PARAMS);
	void HAND_Francois_Rampage(HAND_PARAMS);

	void CONS_Francois_TakeWalk(CONS_PARAMS);
	void HAND_Francois_TakeWalk(HAND_PARAMS);

	void CONS_Francois_HaremVisit(CONS_PARAMS);
	void HAND_Francois_HaremVisit(HAND_PARAMS);

	void CONS_Francois_ChaseBeetle(CONS_PARAMS);
	void HAND_Francois_ChaseBeetle(HAND_PARAMS);

	void CONS_Francois_FindCath(CONS_PARAMS);
	void HAND_Francois_FindCath(HAND_PARAMS);

	void CONS_Francois_LetsGo(CONS_PARAMS);
	void HAND_Francois_LetsGo(HAND_PARAMS);

	void CONS_Francois_Birth(CONS_PARAMS);
	void HAND_Francois_Birth(HAND_PARAMS);

	void CONS_Francois_StartPart1(CONS_PARAMS);
	void HAND_Francois_StartPart1(HAND_PARAMS);

	void CONS_Francois_InComp(CONS_PARAMS);
	void HAND_Francois_InComp(HAND_PARAMS);

	void CONS_Francois_Asleep(CONS_PARAMS);
	void HAND_Francois_Asleep(HAND_PARAMS);

	void CONS_Francois_StartPart2(CONS_PARAMS);
	void HAND_Francois_StartPart2(HAND_PARAMS);

	void CONS_Francois_AtBreakfast(CONS_PARAMS);
	void HAND_Francois_AtBreakfast(HAND_PARAMS);

	void CONS_Francois_WithMama(CONS_PARAMS);
	void HAND_Francois_WithMama(HAND_PARAMS);

	void CONS_Francois_StartPart3(CONS_PARAMS);
	void HAND_Francois_StartPart3(HAND_PARAMS);

	void CONS_Francois_InPart3(CONS_PARAMS);
	void HAND_Francois_InPart3(HAND_PARAMS);

	void CONS_Francois_StartPart4(CONS_PARAMS);
	void HAND_Francois_StartPart4(HAND_PARAMS);

	void CONS_Francois_InPart4(CONS_PARAMS);
	void HAND_Francois_InPart4(HAND_PARAMS);

	void CONS_Francois_StartPart5(CONS_PARAMS);
	void HAND_Francois_StartPart5(HAND_PARAMS);

	void CONS_Francois_Prisoner(CONS_PARAMS);
	void HAND_Francois_Prisoner(HAND_PARAMS);

	void CONS_Francois_Hiding(CONS_PARAMS);
	void HAND_Francois_Hiding(HAND_PARAMS);

	void CONS_Francois_Disappear(CONS_PARAMS);
	void HAND_Francois_Disappear(HAND_PARAMS);

	Common::StringArray _francoisFuncNames = {
		"DebugWalks",
		"DoWait",
		"DoSeqOtis",
		"DoCorrOtis",
		"DoEnterCorrOtis",
		"DoDialog",
		"SaveGame",
		"DoWalk",
		"ExitComp",
		"EnterComp",
		"Rampage",
		"TakeWalk",
		"HaremVisit",
		"ChaseBeetle",
		"FindCath",
		"LetsGo",
		"Birth",
		"StartPart1",
		"InComp",
		"Asleep",
		"StartPart2",
		"AtBreakfast",
		"WithMama",
		"StartPart3",
		"InPart3",
		"StartPart4",
		"InPart4",
		"StartPart5",
		"Prisoner",
		"Hiding",
		"Disappear"
	};

	// HADIJA EVENTS
	void HadijaCall(CALL_PARAMS);

	void CONS_Hadija_DebugWalks(CONS_PARAMS);
	void HAND_Hadija_DebugWalks(HAND_PARAMS);

	void CONS_Hadija_DoCorrOtis(CONS_PARAMS);
	void HAND_Hadija_DoCorrOtis(HAND_PARAMS);

	void CONS_Hadija_DoDialog(CONS_PARAMS);
	void HAND_Hadija_DoDialog(HAND_PARAMS);

	void CONS_Hadija_DoWait(CONS_PARAMS);
	void HAND_Hadija_DoWait(HAND_PARAMS);

	void CONS_Hadija_DoWalk(CONS_PARAMS);
	void HAND_Hadija_DoWalk(HAND_PARAMS);

	void CONS_Hadija_PeekF(CONS_PARAMS);
	void HAND_Hadija_PeekF(HAND_PARAMS);

	void CONS_Hadija_PeekH(CONS_PARAMS);
	void HAND_Hadija_PeekH(HAND_PARAMS);

	void CONS_Hadija_GoFtoH(CONS_PARAMS);
	void HAND_Hadija_GoFtoH(HAND_PARAMS);

	void CONS_Hadija_GoHtoF(CONS_PARAMS);
	void HAND_Hadija_GoHtoF(HAND_PARAMS);

	void CONS_Hadija_Birth(CONS_PARAMS);
	void HAND_Hadija_Birth(HAND_PARAMS);

	void CONS_Hadija_Part1(CONS_PARAMS);
	void HAND_Hadija_Part1(HAND_PARAMS);

	void CONS_Hadija_Asleep(CONS_PARAMS);
	void HAND_Hadija_Asleep(HAND_PARAMS);

	void CONS_Hadija_StartPart2(CONS_PARAMS);
	void HAND_Hadija_StartPart2(HAND_PARAMS);

	void CONS_Hadija_Part2(CONS_PARAMS);
	void HAND_Hadija_Part2(HAND_PARAMS);

	void CONS_Hadija_StartPart3(CONS_PARAMS);
	void HAND_Hadija_StartPart3(HAND_PARAMS);

	void CONS_Hadija_Part3(CONS_PARAMS);
	void HAND_Hadija_Part3(HAND_PARAMS);

	void CONS_Hadija_StartPart4(CONS_PARAMS);
	void HAND_Hadija_StartPart4(HAND_PARAMS);

	void CONS_Hadija_Part4(CONS_PARAMS);
	void HAND_Hadija_Part4(HAND_PARAMS);

	void CONS_Hadija_Asleep4(CONS_PARAMS);
	void HAND_Hadija_Asleep4(HAND_PARAMS);

	void CONS_Hadija_StartPart5(CONS_PARAMS);
	void HAND_Hadija_StartPart5(HAND_PARAMS);

	void CONS_Hadija_Prisoner(CONS_PARAMS);
	void HAND_Hadija_Prisoner(HAND_PARAMS);

	void CONS_Hadija_Free(CONS_PARAMS);
	void HAND_Hadija_Free(HAND_PARAMS);

	void CONS_Hadija_Hiding(CONS_PARAMS);
	void HAND_Hadija_Hiding(HAND_PARAMS);

	void CONS_Hadija_Disappear(CONS_PARAMS);
	void HAND_Hadija_Disappear(HAND_PARAMS);

	Common::StringArray _hadijaFuncNames = {
		"DebugWalks",
		"DoCorrOtis",
		"DoDialog",
		"DoWait",
		"DoWalk",
		"PeekF",
		"PeekH",
		"GoFtoH",
		"GoHtoF",
		"Birth",
		"Part1",
		"Asleep",
		"StartPart2",
		"Part2",
		"StartPart3",
		"Part3",
		"StartPart4",
		"Part4",
		"Asleep4",
		"StartPart5",
		"Prisoner",
		"Free",
		"Hiding",
		"Disappear"
	};


	// HEADWAIT EVENTS

	void HeadWaitCall(CALL_PARAMS);

	void CONS_HeadWait_DoSeqOtis(CONS_PARAMS);
	void HAND_HeadWait_DoSeqOtis(HAND_PARAMS);

	void CONS_HeadWait_WaitRCClear(CONS_PARAMS);
	void HAND_HeadWait_WaitRCClear(HAND_PARAMS);

	void CONS_HeadWait_FinishSeqOtis(CONS_PARAMS);
	void HAND_HeadWait_FinishSeqOtis(HAND_PARAMS);

	void CONS_HeadWait_DoWait(CONS_PARAMS);
	void HAND_HeadWait_DoWait(HAND_PARAMS);

	void CONS_HeadWait_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_HeadWait_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_HeadWait_DoDialog(CONS_PARAMS);
	void HAND_HeadWait_DoDialog(HAND_PARAMS);

	void CONS_HeadWait_DoComplexSeqOtis(CONS_PARAMS);
	void HAND_HeadWait_DoComplexSeqOtis(HAND_PARAMS);

	void CONS_HeadWait_RebeccaHereWeAre(CONS_PARAMS);
	void HAND_HeadWait_RebeccaHereWeAre(HAND_PARAMS);

	void CONS_HeadWait_DoOtis5012C(CONS_PARAMS);
	void HAND_HeadWait_DoOtis5012C(HAND_PARAMS);

	void CONS_HeadWait_DoSeatOtis(CONS_PARAMS);
	void HAND_HeadWait_DoSeatOtis(HAND_PARAMS);

	void CONS_HeadWait_SeatCath(CONS_PARAMS);
	void HAND_HeadWait_SeatCath(HAND_PARAMS);

	void CONS_HeadWait_Birth(CONS_PARAMS);
	void HAND_HeadWait_Birth(HAND_PARAMS);

	void CONS_HeadWait_GreetAugust(CONS_PARAMS);
	void HAND_HeadWait_GreetAugust(HAND_PARAMS);

	void CONS_HeadWait_DoOtis5001C(CONS_PARAMS);
	void HAND_HeadWait_DoOtis5001C(HAND_PARAMS);

	void CONS_HeadWait_GreetAnna(CONS_PARAMS);
	void HAND_HeadWait_GreetAnna(HAND_PARAMS);

	void CONS_HeadWait_GreetTatiana(CONS_PARAMS);
	void HAND_HeadWait_GreetTatiana(HAND_PARAMS);

	void CONS_HeadWait_ServingDinner(CONS_PARAMS);
	void HAND_HeadWait_ServingDinner(HAND_PARAMS);

	void CONS_HeadWait_AfterDinner(CONS_PARAMS);
	void HAND_HeadWait_AfterDinner(HAND_PARAMS);

	void CONS_HeadWait_LockUp(CONS_PARAMS);
	void HAND_HeadWait_LockUp(HAND_PARAMS);

	void CONS_HeadWait_StartPart2(CONS_PARAMS);
	void HAND_HeadWait_StartPart2(HAND_PARAMS);

	void CONS_HeadWait_StartPart3(CONS_PARAMS);
	void HAND_HeadWait_StartPart3(HAND_PARAMS);

	void CONS_HeadWait_InPart3(CONS_PARAMS);
	void HAND_HeadWait_InPart3(HAND_PARAMS);

	void CONS_HeadWait_AbbotSeatMe3(CONS_PARAMS);
	void HAND_HeadWait_AbbotSeatMe3(HAND_PARAMS);

	void CONS_HeadWait_DoOtis5029A(CONS_PARAMS);
	void HAND_HeadWait_DoOtis5029A(HAND_PARAMS);

	void CONS_HeadWait_StartPart4(CONS_PARAMS);
	void HAND_HeadWait_StartPart4(HAND_PARAMS);

	void CONS_HeadWait_InPart4(CONS_PARAMS);
	void HAND_HeadWait_InPart4(HAND_PARAMS);

	void CONS_HeadWait_MeetCond2(CONS_PARAMS);
	void HAND_HeadWait_MeetCond2(HAND_PARAMS);

	void CONS_HeadWait_TellAug(CONS_PARAMS);
	void HAND_HeadWait_TellAug(HAND_PARAMS);

	void CONS_HeadWait_RSWalkDowntrain(CONS_PARAMS);
	void HAND_HeadWait_RSWalkDowntrain(HAND_PARAMS);

	void CONS_HeadWait_RSWalkUptrain(CONS_PARAMS);
	void HAND_HeadWait_RSWalkUptrain(HAND_PARAMS);

	void CONS_HeadWait_StartPart5(CONS_PARAMS);
	void HAND_HeadWait_StartPart5(HAND_PARAMS);

	void CONS_HeadWait_Prisoner(CONS_PARAMS);
	void HAND_HeadWait_Prisoner(HAND_PARAMS);

	void CONS_HeadWait_Hiding(CONS_PARAMS);
	void HAND_HeadWait_Hiding(HAND_PARAMS);

	void CONS_HeadWait_Disappear(CONS_PARAMS);
	void HAND_HeadWait_Disappear(HAND_PARAMS);

	Common::StringArray _headWaitFuncNames = {
		"DoSeqOtis",
		"WaitRCClear",
		"FinishSeqOtis",
		"DoWait",
		"DoBlockSeqOtis",
		"DoDialog",
		"DoComplexSeqOtis",
		"RebeccaHereWeAre",
		"DoOtis5012C",
		"DoSeatOtis",
		"SeatCath",
		"Birth",
		"GreetAugust",
		"DoOtis5001C",
		"GreetAnna",
		"GreetTatiana",
		"ServingDinner",
		"AfterDinner",
		"LockUp",
		"StartPart2",
		"StartPart3",
		"InPart3",
		"AbbotSeatMe3",
		"DoOtis5029A",
		"StartPart4",
		"InPart4",
		"MeetCond2",
		"TellAug",
		"RSWalkDowntrain",
		"RSWalkUptrain",
		"StartPart5",
		"Prisoner"
		"Hiding",
		"Disappear"
	};

	// IVO EVENTS

	void IvoCall(CALL_PARAMS);

	void CONS_Ivo_DebugWalks(CONS_PARAMS);
	void HAND_Ivo_DebugWalks(HAND_PARAMS);

	void CONS_Ivo_DoSeqOtis(CONS_PARAMS);
	void HAND_Ivo_DoSeqOtis(HAND_PARAMS);

	void CONS_Ivo_DoCorrOtis(CONS_PARAMS);
	void HAND_Ivo_DoCorrOtis(HAND_PARAMS);

	void CONS_Ivo_DoWait(CONS_PARAMS);
	void HAND_Ivo_DoWait(HAND_PARAMS);

	void CONS_Ivo_DoWaitReal(CONS_PARAMS);
	void HAND_Ivo_DoWaitReal(HAND_PARAMS);

	void CONS_Ivo_DoWalk(CONS_PARAMS);
	void HAND_Ivo_DoWalk(HAND_PARAMS);

	void CONS_Ivo_FinishSeqOtis(CONS_PARAMS);
	void HAND_Ivo_FinishSeqOtis(HAND_PARAMS);

	void CONS_Ivo_DoDialog(CONS_PARAMS);
	void HAND_Ivo_DoDialog(HAND_PARAMS);

	void CONS_Ivo_WaitRCClear(CONS_PARAMS);
	void HAND_Ivo_WaitRCClear(HAND_PARAMS);

	void CONS_Ivo_SaveGame(CONS_PARAMS);
	void HAND_Ivo_SaveGame(HAND_PARAMS);

	void CONS_Ivo_GoCompartment(CONS_PARAMS);
	void HAND_Ivo_GoCompartment(HAND_PARAMS);

	void CONS_Ivo_DoSplitOtis023A(CONS_PARAMS);
	void HAND_Ivo_DoSplitOtis023A(HAND_PARAMS);

	void CONS_Ivo_DoSplitOtis023D(CONS_PARAMS);
	void HAND_Ivo_DoSplitOtis023D(HAND_PARAMS);

	void CONS_Ivo_Birth(CONS_PARAMS);
	void HAND_Ivo_Birth(HAND_PARAMS);

	void CONS_Ivo_ReturnComp(CONS_PARAMS);
	void HAND_Ivo_ReturnComp(HAND_PARAMS);

	void CONS_Ivo_InComp(CONS_PARAMS);
	void HAND_Ivo_InComp(HAND_PARAMS);

	void CONS_Ivo_Asleep(CONS_PARAMS);
	void HAND_Ivo_Asleep(HAND_PARAMS);

	void CONS_Ivo_StartPart2(CONS_PARAMS);
	void HAND_Ivo_StartPart2(HAND_PARAMS);

	void CONS_Ivo_GoBreakfast(CONS_PARAMS);
	void HAND_Ivo_GoBreakfast(HAND_PARAMS);

	void CONS_Ivo_AtBreakfast(CONS_PARAMS);
	void HAND_Ivo_AtBreakfast(HAND_PARAMS);

	void CONS_Ivo_InComp2(CONS_PARAMS);
	void HAND_Ivo_InComp2(HAND_PARAMS);

	void CONS_Ivo_StartPart3(CONS_PARAMS);
	void HAND_Ivo_StartPart3(HAND_PARAMS);

	void CONS_Ivo_InComp3(CONS_PARAMS);
	void HAND_Ivo_InComp3(HAND_PARAMS);

	void CONS_Ivo_StartPart4(CONS_PARAMS);
	void HAND_Ivo_StartPart4(HAND_PARAMS);

	void CONS_Ivo_AtDinner4(CONS_PARAMS);
	void HAND_Ivo_AtDinner4(HAND_PARAMS);

	void CONS_Ivo_ReturnComp4(CONS_PARAMS);
	void HAND_Ivo_ReturnComp4(HAND_PARAMS);

	void CONS_Ivo_InComp4(CONS_PARAMS);
	void HAND_Ivo_InComp4(HAND_PARAMS);

	void CONS_Ivo_Hiding(CONS_PARAMS);
	void HAND_Ivo_Hiding(HAND_PARAMS);

	void CONS_Ivo_EndPart4(CONS_PARAMS);
	void HAND_Ivo_EndPart4(HAND_PARAMS);

	void CONS_Ivo_StartPart5(CONS_PARAMS);
	void HAND_Ivo_StartPart5(HAND_PARAMS);

	void CONS_Ivo_GoofingOff(CONS_PARAMS);
	void HAND_Ivo_GoofingOff(HAND_PARAMS);

	void CONS_Ivo_FightCath(CONS_PARAMS);
	void HAND_Ivo_FightCath(HAND_PARAMS);

	void CONS_Ivo_KnockedOut(CONS_PARAMS);
	void HAND_Ivo_KnockedOut(HAND_PARAMS);

	void CONS_Ivo_Disappear(CONS_PARAMS);
	void HAND_Ivo_Disappear(HAND_PARAMS);

	Common::StringArray _ivoFuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"DoCorrOtis",
		"DoWait",
		"DoWaitReal",
		"DoWalk",
		"FinishSeqOtis",
		"DoDialog",
		"WaitRCClear",
		"SaveGame",
		"GoCompartment",
		"DoSplitOtis023A",
		"DoSplitOtis023D",
		"Birth",
		"ReturnComp",
		"InComp",
		"Asleep",
		"StartPart2",
		"GoBreakfast",
		"AtBreakfast",
		"InComp2",
		"StartPart3",
		"InComp3",
		"StartPart4",
		"AtDinner4",
		"ReturnComp4",
		"InComp4",
		"Hiding",
		"EndPart4",
		"StartPart5",
		"GoofingOff",
		"FightCath",
		"KnockedOut",
		"Disappear"
	};

	// KAHINA EVENTS

	void KahinaCall(CALL_PARAMS);

	void CONS_Kahina_DebugWalks(CONS_PARAMS);
	void HAND_Kahina_DebugWalks(HAND_PARAMS);

	void CONS_Kahina_DoDialog(CONS_PARAMS);
	void HAND_Kahina_DoDialog(HAND_PARAMS);

	void CONS_Kahina_SaveGame(CONS_PARAMS);
	void HAND_Kahina_SaveGame(HAND_PARAMS);

	void CONS_Kahina_DoWait(CONS_PARAMS);
	void HAND_Kahina_DoWait(HAND_PARAMS);

	void CONS_Kahina_DoWaitReal(CONS_PARAMS);
	void HAND_Kahina_DoWaitReal(HAND_PARAMS);

	void CONS_Kahina_LookingForCath(CONS_PARAMS);
	void HAND_Kahina_LookingForCath(HAND_PARAMS);

	void CONS_Kahina_DoWalkSearchingForCath(CONS_PARAMS);
	void HAND_Kahina_DoWalkSearchingForCath(HAND_PARAMS);

	void CONS_Kahina_DoWalk(CONS_PARAMS);
	void HAND_Kahina_DoWalk(HAND_PARAMS);

	void CONS_Kahina_DoCorrOtis(CONS_PARAMS);
	void HAND_Kahina_DoCorrOtis(HAND_PARAMS);

	void CONS_Kahina_Birth(CONS_PARAMS);
	void HAND_Kahina_Birth(HAND_PARAMS);

	void CONS_Kahina_InSeclusion(CONS_PARAMS);
	void HAND_Kahina_InSeclusion(HAND_PARAMS);

	void CONS_Kahina_AwaitingCath(CONS_PARAMS);
	void HAND_Kahina_AwaitingCath(HAND_PARAMS);

	void CONS_Kahina_CathDone(CONS_PARAMS);
	void HAND_Kahina_CathDone(HAND_PARAMS);

	void CONS_Kahina_DoDogBarkOtis(CONS_PARAMS);
	void HAND_Kahina_DoDogBarkOtis(HAND_PARAMS);

	void CONS_Kahina_SearchTrain(CONS_PARAMS);
	void HAND_Kahina_SearchTrain(HAND_PARAMS);

	void CONS_Kahina_StartPart2(CONS_PARAMS);
	void HAND_Kahina_StartPart2(HAND_PARAMS);

	void CONS_Kahina_InSeclusionPart2(CONS_PARAMS);
	void HAND_Kahina_InSeclusionPart2(HAND_PARAMS);

	void CONS_Kahina_StartPart3(CONS_PARAMS);
	void HAND_Kahina_StartPart3(HAND_PARAMS);

	void CONS_Kahina_DoWalk1033(CONS_PARAMS);
	void HAND_Kahina_DoWalk1033(HAND_PARAMS);

	void CONS_Kahina_BeforeConcert(CONS_PARAMS);
	void HAND_Kahina_BeforeConcert(HAND_PARAMS);

	void CONS_Kahina_Concert(CONS_PARAMS);
	void HAND_Kahina_Concert(HAND_PARAMS);

	void CONS_Kahina_Finished(CONS_PARAMS);
	void HAND_Kahina_Finished(HAND_PARAMS);

	void CONS_Kahina_FindFirebird(CONS_PARAMS);
	void HAND_Kahina_FindFirebird(HAND_PARAMS);

	void CONS_Kahina_SeekCath(CONS_PARAMS);
	void HAND_Kahina_SeekCath(HAND_PARAMS);

	void CONS_Kahina_SearchCath(CONS_PARAMS);
	void HAND_Kahina_SearchCath(HAND_PARAMS);

	void CONS_Kahina_SearchTatiana(CONS_PARAMS);
	void HAND_Kahina_SearchTatiana(HAND_PARAMS);

	void CONS_Kahina_KillCathAnywhere(CONS_PARAMS);
	void HAND_Kahina_KillCathAnywhere(HAND_PARAMS);

	void CONS_Kahina_StartPart4(CONS_PARAMS);
	void HAND_Kahina_StartPart4(HAND_PARAMS);

	void CONS_Kahina_StartPart5(CONS_PARAMS);
	void HAND_Kahina_StartPart5(HAND_PARAMS);

	Common::StringArray _kahinaFuncNames = {
		"DebugWalks",
		"DoDialog",
		"SaveGame",
		"DoWait",
		"DoWaitReal",
		"LookingForCath",
		"DoWalkSearchingForCath",
		"DoWalk",
		"DoCorrOtis",
		"Birth",
		"InSeclusion",
		"AwaitingCath",
		"CathDone",
		"DoDogBarkOtis",
		"SearchTrain",
		"StartPart2",
		"InSeclusionPart2",
		"StartPart3",
		"DoWalk1033",
		"BeforeConcert",
		"Concert",
		"Finished",
		"FindFirebird",
		"SeekCath",
		"SearchCath",
		"SearchTatiana",
		"KillCathAnywhere",
		"StartPart4",
		"StartPart5"
	};

	// KRONOS EVENTS

	void KronosCall(CALL_PARAMS);

	void CONS_Kronos_DebugWalks(CONS_PARAMS);
	void HAND_Kronos_DebugWalks(HAND_PARAMS);

	void CONS_Kronos_SaveGame(CONS_PARAMS);
	void HAND_Kronos_SaveGame(HAND_PARAMS);

	void CONS_Kronos_DoWalk(CONS_PARAMS);
	void HAND_Kronos_DoWalk(HAND_PARAMS);

	void CONS_Kronos_DoDialog(CONS_PARAMS);
	void HAND_Kronos_DoDialog(HAND_PARAMS);

	void CONS_Kronos_DoWait(CONS_PARAMS);
	void HAND_Kronos_DoWait(HAND_PARAMS);

	void CONS_Kronos_DoWaitReal(CONS_PARAMS);
	void HAND_Kronos_DoWaitReal(HAND_PARAMS);

	void CONS_Kronos_Birth(CONS_PARAMS);
	void HAND_Kronos_Birth(HAND_PARAMS);

	void CONS_Kronos_AwaitingCath(CONS_PARAMS);
	void HAND_Kronos_AwaitingCath(HAND_PARAMS);

	void CONS_Kronos_GreetCath(CONS_PARAMS);
	void HAND_Kronos_GreetCath(HAND_PARAMS);

	void CONS_Kronos_CathDone(CONS_PARAMS);
	void HAND_Kronos_CathDone(HAND_PARAMS);

	void CONS_Kronos_PlayingAtNight(CONS_PARAMS);
	void HAND_Kronos_PlayingAtNight(HAND_PARAMS);

	void CONS_Kronos_StartPart2(CONS_PARAMS);
	void HAND_Kronos_StartPart2(HAND_PARAMS);

	void CONS_Kronos_StartPart3(CONS_PARAMS);
	void HAND_Kronos_StartPart3(HAND_PARAMS);

	void CONS_Kronos_InSeclusion(CONS_PARAMS);
	void HAND_Kronos_InSeclusion(HAND_PARAMS);

	void CONS_Kronos_WBWait(CONS_PARAMS);
	void HAND_Kronos_WBWait(HAND_PARAMS);

	void CONS_Kronos_VisitSalon(CONS_PARAMS);
	void HAND_Kronos_VisitSalon(HAND_PARAMS);

	void CONS_Kronos_ReturnComp(CONS_PARAMS);
	void HAND_Kronos_ReturnComp(HAND_PARAMS);

	void CONS_Kronos_PreConcert(CONS_PARAMS);
	void HAND_Kronos_PreConcert(HAND_PARAMS);

	void CONS_Kronos_StartConcert(CONS_PARAMS);
	void HAND_Kronos_StartConcert(HAND_PARAMS);

	void CONS_Kronos_Concert(CONS_PARAMS);
	void HAND_Kronos_Concert(HAND_PARAMS);

	void CONS_Kronos_AfterConcert(CONS_PARAMS);
	void HAND_Kronos_AfterConcert(HAND_PARAMS);

	void CONS_Kronos_AwaitingCath3(CONS_PARAMS);
	void HAND_Kronos_AwaitingCath3(HAND_PARAMS);

	void CONS_Kronos_Finished(CONS_PARAMS);
	void HAND_Kronos_Finished(HAND_PARAMS);

	void CONS_Kronos_StartPart4(CONS_PARAMS);
	void HAND_Kronos_StartPart4(HAND_PARAMS);

	void CONS_Kronos_StartPart5(CONS_PARAMS);
	void HAND_Kronos_StartPart5(HAND_PARAMS);

	Common::StringArray _kronosFuncNames = {
		"DebugWalks",
		"SaveGame",
		"DoWalk",
		"DoDialog",
		"DoWait",
		"DoWaitReal",
		"Birth",
		"AwaitingCath",
		"GreetCath",
		"CathDone",
		"PlayingAtNight",
		"StartPart2",
		"StartPart3",
		"InSeclusion",
		"WBWait",
		"VisitSalon",
		"ReturnComp",
		"PreConcert",
		"StartConcert",
		"Concert",
		"AfterConcert",
		"AwaitingCath3",
		"Finished",
		"StartPart4",
		"StartPart5"
	};

	// MADAME EVENTS

	void MadameCall(CALL_PARAMS);

	void CONS_Madame_DebugWalks(CONS_PARAMS);
	void HAND_Madame_DebugWalks(HAND_PARAMS);

	void CONS_Madame_DoDialog(CONS_PARAMS);
	void HAND_Madame_DoDialog(HAND_PARAMS);

	void CONS_Madame_DoSeqOtis(CONS_PARAMS);
	void HAND_Madame_DoSeqOtis(HAND_PARAMS);

	void CONS_Madame_DoWait(CONS_PARAMS);
	void HAND_Madame_DoWait(HAND_PARAMS);

	void CONS_Madame_DoCorrOtis(CONS_PARAMS);
	void HAND_Madame_DoCorrOtis(HAND_PARAMS);

	void CONS_Madame_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_Madame_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_Madame_DoWalk(CONS_PARAMS);
	void HAND_Madame_DoWalk(HAND_PARAMS);

	void CONS_Madame_ComplainCond2(CONS_PARAMS);
	void HAND_Madame_ComplainCond2(HAND_PARAMS);

	void CONS_Madame_LetsGo(CONS_PARAMS);
	void HAND_Madame_LetsGo(HAND_PARAMS);

	void CONS_Madame_Birth(CONS_PARAMS);
	void HAND_Madame_Birth(HAND_PARAMS);

	void CONS_Madame_FranStory(CONS_PARAMS);
	void HAND_Madame_FranStory(HAND_PARAMS);

	void CONS_Madame_OpenComp1(CONS_PARAMS);
	void HAND_Madame_OpenComp1(HAND_PARAMS);

	void CONS_Madame_ClosedComp1(CONS_PARAMS);
	void HAND_Madame_ClosedComp1(HAND_PARAMS);

	void CONS_Madame_OpenComp(CONS_PARAMS);
	void HAND_Madame_OpenComp(HAND_PARAMS);

	void CONS_Madame_InComp(CONS_PARAMS);
	void HAND_Madame_InComp(HAND_PARAMS);

	void CONS_Madame_Asleep(CONS_PARAMS);
	void HAND_Madame_Asleep(HAND_PARAMS);

	void CONS_Madame_StartPart2(CONS_PARAMS);
	void HAND_Madame_StartPart2(HAND_PARAMS);

	void CONS_Madame_AtBreakfast(CONS_PARAMS);
	void HAND_Madame_AtBreakfast(HAND_PARAMS);

	void CONS_Madame_OpenComp2(CONS_PARAMS);
	void HAND_Madame_OpenComp2(HAND_PARAMS);

	void CONS_Madame_StartPart3(CONS_PARAMS);
	void HAND_Madame_StartPart3(HAND_PARAMS);

	void CONS_Madame_OpenComp3(CONS_PARAMS);
	void HAND_Madame_OpenComp3(HAND_PARAMS);

	void CONS_Madame_StartPart4(CONS_PARAMS);
	void HAND_Madame_StartPart4(HAND_PARAMS);

	void CONS_Madame_OpenComp4(CONS_PARAMS);
	void HAND_Madame_OpenComp4(HAND_PARAMS);

	void CONS_Madame_ClosedComp4(CONS_PARAMS);
	void HAND_Madame_ClosedComp4(HAND_PARAMS);

	void CONS_Madame_Asleep4(CONS_PARAMS);
	void HAND_Madame_Asleep4(HAND_PARAMS);

	void CONS_Madame_StartPart5(CONS_PARAMS);
	void HAND_Madame_StartPart5(HAND_PARAMS);

	void CONS_Madame_Prisoner(CONS_PARAMS);
	void HAND_Madame_Prisoner(HAND_PARAMS);

	void CONS_Madame_Hiding(CONS_PARAMS);
	void HAND_Madame_Hiding(HAND_PARAMS);

	void CONS_Madame_Disappear(CONS_PARAMS);
	void HAND_Madame_Disappear(HAND_PARAMS);

	Common::StringArray _madameFuncNames = {
		"DebugWalks",
		"DoDialog",
		"DoSeqOtis",
		"DoWait",
		"DoCorrOtis",
		"DoEnterCorrOtis",
		"DoWalk",
		"ComplainCond2",
		"LetsGo",
		"Birth",
		"FranStory",
		"OpenComp1",
		"ClosedComp1",
		"OpenComp",
		"InComp",
		"Asleep",
		"StartPart2",
		"AtBreakfast",
		"OpenComp2",
		"StartPart3",
		"OpenComp3",
		"StartPart4",
		"OpenComp4",
		"ClosedComp4",
		"Asleep4",
		"StartPart5",
		"Prisoner",
		"Hiding",
		"Disappear"
	};

	// MAHMUD EVENTS

	void MahmudCall(CALL_PARAMS);

	void CONS_Mahmud_DebugWalks(CONS_PARAMS);
	void HAND_Mahmud_DebugWalks(HAND_PARAMS);

	void CONS_Mahmud_DoSeqOtis(CONS_PARAMS);
	void HAND_Mahmud_DoSeqOtis(HAND_PARAMS);

	void CONS_Mahmud_DoCorrOtis(CONS_PARAMS);
	void HAND_Mahmud_DoCorrOtis(HAND_PARAMS);

	void CONS_Mahmud_DoBumpCorrOtis(CONS_PARAMS);
	void HAND_Mahmud_DoBumpCorrOtis(HAND_PARAMS);

	void CONS_Mahmud_DoDialog(CONS_PARAMS);
	void HAND_Mahmud_DoDialog(HAND_PARAMS);

	void CONS_Mahmud_DoCondDialog(CONS_PARAMS);
	void HAND_Mahmud_DoCondDialog(HAND_PARAMS);

	void CONS_Mahmud_DoWait(CONS_PARAMS);
	void HAND_Mahmud_DoWait(HAND_PARAMS);

	void CONS_Mahmud_SaveGame(CONS_PARAMS);
	void HAND_Mahmud_SaveGame(HAND_PARAMS);

	void CONS_Mahmud_DoWalk(CONS_PARAMS);
	void HAND_Mahmud_DoWalk(HAND_PARAMS);

	void CONS_Mahmud_CathKnockingHarem(CONS_PARAMS);
	void HAND_Mahmud_CathKnockingHarem(HAND_PARAMS);

	void CONS_Mahmud_CondKnocking(CONS_PARAMS);
	void HAND_Mahmud_CondKnocking(HAND_PARAMS);

	void CONS_Mahmud_CheckF(CONS_PARAMS);
	void HAND_Mahmud_CheckF(HAND_PARAMS);

	void CONS_Mahmud_CheckH(CONS_PARAMS);
	void HAND_Mahmud_CheckH(HAND_PARAMS);

	void CONS_Mahmud_Vigilant(CONS_PARAMS);
	void HAND_Mahmud_Vigilant(HAND_PARAMS);

	void CONS_Mahmud_Birth(CONS_PARAMS);
	void HAND_Mahmud_Birth(HAND_PARAMS);

	void CONS_Mahmud_Asleep(CONS_PARAMS);
	void HAND_Mahmud_Asleep(HAND_PARAMS);

	void CONS_Mahmud_StartPart2(CONS_PARAMS);
	void HAND_Mahmud_StartPart2(HAND_PARAMS);

	void CONS_Mahmud_StartPart3(CONS_PARAMS);
	void HAND_Mahmud_StartPart3(HAND_PARAMS);

	void CONS_Mahmud_StartPart4(CONS_PARAMS);
	void HAND_Mahmud_StartPart4(HAND_PARAMS);

	void CONS_Mahmud_StartPart5(CONS_PARAMS);
	void HAND_Mahmud_StartPart5(HAND_PARAMS);

	Common::StringArray _mahmudFuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"DoCorrOtis",
		"DoBumpCorrOtis",
		"DoDialog",
		"DoCondDialog",
		"DoWait",
		"SaveGame",
		"DoWalk",
		"CathKnockingHarem",
		"CondKnocking",
		"CheckF",
		"CheckH",
		"Vigilant",
		"Birth",
		"Asleep",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};

	// MASTER EVENTS

	void MasterCall(CALL_PARAMS);

	void CONS_Master_SaveGame(CONS_PARAMS);
	void HAND_Master_SaveGame(HAND_PARAMS);

	void CONS_Master_Arrive(CONS_PARAMS);
	void HAND_Master_Arrive(HAND_PARAMS);

	void CONS_Master_Depart(CONS_PARAMS);
	void HAND_Master_Depart(HAND_PARAMS);

	void CONS_Master_Birth(CONS_PARAMS);
	void HAND_Master_Birth(HAND_PARAMS);

	void CONS_Master_WalkTest(CONS_PARAMS);
	void HAND_Master_WalkTest(HAND_PARAMS);

	void CONS_Master_FirstDream(CONS_PARAMS);
	void HAND_Master_FirstDream(HAND_PARAMS);

	void CONS_Master_StartPart1(CONS_PARAMS);
	void HAND_Master_StartPart1(HAND_PARAMS);

	void CONS_Master_MovingNight(CONS_PARAMS);
	void HAND_Master_MovingNight(HAND_PARAMS);

	void CONS_Master_SecondSleep(CONS_PARAMS);
	void HAND_Master_SecondSleep(HAND_PARAMS);

	void CONS_Master_StartPart2(CONS_PARAMS);
	void HAND_Master_StartPart2(HAND_PARAMS);

	void CONS_Master_InitPart2(CONS_PARAMS);
	void HAND_Master_InitPart2(HAND_PARAMS);

	void CONS_Master_MovingDay(CONS_PARAMS);
	void HAND_Master_MovingDay(HAND_PARAMS);

	void CONS_Master_StartPart3(CONS_PARAMS);
	void HAND_Master_StartPart3(HAND_PARAMS);

	void CONS_Master_InitPart3(CONS_PARAMS);
	void HAND_Master_InitPart3(HAND_PARAMS);

	void CONS_Master_MovingAfternoon(CONS_PARAMS);
	void HAND_Master_MovingAfternoon(HAND_PARAMS);

	void CONS_Master_EndVienna(CONS_PARAMS);
	void HAND_Master_EndVienna(HAND_PARAMS);

	void CONS_Master_StartPart4(CONS_PARAMS);
	void HAND_Master_StartPart4(HAND_PARAMS);

	void CONS_Master_InitPart4(CONS_PARAMS);
	void HAND_Master_InitPart4(HAND_PARAMS);

	void CONS_Master_MovingSecondNight(CONS_PARAMS);
	void HAND_Master_MovingSecondNight(HAND_PARAMS);

	void CONS_Master_StartPart5(CONS_PARAMS);
	void HAND_Master_StartPart5(HAND_PARAMS);

	void CONS_Master_InitPart5(CONS_PARAMS);
	void HAND_Master_InitPart5(HAND_PARAMS);

	void CONS_Master_MovingPart5(CONS_PARAMS);
	void HAND_Master_MovingPart5(HAND_PARAMS);

	Common::StringArray _masterFuncNames = {
		"SaveGame",
		"Arrive",
		"Depart",
		"Birth",
		"WalkTest",
		"FirstDream",
		"StartPart1",
		"MovingNight",
		"SecondSleep",
		"StartPart2",
		"InitPart2",
		"MovingDay",
		"StartPart3",
		"InitPart3",
		"MovingAfternoon",
		"EndVienna",
		"StartPart4",
		"InitPart4",
		"MovingSecondNight",
		"StartPart5",
		"InitPart5",
		"MovingPart5"
	};

	// MAX EVENTS

	void MaxCall(CALL_PARAMS);

	void CONS_Max_DebugWalks(CONS_PARAMS);
	void HAND_Max_DebugWalks(HAND_PARAMS);

	void CONS_Max_DoDialog(CONS_PARAMS);
	void HAND_Max_DoDialog(HAND_PARAMS);

	void CONS_Max_DoSeqOtis(CONS_PARAMS);
	void HAND_Max_DoSeqOtis(HAND_PARAMS);

	void CONS_Max_DoCorrOtis(CONS_PARAMS);
	void HAND_Max_DoCorrOtis(HAND_PARAMS);

	void CONS_Max_SaveGame(CONS_PARAMS);
	void HAND_Max_SaveGame(HAND_PARAMS);

	void CONS_Max_WithAnna(CONS_PARAMS);
	void HAND_Max_WithAnna(HAND_PARAMS);

	void CONS_Max_GuardingComp(CONS_PARAMS);
	void HAND_Max_GuardingComp(HAND_PARAMS);

	void CONS_Max_InCageFriendly(CONS_PARAMS);
	void HAND_Max_InCageFriendly(HAND_PARAMS);

	void CONS_Max_Escaped(CONS_PARAMS);
	void HAND_Max_Escaped(HAND_PARAMS);

	void CONS_Max_Birth(CONS_PARAMS);
	void HAND_Max_Birth(HAND_PARAMS);

	void CONS_Max_StartPart2(CONS_PARAMS);
	void HAND_Max_StartPart2(HAND_PARAMS);

	void CONS_Max_StartPart3(CONS_PARAMS);
	void HAND_Max_StartPart3(HAND_PARAMS);

	void CONS_Max_WithAnna3(CONS_PARAMS);
	void HAND_Max_WithAnna3(HAND_PARAMS);

	void CONS_Max_InCageMad(CONS_PARAMS);
	void HAND_Max_InCageMad(HAND_PARAMS);

	void CONS_Max_LetMeIn3(CONS_PARAMS);
	void HAND_Max_LetMeIn3(HAND_PARAMS);

	void CONS_Max_StartPart4(CONS_PARAMS);
	void HAND_Max_StartPart4(HAND_PARAMS);

	void CONS_Max_LetMeIn4(CONS_PARAMS);
	void HAND_Max_LetMeIn4(HAND_PARAMS);

	void CONS_Max_StartPart5(CONS_PARAMS);
	void HAND_Max_StartPart5(HAND_PARAMS);

	Common::StringArray _maxFuncNames = {
		"DebugWalks",
		"DoDialog",
		"DoSeqOtis",
		"DoCorrOtis",
		"SaveGame",
		"WithAnna",
		"GuardingComp",
		"InCageFriendly",
		"Escaped",
		"Birth",
		"StartPart2",
		"StartPart3",
		"WithAnna3",
		"InCageMad",
		"LetMeIn3",
		"StartPart4",
		"LetMeIn4",
		"StartPart5"
	};

	// MILOS EVENTS

	void MilosCall(CALL_PARAMS);

	void CONS_Milos_DebugWalks(CONS_PARAMS);
	void HAND_Milos_DebugWalks(HAND_PARAMS);

	void CONS_Milos_DoSeqOtis(CONS_PARAMS);
	void HAND_Milos_DoSeqOtis(HAND_PARAMS);

	void CONS_Milos_DoCorrOtis(CONS_PARAMS);
	void HAND_Milos_DoCorrOtis(HAND_PARAMS);

	void CONS_Milos_DoBriefCorrOtis(CONS_PARAMS);
	void HAND_Milos_DoBriefCorrOtis(HAND_PARAMS);

	void CONS_Milos_FinishSeqOtis(CONS_PARAMS);
	void HAND_Milos_FinishSeqOtis(HAND_PARAMS);

	void CONS_Milos_DoDialog(CONS_PARAMS);
	void HAND_Milos_DoDialog(HAND_PARAMS);

	void CONS_Milos_DoDialogFullVol(CONS_PARAMS);
	void HAND_Milos_DoDialogFullVol(HAND_PARAMS);

	void CONS_Milos_SaveGame(CONS_PARAMS);
	void HAND_Milos_SaveGame(HAND_PARAMS);

	void CONS_Milos_DoWait(CONS_PARAMS);
	void HAND_Milos_DoWait(HAND_PARAMS);

	void CONS_Milos_DoWalk(CONS_PARAMS);
	void HAND_Milos_DoWalk(HAND_PARAMS);

	void CONS_Milos_CompLogic(CONS_PARAMS);
	void HAND_Milos_CompLogic(HAND_PARAMS);

	void CONS_Milos_Birth(CONS_PARAMS);
	void HAND_Milos_Birth(HAND_PARAMS);

	void CONS_Milos_DoOtis5009D(CONS_PARAMS);
	void HAND_Milos_DoOtis5009D(HAND_PARAMS);

	void CONS_Milos_KnockTyler(CONS_PARAMS);
	void HAND_Milos_KnockTyler(HAND_PARAMS);

	void CONS_Milos_AtDinner(CONS_PARAMS);
	void HAND_Milos_AtDinner(HAND_PARAMS);

	void CONS_Milos_ReturnFromDinner(CONS_PARAMS);
	void HAND_Milos_ReturnFromDinner(HAND_PARAMS);

	void CONS_Milos_InComp(CONS_PARAMS);
	void HAND_Milos_InComp(HAND_PARAMS);

	void CONS_Milos_Asleep(CONS_PARAMS);
	void HAND_Milos_Asleep(HAND_PARAMS);

	void CONS_Milos_StartPart2(CONS_PARAMS);
	void HAND_Milos_StartPart2(HAND_PARAMS);

	void CONS_Milos_OnRearPlat(CONS_PARAMS);
	void HAND_Milos_OnRearPlat(HAND_PARAMS);

	void CONS_Milos_InPart2(CONS_PARAMS);
	void HAND_Milos_InPart2(HAND_PARAMS);

	void CONS_Milos_StartPart3(CONS_PARAMS);
	void HAND_Milos_StartPart3(HAND_PARAMS);

	void CONS_Milos_OnRearPlat3(CONS_PARAMS);
	void HAND_Milos_OnRearPlat3(HAND_PARAMS);

	void CONS_Milos_Unfriendly(CONS_PARAMS);
	void HAND_Milos_Unfriendly(HAND_PARAMS);

	void CONS_Milos_Friendly(CONS_PARAMS);
	void HAND_Milos_Friendly(HAND_PARAMS);

	void CONS_Milos_LookingForCath(CONS_PARAMS);
	void HAND_Milos_LookingForCath(HAND_PARAMS);

	void CONS_Milos_DoWalkSearchingForCath(CONS_PARAMS);
	void HAND_Milos_DoWalkSearchingForCath(HAND_PARAMS);

	void CONS_Milos_StartPart4(CONS_PARAMS);
	void HAND_Milos_StartPart4(HAND_PARAMS);

	void CONS_Milos_Conspiring(CONS_PARAMS);
	void HAND_Milos_Conspiring(HAND_PARAMS);

	void CONS_Milos_InCharge(CONS_PARAMS);
	void HAND_Milos_InCharge(HAND_PARAMS);

	void CONS_Milos_LeaveComp4(CONS_PARAMS);
	void HAND_Milos_LeaveComp4(HAND_PARAMS);

	void CONS_Milos_EndPart4(CONS_PARAMS);
	void HAND_Milos_EndPart4(HAND_PARAMS);

	void CONS_Milos_StartPart5(CONS_PARAMS);
	void HAND_Milos_StartPart5(HAND_PARAMS);

	void CONS_Milos_RunningTrain(CONS_PARAMS);
	void HAND_Milos_RunningTrain(HAND_PARAMS);

	void CONS_Milos_Dead(CONS_PARAMS);
	void HAND_Milos_Dead(HAND_PARAMS);

	Common::StringArray _milosFuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"DoCorrOtis",
		"DoBriefCorrOtis",
		"FinishSeqOtis",
		"DoDialog",
		"DoDialogFullVol",
		"SaveGame",
		"DoWait",
		"DoWalk",
		"CompLogic",
		"Birth",
		"DoOtis5009D",
		"KnockTyler",
		"AtDinner",
		"ReturnFromDinner",
		"InComp",
		"Asleep",
		"StartPart2",
		"OnRearPlat",
		"InPart2",
		"StartPart3",
		"OnRearPlat3",
		"Unfriendly",
		"Friendly",
		"LookingForCath",
		"DoWalkSearchingForCath",
		"StartPart4",
		"Conspiring",
		"InCharge",
		"LeaveComp4",
		"EndPart4",
		"StartPart5",
		"RunningTrain",
		"Dead"
	};

	// MITCHELL EVENTS

	void CONS_Mitchell_Birth(CONS_PARAMS);
	void HAND_Mitchell_Birth(HAND_PARAMS);

	void CONS_Mitchell_StartPart2(CONS_PARAMS);
	void HAND_Mitchell_StartPart2(HAND_PARAMS);

	void CONS_Mitchell_StartPart3(CONS_PARAMS);
	void HAND_Mitchell_StartPart3(HAND_PARAMS);

	void CONS_Mitchell_StartPart4(CONS_PARAMS);
	void HAND_Mitchell_StartPart4(HAND_PARAMS);

	void CONS_Mitchell_StartPart5(CONS_PARAMS);
	void HAND_Mitchell_StartPart5(HAND_PARAMS);

	void CONS_Mitchell_OhBabyBaby(CONS_PARAMS);
	void HAND_Mitchell_OhBabyBaby(HAND_PARAMS);

	Common::StringArray _mitchellFuncNames = {
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5",
		"OhBabyBaby",
	};

	// MONSIEUR EVENTS

	void MonsieurCall(CALL_PARAMS);

	void CONS_Monsieur_DebugWalks(CONS_PARAMS);
	void HAND_Monsieur_DebugWalks(HAND_PARAMS);

	void CONS_Monsieur_DoDialog(CONS_PARAMS);
	void HAND_Monsieur_DoDialog(HAND_PARAMS);

	void CONS_Monsieur_DoSeqOtis(CONS_PARAMS);
	void HAND_Monsieur_DoSeqOtis(HAND_PARAMS);

	void CONS_Monsieur_DoWait(CONS_PARAMS);
	void HAND_Monsieur_DoWait(HAND_PARAMS);

	void CONS_Monsieur_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_Monsieur_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_Monsieur_DoCorrOtis(CONS_PARAMS);
	void HAND_Monsieur_DoCorrOtis(HAND_PARAMS);

	void CONS_Monsieur_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_Monsieur_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_Monsieur_FinishSeqOtis(CONS_PARAMS);
	void HAND_Monsieur_FinishSeqOtis(HAND_PARAMS);

	void CONS_Monsieur_WaitRCClear(CONS_PARAMS);
	void HAND_Monsieur_WaitRCClear(HAND_PARAMS);

	void CONS_Monsieur_DoWalk(CONS_PARAMS);
	void HAND_Monsieur_DoWalk(HAND_PARAMS);

	void CONS_Monsieur_GoDining(CONS_PARAMS);
	void HAND_Monsieur_GoDining(HAND_PARAMS);

	void CONS_Monsieur_DoOtis5008A(CONS_PARAMS);
	void HAND_Monsieur_DoOtis5008A(HAND_PARAMS);

	void CONS_Monsieur_DoOtis5008E(CONS_PARAMS);
	void HAND_Monsieur_DoOtis5008E(HAND_PARAMS);

	void CONS_Monsieur_ExitDining(CONS_PARAMS);
	void HAND_Monsieur_ExitDining(HAND_PARAMS);

	void CONS_Monsieur_GoSalon(CONS_PARAMS);
	void HAND_Monsieur_GoSalon(HAND_PARAMS);

	void CONS_Monsieur_ReturnSalon(CONS_PARAMS);
	void HAND_Monsieur_ReturnSalon(HAND_PARAMS);

	void CONS_Monsieur_InSalon(CONS_PARAMS);
	void HAND_Monsieur_InSalon(HAND_PARAMS);

	void CONS_Monsieur_CompLogic(CONS_PARAMS);
	void HAND_Monsieur_CompLogic(HAND_PARAMS);

	void CONS_Monsieur_Birth(CONS_PARAMS);
	void HAND_Monsieur_Birth(HAND_PARAMS);

	void CONS_Monsieur_DoDinner(CONS_PARAMS);
	void HAND_Monsieur_DoDinner(HAND_PARAMS);

	void CONS_Monsieur_InPart1(CONS_PARAMS);
	void HAND_Monsieur_InPart1(HAND_PARAMS);

	void CONS_Monsieur_Asleep(CONS_PARAMS);
	void HAND_Monsieur_Asleep(HAND_PARAMS);

	void CONS_Monsieur_StartPart2(CONS_PARAMS);
	void HAND_Monsieur_StartPart2(HAND_PARAMS);

	void CONS_Monsieur_AtBreakfast(CONS_PARAMS);
	void HAND_Monsieur_AtBreakfast(HAND_PARAMS);

	void CONS_Monsieur_InComp2(CONS_PARAMS);
	void HAND_Monsieur_InComp2(HAND_PARAMS);

	void CONS_Monsieur_StartPart3(CONS_PARAMS);
	void HAND_Monsieur_StartPart3(HAND_PARAMS);

	void CONS_Monsieur_OpenComp3(CONS_PARAMS);
	void HAND_Monsieur_OpenComp3(HAND_PARAMS);

	void CONS_Monsieur_GoLunch(CONS_PARAMS);
	void HAND_Monsieur_GoLunch(HAND_PARAMS);

	void CONS_Monsieur_AtLunch(CONS_PARAMS);
	void HAND_Monsieur_AtLunch(HAND_PARAMS);

	void CONS_Monsieur_AfterLunchOpen(CONS_PARAMS);
	void HAND_Monsieur_AfterLunchOpen(HAND_PARAMS);

	void CONS_Monsieur_StartPart4(CONS_PARAMS);
	void HAND_Monsieur_StartPart4(HAND_PARAMS);

	void CONS_Monsieur_OpenComp4(CONS_PARAMS);
	void HAND_Monsieur_OpenComp4(HAND_PARAMS);

	void CONS_Monsieur_Dinner4(CONS_PARAMS);
	void HAND_Monsieur_Dinner4(HAND_PARAMS);

	void CONS_Monsieur_ReturnComp4(CONS_PARAMS);
	void HAND_Monsieur_ReturnComp4(HAND_PARAMS);

	void CONS_Monsieur_Asleep4(CONS_PARAMS);
	void HAND_Monsieur_Asleep4(HAND_PARAMS);

	void CONS_Monsieur_StartPart5(CONS_PARAMS);
	void HAND_Monsieur_StartPart5(HAND_PARAMS);

	void CONS_Monsieur_Prisoner(CONS_PARAMS);
	void HAND_Monsieur_Prisoner(HAND_PARAMS);

	void CONS_Monsieur_Hiding(CONS_PARAMS);
	void HAND_Monsieur_Hiding(HAND_PARAMS);

	void CONS_Monsieur_Disappear(CONS_PARAMS);
	void HAND_Monsieur_Disappear(HAND_PARAMS);

	Common::StringArray _monsieurFuncNames = {
		"DebugWalks",
		"DoDialog",
		"DoSeqOtis",
		"DoWait",
		"DoBlockSeqOtis",
		"DoCorrOtis",
		"DoEnterCorrOtis",
		"FinishSeqOtis",
		"WaitRCClear",
		"DoWalk",
		"GoDining",
		"DoOtis5008A",
		"DoOtis5008E",
		"ExitDining",
		"GoSalon",
		"ReturnSalon",
		"InSalon",
		"CompLogic",
		"Birth",
		"DoDinner",
		"InPart1",
		"Asleep",
		"StartPart2",
		"AtBreakfast",
		"InComp2",
		"StartPart3",
		"OpenComp3",
		"GoLunch",
		"AtLunch",
		"AfterLunchOpen",
		"StartPart4",
		"OpenComp4",
		"Dinner4",
		"ReturnComp4",
		"Asleep4",
		"StartPart5",
		"Prisoner",
		"Hiding",
		"Disappear"
	};

	// POLICE EVENTS

	void PoliceCall(CALL_PARAMS);

	void CONS_Police_DebugWalks(CONS_PARAMS);
	void HAND_Police_DebugWalks(HAND_PARAMS);

	void CONS_Police_Birth(CONS_PARAMS);
	void HAND_Police_Birth(HAND_PARAMS);

	void CONS_Police_DoSeqOtis(CONS_PARAMS);
	void HAND_Police_DoSeqOtis(HAND_PARAMS);

	void CONS_Police_DoDialog(CONS_PARAMS);
	void HAND_Police_DoDialog(HAND_PARAMS);

	void CONS_Police_DoDialogFullVol(CONS_PARAMS);
	void HAND_Police_DoDialogFullVol(HAND_PARAMS);

	void CONS_Police_DoWait(CONS_PARAMS);
	void HAND_Police_DoWait(HAND_PARAMS);

	void CONS_Police_SaveGame(CONS_PARAMS);
	void HAND_Police_SaveGame(HAND_PARAMS);

	void CONS_Police_DoWalk(CONS_PARAMS);
	void HAND_Police_DoWalk(HAND_PARAMS);

	void CONS_Police_DoComp(CONS_PARAMS);
	void HAND_Police_DoComp(HAND_PARAMS);

	void CONS_Police_TrappedCath(CONS_PARAMS);
	void HAND_Police_TrappedCath(HAND_PARAMS);

	void CONS_Police_ReadyToBoard(CONS_PARAMS);
	void HAND_Police_ReadyToBoard(HAND_PARAMS);

	void CONS_Police_SearchTrain(CONS_PARAMS);
	void HAND_Police_SearchTrain(HAND_PARAMS);

	void CONS_Police_SearchDone(CONS_PARAMS);
	void HAND_Police_SearchDone(HAND_PARAMS);

	void CONS_Police_StartPart2(CONS_PARAMS);
	void HAND_Police_StartPart2(HAND_PARAMS);

	void CONS_Police_StartPart3(CONS_PARAMS);
	void HAND_Police_StartPart3(HAND_PARAMS);

	void CONS_Police_StartPart4(CONS_PARAMS);
	void HAND_Police_StartPart4(HAND_PARAMS);

	void CONS_Police_StartPart5(CONS_PARAMS);
	void HAND_Police_StartPart5(HAND_PARAMS);

	Common::StringArray _policeFuncNames = {
		"DebugWalks",
		"Birth",
		"DoSeqOtis",
		"DoDialog",
		"DoDialogFullVol",
		"DoWait",
		"SaveGame",
		"DoWalk",
		"DoComp",
		"TrappedCath",
		"ReadyToBoard",
		"SearchTrain",
		"SearchDone",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// REBECCA EVENTS

	void RebeccaCall(CALL_PARAMS);

	void CONS_Rebecca_DebugWalks(CONS_PARAMS);
	void HAND_Rebecca_DebugWalks(HAND_PARAMS);

	void CONS_Rebecca_DoWait(CONS_PARAMS);
	void HAND_Rebecca_DoWait(HAND_PARAMS);

	void CONS_Rebecca_DoDialog(CONS_PARAMS);
	void HAND_Rebecca_DoDialog(HAND_PARAMS);

	void CONS_Rebecca_DoCondDialog(CONS_PARAMS);
	void HAND_Rebecca_DoCondDialog(HAND_PARAMS);

	void CONS_Rebecca_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_Rebecca_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_Rebecca_DoSeqOtis(CONS_PARAMS);
	void HAND_Rebecca_DoSeqOtis(HAND_PARAMS);

	void CONS_Rebecca_DoCorrOtis(CONS_PARAMS);
	void HAND_Rebecca_DoCorrOtis(HAND_PARAMS);

	void CONS_Rebecca_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_Rebecca_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_Rebecca_DoBriefCorrOtis(CONS_PARAMS);
	void HAND_Rebecca_DoBriefCorrOtis(HAND_PARAMS);

	void CONS_Rebecca_FinishSeqOtis(CONS_PARAMS);
	void HAND_Rebecca_FinishSeqOtis(HAND_PARAMS);

	void CONS_Rebecca_WaitRCClear(CONS_PARAMS);
	void HAND_Rebecca_WaitRCClear(HAND_PARAMS);

	void CONS_Rebecca_DoWalk(CONS_PARAMS);
	void HAND_Rebecca_DoWalk(HAND_PARAMS);

	void CONS_Rebecca_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_Rebecca_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_Rebecca_DoComplexSeqOtis(CONS_PARAMS);
	void HAND_Rebecca_DoComplexSeqOtis(HAND_PARAMS);

	void CONS_Rebecca_EnterComp(CONS_PARAMS);
	void HAND_Rebecca_EnterComp(HAND_PARAMS);

	void CONS_Rebecca_GotoDinner(CONS_PARAMS);
	void HAND_Rebecca_GotoDinner(HAND_PARAMS);

	void CONS_Rebecca_GotoSalon(CONS_PARAMS);
	void HAND_Rebecca_GotoSalon(HAND_PARAMS);

	void CONS_Rebecca_ReturnFromSalon(CONS_PARAMS);
	void HAND_Rebecca_ReturnFromSalon(HAND_PARAMS);

	void CONS_Rebecca_ReturnFromDinner(CONS_PARAMS);
	void HAND_Rebecca_ReturnFromDinner(HAND_PARAMS);

	void CONS_Rebecca_CompLogic(CONS_PARAMS);
	void HAND_Rebecca_CompLogic(HAND_PARAMS);

	void CONS_Rebecca_Birth(CONS_PARAMS);
	void HAND_Rebecca_Birth(HAND_PARAMS);

	void CONS_Rebecca_InSalon1A(CONS_PARAMS);
	void HAND_Rebecca_InSalon1A(HAND_PARAMS);

	void CONS_Rebecca_InComp1(CONS_PARAMS);
	void HAND_Rebecca_InComp1(HAND_PARAMS);

	void CONS_Rebecca_GoDinner(CONS_PARAMS);
	void HAND_Rebecca_GoDinner(HAND_PARAMS);

	void CONS_Rebecca_AfterDinner(CONS_PARAMS);
	void HAND_Rebecca_AfterDinner(HAND_PARAMS);

	void CONS_Rebecca_InSalon1B(CONS_PARAMS);
	void HAND_Rebecca_InSalon1B(HAND_PARAMS);

	void CONS_Rebecca_Asleep(CONS_PARAMS);
	void HAND_Rebecca_Asleep(HAND_PARAMS);

	void CONS_Rebecca_StartPart2(CONS_PARAMS);
	void HAND_Rebecca_StartPart2(HAND_PARAMS);

	void CONS_Rebecca_InComp2(CONS_PARAMS);
	void HAND_Rebecca_InComp2(HAND_PARAMS);

	void CONS_Rebecca_InSalon2(CONS_PARAMS);
	void HAND_Rebecca_InSalon2(HAND_PARAMS);

	void CONS_Rebecca_OpenComp2(CONS_PARAMS);
	void HAND_Rebecca_OpenComp2(HAND_PARAMS);

	void CONS_Rebecca_StartPart3(CONS_PARAMS);
	void HAND_Rebecca_StartPart3(HAND_PARAMS);

	void CONS_Rebecca_InComp3a(CONS_PARAMS);
	void HAND_Rebecca_InComp3a(HAND_PARAMS);

	void CONS_Rebecca_GoLunch(CONS_PARAMS);
	void HAND_Rebecca_GoLunch(HAND_PARAMS);

	void CONS_Rebecca_InComp3b(CONS_PARAMS);
	void HAND_Rebecca_InComp3b(HAND_PARAMS);

	void CONS_Rebecca_GoSalon(CONS_PARAMS);
	void HAND_Rebecca_GoSalon(HAND_PARAMS);

	void CONS_Rebecca_ClosedComp(CONS_PARAMS);
	void HAND_Rebecca_ClosedComp(HAND_PARAMS);

	void CONS_Rebecca_GoConcert(CONS_PARAMS);
	void HAND_Rebecca_GoConcert(HAND_PARAMS);

	void CONS_Rebecca_Concert(CONS_PARAMS);
	void HAND_Rebecca_Concert(HAND_PARAMS);

	void CONS_Rebecca_LeaveConcert(CONS_PARAMS);
	void HAND_Rebecca_LeaveConcert(HAND_PARAMS);

	void CONS_Rebecca_SiestaTime(CONS_PARAMS);
	void HAND_Rebecca_SiestaTime(HAND_PARAMS);

	void CONS_Rebecca_StartPart4(CONS_PARAMS);
	void HAND_Rebecca_StartPart4(HAND_PARAMS);

	void CONS_Rebecca_InComp4(CONS_PARAMS);
	void HAND_Rebecca_InComp4(HAND_PARAMS);

	void CONS_Rebecca_GoDinner4(CONS_PARAMS);
	void HAND_Rebecca_GoDinner4(HAND_PARAMS);

	void CONS_Rebecca_Asleep4(CONS_PARAMS);
	void HAND_Rebecca_Asleep4(HAND_PARAMS);

	void CONS_Rebecca_StartPart5(CONS_PARAMS);
	void HAND_Rebecca_StartPart5(HAND_PARAMS);

	void CONS_Rebecca_Prisoner(CONS_PARAMS);
	void HAND_Rebecca_Prisoner(HAND_PARAMS);

	void CONS_Rebecca_Hiding(CONS_PARAMS);
	void HAND_Rebecca_Hiding(HAND_PARAMS);

	void CONS_Rebecca_Disappear(CONS_PARAMS);
	void HAND_Rebecca_Disappear(HAND_PARAMS);

	Common::StringArray _rebeccaFuncNames = {
		"DebugWalks",
		"DoWait",
		"DoDialog",
		"DoCondDialog",
		"DoJoinedSeqOtis",
		"DoSeqOtis",
		"DoCorrOtis",
		"DoEnterCorrOtis",
		"DoBriefCorrOtis",
		"FinishSeqOtis",
		"WaitRCClear",
		"DoWalk",
		"DoBlockSeqOtis",
		"DoComplexSeqOtis",
		"EnterComp",
		"GotoDinner",
		"GotoSalon",
		"ReturnFromSalon",
		"ReturnFromDinner",
		"CompLogic",
		"Birth",
		"InSalon1A",
		"InComp1",
		"GoDinner",
		"AfterDinner",
		"InSalon1B",
		"Asleep",
		"StartPart2",
		"InComp2",
		"InSalon2",
		"OpenComp2",
		"StartPart3",
		"InComp3a",
		"GoLunch",
		"InComp3b",
		"GoSalon",
		"ClosedComp",
		"GoConcert",
		"Concert",
		"LeaveConcert",
		"SiestaTime",
		"StartPart4",
		"InComp4",
		"GoDinner4",
		"Asleep4",
		"StartPart5",
		"Prisoner",
		"Hiding",
		"Disappear"
	};


	// SALKO EVENTS

	void SalkoCall(CALL_PARAMS);

	void CONS_Salko_DebugWalks(CONS_PARAMS);
	void HAND_Salko_DebugWalks(HAND_PARAMS);

	void CONS_Salko_DoCorrOtis(CONS_PARAMS);
	void HAND_Salko_DoCorrOtis(HAND_PARAMS);

	void CONS_Salko_DoSeqOtis(CONS_PARAMS);
	void HAND_Salko_DoSeqOtis(HAND_PARAMS);

	void CONS_Salko_DoWalk(CONS_PARAMS);
	void HAND_Salko_DoWalk(HAND_PARAMS);

	void CONS_Salko_DoWait(CONS_PARAMS);
	void HAND_Salko_DoWait(HAND_PARAMS);

	void CONS_Salko_SaveGame(CONS_PARAMS);
	void HAND_Salko_SaveGame(HAND_PARAMS);

	void CONS_Salko_DoWalkBehind(CONS_PARAMS);
	void HAND_Salko_DoWalkBehind(HAND_PARAMS);

	void CONS_Salko_HomeTogether(CONS_PARAMS);
	void HAND_Salko_HomeTogether(HAND_PARAMS);

	void CONS_Salko_Birth(CONS_PARAMS);
	void HAND_Salko_Birth(HAND_PARAMS);

	void CONS_Salko_WithIvo(CONS_PARAMS);
	void HAND_Salko_WithIvo(HAND_PARAMS);

	void CONS_Salko_Asleep(CONS_PARAMS);
	void HAND_Salko_Asleep(HAND_PARAMS);

	void CONS_Salko_StartPart2(CONS_PARAMS);
	void HAND_Salko_StartPart2(HAND_PARAMS);

	void CONS_Salko_JoinIvo(CONS_PARAMS);
	void HAND_Salko_JoinIvo(HAND_PARAMS);

	void CONS_Salko_StartPart3(CONS_PARAMS);
	void HAND_Salko_StartPart3(HAND_PARAMS);

	void CONS_Salko_InComp(CONS_PARAMS);
	void HAND_Salko_InComp(HAND_PARAMS);

	void CONS_Salko_EavesdropAnna(CONS_PARAMS);
	void HAND_Salko_EavesdropAnna(HAND_PARAMS);

	void CONS_Salko_TellMilos(CONS_PARAMS);
	void HAND_Salko_TellMilos(HAND_PARAMS);

	void CONS_Salko_StartPart4(CONS_PARAMS);
	void HAND_Salko_StartPart4(HAND_PARAMS);

	void CONS_Salko_WithIvo4(CONS_PARAMS);
	void HAND_Salko_WithIvo4(HAND_PARAMS);

	void CONS_Salko_InComp4(CONS_PARAMS);
	void HAND_Salko_InComp4(HAND_PARAMS);

	void CONS_Salko_Hiding(CONS_PARAMS);
	void HAND_Salko_Hiding(HAND_PARAMS);

	void CONS_Salko_EndPart4(CONS_PARAMS);
	void HAND_Salko_EndPart4(HAND_PARAMS);

	void CONS_Salko_StartPart5(CONS_PARAMS);
	void HAND_Salko_StartPart5(HAND_PARAMS);

	void CONS_Salko_Guarding(CONS_PARAMS);
	void HAND_Salko_Guarding(HAND_PARAMS);

	void CONS_Salko_Disappear(CONS_PARAMS);
	void HAND_Salko_Disappear(HAND_PARAMS);

	Common::StringArray _salkoFuncNames = {
		"DebugWalks",
		"DoCorrOtis",
		"DoSeqOtis",
		"DoWalk",
		"DoWait",
		"SaveGame",
		"DoWalkBehind",
		"HomeTogether",
		"Birth",
		"WithIvo",
		"Asleep",
		"StartPart2",
		"JoinIvo",
		"StartPart3",
		"InComp",
		"EavesdropAnna",
		"TellMilos",
		"StartPart4",
		"WithIvo4",
		"InComp4",
		"Hiding",
		"EndPart4",
		"StartPart5",
		"Guarding",
		"Disappear"
	};


	// SOPHIE EVENTS

	void SophieCall(CALL_PARAMS);

	void CONS_Sophie_DebugWalks(CONS_PARAMS);
	void HAND_Sophie_DebugWalks(HAND_PARAMS);

	void CONS_Sophie_DoWalkBehind(CONS_PARAMS);
	void HAND_Sophie_DoWalkBehind(HAND_PARAMS);

	void CONS_Sophie_WithRebecca(CONS_PARAMS);
	void HAND_Sophie_WithRebecca(HAND_PARAMS);

	void CONS_Sophie_Birth(CONS_PARAMS);
	void HAND_Sophie_Birth(HAND_PARAMS);

	void CONS_Sophie_Asleep(CONS_PARAMS);
	void HAND_Sophie_Asleep(HAND_PARAMS);

	void CONS_Sophie_StartPart2(CONS_PARAMS);
	void HAND_Sophie_StartPart2(HAND_PARAMS);

	void CONS_Sophie_StartPart3(CONS_PARAMS);
	void HAND_Sophie_StartPart3(HAND_PARAMS);

	void CONS_Sophie_StartPart4(CONS_PARAMS);
	void HAND_Sophie_StartPart4(HAND_PARAMS);

	void CONS_Sophie_Asleep4(CONS_PARAMS);
	void HAND_Sophie_Asleep4(HAND_PARAMS);

	void CONS_Sophie_StartPart5(CONS_PARAMS);
	void HAND_Sophie_StartPart5(HAND_PARAMS);

	void CONS_Sophie_Prisoner(CONS_PARAMS);
	void HAND_Sophie_Prisoner(HAND_PARAMS);

	void CONS_Sophie_Free(CONS_PARAMS);
	void HAND_Sophie_Free(HAND_PARAMS);

	Common::StringArray _sophieFuncNames = {
		"DebugWalks",
		"DoWalkBehind",
		"WithRebecca",
		"Birth",
		"Asleep",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"Asleep4",
		"StartPart5",
		"Prisoner",
		"Free"
	};


	// TABLEA EVENTS

	void CONS_TableA_Birth(CONS_PARAMS);
	void HAND_TableA_Birth(HAND_PARAMS);

	void CONS_TableA_StartPart2(CONS_PARAMS);
	void HAND_TableA_StartPart2(HAND_PARAMS);

	void CONS_TableA_StartPart3(CONS_PARAMS);
	void HAND_TableA_StartPart3(HAND_PARAMS);

	void CONS_TableA_StartPart4(CONS_PARAMS);
	void HAND_TableA_StartPart4(HAND_PARAMS);

	void CONS_TableA_StartPart5(CONS_PARAMS);
	void HAND_TableA_StartPart5(HAND_PARAMS);

	void CONS_TableA_Idling(CONS_PARAMS);
	void HAND_TableA_Idling(HAND_PARAMS);

	// TABLEB EVENTS

	void CONS_TableB_Birth(CONS_PARAMS);
	void HAND_TableB_Birth(HAND_PARAMS);

	void CONS_TableB_StartPart2(CONS_PARAMS);
	void HAND_TableB_StartPart2(HAND_PARAMS);

	void CONS_TableB_StartPart3(CONS_PARAMS);
	void HAND_TableB_StartPart3(HAND_PARAMS);

	void CONS_TableB_StartPart4(CONS_PARAMS);
	void HAND_TableB_StartPart4(HAND_PARAMS);

	void CONS_TableB_StartPart5(CONS_PARAMS);
	void HAND_TableB_StartPart5(HAND_PARAMS);

	void CONS_TableB_Idling(CONS_PARAMS);
	void HAND_TableB_Idling(HAND_PARAMS);

	// TABLEC EVENTS

	void CONS_TableC_Birth(CONS_PARAMS);
	void HAND_TableC_Birth(HAND_PARAMS);

	void CONS_TableC_StartPart2(CONS_PARAMS);
	void HAND_TableC_StartPart2(HAND_PARAMS);

	void CONS_TableC_StartPart3(CONS_PARAMS);
	void HAND_TableC_StartPart3(HAND_PARAMS);

	void CONS_TableC_StartPart4(CONS_PARAMS);
	void HAND_TableC_StartPart4(HAND_PARAMS);

	void CONS_TableC_StartPart5(CONS_PARAMS);
	void HAND_TableC_StartPart5(HAND_PARAMS);

	void CONS_TableC_Idling(CONS_PARAMS);
	void HAND_TableC_Idling(HAND_PARAMS);

	// TABLED EVENTS

	void CONS_TableD_Birth(CONS_PARAMS);
	void HAND_TableD_Birth(HAND_PARAMS);

	void CONS_TableD_StartPart2(CONS_PARAMS);
	void HAND_TableD_StartPart2(HAND_PARAMS);

	void CONS_TableD_StartPart3(CONS_PARAMS);
	void HAND_TableD_StartPart3(HAND_PARAMS);

	void CONS_TableD_StartPart4(CONS_PARAMS);
	void HAND_TableD_StartPart4(HAND_PARAMS);

	void CONS_TableD_StartPart5(CONS_PARAMS);
	void HAND_TableD_StartPart5(HAND_PARAMS);

	void CONS_TableD_Idling(CONS_PARAMS);
	void HAND_TableD_Idling(HAND_PARAMS);

	// TABLEE EVENTS

	void CONS_TableE_Birth(CONS_PARAMS);
	void HAND_TableE_Birth(HAND_PARAMS);

	void CONS_TableE_StartPart2(CONS_PARAMS);
	void HAND_TableE_StartPart2(HAND_PARAMS);

	void CONS_TableE_StartPart3(CONS_PARAMS);
	void HAND_TableE_StartPart3(HAND_PARAMS);

	void CONS_TableE_StartPart4(CONS_PARAMS);
	void HAND_TableE_StartPart4(HAND_PARAMS);

	void CONS_TableE_StartPart5(CONS_PARAMS);
	void HAND_TableE_StartPart5(HAND_PARAMS);

	void CONS_TableE_Idling(CONS_PARAMS);
	void HAND_TableE_Idling(HAND_PARAMS);

	// TABLEF EVENTS

	void CONS_TableF_Birth(CONS_PARAMS);
	void HAND_TableF_Birth(HAND_PARAMS);

	void CONS_TableF_StartPart2(CONS_PARAMS);
	void HAND_TableF_StartPart2(HAND_PARAMS);

	void CONS_TableF_StartPart3(CONS_PARAMS);
	void HAND_TableF_StartPart3(HAND_PARAMS);

	void CONS_TableF_StartPart4(CONS_PARAMS);
	void HAND_TableF_StartPart4(HAND_PARAMS);

	void CONS_TableF_StartPart5(CONS_PARAMS);
	void HAND_TableF_StartPart5(HAND_PARAMS);

	void CONS_TableF_Idling(CONS_PARAMS);
	void HAND_TableF_Idling(HAND_PARAMS);

	Common::StringArray _tablesFuncNames = {
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5",
		"Idling"
	};

	// TATIANA EVENTS

	void TatianaCall(CALL_PARAMS);

	void CONS_Tatiana_DebugWalks(CONS_PARAMS);
	void HAND_Tatiana_DebugWalks(HAND_PARAMS);

	void CONS_Tatiana_DoDialog(CONS_PARAMS);
	void HAND_Tatiana_DoDialog(HAND_PARAMS);

	void CONS_Tatiana_DoSeqOtis(CONS_PARAMS);
	void HAND_Tatiana_DoSeqOtis(HAND_PARAMS);

	void CONS_Tatiana_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_Tatiana_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_Tatiana_DoCorrOtis(CONS_PARAMS);
	void HAND_Tatiana_DoCorrOtis(HAND_PARAMS);

	void CONS_Tatiana_DoEnterCorrOtis(CONS_PARAMS);
	void HAND_Tatiana_DoEnterCorrOtis(HAND_PARAMS);

	void CONS_Tatiana_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_Tatiana_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_Tatiana_FinishSeqOtis(CONS_PARAMS);
	void HAND_Tatiana_FinishSeqOtis(HAND_PARAMS);

	void CONS_Tatiana_DoWaitRealTime(CONS_PARAMS);
	void HAND_Tatiana_DoWaitRealTime(HAND_PARAMS);

	void CONS_Tatiana_DoWait(CONS_PARAMS);
	void HAND_Tatiana_DoWait(HAND_PARAMS);

	void CONS_Tatiana_WaitRCClear(CONS_PARAMS);
	void HAND_Tatiana_WaitRCClear(HAND_PARAMS);

	void CONS_Tatiana_SaveGame(CONS_PARAMS);
	void HAND_Tatiana_SaveGame(HAND_PARAMS);

	void CONS_Tatiana_DoWalk(CONS_PARAMS);
	void HAND_Tatiana_DoWalk(HAND_PARAMS);

	void CONS_Tatiana_EnterComp(CONS_PARAMS);
	void HAND_Tatiana_EnterComp(HAND_PARAMS);

	void CONS_Tatiana_ExitComp(CONS_PARAMS);
	void HAND_Tatiana_ExitComp(HAND_PARAMS);

	void CONS_Tatiana_CompLogic(CONS_PARAMS);
	void HAND_Tatiana_CompLogic(HAND_PARAMS);

	void CONS_Tatiana_Birth(CONS_PARAMS);
	void HAND_Tatiana_Birth(HAND_PARAMS);

	void CONS_Tatiana_DoSpecialSalonWalk(CONS_PARAMS);
	void HAND_Tatiana_DoSpecialSalonWalk(HAND_PARAMS);

	void CONS_Tatiana_AtDinner(CONS_PARAMS);
	void HAND_Tatiana_AtDinner(HAND_PARAMS);

	void CONS_Tatiana_ExitDining(CONS_PARAMS);
	void HAND_Tatiana_ExitDining(HAND_PARAMS);

	void CONS_Tatiana_ReturnToComp(CONS_PARAMS);
	void HAND_Tatiana_ReturnToComp(HAND_PARAMS);

	void CONS_Tatiana_GetSomeAir(CONS_PARAMS);
	void HAND_Tatiana_GetSomeAir(HAND_PARAMS);

	void CONS_Tatiana_ReturnToCompAgain(CONS_PARAMS);
	void HAND_Tatiana_ReturnToCompAgain(HAND_PARAMS);

	void CONS_Tatiana_Asleep(CONS_PARAMS);
	void HAND_Tatiana_Asleep(HAND_PARAMS);

	void CONS_Tatiana_StartPart2(CONS_PARAMS);
	void HAND_Tatiana_StartPart2(HAND_PARAMS);

	void CONS_Tatiana_EatingBreakfast(CONS_PARAMS);
	void HAND_Tatiana_EatingBreakfast(HAND_PARAMS);

	void CONS_Tatiana_BreakfastClick(CONS_PARAMS);
	void HAND_Tatiana_BreakfastClick(HAND_PARAMS);

	void CONS_Tatiana_JoinAlexei(CONS_PARAMS);
	void HAND_Tatiana_JoinAlexei(HAND_PARAMS);

	void CONS_Tatiana_LeaveBreakfast(CONS_PARAMS);
	void HAND_Tatiana_LeaveBreakfast(HAND_PARAMS);

	void CONS_Tatiana_ReturnComp(CONS_PARAMS);
	void HAND_Tatiana_ReturnComp(HAND_PARAMS);

	void CONS_Tatiana_StartPart3(CONS_PARAMS);
	void HAND_Tatiana_StartPart3(HAND_PARAMS);

	void CONS_Tatiana_PlayChess(CONS_PARAMS);
	void HAND_Tatiana_PlayChess(HAND_PARAMS);

	void CONS_Tatiana_ReturnComp3(CONS_PARAMS);
	void HAND_Tatiana_ReturnComp3(HAND_PARAMS);

	void CONS_Tatiana_BeforeConcert(CONS_PARAMS);
	void HAND_Tatiana_BeforeConcert(HAND_PARAMS);

	void CONS_Tatiana_Concert(CONS_PARAMS);
	void HAND_Tatiana_Concert(HAND_PARAMS);

	void CONS_Tatiana_LeaveConcert(CONS_PARAMS);
	void HAND_Tatiana_LeaveConcert(HAND_PARAMS);

	void CONS_Tatiana_AfterConcert(CONS_PARAMS);
	void HAND_Tatiana_AfterConcert(HAND_PARAMS);

	void CONS_Tatiana_CryAnna(CONS_PARAMS);
	void HAND_Tatiana_CryAnna(HAND_PARAMS);

	void CONS_Tatiana_Upset(CONS_PARAMS);
	void HAND_Tatiana_Upset(HAND_PARAMS);

	void CONS_Tatiana_DoConcertWalk(CONS_PARAMS);
	void HAND_Tatiana_DoConcertWalk(HAND_PARAMS);

	void CONS_Tatiana_TrapCath(CONS_PARAMS);
	void HAND_Tatiana_TrapCath(HAND_PARAMS);

	void CONS_Tatiana_WalkSniffle(CONS_PARAMS);
	void HAND_Tatiana_WalkSniffle(HAND_PARAMS);

	void CONS_Tatiana_StartPart4(CONS_PARAMS);
	void HAND_Tatiana_StartPart4(HAND_PARAMS);

	void CONS_Tatiana_InComp(CONS_PARAMS);
	void HAND_Tatiana_InComp(HAND_PARAMS);

	void CONS_Tatiana_MeetAlexei(CONS_PARAMS);
	void HAND_Tatiana_MeetAlexei(HAND_PARAMS);

	void CONS_Tatiana_WithAlexei(CONS_PARAMS);
	void HAND_Tatiana_WithAlexei(HAND_PARAMS);

	void CONS_Tatiana_Thinking(CONS_PARAMS);
	void HAND_Tatiana_Thinking(HAND_PARAMS);

	void CONS_Tatiana_SeekCath(CONS_PARAMS);
	void HAND_Tatiana_SeekCath(HAND_PARAMS);

	void CONS_Tatiana_Asleep4(CONS_PARAMS);
	void HAND_Tatiana_Asleep4(HAND_PARAMS);

	void CONS_Tatiana_AlexeiDead(CONS_PARAMS);
	void HAND_Tatiana_AlexeiDead(HAND_PARAMS);

	void CONS_Tatiana_Calm(CONS_PARAMS);
	void HAND_Tatiana_Calm(HAND_PARAMS);

	void CONS_Tatiana_StartPart5(CONS_PARAMS);
	void HAND_Tatiana_StartPart5(HAND_PARAMS);

	void CONS_Tatiana_Trapped(CONS_PARAMS);
	void HAND_Tatiana_Trapped(HAND_PARAMS);

	void CONS_Tatiana_Autistic(CONS_PARAMS);
	void HAND_Tatiana_Autistic(HAND_PARAMS);

	void CONS_Tatiana_Asleep5(CONS_PARAMS);
	void HAND_Tatiana_Asleep5(HAND_PARAMS);

	Common::StringArray _tatianaFuncNames = {
		"DebugWalks",
		"DoDialog",
		"DoSeqOtis",
		"DoBlockSeqOtis",
		"DoCorrOtis",
		"DoEnterCorrOtis",
		"DoJoinedSeqOtis",
		"FinishSeqOtis",
		"DoWaitRealTime",
		"DoWait",
		"WaitRCClear",
		"SaveGame",
		"DoWalk",
		"EnterComp",
		"ExitComp",
		"CompLogic",
		"Birth",
		"DoSpecialSalonWalk",
		"AtDinner",
		"ExitDining",
		"ReturnToComp",
		"GetSomeAir",
		"ReturnToCompAgain",
		"Asleep",
		"StartPart2",
		"EatingBreakfast",
		"BreakfastClick",
		"JoinAlexei",
		"LeaveBreakfast",
		"ReturnComp",
		"StartPart3",
		"PlayChess",
		"ReturnComp3",
		"BeforeConcert",
		"Concert",
		"LeaveConcert",
		"AfterConcert",
		"CryAnna",
		"Upset",
		"DoConcertWalk",
		"TrapCath",
		"WalkSniffle",
		"StartPart4",
		"InComp",
		"MeetAlexei",
		"WithAlexei",
		"Thinking",
		"SeekCath",
		"Asleep4",
		"AlexeiDead",
		"Calm",
		"StartPart5",
		"Trapped",
		"Autistic",
		"Asleep5"
	};


	// TRAINM EVENTS

	void TrainMCall(CALL_PARAMS);

	void CONS_TrainM_DebugWalks(CONS_PARAMS);
	void HAND_TrainM_DebugWalks(HAND_PARAMS);

	void CONS_TrainM_DoSeqOtis(CONS_PARAMS);
	void HAND_TrainM_DoSeqOtis(HAND_PARAMS);

	void CONS_TrainM_FinishSeqOtis(CONS_PARAMS);
	void HAND_TrainM_FinishSeqOtis(HAND_PARAMS);

	void CONS_TrainM_DoDialog(CONS_PARAMS);
	void HAND_TrainM_DoDialog(HAND_PARAMS);

	void CONS_TrainM_DoDialogFullVol(CONS_PARAMS);
	void HAND_TrainM_DoDialogFullVol(HAND_PARAMS);

	void CONS_TrainM_WaitRCClear(CONS_PARAMS);
	void HAND_TrainM_WaitRCClear(HAND_PARAMS);

	void CONS_TrainM_SaveGame(CONS_PARAMS);
	void HAND_TrainM_SaveGame(HAND_PARAMS);

	void CONS_TrainM_DoWalk(CONS_PARAMS);
	void HAND_TrainM_DoWalk(HAND_PARAMS);

	void CONS_TrainM_Announce(CONS_PARAMS);
	void HAND_TrainM_Announce(HAND_PARAMS);

	void CONS_TrainM_DoAnnounceWalk(CONS_PARAMS);
	void HAND_TrainM_DoAnnounceWalk(HAND_PARAMS);

	void CONS_TrainM_WalkBackToOffice(CONS_PARAMS);
	void HAND_TrainM_WalkBackToOffice(HAND_PARAMS);

	void CONS_TrainM_LeaveOffice(CONS_PARAMS);
	void HAND_TrainM_LeaveOffice(HAND_PARAMS);

	void CONS_TrainM_KickCathOut(CONS_PARAMS);
	void HAND_TrainM_KickCathOut(HAND_PARAMS);

	void CONS_TrainM_DoWait(CONS_PARAMS);
	void HAND_TrainM_DoWait(HAND_PARAMS);

	void CONS_TrainM_DoCond(CONS_PARAMS);
	void HAND_TrainM_DoCond(HAND_PARAMS);

	void CONS_TrainM_DoCondDoubl(CONS_PARAMS);
	void HAND_TrainM_DoCondDoubl(HAND_PARAMS);

	void CONS_TrainM_DoCond1ListMessage(CONS_PARAMS);
	void HAND_TrainM_DoCond1ListMessage(HAND_PARAMS);

	void CONS_TrainM_Birth(CONS_PARAMS);
	void HAND_TrainM_Birth(HAND_PARAMS);

	void CONS_TrainM_DoMadeBedsMessages(CONS_PARAMS);
	void HAND_TrainM_DoMadeBedsMessages(HAND_PARAMS);

	void CONS_TrainM_DoMissingListMessages(CONS_PARAMS);
	void HAND_TrainM_DoMissingListMessages(HAND_PARAMS);

	void CONS_TrainM_DoAfterPoliceMessages(CONS_PARAMS);
	void HAND_TrainM_DoAfterPoliceMessages(HAND_PARAMS);

	void CONS_TrainM_DoHWMessage(CONS_PARAMS);
	void HAND_TrainM_DoHWMessage(HAND_PARAMS);

	void CONS_TrainM_DoSeizure(CONS_PARAMS);
	void HAND_TrainM_DoSeizure(HAND_PARAMS);

	void CONS_TrainM_DoPoliceDoneDialog(CONS_PARAMS);
	void HAND_TrainM_DoPoliceDoneDialog(HAND_PARAMS);

	void CONS_TrainM_BoardPolice(CONS_PARAMS);
	void HAND_TrainM_BoardPolice(HAND_PARAMS);

	void CONS_TrainM_InOffice(CONS_PARAMS);
	void HAND_TrainM_InOffice(HAND_PARAMS);

	void CONS_TrainM_StartPart2(CONS_PARAMS);
	void HAND_TrainM_StartPart2(HAND_PARAMS);

	void CONS_TrainM_InPart2(CONS_PARAMS);
	void HAND_TrainM_InPart2(HAND_PARAMS);

	void CONS_TrainM_StartPart3(CONS_PARAMS);
	void HAND_TrainM_StartPart3(HAND_PARAMS);

	void CONS_TrainM_VisitCond2(CONS_PARAMS);
	void HAND_TrainM_VisitCond2(HAND_PARAMS);

	void CONS_TrainM_DoDogProblem(CONS_PARAMS);
	void HAND_TrainM_DoDogProblem(HAND_PARAMS);

	void CONS_TrainM_AnnounceVienna(CONS_PARAMS);
	void HAND_TrainM_AnnounceVienna(HAND_PARAMS);

	void CONS_TrainM_OnRearPlatform(CONS_PARAMS);
	void HAND_TrainM_OnRearPlatform(HAND_PARAMS);

	void CONS_TrainM_InPart3(CONS_PARAMS);
	void HAND_TrainM_InPart3(HAND_PARAMS);

	void CONS_TrainM_VisitKron(CONS_PARAMS);
	void HAND_TrainM_VisitKron(HAND_PARAMS);

	void CONS_TrainM_StartPart4(CONS_PARAMS);
	void HAND_TrainM_StartPart4(HAND_PARAMS);

	void CONS_TrainM_InOffice4(CONS_PARAMS);
	void HAND_TrainM_InOffice4(HAND_PARAMS);

	void CONS_TrainM_Conferring(CONS_PARAMS);
	void HAND_TrainM_Conferring(HAND_PARAMS);

	void CONS_TrainM_StartPart5(CONS_PARAMS);
	void HAND_TrainM_StartPart5(HAND_PARAMS);

	void CONS_TrainM_Prisoner(CONS_PARAMS);
	void HAND_TrainM_Prisoner(HAND_PARAMS);

	void CONS_TrainM_InCharge(CONS_PARAMS);
	void HAND_TrainM_InCharge(HAND_PARAMS);

	void CONS_TrainM_Disappear(CONS_PARAMS);
	void HAND_TrainM_Disappear(HAND_PARAMS);

	Common::StringArray _trainMFuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"FinishSeqOtis",
		"DoDialog",
		"DoDialogFullVol",
		"WaitRCClear",
		"SaveGame",
		"DoWalk",
		"Announce",
		"DoAnnounceWalk",
		"WalkBackToOffice",
		"LeaveOffice",
		"KickCathOut",
		"DoWait",
		"DoCond",
		"DoCondDoubl",
		"DoCond1ListMessage",
		"Birth",
		"DoMadeBedsMessages",
		"DoMissingListMessages",
		"DoAfterPoliceMessages",
		"DoHWMessage",
		"DoSeizure",
		"DoPoliceDoneDialog",
		"BoardPolice",
		"InOffice",
		"StartPart2",
		"InPart2",
		"StartPart3",
		"VisitCond2",
		"DoDogProblem",
		"AnnounceVienna",
		"OnRearPlatform",
		"InPart3",
		"VisitKron",
		"StartPart4",
		"InOffice4",
		"Conferring",
		"StartPart5",
		"Prisoner",
		"InCharge",
		"Disappear"
	};


	// VASSILI EVENTS

	void VassiliCall(CALL_PARAMS);

	void CONS_Vassili_DebugWalks(CONS_PARAMS);
	void HAND_Vassili_DebugWalks(HAND_PARAMS);

	void CONS_Vassili_DoSeqOtis(CONS_PARAMS);
	void HAND_Vassili_DoSeqOtis(HAND_PARAMS);

	void CONS_Vassili_SaveGame(CONS_PARAMS);
	void HAND_Vassili_SaveGame(HAND_PARAMS);

	void CONS_Vassili_Birth(CONS_PARAMS);
	void HAND_Vassili_Birth(HAND_PARAMS);

	void CONS_Vassili_WithTatiana(CONS_PARAMS);
	void HAND_Vassili_WithTatiana(HAND_PARAMS);

	void CONS_Vassili_InBed(CONS_PARAMS);
	void HAND_Vassili_InBed(HAND_PARAMS);

	void CONS_Vassili_InBed2(CONS_PARAMS);
	void HAND_Vassili_InBed2(HAND_PARAMS);

	void CONS_Vassili_HaveSeizureNow(CONS_PARAMS);
	void HAND_Vassili_HaveSeizureNow(HAND_PARAMS);

	void CONS_Vassili_HavingSeizure(CONS_PARAMS);
	void HAND_Vassili_HavingSeizure(HAND_PARAMS);

	void CONS_Vassili_CathArrives(CONS_PARAMS);
	void HAND_Vassili_CathArrives(HAND_PARAMS);

	void CONS_Vassili_AsleepAgain(CONS_PARAMS);
	void HAND_Vassili_AsleepAgain(HAND_PARAMS);

	void CONS_Vassili_StartPart2(CONS_PARAMS);
	void HAND_Vassili_StartPart2(HAND_PARAMS);

	void CONS_Vassili_InPart2(CONS_PARAMS);
	void HAND_Vassili_InPart2(HAND_PARAMS);

	void CONS_Vassili_StartPart3(CONS_PARAMS);
	void HAND_Vassili_StartPart3(HAND_PARAMS);

	void CONS_Vassili_Asleep(CONS_PARAMS);
	void HAND_Vassili_Asleep(HAND_PARAMS);

	void CONS_Vassili_StartPart4(CONS_PARAMS);
	void HAND_Vassili_StartPart4(HAND_PARAMS);

	void CONS_Vassili_InPart4(CONS_PARAMS);
	void HAND_Vassili_InPart4(HAND_PARAMS);

	void CONS_Vassili_StartPart5(CONS_PARAMS);
	void HAND_Vassili_StartPart5(HAND_PARAMS);

	Common::StringArray _vassiliFuncNames = {
		"DebugWalks",
		"DoSeqOtis",
		"SaveGame",
		"Birth",
		"WithTatiana",
		"InBed",
		"InBed2",
		"HaveSeizureNow",
		"HavingSeizure",
		"CathArrives",
		"AsleepAgain",
		"StartPart2",
		"InPart2",
		"StartPart3",
		"Asleep",
		"StartPart4",
		"InPart4",
		"StartPart5"
	};


	// VESNA EVENTS

	void VesnaCall(CALL_PARAMS);

	void CONS_Vesna_DebugWalks(CONS_PARAMS);
	void HAND_Vesna_DebugWalks(HAND_PARAMS);

	void CONS_Vesna_DoDialog(CONS_PARAMS);
	void HAND_Vesna_DoDialog(HAND_PARAMS);

	void CONS_Vesna_DoCorrOtis(CONS_PARAMS);
	void HAND_Vesna_DoCorrOtis(HAND_PARAMS);

	void CONS_Vesna_DoSeqOtis(CONS_PARAMS);
	void HAND_Vesna_DoSeqOtis(HAND_PARAMS);

	void CONS_Vesna_DoWalk(CONS_PARAMS);
	void HAND_Vesna_DoWalk(HAND_PARAMS);

	void CONS_Vesna_DoWait(CONS_PARAMS);
	void HAND_Vesna_DoWait(HAND_PARAMS);

	void CONS_Vesna_DoWalkBehind(CONS_PARAMS);
	void HAND_Vesna_DoWalkBehind(HAND_PARAMS);

	void CONS_Vesna_WaitRCClear(CONS_PARAMS);
	void HAND_Vesna_WaitRCClear(HAND_PARAMS);

	void CONS_Vesna_FinishSeqOtis(CONS_PARAMS);
	void HAND_Vesna_FinishSeqOtis(HAND_PARAMS);

	void CONS_Vesna_SaveGame(CONS_PARAMS);
	void HAND_Vesna_SaveGame(HAND_PARAMS);

	void CONS_Vesna_HomeAlone(CONS_PARAMS);
	void HAND_Vesna_HomeAlone(HAND_PARAMS);

	void CONS_Vesna_Birth(CONS_PARAMS);
	void HAND_Vesna_Birth(HAND_PARAMS);

	void CONS_Vesna_WithMilos(CONS_PARAMS);
	void HAND_Vesna_WithMilos(HAND_PARAMS);

	void CONS_Vesna_HomeTogether(CONS_PARAMS);
	void HAND_Vesna_HomeTogether(HAND_PARAMS);

	void CONS_Vesna_Asleep(CONS_PARAMS);
	void HAND_Vesna_Asleep(HAND_PARAMS);

	void CONS_Vesna_StartPart2(CONS_PARAMS);
	void HAND_Vesna_StartPart2(HAND_PARAMS);

	void CONS_Vesna_InPart2(CONS_PARAMS);
	void HAND_Vesna_InPart2(HAND_PARAMS);

	void CONS_Vesna_CheckTrain(CONS_PARAMS);
	void HAND_Vesna_CheckTrain(HAND_PARAMS);

	void CONS_Vesna_StartPart3(CONS_PARAMS);
	void HAND_Vesna_StartPart3(HAND_PARAMS);

	void CONS_Vesna_InComp(CONS_PARAMS);
	void HAND_Vesna_InComp(HAND_PARAMS);

	void CONS_Vesna_TakeAWalk(CONS_PARAMS);
	void HAND_Vesna_TakeAWalk(HAND_PARAMS);

	void CONS_Vesna_KillAnna(CONS_PARAMS);
	void HAND_Vesna_KillAnna(HAND_PARAMS);

	void CONS_Vesna_KilledAnna(CONS_PARAMS);
	void HAND_Vesna_KilledAnna(HAND_PARAMS);

	void CONS_Vesna_StartPart4(CONS_PARAMS);
	void HAND_Vesna_StartPart4(HAND_PARAMS);

	void CONS_Vesna_Exit(CONS_PARAMS);
	void HAND_Vesna_Exit(HAND_PARAMS);

	void CONS_Vesna_Done(CONS_PARAMS);
	void HAND_Vesna_Done(HAND_PARAMS);

	void CONS_Vesna_EndPart4(CONS_PARAMS);
	void HAND_Vesna_EndPart4(HAND_PARAMS);

	void CONS_Vesna_StartPart5(CONS_PARAMS);
	void HAND_Vesna_StartPart5(HAND_PARAMS);

	void CONS_Vesna_Guarding(CONS_PARAMS);
	void HAND_Vesna_Guarding(HAND_PARAMS);

	void CONS_Vesna_Climbing(CONS_PARAMS);
	void HAND_Vesna_Climbing(HAND_PARAMS);

	void CONS_Vesna_Disappear(CONS_PARAMS);
	void HAND_Vesna_Disappear(HAND_PARAMS);

	Common::StringArray _vesnaFuncNames = {
		"DebugWalks",
		"DoDialog",
		"DoCorrOtis",
		"DoSeqOtis",
		"DoWalk",
		"DoWait",
		"DoWalkBehind",
		"WaitRCClear",
		"FinishSeqOtis",
		"SaveGame",
		"HomeAlone",
		"Birth",
		"WithMilos",
		"HomeTogether",
		"Asleep",
		"StartPart2",
		"InPart2",
		"CheckTrain",
		"StartPart3",
		"InComp",
		"TakeAWalk",
		"KillAnna",
		"KilledAnna",
		"StartPart4",
		"Exit",
		"Done",
		"EndPart4",
		"StartPart5",
		"Guarding",
		"Climbing",
		"Disappear"
	};


	// WAITER1 EVENTS

	void Waiter1Call(CALL_PARAMS);

	void CONS_Waiter1_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_Waiter1_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_Waiter1_DoWait(CONS_PARAMS);
	void HAND_Waiter1_DoWait(HAND_PARAMS);

	void CONS_Waiter1_DoSeqOtis(CONS_PARAMS);
	void HAND_Waiter1_DoSeqOtis(HAND_PARAMS);

	void CONS_Waiter1_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_Waiter1_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_Waiter1_FinishSeqOtis(CONS_PARAMS);
	void HAND_Waiter1_FinishSeqOtis(HAND_PARAMS);

	void CONS_Waiter1_DoDialog(CONS_PARAMS);
	void HAND_Waiter1_DoDialog(HAND_PARAMS);

	void CONS_Waiter1_RebeccaFeedUs(CONS_PARAMS);
	void HAND_Waiter1_RebeccaFeedUs(HAND_PARAMS);

	void CONS_Waiter1_RebeccaClearOurTable(CONS_PARAMS);
	void HAND_Waiter1_RebeccaClearOurTable(HAND_PARAMS);

	void CONS_Waiter1_AbbotCheckMe(CONS_PARAMS);
	void HAND_Waiter1_AbbotCheckMe(HAND_PARAMS);

	void CONS_Waiter1_AbbotClearTable(CONS_PARAMS);
	void HAND_Waiter1_AbbotClearTable(HAND_PARAMS);

	void CONS_Waiter1_Birth(CONS_PARAMS);
	void HAND_Waiter1_Birth(HAND_PARAMS);

	void CONS_Waiter1_AnnaOrder(CONS_PARAMS);
	void HAND_Waiter1_AnnaOrder(HAND_PARAMS);

	void CONS_Waiter1_AugustOrder(CONS_PARAMS);
	void HAND_Waiter1_AugustOrder(HAND_PARAMS);

	void CONS_Waiter1_ServeAnna(CONS_PARAMS);
	void HAND_Waiter1_ServeAnna(HAND_PARAMS);

	void CONS_Waiter1_ServeAugust(CONS_PARAMS);
	void HAND_Waiter1_ServeAugust(HAND_PARAMS);

	void CONS_Waiter1_ClearAnna(CONS_PARAMS);
	void HAND_Waiter1_ClearAnna(HAND_PARAMS);

	void CONS_Waiter1_ClearTatiana(CONS_PARAMS);
	void HAND_Waiter1_ClearTatiana(HAND_PARAMS);

	void CONS_Waiter1_ClearAugust1(CONS_PARAMS);
	void HAND_Waiter1_ClearAugust1(HAND_PARAMS);

	void CONS_Waiter1_ClearAugust2(CONS_PARAMS);
	void HAND_Waiter1_ClearAugust2(HAND_PARAMS);

	void CONS_Waiter1_ServingDinner(CONS_PARAMS);
	void HAND_Waiter1_ServingDinner(HAND_PARAMS);

	void CONS_Waiter1_AfterDinner(CONS_PARAMS);
	void HAND_Waiter1_AfterDinner(HAND_PARAMS);

	void CONS_Waiter1_LockUp(CONS_PARAMS);
	void HAND_Waiter1_LockUp(HAND_PARAMS);

	void CONS_Waiter1_StartPart2(CONS_PARAMS);
	void HAND_Waiter1_StartPart2(HAND_PARAMS);

	void CONS_Waiter1_InKitchen(CONS_PARAMS);
	void HAND_Waiter1_InKitchen(HAND_PARAMS);

	void CONS_Waiter1_AugustComeHere2(CONS_PARAMS);
	void HAND_Waiter1_AugustComeHere2(HAND_PARAMS);

	void CONS_Waiter1_AugustClearTable2(CONS_PARAMS);
	void HAND_Waiter1_AugustClearTable2(HAND_PARAMS);

	void CONS_Waiter1_StartPart3(CONS_PARAMS);
	void HAND_Waiter1_StartPart3(HAND_PARAMS);

	void CONS_Waiter1_Serving3(CONS_PARAMS);
	void HAND_Waiter1_Serving3(HAND_PARAMS);

	void CONS_Waiter1_AnnaComeHere3(CONS_PARAMS);
	void HAND_Waiter1_AnnaComeHere3(HAND_PARAMS);

	void CONS_Waiter1_AbbotServeLunch3(CONS_PARAMS);
	void HAND_Waiter1_AbbotServeLunch3(HAND_PARAMS);

	void CONS_Waiter1_StartPart4(CONS_PARAMS);
	void HAND_Waiter1_StartPart4(HAND_PARAMS);

	void CONS_Waiter1_Serving4(CONS_PARAMS);
	void HAND_Waiter1_Serving4(HAND_PARAMS);

	void CONS_Waiter1_AugustOrder4(CONS_PARAMS);
	void HAND_Waiter1_AugustOrder4(HAND_PARAMS);

	void CONS_Waiter1_ServeAugust4(CONS_PARAMS);
	void HAND_Waiter1_ServeAugust4(HAND_PARAMS);

	void CONS_Waiter1_AugustClearTable(CONS_PARAMS);
	void HAND_Waiter1_AugustClearTable(HAND_PARAMS);

	void CONS_Waiter1_StartPart5(CONS_PARAMS);
	void HAND_Waiter1_StartPart5(HAND_PARAMS);

	void CONS_Waiter1_Prisoner(CONS_PARAMS);
	void HAND_Waiter1_Prisoner(HAND_PARAMS);

	void CONS_Waiter1_Disappear(CONS_PARAMS);
	void HAND_Waiter1_Disappear(HAND_PARAMS);

	Common::StringArray _waiter1FuncNames = {
		"DoJoinedSeqOtis",
		"DoWait",
		"DoSeqOtis",
		"DoBlockSeqOtis",
		"FinishSeqOtis",
		"DoDialog",
		"RebeccaFeedUs",
		"RebeccaClearOurTable",
		"AbbotCheckMe",
		"AbbotClearTable",
		"Birth",
		"AnnaOrder",
		"AugustOrder",
		"ServeAnna",
		"ServeAugust",
		"ClearAnna",
		"ClearTatiana",
		"ClearAugust1",
		"ClearAugust2",
		"ServingDinner",
		"AfterDinner",
		"LockUp",
		"StartPart2",
		"InKitchen",
		"AugustComeHere2",
		"AugustClearTable2",
		"StartPart3",
		"Serving3",
		"AnnaComeHere3",
		"AbbotServeLunch3",
		"StartPart4",
		"Serving4",
		"AugustOrder4",
		"ServeAugust4",
		"AugustClearTable",
		"StartPart5",
		"Prisoner",
		"Disappear"
	};


	// WAITER2 EVENTS

	void Waiter2Call(CALL_PARAMS);

	void CONS_Waiter2_DoWait(CONS_PARAMS);
	void HAND_Waiter2_DoWait(HAND_PARAMS);

	void CONS_Waiter2_DoSeqOtis(CONS_PARAMS);
	void HAND_Waiter2_DoSeqOtis(HAND_PARAMS);

	void CONS_Waiter2_DoBlockSeqOtis(CONS_PARAMS);
	void HAND_Waiter2_DoBlockSeqOtis(HAND_PARAMS);

	void CONS_Waiter2_FinishSeqOtis(CONS_PARAMS);
	void HAND_Waiter2_FinishSeqOtis(HAND_PARAMS);

	void CONS_Waiter2_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_Waiter2_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_Waiter2_DoDialog(CONS_PARAMS);
	void HAND_Waiter2_DoDialog(HAND_PARAMS);

	void CONS_Waiter2_MonsieurServeUs(CONS_PARAMS);
	void HAND_Waiter2_MonsieurServeUs(HAND_PARAMS);

	void CONS_Waiter2_Birth(CONS_PARAMS);
	void HAND_Waiter2_Birth(HAND_PARAMS);

	void CONS_Waiter2_MilosOrder(CONS_PARAMS);
	void HAND_Waiter2_MilosOrder(HAND_PARAMS);

	void CONS_Waiter2_MonsieurOrder(CONS_PARAMS);
	void HAND_Waiter2_MonsieurOrder(HAND_PARAMS);

	void CONS_Waiter2_ClearAlexei(CONS_PARAMS);
	void HAND_Waiter2_ClearAlexei(HAND_PARAMS);

	void CONS_Waiter2_ClearMilos(CONS_PARAMS);
	void HAND_Waiter2_ClearMilos(HAND_PARAMS);

	void CONS_Waiter2_ClearMonsieur(CONS_PARAMS);
	void HAND_Waiter2_ClearMonsieur(HAND_PARAMS);

	void CONS_Waiter2_ServingDinner(CONS_PARAMS);
	void HAND_Waiter2_ServingDinner(HAND_PARAMS);

	void CONS_Waiter2_AfterDinner(CONS_PARAMS);
	void HAND_Waiter2_AfterDinner(HAND_PARAMS);

	void CONS_Waiter2_LockUp(CONS_PARAMS);
	void HAND_Waiter2_LockUp(HAND_PARAMS);

	void CONS_Waiter2_StartPart2(CONS_PARAMS);
	void HAND_Waiter2_StartPart2(HAND_PARAMS);

	void CONS_Waiter2_InKitchen(CONS_PARAMS);
	void HAND_Waiter2_InKitchen(HAND_PARAMS);

	void CONS_Waiter2_TatianaClearTableB(CONS_PARAMS);
	void HAND_Waiter2_TatianaClearTableB(HAND_PARAMS);

	void CONS_Waiter2_IvoComeHere(CONS_PARAMS);
	void HAND_Waiter2_IvoComeHere(HAND_PARAMS);

	void CONS_Waiter2_IvoClearTableC(CONS_PARAMS);
	void HAND_Waiter2_IvoClearTableC(HAND_PARAMS);

	void CONS_Waiter2_StartPart3(CONS_PARAMS);
	void HAND_Waiter2_StartPart3(HAND_PARAMS);

	void CONS_Waiter2_Serving3(CONS_PARAMS);
	void HAND_Waiter2_Serving3(HAND_PARAMS);

	void CONS_Waiter2_AnnaBringTea3(CONS_PARAMS);
	void HAND_Waiter2_AnnaBringTea3(HAND_PARAMS);

	void CONS_Waiter2_StartPart4(CONS_PARAMS);
	void HAND_Waiter2_StartPart4(HAND_PARAMS);

	void CONS_Waiter2_Serving4(CONS_PARAMS);
	void HAND_Waiter2_Serving4(HAND_PARAMS);

	void CONS_Waiter2_AugustNeedsADrink(CONS_PARAMS);
	void HAND_Waiter2_AugustNeedsADrink(HAND_PARAMS);

	void CONS_Waiter2_ServeAugustADrink(CONS_PARAMS);
	void HAND_Waiter2_ServeAugustADrink(HAND_PARAMS);

	void CONS_Waiter2_AnnaNeedsADrink(CONS_PARAMS);
	void HAND_Waiter2_AnnaNeedsADrink(HAND_PARAMS);

	void CONS_Waiter2_StartPart5(CONS_PARAMS);
	void HAND_Waiter2_StartPart5(HAND_PARAMS);

	void CONS_Waiter2_Prisoner(CONS_PARAMS);
	void HAND_Waiter2_Prisoner(HAND_PARAMS);

	void CONS_Waiter2_Disappear(CONS_PARAMS);
	void HAND_Waiter2_Disappear(HAND_PARAMS);

	Common::StringArray _waiter2FuncNames = {
		"DoWait",
		"DoSeqOtis",
		"DoBlockSeqOtis",
		"FinishSeqOtis",
		"DoJoinedSeqOtis",
		"DoDialog",
		"MonsieurServeUs",
		"Birth",
		"MilosOrder",
		"MonsieurOrder",
		"ClearAlexei",
		"ClearMilos",
		"ClearMonsieur",
		"ServingDinner",
		"AfterDinner",
		"LockUp",
		"StartPart2",
		"InKitchen",
		"TatianaClearTableB",
		"IvoComeHere",
		"IvoClearTableC",
		"StartPart3",
		"Serving3",
		"AnnaBringTea3",
		"StartPart4",
		"Serving4",
		"AugustNeedsADrink",
		"ServeAugustADrink",
		"AnnaNeedsADrink",
		"StartPart5",
		"Prisoner",
		"Disappear"
	};


	// YASMIN EVENTS

	void YasminCall(CALL_PARAMS);

	void CONS_Yasmin_DebugWalks(CONS_PARAMS);
	void HAND_Yasmin_DebugWalks(HAND_PARAMS);

	void CONS_Yasmin_DoCorrOtis(CONS_PARAMS);
	void HAND_Yasmin_DoCorrOtis(HAND_PARAMS);

	void CONS_Yasmin_DoDialog(CONS_PARAMS);
	void HAND_Yasmin_DoDialog(HAND_PARAMS);

	void CONS_Yasmin_DoWait(CONS_PARAMS);
	void HAND_Yasmin_DoWait(HAND_PARAMS);

	void CONS_Yasmin_DoWalk(CONS_PARAMS);
	void HAND_Yasmin_DoWalk(HAND_PARAMS);

	void CONS_Yasmin_GoEtoG(CONS_PARAMS);
	void HAND_Yasmin_GoEtoG(HAND_PARAMS);

	void CONS_Yasmin_GoGtoE(CONS_PARAMS);
	void HAND_Yasmin_GoGtoE(HAND_PARAMS);

	void CONS_Yasmin_Birth(CONS_PARAMS);
	void HAND_Yasmin_Birth(HAND_PARAMS);

	void CONS_Yasmin_Part1(CONS_PARAMS);
	void HAND_Yasmin_Part1(HAND_PARAMS);

	void CONS_Yasmin_Asleep(CONS_PARAMS);
	void HAND_Yasmin_Asleep(HAND_PARAMS);

	void CONS_Yasmin_StartPart2(CONS_PARAMS);
	void HAND_Yasmin_StartPart2(HAND_PARAMS);

	void CONS_Yasmin_Part2(CONS_PARAMS);
	void HAND_Yasmin_Part2(HAND_PARAMS);

	void CONS_Yasmin_StartPart3(CONS_PARAMS);
	void HAND_Yasmin_StartPart3(HAND_PARAMS);

	void CONS_Yasmin_Part3(CONS_PARAMS);
	void HAND_Yasmin_Part3(HAND_PARAMS);

	void CONS_Yasmin_StartPart4(CONS_PARAMS);
	void HAND_Yasmin_StartPart4(HAND_PARAMS);

	void CONS_Yasmin_Part4(CONS_PARAMS);
	void HAND_Yasmin_Part4(HAND_PARAMS);

	void CONS_Yasmin_Asleep4(CONS_PARAMS);
	void HAND_Yasmin_Asleep4(HAND_PARAMS);

	void CONS_Yasmin_StartPart5(CONS_PARAMS);
	void HAND_Yasmin_StartPart5(HAND_PARAMS);

	void CONS_Yasmin_Prisoner(CONS_PARAMS);
	void HAND_Yasmin_Prisoner(HAND_PARAMS);

	void CONS_Yasmin_Free(CONS_PARAMS);
	void HAND_Yasmin_Free(HAND_PARAMS);

	void CONS_Yasmin_Hiding(CONS_PARAMS);
	void HAND_Yasmin_Hiding(HAND_PARAMS);

	void CONS_Yasmin_Disappear(CONS_PARAMS);
	void HAND_Yasmin_Disappear(HAND_PARAMS);

	Common::StringArray _yasminFuncNames = {
		"DebugWalks",
		"DoCorrOtis",
		"DoDialog",
		"DoWait",
		"DoWalk",
		"GoEtoG",
		"GoGtoE",
		"Birth",
		"Part1",
		"Asleep",
		"StartPart2",
		"Part2",
		"StartPart3",
		"Part3",
		"StartPart4",
		"Part4",
		"Asleep4",
		"StartPart5",
		"Prisoner",
		"Free",
		"Hiding",
		"Disappear"
	};


	// CALLBACK ARRAYS
	static void (LogicManager::*_functionsAbbot[])(HAND_PARAMS);
	static void (LogicManager::*_functionsAlexei[])(HAND_PARAMS);
	static void (LogicManager::*_functionsAlouan[])(HAND_PARAMS);
	static void (LogicManager::*_functionsAnna[])(HAND_PARAMS);
	static void (LogicManager::*_functionsAugust[])(HAND_PARAMS);
	static void (LogicManager::*_functionsClerk[])(HAND_PARAMS);
	static void (LogicManager::*_functionsCond1[])(HAND_PARAMS);
	static void (LogicManager::*_functionsCond2[])(HAND_PARAMS);
	static void (LogicManager::*_functionsCook[])(HAND_PARAMS);
	static void (LogicManager::*_functionsFrancois[])(HAND_PARAMS);
	static void (LogicManager::*_functionsHadija[])(HAND_PARAMS);
	static void (LogicManager::*_functionsHeadWait[])(HAND_PARAMS);
	static void (LogicManager::*_functionsIvo[])(HAND_PARAMS);
	static void (LogicManager::*_functionsKahina[])(HAND_PARAMS);
	static void (LogicManager::*_functionsKronos[])(HAND_PARAMS);
	static void (LogicManager::*_functionsMadame[])(HAND_PARAMS);
	static void (LogicManager::*_functionsMahmud[])(HAND_PARAMS);
	static void (LogicManager::*_functionsMaster[])(HAND_PARAMS);
	static void (LogicManager::*_functionsMax[])(HAND_PARAMS);
	static void (LogicManager::*_functionsMilos[])(HAND_PARAMS);
	static void (LogicManager::*_functionsMitchell[])(HAND_PARAMS);
	static void (LogicManager::*_functionsMonsieur[])(HAND_PARAMS);
	static void (LogicManager::*_functionsPolice[])(HAND_PARAMS);
	static void (LogicManager::*_functionsRebecca[])(HAND_PARAMS);
	static void (LogicManager::*_functionsSalko[])(HAND_PARAMS);
	static void (LogicManager::*_functionsSophie[])(HAND_PARAMS);
	static void (LogicManager::*_functionsTableA[])(HAND_PARAMS);
	static void (LogicManager::*_functionsTableB[])(HAND_PARAMS);
	static void (LogicManager::*_functionsTableC[])(HAND_PARAMS);
	static void (LogicManager::*_functionsTableD[])(HAND_PARAMS);
	static void (LogicManager::*_functionsTableE[])(HAND_PARAMS);
	static void (LogicManager::*_functionsTableF[])(HAND_PARAMS);
	static void (LogicManager::*_functionsTatiana[])(HAND_PARAMS);
	static void (LogicManager::*_functionsTrainM[])(HAND_PARAMS);
	static void (LogicManager::*_functionsVassili[])(HAND_PARAMS);
	static void (LogicManager::*_functionsVesna[])(HAND_PARAMS);
	static void (LogicManager::*_functionsWaiter1[])(HAND_PARAMS);
	static void (LogicManager::*_functionsWaiter2[])(HAND_PARAMS);
	static void (LogicManager::*_functionsYasmin[])(HAND_PARAMS);


	Common::StringArray _cathFuncNames = {""};

	// For the debugger
	Common::Array<Common::StringArray> _funcNames = {
		_cathFuncNames,
		_annaFuncNames,
		_augustFuncNames,
		_cond1FuncNames,
		_cond2FuncNames,
		_headWaitFuncNames,
		_waiter1FuncNames,
		_waiter2FuncNames,
		_cookFuncNames,
		_trainMFuncNames,
		_tatianaFuncNames,
		_vassiliFuncNames,
		_alexeiFuncNames,
		_abbotFuncNames,
		_milosFuncNames,
		_vesnaFuncNames,
		_ivoFuncNames,
		_salkoFuncNames,
		_kronosFuncNames,
		_kahinaFuncNames,
		_francoisFuncNames,
		_madameFuncNames,
		_monsieurFuncNames,
		_rebeccaFuncNames,
		_sophieFuncNames,
		_mahmudFuncNames,
		_yasminFuncNames,
		_hadijaFuncNames,
		_alouanFuncNames,
		_policeFuncNames,
		_maxFuncNames,
		_masterFuncNames,
		_clerkFuncNames,
		_tablesFuncNames,
		_tablesFuncNames,
		_tablesFuncNames,
		_tablesFuncNames,
		_tablesFuncNames,
		_tablesFuncNames,
		_mitchellFuncNames
	};

	
	////////////////////////////////////
	//         DEMO FUNCTIONS
	////////////////////////////////////

	// CHAPTER SELECTOR
	void CONS_DemoAbbot(int chapter);
	void CONS_DemoAnna(int chapter);
	void CONS_DemoAugust(int chapter);
	void CONS_DemoCond2(int chapter);
	void CONS_DemoFrancois(int chapter);
	void CONS_DemoIvo(int chapter);
	void CONS_DemoMadame(int chapter);
	void CONS_DemoMaster(int chapter);
	void CONS_DemoMonsieur(int chapter);
	void CONS_DemoRebecca(int chapter);
	void CONS_DemoTableA(int chapter);
	void CONS_DemoTableB(int chapter);
	void CONS_DemoTableC(int chapter);
	void CONS_DemoTableD(int chapter);
	void CONS_DemoTableE(int chapter);
	void CONS_DemoTableF(int chapter);
	void CONS_DemoTatiana(int chapter);
	void CONS_DemoVesna(int chapter);
	void CONS_DemoWaiter1(int chapter);

	// DEMO ANNA EVENTS

	void DemoAnnaCall(CALL_PARAMS);

	void CONS_DemoAnna_DoSeqOtis(CONS_PARAMS);
	void HAND_DemoAnna_DoSeqOtis(HAND_PARAMS);

	void CONS_DemoAnna_FinishSeqOtis(CONS_PARAMS);
	void HAND_DemoAnna_FinishSeqOtis(HAND_PARAMS);

	void CONS_DemoAnna_DoCorrOtis(CONS_PARAMS);
	void HAND_DemoAnna_DoCorrOtis(HAND_PARAMS);

	void CONS_DemoAnna_WaitRCClear(CONS_PARAMS);
	void HAND_DemoAnna_WaitRCClear(HAND_PARAMS);

	void CONS_DemoAnna_SaveGame(CONS_PARAMS);
	void HAND_DemoAnna_SaveGame(HAND_PARAMS);

	void CONS_DemoAnna_DoWalk(CONS_PARAMS);
	void HAND_DemoAnna_DoWalk(HAND_PARAMS);

	void CONS_DemoAnna_Birth(CONS_PARAMS);
	void HAND_DemoAnna_Birth(HAND_PARAMS);

	void CONS_DemoAnna_GoBagg(CONS_PARAMS);
	void HAND_DemoAnna_GoBagg(HAND_PARAMS);

	void CONS_DemoAnna_InBagg(CONS_PARAMS);
	void HAND_DemoAnna_InBagg(HAND_PARAMS);

	void CONS_DemoAnna_DeadBagg(CONS_PARAMS);
	void HAND_DemoAnna_DeadBagg(HAND_PARAMS);

	void CONS_DemoAnna_BaggageFight(CONS_PARAMS);
	void HAND_DemoAnna_BaggageFight(HAND_PARAMS);

	void CONS_DemoAnna_StartPart2(CONS_PARAMS);
	void HAND_DemoAnna_StartPart2(HAND_PARAMS);

	void CONS_DemoAnna_StartPart3(CONS_PARAMS);
	void HAND_DemoAnna_StartPart3(HAND_PARAMS);

	void CONS_DemoAnna_StartPart4(CONS_PARAMS);
	void HAND_DemoAnna_StartPart4(HAND_PARAMS);

	void CONS_DemoAnna_StartPart5(CONS_PARAMS);
	void HAND_DemoAnna_StartPart5(HAND_PARAMS);

	Common::StringArray _demoAnnaFuncNames = {
		"DoSeqOtis",
		"FinishSeqOtis",
		"DoCorrOtis",
		"WaitRCClear",
		"SaveGame",
		"DoWalk",
		"Birth",
		"GoBagg",
		"InBagg",
		"DeadBagg",
		"BaggageFight",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO ABBOT EVENTS

	void DemoAbbotCall(CALL_PARAMS);

	void CONS_DemoAbbot_DoWait(CONS_PARAMS);
	void HAND_DemoAbbot_DoWait(HAND_PARAMS);

	void CONS_DemoAbbot_DoJoinedSeqOtis(CONS_PARAMS);
	void HAND_DemoAbbot_DoJoinedSeqOtis(HAND_PARAMS);

	void CONS_DemoAbbot_WaitRCClear(CONS_PARAMS);
	void HAND_DemoAbbot_WaitRCClear(HAND_PARAMS);

	void CONS_DemoAbbot_Birth(CONS_PARAMS);
	void HAND_DemoAbbot_Birth(HAND_PARAMS);

	void CONS_DemoAbbot_StartPart2(CONS_PARAMS);
	void HAND_DemoAbbot_StartPart2(HAND_PARAMS);

	void CONS_DemoAbbot_StartPart3(CONS_PARAMS);
	void HAND_DemoAbbot_StartPart3(HAND_PARAMS);

	void CONS_DemoAbbot_StartPart4(CONS_PARAMS);
	void HAND_DemoAbbot_StartPart4(HAND_PARAMS);

	void CONS_DemoAbbot_StartPart5(CONS_PARAMS);
	void HAND_DemoAbbot_StartPart5(HAND_PARAMS);

	Common::StringArray _demoAbbotFuncNames = {
		"DoWait",
		"DoJoinedSeqOtis",
		"WaitRCClear",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO AUGUST EVENTS

	void DemoAugustCall(CALL_PARAMS);

	void CONS_DemoAugust_SaveGame(CONS_PARAMS);
	void HAND_DemoAugust_SaveGame(HAND_PARAMS);

	void CONS_DemoAugust_Birth(CONS_PARAMS);
	void HAND_DemoAugust_Birth(HAND_PARAMS);

	void CONS_DemoAugust_StartPart2(CONS_PARAMS);
	void HAND_DemoAugust_StartPart2(HAND_PARAMS);

	void CONS_DemoAugust_StartPart3(CONS_PARAMS);
	void HAND_DemoAugust_StartPart3(HAND_PARAMS);

	void CONS_DemoAugust_StartPart4(CONS_PARAMS);
	void HAND_DemoAugust_StartPart4(HAND_PARAMS);

	void CONS_DemoAugust_StartPart5(CONS_PARAMS);
	void HAND_DemoAugust_StartPart5(HAND_PARAMS);

	Common::StringArray _demoAugustFuncNames = {
		"SaveGame",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO COND2 EVENTS

	void DemoCond2Call(CALL_PARAMS);

	void CONS_DemoCond2_DoSeqOtis(CONS_PARAMS);
	void HAND_DemoCond2_DoSeqOtis(HAND_PARAMS);

	void CONS_DemoCond2_FinishSeqOtis(CONS_PARAMS);
	void HAND_DemoCond2_FinishSeqOtis(HAND_PARAMS);

	void CONS_DemoCond2_SitDown(CONS_PARAMS);
	void HAND_DemoCond2_SitDown(HAND_PARAMS);

	void CONS_DemoCond2_Birth(CONS_PARAMS);
	void HAND_DemoCond2_Birth(HAND_PARAMS);

	void CONS_DemoCond2_StartPart2(CONS_PARAMS);
	void HAND_DemoCond2_StartPart2(HAND_PARAMS);

	void CONS_DemoCond2_StartPart3(CONS_PARAMS);
	void HAND_DemoCond2_StartPart3(HAND_PARAMS);

	void CONS_DemoCond2_StartPart4(CONS_PARAMS);
	void HAND_DemoCond2_StartPart4(HAND_PARAMS);

	void CONS_DemoCond2_StartPart5(CONS_PARAMS);
	void HAND_DemoCond2_StartPart5(HAND_PARAMS);

	Common::StringArray _demoCond2FuncNames = {
		"DoSeqOtis",
		"FinishSeqOtis",
		"SitDown",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO FRANCOIS EVENTS

	void DemoFrancoisCall(CALL_PARAMS);

	void CONS_DemoFrancois_DoSeqOtis(CONS_PARAMS);
	void HAND_DemoFrancois_DoSeqOtis(HAND_PARAMS);

	void CONS_DemoFrancois_SaveGame(CONS_PARAMS);
	void HAND_DemoFrancois_SaveGame(HAND_PARAMS);

	void CONS_DemoFrancois_Birth(CONS_PARAMS);
	void HAND_DemoFrancois_Birth(HAND_PARAMS);

	void CONS_DemoFrancois_StartPart2(CONS_PARAMS);
	void HAND_DemoFrancois_StartPart2(HAND_PARAMS);

	void CONS_DemoFrancois_StartPart3(CONS_PARAMS);
	void HAND_DemoFrancois_StartPart3(HAND_PARAMS);

	void CONS_DemoFrancois_StartPart4(CONS_PARAMS);
	void HAND_DemoFrancois_StartPart4(HAND_PARAMS);

	void CONS_DemoFrancois_StartPart5(CONS_PARAMS);
	void HAND_DemoFrancois_StartPart5(HAND_PARAMS);

	Common::StringArray _demoFrancoisFuncNames = {
		"DoSeqOtis",
		"SaveGame",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO IVO EVENTS

	void DemoIvoCall(CALL_PARAMS);

	void CONS_DemoIvo_WaitRCClear(CONS_PARAMS);
	void HAND_DemoIvo_WaitRCClear(HAND_PARAMS);

	void CONS_DemoIvo_DoSeqOtis(CONS_PARAMS);
	void HAND_DemoIvo_DoSeqOtis(HAND_PARAMS);

	void CONS_DemoIvo_FinishSeqOtis(CONS_PARAMS);
	void HAND_DemoIvo_FinishSeqOtis(HAND_PARAMS);

	void CONS_DemoIvo_DoSplitOtis023A(CONS_PARAMS);
	void HAND_DemoIvo_DoSplitOtis023A(HAND_PARAMS);

	void CONS_DemoIvo_Birth(CONS_PARAMS);
	void HAND_DemoIvo_Birth(HAND_PARAMS);

	void CONS_DemoIvo_StartPart2(CONS_PARAMS);
	void HAND_DemoIvo_StartPart2(HAND_PARAMS);

	void CONS_DemoIvo_StartPart3(CONS_PARAMS);
	void HAND_DemoIvo_StartPart3(HAND_PARAMS);

	void CONS_DemoIvo_StartPart4(CONS_PARAMS);
	void HAND_DemoIvo_StartPart4(HAND_PARAMS);

	void CONS_DemoIvo_StartPart5(CONS_PARAMS);
	void HAND_DemoIvo_StartPart5(HAND_PARAMS);

	Common::StringArray _demoIvoFuncNames = {
		"WaitRCClear",
		"DoSeqOtis",
		"FinishSeqOtis",
		"DoSplitOtis023A",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO MADAME EVENTS

	void DemoMadameCall(CALL_PARAMS);

	void CONS_DemoMadame_DoDialog(CONS_PARAMS);
	void HAND_DemoMadame_DoDialog(HAND_PARAMS);

	void CONS_DemoMadame_Birth(CONS_PARAMS);
	void HAND_DemoMadame_Birth(HAND_PARAMS);

	void CONS_DemoMadame_StartPart2(CONS_PARAMS);
	void HAND_DemoMadame_StartPart2(HAND_PARAMS);

	void CONS_DemoMadame_StartPart3(CONS_PARAMS);
	void HAND_DemoMadame_StartPart3(HAND_PARAMS);

	void CONS_DemoMadame_StartPart4(CONS_PARAMS);
	void HAND_DemoMadame_StartPart4(HAND_PARAMS);

	void CONS_DemoMadame_StartPart5(CONS_PARAMS);
	void HAND_DemoMadame_StartPart5(HAND_PARAMS);

	Common::StringArray _demoMadameFuncNames = {
		"DoDialog",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO MASTER EVENTS

	void DemoMasterCall(CALL_PARAMS);

	void CONS_DemoMaster_SaveGame(CONS_PARAMS);
	void HAND_DemoMaster_SaveGame(HAND_PARAMS);

	void CONS_DemoMaster_Birth(CONS_PARAMS);
	void HAND_DemoMaster_Birth(HAND_PARAMS);

	void CONS_DemoMaster_StartPart2(CONS_PARAMS);
	void HAND_DemoMaster_StartPart2(HAND_PARAMS);

	void CONS_DemoMaster_StartPart3(CONS_PARAMS);
	void HAND_DemoMaster_StartPart3(HAND_PARAMS);

	void CONS_DemoMaster_StartPart4(CONS_PARAMS);
	void HAND_DemoMaster_StartPart4(HAND_PARAMS);

	void CONS_DemoMaster_StartPart5(CONS_PARAMS);
	void HAND_DemoMaster_StartPart5(HAND_PARAMS);

	Common::StringArray _demoMasterFuncNames = {
		"SaveGame",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO MONSIEUR EVENTS

	void DemoMonsieurCall(CALL_PARAMS);

	void CONS_DemoMonsieur_Birth(CONS_PARAMS);
	void HAND_DemoMonsieur_Birth(HAND_PARAMS);

	void CONS_DemoMonsieur_StartPart2(CONS_PARAMS);
	void HAND_DemoMonsieur_StartPart2(HAND_PARAMS);

	void CONS_DemoMonsieur_StartPart3(CONS_PARAMS);
	void HAND_DemoMonsieur_StartPart3(HAND_PARAMS);

	void CONS_DemoMonsieur_StartPart4(CONS_PARAMS);
	void HAND_DemoMonsieur_StartPart4(HAND_PARAMS);

	void CONS_DemoMonsieur_StartPart5(CONS_PARAMS);
	void HAND_DemoMonsieur_StartPart5(HAND_PARAMS);

	Common::StringArray _demoMonsieurFuncNames = {
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO REBECCA EVENTS

	void DemoRebeccaCall(CALL_PARAMS);

	void CONS_DemoRebecca_DoDialog(CONS_PARAMS);
	void HAND_DemoRebecca_DoDialog(HAND_PARAMS);

	void CONS_DemoRebecca_Birth(CONS_PARAMS);
	void HAND_DemoRebecca_Birth(HAND_PARAMS);

	void CONS_DemoRebecca_StartPart2(CONS_PARAMS);
	void HAND_DemoRebecca_StartPart2(HAND_PARAMS);

	void CONS_DemoRebecca_StartPart3(CONS_PARAMS);
	void HAND_DemoRebecca_StartPart3(HAND_PARAMS);

	void CONS_DemoRebecca_StartPart4(CONS_PARAMS);
	void HAND_DemoRebecca_StartPart4(HAND_PARAMS);

	void CONS_DemoRebecca_StartPart5(CONS_PARAMS);
	void HAND_DemoRebecca_StartPart5(HAND_PARAMS);

	Common::StringArray _demoRebeccaFuncNames = {
		"DoDialog",
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO TABLEA EVENTS

	void CONS_DemoTableA_Birth(CONS_PARAMS);
	void HAND_DemoTableA_Birth(HAND_PARAMS);

	void CONS_DemoTableA_StartPart2(CONS_PARAMS);
	void HAND_DemoTableA_StartPart2(HAND_PARAMS);

	void CONS_DemoTableA_StartPart3(CONS_PARAMS);
	void HAND_DemoTableA_StartPart3(HAND_PARAMS);

	void CONS_DemoTableA_StartPart4(CONS_PARAMS);
	void HAND_DemoTableA_StartPart4(HAND_PARAMS);

	void CONS_DemoTableA_StartPart5(CONS_PARAMS);
	void HAND_DemoTableA_StartPart5(HAND_PARAMS);


	// DEMO TABLEB EVENTS

	void CONS_DemoTableB_Birth(CONS_PARAMS);
	void HAND_DemoTableB_Birth(HAND_PARAMS);

	void CONS_DemoTableB_StartPart2(CONS_PARAMS);
	void HAND_DemoTableB_StartPart2(HAND_PARAMS);

	void CONS_DemoTableB_StartPart3(CONS_PARAMS);
	void HAND_DemoTableB_StartPart3(HAND_PARAMS);

	void CONS_DemoTableB_StartPart4(CONS_PARAMS);
	void HAND_DemoTableB_StartPart4(HAND_PARAMS);

	void CONS_DemoTableB_StartPart5(CONS_PARAMS);
	void HAND_DemoTableB_StartPart5(HAND_PARAMS);


	// DEMO TABLEC EVENTS

	void CONS_DemoTableC_Birth(CONS_PARAMS);
	void HAND_DemoTableC_Birth(HAND_PARAMS);

	void CONS_DemoTableC_StartPart2(CONS_PARAMS);
	void HAND_DemoTableC_StartPart2(HAND_PARAMS);

	void CONS_DemoTableC_StartPart3(CONS_PARAMS);
	void HAND_DemoTableC_StartPart3(HAND_PARAMS);

	void CONS_DemoTableC_StartPart4(CONS_PARAMS);
	void HAND_DemoTableC_StartPart4(HAND_PARAMS);

	void CONS_DemoTableC_StartPart5(CONS_PARAMS);
	void HAND_DemoTableC_StartPart5(HAND_PARAMS);


	// DEMO TABLED EVENTS

	void CONS_DemoTableD_Birth(CONS_PARAMS);
	void HAND_DemoTableD_Birth(HAND_PARAMS);

	void CONS_DemoTableD_StartPart2(CONS_PARAMS);
	void HAND_DemoTableD_StartPart2(HAND_PARAMS);

	void CONS_DemoTableD_StartPart3(CONS_PARAMS);
	void HAND_DemoTableD_StartPart3(HAND_PARAMS);

	void CONS_DemoTableD_StartPart4(CONS_PARAMS);
	void HAND_DemoTableD_StartPart4(HAND_PARAMS);

	void CONS_DemoTableD_StartPart5(CONS_PARAMS);
	void HAND_DemoTableD_StartPart5(HAND_PARAMS);


	// DEMO TABLEE EVENTS

	void CONS_DemoTableE_Birth(CONS_PARAMS);
	void HAND_DemoTableE_Birth(HAND_PARAMS);

	void CONS_DemoTableE_StartPart2(CONS_PARAMS);
	void HAND_DemoTableE_StartPart2(HAND_PARAMS);

	void CONS_DemoTableE_StartPart3(CONS_PARAMS);
	void HAND_DemoTableE_StartPart3(HAND_PARAMS);

	void CONS_DemoTableE_StartPart4(CONS_PARAMS);
	void HAND_DemoTableE_StartPart4(HAND_PARAMS);

	void CONS_DemoTableE_StartPart5(CONS_PARAMS);
	void HAND_DemoTableE_StartPart5(HAND_PARAMS);


	// DEMO TABLEF EVENTS

	void CONS_DemoTableF_Birth(CONS_PARAMS);
	void HAND_DemoTableF_Birth(HAND_PARAMS);

	void CONS_DemoTableF_StartPart2(CONS_PARAMS);
	void HAND_DemoTableF_StartPart2(HAND_PARAMS);

	void CONS_DemoTableF_StartPart3(CONS_PARAMS);
	void HAND_DemoTableF_StartPart3(HAND_PARAMS);

	void CONS_DemoTableF_StartPart4(CONS_PARAMS);
	void HAND_DemoTableF_StartPart4(HAND_PARAMS);

	void CONS_DemoTableF_StartPart5(CONS_PARAMS);
	void HAND_DemoTableF_StartPart5(HAND_PARAMS);

	Common::StringArray _demoTablesFuncNames = {
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO TATIANA EVENTS

	void CONS_DemoTatiana_Birth(CONS_PARAMS);
	void HAND_DemoTatiana_Birth(HAND_PARAMS);

	void CONS_DemoTatiana_StartPart2(CONS_PARAMS);
	void HAND_DemoTatiana_StartPart2(HAND_PARAMS);

	void CONS_DemoTatiana_StartPart3(CONS_PARAMS);
	void HAND_DemoTatiana_StartPart3(HAND_PARAMS);

	void CONS_DemoTatiana_StartPart4(CONS_PARAMS);
	void HAND_DemoTatiana_StartPart4(HAND_PARAMS);

	void CONS_DemoTatiana_StartPart5(CONS_PARAMS);
	void HAND_DemoTatiana_StartPart5(HAND_PARAMS);

	Common::StringArray _demoTatianaFuncNames = {
		"Birth",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO VESNA EVENTS

	void DemoVesnaCall(CALL_PARAMS);

	void CONS_DemoVesna_DoCorrOtis(CONS_PARAMS);
	void HAND_DemoVesna_DoCorrOtis(HAND_PARAMS);

	void CONS_DemoVesna_DoSeqOtis(CONS_PARAMS);
	void HAND_DemoVesna_DoSeqOtis(HAND_PARAMS);

	void CONS_DemoVesna_DoWalk(CONS_PARAMS);
	void HAND_DemoVesna_DoWalk(HAND_PARAMS);

	void CONS_DemoVesna_WaitRCClear(CONS_PARAMS);
	void HAND_DemoVesna_WaitRCClear(HAND_PARAMS);

	void CONS_DemoVesna_FinishSeqOtis(CONS_PARAMS);
	void HAND_DemoVesna_FinishSeqOtis(HAND_PARAMS);

	void CONS_DemoVesna_Birth(CONS_PARAMS);
	void HAND_DemoVesna_Birth(HAND_PARAMS);

	void CONS_DemoVesna_InComp(CONS_PARAMS);
	void HAND_DemoVesna_InComp(HAND_PARAMS);

	void CONS_DemoVesna_KillAnna(CONS_PARAMS);
	void HAND_DemoVesna_KillAnna(HAND_PARAMS);

	void CONS_DemoVesna_StartPart2(CONS_PARAMS);
	void HAND_DemoVesna_StartPart2(HAND_PARAMS);

	void CONS_DemoVesna_StartPart3(CONS_PARAMS);
	void HAND_DemoVesna_StartPart3(HAND_PARAMS);

	void CONS_DemoVesna_StartPart4(CONS_PARAMS);
	void HAND_DemoVesna_StartPart4(HAND_PARAMS);

	void CONS_DemoVesna_StartPart5(CONS_PARAMS);
	void HAND_DemoVesna_StartPart5(HAND_PARAMS);

	Common::StringArray _demoVesnaFuncNames = {
		"DoCorrOtis",
		"DoSeqOtis",
		"DoWalk",
		"WaitRCClear",
		"FinishSeqOtis",
		"Birth",
		"InComp",
		"KillAnna",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	// DEMO WAITER1 EVENTS

	void DemoWaiter1Call(CALL_PARAMS);

	void CONS_DemoWaiter1_DoSeqOtis(CONS_PARAMS);
	void HAND_DemoWaiter1_DoSeqOtis(HAND_PARAMS);

	void CONS_DemoWaiter1_Birth(CONS_PARAMS);
	void HAND_DemoWaiter1_Birth(HAND_PARAMS);

	void CONS_DemoWaiter1_RebeccaFeedUs(CONS_PARAMS);
	void HAND_DemoWaiter1_RebeccaFeedUs(HAND_PARAMS);

	void CONS_DemoWaiter1_StartPart2(CONS_PARAMS);
	void HAND_DemoWaiter1_StartPart2(HAND_PARAMS);

	void CONS_DemoWaiter1_StartPart3(CONS_PARAMS);
	void HAND_DemoWaiter1_StartPart3(HAND_PARAMS);

	void CONS_DemoWaiter1_StartPart4(CONS_PARAMS);
	void HAND_DemoWaiter1_StartPart4(HAND_PARAMS);

	void CONS_DemoWaiter1_StartPart5(CONS_PARAMS);
	void HAND_DemoWaiter1_StartPart5(HAND_PARAMS);

	Common::StringArray _demoWaiter1FuncNames = {
		"DoSeqOtis",
		"Birth",
		"RebeccaFeedUs",
		"StartPart2",
		"StartPart3",
		"StartPart4",
		"StartPart5"
	};


	static void (LogicManager::*_functionsDemoAbbot[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoAnna[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoAugust[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoCond2[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoFrancois[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoIvo[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoMadame[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoMaster[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoMonsieur[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoRebecca[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoSophie[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoTableA[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoTableB[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoTableC[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoTableD[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoTableE[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoTableF[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoTatiana[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoVesna[])(HAND_PARAMS);
	static void (LogicManager::*_functionsDemoWaiter1[])(HAND_PARAMS);


	// For the debugger
	Common::Array<Common::StringArray> _demoFuncNames = {
		_cathFuncNames,
		_demoAnnaFuncNames,
		_demoAugustFuncNames,
		_cathFuncNames,
		_demoCond2FuncNames,
		_cathFuncNames,
		_demoWaiter1FuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_demoTatianaFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_demoAbbotFuncNames,
		_cathFuncNames,
		_demoVesnaFuncNames,
		_demoIvoFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_demoFrancoisFuncNames,
		_demoMadameFuncNames,
		_demoMonsieurFuncNames,
		_demoRebeccaFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_cathFuncNames,
		_demoMasterFuncNames,
		_cathFuncNames,
		_demoTablesFuncNames,
		_demoTablesFuncNames,
		_demoTablesFuncNames,
		_demoTablesFuncNames,
		_demoTablesFuncNames,
		_demoTablesFuncNames,
		_cathFuncNames
	};
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_LOGIC_H
