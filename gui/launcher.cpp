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
	int i;
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


	// Retrieve a list of all games defined in the config file
	char domains[255][100];
	int count = g_config->get_domains(domains);
	for (i=0;i<count;i++) {
		String name = (char*)g_config->get("gameid", domains[i]);
		String description = (char*)g_config->get("description", domains[i]);
		
		if (name.isEmpty() || description.isEmpty()) {
			v = version_settings;
			while (v->filename && v->gamename) {
				if (!scumm_stricmp(v->filename, domains[i])) {
					name = domains[i];
					description = v->gamename;
					break;
				}
				v++;
			}
		} 

		if (!name.isEmpty() && !description.isEmpty()) {
			// Insert the game into the launcher list
			int pos = 0, size = l.size();

			while (pos < size && (name > l[pos]))
				pos++;
			l.insert_at(pos, description);
			_filenames.insert_at(pos, domains[i]);
		}
	}

	if (l.size() > 0) 
		_list->setList(l);

	// TODO - make a default selection (maybe the game user played last?)
	//_list->setSelected(0);

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
	case kAddGameCmd:
		// TODO: Allow user to add a new game to the list.
		// 1) show a file selection dialog (to be implemented!) which lets 
		// the user pick the directory the game data resides in.
		// 2) show the user a list of games to pick from. Initially just show
		// all known games. But ideally, we would refine this list by checking
		// which choices are possible. E.g. if we don't find atlantis.000 in that
		// directory, then it's not FOA etc.
		break;
	case kConfigureGameCmd:
		// Set game specifc options. Most of these should be "optional", i.e. by 
		// default set nothing and use the global ScummVM settings. E.g. the user
		// can set here an optional alternate music volume, or for specific games
		// a different music driver etc.
		// This is useful because e.g. MonkeyVGA needs Adlib music to have decent
		// music support etc.
		break;
	case kOptionsCmd:
		// TODO - show up a generic options dialog, loosely based upon the one 
		// we have in scumm/dialogs.cpp. So we will be modifying the settings
		// in _detector, like which music engine to use, volumes, etc.
		//
		// We also allow the global save game path to be set here.
		break;
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
