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
 * $URL$
 * $Id$
 */

#include "common/stdafx.h"

#include "engines/engine.h"
#include "base/game.h"
#include "base/plugins.h"

#include "gui/launcher.h"	// For addGameToConf()
#include "gui/massadd.h"
#include "gui/newgui.h"
#include "gui/widget.h"


namespace GUI {

/*
TODO:
- Themify this dialog
- Add a ListWidget showing all the games we are going to add, and update it live
- Add a 'busy' mouse cursor (animated?) which indicates to the user that
  something is in progress, and show this cursor while we scan
*/

enum {
	// Upper bound (im milliseconds) we want to spend in handleTickle.
	// Setting this low makes the GUI more responsive but also slows
	// down the scanning.
	kMaxScanTime = 50
};

enum {
	kOkCmd = 'OK  ',
	kCancelCmd = 'CNCL'
};



MassAddDialog::MassAddDialog(const FilesystemNode &startDir)
	: Dialog("massadddialog"),
	_dirsScanned(0),
	_okButton(0),
	_dirProgressText(0),
	_gameProgressText(0) {

	// The dir we start our scan at
	_scanStack.push(startDir);


	// Create dialog items
	// We need:
	// - "OK" button, only enabled after the scan has finished
	// - "Cancel" / "Abort" button, always active
	// - static text as headline for the dialog
	// - static text displaying the progress text
	// - (future) a listbox showing all the games we added/are going to add

	new StaticTextWidget(this, "massadddialog_caption",	"Mass Add Dialog");

	_dirProgressText = new StaticTextWidget(this, "massadddialog_dirprogress",
											"... progress ...");

	_gameProgressText = new StaticTextWidget(this, "massadddialog_gameprogress",
											 "... progress ...");

	_okButton = new ButtonWidget(this, "massadddialog_ok", "OK", kOkCmd, '\n');
	_okButton->setEnabled(false);

	new ButtonWidget(this, "massadddialog_cancel", "Cancel", kCancelCmd, '\27');

}


void MassAddDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	// FIXME: It's a really bad thing that we use two arbitrary constants
	if (cmd == kOkCmd) {
		// Add all the detected games to the config
		for (GameList::const_iterator iter = _games.begin(); iter != _games.end(); ++iter) {
			printf("  Added gameid '%s', desc '%s'\n",
				(*iter)["gameid"].c_str(),
				(*iter)["description"].c_str());
			addGameToConf(*iter);
		}

		// Write everything to disk
		ConfMan.flushToDisk();

		close();
	} else if (cmd == kCancelCmd) {
		// User cancelled, so we don't do anything and just leave.
		close();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}

void MassAddDialog::handleTickle() {
	if (_scanStack.empty())
		return;	// We have finished scanning

	uint32 t = g_system->getMillis();

	// Perform a breadth-first scan of the filesystem.
	while (!_scanStack.empty() && (g_system->getMillis() - t) < kMaxScanTime) {
		FilesystemNode dir = _scanStack.pop();
	
		FSList files;
		if (!dir.getChildren(files, FilesystemNode::kListAll)) {
			error("browser returned a node that is not a directory: '%s'",
					dir.getPath().c_str());
		}
	
		// Run the detector on the dir
		GameList candidates(PluginManager::instance().detectGames(files));
		
		if (candidates.size() >= 1) {
			// At least one match was found. For now we just take the first one...
			// a more sophisticated solution would do something more clever here,
			// e.g. ask the user which one to pick (make sure to display the 
			// path, too).
			GameDescriptor result = candidates[0];
			result["path"] = dir.getPath();
			
			_games.push_back(result);
		}
		
		
		// Recurse into all subdirs
		for (FSList::const_iterator file = files.begin(); file != files.end(); ++file) {
			if (file->isDirectory()) {
				_scanStack.push(*file);
			}
		}

		_dirsScanned++;
	}
	
	
	// Update the dialog
	char buf[256];

	if (_scanStack.empty()) {
		// Enable the OK button
		_okButton->setEnabled(true);

		snprintf(buf, sizeof(buf), "Scan complete!");
		_dirProgressText->setLabel(buf);
	
		snprintf(buf, sizeof(buf), "Discovered %d games.", _games.size());
		_gameProgressText->setLabel(buf);

	} else {
		snprintf(buf, sizeof(buf), "Scanned %d directories ...", _dirsScanned);
		_dirProgressText->setLabel(buf);
	
		snprintf(buf, sizeof(buf), "Discovered %d games ...", _games.size());
		_gameProgressText->setLabel(buf);
	}

	drawDialog();
}


} // end of namespace GUI

