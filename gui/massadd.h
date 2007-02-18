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
 * $URL$
 * $Id$
 */

#ifndef MASSADD_DIALOG_H
#define MASSADD_DIALOG_H

#include "gui/dialog.h"
#include "common/fs.h"
#include "common/stack.h"

namespace GUI {

class StaticTextWidget;

class MassAddDialog : public Dialog {
public:
	MassAddDialog(const FilesystemNode &startDir);

	//void open();
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	void handleTickle();

private:
	Common::Stack<FilesystemNode>  _scanStack;
	GameList _games;
	
	int _dirsScanned;

	Widget *_okButton;
	StaticTextWidget *_dirProgressText;
	StaticTextWidget *_gameProgressText;
};


} // End of namespace GUI

#endif
