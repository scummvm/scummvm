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
	void multiGet(uint8 *dst, uint16 x, uint16 y, uint8 width, uint8 height) {
		DreamBase::multiGet(dst, x, y, width, height);
	}
	void multiPut(const uint8 *src, uint16 x, uint16 y, uint8 width, uint8 height) {
		DreamBase::multiPut(src, x, y, width, height);
	}
	void multiDump(uint16 x, uint16 y, uint8 width, uint8 height) {
		DreamBase::multiDump(x, y, width, height);
	}
	void quickQuit();
	void readOneBlock();
	void seeCommandTail();
	void quickQuit2();
	uint8 printDirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered) {
		return DreamBase::printDirect(string, x, y, maxWidth, centered);
	}
	uint8 printDirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
		return DreamBase::printDirect(string, x, y, maxWidth, centered);
	}
	bool checkIfPerson(uint8 x, uint8 y);
	bool checkIfFree(uint8 x, uint8 y);
	bool checkIfEx(uint8 x, uint8 y);
	void walkAndExamine();
	void obName(uint8 command, uint8 commandType);

	bool checkIfSet(uint8 x, uint8 y);
	void obToInv(uint8 index, uint8 flag, uint16 x, uint16 y) {
		DreamBase::obToInv(index, flag, x, y);
	}
	void readSetData();
	void look();
	void autoLook();
	void doLook();
	void inventory();
	void mainScreen();
	void zoomOnOff();
	void initialInv();
	void walkIntoRoom();
	void afterNewRoom();
	void madmanRun();
	void showGun();
	void identifyOb();

#endif
