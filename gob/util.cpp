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

Util::Util(GobEngine *vm) : _vm(vm) {
	_mouseX = 0;
	_mouseY = 0;
	_mouseButtons = 0;
	for (int i = 0; i < KEYBUFSIZE; i++)
		_keyBuffer[i] = 0;
	_keyBufferHead = 0;
	_keyBufferTail = 0;
}

void Util::addKeyToBuffer(int16 key) {
	if ((_keyBufferHead + 1) % KEYBUFSIZE == _keyBufferTail) {
		warning("key buffer overflow!");
		return;
	}

	_keyBuffer[_keyBufferHead] = key;
	_keyBufferHead = (_keyBufferHead + 1) % KEYBUFSIZE;
}

bool Util::keyBufferEmpty() {
	return (_keyBufferHead == _keyBufferTail);
}

bool Util::getKeyFromBuffer(int16& key) {
	if (_keyBufferHead == _keyBufferTail) return false;

	key = _keyBuffer[_keyBufferTail];
	_keyBufferTail = (_keyBufferTail + 1) % KEYBUFSIZE;

	return true;
}


void Util::initInput(void) {
	_mouseX = _mouseY = _mouseButtons = 0;
	_keyBufferHead = _keyBufferTail = 0;
}

void Util::waitKey(void) {
	// FIXME: wrong function name? This functions clears the keyboard buffer.
	processInput();
	_keyBufferHead = _keyBufferTail = 0;
}

int16 Util::translateKey(int16 key) {
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

int16 Util::getKey(void) {
	int16 key;

	while (!getKeyFromBuffer(key)) {
		processInput();

		if (keyBufferEmpty())
			g_system->delayMillis(10);
	}
	return translateKey(key);
}

int16 Util::checkKey(void) {
	int16 key;

	if (!getKeyFromBuffer(key))
		key = 0;

	return translateKey(key);
}

int16 Util::getRandom(int16 max) {
	return _vm->_rnd.getRandomNumber(max - 1);
}

void Util::processInput() {
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

void Util::getMouseState(int16 *pX, int16 *pY, int16 *pButtons) {
	*pX = _mouseX;
	*pY = _mouseY;

	if (pButtons != 0)
		*pButtons = _mouseButtons;
}

void Util::setMousePos(int16 x, int16 y) {
	g_system->warpMouse(x, y);
}

void Util::longDelay(uint16 msecs) {
	uint32 time = g_system->getMillis() + msecs;
	do {
		_vm->_video->waitRetrace(_vm->_global->_videoMode);
		processInput();
		delay(25);
	} while (g_system->getMillis() < time);
}

void Util::delay(uint16 msecs) {
	g_system->delayMillis(msecs);
}

void Util::beep(int16 freq) {
	if (_vm->_global->_soundFlags == 0)
		return;

	_vm->_snd->speakerOn(freq, 50);
}

uint32 Util::getTimeKey(void) {
	return g_system->getMillis();
}

void Util::waitMouseUp(void) {
	int16 x;
	int16 y;
	int16 buttons;

	do {
		processInput();
		getMouseState(&x, &y, &buttons);
		if (buttons != 0) delay(10);
	} while (buttons != 0);
}

void Util::waitMouseDown(void) {
	int16 x;
	int16 y;
	int16 buttons;

	do {
		processInput();
		getMouseState(&x, &y, &buttons);
		if (buttons == 0) delay(10);
	} while (buttons == 0);
}

/* NOT IMPLEMENTED */
int16 Util::calcDelayTime() {
	return 0;
}

/* NOT IMPLEMENTED */
void Util::checkJoystick() {
	_vm->_global->_useJoystick = 0;
}

void Util::setFrameRate(int16 rate) {
	if (rate == 0)
		rate = 1;

	_vm->_global->_frameWaitTime = 1000 / rate;
	_vm->_global->_startFrameTime = getTimeKey();
}

void Util::waitEndFrame() {
	int32 time;

	_vm->_video->waitRetrace(_vm->_global->_videoMode);

	time = getTimeKey() - _vm->_global->_startFrameTime;
	if (time > 1000 || time < 0) {
		_vm->_global->_startFrameTime = getTimeKey();
		return;
	}
	if (_vm->_global->_frameWaitTime - time > 0) {
		delay(_vm->_global->_frameWaitTime - time);
	}
	_vm->_global->_startFrameTime = getTimeKey();
}

int16 joy_getState() {
	return 0;
}

int16 joy_calibrate() {
	return 0;
}

Video::FontDesc *Util::loadFont(const char *path) {
	Video::FontDesc *fontDesc = new Video::FontDesc;
	char *data;

	if (fontDesc == 0)
		return 0;

	data = _vm->_dataio->getData(path);
	if (data == 0) {
		delete fontDesc;
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

void Util::freeFont(Video::FontDesc * fontDesc) {
	delete[] (fontDesc->dataPtr - 4);
	delete fontDesc;
}

void Util::clearPalette(void) {
	int16 i;
	byte colors[768];

	if (_vm->_global->_videoMode != 0x13)
		error("clearPalette: Video mode 0x%x is not supported!",
		    _vm->_global->_videoMode);

	if (_vm->_global->_setAllPalette) {
		for (i = 0; i < 768; i++)
			colors[i] = 0;
		g_system->setPalette(colors, 0, 256);

		return;
	}

	for (i = 0; i < 16; i++)
		_vm->_video->setPalElem(i, 0, 0, 0, 0, _vm->_global->_videoMode);
}

void Util::insertStr(const char *str1, char *str2, int16 pos) {
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

void Util::cutFromStr(char *str, int16 from, int16 cutlen) {
	int16 len;
	int16 i;

	//log_write("cutFromStr: str = %s, ", str);
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

int16 Util::strstr(const char *str1, char *str2) {
	char c;
	uint16 len1;
	uint16 i;

	//log_write("strstr: str1 = %s, str2 = %s\n", str1, str2);

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

void Util::listInsertFront(List * list, void *data) {
	ListNode *node;

	node = new ListNode;
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

void Util::listInsertBack(List * list, void *data) {
	ListNode *node;

	if (list->pHead != 0) {
		if (list->pTail == 0) {
			list->pTail = list->pHead;
			warning("listInsertBack: Broken list!");
		}

		node = new ListNode;
		node->pData = data;
		node->pPrev = list->pTail;
		node->pNext = 0;
		list->pTail->pNext = node;
		list->pTail = node;
	} else {
		listInsertFront(list, data);
	}
}

void Util::listDropFront(List * list) {
	if (list->pHead->pNext == 0) {
		delete list->pHead;
		list->pHead = 0;
		list->pTail = 0;
	} else {
		list->pHead = list->pHead->pNext;
		delete list->pHead->pPrev;
		list->pHead->pPrev = 0;
	}
}

void Util::deleteList(List * list) {
	while (list->pHead != 0) {
		listDropFront(list);
	}

	delete list;
}

const char Util::trStr1[] =
    "       '   + - :0123456789: <=>  abcdefghijklmnopqrstuvwxyz      abcdefghijklmnopqrstuvwxyz     ";
const char Util::trStr2[] =
    " ueaaaaceeeiii     ooouu        aioun                                                           ";
const char Util::trStr3[] = "                                ";

void Util::prepareStr(char *str) {
	uint16 i;
	int16 j;
	char buf[300];

	strcpy(buf, trStr1);
	strcat(buf, trStr2);
	strcat(buf, trStr3);

	for (i = 0; i < strlen(str); i++)
		str[i] = buf[str[i] - 32];

	while (str[0] == ' ')
		cutFromStr(str, 0, 1);

	while (strlen(str) > 0 && str[strlen(str) - 1] == ' ')
		cutFromStr(str, strlen(str) - 1, 1);

	i = strstr(" ", str);

	while (1) {
		if (i == 0)
			return;

		if (str[i] == ' ') {
			cutFromStr(str, i - 1, 1);
			continue;
		}

		j = strstr(" ", str + i);
		if (j != 0)
			i += j;
		else
			i = 0;
	}
}

void Util::waitMouseRelease(char drawMouse) {
	int16 buttons;
	int16 mouseX;
	int16 mouseY;

	do {
		_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, drawMouse);
		if (drawMouse != 0)
			_vm->_draw->animateCursor(2);
		delay(10);
	} while (buttons != 0);
}

void Util::keyboard_release(void) {;}
} // End of namespace Gob
