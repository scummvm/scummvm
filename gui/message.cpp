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

#include "common/str.h"
#include "common/system.h"
#include "common/translation.h"
#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kDefaultCmd = 'DFLT',
	kAltCmd = 'ALTC'
};



// TODO: The default button should be visibly distinct from the alternate button

void MessageDialog::init(const Common::U32String &message,
						 const Common::U32String &defaultButton,
						 const Common::U32StringArray &altButtons,
						 Graphics::TextAlign alignment,
						 const char *url,
						 const Common::U32String &extraMessage) {
	// message widgets will be created in reflowLayout as needed
	_message = message;
	_alignment = alignment;
	_url = url;
	_extraMessage = nullptr;

	// Only use bogus sizes, we do the calculation in reflowLayout
	if (!defaultButton.empty()) {
		// Confirm dialog
		_buttons.push_back(new ButtonWidget(this, 0, 0, 0, 0, defaultButton, Common::U32String(), kDefaultCmd, Common::ASCII_RETURN));
	}

	int buttonHotKey = altButtons.size() == 1 ? Common::ASCII_ESCAPE : 0;
	for (size_t i = 0, total = altButtons.size(); i < total; ++i) {
		_buttons.push_back(new ButtonWidget(this, 0, 0, 0, 0, altButtons[i], Common::U32String(), kAltCmd + i, buttonHotKey));
		buttonHotKey = 0;
	}

	if (!extraMessage.empty()) {
		_extraMessage = new StaticTextWidget(this, 0, 0, 0, 0, extraMessage, Graphics::kTextAlignLeft);
	}
}

void MessageDialog::reflowLayout() {
	const int horizontalMargin = 10;
	const int buttonSpacing = 10;

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	Common::Array<Common::U32String> lines;
	size_t lineCount;

	int maxlineWidth = g_gui.getFont().wordWrapText(_message, screenW - 2 * horizontalMargin - 20, lines);

	const size_t buttonCount = _buttons.size();
	const int buttonsTotalWidth = buttonCount * buttonWidth + (buttonCount - 1) * buttonSpacing;

	// Calculate the desired dialog size
	_w = MAX(maxlineWidth, buttonsTotalWidth) + 2 * horizontalMargin;

	lineCount = lines.size();

	_h = 16;
	if (buttonCount)
		_h += buttonHeight + 8;

	// Limit the number of lines so that the dialog still fits on the screen.
	if (lineCount > size_t((screenH - 20 - _h) / kLineHeight)) {
		lineCount = (screenH - 20 - _h) / kLineHeight;
	}
	_h += lineCount * kLineHeight;

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	// Each line is represented by one static text item.
	// Update existing lines
	size_t toUpdateLines = MIN<size_t>(lineCount, _lines.size());
	for (size_t i = 0; i < toUpdateLines; i++) {
		_lines[i]->setPos(horizontalMargin, 10 + i * kLineHeight);
		_lines[i]->setSize(maxlineWidth, kLineHeight);
		_lines[i]->setLabel(lines[i]);
	}
	// Create missing lines
	for (size_t i = toUpdateLines; i < lineCount; i++) {
		_lines.push_back(new StaticTextWidget(this, horizontalMargin, 10 + i * kLineHeight, maxlineWidth, kLineHeight, lines[i], _alignment));
	}
	// Cleanup old useless lines
	for (size_t i = lineCount, total = _lines.size(); i < total; i++) {
		this->removeWidget(_lines[i]);
		delete _lines[i];
	}
	_lines.resize(lineCount);

	int buttonPos = (_w - buttonsTotalWidth) / 2;
	for (size_t i = 0; i < buttonCount; ++i) {
		_buttons[i]->setPos(buttonPos, _h - buttonHeight - 8);
		_buttons[i]->setSize(buttonWidth, buttonHeight);
		buttonPos += buttonWidth + buttonSpacing;
	}

	if (_extraMessage) {
		_extraMessage->setPos(10, _h);
		_extraMessage->setSize(maxlineWidth, kLineHeight);
		_h += kLineHeight;
	}
}

MessageDialog::MessageDialog(const Common::U32String &message,
							 const Common::U32String &defaultButton,
							 const Common::U32String &altButton,
							 Graphics::TextAlign alignment,
							 const char *url,
							 const Common::U32String &extraMessage)
	: Dialog(30, 20, 260, 124) {

	init(message, defaultButton,
		 altButton.empty() ? Common::U32StringArray() : Common::U32StringArray(1, altButton),
		 alignment, url, extraMessage);
}

MessageDialog::MessageDialog(const Common::String &message,
							 const Common::String &defaultButton,
							 const Common::String &altButton,
							 Graphics::TextAlign alignment,
							 const char *url)
	: Dialog(30, 20, 260, 124) {

	init(Common::U32String(message), Common::U32String(defaultButton),
		 altButton.empty() ? Common::U32StringArray() : Common::U32StringArray(1, Common::U32String(altButton)),
		 alignment, url, Common::U32String());
}

MessageDialog::MessageDialog(const Common::U32String &message,
							 const Common::U32String &defaultButton,
							 const Common::U32StringArray &altButtons,
							 Graphics::TextAlign alignment)
	: Dialog(30, 20, 260, 124) {

	init(message, defaultButton, altButtons, alignment, nullptr, Common::U32String());
}

void MessageDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kDefaultCmd) {
		setResult(kMessageOK);
		close();
		return;
	}
	if (cmd >= kAltCmd) {
		if (_url) {
			if (g_system->hasFeature(OSystem::kFeatureOpenUrl))
				g_system->openUrl(_url);

			setResult(kMessageOK);
		} else {
			setResult(kMessageAlt + cmd - kAltCmd);
		}
		close();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}

TimedMessageDialog::TimedMessageDialog(const Common::U32String &message, uint32 duration)
	: MessageDialog(message, Common::U32String(), Common::U32String()) {
	_timer = g_system->getMillis() + duration;
}

void TimedMessageDialog::handleTickle() {
	MessageDialog::handleTickle();
	if (g_system->getMillis() > _timer)
		close();
}

CountdownMessageDialog::CountdownMessageDialog(const Common::U32String &message,
				  uint32 duration,
				  const Common::U32String &defaultButton,
				  const Common::U32String &altButton,
				  Graphics::TextAlign alignment,
				  const Common::U32String &countdownMessage)
	: MessageDialog(message, defaultButton, altButton, alignment, nullptr, countdownMessage) {
	_startTime = g_system->getMillis();
	_timer = _startTime + duration;

	_countdownMessage = countdownMessage;

	updateCountdown();
}

void CountdownMessageDialog::handleTickle() {
	updateCountdown();

	MessageDialog::handleTickle();
	if (g_system->getMillis() > _timer) {
		setResult(kMessageAlt);
		close();
	}
}

void CountdownMessageDialog::updateCountdown() {
	uint32 secs = (_timer - g_system->getMillis()) / 1000;

	Common::U32String msg = Common::U32String::format(_countdownMessage, secs);

	if (msg != _extraMessage->getLabel()) {
		_extraMessage->setLabel(msg);
	}
}

MessageDialogWithURL::MessageDialogWithURL(const Common::U32String &message, const char *url, const Common::U32String &defaultButton, Graphics::TextAlign alignment)
	: MessageDialog(message, defaultButton, _("Open URL"), alignment, url) {
}

MessageDialogWithURL::MessageDialogWithURL(const Common::String &message, const char *url, const char *defaultButton, Graphics::TextAlign alignment)
		: MessageDialog(Common::U32String(message), Common::U32String(defaultButton), _("Open URL"), alignment, url) {
}


} // End of namespace GUI
