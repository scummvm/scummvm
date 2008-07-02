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

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/events.h"

#include "graphics/scaler.h"

#include "gui/about.h"
#include "gui/eval.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"

#include "engines/dialogs.h"
#include "engines/engine.h"

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

GlobalDialog::GlobalDialog(String name)
	: GUI::Dialog(name) {
_drawingHints |= GUI::THEME_HINT_SPECIAL_COLOR;}

enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kHelpCmd = 'HELP',
	kAboutCmd = 'ABOU',
	kQuitCmd = 'QUIT',
	kRTLCmd = 'RTL',
	kChooseCmd = 'CHOS'
};

MainMenuDialog::MainMenuDialog(Engine *engine)
	: GlobalDialog("globalmain"), _engine(engine) {

	new GUI::ButtonWidget(this, "globalmain_resume", "Resume", kPlayCmd, 'P');

//	new GUI::ButtonWidget(this, "scummmain_load", "Load", kLoadCmd, 'L');
//	new GUI::ButtonWidget(this, "scummmain_save", "Save", kSaveCmd, 'S');

	new GUI::ButtonWidget(this, "globalmain_options", "Options", kOptionsCmd, 'O');

	new GUI::ButtonWidget(this, "globalmain_about", "About", kAboutCmd, 'A');

	new GUI::ButtonWidget(this, "globalmain_rtl", "Return to Launcher", kRTLCmd, 'R');	
	
	new GUI::ButtonWidget(this, "globalmain_quit", "Quit", kQuitCmd, 'Q');

	_aboutDialog = new GUI::AboutDialog();
	_optionsDialog = new ConfigDialog();
}

MainMenuDialog::~MainMenuDialog() {
	delete _aboutDialog;
	delete _optionsDialog;
}

void MainMenuDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kPlayCmd:
		close();
		break;
	case kOptionsCmd:
		_optionsDialog->runModal();
		break;
	case kAboutCmd:
		_aboutDialog->runModal();
		break;
	case kRTLCmd:
		//g_system->getEventManager()->setQuit();
		//g_system->getEventManager()->setRTL();
		_engine->_quit = true;
		_engine->_rtl = true;
		close();
		break;
	case kQuitCmd:
		//g_system->getEventManager()->setQuit();
		_engine->_quit = true;
		close();
		break;
	default:
		GlobalDialog::handleCommand(sender, cmd, data);
	}
}

enum {
	kOKCmd = 'ok  '
};

enum {
	kKeysCmd = 'KEYS'
};

ConfigDialog::ConfigDialog()
	: GUI::OptionsDialog("", "scummconfig") {

	//
	// Sound controllers
	//

	addVolumeControls(this, "scummconfig_");

	//
	// Some misc options
	//

	// SCUMM has a talkspeed range of 0-9
	addSubtitleControls(this, "scummconfig_", 9);

	//
	// Add the buttons
	//

	new GUI::ButtonWidget(this, "scummconfig_ok", "OK", GUI::OptionsDialog::kOKCmd, 'O');
	new GUI::ButtonWidget(this, "scummconfig_cancel", "Cancel", kCloseCmd, 'C');

#ifdef SMALL_SCREEN_DEVICE
	new GUI::ButtonWidget(this, "scummconfig_keys", "Keys", kKeysCmd, 'K');

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

