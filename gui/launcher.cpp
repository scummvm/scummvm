/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include "browser.h"
#include "chooser.h"
#include "message.h"
#include "newgui.h"
#include "options.h"
#include "EditTextWidget.h"
#include "ListWidget.h"

#include "backends/fs/fs.h"
#include "common/config-file.h"
#include "common/engine.h"
#include "common/gameDetector.h"

#include "scumm/scumm.h"	// FIXME: this is only for GF_HUMONGOUS and *EVIL*

enum {
	kStartCmd = 'STRT',
	kOptionsCmd = 'OPTN',
	kAddGameCmd = 'ADDG',
	kEditGameCmd = 'EDTG',
	kRemoveGameCmd = 'REMG',
	kQuitCmd = 'QUIT'
};

typedef ScummVM::List<const VersionSettings *> GameList;

/*
 * A dialog that allows the user to edit a config game entry.
 * TODO: add widgets for some/all of the following
 * - Amiga/subtitles flag? Although those only make sense for Scumm games, not Simon
 * - The music driver for that game (<Default> or custom)
 *   Of course this means we need an API to query the available music drivers.
 * - Maybe scaler/graphics mode. But there are two problems:
 *   1) Different backends can have different scalers with different names,
 *      so we first have to add a way to query those... no Ender, I don't
 *      think a bitmasked property() value is nice for this,  because we would
 *      have to add to the bitmask values whenever a backends adds a new scaler).
 *   2) At the time the launcher is running, the GFX backend is already setup.
 *      So when a game is run via the launcher, the custom scaler setting for it won't be
 *      used. So we'd also have to add an API to change the scaler during runtime
 *      (the SDL backend can already do that based on user input, but there is no API
 *      to achieve it)
 *   If the APIs for 1&2 are in place, we can think about adding this to the Edit&Option dialogs
 * - Maybe SFX/Master/Music volumes?
 */

enum {
	kOKCmd = 'OK  '
};

class EditGameDialog : public Dialog {
	typedef ScummVM::String String;
	typedef ScummVM::StringList StringList;
public:
	EditGameDialog(NewGui *gui, Config &config, const String &domain);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	Config &_config;
	const String &_domain;
	EditTextWidget *_descriptionWidget;
	EditTextWidget *_domainWidget;
	CheckboxWidget *_fullscreenCheckbox;
	CheckboxWidget *_amigaPalCheckbox;
};

EditGameDialog::EditGameDialog(NewGui *gui, Config &config, const String &domain)
	: Dialog(gui, 8, 50, 320 - 2 * 8, 200 - 2 * 40), _config(config), _domain(domain) {
	// Determine the description string
	String gameid(_config.get("gameid", _domain));
	String description(_config.get("description", _domain));
	const VersionSettings *v = version_settings;

	if (gameid.isEmpty())
		gameid = _domain;

	// Find the VersionSettings for this gameid
	while (v->filename) {
		if (!scumm_stricmp(v->filename, gameid.c_str())) {
			break;
		}
		v++;
	}
	if (description.isEmpty()) {
		description = v->gamename;
	}
	
	// Label & edit widget for the game ID
	new StaticTextWidget(this, 10, 10, 40, kLineHeight, "ID: ", kTextAlignRight);
	_domainWidget =
		new EditTextWidget(this, 50, 10, _w - 50 - 10, kLineHeight, _domain);

	// Label & edit widget for the description
	new StaticTextWidget(this, 10, 26, 40, kLineHeight, "Name: ", kTextAlignRight);
	_descriptionWidget =
		new EditTextWidget(this, 50, 26, _w - 50 - 10, kLineHeight, description);

	// Path to game data (view only)
	String path(_config.get("path", _domain));
	new StaticTextWidget(this, 10, 42, 40, kLineHeight, "Path: ", kTextAlignRight);
	new StaticTextWidget(this, 50, 42, _w - 50 - 10, kLineHeight, path, kTextAlignLeft);

	// Full screen checkbox
	_fullscreenCheckbox = new CheckboxWidget(this, 15, 62, 200, 16, "Use Fullscreen Mode", 0, 'F');
	_fullscreenCheckbox->setState(_config.getBool("fullscreen", false, _domain));

	// Display 'Amiga' checkbox, but only for Scumm games.
	if (GID_SIMON_FIRST <= v->id && v->id <= GID_SIMON_LAST) {
		_amigaPalCheckbox = 0;
	} else {
		_amigaPalCheckbox = new CheckboxWidget(this, 15, 82, 200, 16, "Use Amiga Palette", 0, 'A');
		_amigaPalCheckbox->setState(_config.getBool("amiga", false, _domain));
	}

	// Add OK & Cancel buttons
	addButton(_w - 2 * (kButtonWidth + 10), _h - 24, "Cancel", kCloseCmd, 0);
	addButton(_w - (kButtonWidth + 10), _h - 24, "OK", kOKCmd, 0);
}

void EditGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kOKCmd) {
		// Write back changes made to config object
		String newDomain(_domainWidget->getLabel());
		if (newDomain != _domain) {
			if (newDomain.isEmpty() || _config.has_domain(newDomain)) {
				MessageDialog alert(_gui, "This game ID is already taken. Please choose another one.");
				alert.runModal();
				return;
			}
			_config.rename_domain(_domain, newDomain);
		}
		_config.set("description", _descriptionWidget->getLabel(), newDomain);
		if (_amigaPalCheckbox)
			_config.setBool("amiga", _amigaPalCheckbox->getState(), newDomain);
		_config.setBool("fullscreen", _fullscreenCheckbox->getState(), newDomain);
		setResult(1);
		close();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}

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
	: Dialog(gui, 0, 0, 320, 200), _detector(detector) {
	// Show game name
	new StaticTextWidget(this, 10, 8, 300, kLineHeight,
								"ScummVM "SCUMMVM_VERSION " (" SCUMMVM_CVS ")", 
								kTextAlignCenter);

	// Add three buttons at the bottom
	addButton(1 * (_w - kButtonWidth) / 6, _h - 24, "Quit", kQuitCmd, 'Q');
	addButton(3 * (_w - kButtonWidth) / 6, _h - 24, "Options", kOptionsCmd, 'O');
	_startButton = addButton(5 * (_w - kButtonWidth)/6, _h - 24, "Start", kStartCmd, 'S');

	// Add list with game titles
	_list = new ListWidget(this, 10, 28, 300, 112);
	_list->setEditable(false);
	_list->setNumberingMode(kListNumberingOff);

	// Two more buttons directly below the list box
	const int kBigButtonWidth = 90;
	new ButtonWidget(this, 10, 144, kBigButtonWidth, 16, "Add Game...", kAddGameCmd, 'A');
	_editButton = new ButtonWidget(this, (320-kBigButtonWidth) / 2, 144, kBigButtonWidth, 16, "Edit Game...", kEditGameCmd, 'E');
	_removeButton = new ButtonWidget(this, 320-kBigButtonWidth - 10, 144, kBigButtonWidth, 16, "Remove Game", kRemoveGameCmd, 'R');

	// Populate the list
	updateListing();

	// TODO - make a default selection (maybe the game user played last?)
	//_list->setSelected(0);

	// En-/Disable the buttons depending on the list selection
	updateButtons();

	// Create file browser dialog
	_browser = new BrowserDialog(_gui, "Select directory with game data");
}

LauncherDialog::~LauncherDialog() {
	delete _browser;
}

void LauncherDialog::open() {
	Dialog::open();
	g_config->set_writing(true);
}

void LauncherDialog::close() {
	g_config->flush();
	g_config->set_writing(false);
	Dialog::close();
}

void LauncherDialog::updateListing() {
	int i;
	const VersionSettings *v = version_settings;
	ScummVM::StringList l;

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	StringList domains = g_config->get_domains();
	for (i = 0; i < domains.size(); i++) {
		String name(g_config->get("gameid", domains[i]));
		String description(g_config->get("description", domains[i]));

		if (name.isEmpty())
			name = domains[i];
		if (description.isEmpty()) {
			v = version_settings;
			while (v->filename) {
				if (!scumm_stricmp(v->filename, name.c_str())) {
					description = v->gamename;
					break;
				}
				v++;
			}
		} 

		if (!name.isEmpty() && !description.isEmpty()) {
			// Insert the game into the launcher list
			int pos = 0, size = l.size();

			while (pos < size && (description > l[pos]))
				pos++;
			l.insert_at(pos, description);
			_domains.insert_at(pos, domains[i]);
		}
	}

	_list->setList(l);
	updateButtons();
}

/*
 * Return a list of all games which might be the game in the specified directory.
 */
GameList findGame(FilesystemNode *dir) {
	GameList list;

	FSList *files = dir->listDir(FilesystemNode::kListFilesOnly);
	const int size = files->size();
	char detectName[128];
	char detectName2[128];
	int i;

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	const VersionSettings *v = version_settings;
	while (v->filename && v->gamename) {

		// Determine the 'detectname' for this game, that is, the name of a 
		// file that *must* be presented if the directory contains the data
		// for this game. For example, FOA requires atlantis.000
		if (v->detectname) {
			strcpy(detectName, v->detectname);
			strcpy(detectName2, v->detectname);
			strcat(detectName2, ".");
		} else {
			strcpy(detectName, v->filename);
			strcpy(detectName2, v->filename);
			strcat(detectName, ".000");
			if (v->version >= 7) {
				strcat(detectName2, ".la0");
			} else if (v->features & GF_HUMONGOUS)
				strcat(detectName2, ".he0");
			else
				strcat(detectName2, ".sm0");
		}

		// Iterate over all files in the given directory
		for (i = 0; i < size; i++) {
			const char *filename = (*files)[i].displayName().c_str();

			if ((0 == scumm_stricmp(detectName, filename)) || (0 == scumm_stricmp(detectName2, filename))) {
				// Match found, add to list of candidates, then abort inner loop.
				list.push_back(v);
				break;
			}
		}

		v++;
	}

	return list;
}

void LauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int item =  _list->getSelected();

	switch (cmd) {
	case kAddGameCmd:
		// Allow user to add a new game to the list.
		// 1) show a dir selection dialog which lets the user pick the directory
		//    the game data resides in.
		// 2) try to auto detect which game is in the directory, if we cannot
		//    determine it uniquely preent a list of candidates to the user 
		//    to pick from
		// 3) Display the 'Edit' dialog for that item, letting the user specify
		//    an alternate description (to distinguish multiple versions of the
		//    game, e.g. 'Monkey German' and 'Monkey English') and set default
		//    options for that game.
		
		if (_browser->runModal()) {
			// User made his choice...
			FilesystemNode *dir = _browser->getResult();

			// ...so let's determine a list of candidates, games that
			// could be contained in the specified directory.
			GameList candidates = findGame(dir);
			const VersionSettings *v = 0;

			if (candidates.isEmpty()) {
				// No game was found in the specified directory
				MessageDialog alert(_gui, "ScummVM could not find any game in the specified directory!");
				alert.runModal();
			} else if (candidates.size() == 1) {
				// Exact match
				v = candidates[0];
			} else {
				// Display the candidates to the user and let her/him pick one
				StringList list;
				int i;
				for (i = 0; i < candidates.size(); i++)
					list.push_back(candidates[i]->gamename);
				
				ChooserDialog dialog(_gui, "Pick the game:", list);
				i = dialog.runModal();
				if (0 <= i && i < candidates.size())
					v = candidates[i];
			}

			if (v != 0) {
				// The auto detector or the user made a choice.
				// Pick a domain name which does not yet exist (after all, we
				// are *adding* a game to the config, not replacing).
				String domain(v->filename);
				if (g_config->has_domain(domain)) {
					char suffix = 'a';
					domain += suffix;
					while (g_config->has_domain(domain)) {
						domain.deleteLastChar();
						suffix++;
						domain += suffix;
					}
					g_config->set("gameid", v->filename, domain);
					g_config->set("description", v->gamename, domain);
				}
				g_config->set("path", dir->path(), domain);
				
				// Display edit dialog for the new entry
				EditGameDialog editDialog(_gui, *g_config, domain);
				if (editDialog.runModal()) {
					// User pressed OK, so make changes permanent

					// Write config to disk
					g_config->flush();
					
					// Update the ListWidget and force a redraw
					updateListing();
					draw();
				} else {
					// User aborted, remove the the new domain again
					g_config->delete_domain(domain);
				}
			}
		}
		break;
	case kRemoveGameCmd:
		// Remove the currently selected game from the list
		assert(item >= 0);
		g_config->delete_domain(_domains[item]);

		// Write config to disk
		g_config->flush();
		
		// Update the ListWidget and force a redraw
		updateListing();
		draw();
		break;
	case kEditGameCmd: {
		// Set game specifc options. Most of these should be "optional", i.e. by 
		// default set nothing and use the global ScummVM settings. E.g. the user
		// can set here an optional alternate music volume, or for specific games
		// a different music driver etc.
		// This is useful because e.g. MonkeyVGA needs Adlib music to have decent
		// music support etc.
		assert(item >= 0);
		EditGameDialog editDialog(_gui, *g_config, _domains[item]);
		if (editDialog.runModal()) {
			// User pressed OK, so make changes permanent

			// Write config to disk
			g_config->flush();
			
			// Update the ListWidget and force a redraw
			updateListing();
			draw();
		}
		}
		break;
	case kOptionsCmd: {
		// TODO - show up a generic options dialog with global options, including:
		// - the save path (use _browser!)
		// - music & graphics driver (but see also the comments on EditGameDialog
		//   for some techincal difficulties with this)
		// - default volumes (sfx/master/music)
		GlobalOptionsDialog options(_gui, _detector);
		options.runModal();
		}
		break;
	case kStartCmd:
	case kListItemDoubleClickedCmd:
		// Print out what was selected
		assert(item >= 0);
		_detector.setGame(_domains[item]);
		close();
		break;
	case kListSelectionChangedCmd:
		updateButtons();
		break;
	case kQuitCmd:
#ifdef __PALM_OS__
		close();
#endif
		g_system->quit();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void LauncherDialog::updateButtons() {
	bool enable = (_list->getSelected() >= 0);
	if (enable != _startButton->isEnabled()) {
		_startButton->setEnabled(enable);
		_startButton->draw();
	}
	if (enable != _editButton->isEnabled()) {
		_editButton->setEnabled(enable);
		_editButton->draw();
	}
	if (enable != _removeButton->isEnabled()) {
		_removeButton->setEnabled(enable);
		_removeButton->draw();
	}
}
