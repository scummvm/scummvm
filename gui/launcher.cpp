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

#include "gui/about.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/launcher.h"
#include "gui/message.h"
#include "gui/newgui.h"
#include "gui/options.h"
#include "gui/EditTextWidget.h"
#include "gui/ListWidget.h"

#include "backends/fs/fs.h"

#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"

using Common::ConfigManager;

enum {
	kStartCmd = 'STRT',
	kAboutCmd = 'ABOU',
	kOptionsCmd = 'OPTN',
	kAddGameCmd = 'ADDG',
	kEditGameCmd = 'EDTG',
	kRemoveGameCmd = 'REMG',
	kQuitCmd = 'QUIT'
};

typedef Common::List<const GameSettings *> GameList;

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
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	EditGameDialog(NewGui *gui, const String &domain, const GameSettings *target);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	const String &_domain;
	EditTextWidget *_descriptionWidget;
	EditTextWidget *_domainWidget;
	CheckboxWidget *_fullscreenCheckbox;
	CheckboxWidget *_amigaCheckbox;
};

EditGameDialog::EditGameDialog(NewGui *gui, const String &domain, const GameSettings *target)
	: Dialog(gui, 8, 50, 320 - 2 * 8, 200 - 2 * 40),
	  _domain(domain) {

	// Determine the description string
	String description(ConfMan.get("description", domain));
	if (description.isEmpty() && target) {
		description = target->description;
	}

	// Determine whether this is a SCUMM game
	// FIXME: This check is evil, as it requires us to hard code GIDs.
	bool isScumm = target && (GID_SCUMM_FIRST <= target->id && target->id <= GID_SCUMM_LAST);

	
	// Label & edit widget for the game ID
	new StaticTextWidget(this, 10, 10, 40, kLineHeight, "ID: ", kTextAlignRight);
	_domainWidget =
		new EditTextWidget(this, 50, 10, _w - 50 - 10, kLineHeight, _domain);

	// Label & edit widget for the description
	new StaticTextWidget(this, 10, 26, 40, kLineHeight, "Name: ", kTextAlignRight);
	_descriptionWidget =
		new EditTextWidget(this, 50, 26, _w - 50 - 10, kLineHeight, description);

	// Path to game data (view only)
	String path(ConfMan.get("path", _domain));
	new StaticTextWidget(this, 10, 42, 40, kLineHeight, "Path: ", kTextAlignRight);
	new StaticTextWidget(this, 50, 42, _w - 50 - 10, kLineHeight, path, kTextAlignLeft);

	// Full screen checkbox
	_fullscreenCheckbox = new CheckboxWidget(this, 15, 62, 200, 16, "Use Fullscreen Mode", 0, 'F');
	_fullscreenCheckbox->setState(ConfMan.getBool("fullscreen", _domain));

	// Display 'Amiga' checkbox, but only for Scumm games.
	if (isScumm) {
		_amigaCheckbox = new CheckboxWidget(this, 15, 82, 200, 16, "Amiga Version", 0, 'A');
		_amigaCheckbox->setState(ConfMan.getBool("amiga", _domain));
	} else {
		_amigaCheckbox = 0;
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
			if (newDomain.isEmpty() || ConfMan.hasGameDomain(newDomain)) {
				MessageDialog alert(_gui, "This game ID is already taken. Please choose another one.");
				alert.runModal();
				return;
			}
			ConfMan.renameGameDomain(_domain, newDomain);
		}
		ConfMan.set("description", _descriptionWidget->getLabel(), newDomain);
		if (_amigaCheckbox)
			ConfMan.set("amiga", _amigaCheckbox->getState(), newDomain);
		ConfMan.set("fullscreen", _fullscreenCheckbox->getState(), newDomain);
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
	new StaticTextWidget(this, 10, 8, 300, kLineHeight, gScummVMFullVersion, kTextAlignCenter);

	// Add three buttons at the bottom
	const int border = 10;
	const int space = 8;
	const int buttons = 4;
	const int width = (_w - 2 * border - space * (buttons - 1)) / buttons;
	int x = border;
	new ButtonWidget(this, x, _h - 24, width, 16, "Quit", kQuitCmd, 'Q'); x += space + width;
	new ButtonWidget(this, x, _h - 24, width, 16, "About", kAboutCmd, 'B'); x += space + width;
	new ButtonWidget(this, x, _h - 24, width, 16, "Options", kOptionsCmd, 'O'); x += space + width;
	_startButton =
	new ButtonWidget(this, x, _h - 24, width, 16, "Start", kStartCmd, 'S'); x += space + width;

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
/* FIXME / TODO: config rewrite
	g_config->set_writing(true);
*/
}

void LauncherDialog::close() {
	ConfMan.flushToDisk();
/* FIXME / TODO: config rewrite
	g_config->set_writing(false);
*/
	Dialog::close();
}

void LauncherDialog::updateListing() {
	Common::StringList l;

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	ConfigManager::DomainMap::ConstIterator iter = domains.begin();
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		String name(iter->_value.get("gameid"));
		String description(iter->_value.get("description"));

		if (name.isEmpty())
			name = iter->_key;
		if (description.isEmpty()) {
			const GameSettings *v = _detector.findGame(name);
			if (v && v->description)
				description = v->description;
		} 

		if (!name.isEmpty() && !description.isEmpty()) {
			// Insert the game into the launcher list
			int pos = 0, size = l.size();

			while (pos < size && (description > l[pos]))
				pos++;
			l.insert_at(pos, description);
			_domains.insert_at(pos, iter->_key);
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
	char detectName3[128];

	// Iterate over all known games and for each check if it might be
	// the game in the presented directory.
	const PluginList &plugins = PluginManager::instance().getPlugins();
	int p;
	for (p = 0; p < plugins.size(); p++) {
		const GameSettings *v = plugins[p]->getSupportedGames();
		while (v->gameName && v->description) {
	
			// Determine the 'detectname' for this game, that is, the name of a 
			// file that *must* be presented if the directory contains the data
			// for this game. For example, FOA requires atlantis.000
			if (v->detectname) {
				strcpy(detectName, v->detectname);
				strcpy(detectName2, v->detectname);
				strcat(detectName2, ".");
				detectName3[0] = '\0';
			} else {
				strcpy(detectName, v->gameName);
				strcpy(detectName2, v->gameName);
				strcpy(detectName3, v->gameName);
				strcat(detectName, ".000");
				if (v->version >= 7) {
					strcat(detectName2, ".la0");
				} else
					strcat(detectName2, ".sm0");
				strcat(detectName3, ".he0");
			}
	
			// Iterate over all files in the given directory
			for (int i = 0; i < size; i++) {
				const char *gameName = (*files)[i].displayName().c_str();
	
				if ((0 == scumm_stricmp(detectName, gameName))  || 
					(0 == scumm_stricmp(detectName2, gameName)) ||
					(0 == scumm_stricmp(detectName3, gameName))) {
					// Match found, add to list of candidates, then abort inner loop.
					list.push_back(v);
					break;
				}
			}
	
			v++;
		}
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
			const GameSettings *v = 0;

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
					list.push_back(candidates[i]->description);
				
				ChooserDialog dialog(_gui, "Pick the game:", list);
				i = dialog.runModal();
				if (0 <= i && i < candidates.size())
					v = candidates[i];
			}

			if (v != 0) {
				// The auto detector or the user made a choice.
				// Pick a domain name which does not yet exist (after all, we
				// are *adding* a game to the config, not replacing).
				String domain(v->gameName);
				if (ConfMan.hasGameDomain(domain)) {
					char suffix = 'a';
					domain += suffix;
					while (ConfMan.hasGameDomain(domain)) {
						domain.deleteLastChar();
						suffix++;
						domain += suffix;
					}
					ConfMan.set("gameid", v->gameName, domain);
					ConfMan.set("description", v->description, domain);
				}
				ConfMan.set("path", dir->path(), domain);
				
				// Display edit dialog for the new entry
				EditGameDialog editDialog(_gui, domain, v);
				if (editDialog.runModal()) {
					// User pressed OK, so make changes permanent

					// Write config to disk
					ConfMan.flushToDisk();
					
					// Update the ListWidget and force a redraw
					updateListing();
					draw();
				} else {
					// User aborted, remove the the new domain again
					ConfMan.removeGameDomain(domain);
				}
			}
		}
		break;
	case kRemoveGameCmd:
		// Remove the currently selected game from the list
		assert(item >= 0);
		ConfMan.removeGameDomain(_domains[item]);

		// Write config to disk
		ConfMan.flushToDisk();
		
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
		String gameId(ConfMan.get("gameid", _domains[item]));
		if (gameId.isEmpty())
			gameId = _domains[item];
		EditGameDialog editDialog(_gui, _domains[item], _detector.findGame(gameId));
		if (editDialog.runModal()) {
			// User pressed OK, so make changes permanent

			// Write config to disk
			ConfMan.flushToDisk();
			
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
	case kAboutCmd: {
		AboutDialog about(_gui);
		about.runModal();
		}
		break;
	case kStartCmd:
	case kListItemDoubleClickedCmd:
		// Print out what was selected
		assert(item >= 0);
		_detector.setTarget(_domains[item]);
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
