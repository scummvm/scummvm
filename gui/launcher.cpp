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

#include "backends/fs/fs.h"

#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/util.h"

#include "gui/about.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/launcher.h"
#include "gui/message.h"
#include "gui/newgui.h"
#include "gui/options.h"
#include "gui/EditTextWidget.h"
#include "gui/ListWidget.h"
#include "gui/TabWidget.h"
#include "gui/PopUpWidget.h"

#include "sound/mididrv.h"


using Common::ConfigManager;

namespace GUI {

enum {
	kStartCmd = 'STRT',
	kAboutCmd = 'ABOU',
	kOptionsCmd = 'OPTN',
	kAddGameCmd = 'ADDG',
	kEditGameCmd = 'EDTG',
	kRemoveGameCmd = 'REMG',
	kQuitCmd = 'QUIT',
	
	
	kCmdGlobalGraphicsOverride = 'OGFX',
	kCmdGlobalAudioOverride = 'OSFX',
	kCmdGlobalVolumeOverride = 'OVOL'
};

/*
 * A dialog that allows the user to edit a config game entry.
 * TODO: add widgets for some/all of the following
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
 */

class EditGameDialog : public OptionsDialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
public:
	EditGameDialog(const String &domain, GameSettings target);

	void open();
	void close();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	EditTextWidget *_descriptionWidget;
	EditTextWidget *_domainWidget;

	PopUpWidget *_langPopUp;
	PopUpWidget *_platformPopUp;

	CheckboxWidget *_globalGraphicsOverride;
	CheckboxWidget *_globalAudioOverride;
	CheckboxWidget *_globalVolumeOverride;
};

EditGameDialog::EditGameDialog(const String &domain, GameSettings target)
	: OptionsDialog(domain, 10, 40, 320 - 2 * 10, 140) {

	const int x = 5;
	const int w = _w - 15;
	const int labelWidth = 65;
	const int vBorder = 5;	// Tab border
	int yoffset;

	// GAME: Path to game data (r/o)
	String path(ConfMan.get("path", _domain));

	// GAME: Determine the description string
	String description(ConfMan.get("description", domain));
	if (description.isEmpty() && target.description) {
		description = target.description;
	}

	// GUI:  Add tab widget
	TabWidget *tab = new TabWidget(this, 0, vBorder, _w, _h - 24 - 2*vBorder);

	//
	// 1) The game tab
	//
	tab->addTab("Game");
	yoffset = vBorder;

	// GUI:  Label & edit widget for the game ID
	new StaticTextWidget(tab, x, yoffset + 2, labelWidth, kLineHeight, "ID: ", kTextAlignRight);
	_domainWidget = new EditTextWidget(tab, x + labelWidth, yoffset, _w - labelWidth - 10, kLineHeight, _domain);
	yoffset += 16;

	// GUI:  Label & edit widget for the description
	new StaticTextWidget(tab, x, yoffset + 2, labelWidth, kLineHeight, "Name: ", kTextAlignRight);
	_descriptionWidget = new EditTextWidget(tab, x + labelWidth, yoffset, _w - labelWidth - 10, kLineHeight, description);
	yoffset += 16;

	// GUI:  Label for the game path
	new StaticTextWidget(tab, x, yoffset, labelWidth, kLineHeight, "Path: ", kTextAlignRight);
	new StaticTextWidget(tab, x + labelWidth, yoffset, _w - labelWidth - 10, kLineHeight, path, kTextAlignLeft);
	yoffset += 16;

	// Languag popup
	_langPopUp = new PopUpWidget(tab, x, yoffset, w, kLineHeight, "Language: ", labelWidth);
	yoffset += 16;
	_langPopUp->appendEntry("<default>");
	_langPopUp->appendEntry("");
	const Common::LanguageDescription *l = Common::g_languages;
	for (; l->code; ++l) {
		_langPopUp->appendEntry(l->description, l->id);
	}

	// Platform popup
	_platformPopUp = new PopUpWidget(tab, x, yoffset, w, kLineHeight, "Platform: ", labelWidth);
	yoffset += 16;
	_platformPopUp->appendEntry("<default>");
	_platformPopUp->appendEntry("");
	_platformPopUp->appendEntry("Amiga", Common::kPlatformAmiga);
	_platformPopUp->appendEntry("Atari ST", Common::kPlatformAtariST);
	_platformPopUp->appendEntry("Macintosh", Common::kPlatformMacintosh);
	_platformPopUp->appendEntry("PC", Common::kPlatformPC);

	//
	// 2) The graphics tab
	//
	tab->addTab("Graphics");
	yoffset = vBorder;

	_globalGraphicsOverride = new CheckboxWidget(tab, x, yoffset, w, 16, "Override global graphic settings", kCmdGlobalGraphicsOverride);
	yoffset += 16;

	yoffset = addGraphicControls(tab, yoffset);

	//
	// 3) The audio tab
	//
	tab->addTab("Audio");
	yoffset = vBorder;

	_globalAudioOverride = new CheckboxWidget(tab, x, yoffset, w, 16, "Override global audio settings", kCmdGlobalAudioOverride);
	yoffset += 16;

	yoffset = addMIDIControls(tab, yoffset);

	//
	// 3) The volume tab
	//
	tab->addTab("Volume");
	yoffset = vBorder;

	_globalVolumeOverride = new CheckboxWidget(tab, x, yoffset, w, 16, "Override global volume settings", kCmdGlobalVolumeOverride);
	yoffset += 16;

	yoffset = addVolumeControls(tab, yoffset);


	// Activate the first tab
	tab->setActiveTab(0);

	// Add OK & Cancel buttons
	addButton(_w - 2 * (kButtonWidth + 10), _h - 24, "Cancel", kCloseCmd, 0);
	addButton(_w - (kButtonWidth + 10), _h - 24, "OK", kOKCmd, 0);
}

void EditGameDialog::open() {
	OptionsDialog::open();

	// En-/disable dialog items depending on whether overrides are active or not.
	bool e;

	e = ConfMan.hasKey("fullscreen", _domain) ||
		ConfMan.hasKey("aspect_ratio", _domain);
	_globalGraphicsOverride->setState(e);

	e = ConfMan.hasKey("music_driver", _domain) ||
		ConfMan.hasKey("multi_midi", _domain) ||
		ConfMan.hasKey("native_mt32", _domain);
	_globalAudioOverride->setState(e);

	e = ConfMan.hasKey("master_volume", _domain) ||
		ConfMan.hasKey("music_volume", _domain) ||
		ConfMan.hasKey("sfx_volume", _domain);
	_globalVolumeOverride->setState(e);

	int sel = 0;
	
	// TODO: game path

	const Common::LanguageDescription *l = Common::g_languages;
	int lang = Common::parseLanguage(ConfMan.get("language", _domain));
	for (int i = 0; l->code; ++l, ++i) {
		if (lang == l->id)
			sel = i + 2;
	}
	_langPopUp->setSelected(sel);


	switch (Common::parsePlatform(ConfMan.get("platform", _domain))) {
	case Common::kPlatformPC:			sel = 5; break;
	case Common::kPlatformAmiga:		sel = 2; break;
	case Common::kPlatformAtariST:		sel = 3; break;
	case Common::kPlatformMacintosh:	sel = 4; break;
	default:							sel = 0; break;
	}
	_platformPopUp->setSelected(sel);
}


void EditGameDialog::close() {
	if (getResult()) {
		ConfMan.set("description", _descriptionWidget->getLabel(), _domain);

		Common::Language lang = (Common::Language)_langPopUp->getSelectedTag();
		if (lang < 0)
			ConfMan.removeKey("language", _domain);
		else
			ConfMan.set("language", Common::getLanguageCode(lang), _domain);

		Common::Platform platform = (Common::Platform)_platformPopUp->getSelectedTag();
		if (platform < 0)
			ConfMan.removeKey("platform", _domain);
		else
			ConfMan.set("platform", Common::getPlatformCode(platform), _domain);
	}
	OptionsDialog::close();
}

void EditGameDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kCmdGlobalGraphicsOverride:
		setGraphicSettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalAudioOverride:
		setAudioSettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalVolumeOverride:
		setVolumeSettingsState(data != 0);
		draw();
		break;
	case kOKCmd: {
		// Write back changes made to config object
		String newDomain(_domainWidget->getLabel());
		if (newDomain != _domain) {
			if (newDomain.isEmpty() || ConfMan.hasGameDomain(newDomain)) {
				MessageDialog alert("This game ID is already taken. Please choose another one.");
				alert.runModal();
				return;
			}
			ConfMan.renameGameDomain(_domain, newDomain);
			_domain = newDomain;
		}
		}
		// FALL THROUGH to default case
	default:
		OptionsDialog::handleCommand(sender, cmd, data);
	}
}


#pragma mark -


LauncherDialog::LauncherDialog(GameDetector &detector)
	: Dialog(0, 0, 320, 200), _detector(detector) {
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
	_browser = new BrowserDialog("Select directory with game data");
}

LauncherDialog::~LauncherDialog() {
	delete _browser;
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
			GameSettings g = GameDetector::findGame(name);
			if (g.description)
				description = g.description;
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

void LauncherDialog::addGame() {
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
		FSList *files = dir->listDir(FilesystemNode::kListFilesOnly);

		// ...so let's determine a list of candidates, games that
		// could be contained in the specified directory.
		DetectedGameList candidates(PluginManager::instance().detectGames(*files));

		int idx;
		if (candidates.isEmpty()) {
			// No game was found in the specified directory
			MessageDialog alert("ScummVM could not find any game in the specified directory!");
			alert.runModal();
			idx = -1;
		} else if (candidates.size() == 1) {
			// Exact match
			idx = 0;
		} else {
			// Display the candidates to the user and let her/him pick one
			StringList list;
			for (idx = 0; idx < candidates.size(); idx++)
				list.push_back(candidates[idx].description);
			
			ChooserDialog dialog("Pick the game:");
			dialog.setList(list);
			idx = dialog.runModal();
		}
		if (0 <= idx && idx < candidates.size()) {
			DetectedGame result = candidates[idx];

			// The auto detector or the user made a choice.
			// Pick a domain name which does not yet exist (after all, we
			// are *adding* a game to the config, not replacing).
			String domain(result.name);
			if (ConfMan.hasGameDomain(domain)) {
				char suffix = 'a';
				domain += suffix;
				while (ConfMan.hasGameDomain(domain)) {
					assert(suffix < 'z');
					domain.deleteLastChar();
					suffix++;
					domain += suffix;
				}
				ConfMan.set("gameid", result.name, domain);
				ConfMan.set("description", result.description, domain);
			}
			ConfMan.set("path", dir->path(), domain);

			// Set language if specified
			if (result.language != Common::UNK_LANG)
				ConfMan.set("language", Common::getLanguageCode(result.language), domain);

			// Set platform if specified
			if (result.platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(result.platform), domain);

			// Display edit dialog for the new entry
			EditGameDialog editDialog(domain, result);
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
}

void LauncherDialog::removeGame(int item) {
	MessageDialog alert("Do you really want to remove this game configuration?", "Yes", "No");
	
	if (alert.runModal() == 1) {
		// Remove the currently selected game from the list
		assert(item >= 0);
		ConfMan.removeGameDomain(_domains[item]);

		// Write config to disk
		ConfMan.flushToDisk();
		
		// Update the ListWidget and force a redraw
		updateListing();
		draw();
	}
}

void LauncherDialog::editGame(int item) {
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
	EditGameDialog editDialog(_domains[item], GameDetector::findGame(gameId));
	if (editDialog.runModal()) {
		// User pressed OK, so make changes permanent

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget and force a redraw
		updateListing();
		draw();
	}
}

void LauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int item = _list->getSelected();

	switch (cmd) {
	case kAddGameCmd:
		addGame();
		break;
	case kRemoveGameCmd:
		removeGame(item);
		break;
	case kEditGameCmd:
		editGame(item);
		break;
	case kOptionsCmd: {
		GlobalOptionsDialog options(_detector);
		options.runModal();
		}
		break;
	case kAboutCmd: {
		AboutDialog about;
		about.runModal();
		}
		break;
	case kStartCmd:
	case kListItemActivatedCmd:
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

} // End of namespace GUI
