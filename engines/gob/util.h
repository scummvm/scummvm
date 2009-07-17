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

#ifndef GOB_UTIL_H
#define GOB_UTIL_H

#include "common/keyboard.h"

namespace Gob {

class GobEngine;

#define KEYBUFSIZE 16

enum MouseButtons {
	kMouseButtonsNone  = 0,
	kMouseButtonsLeft  = 1,
	kMouseButtonsRight = 2,
	kMouseButtonsBoth  = 3,
	kMouseButtonsAny   = 4
};

enum Keys {
	kKeyNone      = 0x0000,
	kKeyBackspace = 0x0E08,
	kKeySpace     = 0x3920,
	kKeyReturn    = 0x1C0D,
	kKeyEscape    = 0x011B,
	kKeyDelete    = 0x5300,
	kKeyUp        = 0x4800,
	kKeyDown      = 0x5000,
	kKeyRight     = 0x4D00,
	kKeyLeft      = 0x4B00,
	kKeyF1        = 0x3B00,
	kKeyF2        = 0x3C00,
	kKeyF3        = 0x3D00,
	kKeyF4        = 0x3E00,
	kKeyF5        = 0x3F00,
	kKeyF6        = 0x4000,
	kKeyF7        = 0x4100,
	kKeyF8        = 0x4200,
	kKeyF9        = 0x4300,
	kKeyF10       = 0x4400
};

enum ShortKey {
	kShortKeyUp        = 0x0B,
	kShortKeyDown      = 0x0A,
	kShortKeyRight     = 0x09,
	kShortKeyLeft      = 0x08,
	kShortKeyEscape    = 0x1B,
	kShortKeyBackspace = 0x19,
	kShortKeyDelete    = 0x1A
};

class Util {
public:
	struct ListNode;
	struct ListNode {
		void *pData;
		struct ListNode *pNext;
		struct ListNode *pPrev;
		ListNode() : pData(0), pNext(0), pPrev(0) {}
	};

	struct List {
		ListNode *pHead;
		ListNode *pTail;
		List() : pHead(0), pTail(0) {}
	};

	uint32 getTimeKey(void);
	int16 getRandom(int16 max);
	void beep(int16 freq);

	void notifyPaused(uint32 duration);

	void delay(uint16 msecs);
	void longDelay(uint16 msecs);

	void initInput(void);
	void processInput(bool scroll = false);
	void clearKeyBuf(void);
	int16 getKey(void);
	int16 checkKey(void);
	bool checkKey(int16 &key);

	void getMouseState(int16 *pX, int16 *pY, MouseButtons *pButtons);
	void setMousePos(int16 x, int16 y);
	void waitMouseUp(void);
	void waitMouseDown(void);
	void waitMouseRelease(char drawMouse);
	void forceMouseUp(bool onlyWhenSynced = false);

	void clearPalette(void);
	int16 getFrameRate();
	void setFrameRate(int16 rate);
	void notifyNewAnim();
	void waitEndFrame();
	void setScrollOffset(int16 x = -1, int16 y = -1);

	static void insertStr(const char *str1, char *str2, int16 pos);
	static void cutFromStr(char *str, int16 from, int16 cutlen);
	static void cleanupStr(char *str);
	static void replaceChar(char *str, char c1, char c2);

	static void listInsertFront(List *list, void *data);
	static void listInsertBack(List *list, void *data);
	static void listDropFront(List *list);
	static void deleteList(List *list);

	Util(GobEngine *vm);

protected:
	MouseButtons _mouseButtons;

	Common::KeyState _keyBuffer[KEYBUFSIZE];
	int16 _keyBufferHead;
	int16 _keyBufferTail;

	uint8 _fastMode;

	int16 _frameRate;
	int16 _frameWaitTime;
	uint32 _startFrameTime;
	int32 _frameWaitLag;

	GobEngine *_vm;

	bool keyBufferEmpty();
	void addKeyToBuffer(const Common::KeyState &key);
	bool getKeyFromBuffer(Common::KeyState &key);
	int16 translateKey(const Common::KeyState &key);
	void checkJoystick();
};

} // End of namespace Gob

#endif // GOB_UTIL_H
