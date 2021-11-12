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
#include "gui/widgets/groupedlist.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/grid.h"
#include "gui/ThemeEval.h"
#include "engines/advancedDetector.h"

#include "graphics/cursorman.h"
#if defined(USE_CLOUD) && defined(USE_LIBCURL)
#include "backends/cloud/cloudmanager.h"
#endif

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
	kSetGroupMethodCmd = 'GPBY',

	kListSwitchCmd = 'LIST',
	kGridSwitchCmd = 'GRID',

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

bool LauncherFilterMatcher(void *boss, int idx, const Common::U32String &item, Common::U32String token) {
	bool invert = false;
	while (token.size() && token[0] == '!') {
		token = token.substr(1);
		invert = !invert;
	}

	bool result = false;
	Common::String token8 = token;
	size_t pos = token8.findFirstOf(":=~");
	if (pos != token8.npos) {
		Common::String key = token8.substr(0, token8.findFirstOf(token8[pos]));
		Common::String filter = token8.substr(token8.findFirstOf(token8[pos]) + 1);

		if (key.size()) {
			if (Common::String("description").hasPrefix(key)) {
				key = "description";
			} else if (Common::String("engineid").hasPrefix(key)) {
				key = "engineid";
			} else if (Common::String("gameid").hasPrefix(key)) {
				key = "gameid";
			} else if (Common::String("language").hasPrefix(key)) {
				key = "language";
			} else if (Common::String("path").hasPrefix(key)) {
				key = "path";
			} else if (Common::String("platform").hasPrefix(key)) {
				key = "platform";
			}
		}

		LauncherDialog *launcher = (LauncherDialog *)(boss);
		Common::String data = launcher->getGameConfig(idx, key);
		data.toLowercase();

		if (token8[pos] == ':') {
			result = data.contains(filter);
		} else if (token8[pos] == '=') {
			result = data == filter;
		} else if (token8[pos] == '~') {
			result = data.matchString(filter);
		}
	} else {
		result = item.contains(token);
	}

	return invert ? !result : result;
}

LauncherDialog::LauncherDialog(const Common::String &dialogName)
	: Dialog(dialogName), _title(dialogName), _browser(nullptr),
	_loadDialog(nullptr), _searchClearButton(nullptr), _searchDesc(nullptr),
	_grpChooserDesc(nullptr), _grpChooserPopup(nullptr), _groupBy(kGroupByNone)
#ifndef DISABLE_FANCY_THEMES
	, _logo(nullptr), _searchPic(nullptr)
#endif // !DISABLE_FANCY_THEMES
#ifndef DISABLE_LAUNCHERDISPLAY_GRID
	, _listButton(nullptr), _gridButton(nullptr)
#endif // !DISABLE_LAUNCHERDISPLAY_GRID
	{
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundMain;

	GUI::GuiManager::instance()._launched = true;
#ifndef DISABLE_LAUNCHERDISPLAY_GRID
	addLayoutChooserButtons();
#endif // !DISABLE_LAUNCHERDISPLAY_GRID
	Common::FSDirectory *mdDir = new Common::FSDirectory(Common::String("./metadata/"));
	Common::ArchiveMemberList mdFiles;
	mdDir->listMatchingMembers(mdFiles, "*.xml");
	for (Common::ArchiveMemberList::iterator md = mdFiles.begin(); md != mdFiles.end(); ++md) {
		if (_metadataParser.loadStream((*md)->createReadStream()) == false) {
			warning("Failed to load XML file '%s'", (*md)->getDisplayName().encode().c_str());
			_metadataParser.close();
		}
		if (_metadataParser.parse() == false) {
			warning("Failed to parse XML file '%s'", (*md)->getDisplayName().encode().c_str());
			_metadataParser.close();
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
	_grpChooserDesc = new StaticTextWidget(this, Common::String(_title + ".laGroupPopupDesc"), Common::U32String("Group by: "));
	_grpChooserPopup = new PopUpWidget(this, Common::String(_title + ".laGroupPopup"), Common::U32String("Select a criteria to group the entries"), kSetGroupMethodCmd);
	// I18N: Group name for the game list
	_grpChooserPopup->appendEntry(_c("None", "group"), kGroupByNone);
	// I18N: Group name for the game list, grouped by the first letter of the game title
	_grpChooserPopup->appendEntry(_("First letter"), kGroupByFirstLetter);
	// I18N: Group name for the game list, grouped by game engine
	_grpChooserPopup->appendEntry(_("Engine"), kGroupByEngine);
	// I18N: Group name for the game list, grouped by game series
	_grpChooserPopup->appendEntry(_("Series"), kGroupBySeries);
	// I18N: Group name for the game list, grouped by game publisher
	_grpChooserPopup->appendEntry(_("Publisher"), kGroupByCompany);
	// I18N: Group name for the game list, grouped by language
	_grpChooserPopup->appendEntry(_("Language"), kGroupByLanguage);
	// I18N: Group name for the game list, grouped by game platform
	_grpChooserPopup->appendEntry(_("Platform"), kGroupByPlatform);
	// TODO: Save the last grouping method in config
	_grpChooserPopup->setSelected(kGroupByNone);
	if (g_gui.xmlEval()->getVar("Globals.ShowLauncherLogo") == 1 && g_gui.theme()->supportsImages()) {
		_logo = new GraphicsWidget(this, _title + ".Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfxFromTheme(ThemeEngine::kImageLogo);

		new StaticTextWidget(this, _title + ".Version", Common::U32String(gScummVMVersionDate));
	} else
		new StaticTextWidget(this, _title + ".Version", Common::U32String(gScummVMFullVersion));
#else
	// Show ScummVM version
	new StaticTextWidget(this, _title + ".Version", Common::U32String(gScummVMFullVersion));
#endif
	if (!g_system->hasFeature(OSystem::kFeatureNoQuit))
		new ButtonWidget(this, _title + ".QuitButton", _("~Q~uit"), _("Quit ScummVM"), kQuitCmd);
	new ButtonWidget(this, _title + ".AboutButton", _("A~b~out..."), _("About ScummVM"), kAboutCmd);
	new ButtonWidget(this, _title + ".OptionsButton", _("~O~ptions..."), _("Change global ScummVM options"), kOptionsCmd);

	// Above the lowest button rows: two more buttons (directly below the list box)
	if (g_system->getOverlayWidth() > 320) {
		DropdownButtonWidget *addButton =
			new DropdownButtonWidget(this, _title + ".AddGameButton", _("~A~dd Game..."), _("Add games to the list"), kAddGameCmd);
		addButton->appendEntry(_("Mass Add..."), kMassAddGameCmd);
		_addButton = addButton;

		_removeButton =
			new ButtonWidget(this, _title + ".RemoveGameButton", _("~R~emove Game"), _("Remove game from the list. The game data files stay intact"), kRemoveGameCmd);
	} else {
		DropdownButtonWidget *addButton =
			new DropdownButtonWidget(this, _title + ".AddGameButton", _c("~A~dd Game...", "lowres"), _("Add games to the list"), kAddGameCmd);
		addButton->appendEntry(_c("Mass Add...", "lowres"), kMassAddGameCmd);
		_addButton = addButton;

		_removeButton =
		new ButtonWidget(this, _title + ".RemoveGameButton", _c("~R~emove Game", "lowres"), _("Remove game from the list. The game data files stay intact"), kRemoveGameCmd);
	}

	// Search box
	_searchDesc = nullptr;
#ifndef DISABLE_FANCY_THEMES
	_searchPic = nullptr;
	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		_searchPic = new GraphicsWidget(this, _title + ".SearchPic", _("Search in game list"));
		_searchPic->setGfxFromTheme(ThemeEngine::kImageSearch);
	} else
#endif
		_searchDesc = new StaticTextWidget(this, _title + ".SearchDesc", _("Search:"));

	_searchWidget = new EditTextWidget(this, _title + ".Search", _search, Common::U32String(), kSearchCmd);
	_searchClearButton = addClearButton(this, _title + ".SearchClearButton", kSearchClearCmd);



	// Create file browser dialog
	_browser = new BrowserDialog(_("Select directory with game data"), true);

	// Create Load dialog
	_loadDialog = new SaveLoadChooser(_("Load game:"), _("Load"), false);
}

void LauncherDialog::clean() {
	while (_firstWidget) {
		Widget *w = _firstWidget;
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

int LauncherDialog::run() {
	// Open up
	open();

	// Start processing events
	g_gui.runLoop();

	// Return the result code
	return getResult();
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
	const int sel = getSelected();
	if (sel >= 0)
		ConfMan.set("lastselectedgame", _domains[sel], Common::ConfigManager::kApplicationDomain);
	else
		ConfMan.removeKey("lastselectedgame", Common::ConfigManager::kApplicationDomain);

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
		MD5Man.clear();
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

Common::String LauncherDialog::getGameConfig(int item, Common::String key) {
	if (ConfMan.hasKey(key, _domains[item])) {
		return ConfMan.get(key, _domains[item]);
	}
	return "";
}

void LauncherDialog::removeGame(int item) {
	MessageDialog alert(_("Do you really want to remove this game configuration?"), _("Yes"), _("No"));

	if (alert.runModal() == GUI::kMessageOK) {
		// Remove the currently selected game from the list
		assert(item >= 0);
		ConfMan.removeGameDomain(_domains[item]);

		// Write config to disk
		ConfMan.flushToDisk();

		// Update the ListWidget/GridWidget and force a redraw
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
		ConfMan.set("record_mode", "playback", Common::ConfigManager::kTransientDomain);
		ConfMan.set("record_file_name", recorderDialog.getFileName(), Common::ConfigManager::kTransientDomain);
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
		ConfMan.set("record_mode", "record", Common::ConfigManager::kTransientDomain);
		break;
	}
}
#endif

void LauncherDialog::loadGame(int item) {
	Common::String target = _domains[item];
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
		assert(enginePlugin->getType() == PLUGIN_TYPE_ENGINE);
		const MetaEngine &metaEngine = enginePlugin->get<MetaEngine>();
		if (metaEngine.hasFeature(MetaEngine::kSupportsListSaves) &&
			metaEngine.hasFeature(MetaEngine::kSupportsLoadingDuringStartup)) {
			int slot = _loadDialog->runModalWithMetaEngineAndTarget(&metaEngine, target);
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
	Dialog::handleKeyDown(state);
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
		Common::U32StringArray list;
		for (idx = 0; idx < (int)candidates.size(); idx++) {
			Common::U32String description = candidates[idx].description;

			if (candidates[idx].hasUnknownFiles) {
				description += Common::U32String(" - ");
				// I18N: Unknown game variant
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
	int item = getSelected();

	switch (cmd) {
	case kAddGameCmd:
		addGame();
		break;
	case kMassAddGameCmd:
		massAddGame();
		break;
	case kRemoveGameCmd:
		if (item < 0) return;
		removeGame(item);
		break;
	case kEditGameCmd:
		if (item < 0) return;
		editGame(item);
		break;
	case kLoadGameCmd:
		if (item < 0) return;
		loadGame(item);
		break;
#ifdef ENABLE_EVENTRECORDER
	case kRecordGameCmd:
		if (item < 0) return;
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
		// Start the selected game.
		if (item < 0) return;
		ConfMan.setActiveDomain(_domains[item]);
		close();
		break;
	case kQuitCmd:
		ConfMan.setActiveDomain("");
		setResult(-1);
		close();
		break;
#ifndef DISABLE_LAUNCHERDISPLAY_GRID
	case kGridSwitchCmd:
		setResult(kSwitchLauncherDialog);
		ConfMan.set("gui_launcher_chooser", "grid", Common::ConfigManager::kApplicationDomain);
		close();
		break;
	case kListSwitchCmd:
		setResult(kSwitchLauncherDialog);
		ConfMan.set("gui_launcher_chooser", "list", Common::ConfigManager::kApplicationDomain);
		close();
		break;
#endif
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void LauncherDialog::reflowLayout() {
	if (getType() == kLauncherDisplayGrid && !g_gui.xmlEval()->getVar("Globals.GridSupported", 0)) {
		setResult(kSwitchLauncherDialog);
		close();
		return;
	}
#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowLauncherLogo") == 1 && g_gui.theme()->supportsImages()) {
		StaticTextWidget *ver = (StaticTextWidget *)findWidget(Common::String(_title + ".Version").c_str());
		if (ver) {
			ver->setAlign(g_gui.xmlEval()->getWidgetTextHAlign(_title + ".Version"));
			ver->setLabel(Common::U32String(gScummVMVersionDate));
		}

		if (!_logo)
			_logo = new GraphicsWidget(this, _title + ".Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfxFromTheme(ThemeEngine::kImageLogo);
	} else {
		StaticTextWidget *ver = (StaticTextWidget *)findWidget(Common::String(_title + ".Version").c_str());
		if (ver) {
			ver->setAlign(g_gui.xmlEval()->getWidgetTextHAlign(_title + ".Version"));
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
			_searchPic = new GraphicsWidget(this, _title + ".SearchPic");
		_searchPic->setGfxFromTheme(ThemeEngine::kImageSearch);

		if (_searchDesc) {
			removeWidget(_searchDesc);
			_searchDesc->setNext(nullptr);
			delete _searchDesc;
			_searchDesc = nullptr;
		}
	} else {
		if (!_searchDesc)
			_searchDesc = new StaticTextWidget(this, _title + ".SearchDesc", _("Search:"));

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
	_searchClearButton = addClearButton(this, _title + ".SearchClearButton", kSearchClearCmd);
#endif
#ifndef DISABLE_LAUNCHERDISPLAY_GRID
	addLayoutChooserButtons();
#endif

	_w = g_system->getOverlayWidth();
	_h = g_system->getOverlayHeight();

	Dialog::reflowLayout();
}

#ifndef DISABLE_LAUNCHERDISPLAY_GRID
void LauncherDialog::addLayoutChooserButtons() {
	if (_listButton) {
		removeWidget(_listButton);
		delete _listButton;
		_listButton = nullptr;
	}

	if (_gridButton) {
		removeWidget(_gridButton);
		delete _gridButton;
		_gridButton = nullptr;
	}

	if (!g_gui.xmlEval()->getVar("Globals.GridSupported", 0))
		return;

	_listButton = createSwitchButton(_title + ".ListSwitch", Common::U32String("L"), _("List view"), ThemeEngine::kImageList, kListSwitchCmd);
	_gridButton = createSwitchButton(_title + ".GridSwitch", Common::U32String("G"), _("Grid view"), ThemeEngine::kImageGrid, kGridSwitchCmd);
}

ButtonWidget *LauncherDialog::createSwitchButton(const Common::String &name, const Common::U32String &desc, const Common::U32String &tooltip, const char *image, uint32 cmd) {
	ButtonWidget *button;

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowChooserPics") == 1 && g_gui.theme()->supportsImages()) {
		button = new PicButtonWidget(this, name, tooltip, cmd);
		((PicButtonWidget *)button)->useThemeTransparency(true);
		((PicButtonWidget *)button)->setGfx(g_gui.theme()->getImageSurface(image), kPicButtonStateEnabled, false);
	} else
#endif
		button = new ButtonWidget(this, name, desc, tooltip, cmd);

	return button;
}
#endif // !DISABLE_LAUNCHERDISPLAY_GRID

bool LauncherDialog::checkModifier(int checkedModifier) {
	int modifiers = g_system->getEventManager()->getModifierState();
	return (modifiers & checkedModifier) != 0;
}

#pragma mark -

LauncherChooser::LauncherChooser() : _impl(nullptr) {}

LauncherChooser::~LauncherChooser() {
	delete _impl;
	_impl = nullptr;
}

#ifndef DISABLE_LAUNCHERDISPLAY_GRID
LauncherDisplayType getRequestedLauncherType() {
	const Common::String &userConfig = ConfMan.get("gui_launcher_chooser", Common::ConfigManager::kApplicationDomain);
	// If grid needs to be disabled on certain resolutions,
	// those conditions need to be added here
	if (userConfig.equalsIgnoreCase("grid") && g_gui.xmlEval()->getVar("Globals.GridSupported", 0)) {
		return kLauncherDisplayGrid;
	} else {
		return kLauncherDisplayList;
	}
}
#endif // !DISABLE_LAUNCHERDISPLAY_GRID

class LauncherSimple : public LauncherDialog {
public:
	LauncherSimple(const Common::String &title);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleKeyDown(Common::KeyState state) override;

	LauncherDisplayType getType() const override { return kLauncherDisplayList; }

protected:
	void updateListing() override;
	void groupEntries(const Common::Array<const Common::ConfigManager::Domain *> &metadata);
	void updateButtons() override;
	void selectTarget(const Common::String &target) override;
	int getSelected() override;
	void build() override;
private:
	GroupedListWidget 		*_list;
};

#ifndef DISABLE_LAUNCHERDISPLAY_GRID
class LauncherGrid : public LauncherDialog {
public:
	LauncherGrid(const Common::String &title);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleKeyDown(Common::KeyState state) override;

	LauncherDisplayType getType() const override { return kLauncherDisplayGrid; }

protected:
	void updateListing() override;
	void groupEntries(const Common::Array<const Common::ConfigManager::Domain *> &metadata);
	void updateButtons() override;
	void selectTarget(const Common::String &target) override;
	int getSelected() override;
	void build() override;
private:
	GridWidget		*_grid;
};
#endif // !DISABLE_LAUNCHERDISPLAY_GRID


void LauncherChooser::selectLauncher() {
#ifndef DISABLE_LAUNCHERDISPLAY_GRID
	LauncherDisplayType requestedType = getRequestedLauncherType();
	if (!_impl || _impl->getType() != requestedType) {
		delete _impl;
		_impl = nullptr;

		switch (requestedType) {
		case kLauncherDisplayGrid:
			_impl = new LauncherGrid("LauncherGrid");
			break;

		default:
			// fallthrough intended
		case kLauncherDisplayList:
#endif // !DISABLE_LAUNCHERDISPLAY_GRID
			_impl = new LauncherSimple("Launcher");
#ifndef DISABLE_LAUNCHERDISPLAY_GRID
			break;
		}
	}
#endif // !DISABLE_LAUNCHERDISPLAY_GRID
}

int LauncherChooser::runModal() {
	if (!_impl)
		return -1;

	int ret;
	do {
		ret = _impl->run();
		if (ret == kSwitchLauncherDialog) {
			selectLauncher();
		}
	} while (ret < -1);
	return ret;
}

#pragma mark -

LauncherSimple::LauncherSimple(const Common::String &title)
	: LauncherDialog(title),
	_list(nullptr) {
	build();
}

void LauncherSimple::selectTarget(const Common::String &target) {
	if (!target.empty()) {
		int itemToSelect = 0;
		Common::StringArray::const_iterator iter;
		for (iter = _domains.begin(); iter != _domains.end(); ++iter, ++itemToSelect) {
			if (target == *iter) {
				_list->setSelected(itemToSelect);
				break;
			}
		}
	}
}

int LauncherSimple::getSelected() { return _list->getSelected(); }

void LauncherSimple::build() {
	LauncherDialog::build();

	_startButton =
		new ButtonWidget(this, "Launcher.StartButton", _("~S~tart"), _("Start selected game"), kStartCmd);

	DropdownButtonWidget *loadButton =
	        new DropdownButtonWidget(this, "Launcher.LoadGameButton", _("~L~oad..."), _("Load saved game for selected game"), kLoadGameCmd);
#ifdef ENABLE_EVENTRECORDER
	loadButton->appendEntry(_("Record..."), kRecordGameCmd);
#endif
	_loadButton = loadButton;

	// Add edit button
	if (g_system->getOverlayWidth() > 320) {
		_editButton =
			new ButtonWidget(this, "Launcher.EditGameButton", _("~E~dit Game..."), _("Change game options"), kEditGameCmd);
	} else {
		_editButton =
			new ButtonWidget(this, "Launcher.EditGameButton", _c("~E~dit Game...", "lowres"), _("Change game options"), kEditGameCmd);
	}

	// Add list with game titles
	_list = new GroupedListWidget(this, "Launcher.GameList", Common::U32String(), kListSearchCmd);
	_list->setEditable(false);
	_list->enableDictionarySelect(true);
	_list->setNumberingMode(kListNumberingOff);
	_list->setFilterMatcher(LauncherFilterMatcher, this);

	// Populate the list
	updateListing();

	// Restore last selection
	Common::String last(ConfMan.get("lastselectedgame", Common::ConfigManager::kApplicationDomain));
	selectTarget(last);

	// En-/disable the buttons depending on the list selection
	updateButtons();
}

void LauncherSimple::updateListing() {
	Common::U32StringArray l;
	Common::Array<const Common::ConfigManager::Domain *> attrs;
	ListWidget::ColorList colors;
	ThemeEngine::FontColor color;
	int numEntries = ConfMan.getInt("gui_list_max_scan_entries");

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	const Common::ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	bool scanEntries = numEntries == -1 ? true : ((int)domains.size() <= numEntries);

	// Turn it into a list of pointers
	Common::List<LauncherEntry> domainList;
	for (Common::ConfigManager::DomainMap::const_iterator iter = domains.begin(); iter != domains.end(); ++iter) {
		// Do not list temporary targets added when starting a game from the command line
		if (iter->_value.contains("id_came_from_command_line"))
			continue;

		Common::String description;

		if (!iter->_value.tryGetVal("description", description)) {
			QualifiedGameDescriptor g = EngineMan.findTarget(iter->_key);
			if (!g.description.empty())
				description = g.description;
		}

		if (description.empty()) {
			Common::String gameid;
			if (!iter->_value.tryGetVal("gameid", gameid));
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
			Common::String path;
			if (!iter->domain->tryGetVal("path", path) || !Common::FSNode(path).isDirectory()) {
				color = ThemeEngine::kFontColorAlternate;
				// If more conditions which grey out entries are added we should consider
				// enabling this so that it is easy to spot why a certain game entry cannot
				// be started.

				// description += Common::String::format(" (%s)", _("Not found"));
			}
		}
		l.push_back(iter->description);
		colors.push_back(color);
		attrs.push_back(iter->domain);
		_domains.push_back(iter->key);
	}

	const int oldSel = _list->getSelected();
	_list->setList(l, &colors);

	groupEntries(attrs);

	if (oldSel < (int)l.size() && oldSel >= 0)
		_list->setSelected(oldSel);	// Restore the old selection
	else if (oldSel != -1)
		// Select the last entry if the list has been reduced
		_list->setSelected(_list->getList().size() - 1);
	updateButtons();

	// Update the filter settings, those are lost when "setList"
	// is called.
	_list->setFilter(_searchWidget->getEditString());
}

void LauncherSimple::groupEntries(const Common::Array<const Common::ConfigManager::Domain *> &metadata) {
	Common::U32StringArray attrs;
	Common::StringMap metadataNames;
	_list->setGroupsVisibility(true);
	switch (_groupBy) {
	case kGroupByFirstLetter: {
		for (uint i = 0; i < metadata.size(); ++i) {
			attrs.push_back(metadata[i]->getValOrDefault("description").substr(0, 1));
		}
		_list->setGroupHeaderFormat(Common::U32String(""), Common::U32String("..."));
		break;
	}
	case kGroupByEngine: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String engineid = metadata[i]->contains(Common::String("engineid")) ?
								metadata[i]->getVal(Common::String("engineid")) : Common::String("");
			attrs.push_back(engineid);
		}
		_list->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List grouping when no engine is specified
		metadataNames[""] = _("Unknown Engine");
		Common::HashMap<Common::String, MetadataEngine, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = _metadataParser._engineInfo.begin();
		for (; i != _metadataParser._engineInfo.end(); ++i) {
			if (i->_value.alt_name.empty()) {
				metadataNames[i->_key] = i->_value.name;
			} else {
				metadataNames[i->_key] = Common::String::format("%s (%s)", i->_value.name.c_str(), i->_value.alt_name.c_str());
			}
		}
		break;
	}
	case kGroupByCompany: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String gameid = metadata[i]->getVal(Common::String("gameid"));
			attrs.push_back(_metadataParser._gameInfo[gameid].company_id);
		}
		_list->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List grouping when no pubisher is specified
		metadataNames[""] = _("Unknown Publisher");
		Common::HashMap<Common::String, MetadataCompany, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = _metadataParser._companyInfo.begin();
		for (; i != _metadataParser._companyInfo.end(); ++i) {
			if (i->_value.alt_name.empty()) {
				metadataNames[i->_key] = i->_value.name;
			} else {
				metadataNames[i->_key] = Common::String::format("%s (%s)", i->_value.name.c_str(), i->_value.alt_name.c_str());
			}
		}
		break;
	}
	case kGroupBySeries: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String gameid = metadata[i]->getVal(Common::String("gameid"));
			attrs.push_back(_metadataParser._gameInfo[gameid].series_id);
		}
		_list->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List group when no game series is specified
		metadataNames[""] = _("No Series");
		Common::HashMap<Common::String, MetadataSeries, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = _metadataParser._seriesInfo.begin();
		for (; i != _metadataParser._seriesInfo.end(); ++i) {
			metadataNames[i->_key] = i->_value.name;
		}
		break;
	}
	case kGroupByLanguage: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String language = metadata[i]->contains(Common::String("language")) ?
								metadata[i]->getVal(Common::String("language")) : Common::String("");
			attrs.push_back(language);
		}
		_list->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List group when no languageis specified
		metadataNames[""] = _("Language not detected");
		const Common::LanguageDescription *l = Common::g_languages;
		for (; l->code; ++l) {
			metadataNames[l->code] = l->description;
		}
		break;
	}
	case kGroupByPlatform: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String platform = metadata[i]->contains(Common::String("Platform")) ?
								metadata[i]->getVal(Common::String("Platform")) : Common::String("");
			attrs.push_back(platform);
		}
		_list->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List group when no platform is specified
		metadataNames[""] = _("Platform not detected");
		const Common::PlatformDescription *p = Common::g_platforms;
		for (; p->code; ++p) {
			metadataNames[p->code] = p->description;
		}
		break;
	}
	case kGroupByNone:	// Fall-through intentional
	default:
		_list->setGroupsVisibility(false);
		break;
	}
	_list->setMetadataNames(metadataNames);
	_list->setAttributeValues(attrs);
	_list->groupByAttribute();
}

void LauncherSimple::handleKeyDown(Common::KeyState state) {
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

void LauncherSimple::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		LauncherDialog::handleCommand(sender, kStartCmd, 0);
		break;
	case kListItemRemovalRequestCmd:
		LauncherDialog::handleCommand(sender, kRemoveGameCmd, 0);
		break;
	case kListSelectionChangedCmd:
		updateButtons();
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
	case kSetGroupMethodCmd: {
		// Change the grouping criteria
		GroupingMethod newGroupBy = (GroupingMethod)data;
		if (_groupBy != newGroupBy) {
			_groupBy = newGroupBy;
			updateListing();
		}
		break;
	}
	default:
		LauncherDialog::handleCommand(sender, cmd, data);
	}
}

void LauncherSimple::updateButtons() {
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

#pragma mark -

#ifndef DISABLE_LAUNCHERDISPLAY_GRID
LauncherGrid::LauncherGrid(const Common::String &title)
	: LauncherDialog(title),
	_grid(nullptr) {
	build();
}

void LauncherGrid::groupEntries(const Common::Array<const Common::ConfigManager::Domain *> &metadata) {
	Common::U32StringArray attrs;
	Common::StringMap metadataNames;
	switch (_groupBy) {
	case kGroupByFirstLetter: {
		for (uint i = 0; i < metadata.size(); ++i) {
			attrs.push_back(metadata[i]->getVal("description").substr(0, 1));
		}
		_grid->setGroupHeaderFormat(Common::U32String(""), Common::U32String("..."));
		break;
	}
	case kGroupByEngine: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String engineid = metadata[i]->contains(Common::String("engineid")) ?
								metadata[i]->getVal(Common::String("engineid")) : Common::String("");
			attrs.push_back(engineid);
		}
		_grid->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List grouping when no enginr is specified
		metadataNames[""] = _("Unknown Engine");
		Common::HashMap<Common::String, MetadataEngine, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = _metadataParser._engineInfo.begin();
		for (; i != _metadataParser._engineInfo.end(); ++i) {
			if (i->_value.alt_name.empty()) {
				metadataNames[i->_key] = i->_value.name;
			} else {
				metadataNames[i->_key] = Common::String::format("%s (%s)", i->_value.name.c_str(), i->_value.alt_name.c_str());
			}
		}
		break;
	}
	case kGroupBySeries: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String gameid = metadata[i]->getVal(Common::String("gameid"));
			attrs.push_back(_metadataParser._gameInfo[gameid].series_id);
		}
		_grid->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List grouping when no game series is specified
		metadataNames[""] = _("No Series");
		Common::HashMap<Common::String, MetadataSeries, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = _metadataParser._seriesInfo.begin();
		for (; i != _metadataParser._seriesInfo.end(); ++i) {
			metadataNames[i->_key] = i->_value.name;
		}
		break;
	}
	case kGroupByCompany: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String gameid = metadata[i]->getVal(Common::String("gameid"));
			attrs.push_back(_metadataParser._gameInfo[gameid].company_id);
		}
		_grid->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List group when no publisher is specified
		metadataNames[""] = _("Unknown Publisher");
		Common::HashMap<Common::String, MetadataCompany, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator i = _metadataParser._companyInfo.begin();
		for (; i != _metadataParser._companyInfo.end(); ++i) {
			if (i->_value.alt_name.empty()) {
				metadataNames[i->_key] = i->_value.name;
			} else {
				metadataNames[i->_key] = Common::String::format("%s (%s)", i->_value.name.c_str(), i->_value.alt_name.c_str());
			}
		}
		break;
	}
	case kGroupByLanguage: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String language = metadata[i]->contains(Common::String("language")) ?
								metadata[i]->getVal(Common::String("language")) : Common::String("");
			attrs.push_back(language);
		}
		_grid->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List group when no language is specified
		metadataNames[""] = _("Language not detected");
		const Common::LanguageDescription *l = Common::g_languages;
		for (; l->code; ++l) {
			metadataNames[l->code] = l->description;
		}
		break;
	}
	case kGroupByPlatform: {
		for (uint i = 0; i < metadata.size(); ++i) {
			Common::U32String platform = metadata[i]->contains(Common::String("Platform")) ?
								metadata[i]->getVal(Common::String("Platform")) : Common::String("");
			attrs.push_back(platform);
		}
		_grid->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		// I18N: List group when no platform is specified
		metadataNames[""] = _("Platform not detected");
		const Common::PlatformDescription *p = Common::g_platforms;
		for (; p->code; ++p) {
			metadataNames[p->code] = p->description;
		}
		break;
	}
	case kGroupByNone:	// Fall-through intentional
	default:
		for (uint i = 0; i < metadata.size(); ++i) {
			// I18N: Group for All items
			attrs.push_back(_("All"));
		}
		_grid->setGroupHeaderFormat(Common::U32String(""), Common::U32String(""));
		break;
	}
	_grid->setMetadataNames(metadataNames);
	_grid->setAttributeValues(attrs);
	_grid->groupEntries();
}

void LauncherGrid::handleKeyDown(Common::KeyState state) {
	if (state.keycode == Common::KEYCODE_TAB) {
		// Toggle between the game list and the quick search field.
		if (getFocusWidget() == _searchWidget) {
			setFocusWidget(_grid);
		} else {
			setFocusWidget(_searchWidget);
		}
	}
	Dialog::handleKeyDown(state);
	updateButtons();
}

void LauncherGrid::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
	case kPlayButtonCmd:
	case kItemDoubleClickedCmd:
		LauncherDialog::handleCommand(sender, kStartCmd, 0);
		break;
	case kEditButtonCmd:
		LauncherDialog::handleCommand(sender, kEditGameCmd, 0);
		break;
	case kLoadButtonCmd:
		LauncherDialog::handleCommand(sender, kLoadGameCmd, 0);
		break;
	case kItemClicked:
		updateButtons();
		break;
	case kSearchCmd:
		// Update the active search filter.
		_grid->setFilter(_searchWidget->getEditString());
		break;
	case kSearchClearCmd:
		// Reset the active search filter, thus showing all games again
		_searchWidget->setEditString(Common::U32String());
		_grid->setFilter(Common::U32String());
		break;
	case kSetGroupMethodCmd: {
		// Change the grouping criteria
		GroupingMethod newGroupBy = (GroupingMethod)data;
		if (_groupBy != newGroupBy) {
			_groupBy = newGroupBy;
			updateListing();
		}
		break;
	}
	default:
		LauncherDialog::handleCommand(sender, cmd, data);
	}
}

void LauncherGrid::updateListing() {
	Common::Array<const Common::ConfigManager::Domain *> attrs;

	// Retrieve a list of all games defined in the config file
	_domains.clear();
	const Common::ConfigManager::DomainMap &domains = ConfMan.getGameDomains();
	// Turn it into a list of pointers
	Common::Array<LauncherEntry> domainList;
	for (Common::ConfigManager::DomainMap::const_iterator iter = domains.begin(); iter != domains.end(); ++iter) {
		// Do not list temporary targets added when starting a game from the command line
		if (iter->_value.contains("id_came_from_command_line"))
			continue;

		Common::String description;

		if (!iter->_value.tryGetVal("description", description)) {
			QualifiedGameDescriptor g = EngineMan.findTarget(iter->_key);
			if (!g.description.empty())
				description = g.description;
		}

		if (description.empty()) {
			Common::String gameid;
			if (!iter->_value.tryGetVal("gameid", gameid));
				gameid = iter->_key;

			description = Common::String::format("Unknown (target %s, gameid %s)", iter->_key.c_str(), gameid.c_str());
		}

		// Strip platform language from the title.
		size_t extraPos = description.findFirstOf("(");
		if (extraPos != Common::String::npos)
			description = Common::String(description.c_str(), extraPos);

		if (!description.empty())
			domainList.push_back(LauncherEntry(iter->_key, description, &iter->_value));
	}

	// Now sort the list in dictionary order
	Common::sort(domainList.begin(), domainList.end(), LauncherEntryComparator());

	Common::Array<GridItemInfo> gridList;

	int k = 0;
	for (Common::Array<LauncherEntry>::const_iterator iter = domainList.begin(); iter != domainList.end(); ++iter) {
		Common::String gameid = iter->domain->getVal("gameid");
		Common::String title = iter->description;
		Common::String engineid = "UNK";
		Common::String language = "XX";
		Common::String platform;
		iter->domain->tryGetVal("engineid", engineid);
		iter->domain->tryGetVal("language", language);
		iter->domain->tryGetVal("platform", platform);
		attrs.push_back(iter->domain);
		gridList.push_back(GridItemInfo(k++, engineid, gameid, title, Common::parseLanguage(language), Common::parsePlatform(platform)));
	}

	_grid->setEntryList(&gridList);
	groupEntries(attrs);
	// And fill out our structures
	for (Common::Array<LauncherEntry>::const_iterator iter = domainList.begin(); iter != domainList.end(); ++iter) {
		_domains.push_back(iter->key);
	}
}

void LauncherGrid::updateButtons() {
	bool enable = (_grid->getSelected() >= 0);
	if (enable != _removeButton->isEnabled()) {
		_removeButton->setEnabled(enable);
		_removeButton->markAsDirty();
	}
}

void LauncherGrid::selectTarget(const Common::String &target) {}
int LauncherGrid::getSelected() { return _grid->getSelected(); }

void LauncherGrid::build() {
	LauncherDialog::build();

	// Add list with game titles
	_grid = new GridWidget(this, "LauncherGrid.IconArea");
	// Populate the list
	updateListing();

	// Restore last selection
	Common::String last(ConfMan.get("lastselectedgame", Common::ConfigManager::kApplicationDomain));
	selectTarget(last);

	// En-/disable the buttons depending on the list selection
	updateButtons();
}
#endif // !DISABLE_LAUNCHERDISPLAY_GRID

} // End of namespace GUI
