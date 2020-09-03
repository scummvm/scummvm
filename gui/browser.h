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

#ifndef BROWSER_DIALOG_H
#define BROWSER_DIALOG_H

#include "gui/dialog.h"
#include "common/fs.h"
#include "common/str.h"

namespace GUI {

class ListWidget;
class EditTextWidget;
class CheckboxWidget;
class CommandSender;

class BrowserDialog : public Dialog {
public:
	BrowserDialog(const Common::U32String &title, bool dirBrowser);

	int runModal() override;
	void open() override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	const Common::FSNode	&getResult() { return _choice; }

protected:
	ListWidget		*_fileList;
	EditTextWidget	*_currentPath;
	Common::FSNode	_node;
	Common::FSList	_nodeContent;

	bool _showHidden;
	CheckboxWidget *_showHiddenWidget;

	Common::FSNode		_choice;
	Common::U32String	_title;
	bool				_isDirBrowser;

	void updateListing();
};

} // End of namespace GUI

#endif
