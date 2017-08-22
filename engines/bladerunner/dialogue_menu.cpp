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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/dialogue_menu.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/font.h"
#include "bladerunner/mouse.h"
#include "bladerunner/shape.h"
#include "bladerunner/text_resource.h"

#include "common/debug.h"
#include "common/util.h"

#define LINE_HEIGHT  9
#define BORDER_SIZE 10

namespace BladeRunner {

DialogueMenu::DialogueMenu(BladeRunnerEngine *vm)
	: _vm(vm)
{
	reset();
	_textResource = new TextResource(_vm);
	_shapes.reserve(8);
	for (int i = 0; i != 8; ++i) {
		_shapes.push_back(Shape(_vm));
		bool r = _shapes[i].readFromContainer("DIALOG.SHP", i);
		assert(r);
		(void)r;
	}
}

DialogueMenu::~DialogueMenu() {
	delete _textResource;
}

bool DialogueMenu::loadText(const char *name) {
	bool r = _textResource->open(name);
	if (!r) {
		error("Failed to load dialogue menu text");
	}
	return r;
}

bool DialogueMenu::show() {
	int x, y;

	_vm->_mouse->getXY(&x, &y);

	return showAt(x, y);
}

bool DialogueMenu::showAt(int x, int y) {
	debug("DialogueMenu::showAt %d %d %d", _isVisible, x, y);
	if (_isVisible) {
		return false;
	}

	_isVisible = true;
	_selectedItemIndex = 0;
	_centerX = x;
	_centerY = y;
	calculatePosition(x, y);

	return true;
}

bool DialogueMenu::hide() {
	_waitingForInput = 0;
	if (!_isVisible) {
		return false;
	}

	_isVisible = false;
	return true;
}

bool DialogueMenu::clearList() {
	_selectedItemIndex = -1;
	_listSize = 0;
	return true;
}

bool DialogueMenu::addToList(int answer, int a3, int a4, int a5, int a6) {
	if (_listSize >= 10)
		return false;
	if (getAnswerIndex(answer) != -1)
		return false;

	const char *text = _textResource->getText(answer);
	if (!text || strlen(text) >= 50)
		return false;

	int index = _listSize++;
	strcpy(_items[index].text, text);
	_items[index].answerValue = answer;
	_items[index].field_36 = 0;
	_items[index].field_46 = a3;
	_items[index].field_3A = a4;
	_items[index].field_3E = a5;
	_items[index].field_42 = a6;

	// CHECK(madmoose): BLADE.EXE calls this needlessly
	// calculatePosition();

	return true;
}

bool DialogueMenu::addToListNeverRepeatOnceSelected(int answer, int a3, int a4, int a5) {
	for (int i = 0; i != _neverRepeatListSize; ++i) {
		if (answer == _neverRepeatValues[i] && _neverRepeatWasSelected[i]) {
			return true;
		}
	}

	_neverRepeatValues[_neverRepeatListSize] = answer;
	_neverRepeatWasSelected[_neverRepeatListSize] = false;
	++_neverRepeatListSize;
	return addToList(answer, 0, a3, a4, a5);
}

int DialogueMenu::queryInput() {
	if (!_isVisible || _listSize == 0)
		return -1;

	int answer = -1;
	if (_listSize == 1) {
		_selectedItemIndex = 0;
		answer = _items[0].answerValue;
	} else if (_listSize == 2) {
		if (_items[0].field_46) {
			_selectedItemIndex = 1;
			answer = _items[0].answerValue;
		} else if (_items[0].field_46) {
			_selectedItemIndex = 0;
			answer = _items[1].answerValue;
		}
	}

	if (answer == -1) {
		int agenda = 4; //_vm->_settings.getPlayerAgenda();
		if (agenda == 4) {
			_waitingForInput = true;
			do {
				// if (!_vm->_gameRunning)
				// 	break;

				while (!_vm->playerHasControl()) {
					_vm->playerGainsControl();
				}

				while (_vm->_mouse->isDisabled()) {
					_vm->_mouse->enable();
				}

				_vm->gameTick();
			} while (_waitingForInput);

		} else if (agenda == 3) {
			int tries = 0;
			bool searching = true;
			int i;
			do {
				i = _vm->_rnd.getRandomNumber(_listSize - 1);
				if (!_items[i].field_46) {
					searching = false;
				} else if (++tries > 1000) {
					searching = false;
					i = 0;
				}
			} while (searching);
			_selectedItemIndex = i;
		} else {
			error("unimplemented...");
		}
	}

	answer = _items[_selectedItemIndex].answerValue;
	for (int i = 0; i != _neverRepeatListSize; ++i) {
		if (answer == _neverRepeatValues[i]) {
			_neverRepeatWasSelected[i] = true;
			break;
		}
	}

	if (_selectedItemIndex >= 0) {
		debug("DM Query Input: %d %s", answer, _items[_selectedItemIndex].text);
	}

	return answer;
}

int DialogueMenu::listSize() {
	return _listSize;
}

bool DialogueMenu::isVisible() {
	return _isVisible;
}

bool DialogueMenu::isOpen() {
	return _isVisible || _waitingForInput;
}

void DialogueMenu::tick(int x, int y) {
	if (!_isVisible || _listSize == 0) {
		return;
	}

	int line = (y - (_screenY + BORDER_SIZE)) / LINE_HEIGHT;
	line = CLIP(line, 0, _listSize - 1);

	_selectedItemIndex = line;
}

void DialogueMenu::draw() {
	if (!_isVisible || _listSize == 0)
		return;

	for (int i = 0; i != _listSize; ++i) {
		if (i == _selectedItemIndex) {
			_items[i].field_36 = 31;
		} else {
			_items[i].field_36 = 16;
		}

		// TODO(madmoose): There's more logic here
	}

	const int x1 = _screenX;
	const int y1 = _screenY;
	const int x2 = _screenX + BORDER_SIZE + _maxItemWidth;
	const int y2 = _screenY + BORDER_SIZE + _listSize * LINE_HEIGHT;

	Graphics::Surface &s = _vm->_surfaceGame;

	darkenRect(s, x1 + 8, y1 + 8, x2 + 2, y2 + 2);

	_shapes[0].draw(s, x1, y1);
	_shapes[3].draw(s, x2, y1);
	_shapes[2].draw(s, x1, y2);
	_shapes[5].draw(s, x2, y2);

	int x = x1 + BORDER_SIZE;
	int y = y1 + BORDER_SIZE;
	for (int i = 0; i != _listSize; ++i) {
		_shapes[1].draw(s, x1, y);
		_shapes[4].draw(s, x2, y);
		uint16 color = ((_items[i].field_36 >> 1) << 10) | ((_items[i].field_36 >> 1) << 6) | _items[i].field_36;
		_vm->_mainFont->drawColor(_items[i].text, s, x, y, color);
		y += LINE_HEIGHT;
	}
	for (; x != x2; ++x) {
		_shapes[6].draw(s, x, y1);
		_shapes[7].draw(s, x, y2);
	}
}

int DialogueMenu::getAnswerIndex(int answer) {
	for (int i = 0; i != _listSize; ++i) {
		if (_items[i].answerValue == answer) {
			return i;
		}
	}

	return -1;
}

const char *DialogueMenu::getText(int id) {
	return _textResource->getText((uint32)id);
}

void DialogueMenu::calculatePosition(int unusedX, int unusedY) {
	_maxItemWidth = 0;
	for (int i = 0; i != _listSize; ++i) {
		_maxItemWidth = MAX(_maxItemWidth, _vm->_mainFont->getTextWidth(_items[i].text));
	}
	_maxItemWidth += 2;

	int w = BORDER_SIZE + _shapes[4].getWidth() + _maxItemWidth;
	int h = BORDER_SIZE + _shapes[7].getHeight() + LINE_HEIGHT * _listSize;

	_screenX = _centerX - w / 2;
	_screenY = _centerY - h / 2;

	_screenX = CLIP(_screenX, 0, 640 - w);
	_screenY = CLIP(_screenY, 0, 480 - h);

	debug("calculatePosition: %d %d %d %d %d", _screenX, _screenY, _centerX, _centerY, _maxItemWidth);
}

void DialogueMenu::mouseUp() {
	_waitingForInput = false;
}

bool DialogueMenu::waitingForInput() {
	return _waitingForInput;
}

void DialogueMenu::clear() {
	_isVisible = false;
	_waitingForInput = false;
	_selectedItemIndex = 0;
	_listSize = 0;
	for (int i = 0; i != 10; ++i) {
		_items[0].text[0] = '\0';
		_items[0].answerValue = -1;
		_items[0].field_36 = 0;
		_items[0].field_42 = -1;
		_items[0].field_3A = -1;
		_items[0].field_3E = -1;
		_items[0].field_46 = 0;
	}
	_neverRepeatListSize = 0;
	for (int i = 0; i != 100; ++i) {
		_neverRepeatValues[i]      = -1;
		_neverRepeatWasSelected[i] = false;
	}
	_centerX = 0;
	_centerY = 0;
}

void DialogueMenu::reset() {
	clear();
	_textResource = nullptr;
}

void DialogueMenu::darkenRect(Graphics::Surface &s, int x1, int y1, int x2, int y2) {
	for (int y = y1; y != y2; ++y) {
		for (int x = x1; x != x2; ++x) {
			// TODO(madmoose)
			uint16 *p = (uint16*)s.getBasePtr(x, y);
			*p = 0;
		}
	}
}

} // End of namespace BladeRunner
