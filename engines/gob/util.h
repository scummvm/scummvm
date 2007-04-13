/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#include "gob/video.h"

namespace Gob {

#define KEYBUFSIZE 16

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

	void delay(uint16 msecs);
	void longDelay(uint16 msecs);

	void initInput(void);
	void processInput(bool scroll = false);
	void clearKeyBuf(void);
	int16 getKey(void);
	int16 checkKey(void);

	void getMouseState(int16 *pX, int16 *pY, int16 *pButtons);
	void setMousePos(int16 x, int16 y);
	void waitMouseUp(void);
	void waitMouseDown(void);
	void waitMouseRelease(char drawMouse);
	void forceMouseUp(void);

	void clearPalette(void);
	void setFrameRate(int16 rate);
	void waitEndFrame();
	void setScrollOffset(int16 x = -1, int16 y = -1);

	Video::FontDesc *loadFont(const char *path);
	void freeFont(Video::FontDesc *fontDesc);

	static void insertStr(const char *str1, char *str2, int16 pos);
	static void cutFromStr(char *str, int16 from, int16 cutlen);
	static void prepareStr(char *str);

	static void listInsertFront(List *list, void *data);
	static void listInsertBack(List *list, void *data);
	static void listDropFront(List *list);
	static void deleteList(List *list);

	Util(GobEngine *vm);

protected:
	int16 _mouseButtons;
	int16 _keyBuffer[KEYBUFSIZE];
	int16 _keyBufferHead;
	int16 _keyBufferTail;

	uint8 _fastMode;

	GobEngine *_vm;

	bool keyBufferEmpty();
	void addKeyToBuffer(int16 key);
	bool getKeyFromBuffer(int16& key);
	int16 translateKey(int16 key);
	void checkJoystick();
};

} // End of namespace Gob

#endif // GOB_UTIL_H
