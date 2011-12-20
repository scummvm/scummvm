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

#ifndef DREAMBASE_H
#define DREAMBASE_H

#include "common/scummsys.h"

#include "dreamweb/segment.h"

namespace DreamWeb {
	class DreamWebEngine;
}


namespace DreamGen {


const unsigned int kNumReelRoutines = 57;

/**
 * This class is one of the parent classes of DreamGenContext. Its sole purpose
 * is to allow us to incrementally move things out of DreamGenContext into this
 * base class, as soon as they don't modify any context registers (ax, bx, cx, ...)
 * anymore.
 * Ultimately, DreamGenContext should be empty, at which point it can be removed
 * together with class Context. When that happens, we can probably merge
 * DreamBase into DreamWebEngine.
 */
class DreamBase : public SegmentManager {
protected:
	DreamWeb::DreamWebEngine *engine;

	// from object.cpp
	uint16 _openChangeSize;

	// from pathfind.cpp
	Common::Point _lineData[200];		// Output of Bresenham

	// from saveload.cpp
	char _saveNames[17*7];
	char _saveNamesOld[17*7];

	// from vgagrafx.cpp
	uint8 _workspace[(0x1000 + 2) * 16];

	// from people.cpp
	ReelRoutine _reelRoutines[kNumReelRoutines+1];

public:
	DreamBase(DreamWeb::DreamWebEngine *en);

public:
	// from backdrop.cpp
	void doBlocks();
	uint8 getXAd(const uint8 *setData, uint8 *result);
	uint8 getYAd(const uint8 *setData, uint8 *result);
	uint8 getMapAd(const uint8 *setData);
	void calcFrFrame(uint16 frame, uint8* width, uint8* height);
	void finalFrame(uint16 *x, uint16 *y);
	void makeBackOb(SetObject *objData);
	void showAllObs();
	bool addAlong(const uint8 *mapFlags);
	bool addLength(const uint8 *mapFlags);
	void getDimension(uint8 *mapXstart, uint8 *mapYstart, uint8 *mapXsize, uint8 *mapYsize);
	void calcMapAd();
	void showAllFree();
	void drawFlags();
	void showAllEx();

	// from keypad.cpp
	void getUnderMenu();
	void putUnderMenu();
	void singleKey(uint8 key, uint16 x, uint16 y);
	void loadKeypad();
	void showKeypad();
	bool isItRight(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3);
	void addToPressList();
	void buttonOne();
	void buttonTwo();
	void buttonThree();
	void buttonFour();
	void buttonFive();
	void buttonSix();
	void buttonSeven();
	void buttonEight();
	void buttonNine();
	void buttonNought();
	void buttonEnter();
	void buttonPress(uint8 buttonId);
	void showOuterPad();
	void dumpKeypad();
	void dumpSymbol();
	void dumpSymBox();
	void quitSymbol();

	// from monitor.cpp
	void input();
	byte makeCaps(byte c);
	void delChar();
	void monMessage(uint8 index);
	void netError();
	void monitorLogo();
	void randomAccess(uint16 count);
	void printOuterMon();
	void showCurrentFile();
	void accessLightOn();
	void accessLightOff();
	void turnOnPower();
	void powerLightOn();
	void powerLightOff();
	void printLogo();
	void scrollMonitor();
	const char *monPrint(const char *string);
	void lockLightOn();
	void lockLightOff();
	void loadPersonal();
	void loadNews();
	void loadCart();

	// from object.cpp
	void obIcons();
	void fillRyan();
	void findAllRyan(uint8 *inv);
	void obToInv(uint8 index, uint8 flag, uint16 x, uint16 y);
	void obPicture();
	void removeObFromInv();
	void deleteExObject(uint8 index);
	void deleteExFrame(uint8 frameNum);
	void deleteExText(uint8 textNum);

	// from pathfind.cpp
	void turnPathOn(uint8 param);
	void turnPathOff(uint8 param);
	void turnAnyPathOn(uint8 param, uint8 room);
	void turnAnyPathOff(uint8 param, uint8 room);
	RoomPaths *getRoomsPaths();
	void faceRightWay();
	void setWalk();
	void autoSetWalk();
	void checkDest(const RoomPaths *roomsPaths);
	void findXYFromPath();
	bool checkIfPathIsOn(uint8 index);
	void bresenhams();
	void workoutFrames();

	// from people.cpp
	void setupInitialReelRoutines();
	void updatePeople();
	void madmanText();
	void madman(ReelRoutine &routine);
	void madMode();
	void addToPeopleList(ReelRoutine *routine);
	bool checkSpeed(ReelRoutine &routine);
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
	void mugger(ReelRoutine &routine);
	void businessMan(ReelRoutine &routine);
	void endGameSeq(ReelRoutine &routine);
	void poolGuard(ReelRoutine &routine);

	// from print.cpp
	uint8 getNextWord(const Frame *charSet, const uint8 *string, uint8 *totalWidth, uint8 *charCount);
	void printChar(const Frame* charSet, uint16 *x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height);
	void printChar(const Frame* charSet, uint16 x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height);
	void printBoth(const Frame* charSet, uint16 *x, uint16 y, uint8 c, uint8 nextChar);
	uint8 printDirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered);
	uint8 printDirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered);
	uint8 getNumber(const Frame *charSet, const uint8 *string, uint16 maxWidth, bool centered, uint16 *offset);
	uint8 kernChars(uint8 firstChar, uint8 secondChar, uint8 width);
	uint8 printSlow(const uint8 *string, uint16 x, uint16 y, uint8 maxWidth, bool centered);
	uint16 waitFrames();
	void printCurs();
	void delCurs();

	// from saveload.cpp
	void oldToNames();
	void namesToOld();
	void showMainOps();
	void showDiscOps();
	void actualSave();
	void actualLoad();
	void loadPosition(unsigned int slot);
	void savePosition(unsigned int slot, const char *descbuf);
	void loadSaveBox();
	void showNames();
	void checkInput();
	void selectSlot();
	void showSlots();
	void showOpBox();
	void showSaveOps();
	void showLoadOps();

	// from sound.cpp
	bool loadSpeech(byte type1, int idx1, byte type2, int idx2);
	void volumeAdjust();
	void cancelCh0();
	void cancelCh1();
	void loadRoomsSample();
	void playChannel0(uint8 index, uint8 repeat);
	void playChannel1(uint8 index);

	// from sprite.cpp
	Sprite *spriteTable();
	void printSprites();
	void printASprite(const Sprite *sprite);
	void clearSprites();
	Sprite *makeSprite(uint8 x, uint8 y, uint16 updateCallback, uint16 frameData, uint16 somethingInDi);
	void initMan();
	void walking(Sprite *sprite);
	void aboutTurn(Sprite *sprite);
	void backObject(Sprite *sprite);
	void constant(Sprite *sprite, SetObject *objData);
	void randomSprite(Sprite *sprite, SetObject *objData);
	void doorway(Sprite *sprite, SetObject *objData);
	void wideDoor(Sprite *sprite, SetObject *objData);
	void doDoor(Sprite *sprite, SetObject *objData, Common::Rect check);
	void steady(Sprite *sprite, SetObject *objData);
	void lockedDoorway(Sprite *sprite, SetObject *objData);
	void liftSprite(Sprite *sprite, SetObject *objData);

	Reel *getReelStart(uint16 reelPointer);
	const Frame *findSource(uint16 &frame);
	void showReelFrame(Reel *reel);
	void showGameReel(ReelRoutine *routine);
	const Frame *getReelFrameAX(uint16 frame);
	void moveMap(uint8 param);
	void checkOne(uint8 x, uint8 y, uint8 *flag, uint8 *flagEx, uint8 *type, uint8 *flagX, uint8 *flagY);

	uint8 getBlockOfPixel(uint8 x, uint8 y);
	Rain *splitIntoLines(uint8 x, uint8 y, Rain *rain);
	void initRain();

	void intro1Text();
	void intro2Text(uint16 nextReelPointer);
	void intro3Text(uint16 nextReelPointer);

	void rollEndCredits();
	void monks2text();
	void textForEnd();
	void textForMonkHelper(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount);
	void textForMonk();
	void priestText(ReelRoutine &routine);
	void soundOnReels(uint16 reelPointer);
	void clearBeforeLoad();
	void clearReels();
	void getRidOfReels();
	void liftNoise(uint8 index);
	void checkForExit(Sprite *sprite);
	void mainMan(Sprite *sprite);
	void spriteUpdate();
	void showRain();
	void reconstruct();
	void reelsOnScreen();

	// from stubs.cpp
	bool isCD();
	void crosshair();
	void delTextLine();
	void showBlink();
	void dumpBlink();
	void dumpPointer();
	void showPointer();
	void delPointer();
	void showRyanPage();
	void switchRyanOn();
	void switchRyanOff();
	Frame *tempGraphics();
	Frame *tempGraphics2();
	Frame *tempGraphics3();
	void showArrows();
	void middlePanel();
	void showDiary();
	void readMouse();
	uint16 readMouseState();
	void hangOn(uint16 frameCount);
	bool quitRequested();
	void lockMon();
	uint8 *textUnder();
	void readKey();
	void findOrMake(uint8 index, uint8 value, uint8 type);
	DynObject *getFreeAd(uint8 index);
	DynObject *getExAd(uint8 index);
	DynObject *getEitherAdCPP();
	void *getAnyAdDir(uint8 index, uint8 flag);
	void showWatch();
	void showTime();
	void showExit();
	void showMan();
	void panelIcons1();
	SetObject *getSetAd(uint8 index);
	void *getAnyAd(uint8 *value1, uint8 *value2);
	const uint8 *getTextInFile1(uint16 index);
	uint8 findNextColon(const uint8 **string);
	void allocateBuffers();
	uint16 allocateMem(uint16 paragraphs);
	void deallocateMem(uint16 segment);
	uint16 allocateAndLoad(unsigned int size);
	uint16 standardLoad(const char *fileName, uint16 *outSizeInBytes = NULL); // Returns a segment handle which needs to be freed with deallocatemem for symmetry
	void *standardLoadCPP(const char *fileName, uint16 *outSizeInBytes = NULL); // And this one should be 'free'd
	void loadIntoTemp(const char *fileName);
	void loadIntoTemp2(const char *fileName);
	void loadIntoTemp3(const char *fileName);
	void loadTempCharset(const char *fileName);
	void loadTravelText();
	void loadTempText(const char *fileName);
	void clearAndLoad(uint8 *buf, uint8 c, unsigned int size, unsigned int maxSize);
	void clearAndLoad(uint16 seg, uint8 c, unsigned int size, unsigned int maxSize);
	void sortOutMap();
	void loadRoomData(const Room &room, bool skipDat);
	void useTempCharset();
	void useCharset1();
	void printMessage(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered);
	void printMessage2(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered, uint8 count);
	bool isItDescribed(const ObjPos *objPos);
	void zoomIcon();
	void roomName();
	void showIcon();
	void eraseOldObs();
	void commandOnly(uint8 command);
	void blank();
	void setTopLeft();
	void setTopRight();
	void setBotLeft();
	void setBotRight();
	void examIcon();
	void animPointer();
	void getFlagUnderP(uint8 *flag, uint8 *flagEx);
	void workToScreenM();
	void quitKey();
	void restoreReels();
	void loadFolder();
	void folderHints();
	void folderExit();
	void showFolder();
	void showLeftPage();
	void showRightPage();
	void underTextLine();
	void hangOnP(uint16 count);
	void getUnderZoom();
	void putUnderZoom();
	void examineInventory();
	void openInv();
	void getBack1();
	void getBackFromOb();
	void getBackFromOps();
	void getBackToOps();
	void DOSReturn();
	bool isItWorn(const DynObject *object);
	bool compare(uint8 index, uint8 flag, const char id[4]);
	void hangOnW(uint16 frameCount);
	void getRidOfTemp();
	void getRidOfTempText();
	void getRidOfTemp2();
	void getRidOfTemp3();
	void getRidOfTempCharset();
	void getRidOfTempsP();
	void getRidOfAll();
	void placeSetObject(uint8 index);
	void removeSetObject(uint8 index);
	bool isSetObOnMap(uint8 index);
	void dumpZoom();
	void diaryKeyP();
	void diaryKeyN();
	void findRoomInLoc();
	void loadMenu();
	void showMenu();
	void dumpMenu();
	void dealWithSpecial(uint8 firstParam, uint8 secondParam);
	void plotReel(uint16 &reelPointer);
	void setupTimedTemp(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount);
	void getUnderTimed();
	void putUnderTimed();
	void dumpTextLine();
	void useTimedText();
	void dumpTimedText();
	void getTime();
	void doChange(uint8 index, uint8 value, uint8 type);
	bool isRyanHolding(const char *id);
	void clearBuffers();
	void clearChanges();
	void drawFloor();
	uint16 findSetObject(const char *id);
	void hangOnCurs(uint16 frameCount);
	const uint8 *findObName(uint8 type, uint8 index);
	void copyName(uint8 type, uint8 index, uint8 *dst);
	uint16 findExObject(const char *id);
	void makeMainScreen();
	void showWatchReel();
	void watchReel();
	void commandWithOb(uint8 command, uint8 type, uint8 index);
	void examineObText();
	void blockNameText();
	void personNameText();
	void walkToText();
	void entryTexts();
	void setAllChanges();
	void restoreAll();
	void redrawMainScrn();
	template <class T> void checkCoords(const RectWithCallback<T> *rectWithCallbacks);

	// from use.cpp
	void placeFreeObject(uint8 index);
	void removeFreeObject(uint8 index);
	void setupTimedUse(uint16 offset, uint16 countToTimed, uint16 timeCount, byte x, byte y);
	void withWhat();
	uint16 checkInside(uint16 command, uint16 type);
	void showPuzText(uint16 command, uint16 count);

	// from vgafades.cpp
	uint8 *mainPalette();
	uint8 *startPalette();
	uint8 *endPalette();
	void clearStartPal();
	void clearEndPal();
	void palToStartPal();
	void endPalToStart();
	void startPalToEnd();
	void palToEndPal();
	void fadeDOS();
	void doFade();
	void fadeCalculation();
	void fadeUpYellows();
	void fadeUpMonFirst();
	void fadeUpMon();
	void fadeDownMon();
	void initialMonCols();
	void fadeScreenUp();
	void fadeScreenUps();
	void fadeScreenUpHalf();
	void fadeScreenDown();
	void fadeScreenDowns();
	void fadeScreenDownHalf();
	void clearPalette();
	void greyscaleSum();
	void allPalette();
	void dumpCurrent();

	// from vgagrafx.cpp
	inline uint8 *workspace() { return _workspace; }
	void clearWork();

	uint8 getLocation(uint8 index);
	void setLocation(uint8 index);
	void getUnderCentre();
	void putUnderCentre();
	uint8 *mapStore();
	void panelToMap();
	void mapToPanel();
	void dumpMap();
	void transferInv();
	void zoom();
	void multiGet(uint8 *dst, uint16 x, uint16 y, uint8 width, uint8 height);
	void multiPut(const uint8 *src, uint16 x, uint16 y, uint8 width, uint8 height);
	void multiDump(uint16 x, uint16 y, uint8 width, uint8 height);
	void workToScreenCPP();
	void printUnderMon();
	void cls();
	void frameOutV(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, int16 x, int16 y);
	void frameOutNm(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameOutBh(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void frameOutFx(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y);
	void doShake();
	void vSync();
	void setMode();
	void showPCX(const Common::String &name);
	void showFrame(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height);
	void showFrame(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag);
	bool pixelCheckSet(const ObjPos *pos, uint8 x, uint8 y);
	void loadPalFromIFF();
	void createPanel();
	void createPanel2();
	void showPanel();
};


} // End of namespace DreamGen

#endif
