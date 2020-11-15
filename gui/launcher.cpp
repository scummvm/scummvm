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
 */

#include "base/version.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/util.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/about.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/editgamedialog.h"
#include "gui/launcher.h"
#include "gui/massadd.h"
#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/options.h"
#ifdef ENABLE_EVENTRECORDER
#include "gui/onscreendialog.h"
#include "gui/recorderdialog.h"
#include "gui/EventRecorder.h"
#endif
#include "gui/saveload.h"
#include "gui/unknown-game-dialog.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/list.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/popup.h"
#include "gui/ThemeEval.h"

#include "graphics/cursorman.h"
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
#include "backends/cloud/cloudmanager.h"
#endif

using Common::ConfigManager;

namespace GUI {

enum {
	kStartCmd = 'STRT',
	kAboutCmd = 'ABOU',
	kOptionsCmd = 'OPTN',
	kAddGameCmd = 'ADDG',
	kMassAddGameCmd = 'MADD',
	kEditGameCmd = 'EDTG',
	kRemoveGameCmd = 'REMG',
	kLoadGameCmd = 'LOAD',
	kRecordGameCmd = 'RECG',
	kQuitCmd = 'QUIT',
	kSearchCmd = 'SRCH',
	kListSearchCmd = 'LSSR',
	kSearchClearCmd = 'SRCL',

	kCmdGlobalGraphicsOverride = 'OGFX',
	kCmdGlobalAudioOverride = 'OSFX',
	kCmdGlobalMIDIOverride = 'OMID',
	kCmdGlobalMT32Override = 'OM32',
	kCmdGlobalVolumeOverride = 'OVOL',

	kCmdChooseSoundFontCmd = 'chsf',

	kCmdExtraBrowser = 'PEXT',
	kCmdExtraPathClear = 'PEXC',
	kCmdGameBrowser = 'PGME',
	kCmdSaveBrowser = 'PSAV',
	kCmdSavePathClear = 'PSAC'
};

#pragma mark -

LauncherDialog::LauncherDialog()
	: Dialog("Launcher") {

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	build();

	GUI::GuiManager::instance()._launched = true;
}

void LauncherDialog::selectTarget(const String &target) {
	if (!target.empty()) {
		int itemToSelect = 0;
		StringArray::const_iterator iter;
		for (iter = _domains.begin(); iter != _domains.end(); ++iter, ++itemToSelect) {
			if (target == *iter) {
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

void LauncherDialog::build() {
#ifndef DISABLE_FANCY_THEMES
	_logo = nullptr;
	if (g_gui.xmlEval()->getVar("Globals.ShowLauncherLogo") == 1 && g_gui.theme()->supportsImages()) {
		_logo = new GraphicsWidget(this, "Launcher.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageLogo));

		new StaticTextWidget(this, "Launcher.Version", Common::U32String(gScummVMVersionDate));
	} else
		new StaticTextWidget(this, "Launcher.Version", Common::U32String(gScummVMFullVersion));
#else
	// Show ScummVM version
	new StaticTextWidget(this, "Launcher.Version", Common::U32String(gScummVMFullVersion));
#endif
	if (!g_system->hasFeature(OSystem::kFeatureNoQuit))
		new ButtonWidget(this, "Launcher.QuitButton", _("~Q~uit"), _("Quit ScummVM"), kQuitCmd);
	new ButtonWidget(this, "Launcher.AboutButton", _("A~b~out..."), _("About ScummVM"), kAboutCmd);
	new ButtonWidget(this, "Launcher.OptionsButton", _("~O~ptions..."), _("Change global ScummVM options"), kOptionsCmd);
	_startButton =
		new ButtonWidget(this, "Launcher.StartButton", _("~S~tart"), _("Start selected game"), kStartCmd);

	DropdownButtonWidget *loadButton =
	        new DropdownButtonWidget(this, "Launcher.LoadGameButton", _("~L~oad..."), _("Load saved game for selected game"), kLoadGameCmd);
#ifdef ENABLE_EVENTRECORDER
	loadButton->appendEntry(_("Record..."), kRecordGameCmd);
#endif
	_loadButton = loadButton;

	// Above the lowest button rows: two more buttons (directly below the list box)
	if (g_system->getOverlayWidth() > 320) {
		DropdownButtonWidget *addButton =
			new DropdownButtonWidget(this, "Launcher.AddGameButton", _("~A~dd Game..."), _("Add games to the list"), kAddGameCmd);
		addButton->appendEntry(_("Mass Add..."), kMassAddGameCmd);
		_addButton = addButton;

		_editButton =
			new ButtonWidget(this, "Launcher.EditGameButton", _("~E~dit Game..."), _("Change game options"), kEditGameCmd);
		_removeButton =
			new ButtonWidget(this, "Launcher.RemoveGameButton", _("~R~emove Game"), _("Remove game from the list. The game data files stay intact"), kRemoveGameCmd);
	} else {
		DropdownButtonWidget *addButton =
			new DropdownButtonWidget(this, "Launcher.AddGameButton", _c("~A~dd Game...", "lowres"), _("Add games to the list"), kAddGameCmd);
		addButton->appendEntry(_c("Mass Add...", "lowres"), kMassAddGameCmd);
		_addButton = addButton;

		_editButton =
		new ButtonWidget(this, "Launcher.EditGameButton", _c("~E~dit Game...", "lowres"), _("Change game options"), kEditGameCmd);
		_removeButton =
		new ButtonWidget(this, "Launcher.RemoveGameButton", _c("~R~emove Game", "lowres"), _("Remove game from the list. The game data files stay intact"), kRemoveGameCmd);
	}

	// Search box
	_searchDesc = nullptr;
#ifndef DISABLE_FANCY_THEMES
	_searchPic = nullptr;
	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		_searchPic = new GraphicsWidget(this, "Launcher.SearchPic", _("Search in game list"));
		_searchPic->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageSearch));
	} else
#endif
		_searchDesc = new StaticTextWidget(this, "Launcher.SearchDesc", _("Search:"));

	_searchWidget = new EditTextWidget(this, "Launcher.Search", _search, Common::U32String(), kSearchCmd);
	_searchClearButton = addClearButton(this, "Launcher.SearchClearButton", kSearchClearCmd);

	// Add list with game titles
	_list = new ListWidget(this, "Launcher.GameList", Common::U32String(), kListSearchCmd);
	_list->setEditable(false);
	_list->enableDictionarySelect(true);
	_list->setNumberingMode(kListNumberingOff);

	// Populate the list
	updateListing();

	// Restore last selection
	String last(ConfMan.get("lastselectedgame", ConfigManager::kApplicationDomain));
	selectTarget(last);

	// En-/disable the buttons depending on the list selection
	updateButtons();

	// Create file browser dialog
	_browser = new BrowserDialog(_("Select directory with game data"), true);

	// Create Load dialog
	_loadDialog = new SaveLoadChooser(_("Load game:"), _("Load"), false);
}

void LauncherDialog::clean() {
	while (_firstWidget) {
		Widget* w = _firstWidget;
		removeWidget(w);
		// This is called from rebuild() which may result from handleCommand being called by
		// a child widget sendCommand call. In such a case sendCommand is still being executed
		// so we should not delete yet the child widget. Thus delay the deletion.
		g_gui.addToTrash(w, this);
	}
	delete _browser;
	delete _loadDialog;
}

void LauncherDialog::rebuild() {
	clean();
	build();
	reflowLayout();
	setDefaultFocusedWidget();
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
struct LauncherEntry {
	Common::String key;
	Common::String description;
	const Common::ConfigManager::Domain *domain;

	LauncherEntry(Common::String k, Common::String d, const Common::ConfigManager::Domain *v) {
		key = k; description = d, domain = v;
	}
};

struct LauncherEntryComparator {
	bool operator()(const LauncherEntry &x, const LauncherEntry &y) const {
			return scumm_compareDictionary(x.description.c_str(), y.description.c_str()) < 0;
	}
};

void LauncherDialog::updateListing() {
	U32StringArray l;
	ListWidget::ColorList colors;
	ThemeEngine::FontColor color;
	int numEntries = ConfMan.getInt("gui_list_max_scan_entries");

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	const ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	bool scanEntries = numEntries == -1 ? true : (domains.size() <= numEntries);

	// Turn it into a list of pointers
	Common::List<LauncherEntry> domainList;
	for (ConfigManager::DomainMap::const_iterator iter = domains.begin(); iter != domains.end(); ++iter) {
#ifdef __DS__
		// DS port uses an extra section called 'ds'.  This prevents the section from being
		// detected as a game.
		if (iter->_key == "ds") {
			continue;
		}
#endif

		String description(iter->_value.getVal("description"));

		if (description.empty()) {
			QualifiedGameDescriptor g = EngineMan.findTarget(iter->_key);
			if (!g.description.empty())
				description = g.description;
		}

		if (description.empty()) {
			String gameid(iter->_value.getVal("gameid"));

			if (gameid.empty())
				gameid = iter->_key;

			description = Common::String::format("Unknown (target %s, gameid %s)", iter->_key.c_str(), gameid.c_str());
		}

		if (!description.empty())
			domainList.push_back(LauncherEntry(iter->_key, description, &iter->_value));
	}

	// Now sort the list in dictionary order
	Common::sort(domainList.begin(), domainList.end(), LauncherEntryComparator());

	// And fill out our structures
	for (Common::List<LauncherEntry>::const_iterator iter = domainList.begin(); iter != domainList.end(); ++iter) {
		color = ThemeEngine::kFontColorNormal;

		if (scanEntries) {
			Common::FSNode path(iter->domain->getVal("path"));
			if (!path.isDirectory()) {
				color = ThemeEngine::kFontColorAlternate;
				// If more conditions which grey out entries are added we should consider
				// enabling this so that it is easy to spot why a certain game entry cannot
				// be started.

				// description += Common::String::format(" (%s)", _("Not found"));
			}
		}

		l.push_back(iter->description);
		colors.push_back(color);
		_domains.push_back(iter->key);
	}

	const int oldSel = _list->getSelected();
	_list->setList(l, &colors);
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
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
			String selectedDirectory = _browser->getResult().getPath();
			String bannedDirectory = CloudMan.getDownloadLocalDirectory();
			if (selectedDirectory.size() && selectedDirectory.lastChar() != '/' && selectedDirectory.lastChar() != '\\')
				selectedDirectory += '/';
			if (bannedDirectory.size() && bannedDirectory.lastChar() != '/' && bannedDirectory.lastChar() != '\\') {
				if (selectedDirectory.size()) {
					bannedDirectory += selectedDirectory.lastChar();
				} else {
					bannedDirectory += '/';
				}
			}
			if (selectedDirectory.size() && bannedDirectory.size() && selectedDirectory.equalsIgnoreCase(bannedDirectory)) {
				MessageDialog alert(_("This directory cannot be used yet, it is being downloaded into!"));
				alert.runModal();
				return;
			}
#endif
			looping = !doGameDetection(_browser->getResult().getPath());
		}
	} while (looping);
}

void LauncherDialog::massAddGame() {
	MessageDialog alert(_("Do you really want to run the mass game detector? "
						  "This could potentially add a huge number of games."), _("Yes"), _("No"));
	if (alert.runModal() == GUI::kMessageOK && _browser->runModal() > 0) {
		MassAddDialog massAddDlg(_browser->getResult());

		massAddDlg.runModal();

		// Update the ListWidget and force a redraw

		// If new target(s) were added, update the ListWidget and move
		// the selection to to first newly detected game.
		Common::String newTarget = massAddDlg.getFirstAddedTarget();
		if (!newTarget.empty()) {
			updateListing();
			selectTarget(newTarget);
		}

		g_gui.scheduleTopDialogRedraw();
	}
}

void LauncherDialog::removeGame(int item) {
	MessageDialog alert(_("Do you really want to remove this game configuration?"), _("Yes"), _("No"));

	if (alert.runModal() == GUI::kMessageOK) {
		// Remove the currently selected game from the list
		assert(item >= 0);
		ConfMan.removeGameDomain(_domains[item]);

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget and force a redraw
		updateListing();
		g_gui.scheduleTopDialogRedraw();
	}
}

void LauncherDialog::editGame(int item) {
	// Set game specific options. Most of these should be "optional", i.e. by
	// default set nothing and use the global ScummVM settings. E.g. the user
	// can set here an optional alternate music volume, or for specific games
	// a different music driver etc.
	// This is useful because e.g. MonkeyVGA needs AdLib music to have decent
	// music support etc.
	assert(item >= 0);

	EditGameDialog editDialog(_domains[item]);
	if (editDialog.runModal() > 0) {
		// User pressed OK, so make changes permanent

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget, reselect the edited game and force a redraw
		updateListing();
		selectTarget(editDialog.getDomain());
		g_gui.scheduleTopDialogRedraw();
	}
}

#ifdef ENABLE_EVENTRECORDER
void LauncherDialog::recordGame(int item) {
	RecorderDialog recorderDialog;
	MessageDialog alert(_("Do you want to load saved game?"),
		_("Yes"), _("No"));
	switch(recorderDialog.runModal(_domains[item])) {
	default:
		// fallthrough intended
	case RecorderDialog::kRecordDialogClose:
		break;
	case RecorderDialog::kRecordDialogPlayback:
		ConfMan.setActiveDomain(_domains[item]);
		close();
		ConfMan.set("record_mode", "playback", ConfigManager::kTransientDomain);
		ConfMan.set("record_file_name", recorderDialog.getFileName(), ConfigManager::kTransientDomain);
		break;
	case RecorderDialog::kRecordDialogRecord:
		ConfMan.setActiveDomain(_domains[item]);
		if (alert.runModal() == GUI::kMessageOK) {
			loadGame(item);
		}
		close();
		g_eventRec.setAuthor(recorderDialog._author);
		g_eventRec.setName(recorderDialog._name);
		g_eventRec.setNotes(recorderDialog._notes);
		ConfMan.set("record_mode", "record", ConfigManager::kTransientDomain);
		break;
	}
}
#endif

void LauncherDialog::loadGame(int item) {
	String target = _domains[item];
	target.toLowercase();

	EngineMan.upgradeTargetIfNecessary(target);

	// Look for the plugin
	const Plugin *metaEnginePlugin = nullptr;
	const Plugin *enginePlugin = nullptr;
	EngineMan.findTarget(target, &metaEnginePlugin);

	// If we found a relevant plugin, find the matching engine plugin.
	if (metaEnginePlugin) {
		enginePlugin = PluginMan.getEngineFromMetaEngine(metaEnginePlugin);
	}

	if (enginePlugin) {
		const MetaEngine &metaEngineConnect = enginePlugin->get<MetaEngine>();
		if (metaEngineConnect.hasFeature(MetaEngine::kSupportsListSaves) &&
			metaEngineConnect.hasFeature(MetaEngine::kSupportsLoadingDuringStartup)) {
			int slot = _loadDialog->runModalWithPluginAndTarget(enginePlugin, target);
			if (slot >= 0) {
				ConfMan.setActiveDomain(_domains[item]);
				ConfMan.setInt("save_slot", slot, Common::ConfigManager::kTransientDomain);
				close();
			}
		} else {
			MessageDialog dialog
				(_("This game does not support loading games from the launcher."), _("OK"));
			dialog.runModal();
		}
	} else {
		MessageDialog dialog(_("ScummVM could not find any engine capable of running the selected game!"), _("OK"));
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

void LauncherDialog::handleOtherEvent(const Common::Event &evt) {
	Dialog::handleOtherEvent(evt);
	if (evt.type == Common::EVENT_DROP_FILE) {
		doGameDetection(evt.path);
	}
}

bool LauncherDialog::doGameDetection(const Common::String &path) {
	// Allow user to add a new game to the list.
	// 2) try to auto detect which game is in the directory, if we cannot
	//    determine it uniquely present a list of candidates to the user
	//    to pick from
	// 3) Display the 'Edit' dialog for that item, letting the user specify
	//    an alternate description (to distinguish multiple versions of the
	//    game, e.g. 'Monkey German' and 'Monkey English') and set default
	//    options for that game
	// 4) If no game is found in the specified directory, return to the
	//    dialog.

	// User made his choice...
	Common::FSNode dir(path);
	Common::FSList files;
	if (!dir.getChildren(files, Common::FSNode::kListAll)) {
		MessageDialog alert(_("ScummVM couldn't open the specified directory!"));
		alert.runModal();
		return true;
	}

	// ...so let's determine a list of candidates, games that
	// could be contained in the specified directory.
	DetectionResults detectionResults = EngineMan.detectGames(files);

	if (detectionResults.foundUnknownGames()) {
		Common::U32String report = detectionResults.generateUnknownGameReport(false, 80);
		g_system->logMessage(LogMessageType::kInfo, report.encode().c_str());
	}

	Common::Array<DetectedGame> candidates = detectionResults.listDetectedGames();

	int idx;
	if (candidates.empty()) {
		// No game was found in the specified directory
		MessageDialog alert(_("ScummVM could not find any game in the specified directory!"));
		alert.runModal();
		idx = -1;
		return false;
	} else if (candidates.size() == 1) {
		// Exact match
		idx = 0;
	} else {
		// Display the candidates to the user and let her/him pick one
		U32StringArray list;
		for (idx = 0; idx < (int)candidates.size(); idx++) {
			Common::U32String description = candidates[idx].description;

			if (candidates[idx].hasUnknownFiles) {
				description += Common::U32String(" - ");
				description += _("Unknown variant");
			}

			list.push_back(description);
		}

		ChooserDialog dialog(_("Pick the game:"));
		dialog.setList(list);
		idx = dialog.runModal();
	}

	if (0 <= idx && idx < (int)candidates.size()) {
		const DetectedGame &result = candidates[idx];

		if (result.hasUnknownFiles) {
			UnknownGameDialog dialog(result);

			bool cancel = dialog.runModal() == -1;
			if (cancel) {
				idx = -1;
			}
		}
	}

	if (0 <= idx && idx < (int)candidates.size()) {
		const DetectedGame &result = candidates[idx];
		Common::String domain = EngineMan.createTargetForGame(result);

		// Display edit dialog for the new entry
		EditGameDialog editDialog(domain);
		if (editDialog.runModal() > 0) {
			// User pressed OK, so make changes permanent

			// Write config to disk
			ConfMan.flushToDisk();

			// Update the ListWidget, select the new item, and force a redraw
			updateListing();
			selectTarget(editDialog.getDomain());
			g_gui.scheduleTopDialogRedraw();
		} else {
			// User aborted, remove the the new domain again
			ConfMan.removeGameDomain(domain);
		}

	}

	return true;
}

void LauncherDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int item = _list->getSelected();

	switch (cmd) {
	case kAddGameCmd:
		addGame();
		break;
	case kMassAddGameCmd:
		massAddGame();
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
#ifdef ENABLE_EVENTRECORDER
	case kRecordGameCmd:
		recordGame(item);
		break;
#endif
	case kOptionsCmd: {
		GlobalOptionsDialog options(this);
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
		// Start the selected game.
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
		// Update the active search filter.
		_list->setFilter(_searchWidget->getEditString());
		break;
	case kSearchClearCmd:
		// Reset the active search filter, thus showing all games again
		_searchWidget->setEditString(Common::U32String());
		_list->setFilter(Common::U32String());
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void LauncherDialog::updateButtons() {
	bool enable = (_list->getSelected() >= 0);
	if (enable != _startButton->isEnabled()) {
		_startButton->setEnabled(enable);
		_startButton->markAsDirty();
	}
	if (enable != _editButton->isEnabled()) {
		_editButton->setEnabled(enable);
		_editButton->markAsDirty();
	}
	if (enable != _removeButton->isEnabled()) {
		_removeButton->setEnabled(enable);
		_removeButton->markAsDirty();
	}

	int item = _list->getSelected();
	bool en = enable;

	if (item >= 0)
		en = !(Common::checkGameGUIOption(GUIO_NOLAUNCHLOAD, ConfMan.get("guioptions", _domains[item])));

	if (en != _loadButton->isEnabled()) {
		_loadButton->setEnabled(en);
		_loadButton->markAsDirty();
	}
}

void LauncherDialog::reflowLayout() {
#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowLauncherLogo") == 1 && g_gui.theme()->supportsImages()) {
		StaticTextWidget *ver = (StaticTextWidget *)findWidget("Launcher.Version");
		if (ver) {
			ver->setAlign(g_gui.xmlEval()->getWidgetTextHAlign("Launcher.Version"));
			ver->setLabel(Common::U32String(gScummVMVersionDate));
		}

		if (!_logo)
			_logo = new GraphicsWidget(this, "Launcher.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageLogo));
	} else {
		StaticTextWidget *ver = (StaticTextWidget *)findWidget("Launcher.Version");
		if (ver) {
			ver->setAlign(g_gui.xmlEval()->getWidgetTextHAlign("Launcher.Version"));
			ver->setLabel(Common::U32String(gScummVMFullVersion));
		}

		if (_logo) {
			removeWidget(_logo);
			_logo->setNext(nullptr);
			delete _logo;
			_logo = nullptr;
		}
	}

	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		if (!_searchPic)
			_searchPic = new GraphicsWidget(this, "Launcher.SearchPic");
		_searchPic->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageSearch));

		if (_searchDesc) {
			removeWidget(_searchDesc);
			_searchDesc->setNext(nullptr);
			delete _searchDesc;
			_searchDesc = nullptr;
		}
	} else {
		if (!_searchDesc)
			_searchDesc = new StaticTextWidget(this, "Launcher.SearchDesc", _("Search:"));

		if (_searchPic) {
			removeWidget(_searchPic);
			_searchPic->setNext(nullptr);
			delete _searchPic;
			_searchPic = nullptr;
		}
	}

	removeWidget(_searchClearButton);
	_searchClearButton->setNext(nullptr);
	delete _searchClearButton;
	_searchClearButton = addClearButton(this, "Launcher.SearchClearButton", kSearchClearCmd);
#endif

	_w = g_system->getOverlayWidth();
	_h = g_system->getOverlayHeight();

	Dialog::reflowLayout();
}

bool LauncherDialog::checkModifier(int checkedModifier) {
	int modifiers = g_system->getEventManager()->getModifierState();
	return (modifiers & checkedModifier) != 0;
}

} // End of namespace GUI
