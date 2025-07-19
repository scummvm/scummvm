/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "mm/xeen/dialogs/dialogs.h"
#include "mm/xeen/events.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/screen.h"
#include "mm/xeen/xeen.h"

namespace MM {
namespace Xeen {

void ButtonContainer::saveButtons() {
	_savedButtons.push(_buttons);
	clearButtons();
}

/*
 * Clears the current list of defined buttons
 */
void ButtonContainer::clearButtons() {
	_buttons.clear();
}

void ButtonContainer::restoreButtons() {
	_buttons = _savedButtons.pop();
}

void ButtonContainer::addButton(const Common::Rect &bounds, int val,
		SpriteResource *sprites, uint8 ttsIndex) {
	_buttons.push_back(UIButton(bounds, val, _buttons.size() * 2, sprites, sprites != nullptr, ttsIndex, ttsIndex < UINT8_MAX));
}

void ButtonContainer::addButton(const Common::Rect &bounds, int val,
		int frameNum, SpriteResource *sprites, uint8 ttsIndex) {
	_buttons.push_back(UIButton(bounds, val, frameNum, sprites, sprites != nullptr, ttsIndex, ttsIndex < UINT8_MAX));
}

void ButtonContainer::addPartyButtons(XeenEngine *vm) {
	for (uint idx = 0; idx < MAX_ACTIVE_PARTY; ++idx) {
		addButton(Common::Rect(Res.CHAR_FACES_X[idx], 150, Res.CHAR_FACES_X[idx] + 32, 182),
			Common::KEYCODE_F1 + idx);
	}
}

bool ButtonContainer::checkEvents(XeenEngine *vm) {
	EventsManager &events = *vm->_events;
	Party &party = *vm->_party;
	Windows &windows = *_vm->_windows;
	PendingEvent event;
	_buttonValue = 0;

#ifdef USE_TTS
	bool buttonKeyboardPressed = false;
	checkHoverOverButton();
#endif

	if (events.getEvent(event)) {
		if (event._leftButton) {
			Common::Point pt = events._mousePos;

			// Check for party member glyphs being clicked
			Common::Rect r(0, 0, 32, 32);
			for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
				r.moveTo(Res.CHAR_FACES_X[idx], 150);
				if (r.contains(pt)) {
					_buttonValue = Common::KEYCODE_F1 + idx;
					break;
				}
			}

			// Check whether any button is selected
			for (uint i = 0; i < _buttons.size(); ++i) {
				if (_buttons[i]._bounds.contains(pt) && _buttons[i]._value) {
					events.debounceMouse();

					_buttonValue = _buttons[i]._value;
					break;
				}
			}

			if (!_buttonValue && _waitBounds.contains(pt)) {
				_buttonValue = Common::KEYCODE_SPACE;
				return true;
			}

		} else if (event.isKeyboard()) {
			const Common::KeyCode &keycode = event._keyState.keycode;

			if (keycode == Common::KEYCODE_KP8)
				_buttonValue = Common::KEYCODE_UP;
			else if (keycode == Common::KEYCODE_KP2)
				_buttonValue = Common::KEYCODE_DOWN;
			else if (keycode == Common::KEYCODE_KP_ENTER)
				_buttonValue = Common::KEYCODE_RETURN;
			else if (keycode != Common::KEYCODE_LCTRL && keycode != Common::KEYCODE_RCTRL
					&& keycode != Common::KEYCODE_LALT && keycode != Common::KEYCODE_RALT)
				_buttonValue = keycode;

			if (_buttonValue) {
				_buttonValue |= (event._keyState.flags & ~Common::KBD_STICKY) << 16;
#ifdef USE_TTS
				buttonKeyboardPressed = true;
#endif
			}
		}
	}

	if (_buttonValue) {
		// Check for a button matching the selected _buttonValue
		Window &win = windows[39];
		for (uint btnIndex = 0; btnIndex < _buttons.size(); ++btnIndex) {
			UIButton &btn = _buttons[btnIndex];
			if (btn._value == _buttonValue) {
#ifdef USE_TTS
				// Only voice the button's text if it was activated by keyboard press, to avoid voicing it
				// every time the player clicks it
				if (buttonKeyboardPressed && btn._canVoice && btn._ttsIndex < _buttonTexts.size()) {
					_vm->sayText(_buttonTexts[btn._ttsIndex], Common::TextToSpeechManager::INTERRUPT);
				}
#endif

				if (btn._draw) {
					// Found the correct button
					// Draw button depressed
					btn._sprites->draw(0, btn._selectedFrame, Common::Point(btn._bounds.left, btn._bounds.top));
					win.setBounds(btn._bounds);
					win.update();

					// Slight delay
					events.updateGameCounter();
					events.wait(2);

					// Redraw button in it's original non-depressed form
					btn._sprites->draw(0, btn._frameNum, Common::Point(btn._bounds.left, btn._bounds.top));
					win.setBounds(btn._bounds);
					win.update();
					break;
				}
			}
		}

		return true;
	}

	return false;
}

#ifdef USE_TTS

void ButtonContainer::checkHoverOverButton() {
	if (g_vm->_mouseMoved) {
		bool hoveringOverButton = false;

		Common::Point pt = g_vm->_events->_mousePos;

		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._canVoice && _buttons[i]._ttsIndex < _buttonTexts.size() && _buttons[i]._bounds.contains(pt) && _buttons[i]._value) {
				hoveringOverButton = true;

				if (_previousButton != (int)i) {
					_vm->sayText(_buttonTexts[_buttons[i]._ttsIndex], Common::TextToSpeechManager::INTERRUPT);
					_previousButton = (int)i;
				}

				break;
			}
		}

		if (!hoveringOverButton) {
			_previousButton = -1;
		}

		g_vm->_mouseMoved = false;
	}
}

#endif

void ButtonContainer::drawButtons(XSurface *surface) {
	for (uint btnIndex = 0; btnIndex < _buttons.size(); ++btnIndex) {
		UIButton &btn = _buttons[btnIndex];
		if (btn._draw) {
			assert(btn._sprites);
			btn._sprites->draw(*surface, btn._frameNum,
				Common::Point(btn._bounds.left, btn._bounds.top));
		}
	}
}

bool ButtonContainer::doScroll(bool rollUp, bool fadeIn) {
	if (_vm->_files->_ccNum) {
		return Cutscenes::doScroll(rollUp, fadeIn);
	} else {
		saveButtons();
		clearButtons();
		bool result = Cutscenes::doScroll(rollUp, fadeIn);
		restoreButtons();
		return result;
	}
}

void ButtonContainer::loadStrings(const Common::Path &name) {
	File f(name);
	_textStrings.clear();
	while (f.pos() < f.size())
		_textStrings.push_back(f.readString());
	f.close();
}

void ButtonContainer::loadStrings(const Common::Path &name, int ccMode) {
	File f(name, ccMode);
	_textStrings.clear();
	while (f.pos() < f.size())
		_textStrings.push_back(f.readString());
	f.close();
}

void ButtonContainer::setWaitBounds() {
	_waitBounds = Common::Rect(8, 8, 224, 140);
}

#ifdef USE_TTS

Common::String ButtonContainer::getNextTextSection(const Common::String &text, uint &index, uint count, const char *separator) const {
	Common::String result;
	for (uint i = 0; i < count; ++i) {
		result += getNextTextSection(text, index) + separator;

		if (index == Common::String::npos) {
			break;
		}
	}
	
	return result;
}

Common::String ButtonContainer::getNextTextSection(const Common::String &text, uint &index, uint count) const {
	return getNextTextSection(text, index, count, "\n");
}

Common::String ButtonContainer::getNextTextSection(const Common::String &text, uint &index) const {
	Common::String result;

	index = text.findFirstNotOf('\n', index);
	if (index == Common::String::npos) {
		return result;
	}

	uint endIndex = text.find('\n', index + 1);

	if (endIndex == Common::String::npos) {
		result = text.substr(index);
		index = endIndex;
		return result;
	}

	result = text.substr(index, endIndex - index);
	index = endIndex + 1;
	return result;
}

Common::String ButtonContainer::addNextTextToButtons(const Common::String &text, uint &index) {
	Common::String buttonText = getNextTextSection(text, index) + '\n';
	_buttonTexts.push_back(buttonText);
	return buttonText;
}

Common::String ButtonContainer::addNextTextToButtons(const Common::String &text, uint &index, uint count) {
	Common::String result;
	for (uint i = 0; i < count; ++i) {
		result += addNextTextToButtons(text, index);

		if (index == Common::String::npos) {
			break;
		}
	}
	return result;
}

void ButtonContainer::setButtonTexts(const Common::String &text) {
	_buttonTexts.clear();

	uint index = 0;
	for (uint i = 0; i < _buttons.size(); ++i) {
		if (_buttons[i]._value) {
			_buttonTexts.push_back(getNextTextSection(text, index));
		} else {
			_buttonTexts.push_back("");
		}

		if (index == Common::String::npos) {
			break;
		}
	}
}

void ButtonContainer::disableButtonVoicing(uint startIndex, uint endIndex) {
	for (uint i = startIndex; i < endIndex; ++i) {
		if (i < _buttons.size()) {
			_buttons[i]._canVoice = false;
		}
	}
}

void ButtonContainer::enableButtonVoicing(uint startIndex, uint endIndex) {
	for (uint i = startIndex; i < endIndex; ++i) {
		if (i < _buttons.size()) {
			_buttons[i]._canVoice = true;
		}
	}
}

#endif

/*------------------------------------------------------------------------*/

void SettingsBaseDialog::showContents(SpriteResource &title1, bool waitFlag) {
	_vm->_events->pollEventsAndWait();
	checkEvents(_vm);
}

} // End of namespace Xeen
} // End of namespace MM
