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

#include "common/config-manager.h"
#include "got/views/view.h"
#include "got/gfx/palette.h"
#include "got/vars.h"

namespace Got {
namespace Views {

void View::checkFocusedControl(const Common::Point &mousePos) {
	if (_focusedElement) {
		if (!_focusedElement->getBounds().contains(mousePos)) {
			_focusedElement->send(MouseLeaveMessage());
			_focusedElement = nullptr;
		}

	} else {
		for (UIElement *child : _children) {
			if (child->getBounds().contains(mousePos)) {
				_focusedElement = child;
				child->send(MouseEnterMessage());
				break;
			}
		}
	}
}

UIElement *View::getElementAtPos(const Common::Point &pos) const {
	for (UIElement *child : _children) {
		if (child->getBounds().contains(pos))
			return child;
	}

	return nullptr;
}

bool View::msgFocus(const FocusMessage &msg) {
	_focusedElement = nullptr;
	return UIElement::msgFocus(msg);
}

bool View::msgUnfocus(const UnfocusMessage &msg) {
	if (_focusedElement)
		_focusedElement->send(MouseLeaveMessage());

	return UIElement::msgUnfocus(msg);
}

bool View::msgMouseMove(const MouseMoveMessage &msg) {
	checkFocusedControl(msg._pos);
	return true;
}

bool View::msgMouseDown(const MouseDownMessage &msg) {
	UIElement *child = getElementAtPos(msg._pos);
	return child ? child->send(msg) : false;
}

bool View::msgMouseUp(const MouseUpMessage &msg) {
	UIElement *child = getElementAtPos(msg._pos);
	return child ? child->send(msg) : false;
}

void View::playSound(int index, bool priority_override) {
	_G(sound).playSound(index, priority_override);
}

void View::playSound(const Gfx::GraphicChunk &src) {
	_G(sound).playSound(src);
}

void View::musicPlay(int num, bool override) {
	_G(sound).musicPlay(num, override);
}

void View::musicPlay(const char *name, bool override) {
	_G(sound).musicPlay(name, override);
}

void View::musicPause() {
	_G(sound).musicPause();
}

void View::musicResume() {
	_G(sound).musicResume();
}

void View::musicStop() {
	_G(sound).musicStop();
}

bool View::musicIsOn() const {
	return _G(sound).musicIsOn();
}

void View::fadeOut() {
	Gfx::fadeOut();
}

void View::fadeIn(const byte *pal) {
	Gfx::fadeIn(pal);
}

#ifdef USE_TTS

void View::sayText(const Common::String &text, Common::TextToSpeechManager::Action action) {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();

	// _previousSaid is used to prevent the TTS from looping when sayText is called inside a loop,
	// for example when options in selection menus are voiced. Without it when the text ends it would speak
	// the same text again.
	// _previousSaid is cleared when appropriate to allow for repeat requests
	if (ttsMan && ConfMan.getBool("tts_enabled") && _previousSaid != text) {
		Common::String ttsMessage;

		for (uint i = 0; i < text.size(); ++i) {
			// Some text is enclosed in < and >, which causes the text to not be voiced by TTS if they aren't replaced
			if (text[i] == '<' || text[i] == '>') {
				ttsMessage += ", ";
				continue;
			}

			// Ignore color changing characters
			if (text[i] == '~' && i < text.size() - 1 && Common::isXDigit(text[i + 1])) {
				i++;
				continue;
			}

			// Replace single newlines with spaces to make voicing of dialog smoother. If there are two or more newlines in a row,
			// then the pieces of text are most likely supposed to be voiced as separate sentences, so keep the newlines in
			// that case
			if (text[i] == '\n') {
				if (i < text.size() - 1 && text[i + 1] == '\n') {
					i++;
				} else {
					ttsMessage += ' ';
					continue;
				}
			}

			ttsMessage += text[i];
		}

		ttsMan->say(ttsMessage, action, Common::CodePage::kDos850);
		_previousSaid = text;
	}
}

#endif

} // namespace Views
} // namespace Got
