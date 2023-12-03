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

#include "ultima/nuvie/keybinding/key_help_dialog.h"
#include "common/translation.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/richtext.h"

namespace Ultima {
namespace Nuvie {

KeyHelpDialog::KeyHelpDialog(const Common::U32String &helpTxt)
	: GUI::Dialog("HelpDialog") {
	new GUI::RichTextWidget(this, "HelpDialog.TabWidget", helpTxt);
	new GUI::ButtonWidget(this, "HelpDialog.Close", Common::U32String("Close"), Common::U32String(), GUI::kCloseCmd);
}

void KeyHelpDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
		case GUI::kCloseCmd:
			close();
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
