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

#include "base/version.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/util.h"
#include "common/savefile.h"
#include "common/system.h"

#include "gui/about.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/launcher.h"
#include "gui/massadd.h"
#include "gui/message.h"
#include "gui/GuiManager.h"
#include "gui/options.h"
#include "gui/saveload.h"
#include "gui/EditTextWidget.h"
#include "gui/ListWidget.h"
#include "gui/TabWidget.h"
#include "gui/PopUpWidget.h"
#include "gui/ThemeEval.h"

#include "graphics/cursorman.h"

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
	kLoadGameCmd = 'LOAD',
	kQuitCmd = 'QUIT',
	kSearchCmd = 'SRCH',
	kListSearchCmd = 'LSSR',
	kSearchClearCmd = 'SRCL',

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
	DomainEditTextWidget(GuiObject *boss, const String &name, const String &text)
		: EditTextWidget(boss, name, text) {
	}

protected:
	bool tryInsertChar(byte c, int pos) {
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
	EditGameDialog(const String &domain, const String &desc);

	void open();
	void close();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	EditTextWidget *_descriptionWidget;
	DomainEditTextWidget *_domainWidget;

	StaticTextWidget *_gamePathWidget;
	StaticTextWidget *_extraPathWidget;
	StaticTextWidget *_savePathWidget;

	StaticTextWidget *_langPopUpDesc;
	PopUpWidget *_langPopUp;
	StaticTextWidget *_platformPopUpDesc;
	PopUpWidget *_platformPopUp;

	CheckboxWidget *_globalGraphicsOverride;
	CheckboxWidget *_globalAudioOverride;
	CheckboxWidget *_globalMIDIOverride;
	CheckboxWidget *_globalVolumeOverride;
};

EditGameDialog::EditGameDialog(const String &domain, const String &desc)
	: OptionsDialog(domain, "GameOptions") {

	// GAME: Path to game data (r/o), extra data (r/o), and save data (r/w)
	String gamePath(ConfMan.get("path", _domain));
	String extraPath(ConfMan.get("extrapath", _domain));
	String savePath(ConfMan.get("savepath", _domain));

	// GAME: Determine the description string
	String description(ConfMan.get("description", domain));
	if (description.empty() && !desc.empty()) {
		description = desc;
	}

	// GUI:  Add tab widget
	TabWidget *tab = new TabWidget(this, "GameOptions.TabWidget");

	//
	// 1) The game tab
	//
	tab->addTab("Game");

	// GUI:  Label & edit widget for the game ID
	new StaticTextWidget(tab, "GameOptions_Game.Id", "ID:");
	_domainWidget = new DomainEditTextWidget(tab, "GameOptions_Game.Domain", _domain);

	// GUI:  Label & edit widget for the description
	new StaticTextWidget(tab, "GameOptions_Game.Name", "Name:");
	_descriptionWidget = new EditTextWidget(tab, "GameOptions_Game.Desc", description);

	// Language popup
	_langPopUpDesc = new StaticTextWidget(tab, "GameOptions_Game.LangPopupDesc", "Language:");
	_langPopUp = new PopUpWidget(tab, "GameOptions_Game.LangPopup");
	_langPopUp->appendEntry("<default>");
	_langPopUp->appendEntry("");
	const Common::LanguageDescription *l = Common::g_languages;
	for (; l->code; ++l) {
		_langPopUp->appendEntry(l->description, l->id);
	}

	// Platform popup
	_platformPopUpDesc = new StaticTextWidget(tab, "GameOptions_Game.PlatformPopupDesc", "Platform:");
	_platformPopUp = new PopUpWidget(tab, "GameOptions_Game.PlatformPopup");
	_platformPopUp->appendEntry("<default>");
	_platformPopUp->appendEntry("");
	const Common::PlatformDescription *p = Common::g_platforms;
	for (; p->code; ++p) {
		_platformPopUp->appendEntry(p->description, p->id);
	}

	//
	// 3) The graphics tab
	//
	_graphicsTabId = tab->addTab(g_system->getOverlayWidth() > 320 ? "Graphics" : "GFX");

	_globalGraphicsOverride = new CheckboxWidget(tab, "GameOptions_Graphics.EnableTabCheckbox", "Override global graphic settings", kCmdGlobalGraphicsOverride, 0);

	addGraphicControls(tab, "GameOptions_Graphics.");

	//
	// 4) The audio tab
	//
	tab->addTab("Audio");

	_globalAudioOverride = new CheckboxWidget(tab, "GameOptions_Audio.EnableTabCheckbox", "Override global audio settings", kCmdGlobalAudioOverride, 0);

	addAudioControls(tab, "GameOptions_Audio.");
	addSubtitleControls(tab, "GameOptions_Audio.");

	//
	// 5) The volume tab
	//
	tab->addTab("Volume");

	_globalVolumeOverride = new CheckboxWidget(tab, "GameOptions_Volume.EnableTabCheckbox", "Override global volume settings", kCmdGlobalVolumeOverride, 0);

	addVolumeControls(tab, "GameOptions_Volume.");

	//
	// 6) The MIDI tab
	//
	tab->addTab("MIDI");

	_globalMIDIOverride = new CheckboxWidget(tab, "GameOptions_MIDI.EnableTabCheckbox", "Override global MIDI settings", kCmdGlobalMIDIOverride, 0);

	if (_guioptions & Common::GUIO_NOMIDI)
		_globalMIDIOverride->setEnabled(false);

	addMIDIControls(tab, "GameOptions_MIDI.");

	//
	// 2) The 'Path' tab
	//
	tab->addTab("Paths");

	// These buttons have to be extra wide, or the text will be truncated
	// in the small version of the GUI.

	// GUI:  Button + Label for the game path
	new ButtonWidget(tab, "GameOptions_Paths.Gamepath", "Game Path:", kCmdGameBrowser, 0);
	_gamePathWidget = new StaticTextWidget(tab, "GameOptions_Paths.GamepathText", gamePath);

	// GUI:  Button + Label for the additional path
	new ButtonWidget(tab, "GameOptions_Paths.Extrapath", "Extra Path:", kCmdExtraBrowser, 0);
	_extraPathWidget = new StaticTextWidget(tab, "GameOptions_Paths.ExtrapathText", extraPath);

	// GUI:  Button + Label for the save path
	new ButtonWidget(tab, "GameOptions_Paths.Savepath", "Save Path:", kCmdSaveBrowser, 0);
	_savePathWidget = new StaticTextWidget(tab, "GameOptions_Paths.SavepathText", savePath);

	// Activate the first tab
	tab->setActiveTab(0);
	_tabWidget = tab;

	// Add OK & Cancel buttons
	new ButtonWidget(this, "GameOptions.Cancel", "Cancel", kCloseCmd, 0);
	new ButtonWidget(this, "GameOptions.Ok", "OK", kOKCmd, 0);
}

void EditGameDialog::open() {
	OptionsDialog::open();

	String extraPath(ConfMan.get("extrapath", _domain));
	if (extraPath.empty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPathWidget->setLabel("None");
	}

	String savePath(ConfMan.get("savepath", _domain));
	if (savePath.empty() || !ConfMan.hasKey("savepath", _domain)) {
		_savePathWidget->setLabel("Default");
	}

	int sel, i;
	bool e;

	// En-/disable dialog items depending on whether overrides are active or not.

	e = ConfMan.hasKey("gfx_mode", _domain) ||
		ConfMan.hasKey("render_mode", _domain) ||
		ConfMan.hasKey("fullscreen", _domain) ||
		ConfMan.hasKey("aspect_ratio", _domain);
	_globalGraphicsOverride->setState(e);

	e = ConfMan.hasKey("music_driver", _domain) ||
		ConfMan.hasKey("output_rate", _domain) ||
		ConfMan.hasKey("opl_driver", _domain) ||
		ConfMan.hasKey("subtitles", _domain) ||
		ConfMan.hasKey("talkspeed", _domain);
	_globalAudioOverride->setState(e);

	e = ConfMan.hasKey("music_volume", _domain) ||
		ConfMan.hasKey("sfx_volume", _domain) ||
		ConfMan.hasKey("speech_volume", _domain);
	_globalVolumeOverride->setState(e);

	e = ConfMan.hasKey("soundfont", _domain) ||
		ConfMan.hasKey("multi_midi", _domain) ||
		ConfMan.hasKey("native_mt32", _domain) ||
		ConfMan.hasKey("enable_gs", _domain) ||
		ConfMan.hasKey("midi_gain", _domain);
	_globalMIDIOverride->setState(e);

	// TODO: game path

	const Common::LanguageDescription *l = Common::g_languages;
	const Common::Language lang = Common::parseLanguage(ConfMan.get("language", _domain));

	sel = 0;
	if (ConfMan.hasKey("language", _domain)) {
		for (i = 0; l->code; ++l, ++i) {
			if (lang == l->id)
				sel = i + 2;
		}
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

		String gamePath(_gamePathWidget->getLabel());
		if (!gamePath.empty())
			ConfMan.set("path", gamePath, _domain);

		String extraPath(_extraPathWidget->getLabel());
		if (!extraPath.empty() && (extraPath != "None"))
			ConfMan.set("extrapath", extraPath, _domain);

		String savePath(_savePathWidget->getLabel());
		if (!savePath.empty() && (savePath != "Default"))
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
		setSubtitleSettingsState(data != 0);
		if (_globalVolumeOverride == NULL)
			setVolumeSettingsState(data != 0);
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
			Common::FSNode file(browser.getResult());
			_soundFont->setLabel(file.getPath());

			if (!file.getPath().empty() && (file.getPath() != "None"))
				_soundFontClearButton->setEnabled(true);
			else
				_soundFontClearButton->setEnabled(false);

			draw();
		}
		break;
	}

	// Change path for the game
	case kCmdGameBrowser: {
		BrowserDialog browser("Select directory with game data", true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());

			// TODO: Verify the game can be found in the new directory... Best
			// done with optional specific gameid to pluginmgr detectgames?
			// FSList files = dir.listDir(FSNode::kListFilesOnly);

			_gamePathWidget->setLabel(dir.getPath());
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
			Common::FSNode dir(browser.getResult());
			_extraPathWidget->setLabel(dir.getPath());
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
			Common::FSNode dir(browser.getResult());
			_savePathWidget->setLabel(dir.getPath());
			draw();
		}
		draw();
		break;
	}

	case kOKCmd: {
		// Write back changes made to config object
		String newDomain(_domainWidget->getEditString());
		if (newDomain != _domain) {
			if (newDomain.empty()
				|| newDomain.hasPrefix("_")
				|| newDomain == ConfigManager::kApplicationDomain
				|| ConfMan.hasGameDomain(newDomain)) {
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

LauncherDialog::LauncherDialog()
	: Dialog(0, 0, 320, 200) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	_w = screenW;
	_h = screenH;

#ifndef DISABLE_FANCY_THEMES
	_logo = 0;
	if (g_gui.xmlEval()->getVar("Globals.ShowLauncherLogo") == 1 && g_gui.theme()->supportsImages()) {
		_logo = new GraphicsWidget(this, "Launcher.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageLogo));

		new StaticTextWidget(this, "Launcher.Version", gScummVMVersionDate);
	} else
		new StaticTextWidget(this, "Launcher.Version", gScummVMFullVersion);
#else
	// Show ScummVM version
	new StaticTextWidget(this, "Launcher.Version", gScummVMFullVersion);
#endif

	new ButtonWidget(this, "Launcher.QuitButton", "Quit", kQuitCmd, 'Q');
	new ButtonWidget(this, "Launcher.AboutButton", "About", kAboutCmd, 'B');
	new ButtonWidget(this, "Launcher.OptionsButton", "Options", kOptionsCmd, 'O');
	_startButton =
			new ButtonWidget(this, "Launcher.StartButton", "Start", kStartCmd, 'S');

	_loadButton =
		new ButtonWidget(this, "Launcher.LoadGameButton", "Load", kLoadGameCmd, 'L');

	// Above the lowest button rows: two more buttons (directly below the list box)
	_addButton =
		new ButtonWidget(this, "Launcher.AddGameButton", "Add Game", kAddGameCmd, 'A');
	_editButton =
		new ButtonWidget(this, "Launcher.EditGameButton", "Edit Game", kEditGameCmd, 'E');
	_removeButton =
		new ButtonWidget(this, "Launcher.RemoveGameButton", "Remove Game", kRemoveGameCmd, 'R');

	// Search box
	_searchDesc = 0;
#ifndef DISABLE_FANCY_THEMES
	_searchPic = 0;
	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		_searchPic = new GraphicsWidget(this, "Launcher.SearchPic");
		_searchPic->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageSearch));
	} else
#endif
		_searchDesc = new StaticTextWidget(this, "Launcher.SearchDesc", "Search:");

	_searchWidget = new EditTextWidget(this, "Launcher.Search", _search, kSearchCmd);
	_searchClearButton = new ButtonWidget(this, "Launcher.SearchClearButton", "C", kSearchClearCmd, 0);

	// Add list with game titles
	_list = new ListWidget(this, "Launcher.GameList", kListSearchCmd);
	_list->setEditable(false);
	_list->setNumberingMode(kListNumberingOff);


	// Populate the list
	updateListing();

	// Restore last selection
	String last(ConfMan.get("lastselectedgame", ConfigManager::kApplicationDomain));
	selectGame(last);

	// En-/disable the buttons depending on the list selection
	updateButtons();

	// Create file browser dialog
	_browser = new BrowserDialog("Select directory with game data", true);

	// Create Load dialog
	_loadDialog = new SaveLoadChooser("Load game:", "Load");
}

void LauncherDialog::selectGame(const String &name) {
	if (!name.empty()) {
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
	delete _loadDialog;
}

void LauncherDialog::open() {
	// Clear the active domain, in case we return to the dialog from a
	// failure to launch a game. Otherwise, pressing ESC will attempt to
	// re-launch the same game again.
	ConfMan.setActiveDomain("");

	CursorMan.popAllCursors();
	Dialog::open();

	updateButtons();
}

void LauncherDialog::close() {
	// Save last selection
	const int sel = _list->getSelected();
	if (sel >= 0)
		ConfMan.set("lastselectedgame", _domains[sel], ConfigManager::kApplicationDomain);
	else
		ConfMan.removeKey("lastselectedgame", ConfigManager::kApplicationDomain);

	ConfMan.flushToDisk();
	Dialog::close();
}

void LauncherDialog::updateListing() {
	Common::StringList l;

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	ConfigManager::DomainMap::const_iterator iter;
	for (iter = domains.begin(); iter != domains.end(); ++iter) {
#ifdef __DS__
		// DS port uses an extra section called 'ds'.  This prevents the section from being
		// detected as a game.
		if (iter->_key == "ds") {
			continue;
		}
#endif

		String gameid(iter->_value.get("gameid"));
		String description(iter->_value.get("description"));

		if (gameid.empty())
			gameid = iter->_key;
		if (description.empty()) {
			GameDescriptor g = EngineMan.findGame(gameid);
			if (g.contains("description"))
				description = g.description();
		}

		if (description.empty())
			description = "Unknown (target " + iter->_key + ", gameid " + gameid + ")";

		if (!gameid.empty() && !description.empty()) {
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
	else if (oldSel != -1)
		// Select the last entry if the list has been reduced
		_list->setSelected(_list->getList().size() - 1);
	updateButtons();

	// Update the filter settings, those are lost when "setList"
	// is called.
	_list->setFilter(_searchWidget->getEditString());
}

void LauncherDialog::addGame() {
	int modifiers = g_system->getEventManager()->getModifierState();
	bool massAdd = (modifiers & Common::KBD_SHIFT) != 0;

	if (massAdd) {
		MessageDialog alert("Do you really want to run the mass game detector? "
							"This could potentially add a huge number of games.", "Yes", "No");
		if (alert.runModal() == GUI::kMessageOK && _browser->runModal() > 0) {
			MassAddDialog massAddDlg(_browser->getResult());

			if (_list->getSelected() != -1) {
				// Save current game position, so on cancel cursor will move back
				ConfMan.set("temp_selection", _domains[_list->getSelected()], ConfigManager::kApplicationDomain);
			}

			massAddDlg.runModal();

			// Update the ListWidget and force a redraw
			updateListing();

			// Set cursor to first detected game
			selectGame(ConfMan.get("temp_selection", ConfigManager::kApplicationDomain));
			ConfMan.removeKey("temp_selection", ConfigManager::kApplicationDomain);

			draw();
		}

		// We need to update the buttons here, so "Mass add" will revert to "Add game"
		// without any additional event.
		updateButtons();
		return;
	}

	// Allow user to add a new game to the list.
	// 1) show a dir selection dialog which lets the user pick the directory
	//    the game data resides in.
	// 2) try to auto detect which game is in the directory, if we cannot
	//    determine it uniquely present a list of candidates to the user
	//    to pick from
	// 3) Display the 'Edit' dialog for that item, letting the user specify
	//    an alternate description (to distinguish multiple versions of the
	//    game, e.g. 'Monkey German' and 'Monkey English') and set default
	//    options for that game
	// 4) If no game is found in the specified directory, return to the
	//    dialog.

	bool looping;
	do {
		looping = false;

		if (_browser->runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(_browser->getResult());
			Common::FSList files;
			if (!dir.getChildren(files, Common::FSNode::kListAll)) {
				MessageDialog alert("ScummVM couldn't open the specified directory!");
				alert.runModal();
				return;
			}

			// ...so let's determine a list of candidates, games that
			// could be contained in the specified directory.
			GameList candidates(EngineMan.detectGames(files));

			int idx;
			if (candidates.empty()) {
				// No game was found in the specified directory
				MessageDialog alert("ScummVM could not find any game in the specified directory!");
				alert.runModal();
				idx = -1;

				looping = true;
			} else if (candidates.size() == 1) {
				// Exact match
				idx = 0;
			} else {
				// Display the candidates to the user and let her/him pick one
				StringList list;
				for (idx = 0; idx < (int)candidates.size(); idx++)
					list.push_back(candidates[idx].description());

				ChooserDialog dialog("Pick the game:");
				dialog.setList(list);
				idx = dialog.runModal();
			}
			if (0 <= idx && idx < (int)candidates.size()) {
				GameDescriptor result = candidates[idx];

				// TODO: Change the detectors to set "path" !
				result["path"] = dir.getPath();

				Common::String domain = addGameToConf(result);

				// Display edit dialog for the new entry
				EditGameDialog editDialog(domain, result.description());
				if (editDialog.runModal() > 0) {
					// User pressed OK, so make changes permanent

					// Write config to disk
					ConfMan.flushToDisk();

					// Update the ListWidget, select the new item, and force a redraw
					updateListing();
					selectGame(editDialog.getDomain());
					draw();
				} else {
					// User aborted, remove the the new domain again
					ConfMan.removeGameDomain(domain);
				}

			}
		}
	} while (looping);
}

Common::String addGameToConf(const GameDescriptor &result) {
	// The auto detector or the user made a choice.
	// Pick a domain name which does not yet exist (after all, we
	// are *adding* a game to the config, not replacing).
	Common::String domain = result.preferredtarget();

	assert(!domain.empty());
	if (ConfMan.hasGameDomain(domain)) {
		int suffixN = 1;
		char suffix[16];
		Common::String gameid(domain);

		while (ConfMan.hasGameDomain(domain)) {
			snprintf(suffix, 16, "-%d", suffixN);
			domain = gameid + suffix;
			suffixN++;
		}
	}

	// Add the name domain
	ConfMan.addGameDomain(domain);

	// Copy all non-empty key/value pairs into the new domain
	for (GameDescriptor::const_iterator iter = result.begin(); iter != result.end(); ++iter) {
		if (!iter->_value.empty() && iter->_key != "preferredtarget")
			ConfMan.set(iter->_key, iter->_value, domain);
	}

	// TODO: Setting the description field here has the drawback
	// that the user does never notice when we upgrade our descriptions.
	// It might be nice ot leave this field empty, and only set it to
	// a value when the user edits the description string.
	// However, at this point, that's impractical. Once we have a method
	// to query all backends for the proper & full description of a given
	// game target, we can change this (currently, you can only query
	// for the generic gameid description; it's not possible to obtain
	// a description which contains extended information like language, etc.).

	return domain;
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
	// Set game specific options. Most of these should be "optional", i.e. by
	// default set nothing and use the global ScummVM settings. E.g. the user
	// can set here an optional alternate music volume, or for specific games
	// a different music driver etc.
	// This is useful because e.g. MonkeyVGA needs Adlib music to have decent
	// music support etc.
	assert(item >= 0);
	String gameId(ConfMan.get("gameid", _domains[item]));
	if (gameId.empty())
		gameId = _domains[item];
	EditGameDialog editDialog(_domains[item], EngineMan.findGame(gameId).description());
	if (editDialog.runModal() > 0) {
		// User pressed OK, so make changes permanent

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget, reselect the edited game and force a redraw
		updateListing();
		selectGame(editDialog.getDomain());
		draw();
	}
}

void LauncherDialog::loadGame(int item) {
	String gameId = ConfMan.get("gameid", _domains[item]);
	if (gameId.empty())
		gameId = _domains[item];

	const EnginePlugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);

	String target = _domains[item];
	target.toLowercase();

	if (plugin) {
		if ((*plugin)->hasFeature(MetaEngine::kSupportsListSaves) &&
			(*plugin)->hasFeature(MetaEngine::kSupportsLoadingDuringStartup)) {
			int slot = _loadDialog->runModal(plugin, target);
			if (slot >= 0) {
				ConfMan.setActiveDomain(_domains[item]);
				ConfMan.setInt("save_slot", slot, Common::ConfigManager::kTransientDomain);
				close();
			}
		} else {
			MessageDialog dialog
				("This game does not support loading games from the launcher.", "OK");
			dialog.runModal();
		}
	} else {
		MessageDialog dialog("ScummVM could not find any engine capable of running the selected game!", "OK");
		dialog.runModal();
	}
}

void LauncherDialog::handleKeyDown(Common::KeyState state) {
	if (state.keycode == Common::KEYCODE_TAB) {
		// Toggle between the game list and the quick search field.
		if (getFocusWidget() == _searchWidget) {
			setFocusWidget(_list);
		} else if (getFocusWidget() == _list) {
			setFocusWidget(_searchWidget);
		}
	}
	Dialog::handleKeyDown(state);
	updateButtons();
}

void LauncherDialog::handleKeyUp(Common::KeyState state) {
	Dialog::handleKeyUp(state);
	updateButtons();
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
	case kLoadGameCmd:
		loadGame(item);
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
		ConfMan.setActiveDomain(_domains[item]);
		close();
		break;
	case kListItemRemovalRequestCmd:
		removeGame(item);
		break;
	case kListSelectionChangedCmd:
		updateButtons();
		break;
	case kQuitCmd:
		ConfMan.setActiveDomain("");
		setResult(-1);
		close();
		break;
	case kSearchCmd:
		_list->setFilter(_searchWidget->getEditString());
		break;
	case kSearchClearCmd:
		_searchWidget->setEditString("");
		_list->setFilter("");
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

	int item = _list->getSelected();
	bool en = enable;

	if (item >= 0)
		en = !(Common::checkGameGUIOption(Common::GUIO_NOLAUNCHLOAD, ConfMan.get("guioptions", _domains[item])));

	if (en != _loadButton->isEnabled()) {
		_loadButton->setEnabled(en);
		_loadButton->draw();
	}

	// Update the label of the "Add" button depending on whether shift is pressed or not
	int modifiers = g_system->getEventManager()->getModifierState();
	const char *newAddButtonLabel = ((modifiers & Common::KBD_SHIFT) != 0)
		? "Mass Add"
		: "Add Game";

	if (_addButton->getLabel() != newAddButtonLabel)
		_addButton->setLabel(newAddButtonLabel);
}

void LauncherDialog::reflowLayout() {
#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowLauncherLogo") == 1 && g_gui.theme()->supportsImages()) {
		StaticTextWidget *ver = (StaticTextWidget*)findWidget("Launcher.Version");
		if (ver) {
			ver->setAlign((Graphics::TextAlign)g_gui.xmlEval()->getVar("Launcher.Version.Align", Graphics::kTextAlignCenter));
			ver->setLabel(gScummVMVersionDate);
		}

		if (!_logo)
			_logo = new GraphicsWidget(this, "Launcher.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageLogo));
	} else {
		StaticTextWidget *ver = (StaticTextWidget*)findWidget("Launcher.Version");
		if (ver) {
			ver->setAlign((Graphics::TextAlign)g_gui.xmlEval()->getVar("Launcher.Version.Align", Graphics::kTextAlignCenter));
			ver->setLabel(gScummVMFullVersion);
		}

		if (_logo) {
			removeWidget(_logo);
			_logo->setNext(0);
			delete _logo;
			_logo = 0;
		}
	}

	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		if (!_searchPic)
			_searchPic = new GraphicsWidget(this, "Launcher.SearchPic");
		_searchPic->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageSearch));

		if (_searchDesc) {
			removeWidget(_searchDesc);
			_searchDesc->setNext(0);
			delete _searchDesc;
			_searchDesc = 0;
		}
	} else {
		if (!_searchDesc)
			_searchDesc = new StaticTextWidget(this, "Launcher.SearchDesc", "Search:");

		if (_searchPic) {
			removeWidget(_searchPic);
			_searchPic->setNext(0);
			delete _searchPic;
			_searchPic = 0;
		}
	}
#endif

	_w = g_system->getOverlayWidth();
	_h = g_system->getOverlayHeight();

	Dialog::reflowLayout();
}

} // End of namespace GUI
