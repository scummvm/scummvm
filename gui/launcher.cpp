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

#include "common/config-file.h"
#include "common/engine.h"
#include "common/gameDetector.h"

enum {
	kStartCmd = 'STRT',
	kOptionsCmd = 'OPTN',
	kAddGameCmd = 'ADDG',
	kConfigureGameCmd = 'CONF',
	kQuitCmd = 'QUIT'
};
	
/*
 * TODO list
 * - add an text entry widget
 * - add an "Add Game..." button that opens a dialog where new games can be 
 *   configured and added to the list of games
 * - add an "Edit Game..." button that opens a dialog that allows to edit game
 *   settings, i.e. the datapath/savepath/sound driver/... for that game
 * - add an "options" dialog
 * - ...
 */

LauncherDialog::LauncherDialog(NewGui *gui, GameDetector &detector)
	: Dialog(gui, 0, 0, 320, 200), _detector(detector)
{
	Widget *bw;

	// Show game name
	new StaticTextWidget(this, 10, 8, 300, kLineHeight,
								"ScummVM "SCUMMVM_VERSION " (" SCUMMVM_CVS ")", 
								kTextAlignCenter);

	// Add three buttons at the bottom
	bw = addButton(1*(_w - kButtonWidth)/6, _h - 24, "Quit", kQuitCmd, 'Q');
	bw = addButton(3*(_w - kButtonWidth)/6, _h - 24, "Options", kOptionsCmd, 'O');
	bw->setEnabled(false);
	_startButton = addButton(5*(_w - kButtonWidth)/6, _h - 24, "Start", kStartCmd, 'S');
	_startButton->setEnabled(false);

	// Add list with game titles
	_list = new ListWidget(this, 10, 28, 300, 112);
	_list->setEditable(false);
	_list->setNumberingMode(kListNumberingOff);
	
	const VersionSettings *v = version_settings;
	ScummVM::StringList l;

	// TODO - maybe only display those games for which settings are known
	// (i.e. a path to the game data was set and is accesible) ?
	while (v->filename && v->gamename) {
		if (g_config->has_domain(v->filename)) {
			String name = v->gamename;
			int pos = 0, size = l.size();
			while (pos < size && (name > l[pos]))
				pos++;
			l.insert_at(pos, name);
			_filenames.insert_at(pos, v->filename);
		}
		v++;
	}

	_list->setList(l);
//	_list->setSelected(0);

	// Two more buttons directly below the list box
	bw = new ButtonWidget(this, 10, 144, 80, 16, "Add Game...", kAddGameCmd, 'A');
	bw->setEnabled(false);
	bw = new ButtonWidget(this, 320-90, 144, 80, 16, "Configure...", kConfigureGameCmd, 'C');
	bw->setEnabled(false);
}

void LauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	int item;
	
	switch (cmd) {
	case kStartCmd:
	case kListItemDoubleClickedCmd:
		// Print out what was selected
		item =  _list->getSelected();
		assert(item >= 0);
		_detector.setGame(_filenames[item].c_str());
		close();
		break;
	case kListSelectionChangedCmd:
		_startButton->setEnabled(_list->getSelected() >= 0);
		_startButton->draw();
		break;
	case kQuitCmd:
		g_system->quit();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}
