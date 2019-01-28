/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/world/dialogpanel.h"

#include "engines/stark/gfx/driver.h"

#include "engines/stark/resources/speech.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/settings.h"
#include "engines/stark/services/userinterface.h"

#include "engines/stark/ui/cursor.h"
#include "engines/stark/ui/world/clicktext.h"

#include "engines/stark/visual/image.h"
#include "engines/stark/visual/text.h"

namespace Stark {

const Color DialogPanel::_aprilColor = Color(0xFF, 0xC0, 0x00);
const Color DialogPanel::_otherColor = Color(0xFF, 0x40, 0x40);

DialogPanel::DialogPanel(Gfx::Driver *gfx, Cursor *cursor) :
		Window(gfx, cursor),
		_subtitleVisual(nullptr),
		_currentSpeech(nullptr),
		_scrollUpArrowVisible(false),
		_scrollDownArrowVisible(false),
		_firstVisibleOption(0),
		_lastVisibleOption(0),
		_focusedOption(0),
		_acceptIdleMousePos(false) {
	_position = Common::Rect(Gfx::Driver::kOriginalWidth, Gfx::Driver::kBottomBorderHeight);
	_position.translate(0, Gfx::Driver::kTopBorderHeight + Gfx::Driver::kGameViewportHeight);

	_visible = true;

	_activeBackGroundTexture = StarkStaticProvider->getUIElement(StaticProvider::kTextBackgroundActive);
	_passiveBackGroundTexture = StarkStaticProvider->getUIElement(StaticProvider::kTextBackgroundPassive);
	_scrollUpArrowImage = StarkStaticProvider->getUIElement(StaticProvider::kTextScrollUpArrow);
	_scrollDownArrowImage = StarkStaticProvider->getUIElement(StaticProvider::kTextScrollDownArrow);
	_dialogOptionBullet = StarkStaticProvider->getUIImage(StaticProvider::kDialogOptionBullet);

	_scrollUpArrowRect = Common::Rect(_scrollUpArrowImage->getWidth(), _scrollUpArrowImage->getHeight());
	_scrollUpArrowRect.translate(0, _optionsTop);

	_scrollDownArrowRect = Common::Rect(_scrollDownArrowImage->getWidth(), _scrollDownArrowImage->getHeight());
	_scrollDownArrowRect.translate(0, _optionsTop + _optionsHeight - _scrollDownArrowImage->getHeight() - 9);
}

DialogPanel::~DialogPanel() {
	clearOptions();
	clearSubtitleVisual();
}

void DialogPanel::abortCurrentSpeech() {
	if (_currentSpeech) {
		_currentSpeech->stop();
		_currentSpeech = nullptr;
	}
}

void DialogPanel::clearSubtitleVisual() {
	delete _subtitleVisual;
	_subtitleVisual = nullptr;
}

void DialogPanel::clearOptions() {
	for (uint i = 0; i < _options.size(); i++) {
		delete _options[i];
	}
	_options.clear();
}

void DialogPanel::renderOptions() {
	uint32 pos = _optionsTop;
	for (uint i = _firstVisibleOption; i <= _lastVisibleOption; ++i) {
		_options[i]->setPosition(Common::Point(_optionsLeft, pos));
		_options[i]->render();

		_dialogOptionBullet->render(Common::Point(_optionsLeft - 13, pos + 3), false);

		pos += _options[i]->getHeight();
	}

	_scrollUpArrowVisible = _firstVisibleOption > 0;
	_scrollDownArrowVisible = _lastVisibleOption < _options.size() - 1;
}

void DialogPanel::renderScrollArrows() const {
	if (_scrollUpArrowVisible) {
		_scrollUpArrowImage->render(Common::Point(_scrollUpArrowRect.left, _scrollUpArrowRect.top), true);
	}

	if (_scrollDownArrowVisible) {
		_scrollDownArrowImage->render(Common::Point(_scrollDownArrowRect.left, _scrollDownArrowRect.top), true);
	}
}

void DialogPanel::onGameLoop() {
	// Clear completed speeches
	if (!_currentSpeech || !_currentSpeech->isPlaying()) {
		_currentSpeech = nullptr;

		clearSubtitleVisual();

		// Toggle subtitles on and off when requested
		if (StarkUserInterface->hasToggleSubtitleRequest()) {
			StarkUserInterface->performToggleSubtitle();
		}
	}

	// Update the dialog engine
	StarkDialogPlayer->update();

	// Check if a new speech can be played
	if (StarkDialogPlayer->isSpeechReady()) {
		_currentSpeech = StarkDialogPlayer->acquireReadySpeech();
		_currentSpeech->playSound();
		updateSubtitleVisual();
	}

	if (_options.empty() && StarkDialogPlayer->areOptionsAvailable()) {
		updateDialogOptions();
	}
}

void DialogPanel::onRender() {
	// Draw options if available
	if (!_options.empty()) {
		_activeBackGroundTexture->render(Common::Point(0, 0), false);

		renderOptions();
		renderScrollArrows();
	} else {
		_passiveBackGroundTexture->render(Common::Point(0, 0), false);

		// Draw subtitle if available
		if (_subtitleVisual && StarkSettings->getBoolSetting(Settings::kSubtitle)) {
			_subtitleVisual->render(Common::Point(_optionsLeft, _optionsTop));
		}
	}
}

void DialogPanel::updateSubtitleVisual() {
	clearSubtitleVisual();

	Color color = _otherColor;
	if (_currentSpeech->characterIsApril())
		color = _aprilColor;

	_subtitleVisual = new VisualText(_gfx);
	_subtitleVisual->setText(_currentSpeech->getPhrase());
	_subtitleVisual->setColor(color);
	_subtitleVisual->setFont(FontProvider::kBigFont);
	_subtitleVisual->setTargetWidth(600);
}

void DialogPanel::updateDialogOptions() {
	clearOptions();

	_firstVisibleOption = 0;
	_lastVisibleOption = 0;
	_focusedOption = 0;
	Common::Array<DialogPlayer::Option> options = StarkDialogPlayer->listOptions();

	for (uint i = 0; i < options.size(); i++) {
		_options.push_back(new ClickText(options[i]._caption, _aprilColor));
	}

	if (!_options.empty()) {
		updateLastVisibleOption();
		_options[_focusedOption]->setActive();
		_acceptIdleMousePos = true;
	}
}

void DialogPanel::onMouseMove(const Common::Point &pos) {
	static Common::Point prevPos;

	if (_subtitleVisual) {
		_cursor->setCursorType(Cursor::kDefault);
	} else if (!_options.empty()) {
		if (pos != prevPos || _acceptIdleMousePos) {
			for (uint i = _firstVisibleOption; i <= _lastVisibleOption; ++i) {
				if (_options[i]->containsPoint(pos)) {
					_options[_focusedOption]->setPassive();
					_focusedOption = i;
					_options[_focusedOption]->setActive();

					_cursor->setCursorType(Cursor::kActive);
					_acceptIdleMousePos = false;

					prevPos = pos;
					return;
				}
			}
		}
		
		if (_options[_focusedOption]->containsPoint(pos)) {
			_cursor->setCursorType(Cursor::kActive);
		} else if (_scrollUpArrowVisible && _scrollUpArrowRect.contains(pos)) {
			_cursor->setCursorType(Cursor::kActive);
		} else if (_scrollDownArrowVisible && _scrollDownArrowRect.contains(pos)) {
			_cursor->setCursorType(Cursor::kActive);
		} else {
			_cursor->setCursorType(Cursor::kDefault);
		}
	} else {
		_cursor->setCursorType(Cursor::kDefault);
	}

	prevPos = pos;
}

void DialogPanel::onClick(const Common::Point &pos) {
	if (!_options.empty()) {
		if (_options[_focusedOption]->containsPoint(pos)) {
			selectFocusedOption();
		}

		if (_scrollUpArrowVisible && _scrollUpArrowRect.contains(pos)) {
			scrollUp();
		}

		if (_scrollDownArrowVisible && _scrollDownArrowRect.contains(pos)) {
			scrollDown();
		}
	}
}

void DialogPanel::onRightClick(const Common::Point &pos) {
	if (_currentSpeech && _currentSpeech->isPlaying()) {
		abortCurrentSpeech();
		clearSubtitleVisual();
	}
}

void DialogPanel::reset() {
	abortCurrentSpeech();
	clearSubtitleVisual();
	clearOptions();

	StarkDialogPlayer->reset();
}

void DialogPanel::scrollUp() {
	if (!_scrollUpArrowVisible) return;

	_lastVisibleOption = _firstVisibleOption;
	updateFirstVisibleOption();

	_options[_focusedOption]->setPassive();
	_focusedOption = _lastVisibleOption;
	_options[_focusedOption]->setActive();
}

void DialogPanel::scrollDown() {
	if (!_scrollDownArrowVisible) return;

	_firstVisibleOption = _lastVisibleOption;
	updateLastVisibleOption();

	_options[_focusedOption]->setPassive();
	_focusedOption = _firstVisibleOption;
	_options[_focusedOption]->setActive();
}

void DialogPanel::focusNextOption() {
	if (_options.empty() || _focusedOption == _options.size() - 1) return;

	_options[_focusedOption]->setPassive();
	++_focusedOption;
	_options[_focusedOption]->setActive();

	if (_focusedOption > _lastVisibleOption) {
		_lastVisibleOption = _focusedOption;
		updateFirstVisibleOption();
	}
}

void DialogPanel::focusPrevOption() {
	if (_options.empty() || _focusedOption == 0) return;

	_options[_focusedOption]->setPassive();
	--_focusedOption;
	_options[_focusedOption]->setActive();

	if (_focusedOption < _firstVisibleOption) {
		_firstVisibleOption = _focusedOption;
		updateLastVisibleOption();
	}
}

void DialogPanel::selectFocusedOption() {
	if (_options.size() > 0) {
		StarkDialogPlayer->selectOption(_focusedOption);
		clearOptions();
	}
}

void DialogPanel::selectOption(uint index) {
	if (_options.size() <= index) return;

	StarkDialogPlayer->selectOption(index);
	clearOptions();
}

void DialogPanel::onScreenChanged() {
	if (_currentSpeech) {
		updateSubtitleVisual();
	} else {
		updateDialogOptions();
	}
}

void DialogPanel::updateFirstVisibleOption() {
	_firstVisibleOption = _lastVisibleOption;
	uint32 height = _optionsTop + _options[_lastVisibleOption]->getHeight();

	while (_firstVisibleOption > 0) {
		height += _options[_firstVisibleOption - 1]->getHeight();
		if (height <= _optionsHeight) {
			--_firstVisibleOption;
		} else {
			break;
		}
	}

	if (_firstVisibleOption == 0) {
		while (_lastVisibleOption < _options.size() - 1) {
			height += _options[_lastVisibleOption + 1]->getHeight();
			if (height <= _optionsHeight) {
				++_lastVisibleOption;
			} else {
				break;
			}
		}
	}
}

void DialogPanel::updateLastVisibleOption() {
	_lastVisibleOption = _firstVisibleOption;
	uint32 height = _optionsTop + _options[_firstVisibleOption]->getHeight();

	while (_lastVisibleOption < _options.size() - 1) {
		height += _options[_lastVisibleOption + 1]->getHeight();
		if (height <= _optionsHeight) {
			++_lastVisibleOption;
		} else {
			break;
		}
	}

	if (_lastVisibleOption == _options.size() - 1) {
		while (_firstVisibleOption > 0) {
			height += _options[_firstVisibleOption - 1]->getHeight();
			if (height <= _optionsHeight) {
				--_firstVisibleOption;
			} else {
				break;
			}
		}
	}
}

} // End of namespace Stark
