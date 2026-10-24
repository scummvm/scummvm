/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "gui/dialog-registry.h"
#include "common/translation.h"
#include "common/printman.h"
#include "engines/game.h"
#include "engines/metaengine.h"
#include "base/plugins.h"

#include "gui/about.h"
#include "gui/browser.h"
#include "gui/chooser.h"
#include "gui/console.h"
#include "gui/dlcsdialog.h"
#include "gui/downloaddialog.h"
#include "gui/downloadpacksdialog.h"
#include "gui/fluidsynth-dialog.h"
#include "gui/themebrowser.h"
#include "gui/massadd.h"
#include "gui/options.h"
#include "gui/launcher.h"
#include "gui/message.h"
#include "gui/remotebrowser.h"
#include "gui/integrity-dialog.h"
#include "gui/printing-dialog.h"
#include "gui/recorderdialog.h"
#include "gui/predictivedialog.h"
#include "gui/saveload-dialog.h"
#include "gui/shaderbrowser-dialog.h"
#include "gui/textviewer.h"
#include "gui/unknown-game-dialog.h"
#include "gui/updates-dialog.h"
#include "gui/helpdialog.h"
#include "gui/editgamedialog.h"
#include "gui/editrecorddialog.h"
#include "gui/filebrowser-dialog.h"
#include "gui/onscreendialog.h"
#include "gui/cloudconnectionwizard.h"
#include "gui/downloaddlcsdialog.h"
#include "gui/remotebrowser.h"

namespace GUI {

#ifdef USE_CLOUD
class DumpDownloadDialog : public DownloadDialog {
public:
	DumpDownloadDialog() : DownloadDialog(0, nullptr) {}

	void open() override {
		Dialog::open();
		reflowLayout();
	}
};
#endif

/**
 * The console opens above the screen and slides down in handleTickle(), which
 * the dumper never calls. Finish the slide right away so it is drawn in place.
 */
class DumpConsoleDialog : public ConsoleDialog {
public:
	DumpConsoleDialog() : ConsoleDialog(0.5f, 0.5f) {}

	void open() override {
		ConsoleDialog::open();
		_slideMode = kNoSlideMode;
		_y = 0;
	}
};

/**
 * The save/load choosers query a MetaEngine while laying out, so pick one that is
 * built in. Prefer an engine with thumbnails, so the snapshot shows the full layout.
 */
static const MetaEngine *findDumpMetaEngine() {
	const MetaEngine *fallback = nullptr;
	for (const Plugin *plugin : EngineMan.getPlugins(PLUGIN_TYPE_ENGINE)) {
		const MetaEngine &metaEngine = plugin->get<MetaEngine>();
		if (metaEngine.hasFeature(MetaEngine::kSavesSupportMetaInfo) && metaEngine.hasFeature(MetaEngine::kSavesSupportThumbnail))
			return &metaEngine;
		if (!fallback)
			fallback = &metaEngine;
	}
	return fallback;
}

/**
 * SaveLoadChooserDialog inherits Dialog as protected, which a derived class can undo.
 * The chooser closes itself during reflowLayout() when the resolution asks for the
 * other chooser type; the dumper owns the dialog, so that close is ignored.
 */
template<class Chooser>
class DumpSaveLoadChooser : public Chooser {
public:
	template<typename... Args>
	DumpSaveLoadChooser(const MetaEngine *metaEngine, Args... args) : Chooser(args...) {
		this->_metaEngine = metaEngine;
		this->_target = "dump";
		this->_delSupport = metaEngine->hasFeature(MetaEngine::kSupportsDeleteSave);
		this->_metaInfoSupport = metaEngine->hasFeature(MetaEngine::kSavesSupportMetaInfo);
		this->_thumbnailSupport = this->_metaInfoSupport && metaEngine->hasFeature(MetaEngine::kSavesSupportThumbnail);
		this->_saveDateSupport = this->_metaInfoSupport && metaEngine->hasFeature(MetaEngine::kSavesSupportCreationDate);
		this->_playTimeSupport = this->_metaInfoSupport && metaEngine->hasFeature(MetaEngine::kSavesSupportPlayTime);
	}

	Dialog *asDialog() { return this; }

	void reflowLayout() override {
		_inReflow = true;
		Chooser::reflowLayout();
		_inReflow = false;
	}

	void close() override {
		if (!_inReflow)
			Chooser::close();
	}

private:
	bool _inReflow = false;
};

void registerAllDialogs() {
	// Simple dialogs (no special state iteration needed)
	DialogDumperRegistry::registerDialog("MessageDialog",
		[]() -> Dialog * { return new MessageDialog(Common::U32String("Test message")); });

	DialogDumperRegistry::registerDialog("TimedMessageDialog",
		[]() -> Dialog * { return new TimedMessageDialog(Common::U32String("Test timed message"), 5); });

	DialogDumperRegistry::registerDialog("CountdownMessageDialog",
		[]() -> Dialog * { return new CountdownMessageDialog(Common::U32String("Test countdown"), 10, _("OK"), _("Cancel"), Graphics::kTextAlignCenter, _("Closing in %d seconds")); });

	DialogDumperRegistry::registerDialog("MessageDialogWithURL",
		[]() -> Dialog * { return new MessageDialogWithURL(Common::U32String("Test with URL"), "https://example.com"); });

	DialogDumperRegistry::registerDialog("AboutDialog",
		[]() -> Dialog * { return new AboutDialog(); });

	DialogDumperRegistry::registerDialog("BrowserDialog",
		[]() -> Dialog * { return new BrowserDialog(Common::U32String("Select directory"), true); });

	DialogDumperRegistry::registerDialog("ChooserDialog",
		[]() -> Dialog * { return new ChooserDialog(Common::U32String("Pick the game:")); });

	DialogDumperRegistry::registerDialog("ConsoleDialog",
		[]() -> Dialog * { return new DumpConsoleDialog(); });

#ifdef USE_DLC
	DialogDumperRegistry::registerDialog("DLCsDialog",
		[]() -> Dialog * { return new DLCsDialog(); });
#endif

#ifdef USE_CLOUD
	// The regular open() asks for the directories with modal browsers.
	DialogDumperRegistry::registerDialog("DownloadDialog",
		[]() -> Dialog * { return new DumpDownloadDialog(); });
#endif

#ifdef USE_DLC
	DialogDumperRegistry::registerDialog("DownloadDLCsDialog",
		[]() -> Dialog * { return new DownloadDLCsDialog(); });
#endif

#ifdef USE_HTTP
	DialogDumperRegistry::registerDialog("DownloadPacksDialog",
		[]() -> Dialog * { return new DownloadPacksDialog(Common::U32String("test packs"), "LIST", "gui-icons*.dat"); });
#endif

#ifdef USE_FLUIDSYNTH
	DialogDumperRegistry::registerDialog("FluidSynthSettingsDialog",
		[]() -> Dialog * { return new FluidSynthSettingsDialog(); });
#endif

	DialogDumperRegistry::registerDialog("ThemeBrowserDialog",
		[]() -> Dialog * { return new ThemeBrowser(); });

	DialogDumperRegistry::registerDialog("MassAddDialog",
		[]() -> Dialog * { return new MassAddDialog(Common::FSNode(".")); });

	// GlobalOptionsDialog needs a launcher as its parent, keep a single one around.
	DialogDumperRegistry::registerDialog("GlobalOptionsDialog",
		[]() -> Dialog * {
			static LauncherSimple *launcher = new LauncherSimple("Launcher");
			return new GlobalOptionsDialog(launcher);
		});

	DialogDumperRegistry::registerDialog("LauncherSimple",
		[]() -> Dialog * { return new LauncherSimple("Launcher"); });

	DialogDumperRegistry::registerDialog("RemovalConfirmationDialog",
		[]() -> Dialog * {
			Common::StringArray titles;
			titles.push_back("Test game 1");
			titles.push_back("Test game 2");
			return new RemovalConfirmationDialog(Common::U32String("Remove these games from the list?"), titles);
		});

#ifdef USE_CLOUD
	DialogDumperRegistry::registerDialog("RemoteBrowserDialog",
		[]() -> Dialog * { return new RemoteBrowserDialog(Common::U32String("Select directory")); });
#endif

	// IntegrityDialog needs a game target, the dialog dumper registers it.

	// The dialog keeps a reference to the surface to print
	DialogDumperRegistry::registerDialog("PrintingDialog",
		[]() -> Dialog * {
			static Graphics::ManagedSurface surface(64, 64, g_system->getOverlayFormat());
			return new PrintingDialog(surface, true, true, kPageOrientationPortrait);
		});

#ifdef ENABLE_EVENTRECORDER
	DialogDumperRegistry::registerDialog("RecorderDialog",
		[]() -> Dialog * { return new RecorderDialog(); });
#endif

	DialogDumperRegistry::registerDialog("PredictiveDialog",
		[]() -> Dialog * { return new PredictiveDialog(); });

	DialogDumperRegistry::registerDialog("SaveLoadCloudSyncProgressDialog",
		[]() -> Dialog * { return new SaveLoadCloudSyncProgressDialog(false, nullptr); });

	DialogDumperRegistry::registerDialog("SavenameDialog",
		[]() -> Dialog * { return new SavenameDialog(); });

	DialogDumperRegistry::registerDialog("ShaderBrowserDialog",
		[]() -> Dialog * { return new ShaderBrowserDialog(Common::Path()); });

	// TextViewerDialog shows a sample file, the dialog dumper registers it.

	// The dialog keeps a reference to the detected game
	DialogDumperRegistry::registerDialog("UnknownGameDialog",
		[]() -> Dialog * {
			// The dialog keeps a reference to the game. It is never freed: destroying it
			// during static destruction, after the managers are gone, crashes on exit.
			static DetectedGame *game = new DetectedGame("scumm", "monkey", "The Secret of Monkey Island", Common::EN_ANY, Common::kPlatformDOS);
			game->shortPath = "monkey";
			game->path = Common::Path("monkey");
			game->preferredTarget = "monkey";
			game->hasUnknownFiles = true;
			game->canBeAdded = true;
			return new UnknownGameDialog(*game);
		});

#ifdef USE_UPDATES
	DialogDumperRegistry::registerDialog("UpdatesDialog",
		[]() -> Dialog * { return new UpdatesDialog(); });
#endif

	DialogDumperRegistry::registerDialog("HelpDialog",
		[]() -> Dialog * { return new HelpDialog(); });

	// EditGameDialog needs a game target, the dialog dumper registers it.

#ifdef ENABLE_EVENTRECORDER
	DialogDumperRegistry::registerDialog("EditRecordDialog",
		[]() -> Dialog * { return new EditRecordDialog(Common::U32String("Author"), "Name", "Notes"); });
#endif

	DialogDumperRegistry::registerDialog("FileBrowserDialog",
		[]() -> Dialog * { return new FileBrowserDialog("Select file", "", 0); });

#ifdef ENABLE_EVENTRECORDER
	DialogDumperRegistry::registerDialog("OnScreenDialog",
		[]() -> Dialog * { return new OnScreenDialog(true); });
#endif

#ifdef USE_CLOUD
	DialogDumperRegistry::registerDialog("CloudConnectionWizard",
		[]() -> Dialog * { return new CloudConnectionWizard(); });
#endif

	DialogDumperRegistry::registerDialog("SaveLoadChooserSimple",
		[]() -> Dialog * {
			const MetaEngine *metaEngine = findDumpMetaEngine();
			if (!metaEngine)
				return nullptr;
			return (new DumpSaveLoadChooser<SaveLoadChooserSimple>(metaEngine, _("Save game:"), _("Save"), true))->asDialog();
		});
#ifndef DISABLE_SAVELOADCHOOSER_GRID
	DialogDumperRegistry::registerDialog("SaveLoadChooserGrid",
		[]() -> Dialog * {
			const MetaEngine *metaEngine = findDumpMetaEngine();
			if (!metaEngine)
				return nullptr;
			// Load mode: in save mode a single-slot page at low resolutions leaves no room for saves
			return (new DumpSaveLoadChooser<SaveLoadChooserGrid>(metaEngine, _("Restore game:"), false))->asDialog();
		});
#endif

	// Note: PopUpDialog is internal, not registered for dumping
}

} // End of namespace GUI