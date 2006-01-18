/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * $Header$
 */

#ifndef LAUNCHER_DIALOG_H
#define LAUNCHER_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"

class GameDetector;

namespace GUI {

class BrowserDialog;
class ListWidget;

class LauncherDialog : public Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	LauncherDialog(GameDetector &detector);
	~LauncherDialog();

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	ListWidget		*_list;
	Widget			*_startButton;
	Widget			*_editButton;
	Widget			*_removeButton;
	StringList		_domains;
	GameDetector 	&_detector;
	BrowserDialog	*_browser;

	void updateListing();
	void updateButtons();

	void close();
	virtual void addGame();
	void removeGame(int item);
	void editGame(int item);

	void selectGame(const String &name);
};

} // End of namespace GUI

#endif
