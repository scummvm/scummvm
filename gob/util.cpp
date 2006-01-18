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
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/global.h"
#include "gob/timer.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/game.h"

namespace Gob {

static const int kKeyBufSize = 16;

static int16 _mouseX, _mouseY, _mouseButtons;
static int16 _keyBuffer[kKeyBufSize], _keyBufferHead, _keyBufferTail;

static void addKeyToBuffer(int16 key) {
	if ((_keyBufferHead + 1) % kKeyBufSize == _keyBufferTail) {
		warning("key buffer overflow!");
		return;
	}

	_keyBuffer[_keyBufferHead] = key;
	_keyBufferHead = (_keyBufferHead + 1) % kKeyBufSize;
}

static bool keyBufferEmpty() {
	return (_keyBufferHead == _keyBufferTail);
}

static bool getKeyFromBuffer(int16& key) {
	if (_keyBufferHead == _keyBufferTail) return false;

	key = _keyBuffer[_keyBufferTail];
	_keyBufferTail = (_keyBufferTail + 1) % kKeyBufSize;

	return true;
}


void util_initInput(void) {
	_mouseX = _mouseY = _mouseButtons = 0;
	_keyBufferHead = _keyBufferTail = 0;
}

void util_waitKey(void) {
	// FIXME: wrong function name? This functions clears the keyboard buffer.
	util_processInput();
	_keyBufferHead = _keyBufferTail = 0;
}

int16 util_translateKey(int16 key) {
	struct keyS {
		int16 from;
		int16 to;
	} keys[] = {
		{8,   0x0e08},	// Backspace
		{13,  0x1C0D},	// Enter
		{27,  0x011b},	// ESC
		{127, 0x5300},	// Del
		{273, 0x4800},	// Up arrow
		{274, 0x5000},	// Down arrow
		{275, 0x4D00},	// Right arrow
		{276, 0x4B00},	// Left arrow
		{282, 0x3b00},	// F1
		{283, 0x3c00},	// F2
		{284, 0x3d00},	// F3
		{285, 0x3E00},	// F4
		{286, 0x011b},	// F5
		{287, 0x4000},	// F6
		{288, 0x4100},	// F7
		{289, 0x4200},	// F8
		{290, 0x4300},	// F9
		{291, 0x4400}	// F10
	};
	int i;

	for (i = 0; i < ARRAYSIZE(keys); i++)
		if (key == keys[i].from)
			return keys[i].to;

	if (key < 32 || key >= 128)
		return 0;

	return key;
}

int16 util_getKey(void) {
	int16 key;

	while (!getKeyFromBuffer(key)) {
		util_processInput();

		if (keyBufferEmpty())
			g_system->delayMillis(10);
	}
	return util_translateKey(key);
}

int16 util_checkKey(void) {
	int16 key;

	if (!getKeyFromBuffer(key))
		key = 0;

	return util_translateKey(key);
}

int16 util_getRandom(int16 max) {
	return _vm->_rnd.getRandomNumber(max - 1);
}

void util_processInput() {
	OSystem::Event event;
	while (g_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			_mouseButtons |= 1;
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			_mouseButtons |= 2;
			break;
		case OSystem::EVENT_LBUTTONUP:
			_mouseButtons &= ~1;
			break;
		case OSystem::EVENT_RBUTTONUP:
			_mouseButtons &= ~2;
			break;
		case OSystem::EVENT_KEYDOWN:
			addKeyToBuffer(event.kbd.keycode);
			break;
		case OSystem::EVENT_KEYUP:
			break;
		case OSystem::EVENT_QUIT:
			g_system->quit();
			break;
		default:
			break;
		}
	}
}

void util_getMouseState(int16 *pX, int16 *pY, int16 *pButtons) {
	*pX = _mouseX;
	*pY = _mouseY;

	if (pButtons != 0)
		*pButtons = _mouseButtons;
}

void util_setMousePos(int16 x, int16 y) {
	g_system->warpMouse(x, y);
}

void util_longDelay(uint16 msecs)
{
	uint32 time = g_system->getMillis() + msecs;
	do {
		vid_waitRetrace(videoMode);
		util_processInput();
		util_delay(25);
	} while (g_system->getMillis() < time);
}

void util_delay(uint16 msecs) {
	g_system->delayMillis(msecs);
}

void util_beep(int16 freq) {
	if (soundFlags == 0)
		return;

	snd_speakerOn(freq, 50);
}

uint32 util_getTimeKey(void) {
	return g_system->getMillis();
}

void util_waitMouseUp(void) {
	int16 x;
	int16 y;
	int16 buttons;

	do {
		util_processInput();
		util_getMouseState(&x, &y, &buttons);
		if (buttons != 0) util_delay(10);
	} while (buttons != 0);
}

void util_waitMouseDown(void) {
	int16 x;
	int16 y;
	int16 buttons;

	do {
		util_processInput();
		util_getMouseState(&x, &y, &buttons);
		if (buttons == 0) util_delay(10);
	} while (buttons == 0);
}

/* NOT IMPLEMENTED */
int16 util_calcDelayTime() {
	return 0;
}

/* NOT IMPLEMENTED */
void util_checkJoystick() {
	useJoystick = 0;
}

void util_setFrameRate(int16 rate) {
	if (rate == 0)
		rate = 1;

	frameWaitTime = 1000 / rate;
	startFrameTime = util_getTimeKey();
}

void util_waitEndFrame() {
	int32 time;

	vid_waitRetrace(videoMode);

	time = util_getTimeKey() - startFrameTime;
	if (time > 1000 || time < 0) {
		startFrameTime = util_getTimeKey();
		return;
	}
	if (frameWaitTime - time > 0) {
		util_delay(frameWaitTime - time);
	}
	startFrameTime = util_getTimeKey();
}

int16 joy_getState() {
	return 0;
}

int16 joy_calibrate() {
	return 0;
}

FontDesc *util_loadFont(const char *path) {
	FontDesc *fontDesc = (FontDesc *) malloc(sizeof(FontDesc));
	char *data;

	if (fontDesc == 0)
		return 0;

	data = data_getData(path);
	if (data == 0) {
		free(fontDesc);
		return 0;
	}

	fontDesc->dataPtr = data + 4;
	fontDesc->itemWidth = data[0] & 0x7f;
	fontDesc->itemHeight = data[1];
	fontDesc->startItem = data[2];
	fontDesc->endItem = data[3];

	fontDesc->itemSize =
	    ((fontDesc->itemWidth - 1) / 8 + 1) * fontDesc->itemHeight;
	fontDesc->bitWidth = fontDesc->itemWidth;

	if (data[0] & 0x80)
		fontDesc->extraData =
		    data + 4 + fontDesc->itemSize * (fontDesc->endItem -
		    fontDesc->startItem + 1);
	else
		fontDesc->extraData = 0;
	return fontDesc;
}

void util_freeFont(FontDesc * fontDesc) {
	free(fontDesc->dataPtr - 4);
	free(fontDesc);
}

void util_clearPalette(void) {
	int16 i;
	byte colors[768];

	if (videoMode != 0x13)
		error("util_clearPalette: Video mode 0x%x is not supported!",
		    videoMode);

	if (setAllPalette) {
		for (i = 0; i < 768; i++)
			colors[i] = 0;
		g_system->setPalette(colors, 0, 256);

		return;
	}

	for (i = 0; i < 16; i++)
		vid_setPalElem(i, 0, 0, 0, 0, videoMode);
}

void util_insertStr(const char *str1, char *str2, int16 pos) {
	int16 len1;
	int16 i;
	int16 from;

	i = strlen(str2);
	len1 = strlen(str1);
	if (pos < i)
		from = pos;
	else
		from = i;

	for (; i >= from; i--)
		str2[len1 + i] = str2[i];

	for (i = 0; i < len1; i++)
		str2[i + from] = str1[i];
}

void util_cutFromStr(char *str, int16 from, int16 cutlen) {
	int16 len;
	int16 i;

	//log_write("util_cutFromStr: str = %s, ", str);
	len = strlen(str);
	if (from >= len)
		return;
	if (from + cutlen > len) {
		str[from] = 0;
		//log_write("res = %s\n", str);
		return;
	}

	i = from;
	do {
		str[i] = str[i + cutlen];
		i++;
	} while (str[i] != 0);
	//log_write("res = %s\n", str);
}

int16 util_strstr(const char *str1, char *str2) {
	char c;
	uint16 len1;
	uint16 i;

	//log_write("util_strstr: str1 = %s, str2 = %s\n", str1, str2);

	for (i = 0, len1 = strlen(str1); strlen(str2 + i) >= len1; i++) {
		c = str2[i + len1];
		str2[i + len1] = 0;
		if (strcmp(str2 + i, str1) == 0) {
			str2[i + len1] = c;
			return i + 1;
		}
		str2[i + len1] = c;
	}
	return 0;
}

void util_listInsertFront(Util_List * list, void *data) {
	Util_ListNode *node;

	node = (Util_ListNode *) malloc(sizeof(Util_ListNode));
	if (list->pHead != 0) {
		node->pData = data;
		node->pNext = list->pHead;
		node->pPrev = 0;
		list->pHead->pPrev = node;
		list->pHead = node;
	} else {
		list->pHead = node;
		list->pTail = node;
		node->pData = data;
		node->pNext = 0;
		node->pPrev = 0;
	}
}

void util_listInsertBack(Util_List * list, void *data) {
	Util_ListNode *node;

	if (list->pHead != 0) {
		if (list->pTail == 0) {
			list->pTail = list->pHead;
			warning("util_listInsertBack: Broken list!");
		}

		node =
		    (Util_ListNode *) malloc(sizeof(Util_ListNode));
		node->pData = data;
		node->pPrev = list->pTail;
		node->pNext = 0;
		list->pTail->pNext = node;
		list->pTail = node;
	} else {
		util_listInsertFront(list, data);
	}
}

void util_listDropFront(Util_List * list) {
	if (list->pHead->pNext == 0) {
		free((list->pHead));
		list->pHead = 0;
		list->pTail = 0;
	} else {
		list->pHead = list->pHead->pNext;
		free((list->pHead->pPrev));
		list->pHead->pPrev = 0;
	}
}

void util_deleteList(Util_List * list) {
	while (list->pHead != 0) {
		util_listDropFront(list);
	}

	free(list);
}

char util_str1[] =
    "       '   + - :0123456789: <=>  abcdefghijklmnopqrstuvwxyz      abcdefghijklmnopqrstuvwxyz     ";
char util_str2[] =
    " ueaaaaceeeiii     ooouu        aioun                                                           ";
char util_str3[] = "                                ";

void util_prepareStr(char *str) {
	uint16 i;
	int16 j;
	char buf[300];

	strcpy(buf, util_str1);
	strcat(buf, util_str2);
	strcat(buf, util_str3);

	for (i = 0; i < strlen(str); i++)
		str[i] = buf[str[i] - 32];

	while (str[0] == ' ')
		util_cutFromStr(str, 0, 1);

	while (strlen(str) > 0 && str[strlen(str) - 1] == ' ')
		util_cutFromStr(str, strlen(str) - 1, 1);

	i = util_strstr(" ", str);

	while (1) {
		if (i == 0)
			return;

		if (str[i] == ' ') {
			util_cutFromStr(str, i - 1, 1);
			continue;
		}

		j = util_strstr(" ", str + i);
		if (j != 0)
			i += j;
		else
			i = 0;
	}
}

void util_waitMouseRelease(char drawMouse) {
	int16 buttons;
	int16 mouseX;
	int16 mouseY;

	do {
		game_checkKeys(&mouseX, &mouseY, &buttons, drawMouse);
		if (drawMouse != 0)
			draw_animateCursor(2);
		util_delay(10);
	} while (buttons != 0);
}

void keyboard_release(void) {;}
} // End of namespace Gob
