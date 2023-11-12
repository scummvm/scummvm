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

	if (outFile.open(outName)) {
		Image::writePNG(outFile, surf);
		outFile.finalize();
		outFile.close();

		warning("Dumped %s", filename.c_str());
	}
}

void dumpDialogs(const Common::String &message, int res, const Common::String &lang) {
#ifdef USE_TRANSLATION
	// Update GUI language
	TransMan.setLanguage(lang);
#endif

	// Update resolution
	ConfMan.setInt("last_window_width", res, Common::ConfigManager::kApplicationDomain);
	ConfMan.setInt("last_window_height", 600, Common::ConfigManager::kApplicationDomain);
	g_system->beginGFXTransaction();
		g_system->initSize(res, 600);
	g_system->endGFXTransaction();

	Graphics::Surface surf;
	surf.create(g_system->getOverlayWidth(), g_system->getOverlayHeight(), g_system->getOverlayFormat());

	Common::String filename = Common::String::format("%d-%s.png", res, lang.c_str());

	// Skipping Tooltips as not required

	// MessageDialog
	GUI::MessageDialog messageDialog(message);
	messageDialog.open();     // For rendering
	messageDialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "message-" + filename);
	messageDialog.close();

	// AboutDialog
	GUI::AboutDialog aboutDialog;
	aboutDialog.open();     // For rendering
	aboutDialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "about-" + filename);
	aboutDialog.close();

	// BrowserDialog
	GUI::BrowserDialog browserDialog(_("Select directory with game data"), true);
	browserDialog.open(); //For rendering
	browserDialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "browser-" + filename);
	browserDialog.close();

	// RemoteBrowserDialog
	GUI::RemoteBrowserDialog remoteBrowserDialog(_("Select directory with game data"));
	remoteBrowserDialog.open(); //For rendering
	remoteBrowserDialog.reflowLayout(); //For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "remoteBrowser-" + filename);
	remoteBrowserDialog.close();

	// ChooserDialog
	GUI::ChooserDialog chooserDialog(_("Pick the game:"));
	chooserDialog.open();         // For rendering
	chooserDialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "chooserDialog-" + filename);
	chooserDialog.close();

	// CloudConnectingWizard
	GUI::CloudConnectionWizard cloudConnectingWizard;
	cloudConnectingWizard.open();         // For rendering
	cloudConnectingWizard.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "cloudConnectingWizard-" + filename);
	cloudConnectingWizard.close();

	// DownloadIconPacksDialog
	GUI::DownloadPacksDialog downloadIconPacksDialog(_("icon packs"), "LIST", "gui-icons*.dat");
	downloadIconPacksDialog.open();         // For rendering
	downloadIconPacksDialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "downloadIconPacksDialog-" + filename);
	downloadIconPacksDialog.close();

	// DownloadShaderPacksDialog
	GUI::DownloadPacksDialog downloadShaderPacksDialog(_("shader packs"), "LIST-SHADERS", "shaders*.dat");
	downloadShaderPacksDialog.open();         // For rendering
	downloadShaderPacksDialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "downloadShaderPacksDialog-" + filename);
	downloadShaderPacksDialog.close();

	// FluidSynthSettingsDialog
	GUI::FluidSynthSettingsDialog fluidSynthSettingsDialog;
	fluidSynthSettingsDialog.open();         // For rendering
	fluidSynthSettingsDialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "fluidSynthSettingsDialog-" + filename);
	fluidSynthSettingsDialog.close();

	// ThemeBrowserDialog
	GUI::ThemeBrowser themeBrowser;
	themeBrowser.open();         // For rendering
	themeBrowser.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "themeBrowser-" + filename);
	themeBrowser.close();


	// MassAddDialog
	GUI::MassAddDialog massAddDialog(Common::FSNode("."));
	massAddDialog.open();         // For rendering
	massAddDialog.reflowLayout(); // For updating surface
	g_gui.redrawFull();
	g_system->grabOverlay(surf);
	saveGUISnapshot(surf, "massAddDialog-" + filename);
	massAddDialog.close();




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

	Common::FSNode dumpDir("snapshots");
	if (!dumpDir.isDirectory())
		dumpDir.createDirectory();

	// Iterate through all languages available
	for (Common::String lang : list) {
		// Iterating through the resolutions doesn't work so you have to manually change it here.
		// TODO: Find a way to iterate through the resolutions using code.
		Common::Array<int> res_to_test = {800, 640, 320};
		dumpDialogs(message, res_to_test[0], lang);
	}

#ifdef USE_TRANSLATION
	TransMan.setLanguage(originalLang);
#endif
	ConfMan.setInt("last_window_width", original_window_width, Common::ConfigManager::kApplicationDomain);
	ConfMan.setInt("last_window_height", original_window_height, Common::ConfigManager::kApplicationDomain);

	g_system->beginGFXTransaction();
		g_system->initSize(original_window_width, original_window_height);
	g_system->endGFXTransaction();
}

} // End of namespace GUI
