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
#include "bladerunner/game_constants.h"
#include "bladerunner/mouse.h"
#include "bladerunner/savefile.h"
#include "bladerunner/settings.h"
#include "bladerunner/shape.h"
#include "bladerunner/text_resource.h"

#include "common/debug.h"
#include "common/rect.h"
#include "common/util.h"

namespace BladeRunner {

DialogueMenu::DialogueMenu(BladeRunnerEngine *vm) {
	_vm = vm;
	reset();
	_textResource = new TextResource(_vm);
	_shapes.reserve(8);
	for (int i = 0; i != 8; ++i) {
		_shapes.push_back(Shape(_vm));
		bool r = _shapes[i].open("DIALOG.SHP", i);
		assert(r);
		(void)r;
	}

	_screenX = 0;
	_screenY = 0;
	_maxItemWidth = 0;
	_fadeInItemIndex = 0;
}

DialogueMenu::~DialogueMenu() {
	delete _textResource;
}

bool DialogueMenu::loadText(const Common::String &name) {
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
	_waitingForInput = false;
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

bool DialogueMenu::addToList(int answer, bool done, int priorityPolite, int priorityNormal, int prioritySurly) {
	if (_listSize >= kMaxItems) {
		return false;
	}
	if (getAnswerIndex(answer) != -1) {
		return false;
	}

#if BLADERUNNER_ORIGINAL_BUGS
// Original uses incorrect spelling for entry id 1020: DRAGONFLY JEWERLY
	const Common::String &text = _textResource->getText(answer);
#else
// fix spelling or entry id 1020 to DRAGONFLY JEWELRY in English version
	const char *answerTextCP = _textResource->getText(answer);
	if (_vm->_language == Common::EN_ANY && answer == 1020 && strcmp(answerTextCP, "DRAGONFLY JEWERLY") == 0) {
		answerTextCP = "DRAGONFLY JEWELRY";
	}
	const Common::String &text = answerTextCP;
#endif // BLADERUNNER_ORIGINAL_BUGS
	if (text.empty() || text.size() >= 50) {
		return false;
	}

	int index = _listSize++;
	_items[index].text = text;
	_items[index].answerValue = answer;
	_items[index].colorIntensity = 0;
	_items[index].isDone = done;
	_items[index].priorityPolite = priorityPolite;
	_items[index].priorityNormal = priorityNormal;
	_items[index].prioritySurly  = prioritySurly;

	// CHECK(madmoose): BLADE.EXE calls this needlessly
	// calculatePosition();

	return true;
}

/**
* Aux function - used in cut content mode to re-use some NeverRepeatOnceSelected dialogue options for different characters
*/
bool DialogueMenu::clearNeverRepeatWasSelectedFlag(int answer) {
	int foundIndex = -1;
	for (int i = 0; i != _neverRepeatListSize; ++i) {
		if (answer == _neverRepeatValues[i]) {
			foundIndex = i;
			break;
		}
	}

	if (foundIndex >= 0 && _neverRepeatWasSelected[foundIndex]) {
		_neverRepeatWasSelected[foundIndex] = false;
		return true;
	}
	return false;
}

bool DialogueMenu::addToListNeverRepeatOnceSelected(int answer, int priorityPolite, int priorityNormal, int prioritySurly) {
	int foundIndex = -1;
	for (int i = 0; i != _neverRepeatListSize; ++i) {
		if (answer == _neverRepeatValues[i]) {
			foundIndex = i;
			break;
		}
	}

	if (foundIndex >= 0 && _neverRepeatWasSelected[foundIndex]) {
		return true;
	}

	if (foundIndex == -1) {
		_neverRepeatValues[_neverRepeatListSize] = answer;
		_neverRepeatWasSelected[_neverRepeatListSize] = false;
		++_neverRepeatListSize;

		assert(_neverRepeatListSize <= 100);
	}

	return addToList(answer, false, priorityPolite, priorityNormal, prioritySurly);
}

bool DialogueMenu::removeFromList(int answer) {
	int index = getAnswerIndex(answer);
	if (index < 0) {
		return false;
	}
	if (index < _listSize - 1) {
		for (int i = index; i < _listSize; ++i) {
			_items[index] = _items[index + 1];
		}
	}
	--_listSize;

	calculatePosition();
	return true;
}

int DialogueMenu::queryInput() {
	if (!_isVisible || _listSize == 0) {
		return -1;
	}

	int answer = -1;
	if (_listSize == 1) {
		_selectedItemIndex = 0;
		answer = _items[_selectedItemIndex].answerValue;
	} else if (_listSize == 2) {
#if BLADERUNNER_ORIGINAL_BUGS
		if (_items[0].isDone) {
			_selectedItemIndex = 1;
			answer = _items[_selectedItemIndex].answerValue;
		} else if (_items[1].isDone) {
			_selectedItemIndex = 0;
			answer = _items[_selectedItemIndex].answerValue;
		}
#else
		// In User Choice mode, avoid auto-select of last option
		// In this mode, player should still have agency to skip the last (non- "DONE")
		// question instead of automatically asking it because the other remaining option is "DONE"
		if (_vm->_settings->getPlayerAgenda() != kPlayerAgendaUserChoice) {
			if (_items[0].isDone) {
				_selectedItemIndex = 1;
				answer = _items[_selectedItemIndex].answerValue;
			} else if (_items[1].isDone) {
				_selectedItemIndex = 0;
				answer = _items[_selectedItemIndex].answerValue;
			}
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
	}

	if (answer == -1) {
		int agenda = _vm->_settings->getPlayerAgenda();
		if (agenda == kPlayerAgendaUserChoice) {
			_waitingForInput = true;
			do {
				while (!_vm->playerHasControl()) {
					_vm->playerGainsControl();
				}

				while (_vm->_mouse->isDisabled()) {
					_vm->_mouse->enable();
				}

				_vm->gameTick();
			} while (_vm->_gameIsRunning && _waitingForInput);
		} else if (agenda == kPlayerAgendaErratic) {
			int tries = 0;
			bool searching = true;
			int i;
			do {
				i = _vm->_rnd.getRandomNumber(_listSize - 1);
				if (!_items[i].isDone) {
					searching = false;
				} else if (++tries > 1000) {
					searching = false;
					i = 0;
				}
			} while (searching);
			_selectedItemIndex = i;
		} else {
			int priority = -1;
			for (int i = 0; i < _listSize; i++) {
				int priorityCompare = -1;
				if (agenda == kPlayerAgendaPolite) {
					priorityCompare = _items[i].priorityPolite;
				} else if (agenda == kPlayerAgendaNormal) {
					priorityCompare = _items[i].priorityNormal;
				} else if (agenda == kPlayerAgendaSurly) {
					priorityCompare = _items[i].prioritySurly;
				}
				if (priority < priorityCompare) {
					priority = priorityCompare;
					_selectedItemIndex = i;
				}
			}
		}
	}

	answer = _items[_selectedItemIndex].answerValue;
	for (int i = 0; i != _neverRepeatListSize; ++i) {
		if (answer == _neverRepeatValues[i]) {
			_neverRepeatWasSelected[i] = true;
			break;
		}
	}

	// debug("DM Query Input: %d %s", answer, _items[_selectedItemIndex].text.c_str());

	return answer;
}

int DialogueMenu::listSize() const {
	return _listSize;
}

bool DialogueMenu::isVisible() const {
	return _isVisible;
}

bool DialogueMenu::isOpen() const {
	return _isVisible || _waitingForInput;
}

void DialogueMenu::tick(int x, int y) {
	if (!_isVisible || _listSize == 0) {
		return;
	}

	int line = (y - (_screenY + kBorderSize)) / kLineHeight;
	line = CLIP(line, 0, _listSize - 1);

	_selectedItemIndex = line;
}

void DialogueMenu::draw(Graphics::Surface &s) {
	if (!_isVisible || _listSize == 0) {
		return;
	}

	int fadeInItemIndex = _fadeInItemIndex;
	if (fadeInItemIndex < listSize()) {
		++_fadeInItemIndex;
	}

	for (int i = 0; i != _listSize; ++i) {
		int targetColorIntensity = 0;
		if (i == _selectedItemIndex) {
			targetColorIntensity = 31;
		} else {
			targetColorIntensity = 16;
		}
		if (i > fadeInItemIndex) {
			targetColorIntensity = 0;
		}

		if (_items[i].colorIntensity < targetColorIntensity) {
			_items[i].colorIntensity += 4;
			if (_items[i].colorIntensity > targetColorIntensity) {
				_items[i].colorIntensity = targetColorIntensity;
			}
		} else if (_items[i].colorIntensity > targetColorIntensity) {
			_items[i].colorIntensity -= 2;
			if (_items[i].colorIntensity < targetColorIntensity) {
				_items[i].colorIntensity = targetColorIntensity;
			}
		}
	}

	const int x1 = _screenX;
	const int y1 = _screenY;
	const int x2 = _screenX + kBorderSize + _maxItemWidth;
	const int y2 = _screenY + kBorderSize + _listSize * kLineHeight;

	darkenRect(s, x1 + 8, y1 + 8, x2 + 2, y2 + 2);

	int x = x1 + kBorderSize;
	int y = y1 + kBorderSize;

	Common::Point mouse = _vm->getMousePos();
	if (mouse.x >= x && mouse.x < x2) {
		s.vLine(mouse.x, y1 + 8, y2 + 2, s.format.RGBToColor(64, 64, 64));
	}
	if (mouse.y >= y && mouse.y < y2) {
		s.hLine(x1 + 8, mouse.y, x2 + 2, s.format.RGBToColor(64, 64, 64));
	}

	_shapes[0].draw(s, x1, y1);
	_shapes[3].draw(s, x2, y1);
	_shapes[2].draw(s, x1, y2);
	_shapes[5].draw(s, x2, y2);

	for (int i = 0; i != _listSize; ++i) {
		_shapes[1].draw(s, x1, y);
		_shapes[4].draw(s, x2, y);
		uint32 color = s.format.RGBToColor((_items[i].colorIntensity / 2) * (256 / 32), (_items[i].colorIntensity / 2) * (256 / 32), _items[i].colorIntensity * (256 / 32));
		_vm->_mainFont->drawString(&s, _items[i].text, x, y, s.w, color);
		y += kLineHeight;
	}
	for (; x != x2; ++x) {
		_shapes[6].draw(s, x, y1);
		_shapes[7].draw(s, x, y2);
	}
}

int DialogueMenu::getAnswerIndex(int answer) const {
	for (int i = 0; i != _listSize; ++i) {
		if (_items[i].answerValue == answer) {
			return i;
		}
	}

	return -1;
}

const char *DialogueMenu::getText(int id) const {
	return _textResource->getText((uint32)id);
}

void DialogueMenu::calculatePosition(int unusedX, int unusedY) {
	_maxItemWidth = 0;
	for (int i = 0; i != _listSize; ++i) {
		_maxItemWidth = MAX(_maxItemWidth, _vm->_mainFont->getStringWidth(_items[i].text));
	}
	_maxItemWidth += 2;

	int w = kBorderSize + _shapes[4].getWidth() + _maxItemWidth;
	int h = kBorderSize + _shapes[7].getHeight() + kLineHeight * _listSize;

	_screenX = _centerX - w / 2;
	_screenY = _centerY - h / 2;

	_screenX = CLIP(_screenX, 0, 640 - w);
	_screenY = CLIP(_screenY, 0, 480 - h);

	_fadeInItemIndex = 0;
}

void DialogueMenu::mouseUp() {
	_waitingForInput = false;
}

bool DialogueMenu::waitingForInput() const {
	return _waitingForInput;
}

void DialogueMenu::save(SaveFileWriteStream &f) {
	f.writeBool(_isVisible);
	f.writeBool(_waitingForInput);
	f.writeInt(_selectedItemIndex);
	f.writeInt(_listSize);

	f.writeInt(_neverRepeatListSize);
	for (int i = 0; i < 100; ++i) {
		f.writeInt(_neverRepeatValues[i]);
	}
	for (int i = 0; i < 100; ++i) {
		f.writeBool(_neverRepeatWasSelected[i]);
	}
	for (int i = 0; i < 10; ++i) {
		f.writeStringSz(_items[i].text, 50);
		f.writeInt(_items[i].answerValue);
		f.writeInt(_items[i].colorIntensity);
		f.writeInt(_items[i].priorityPolite);
		f.writeInt(_items[i].priorityNormal);
		f.writeInt(_items[i].prioritySurly);
		f.writeInt(_items[i].isDone);
	}
}

void DialogueMenu::load(SaveFileReadStream &f) {
	_isVisible = f.readBool();
	_waitingForInput = f.readBool();
	_selectedItemIndex = f.readInt();
	_listSize = f.readInt();

#if 0
	/* fix for duplicated non-repeated entries in the save game */
	f.readInt();
	_neverRepeatListSize = 0;
	int answer[100];
	bool selected[100];
	for (int i = 0; i < 100; ++i) {
		_neverRepeatValues[i] = -1;
		answer[i] = f.readInt();
	}
	for (int i = 0; i < 100; ++i) {
		_neverRepeatWasSelected[i] = false;
		selected[i] = f.readBool();
	}
	for (int i = 0; i < 100; ++i) {
		int found = false;
		bool value = false;

		for (int j = 0; j < 100; ++j) {
			if (_neverRepeatValues[j] == answer[i]) {
				found = true;
			}
			if (answer[j] == answer[i]) {
				value |= selected[j];
			}
		}

		if (!found) {
			_neverRepeatValues[_neverRepeatListSize] = answer[i];
			_neverRepeatWasSelected[_neverRepeatListSize] = value;
			++_neverRepeatListSize;
		}
	}
#else
	_neverRepeatListSize = f.readInt();
	for (int i = 0; i < 100; ++i) {
		_neverRepeatValues[i] = f.readInt();
	}
	for (int i = 0; i < 100; ++i) {
		_neverRepeatWasSelected[i] = f.readBool();
	}
#endif

	for (int i = 0; i < 10; ++i) {
		_items[i].text = f.readStringSz(50);
		_items[i].answerValue = f.readInt();
		_items[i].colorIntensity = f.readInt();
		_items[i].priorityPolite = f.readInt();
		_items[i].priorityNormal = f.readInt();
		_items[i].prioritySurly = f.readInt();
		_items[i].isDone = f.readInt();
	}
}

void DialogueMenu::clear() {
	_isVisible = false;
	_waitingForInput = false;
	_selectedItemIndex = 0;
	_listSize = 0;
	for (int i = 0; i != kMaxItems; ++i) {
		_items[i].text.clear();
		_items[i].answerValue = -1;
		_items[i].isDone = 0;
		_items[i].priorityPolite = -1;
		_items[i].priorityNormal = -1;
		_items[i].prioritySurly = -1;
		_items[i].colorIntensity = 0;
	}
	_neverRepeatListSize = 0;
	for (int i = 0; i != kMaxRepeatHistory; ++i) {
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
	x1 = MAX(x1, 0);
	y1 = MAX(y1, 0);
	x2 = MIN(x2, 640);
	y2 = MIN(y2, 480);

	if (x1 < x2 && y1 < y2) {
		for (int y = y1; y != y2; ++y) {
			for (int x = x1; x != x2; ++x) {
				void *p = s.getBasePtr(CLIP(x, 0, s.w - 1), CLIP(y, 0, s.h - 1));
				uint8 r, g, b;
				s.format.colorToRGB(READ_UINT32(p), r, g, b);
				r /= 4;
				g /= 4;
				b /= 4;
				drawPixel(s, p, s.format.RGBToColor(r, g, b));
			}
		}
	}
}

} // End of namespace BladeRunner
