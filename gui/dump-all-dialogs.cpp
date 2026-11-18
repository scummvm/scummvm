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

#include "gui/dump-all-dialogs.h"

#include "common/file.h"
#include "common/fs.h"
#include "common/language.h"
#include "common/printman.h"
#include "common/translation.h"

#include "engines/game.h"

#include "gui/ThemeEngine.h"
#include "gui/about.h"
#include "gui/gui-manager.h"
#include "gui/launcher.h"
#include "gui/message.h"
#include "gui/browser.h"
#include "gui/downloaddialog.h"
#include "gui/remotebrowser.h"
#include "gui/chooser.h"
#include "gui/cloudconnectionwizard.h"
#include "gui/dialog.h"
#include "gui/downloaddialog.h"
#include "gui/downloadpacksdialog.h"
#include "gui/fluidsynth-dialog.h"
#include "gui/themebrowser.h"
#include "gui/massadd.h"
#include "gui/options.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/widgets/tab.h"
#include "gui/launcher.h"
#include "gui/helpdialog.h"
#include "gui/predictivedialog.h"
#include "gui/printing-dialog.h"
#include "gui/saveload-dialog.h"
#include "gui/shaderbrowser-dialog.h"
#include "gui/unknown-game-dialog.h"

#ifdef USE_DLC
#include "gui/dlcsdialog.h"
#include "gui/downloaddlcsdialog.h"
#endif

#ifdef ENABLE_EVENTRECORDER
#include "gui/editrecorddialog.h"
#include "gui/recorderdialog.h"
#endif

#ifdef USE_UPDATES
#include "gui/updates-dialog.h"
#endif

#include "image/png.h"

namespace GUI {

void saveGUISnapshot(Graphics::Surface surf, const Common::String &filename) {
	Common::DumpFile outFile;
	Common::String outName = Common::String::format("snapshots/%s", filename.c_str());

	if (outFile.open(Common::Path(outName, '/'))) {
		Image::writePNG(outFile, surf);
		outFile.finalize();
		outFile.close();

		warning("Dumped %s", filename.c_str());
	}
}

// Returns the scroll container of the dialog, or of its active tab
static ScrollContainerWidget *findScrollContainer(GUI::Dialog &dialog) {
	Widget *widget = dialog.findWidget((uint32)kScrollContainerWidget);

	if (!widget) {
		TabWidget *tabWidget = (TabWidget *)dialog.findWidget((uint32)kTabWidget);
		if (tabWidget)
			widget = Widget::findWidgetInChain(tabWidget->getActiveTabWidgets(), (uint32)kScrollContainerWidget);
	}

	return (ScrollContainerWidget *)widget;
}

// Dumps the rest of the contents of a scroll container, one page at a time
static void dumpScrolledPages(GUI::Dialog &dialog, const Common::String &filename, Graphics::Surface surf) {
	ScrollContainerWidget *container = findScrollContainer(dialog);
	if (!container)
		return;

	// Overlap the pages, so the line cut at the bottom of a page is complete on the next one
	const int step = container->getHeight() * 3 / 4;
	const int maxPos = container->getMaxScrollPos();
	const int oldPos = container->getScrollPos();
	if (step <= 0)
		return;

	// The first page is the snapshot of the dialog itself
	Common::String baseName = filename.substr(0, filename.size() - 4);
	int page = 2;
	for (int pos = step; pos < maxPos + step; pos += step, page++) {
		container->setScrollPos(MIN(pos, maxPos));
		g_gui.redrawFull();
		g_system->grabOverlay(surf);
		saveGUISnapshot(surf, Common::String::format("%s-page%d.png", baseName.c_str(), page));
	}

	container->setScrollPos(oldPos);
}

// Dumps an open dialog, including the hidden part of its scroll container
static void dumpOpenDialog(GUI::Dialog &dialog, const Common::String &filename, Graphics::Surface surf) {
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, filename);
	dumpScrolledPages(dialog, filename, surf);
}

void handleSimpleDialog(GUI::Dialog &dialog, const Common::String &filename,Graphics::Surface surf) {
	dialog.open();         // For rendering
	dialog.reflowLayout(); // For updating surface
	dumpOpenDialog(dialog, filename, surf);
	dialog.close();
}

void loopThroughTabs(GUI::Dialog &dialog, const Common::String &lang, Graphics::Surface surf, const Common::String name) {
	// Open the dialog only once: the options dialogs build new widgets each time they are opened
	dialog.open();
	dialog.reflowLayout();
	GUI::Widget *widget = nullptr;
	widget = dialog.findWidget((uint32)kTabWidget);

	if (widget) {
		TabWidget *tabWidget = (TabWidget *)widget;
		for (int tabNo = 0; tabNo < tabWidget->getTabCount(); tabNo++) {
			Common::String suffix = Common::String::format("-%d-%dx%d-%s.png", tabNo + 1, g_system->getOverlayWidth(), g_system->getOverlayHeight(), lang.c_str());
			tabWidget->setActiveTab(tabNo);
			dumpOpenDialog(dialog, name + suffix, surf);
		}
	}
	dialog.close();
}

void dumpDialogs(const Common::String &message, const Common::String &lang) {
#ifdef USE_TRANSLATION
	// Update GUI language
	TransMan.setLanguage(lang);
#endif

	Graphics::Surface surf;
	surf.create(g_system->getOverlayWidth(), g_system->getOverlayHeight(), g_system->getOverlayFormat());

	Common::String suffix = Common::String::format("-%dx%d-%s.png", g_system->getOverlayWidth(), g_system->getOverlayHeight(), lang.c_str());

	// Skipping Tooltips as not required

	// MessageDialog
	GUI::MessageDialog messageDialog(message);
	handleSimpleDialog(messageDialog, "messageDialog" + suffix, surf);
	// AboutDialog
	GUI::AboutDialog aboutDialog;
	handleSimpleDialog(aboutDialog, "aboutDialog" + suffix, surf);

#ifdef USE_CLOUD
	// CloudConnectingWizard
	GUI::CloudConnectionWizard cloudConnectingWizard;
	handleSimpleDialog(cloudConnectingWizard, "cloudConnectingWizard" + suffix, surf);

	// RemoteBrowserDialog
	GUI::RemoteBrowserDialog remoteBrowserDialog(_("Select directory with game data"));
	handleSimpleDialog(remoteBrowserDialog, "remoteBrowserDialog" + suffix, surf);
#endif

#ifdef USE_HTTP
	// DownloadIconPacksDialog
	GUI::DownloadPacksDialog downloadIconPacksDialog(_("icon packs"), "LIST", "gui-icons*.dat");
	handleSimpleDialog(downloadIconPacksDialog, "downloadIconPacksDialog" + suffix, surf);

	// DownloadShaderPacksDialog
	GUI::DownloadPacksDialog downloadShaderPacksDialog(_("shader packs"), "LIST-SHADERS", "shaders*.dat");
	handleSimpleDialog(downloadShaderPacksDialog, "downloadShaderPacksDialog" + suffix, surf);
#endif

#ifdef USE_FLUIDSYNTH
	// FluidSynthSettingsDialog
	GUI::FluidSynthSettingsDialog fluidSynthSettingsDialog;
	handleSimpleDialog(fluidSynthSettingsDialog, "fluidSynthSettings-" + suffix, surf);
#endif

	// ThemeBrowserDialog
	GUI::ThemeBrowser themeBrowser;
	handleSimpleDialog(themeBrowser, "themeBrowser-" + suffix, surf);

	// BrowserDialog
	GUI::BrowserDialog browserDialog(_("Select directory with game data"), true);
	handleSimpleDialog(browserDialog, "browserDialog-" + suffix, surf);

	// ChooserDialog
	GUI::ChooserDialog chooserDialog(_("Pick the game:"));
	handleSimpleDialog(chooserDialog, "chooserDialog-" + suffix, surf);

	// MassAddDialog
	GUI::MassAddDialog massAddDialog(Common::FSNode("."));
	handleSimpleDialog(massAddDialog, "massAddDialog-" + suffix, surf);

	// MessageDialogWithURL
	GUI::MessageDialogWithURL messageDialogWithURL(message, "https://www.scummvm.org");
	handleSimpleDialog(messageDialogWithURL, "messageDialogWithURL" + suffix, surf);

	// RemovalConfirmationDialog
	Common::StringArray gameTitles;
	gameTitles.push_back("Game 1");
	gameTitles.push_back("Game 2");
	Common::U32String removalMessage = Common::U32String::format(
		_("Do you really want to remove the following %d game configurations?\n\n"), (int)gameTitles.size());
	GUI::RemovalConfirmationDialog removalConfirmationDialog(removalMessage, gameTitles);
	handleSimpleDialog(removalConfirmationDialog, "removalConfirmationDialog" + suffix, surf);

	// UnknownGameDialog
	DetectedGame detectedGame("scumm", "monkey", "The Secret of Monkey Island", Common::EN_ANY, Common::kPlatformDOS);
	detectedGame.hasUnknownFiles = true;
	detectedGame.canBeAdded = true;
	GUI::UnknownGameDialog unknownGameDialog(detectedGame);
	handleSimpleDialog(unknownGameDialog, "unknownGameDialog" + suffix, surf);

	// HelpDialog
	GUI::HelpDialog helpDialog;
	loopThroughTabs(helpDialog, lang, surf, "helpDialog");

	// PredictiveDialog
	GUI::PredictiveDialog predictiveDialog;
	handleSimpleDialog(predictiveDialog, "predictiveDialog" + suffix, surf);

	// PrintingDialog
	Graphics::ManagedSurface printSurface(64, 64, g_system->getOverlayFormat());
	GUI::PrintingDialog printingDialog(printSurface, true, true, kPageOrientationPortrait);
	handleSimpleDialog(printingDialog, "printingDialog" + suffix, surf);

	// SavenameDialog
	GUI::SavenameDialog savenameDialog;
	handleSimpleDialog(savenameDialog, "savenameDialog" + suffix, surf);

	// ShaderBrowserDialog
	GUI::ShaderBrowserDialog shaderBrowserDialog((Common::Path()));
	handleSimpleDialog(shaderBrowserDialog, "shaderBrowserDialog" + suffix, surf);

#ifdef USE_DLC
	// DLCsDialog
	GUI::DLCsDialog dlcsDialog;
	handleSimpleDialog(dlcsDialog, "dlcsDialog" + suffix, surf);

	// DownloadDLCsDialog
	GUI::DownloadDLCsDialog downloadDLCsDialog;
	handleSimpleDialog(downloadDLCsDialog, "downloadDLCsDialog" + suffix, surf);
#endif

#ifdef ENABLE_EVENTRECORDER
	// RecorderDialog
	GUI::RecorderDialog recorderDialog;
	handleSimpleDialog(recorderDialog, "recorderDialog" + suffix, surf);

	// EditRecordDialog
	GUI::EditRecordDialog editRecordDialog(_("Unknown Author"), "Name", "Notes");
	handleSimpleDialog(editRecordDialog, "editRecordDialog" + suffix, surf);
#endif

#ifdef USE_UPDATES
	// UpdatesDialog
	GUI::UpdatesDialog updatesDialog;
	handleSimpleDialog(updatesDialog, "updatesDialog" + suffix, surf);
#endif

	// GlobalOptionsDialog
	LauncherSimple launcherDialog("Launcher");
	GUI::GlobalOptionsDialog globalOptionsDialog(&launcherDialog);
	loopThroughTabs(globalOptionsDialog, lang, surf, "GlobalOptionDialog");

	// LauncherDialog
#if 0
	GUI::LauncherChooser chooser;
	chooser.selectLauncher();
	chooser.open();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "launcher-" + filename);
	chooser.close();
#endif
}

void dumpAllDialogs(const Common::String &message) {
#ifdef USE_TRANSLATION
	auto originalLang = TransMan.getCurrentLanguage();
#endif
	int original_window_width = ConfMan.getInt("last_window_width", Common::ConfigManager::kApplicationDomain);
	int original_window_height = ConfMan.getInt("last_window_height", Common::ConfigManager::kApplicationDomain);
	Common::List<Common::String> list = Common::getLanguageList();
	const int res[] = {320, 200,
					   320, 240,
					   640, 400,
					   640, 480,
					   800, 600,
					   0};

	// HACK: Pass info to backend to force window resize
	ConfMan.setBool("dumper_force_resize", true, Common::ConfigManager::kApplicationDomain);
	Common::FSNode dumpDir("snapshots");

	if (!dumpDir.isDirectory())
		dumpDir.createDirectory();

	// Iterate through all resolutions available
	for (const int *r = res; *r; r += 2) {
		int w = r[0];
		int h = r[1];

		// Update resolution
		ConfMan.setInt("last_window_width" , w, Common::ConfigManager::kApplicationDomain);
		ConfMan.setInt("last_window_height", h, Common::ConfigManager::kApplicationDomain);
		g_system->beginGFXTransaction();
			g_system->initSize(w, h);
		g_system->endGFXTransaction();

		// Iterate through all langauges
		for (Common::String &lang : list) {

			dumpDialogs(message, lang);
		}

	}

#ifdef USE_TRANSLATION
	TransMan.setLanguage(originalLang);
#endif
	ConfMan.setInt("last_window_width", original_window_width, Common::ConfigManager::kApplicationDomain);
	ConfMan.setInt("last_window_height", original_window_height, Common::ConfigManager::kApplicationDomain);

	g_system->beginGFXTransaction();
		g_system->initSize(original_window_width, original_window_height);
	g_system->endGFXTransaction();

	// Clean up the temporary flag.
	// Since we are still within the same method where we added,
	// there is no need to flush config to the disk
	ConfMan.removeKey("dumper_force_resize", Common::ConfigManager::kApplicationDomain);
	ConfMan.flushToDisk();

	warning("ALL DIALOGS DUMPED");
}

} // End of namespace GUI
