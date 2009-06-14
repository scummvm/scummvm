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


#include "gob/gob.h"
#include "gob/util.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/video.h"
#include "gob/sound/sound.h"

#include "common/events.h"

namespace Gob {

Util::Util(GobEngine *vm) : _vm(vm) {
	_mouseButtons = 0;
	_keyBufferHead = 0;
	_keyBufferTail = 0;
	_fastMode = 0;
	_frameRate = 12;
	_frameWaitTime = 0;
	_startFrameTime = 0;
	_frameWaitLag = 0;
}

uint32 Util::getTimeKey(void) {
	return g_system->getMillis() * _vm->_global->_speedFactor;
}

int16 Util::getRandom(int16 max) {
	if (max == 0)
		return 0;

	return _vm->_rnd.getRandomNumber(max - 1);
}

void Util::beep(int16 freq) {
	if (_vm->_global->_soundFlags == 0)
		return;

	_vm->_sound->speakerOn(freq, 50);
}

void Util::notifyPaused(uint32 duration) {
	_startFrameTime += duration;
}

void Util::delay(uint16 msecs) {
	g_system->delayMillis(msecs / _vm->_global->_speedFactor);
}

void Util::longDelay(uint16 msecs) {
	uint32 time = g_system->getMillis() * _vm->_global->_speedFactor + msecs;
	do {
		_vm->_video->waitRetrace();
		processInput();
		delay(15);
	} while (!_vm->shouldQuit() &&
	         ((g_system->getMillis() * _vm->_global->_speedFactor) < time));
}

void Util::initInput(void) {
	_mouseButtons = 0;
	_keyBufferHead = _keyBufferTail = 0;
}

void Util::processInput(bool scroll) {
	Common::Event event;
	Common::EventManager *eventMan = g_system->getEventManager();
	int16 x = 0, y = 0;
	bool hasMove = false;

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			hasMove = true;
			x = event.mouse.x;
			y = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseButtons |= 1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons |= 2;
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseButtons &= ~1;
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseButtons &= ~2;
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.flags == Common::KBD_CTRL) {
				if (event.kbd.keycode == Common::KEYCODE_f)
					_fastMode ^= 1;
				else if (event.kbd.keycode == Common::KEYCODE_g)
					_fastMode ^= 2;
				else if (event.kbd.keycode == Common::KEYCODE_p)
					_vm->pauseGame();
				break;
			}
			addKeyToBuffer(event.kbd);
			break;
		case Common::EVENT_KEYUP:
			break;
		default:
			break;
		}
	}

	_vm->_global->_speedFactor = MIN(_fastMode + 1, 3);
	if (hasMove && scroll) {
		x = CLIP(x, _vm->_global->_mouseMinX, _vm->_global->_mouseMaxX);
		y = CLIP(y, _vm->_global->_mouseMinY, _vm->_global->_mouseMaxY);

		x -= _vm->_video->_screenDeltaX;
		y -= _vm->_video->_screenDeltaY;

		_vm->_util->setMousePos(x, y);
		_vm->_game->evaluateScroll(x, y);
	}
}

void Util::clearKeyBuf(void) {
	processInput();
	_keyBufferHead = _keyBufferTail = 0;
}

bool Util::keyBufferEmpty() {
	return (_keyBufferHead == _keyBufferTail);
}

void Util::addKeyToBuffer(const Common::KeyState &key) {
	if ((_keyBufferHead + 1) % KEYBUFSIZE == _keyBufferTail) {
		warning("key buffer overflow");
		return;
	}

	_keyBuffer[_keyBufferHead] = key;
	_keyBufferHead = (_keyBufferHead + 1) % KEYBUFSIZE;
}

bool Util::getKeyFromBuffer(Common::KeyState &key) {
	if (_keyBufferHead == _keyBufferTail) return false;

	key = _keyBuffer[_keyBufferTail];
	_keyBufferTail = (_keyBufferTail + 1) % KEYBUFSIZE;

	return true;
}

int16 Util::translateKey(const Common::KeyState &key) {
	static struct keyS {
		int16 from;
		int16 to;
	} keys[] = {
		{Common::KEYCODE_INVALID,   0x0000},
		{Common::KEYCODE_BACKSPACE, 0x0E08},
		{Common::KEYCODE_SPACE,     0x3920},
		{Common::KEYCODE_RETURN,    0x1C0D},
		{Common::KEYCODE_ESCAPE,    0x011B},
		{Common::KEYCODE_DELETE,    0x5300},
		{Common::KEYCODE_UP,        0x4800},
		{Common::KEYCODE_DOWN,      0x5000},
		{Common::KEYCODE_RIGHT,     0x4D00},
		{Common::KEYCODE_LEFT,      0x4B00},
		{Common::KEYCODE_F1,        0x3B00},
		{Common::KEYCODE_F2,        0x3C00},
		{Common::KEYCODE_F3,        0x3D00},
		{Common::KEYCODE_F4,        0x3E00},
		{Common::KEYCODE_F5,        0x011B},
		{Common::KEYCODE_F6,        0x4000},
		{Common::KEYCODE_F7,        0x4100},
		{Common::KEYCODE_F8,        0x4200},
		{Common::KEYCODE_F9,        0x4300},
		{Common::KEYCODE_F10,       0x4400}
	};

	for (int i = 0; i < ARRAYSIZE(keys); i++)
		if (key.keycode == keys[i].from)
			return keys[i].to;

	if ((key.keycode >= Common::KEYCODE_SPACE) &&
	    (key.keycode <= Common::KEYCODE_DELETE)) {

		// Used as a user input in Gobliins 2 notepad, in the save dialog, ...
		return key.ascii;
	}

	return 0;
}

int16 Util::getKey(void) {
	Common::KeyState key;

	while (!getKeyFromBuffer(key)) {
		processInput();

		if (keyBufferEmpty())
			g_system->delayMillis(10 / _vm->_global->_speedFactor);
	}
	return translateKey(key);
}

int16 Util::checkKey(void) {
	Common::KeyState key;

	getKeyFromBuffer(key);

	return translateKey(key);
}

bool Util::checkKey(int16 &key) {
	Common::KeyState keyS;

	if (!getKeyFromBuffer(keyS))
		return false;

	key = translateKey(keyS);

	return true;
}

void Util::getMouseState(int16 *pX, int16 *pY, int16 *pButtons) {
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	*pX = mouse.x + _vm->_video->_scrollOffsetX - _vm->_video->_screenDeltaX;
	*pY = mouse.y + _vm->_video->_scrollOffsetY - _vm->_video->_screenDeltaY;

	if (pButtons != 0)
		*pButtons = _mouseButtons;
}

void Util::setMousePos(int16 x, int16 y) {
	x = CLIP<int>(x + _vm->_video->_screenDeltaX, 0, _vm->_width - 1);
	y = CLIP<int>(y + _vm->_video->_screenDeltaY, 0, _vm->_height - 1);
	g_system->warpMouse(x, y);
}

void Util::waitMouseUp(void) {
	do {
		processInput();
		if (_mouseButtons != 0)
			delay(10);
	} while (_mouseButtons != 0);
}

void Util::waitMouseDown(void) {
	int16 x;
	int16 y;
	int16 buttons;

	do {
		processInput();
		getMouseState(&x, &y, &buttons);
		if (buttons == 0)
			delay(10);
	} while (buttons == 0);
}

void Util::waitMouseRelease(char drawMouse) {
	int16 buttons;
	int16 mouseX;
	int16 mouseY;

	_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, drawMouse);
	while (buttons != 0) {
		if (drawMouse != 0)
			_vm->_draw->animateCursor(2);
		delay(10);
		_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, drawMouse);
	}
}

void Util::forceMouseUp(bool onlyWhenSynced) {
	if (onlyWhenSynced && (_vm->_game->_mouseButtons != _mouseButtons))
		return;

	_vm->_game->_mouseButtons = 0;
	_mouseButtons = 0;
}

void Util::clearPalette(void) {
	int16 i;
	byte colors[1024];

	_vm->validateVideoMode(_vm->_global->_videoMode);

	if (_vm->_global->_setAllPalette) {
		memset(colors, 0, 1024);
		g_system->setPalette(colors, 0, 256);
		return;
	}

	for (i = 0; i < 16; i++)
		_vm->_video->setPalElem(i, 0, 0, 0, 0, _vm->_global->_videoMode);
}

int16 Util::getFrameRate() {
	return _frameRate;
}

void Util::setFrameRate(int16 rate) {
	if (rate == 0)
		rate = 1;

	_frameRate = rate;
	_frameWaitTime = 1000 / rate;
	_startFrameTime = getTimeKey();
	_frameWaitLag = 0;
}

void Util::notifyNewAnim() {
	_startFrameTime = getTimeKey();
	_frameWaitLag = 0;
}

void Util::waitEndFrame() {
	int32 time;

	_vm->_video->waitRetrace();

	time = getTimeKey() - _startFrameTime;
	if ((time > 1000) || (time < 0)) {
		_startFrameTime = getTimeKey();
		return;
	}

	int32 waitTime = _frameWaitTime - _frameWaitLag;
	int32 toWait = waitTime - time;

	if (toWait > 0)
		delay(toWait);

	int32 now = getTimeKey();

	_frameWaitLag = (now - _startFrameTime) - waitTime;

	_startFrameTime = now;
}

void Util::setScrollOffset(int16 x, int16 y) {
	processInput();

	if (x >= 0)
		_vm->_video->_scrollOffsetX = x;
	else
		_vm->_video->_scrollOffsetX = _vm->_draw->_scrollOffsetX;

	if (y >= 0)
		_vm->_video->_scrollOffsetY = y;
	else
		_vm->_video->_scrollOffsetY = _vm->_draw->_scrollOffsetY;

	_vm->_video->waitRetrace();
}

Video::FontDesc *Util::loadFont(const char *path) {
	Video::FontDesc *fontDesc = new Video::FontDesc;
	byte *data;

	if (!fontDesc)
		return 0;

	data = _vm->_dataIO->getData(path);
	if (!data) {
		delete fontDesc;
		return 0;
	}

	fontDesc->dataPtr = data + 4;
	fontDesc->itemWidth = data[0] & 0x7F;
	fontDesc->itemHeight = data[1];
	fontDesc->startItem = data[2];
	fontDesc->endItem = data[3];

	fontDesc->itemSize =
	    ((fontDesc->itemWidth - 1) / 8 + 1) * fontDesc->itemHeight;
	fontDesc->bitWidth = fontDesc->itemWidth;

	if (data[0] & 0x80)
		fontDesc->extraData = data + 4 + fontDesc->itemSize *
			(fontDesc->endItem - fontDesc->startItem + 1);
	else
		fontDesc->extraData = 0;

	return fontDesc;
}

void Util::insertStr(const char *str1, char *str2, int16 pos) {
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	int from = MIN((int) pos, len2);

	for (int i = len2; i >= from; i--)
		str2[len1 + i] = str2[i];
	for (int i = 0; i < len1; i++)
		str2[i + from] = str1[i];
}

void Util::cutFromStr(char *str, int16 from, int16 cutlen) {
	int len = strlen(str);

	if (from >= len)
		return;
	if ((from + cutlen) > len) {
		str[from] = 0;
		return;
	}

	int i = from;
	do {
		str[i] = str[i + cutlen];
		i++;
	} while (str[i] != 0);
}

void Util::replaceChar(char *str, char c1, char c2) {
	while ((str = strchr(str, c1)))
		*str = c2;
}

static const char trStr1[] =
	"       '   + - :0123456789: <=>  abcdefghijklmnopqrstuvwxyz      "
	"abcdefghijklmnopqrstuvwxyz     ";
static const char trStr2[] =
	" ueaaaaceeeiii     ooouu        aioun"
	"                                                           ";
static const char trStr3[] = "                                ";

void Util::prepareStr(char *str) {
	char *start, *end;
	char buf[300];

	strcpy(buf, trStr1);
	strcat(buf, trStr2);
	strcat(buf, trStr3);

	for (size_t i = 0; i < strlen(str); i++)
		str[i] = buf[str[i] - 32];

	while (str[0] == ' ')
		cutFromStr(str, 0, 1);

	while ((strlen(str) > 0) && (str[strlen(str) - 1] == ' '))
		cutFromStr(str, strlen(str) - 1, 1);

	start = strchr(str, ' ');

	while (start) {
		if (start[1] == ' ') {
			cutFromStr(str, start - str, 1);
			continue;
		}

		end = strchr(start + 1, ' ');
		start = end ? end + 1 : 0;
	}
}

void Util::listInsertFront(List *list, void *data) {
	ListNode *node;

	node = new ListNode;
	if (list->pHead) {
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

void Util::listInsertBack(List *list, void *data) {
	ListNode *node;

	if (list->pHead != 0) {
		if (list->pTail == 0) {
			list->pTail = list->pHead;
			warning("Util::listInsertBack(): Broken list");
		}

		node = new ListNode;
		node->pData = data;
		node->pPrev = list->pTail;
		node->pNext = 0;
		list->pTail->pNext = node;
		list->pTail = node;
	} else
		listInsertFront(list, data);
}

void Util::listDropFront(List *list) {
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

void Util::deleteList(List *list) {
	while (list->pHead)
		listDropFront(list);

	delete list;
}

/* NOT IMPLEMENTED */
void Util::checkJoystick() {
	_vm->_global->_useJoystick = 0;
}

} // End of namespace Gob
