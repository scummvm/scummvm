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

#ifndef NUVIE_KEYHELP_DIALOG_H
#define NUVIE_KEYHELP_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/str-array.h"

namespace GUI {
	class CommandSender;
}

namespace Ultima {
namespace Nuvie {

/**
 * Key help dialog - just a rich text and a close button.
 */
class KeyHelpDialog : public GUI::Dialog {
public:
	KeyHelpDialog(const Common::U32String &helpStr);

	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
};


} // End of namespace Nuvie
} // End of namespace Ultima

#endif
