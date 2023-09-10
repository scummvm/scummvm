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
#include "common/translation.h"

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

void handleSimpleDialog(GUI::Dialog &dialog, const Common::String &filename,Graphics::Surface surf) {
	dialog.open();         // For rendering
	dialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, filename);
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

#if defined(USE_CLOUD) && defined(USE_LIBCURL)
	// CloudConnectingWizard
	GUI::CloudConnectionWizard cloudConnectingWizard;
	handleSimpleDialog(cloudConnectingWizard, "cloudConnectingWizard" + suffix, surf);

	// RemoteBrowserDialog
	GUI::RemoteBrowserDialog remoteBrowserDialog(_("Select directory with game data"));
	handleSimpleDialog(remoteBrowserDialog, "remoteBrowserDialog" + suffix, surf);

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
	ConfMan.setBool("force_resize", true, Common::ConfigManager::kApplicationDomain);
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
	ConfMan.removeKey("force_resize", Common::ConfigManager::kApplicationDomain);
}

} // End of namespace GUI
