/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "launcher.h"
#include "newgui.h"
#include "ListWidget.h"

#include "common/engine.h"
#include "common/gameDetector.h"
#include "common/list.h"

enum {
	kOptionsCmd = 'QUIT',
	kQuitCmd = 'QUIT'
};

LauncherDialog::LauncherDialog(NewGui *gui)
	: Dialog(gui, 0, 0, 320, 200)
{
	// Add three buttons at the bottom
	addButton(1*(_w - 54)/4, _h - 24, 54, 16, "Quit", kQuitCmd, 'Q');
	addButton(2*(_w - 54)/4, _h - 24, 54, 16, "Options", kOptionsCmd, 'O');
	addButton(3*(_w - 54)/4, _h - 24, 54, 16, "Run", kCloseCmd, 'R');

	// Add list with game titles
	ListWidget *w = new ListWidget(this, 10, 10, 300, 112);
	w->setNumberingMode(kListNumberingOff);
	
	const VersionSettings *v = version_settings;
	ScummVM::StringList l;

	while (v->filename && v->gamename) {
		l.push_back(v->gamename);
		v++;
	}

	w->setList(l);
	
	// TODO - add an edit field with the path information; or maybe even a "file selector" ?
}

void LauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	switch (cmd) {
	case kListItemChangedCmd:
		break;
	case kListItemDoubleClickedCmd:
		break;
	case kQuitCmd:
		g_system->quit();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}
