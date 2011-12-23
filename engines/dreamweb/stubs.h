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
	void startup1();
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
	void quickQuit();
	void readOneBlock();
	void readCityPic();
	void readDestIcon();
	void seeCommandTail();
	void quickQuit2();
	void printDirect();
	uint8 printDirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered) {
		return DreamBase::printDirect(string, x, y, maxWidth, centered);
	}
	uint8 printDirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
		return DreamBase::printDirect(string, x, y, maxWidth, centered);
	}
	void showFrame();
	void showFrame(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height) {
		DreamBase::showFrame(frameData, x, y, frameNumber, effectsFlag, width, height);
	}
	void showFrame(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag) {
		DreamBase::showFrame(frameData, x, y, frameNumber, effectsFlag);
	}
	void width160();
	void commandOnly();
	void commandOnly(uint8 command) {
		DreamBase::commandOnly(command);
	}
	void checkIfPerson();
	bool checkIfPerson(uint8 x, uint8 y);
	void checkIfFree();
	bool checkIfFree(uint8 x, uint8 y);
	void checkIfEx();
	bool checkIfEx(uint8 x, uint8 y);
	void commandWithOb();
	void commandWithOb(uint8 command, uint8 type, uint8 index) {
		DreamBase::commandWithOb(command, type, index);
	}
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
	bool finishedWalkingCPP();
	void finishedWalking();
	void checkOne();
	void checkOne(uint8 x, uint8 y, uint8 *flag, uint8 *flagEx, uint8 *type, uint8 *flagX, uint8 *flagY) {
		DreamBase::checkOne(x, y, flag, flagEx, type, flagX, flagY);
	}
	void walkAndExamine();
	void obName(uint8 command, uint8 commandType);

	void getExPos();
	void compare();
	bool compare(uint8 index, uint8 flag, const char id[4]) {
		return DreamBase::compare(index, flag, id);
	}
	bool checkIfSet(uint8 x, uint8 y);
	void isItWorn();
	bool isItWorn(const DynObject *object) {
		return DreamBase::isItWorn(object);
	}
	void wornError();
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
	uint8 findNextColon(const uint8 **string) {
		return DreamBase::findNextColon(string);
	}
	void findNextColon();
	const uint8 *getObTextStartCPP();
	void useText(const uint8 *string);
	void showCity();
	uint16 getPersFrame(uint8 index);
	void convIcons();
	void examineOb(bool examineAgain = true);
	void dumpWatch();
	void transferText();
	void watchCount();
	void readSetData();
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
	void look();
	void autoLook();
	void doLook();
	void showFirstUse();
	void showSecondUse();
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
	void singleKey(uint8 key, uint16 x, uint16 y);
	uint8 nextSymbol(uint8 symbol);
	void showSymbol();
	void enterCode(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3);
	void inventory();
	void mainScreen();
	void zoomOnOff();
	void hangOne(uint16 delay);
	void hangOne();
	void bibleQuote();
	void realCredits();
	void runIntroSeq();
	void intro();
	void pickupOb(uint8 command, uint8 pos);
	void initialInv();
	void walkIntoRoom();
	void loadIntroRoom();
	void afterIntroRoom();
	void gettingShot();
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
	void delEverything();
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
	void selectLocation();
	void loadSpeech();
	bool loadSpeech(byte type1, int idx1, byte type2, int idx2) {
		return DreamBase::loadSpeech(type1, idx1, type2, idx2);
	}
	void set16ColPalette();
	void afterNewRoom();
	void madmanRun();
	void decide();
	void talk();
	void useDiary();
	void hangOnPQ();
	void showGun();
	void endGame();
	void dropError();
	void cantDrop();
	void newPlace();
	void monkSpeaking();
	void rollEndCredits2();
	void useButtonA();
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
	void findExObject();
	uint16 findExObject(const char *id) {
		return DreamBase::findExObject(id);
	}
	void isRyanHolding();
	bool isRyanHolding(const char *id) {
		return DreamBase::isRyanHolding(id);
	}
	void describeOb();
	void getOpenedSize();
	byte getOpenedSizeCPP();
	void openOb();
	void notHeldError();
	void useGun();
	void identifyOb();
	void useCashCard();
	void useStereo();
	void selectOb();
	void findInvPos();
	uint16 findInvPosCPP();
	void setPickup();
	void showDiaryKeys();
	void showKeys();
	void getKeyAndLogo();
	void deleteExObject();
	void deleteExObject(uint8 index) {
		DreamBase::deleteExObject(index);
	}
	void deleteExFrame();
	void deleteExFrame(uint8 frameNum) {
		DreamBase::deleteExFrame(frameNum);
	}
	void deleteExText();
	void deleteExText(uint8 textNum) {
		DreamBase::deleteExText(textNum);
	}
	void signOn();
	void lookAtPlace();
	void inToInv();
	void outOfInv();
	void edensFlatReminders();
	void incRyanPage();

#endif
