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
#ifndef DREAMWEB_STUBS_H
#define DREAMWEB_STUBS_H

	void screenUpdate();
	void startup();
	void startup1();
	void switchRyanOn();
	void switchRyanOff();
	void saveLoad();
	void hangOnCurs(uint16 frameCount);
	void hangOnCurs();
	void workToScreen();
	void multiGet();
	void multiGet(uint8 *dst, uint16 x, uint16 y, uint8 width, uint8 height) {
		DreamBase::multiGet(dst, x, y, width, height);
	}
	void multiPut();
	void multiPut(const uint8 *src, uint16 x, uint16 y, uint8 width, uint8 height) {
		DreamBase::multiPut(src, x, y, width, height);
	}
	void multiDump();
	void multiDump(uint16 x, uint16 y, uint8 width, uint8 height) {
		DreamBase::multiDump(x, y, width, height);
	}
	void printSprites();
	void quickQuit();
	void readOneBlock();
	void readCityPic();
	void readDestIcon();
	void seeCommandTail();
	void randomNumber();
	void quickQuit2();
	void printDirect();
	uint8 printDirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered) {
		return DreamBase::printDirect(string, x, y, maxWidth, centered);
	}
	uint8 printDirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
		return DreamBase::printDirect(string, x, y, maxWidth, centered);
	}
	void useTimedText();
	void dumpTimedText();
	void setupTimedTemp(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount);
	void getUnderTimed();
	void putUnderTimed();
	void dumpTextLine();
	void startLoading(const Room &room);
	void showFrame();
	void showFrame(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height) {
		DreamBase::showFrame(frameData, x, y, frameNumber, effectsFlag, width, height);
	}
	void showFrame(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag) {
		DreamBase::showFrame(frameData, x, y, frameNumber, effectsFlag);
	}
	void printASprite(const Sprite *sprite);
	void width160();
	void clearSprites();
	Sprite *makeSprite(uint8 x, uint8 y, uint16 updateCallback, uint16 frameData, uint16 somethingInDi);
	void spriteUpdate();
	void initMan();
	void mainMan(Sprite *sprite);
	void mainMan();
	void walking(Sprite *sprite);
	void aboutTurn(Sprite *sprite);
	void backObject(Sprite *sprite);
	void backObject();
	void constant(Sprite *sprite, SetObject *objData);
	void steady(Sprite *sprite, SetObject *objData);
	void randomSprite(Sprite *sprite, SetObject *objData);
	void doDoor(Sprite *sprite, SetObject *objData, Common::Rect check);
	void doorway(Sprite *sprite, SetObject *objData);
	void wideDoor(Sprite *sprite, SetObject *objData);
	void lockedDoorway(Sprite *sprite, SetObject *objData);
	void liftSprite(Sprite *sprite, SetObject *objData);
	void showGameReel();
	void showGameReel(ReelRoutine *routine);
	void turnPathOn(uint8 param);
	void turnPathOff(uint8 param);
	void turnPathOn();
	void turnPathOff();
	void turnAnyPathOn(uint8 param, uint8 room);
	void turnAnyPathOff(uint8 param, uint8 room);
	void turnAnyPathOn();
	void turnAnyPathOff();
	void makeBackOb(SetObject *objData);
	void dealWithSpecial(uint8 firstParam, uint8 secondParam);
	void zoom();
	void showRain();
	void commandOnly();
	void commandOnly(uint8 command) {
		DreamBase::commandOnly(command);
	}
	void doBlocks();
	void checkIfPerson();
	bool checkIfPerson(uint8 x, uint8 y);
	void checkIfFree();
	bool checkIfFree(uint8 x, uint8 y);
	void checkIfEx();
	bool checkIfEx(uint8 x, uint8 y);
	const uint8 *findObName(uint8 type, uint8 index);
	void copyName();
	void copyName(uint8 type, uint8 index, uint8 *dst);
	void commandWithOb();
	void commandWithOb(uint8 command, uint8 type, uint8 index);
	void updatePeople();
	void madman(ReelRoutine &routine);
	void madmanText();
	void madMode();
	void moveMap(uint8 param);
	bool addAlong(const uint8 *mapFlags);
	bool addLength(const uint8 *mapFlags);
	void getDimension();
	void getDimension(uint8 *mapXstart, uint8 *mapYstart, uint8 *mapXsize, uint8 *mapYsize);
	void getMapAd();
	void calcMapAd();
	uint8 getMapAd(const uint8 *setData);
	uint8 getXAd(const uint8 *setData, uint8 *result);
	uint8 getYAd(const uint8 *setData, uint8 *result);
	void calcFrFrame(uint16 frame);
	void calcFrFrame(uint16 frame, uint8* width, uint8* height);
	void finalFrame();
	void finalFrame(uint16 *x, uint16 *y);
	void showAllObs();
	void blockNameText();
	void walkToText();
	void personNameText();
	DynObject *getFreeAd(uint8 index) {
		return DreamBase::getFreeAd(index);
	}
	DynObject *getExAd(uint8 index) {
		return DreamBase::getExAd(index);
	}
	void *getAnyAd(uint8 *value1, uint8 *value2) {
		return DreamBase::getAnyAd(value1, value2);
	}
	SetObject *getSetAd(uint8 index) {
		return DreamBase::getSetAd(index);
	}
	void setAllChanges();
	void doChange(uint8 index, uint8 value, uint8 type);
	void deleteTaken();
	bool isCD();
	void placeSetObject();
	void placeSetObject(uint8 index);
	void removeSetObject();
	void removeSetObject(uint8 index);
	void showAllFree();
	void showAllEx();
	bool finishedWalkingCPP();
	void finishedWalking();
	void checkOne();
	void checkOne(uint8 x, uint8 y, uint8 *flag, uint8 *flagEx, uint8 *type, uint8 *flagX, uint8 *flagY) {
		DreamBase::checkOne(x, y, flag, flagEx, type, flagX, flagY);
	}
	void walkAndExamine();
	void obName();
	void obName(uint8 command, uint8 commandType);
	void checkCoords(const RectWithCallback *rectWithCallbacks);
	void addToPeopleList();
	void addToPeopleList(ReelRoutine *routine);
	void getExPos();
	void compare();
	bool compare(uint8 index, uint8 flag, const char id[4]) {
		return DreamBase::compare(index, flag, id);
	}
	bool pixelCheckSet(const ObjPos *pos, uint8 x, uint8 y);
	void checkIfSet();
	bool checkIfSet(uint8 x, uint8 y);
	void checkIfPathIsOn();
	bool checkIfPathIsOn(uint8 index);
	void isItWorn();
	bool isItWorn(const DynObject *object) {
		return DreamBase::isItWorn(object);
	}
	void wornError();
	void makeWorn();
	void makeWorn(DynObject *object);
	void obToInv();
	void obToInv(uint8 index, uint8 flag, uint16 x, uint16 y) {
		DreamBase::obToInv(index, flag, x, y);
	}
	void useRoutine();
	void hangOn();
	void hangOn(uint16 frameCount) {
		DreamBase::hangOn(frameCount);
	}
	void hangOnP();
	void hangOnP(uint16 count) {
		DreamBase::hangOnP(count);
	}
	uint8 findNextColon(const uint8 **string) {
		return DreamBase::findNextColon(string);
	}
	void findNextColon();
	const uint8 *getObTextStartCPP();
	void useText(const uint8 *string);
	void useText();
	void getBlockOfPixel();
	uint8 getBlockOfPixel(uint8 x, uint8 y);
	void examineObText();
	void showCity();
	uint16 getPersFrame(uint8 index);
	void convIcons();
	void examineOb(bool examineAgain = true);
	void dumpWatch();
	void transferText();
	void initRain();
	Rain *splitIntoLines(uint8 x, uint8 y, Rain *rain);
	void watchCount();
	void loadRoom();
	void textForMonk();
	void textForMonkHelper(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount);
	void textForEnd();
	void monks2text();
	void intro1Text();
	void intro2Text();
	void intro3Text();
	void readSetData();
	void fadeupYellows();
	void fadeupMonFirst();
	void loadMenu();
	void showMenu();
	void dumpMenu();
	void useMenu();
	void useMon();
	void makeCaps();
	byte makeCaps(byte c) {
		return DreamBase::makeCaps(c);
	}
	void monPrint();
	const char *monPrint(const char *string) {
		return DreamBase::monPrint(string);
	}
	void randomAccess();
	void randomAccess(uint16 count) {
		DreamBase::randomAccess(count);
	}
	void monMessage();
	void monMessage(uint8 index) {
		DreamBase::monMessage(index);
	}
	void playChannel1();
	void playChannel1(uint8 index) {
		DreamBase::playChannel1(index);
	}
	void findRoomInLoc();
	void reelsOnScreen();
	void reconstruct();
	void look();
	void autoLook();
	void doLook();
	void showFirstUse();
	void showSecondUse();
	void actualSave();
	void actualLoad();
	void loadPosition(unsigned int slot);
	void savePosition(unsigned int slot, const char *descbuf);
	void restoreAll();
	void enterSymbol();
	void viewFolder();
	void edensCDPlayer();
	void hotelBell();
	void playGuitar();
	void callEdensDLift();
	void callEdensLift();
	void sitDownInBar();
	void trapDoor();
	void useBalcony();
	void useChurchHole();
	void useCoveredBox();
	void useElevator1();
	void useElevator2();
	void useElevator3();
	void useElevator4();
	void useElevator5();
	void useDryer();
	void useRailing();
	void useWindow();
	void useHatch();
	void useLighter();
	void useSLab();
	void usePipe();
	void useOpenBox();
	void useAxe();
	void useKey();
	void wheelSound();
	void callHotelLift();
	void useShield();
	void useWall();
	void useChurchGate();
	void useFullCart();
	void useClearBox();
	void usePlate();
	void usePlinth();
	void useElvDoor();
	void useObject();
	void useWinch();
	void useCardReader1();
	void useCardReader2();
	void useCardReader3();
	void usePoolReader();
	void useCooker();
	void useWire();
	void useControl();
	void useHandle();
	void useAltar();
	bool defaultUseHandler(const char *id);
	void openTVDoor();
	void wearWatch();
	void wearShades();
	void checkFolderCoords();
	void nextFolder();
	void lastFolder();
	void setLocation();
	void setLocation(uint8 index) {
		DreamBase::setLocation(index);
	}
	void drawFloor();
	void allocateBuffers();
	bool checkSpeed(ReelRoutine &routine);
	void checkSpeed();
	void sparkyDrip(ReelRoutine &routine);
	void genericPerson(ReelRoutine &routine);
	void gamer(ReelRoutine &routine);
	void eden(ReelRoutine &routine);
	void sparky(ReelRoutine &routine);
	void rockstar(ReelRoutine &routine);
	void madmansTelly(ReelRoutine &routine);
	void smokeBloke(ReelRoutine &routine);
	void manAsleep(ReelRoutine &routine);
	void drunk(ReelRoutine &routine);
	void introMagic1(ReelRoutine &routine);
	void introMagic2(ReelRoutine &routine);
	void introMagic3(ReelRoutine &routine);
	void introMusic(ReelRoutine &routine);
	void candles(ReelRoutine &routine);
	void candles1(ReelRoutine &routine);
	void candles2(ReelRoutine &routine);
	void smallCandle(ReelRoutine &routine);
	void gates(ReelRoutine &routine);
	void security(ReelRoutine &routine);
	void edenInBath(ReelRoutine &routine);
	void louis(ReelRoutine &routine);
	void handClap(ReelRoutine &routine);
	void carParkDrip(ReelRoutine &routine);
	void foghornSound(ReelRoutine &routine);
	void train(ReelRoutine &routine);
	void attendant(ReelRoutine &routine);
	void keeper(ReelRoutine &routine);
	void interviewer(ReelRoutine &routine);
	void drinker(ReelRoutine &routine);
	void alleyBarkSound(ReelRoutine &routine);
	void louisChair(ReelRoutine &routine);
	void bossMan(ReelRoutine &routine);
	void priest(ReelRoutine &routine);
	void monkAndRyan(ReelRoutine &routine);
	void copper(ReelRoutine &routine);
	void introMonks1(ReelRoutine &routine);
	void introMonks2(ReelRoutine &routine);
	void soldier1(ReelRoutine &routine);
	void receptionist(ReelRoutine &routine);
	void bartender(ReelRoutine &routine);
	void heavy(ReelRoutine &routine);
	void helicopter(ReelRoutine &routine);
	void singleKey(uint8 key, uint16 x, uint16 y);
	void loadSaveBox();
	uint8 nextSymbol(uint8 symbol);
	void showSymbol();
	void enterCode(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3);
	unsigned int scanForNames();
	void doLoad(int slot);
	void loadOld();
	void inventory();
	void mainScreen();
	void loadGame();
	void saveGame();
	void zoomOnOff();
	void atmospheres();
	void loadPersonal();
	void loadNews();
	void loadCart();
	void hangOne(uint16 delay);
	void hangOne();
	void bibleQuote();
	void realCredits();
	void runIntroSeq();
	void intro();
	void newGame();
	void pickupOb(uint8 command, uint8 pos);
	void initialInv();
	void walkIntoRoom();
	void loadIntroRoom();
	void afterIntroRoom();
	void gettingShot();
	void redrawMainScrn();
	void selectSlot();
	void selectSlot2();
	void allPointer();
	void openYourNeighbour();
	void openRyan();
	void openPoolBoss();
	void openEden();
	void openSarters();
	void openLouis();
	void useLadder();
	void useLadderB();
	void useCart();
	void useTrainer();
	void useHole();
	void runTap();
	void chewy();
	void sLabDoorA();
	void sLabDoorB();
	void sLabDoorC();
	void sLabDoorE();
	void sLabDoorD();
	void sLabDoorF();
	void openHotelDoor();
	void openHotelDoor2();
	void grafittiDoor();
	void openTomb();
	void hotelControl();
	void obsThatDoThings();
	void makeMainScreen();
	void delEverything();
	void clearPalette();
	void errorMessage1();
	void errorMessage2();
	void errorMessage3();
	void reExFromOpen();
	void nextDest();
	void lastDest();
	void destSelect();
	void putBackObStuff();
	void moreTalk();
	void redes();
	void isSetObOnMap();
	bool isSetObOnMap(uint8 index);
	void dumpZoom();
	void selectLocation();
	void showGroup();
	void loadSpeech();
	bool loadSpeech(byte type1, int idx1, byte type2, int idx2) {
		return DreamBase::loadSpeech(type1, idx1, type2, idx2);
	}
	void getTime();
	void set16ColPalette();
	void showSaveOps();
	void showLoadOps();
	void watchReel();
	void showWatchReel();
	void plotReel(uint16 &reelPointer);
	void removeFreeObject(uint8 index) {
		DreamBase::removeFreeObject(index);
	}
	void afterNewRoom();
	void madmanRun();
	void showDecisions();
	void decide();
	void talk();
	void discOps();
	void doSaveLoad();
	void useDiary();
	void hangOnPQ();
	void showGun();
	void endGame();
	void diaryKeyP();
	void diaryKeyN();
	void checkInput();
	void dropError();
	void cantDrop();
	void newPlace();
	void showPuzText(uint16 command, uint16 count);
	void showPuzText();
	void monkSpeaking();
	void rollEndCredits2();
	void useButtonA();
	void autoAppear();
	void setupTimedUse();
	void entryAnims();
	void triggerMessage(uint16 index);
	void processTrigger();
	void updateSymbolTop();
	void updateSymbolBot();
	void showDiaryPage();
	void dumpDiaryKeys();
	void runEndSeq();
	void lookAtCard();
	bool execCommand();
	void findSetObject();
	uint16 findSetObject(const char *id);
	void findExObject();
	uint16 findExObject(const char *id);
	void describeOb();
	void getOpenedSize();
	byte getOpenedSizeCPP();
	void openOb();
	void withWhat();
	void notHeldError();

#endif
