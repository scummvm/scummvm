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

#include "graphics/scaler.h"

#include "gui/about.h"
#include "gui/newgui.h"
#include "gui/launcher.h"
#include "gui/ListWidget.h"
#include "gui/theme.h"

#include "engines/dialogs.h"
#include "engines/engine.h"
#include "engines/metaengine.h"

#ifdef SMALL_SCREEN_DEVICE
#include "gui/KeysDialog.h"
#endif

using GUI::CommandSender;
using GUI::StaticTextWidget;
using GUI::kButtonWidth;
using GUI::kButtonHeight;
using GUI::kBigButtonWidth;
using GUI::kBigButtonHeight;
using GUI::kCloseCmd;
using GUI::kTextAlignCenter;
using GUI::kTextAlignLeft;
using GUI::WIDGET_ENABLED;

typedef GUI::OptionsDialog GUI_OptionsDialog;
typedef GUI::Dialog GUI_Dialog;

GlobalDialog::GlobalDialog(String name) : GUI::Dialog(name) {}

enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kHelpCmd = 'HELP',
	kAboutCmd = 'ABOU',
	kQuitCmd = 'QUIT',
	kRTLCmd = 'RTL ',
	kChooseCmd = 'CHOS'
};

MainMenuDialog::MainMenuDialog(Engine *engine)
	: GlobalDialog("GlobalMenu"), _engine(engine) {
	_backgroundType = GUI::Theme::kDialogBackgroundSpecial;

#ifndef DISABLE_FANCY_THEMES
	_logo = 0;
	if (g_gui.xmlEval()->getVar("Globals.ShowGlobalMenuLogo", 0) == 1 && g_gui.theme()->supportsImages()) {
		_logo = new GUI::GraphicsWidget(this, "GlobalMenu.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(GUI::Theme::kImageLogoSmall));
	} else {
		StaticTextWidget *title = new StaticTextWidget(this, "GlobalMenu.Title", "ScummVM");
		title->setAlign(GUI::kTextAlignCenter);
	}
#else
	StaticTextWidget *title = new StaticTextWidget(this, "GlobalMenu.Title", "ScummVM");
	title->setAlign(GUI::kTextAlignCenter);
#endif

	StaticTextWidget *version = new StaticTextWidget(this, "GlobalMenu.Version", gScummVMVersionDate);
	version->setAlign(GUI::kTextAlignCenter);
		
	new GUI::ButtonWidget(this, "GlobalMenu.Resume", "Resume", kPlayCmd, 'P');

	_loadButton = new GUI::ButtonWidget(this, "GlobalMenu.Load", "Load", kLoadCmd, 'L');
	// TODO: setEnabled -> setVisible
	_loadButton->setEnabled(_engine->hasFeature(Engine::kSupportsListSaves) && 
							_engine->hasFeature(Engine::kSupportsLoadingDuringRuntime));
 
	_saveButton = new GUI::ButtonWidget(this, "GlobalMenu.Save", "Save", kSaveCmd, 'S');
	// TODO: setEnabled -> setVisible
	_saveButton->setEnabled(_engine->hasFeature(Engine::kSupportsListSaves) && 
							_engine->hasFeature(Engine::kSupportsSavingDuringRuntime));

	new GUI::ButtonWidget(this, "GlobalMenu.Options", "Options", kOptionsCmd, 'O');

	new GUI::ButtonWidget(this, "GlobalMenu.About", "About", kAboutCmd, 'A');

	_rtlButton = new GUI::ButtonWidget(this, "GlobalMenu.RTL", "Return to Launcher", kRTLCmd, 'R');	
	_rtlButton->setEnabled(_engine->hasFeature(Engine::kSupportsRTL));


	new GUI::ButtonWidget(this, "GlobalMenu.Quit", "Quit", kQuitCmd, 'Q');

	_aboutDialog = new GUI::AboutDialog();
	_optionsDialog = new ConfigDialog();
	_loadDialog = new GUI::SaveLoadChooser("Load game:", "Load");
}

MainMenuDialog::~MainMenuDialog() {
	delete _aboutDialog;
	delete _optionsDialog;
	delete _loadDialog;
	//delete _saveDialog;
}

void MainMenuDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kPlayCmd:
		close();
		break;
	case kLoadCmd:
		{
		String gameId = ConfMan.get("gameid");

		const EnginePlugin *plugin = 0;
		EngineMan.findGame(gameId, &plugin);

		int slot = _loadDialog->runModal(plugin, ConfMan.getActiveDomainName());

		if (slot >= 0) {
			_engine->loadGameState(slot);
			close();
		}

		}
		break;
	case kSaveCmd:
		/*
		String gameId = ConfMan.get("gameid");

		const EnginePlugin *plugin = 0;
		EngineMan.findGame(gameId, &plugin);

		int slot = _saveDialog->runModal(plugin, ConfMan.getActiveDomainName());

		if (slot >= 0) {
			_engine->saveGameState(slot);
			close();
		}

		}
		*/
		break;
	case kOptionsCmd:
		_optionsDialog->runModal();
		break;
	case kAboutCmd:
		_aboutDialog->runModal();
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
		GlobalDialog::handleCommand(sender, cmd, data);
	}
}

void MainMenuDialog::reflowLayout() {
	if (_engine->hasFeature(Engine::kSupportsListSaves)) {
		if (_engine->hasFeature(Engine::kSupportsLoadingDuringRuntime)) 
			_loadButton->setEnabled(_engine->canLoadGameStateCurrently());
		if (_engine->hasFeature(Engine::kSupportsSavingDuringRuntime))
			_saveButton->setEnabled(_engine->canSaveGameStateCurrently());
	}

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowGlobalMenuLogo", 0) == 1 && g_gui.theme()->supportsImages()) {
		if (!_logo)
			_logo = new GUI::GraphicsWidget(this, "GlobalMenu.Logo");
		_logo->useThemeTransparency(true);
		_logo->setGfx(g_gui.theme()->getImageSurface(GUI::Theme::kImageLogoSmall));

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
			title->setAlign(GUI::kTextAlignCenter);
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

enum {
	kOKCmd = 'ok  '
};

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
ConfigDialog::ConfigDialog()
	: GUI::OptionsDialog("", "ScummConfig") {

	//
	// Sound controllers
	//

	addVolumeControls(this, "ScummConfig.");

	//
	// Some misc options
	//

	// SCUMM has a talkspeed range of 0-9
	addSubtitleControls(this, "ScummConfig.", 9);

	//
	// Add the buttons
	//

	new GUI::ButtonWidget(this, "ScummConfig.Ok", "OK", GUI::OptionsDialog::kOKCmd, 'O');
	new GUI::ButtonWidget(this, "ScummConfig.Cancel", "Cancel", kCloseCmd, 'C');

#ifdef SMALL_SCREEN_DEVICE
	new GUI::ButtonWidget(this, "ScummConfig.Keys", "Keys", kKeysCmd, 'K');

	//
	// Create the sub dialog(s)
	//

	_keysDialog = new GUI::KeysDialog();
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
		_keysDialog->runModal();
#endif
		break;
	default:
		GUI_OptionsDialog::handleCommand (sender, cmd, data);
	}
}

