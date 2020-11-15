/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/system.h"

#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/edittext.h"

#include "engines/grim/inputdialog.h"

namespace Grim {

InputDialog::InputDialog(const Common::String &message, const Common::String &string, bool hasTextField) :
		GUI::Dialog(30, 20, 260, 124), _hasTextField(hasTextField),
		m_text(nullptr) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	Common::Array<Common::String> lines;
	int lineCount;
	int maxlineWidth = g_gui.getFont().wordWrapText(message, screenW - 2 * 20, lines);

	// Calculate the desired dialog size (maxing out at 300*180 for now)
	_w = MAX(maxlineWidth, (2 * buttonWidth) + 10) + 20;

	lineCount = lines.size();

	_h = 30 + kLineHeight;
	_h += buttonHeight + 10;

	// Limit the number of lines so that the dialog still fits on the screen.
	if (lineCount > (screenH - 20 - _h) / kLineHeight) {
		lineCount = (screenH - 20 - _h) / kLineHeight;
	}
	_h += lineCount * kLineHeight;

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	// Each line is represented by one static text item.
	int height = 10;
	for (int i = 0; i < lineCount; i++) {
		new GUI::StaticTextWidget(this, 10, height, maxlineWidth, kLineHeight,
								  lines[i], Graphics::kTextAlignCenter);
		height += kLineHeight;
	}
	height += 10;
	if (_hasTextField) {
		m_text = new GUI::EditTextWidget(this, 10, height, _w - 20, kLineHeight, Common::U32String(string), Common::U32String("input"));
		height += kLineHeight + 10;
	}

	new GUI::ButtonWidget(this, 10, height, buttonWidth, buttonHeight, Common::U32String("Ok"), Common::U32String(), GUI::kOKCmd, Common::ASCII_RETURN); // Confirm dialog
	new GUI::ButtonWidget(this, _w - buttonWidth - 10, height, buttonWidth, buttonHeight, Common::U32String("Cancel"), Common::U32String(), GUI::kCloseCmd, Common::ASCII_ESCAPE);   // Cancel dialog
}

Common::String InputDialog::getString() {
	return m_text->getEditString();
}

void InputDialog::handleKeyDown(Common::KeyState state) {
	if (state.keycode == Common::KEYCODE_RETURN) {
		setResult(1);
		close();
	} else if (state.keycode == Common::KEYCODE_ESCAPE) {
		setResult(0);
		close();
	} else {
		GUI::Dialog::handleKeyDown(state);
	}
}

void InputDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == GUI::kOKCmd) {
		setResult(1);
		close();
	} else if (cmd == GUI::kCloseCmd) {
		setResult(0);
		close();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

}
