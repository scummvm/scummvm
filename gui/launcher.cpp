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

#include "common/stdafx.h"

#include "backends/fs/fs.h"

#include "base/engine.h"
#include "base/gameDetector.h"
#include "base/plugins.h"
#include "base/version.h"

#include "common/config-manager.h"
#include "common/util.h"
#include "common/system.h"

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
	kCmdGlobalMIDIOverride = 'OMID',
	kCmdGlobalVolumeOverride = 'OVOL',

	kCmdChooseSoundFontCmd = 'chsf',

	kCmdExtraBrowser = 'PEXT',
	kCmdGameBrowser = 'PGME',
	kCmdSaveBrowser = 'PSAV'
};

/*
 * TODO: Clean up this ugly design: we subclass EditTextWidget to perform
 * input validation. It would be much more elegant to use a decorator pattern,
 * or a validation callback, or something like that.
 */
class DomainEditTextWidget : public EditTextWidget {
public:
	DomainEditTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, WidgetSize ws = kNormalWidgetSize)
		: EditTextWidget(boss, x, y, w, h, text, ws) {
	}

protected:
	bool tryInsertChar(char c, int pos) {
		if (isalnum(c) || c == '-' || c == '_') {
			_editString.insertChar(c, pos);
			return true;
		}
		return false;
	}
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
	DomainEditTextWidget *_domainWidget;

	StaticTextWidget *_gamePathWidget;
	StaticTextWidget *_extraPathWidget;
	StaticTextWidget *_savePathWidget;

	PopUpWidget *_langPopUp;
	PopUpWidget *_platformPopUp;

	CheckboxWidget *_globalGraphicsOverride;
	CheckboxWidget *_globalAudioOverride;
	CheckboxWidget *_globalMIDIOverride;
	CheckboxWidget *_globalVolumeOverride;
};

EditGameDialog::EditGameDialog(const String &domain, GameSettings target)
	: OptionsDialog(domain, 10, 40, 320 - 2 * 10, 140) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_w = screenW - 2 * 10;

	GUI::WidgetSize ws;
	int buttonHeight;
	int buttonWidth;
	int labelWidth;

	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
		_h = screenH - 2 * 40;	// TODO/FIXME
		buttonHeight = kBigButtonHeight;
		buttonWidth = kBigButtonWidth;
		labelWidth = 90;
	} else {
		ws = GUI::kNormalWidgetSize;
		_h = screenH - 2 * 30;	// TODO/FIXME
		buttonHeight = kButtonHeight;
		buttonWidth = kButtonWidth;
		labelWidth = 60;
	}

	const int x = 5;
	const int w = _w - 15;
	const int vBorder = 5;	// Tab border
	int yoffset;

	// GAME: Path to game data (r/o), extra data (r/o), and save data (r/w)
	String gamePath(ConfMan.get("path", _domain));
	String extraPath(ConfMan.get("extrapath", _domain));
	String savePath(ConfMan.get("savepath", _domain));

	// GAME: Determine the description string
	String description(ConfMan.get("description", domain));
	if (description.isEmpty() && target.description) {
		description = target.description;
	}

	// GUI:  Add tab widget
	TabWidget *tab = new TabWidget(this, 0, vBorder, _w, _h - buttonHeight - 8 - 2 * vBorder, ws);

	//
	// 1) The game tab
	//
	tab->addTab("Game");
	yoffset = vBorder;

	// GUI:  Label & edit widget for the game ID
	new StaticTextWidget(tab, x, yoffset + 2, labelWidth, kLineHeight, "ID: ", kTextAlignRight, ws);
	_domainWidget = new DomainEditTextWidget(tab, x + labelWidth, yoffset, _w - labelWidth - 10 - x, kLineHeight, _domain, ws);
	yoffset += _domainWidget->getHeight() + 3;

	// GUI:  Label & edit widget for the description
	new StaticTextWidget(tab, x, yoffset + 2, labelWidth, kLineHeight, "Name: ", kTextAlignRight, ws);
	_descriptionWidget = new EditTextWidget(tab, x + labelWidth, yoffset, _w - labelWidth - 10 - x, kLineHeight, description, ws);
	yoffset += _descriptionWidget->getHeight() + 3;

	// Language popup
	_langPopUp = addPopUp(tab, x, yoffset, w, "Language: ", labelWidth, ws);
	yoffset += _langPopUp->getHeight() + 4;
	_langPopUp->appendEntry("<default>");
	_langPopUp->appendEntry("");
	const Common::LanguageDescription *l = Common::g_languages;
	for (; l->code; ++l) {
		_langPopUp->appendEntry(l->description, l->id);
	}

	// Platform popup
	_platformPopUp = addPopUp(tab, x, yoffset, w, "Platform: ", labelWidth, ws);
	yoffset += _platformPopUp->getHeight() + 4;
	_platformPopUp->appendEntry("<default>");
	_platformPopUp->appendEntry("");
	const Common::PlatformDescription *p = Common::g_platforms;
	for (; p->code; ++p) {
		_platformPopUp->appendEntry(p->description, p->id);
	}

	// 2) The 'Path' tab
	tab->addTab("Paths");
	yoffset = vBorder;

	// These buttons have to be extra wide, or the text will be truncated
	// in the small version of the GUI.

	// GUI:  Button + Label for the game path
	new ButtonWidget(tab, x, yoffset, buttonWidth + 5, buttonHeight, "Game Path: ", kCmdGameBrowser, 0, ws);
	_gamePathWidget = new StaticTextWidget(tab, x + buttonWidth + 20, yoffset + 3, _w - (x + buttonWidth + 20) - 10, kLineHeight, gamePath, kTextAlignLeft, ws);
	yoffset += buttonHeight + 4;

	// GUI:  Button + Label for the additional path
	new ButtonWidget(tab, x, yoffset, buttonWidth + 5, buttonHeight, "Extra Path:", kCmdExtraBrowser, 0, ws);
	_extraPathWidget = new StaticTextWidget(tab, x + buttonWidth + 20, yoffset + 3, _w - (x + buttonWidth + 20) - 10, kLineHeight, extraPath, kTextAlignLeft, ws);
	if (extraPath.isEmpty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPathWidget->setLabel("None");
	}
	yoffset += buttonHeight + 4;

	// GUI:  Button + Label for the save path
	new ButtonWidget(tab, x, yoffset, buttonWidth + 5, buttonHeight, "Save Path: ", kCmdSaveBrowser, 0, ws);
	_savePathWidget = new StaticTextWidget(tab, x + buttonWidth + 20, yoffset + 3, _w - (x + buttonWidth + 20) - 10, kLineHeight, savePath, kTextAlignLeft, ws);
	if (savePath.isEmpty() || !ConfMan.hasKey("savepath", _domain)) {
		_savePathWidget->setLabel("Default");
	}
	yoffset += buttonHeight + 4;

	//
	// 3) The graphics tab
	//
	tab->addTab("Gfx");
	yoffset = vBorder;

	_globalGraphicsOverride = addCheckbox(tab, x, yoffset, "Override global graphic settings", kCmdGlobalGraphicsOverride, 0, ws);
	yoffset += _globalGraphicsOverride->getHeight();

	yoffset = addGraphicControls(tab, yoffset, ws);

	//
	// 4) The audio tab
	//
	tab->addTab("Audio");
	yoffset = vBorder;

	_globalAudioOverride = addCheckbox(tab, x, yoffset, "Override global audio settings", kCmdGlobalAudioOverride, 0, ws);
	yoffset += _globalAudioOverride->getHeight();

	yoffset = addAudioControls(tab, yoffset, ws);

	//
	// 5) The MIDI tab
	//
	tab->addTab("MIDI");
	yoffset = vBorder;

	_globalMIDIOverride = addCheckbox(tab, x, yoffset, "Override global MIDI settings", kCmdGlobalMIDIOverride, 0, ws);
	yoffset += _globalMIDIOverride->getHeight();

	yoffset = addMIDIControls(tab, yoffset, ws);

	//
	// 6) The volume tab
	//
	tab->addTab("Volume");
	yoffset = vBorder;

	_globalVolumeOverride = addCheckbox(tab, x, yoffset, "Override global volume settings", kCmdGlobalVolumeOverride, 0, ws);
	yoffset += _globalVolumeOverride->getHeight();

	yoffset = addVolumeControls(tab, yoffset, ws);


	// Activate the first tab
	tab->setActiveTab(0);

	// Add OK & Cancel buttons
	addButton(this, _w - 2 * (buttonWidth + 10), _h - buttonHeight - 8, "Cancel", kCloseCmd, 0, ws);
	addButton(this, _w - (buttonWidth + 10), _h - buttonHeight - 8, "OK", kOKCmd, 0, ws);
}

void EditGameDialog::open() {
	OptionsDialog::open();

	int sel, i;
	bool e;

	// En-/disable dialog items depending on whether overrides are active or not.

	e = ConfMan.hasKey("fullscreen", _domain) ||
		ConfMan.hasKey("aspect_ratio", _domain);
	_globalGraphicsOverride->setState(e);

	e = ConfMan.hasKey("music_driver", _domain) ||
		ConfMan.hasKey("subtitles", _domain);
	_globalAudioOverride->setState(e);

	e = ConfMan.hasKey("multi_midi", _domain) ||
		ConfMan.hasKey("native_mt32", _domain)||
		ConfMan.hasKey("enable_gs", _domain);
	_globalMIDIOverride->setState(e);

	e = ConfMan.hasKey("music_volume", _domain) ||
		ConfMan.hasKey("sfx_volume", _domain) ||
		ConfMan.hasKey("speech_volume", _domain);
	_globalVolumeOverride->setState(e);

	// TODO: game path

	const Common::LanguageDescription *l = Common::g_languages;
	const Common::Language lang = Common::parseLanguage(ConfMan.get("language", _domain));
	sel = 0;
	for (i = 0; l->code; ++l, ++i) {
		if (lang == l->id)
			sel = i + 2;
	}
	_langPopUp->setSelected(sel);


	const Common::PlatformDescription *p = Common::g_platforms;
	const Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", _domain));
	sel = 0;
	for (i = 0; p->code; ++p, ++i) {
		if (platform == p->id)
			sel = i + 2;
	}
	_platformPopUp->setSelected(sel);
}


void EditGameDialog::close() {
	if (getResult()) {
		ConfMan.set("description", _descriptionWidget->getEditString(), _domain);

		Common::Language lang = (Common::Language)_langPopUp->getSelectedTag();
		if (lang < 0)
			ConfMan.removeKey("language", _domain);
		else
			ConfMan.set("language", Common::getLanguageCode(lang), _domain);

		String gamePath = _gamePathWidget->getLabel();
		if (!gamePath.isEmpty())
			ConfMan.set("path", gamePath, _domain);

		String extraPath = _extraPathWidget->getLabel();
		if (!extraPath.isEmpty() && (extraPath != "None"))
			ConfMan.set("extrapath", extraPath, _domain);

		String savePath = _savePathWidget->getLabel();
		if (!savePath.isEmpty() && (savePath != "Default"))
			ConfMan.set("savepath", savePath, _domain);

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
	case kCmdGlobalMIDIOverride:
		setMIDISettingsState(data != 0);
		draw();
		break;
	case kCmdGlobalVolumeOverride:
		setVolumeSettingsState(data != 0);
		draw();
		break;

	case kCmdChooseSoundFontCmd: {
		BrowserDialog browser("Select SoundFont", false);

		if (browser.runModal() > 0) {
			// User made this choice...
			FilesystemNode file(browser.getResult());
			_soundFont->setLabel(file.path());
			draw();
		}
		break;
	}

	// Change path for the game
	case kCmdGameBrowser: {
		BrowserDialog browser("Select directory with game data", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());

			// TODO: Verify the game can be found in the new directory... Best
			// done with optional specific gameid to pluginmgr detectgames?
			// FSList files = dir.listDir(FilesystemNode::kListFilesOnly);

			_gamePathWidget->setLabel(dir.path());
			draw();
		}
		draw();
		break;
	}

	// Change path for extra game data (eg, using sword cutscenes when playing via CD)
	case kCmdExtraBrowser: {
		BrowserDialog browser("Select additional game directory", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());
			_extraPathWidget->setLabel(dir.path());
			draw();
		}
		draw();
		break;
	}
	// Change path for stored save game (perm and temp) data
	case kCmdSaveBrowser: {
		BrowserDialog browser("Select directory for saved games", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			FilesystemNode dir(browser.getResult());
			_savePathWidget->setLabel(dir.path());
			draw();
		}
		draw();
		break;
	}

	case kOKCmd: {
		// Write back changes made to config object
		String newDomain(_domainWidget->getEditString());
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

#define BEGIN_BUTTONS(numButtons, hSpace, top) \
	{ \
		const int space = hSpace; \
		const int width = (_w - 2 * hBorder - space * (numButtons - 1)) / numButtons; \
		int x = hBorder; \
		const int y = top;

#define ADD(name, cmd, hotkey) \
	new ButtonWidget(this, x, y, width, buttonHeight, name, cmd, hotkey, ws); x += space + width

#define END_BUTTONS \
	}

LauncherDialog::LauncherDialog(GameDetector &detector)
	: Dialog(0, 0, 320, 200), _detector(detector) {
	_mainDialog = true;

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	const int hBorder = 10;

	_w = screenW;
	_h = screenH;

	GUI::WidgetSize ws;
	int buttonHeight;
	int top;

	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
		buttonHeight = kBigButtonHeight;
	} else {
		ws = GUI::kNormalWidgetSize;
		buttonHeight = kButtonHeight;
	}

	// Show ScummVM version
	new StaticTextWidget(this, hBorder, 8, _w - 2*hBorder, kLineHeight, gScummVMFullVersion, kTextAlignCenter, ws);

	// Add some buttons at the bottom
	// TODO: Rearrange them a bit? In particular, we could put a slightly smaller space
	// between About and Options, and in exchange remove those a bit from Quit and Start.
	top = _h - 8 - buttonHeight;
	BEGIN_BUTTONS(4, 8, top)
		ADD("Quit", kQuitCmd, 'Q');
		ADD("About", kAboutCmd, 'B');
		ADD("Options", kOptionsCmd, 'O');
		_startButton =
		ADD("Start", kStartCmd, 'S');
	END_BUTTONS

	// Above the lowest button rows: two more buttons (directly below the list box)
	top -= 2 * buttonHeight;
	BEGIN_BUTTONS(3, 10, top)
		ADD("Add Game...", kAddGameCmd, 'A');
		_editButton =
		ADD("Edit Game...", kEditGameCmd, 'E');
		_removeButton =
		ADD("Remove Game", kRemoveGameCmd, 'R');
	END_BUTTONS


	// Add list with game titles
	_list = new ListWidget(this, hBorder, kLineHeight + 16, _w - 2 * hBorder, top - kLineHeight - 20, ws);
	_list->setEditable(false);
	_list->setNumberingMode(kListNumberingOff);


	// Populate the list
	updateListing();

	// Restore last selection
	String last = ConfMan.get(String("lastselectedgame"), ConfigManager::kApplicationDomain);
	selectGame(last);

	// En-/disable the buttons depending on the list selection
	updateButtons();

	// Create file browser dialog
	_browser = new BrowserDialog("Select directory with game data", true);
}

void LauncherDialog::selectGame(const String &name) {
	if (!name.isEmpty()) {
		int itemToSelect = 0;
		StringList::const_iterator iter;
		for (iter = _domains.begin(); iter != _domains.end(); ++iter, ++itemToSelect) {
			if (name == *iter) {
				_list->setSelected(itemToSelect);
				break;
			}
		}
	}
}

LauncherDialog::~LauncherDialog() {
	delete _browser;
}

void LauncherDialog::close() {
	// Save last selection
	const int sel = _list->getSelected();
	if (sel >= 0)
		ConfMan.set(String("lastselectedgame"), _domains[sel], ConfigManager::kApplicationDomain);
	else
		ConfMan.removeKey(String("lastselectedgame"), ConfigManager::kApplicationDomain);

	ConfMan.flushToDisk();
	Dialog::close();
}

void LauncherDialog::updateListing() {
	Common::StringList l;

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	ConfigManager::DomainMap::const_iterator iter = domains.begin();
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
		String gameid(iter->_value.get("gameid"));
		String description(iter->_value.get("description"));

		if (gameid.isEmpty())
			gameid = iter->_key;
		if (description.isEmpty()) {
			GameSettings g = GameDetector::findGame(gameid);
			if (g.description)
				description = g.description;
		}

		if (!gameid.isEmpty() && !description.isEmpty()) {
			// Insert the game into the launcher list
			int pos = 0, size = l.size();

			while (pos < size && (scumm_stricmp(description.c_str(), l[pos].c_str()) > 0))
				pos++;
			l.insert_at(pos, description);
			_domains.insert_at(pos, iter->_key);
		}
	}

	const int oldSel = _list->getSelected();
	_list->setList(l);
	if (oldSel < (int)l.size())
		_list->setSelected(oldSel);	// Restore the old selection
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

	if (_browser->runModal() > 0) {
		// User made his choice...
		FilesystemNode dir(_browser->getResult());
		FSList files = dir.listDir(FilesystemNode::kListAll);

		// ...so let's determine a list of candidates, games that
		// could be contained in the specified directory.
		DetectedGameList candidates(PluginManager::instance().detectGames(files));

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
			for (idx = 0; idx < (int)candidates.size(); idx++)
				list.push_back(candidates[idx].description);

			ChooserDialog dialog("Pick the game:");
			dialog.setList(list);
			idx = dialog.runModal();
		}
		if (0 <= idx && idx < (int)candidates.size()) {
			DetectedGame result = candidates[idx];

			// The auto detector or the user made a choice.
			// Pick a domain name which does not yet exist (after all, we
			// are *adding* a game to the config, not replacing).
			String domain(result.gameid);
			if (ConfMan.hasGameDomain(domain)) {
				char suffix = 'a';
				domain += suffix;
				while (ConfMan.hasGameDomain(domain)) {
					assert(suffix < 'z');
					domain.deleteLastChar();
					suffix++;
					domain += suffix;
				}
				ConfMan.set("description", result.description, domain);
			}
			ConfMan.set("gameid", result.gameid, domain);
			ConfMan.set("path", dir.path(), domain);

			const bool customLanguage = (result.language != Common::UNK_LANG);
			const bool customPlatform = (result.platform != Common::kPlatformUnknown);

			// Set language if specified
			if (customLanguage)
				ConfMan.set("language", Common::getLanguageCode(result.language), domain);

			// Set platform if specified
			if (customPlatform)
				ConfMan.set("platform", Common::getPlatformCode(result.platform), domain);

			// Adapt the description string if custom platform/language is set
			if (customLanguage || customPlatform) {
				String desc = result.description;
				desc += " (";
				if (customLanguage)
					desc += Common::getLanguageDescription(result.language);
				if (customLanguage && customPlatform)
					desc += "/";
				if (customPlatform)
					desc += Common::getPlatformDescription(result.platform);
				desc += ")";

				ConfMan.set("description", desc, domain);
			}

			// Display edit dialog for the new entry
			EditGameDialog editDialog(domain, result);
			if (editDialog.runModal() > 0) {
				// User pressed OK, so make changes permanent

				// Write config to disk
				ConfMan.flushToDisk();

				// Update the ListWidget, select the new item, and force a redraw
				updateListing();
				selectGame(domain);
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

	if (alert.runModal() == GUI::kMessageOK) {
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
	if (editDialog.runModal() > 0) {
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
		GlobalOptionsDialog options;
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
		setResult(-1);
		close();
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
