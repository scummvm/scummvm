/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef GRIM_INPUTDIALOG_H
#define GRIM_INPUTDIALOG_H

#include "gui/dialog.h"

namespace GUI {
class CommandSender;
class EditTextWidget;
}

namespace Grim {

class InputDialog : public GUI::Dialog {
public:
	InputDialog(const Common::String &message, const Common::String &string);

	const Common::String &getString() const;

protected:
	virtual void handleKeyDown(Common::KeyState state);
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);

private:
	GUI::EditTextWidget *m_text;

};

}

#endif
