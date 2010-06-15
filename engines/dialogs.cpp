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
#include "common/savefile.h"
#include "common/system.h"
#include "common/events.h"
#include "common/translation.h"

#include "graphics/scaler.h"

#include "gui/about.h"
#include "gui/GuiManager.h"
#include "gui/launcher.h"
#include "gui/ListWidget.h"
#include "gui/options.h"
#include "gui/saveload.h"
#include "gui/ThemeEval.h"

#include "engines/dialogs.h"
#include "engines/engine.h"
#include "engines/metaengine.h"

#ifdef SMALL_SCREEN_DEVICE
#include "gui/KeysDialog.h"
#endif

using GUI::CommandSender;
using GUI::StaticTextWidget;

class ConfigDialog : public GUI::OptionsDialog {
protected:
#ifdef SMALL_SCREEN_DEVICE
	GUI::Dialog		*_keysDialog;
#endif

public:
	ConfigDialog(bool subtitleControls);
	~ConfigDialog();

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
};

MainMenuDialog::MainMenuDialog(Engine *engine)
	: GUI::Dialog("GlobalMenu"), _engine(engine) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;

#ifndef DISABLE_FANCY_THEMES
	_logo = 0;
	if (g_gui.xmlEval()->getVar("Globals.ShowGlobalMenuLogo", 0) == 1 && g_gui.theme()->supportsImages()) {
		_logo = new GUI::GraphicsWidget(this, "GlobalMenu.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(GUI::ThemeEngine::kImageLogoSmall));
	} else {
		StaticTextWidget *title = new StaticTextWidget(this, "GlobalMenu.Title", "ScummVM");
		title->setAlign(Graphics::kTextAlignCenter);
	}
#else
	StaticTextWidget *title = new StaticTextWidget(this, "GlobalMenu.Title", "ScummVM");
	title->setAlign(Graphics::kTextAlignCenter);
#endif

	StaticTextWidget *version = new StaticTextWidget(this, "GlobalMenu.Version", gScummVMVersionDate);
	version->setAlign(Graphics::kTextAlignCenter);

	new GUI::ButtonWidget(this, "GlobalMenu.Resume", _("Resume"), kPlayCmd, 'P');

	_loadButton = new GUI::ButtonWidget(this, "GlobalMenu.Load", _("Load"), kLoadCmd, 'L');
	// TODO: setEnabled -> setVisible
	_loadButton->setEnabled(_engine->hasFeature(Engine::kSupportsLoadingDuringRuntime));

	_saveButton = new GUI::ButtonWidget(this, "GlobalMenu.Save", _("Save"), kSaveCmd, 'S');
	// TODO: setEnabled -> setVisible
	_saveButton->setEnabled(_engine->hasFeature(Engine::kSupportsSavingDuringRuntime));

	new GUI::ButtonWidget(this, "GlobalMenu.Options", _("Options"), kOptionsCmd, 'O');

	// The help button is disabled by default.
	// To enable "Help", an engine needs to use a subclass of MainMenuDialog
	// (at least for now, we might change how this works in the future).
	_helpButton = new GUI::ButtonWidget(this, "GlobalMenu.Help", _("Help"), kHelpCmd, 'H');
	_helpButton->setEnabled(false);

	new GUI::ButtonWidget(this, "GlobalMenu.About", _("About"), kAboutCmd, 'A');

	_rtlButton = new GUI::ButtonWidget(this, "GlobalMenu.RTL", _("Return to Launcher"), kRTLCmd, 'R');
	_rtlButton->setEnabled(_engine->hasFeature(Engine::kSupportsRTL));


	new GUI::ButtonWidget(this, "GlobalMenu.Quit", _("Quit"), kQuitCmd, 'Q');

	_aboutDialog = new GUI::AboutDialog();
	_optionsDialog = new ConfigDialog(_engine->hasFeature(Engine::kSupportsSubtitleOptions));
	_loadDialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"));
	_loadDialog->setSaveMode(false);
	_saveDialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"));
	_saveDialog->setSaveMode(true);
}

MainMenuDialog::~MainMenuDialog() {
	delete _aboutDialog;
	delete _optionsDialog;
	delete _loadDialog;
	delete _saveDialog;
}

void MainMenuDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kPlayCmd:
		close();
		break;
	case kLoadCmd:
		load();
		break;
	case kSaveCmd:
		save();
		break;
	case kOptionsCmd:
		_optionsDialog->runModal();
		break;
	case kAboutCmd:
		_aboutDialog->runModal();
		break;
	case kHelpCmd:
		// Not handled here -- needs to be handled by a subclass (for now)
		break;
	case kRTLCmd: {
		Common::Event eventRTL;
		eventRTL.type = Common::EVENT_RTL;
		g_system->getEventManager()->pushEvent(eventRTL);
		close();
		}
		break;
	case kQuitCmd: {
		Common::Event eventQ;
		eventQ.type = Common::EVENT_QUIT;
		g_system->getEventManager()->pushEvent(eventQ);
		close();
		}
		break;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

void MainMenuDialog::reflowLayout() {
	if (_engine->hasFeature(Engine::kSupportsLoadingDuringRuntime))
		_loadButton->setEnabled(_engine->canLoadGameStateCurrently());
	if (_engine->hasFeature(Engine::kSupportsSavingDuringRuntime))
		_saveButton->setEnabled(_engine->canSaveGameStateCurrently());

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowGlobalMenuLogo", 0) == 1 && g_gui.theme()->supportsImages()) {
		if (!_logo)
			_logo = new GUI::GraphicsWidget(this, "GlobalMenu.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(GUI::ThemeEngine::kImageLogoSmall));

		GUI::StaticTextWidget *title = (StaticTextWidget *)findWidget("GlobalMenu.Title");
		if (title) {
			removeWidget(title);
			title->setNext(0);
			delete title;
		}
	} else {
		GUI::StaticTextWidget *title = (StaticTextWidget *)findWidget("GlobalMenu.Title");
		if (!title) {
			title = new StaticTextWidget(this, "GlobalMenu.Title", "ScummVM");
			title->setAlign(Graphics::kTextAlignCenter);
		}

		if (_logo) {
			removeWidget(_logo);
			_logo->setNext(0);
			delete _logo;
			_logo = 0;
		}
	}
#endif

	Dialog::reflowLayout();
}

void MainMenuDialog::save() {
	Common::String gameId = ConfMan.get("gameid");

	const EnginePlugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);

	int slot = _saveDialog->runModal(plugin, ConfMan.getActiveDomainName());

	if (slot >= 0) {
		Common::String result(_saveDialog->getResultString());
		if (result.empty()) {
			// If the user was lazy and entered no save name, come up with a default name.
			char buf[20];
			snprintf(buf, 20, "Save %d", slot + 1);
			_engine->saveGameState(slot, buf);
		} else {
			_engine->saveGameState(slot, result.c_str());
		}

		close();
	}
}

void MainMenuDialog::load() {
	Common::String gameId = ConfMan.get("gameid");

	const EnginePlugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);

	int slot = _loadDialog->runModal(plugin, ConfMan.getActiveDomainName());

	if (slot >= 0) {
		// FIXME: For now we just ignore the return
		// value, which is quite bad since it could
		// be a fatal loading error, which renders
		// the engine unusable.
		_engine->loadGameState(slot);
		close();
	}
}

enum {
	kKeysCmd = 'KEYS'
};

// FIXME: We use the empty string as domain name here. This tells the
// ConfigManager to use the 'default' domain for all its actions. We do that
// to get as close as possible to editing the 'active' settings.
//
// However, that requires bad & evil hacks in the ConfigManager code,
// and even then still doesn't work quite correctly.
// For example, if the transient domain contains 'false' for the 'fullscreen'
// flag, but the user used a hotkey to switch to windowed mode, then the dialog
// will display the wrong value anyway.
//
// Proposed solution consisting of multiple steps:
// 1) Add special code to the open() code that reads out everything stored
//    in the transient domain that is controlled by this dialog, and updates
//    the dialog accordingly.
// 2) Even more code is added to query the backend for current settings, like
//    the fullscreen mode flag etc., and also updates the dialog accordingly.
// 3) The domain being edited is set to the active game domain.
// 4) If the dialog is closed with the "OK" button, then we remove everything
//    stored in the transient domain (or at least everything corresponding to
//    switches in this dialog.
//    If OTOH the dialog is closed with "Cancel" we do no such thing.
//
// These changes will achieve two things at once: Allow us to get rid of using
//  "" as value for the domain, and in fact provide a somewhat better user
// experience at the same time.
ConfigDialog::ConfigDialog(bool subtitleControls)
	: GUI::OptionsDialog("", "GlobalConfig") {

	//
	// Sound controllers
	//

	addVolumeControls(this, "GlobalConfig.");
	setVolumeSettingsState(true); // could disable controls by GUI options

	//
	// Subtitle speed and toggle controllers
	//

	if (subtitleControls) {
		// Global talkspeed range of 0-255
		addSubtitleControls(this, "GlobalConfig.", 255);
		setSubtitleSettingsState(true); // could disable controls by GUI options
	}

	//
	// Add the buttons
	//

	new GUI::ButtonWidget(this, "GlobalConfig.Ok", _("OK"), GUI::kOKCmd, 'O');
	new GUI::ButtonWidget(this, "GlobalConfig.Cancel", _("Cancel"), GUI::kCloseCmd, 'C');

#ifdef SMALL_SCREEN_DEVICE
	new GUI::ButtonWidget(this, "GlobalConfig.Keys", _("Keys"), kKeysCmd, 'K');
	_keysDialog = NULL;
#endif
}

ConfigDialog::~ConfigDialog() {
#ifdef SMALL_SCREEN_DEVICE
	delete _keysDialog;
#endif
}

void ConfigDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kKeysCmd:

#ifdef SMALL_SCREEN_DEVICE
	//
	// Create the sub dialog(s)
	//
	_keysDialog = new GUI::KeysDialog();
	_keysDialog->runModal();
	delete _keysDialog;
	_keysDialog = NULL;
#endif
		break;
	default:
		GUI::OptionsDialog::handleCommand (sender, cmd, data);
	}
}

