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

#include "engines/metaengine.h"
#include "common/algorithm.h"
#include "common/events.h"
#include "common/func.h"
#include "common/config-manager.h"

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



MassAddDialog::MassAddDialog(const Common::FSNode &startDir)
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

	_okButton = new ButtonWidget(this, "massadddialog_ok", "OK", kOkCmd, Common::ASCII_RETURN);
	_okButton->setEnabled(false);

	new ButtonWidget(this, "massadddialog_cancel", "Cancel", kCancelCmd, Common::ASCII_ESCAPE);

	// Build a map from all configured game paths to the targets using them
	const Common::ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	Common::ConfigManager::DomainMap::const_iterator iter;
	for (iter = domains.begin(); iter != domains.end(); ++iter) {

#ifdef __DS__
		// DS port uses an extra section called 'ds'.  This prevents the section from being
		// detected as a game.
		if (iter->_key == "ds") {
			continue;
		}
#endif

		Common::String path(iter->_value.get("path"));
		// Remove trailing slash, so that "/foo" and "/foo/" match.
		// This works around a bug in the POSIX FS code (and others?)
		// where paths are not normalized (so FSNodes refering to identical
		// FS objects may return different values in path()).
		while (path != "/" && path.lastChar() == '/')
			path.deleteLastChar();
		if (!path.empty())
			_pathToTargets[path].push_back(iter->_key);
	}
}

struct GameDescLess {
	bool operator()(const GameDescriptor &x, const GameDescriptor &y) const {
		return x.preferredtarget().compareToIgnoreCase(y.preferredtarget()) < 0;
	}
};


void MassAddDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	// FIXME: It's a really bad thing that we use two arbitrary constants
	if (cmd == kOkCmd) {
		// Sort the detected games. This is not strictly necessary, but nice for
		// people who want to edit their config file by hand after a mass add.
		sort(_games.begin(), _games.end(), GameDescLess());
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
		Common::FSNode dir = _scanStack.pop();

		Common::FSList files;
		if (!dir.getChildren(files, Common::FSNode::kListAll)) {
			error("browser returned a node that is not a directory: '%s'",
					dir.getPath().c_str());
		}

		// Run the detector on the dir
		GameList candidates(EngineMan.detectGames(files));

		// Just add all detected games / game variants. If we get more than one,
		// that either means the directory contains multiple games, or the detector
		// could not fully determine which game variant it was seeing. In either
		// case, let the user choose which entries he wants to keep.
		//
		// However, we only add games which are not already in the config file.
		for (GameList::const_iterator cand = candidates.begin(); cand != candidates.end(); ++cand) {
			GameDescriptor result = *cand;
			Common::String path = dir.getPath();

			// Remove trailing slashes
			while (path != "/" && path.lastChar() == '/')
				path.deleteLastChar();

			// Check for existing config entries for this path/gameid/lang/platform combination
			if (_pathToTargets.contains(path)) {
				bool duplicate = false;
				const Common::StringList &targets = _pathToTargets[path];
				for (Common::StringList::const_iterator iter = targets.begin(); iter != targets.end(); ++iter) {
					// If the gameid, platform and language match -> skip it
					Common::ConfigManager::Domain *dom = ConfMan.getDomain(*iter);
					assert(dom);

					if ((*dom)["gameid"] == result["gameid"] &&
					    (*dom)["platform"] == result["platform"] && 
					    (*dom)["language"] == result["language"]) {
						duplicate = true;
						break;
					}
				}
				if (duplicate)
					break;	// Skip duplicates
			}
			result["path"] = path;
			_games.push_back(result);
		}


		// Recurse into all subdirs
		for (Common::FSList::const_iterator file = files.begin(); file != files.end(); ++file) {
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

		snprintf(buf, sizeof(buf), "Discovered %d new games.", _games.size());
		_gameProgressText->setLabel(buf);

	} else {
		snprintf(buf, sizeof(buf), "Scanned %d directories ...", _dirsScanned);
		_dirProgressText->setLabel(buf);

		snprintf(buf, sizeof(buf), "Discovered %d new games ...", _games.size());
		_gameProgressText->setLabel(buf);
	}

	drawDialog();
}


} // end of namespace GUI

