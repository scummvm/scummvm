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

public:
	DreamBase(DreamWeb::DreamWebEngine *en);

public:
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

	// from object.cpp
	void obIcons();

	// from pathfind.cpp
	void checkDest(const RoomPaths *roomsPaths);
	RoomPaths *getRoomsPaths();
	void faceRightWay();
	void setWalk();
	void autoSetWalk();
	void findXYFromPath();
	void bresenhams();
	void workoutFrames();

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
	char _saveNames[17*7];
	char _saveNamesOld[17*7];
	void showNames();

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
	Reel *getReelStart(uint16 reelPointer);
	const Frame *findSource(uint16 &frame);
	void showReelFrame(Reel *reel);
	const Frame *getReelFrameAX(uint16 frame);
	void soundOnReels(uint16 reelPointer);
	void rollEndCredits();
	void priestText(ReelRoutine &routine);

	// from stubs.cpp
	void crosshair();
	void delTextLine();
	void showBlink();
	void dumpBlink();
	void dumpPointer();
	void showPointer();
	void delPointer();
	void showRyanPage();
	Frame *tempGraphics();
	Frame *tempGraphics2();
	Frame *tempGraphics3();
	void showArrows();
	void showOpBox();
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
	void showWatch();
	void showTime();
	void showExit();
	void showMan();
	void panelIcons1();
	SetObject *getSetAd(uint8 index);
	void *getAnyAd(uint8 *value1, uint8 *value2);
	const uint8 *getTextInFile1(uint16 index);
	uint8 findNextColon(const uint8 **string);
	uint16 allocateMem(uint16 paragraphs);
	void deallocateMem(uint16 segment);
	uint16 allocateAndLoad(unsigned int size);
	uint16 standardLoad(const char *fileName, uint16 *outSizeInBytes = NULL); // Returns a segment handle which needs to be freed with deallocatemem for symmetry
	void *standardLoadCPP(const char *fileName, uint16 *outSizeInBytes = NULL); // And this one should be 'free'd
	void loadIntoTemp(const char *fileName);
	void loadIntoTemp2(const char *fileName);
	void loadIntoTemp3(const char *fileName);
	void loadTempCharset(const char *fileName);
	void clearAndLoad(uint8 *buf, uint8 c, unsigned int size, unsigned int maxSize);
	void clearAndLoad(uint16 seg, uint8 c, unsigned int size, unsigned int maxSize);
	void sortOutMap();
	void loadRoomData(const Room &room, bool skipDat);
	void useTempCharset();
	void useCharset1();

	// from use.cpp
	void placeFreeObject(uint8 index);
	void removeFreeObject(uint8 index);
	void setupTimedUse(uint16 offset, uint16 countToTimed, uint16 timeCount, byte x, byte y);

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
	void fadeScreenUp();
	void fadeScreenUps();
	void fadeScreenUpHalf();
	void fadeScreenDown();
	void fadeScreenDowns();
	void greyscaleSum();
	void allPalette();
	void dumpCurrent();

	// from vgagrafx.cpp
	uint8 _workspace[(0x1000 + 2) * 16];
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
	void loadPalFromIFF();
	void createPanel();
	void createPanel2();
	void showPanel();
	void entryTexts();
};


} // End of namespace DreamGen

#endif
