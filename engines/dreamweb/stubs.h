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
	void seeCommandTail();
	void quickQuit2();
	void printDirect();
	uint8 printDirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered) {
		return DreamBase::printDirect(string, x, y, maxWidth, centered);
	}
	uint8 printDirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
		return DreamBase::printDirect(string, x, y, maxWidth, centered);
	}
	void width160();
	bool checkIfPerson(uint8 x, uint8 y);
	bool checkIfFree(uint8 x, uint8 y);
	bool checkIfEx(uint8 x, uint8 y);
	DynObject *getFreeAd(uint8 index) {
		return DreamBase::getFreeAd(index);
	}
	DynObject *getExAd(uint8 index) {
		return DreamBase::getExAd(index);
	}
	void *getAnyAd(uint8 *slotSize, uint8 *slotCount) {
		return DreamBase::getAnyAd(slotSize, slotCount);
	}
	SetObject *getSetAd(uint8 index) {
		return DreamBase::getSetAd(index);
	}
	void walkAndExamine();
	void obName(uint8 command, uint8 commandType);

	void getExPos();
	bool checkIfSet(uint8 x, uint8 y);
	void obToInv();
	void obToInv(uint8 index, uint8 flag, uint16 x, uint16 y) {
		DreamBase::obToInv(index, flag, x, y);
	}
	void useRoutine();
	void examineOb(bool examineAgain = true);
	void dumpWatch();
	void transferText();
	void watchCount();
	void readSetData();
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
	void look();
	void autoLook();
	void doLook();
	void useKey();
	void useObject();
	void singleKey(uint8 key, uint16 x, uint16 y);
	void inventory();
	void mainScreen();
	void zoomOnOff();
	void pickupOb(uint8 command, uint8 pos);
	void initialInv();
	void walkIntoRoom();
	void allPointer();
	void errorMessage1();
	void errorMessage2();
	void errorMessage3();
	void afterNewRoom();
	void madmanRun();
	void decide();
	void showGun();
	void triggerMessage(uint16 index);
	void processTrigger();
	bool execCommand();
	bool checkObjectSizeCPP();
	void identifyOb();
	void selectOb();
	void findInvPos();
	uint16 findInvPosCPP();
	void setPickup();
	void getKeyAndLogo();
	void signOn();
	void inToInv();
	void outOfInv();
	void selectOpenOb();
	void reExFromInv();
	void useOpened();
	void outOfOpen();
	void swapWithOpen();
	void swapWithInv();
	void searchForFiles();
	byte transferToEx();

#endif
