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

#include "mohawk/mohawk.h"
#include "mohawk/dialogs.h"

#include "gui/gui-manager.h"
#include "gui/saveload.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"
#include "common/system.h"
#include "common/translation.h"

#ifdef ENABLE_MYST
#include "mohawk/myst.h"
#include "mohawk/myst_scripts.h"
#endif

#ifdef ENABLE_RIVEN
#include "mohawk/riven.h"
#include "mohawk/riven_graphics.h"
#endif

namespace Mohawk {

// This used to have GUI::Dialog("MohawkDummyDialog"), but that doesn't work with the gui branch merge :P (Sorry, Tanoku!)
InfoDialog::InfoDialog(MohawkEngine *vm, const Common::String &message) : _vm(vm), GUI::Dialog(0, 0, 1, 1), _message(message) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;

	_text = new GUI::StaticTextWidget(this, 0, 0, 10, 10, _message, Graphics::kTextAlignCenter);
}

void InfoDialog::setInfoText(const Common::String &message) {
	_message = message;
	_text->setLabel(_message);
}

void InfoDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int width = g_gui.getStringWidth(_message) + 16;
	int height = g_gui.getFontHeight() + 8;

	_w = width;
	_h = height;
	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;

	_text->setSize(_w, _h);
}

PauseDialog::PauseDialog(MohawkEngine *vm, const Common::String &message) : InfoDialog(vm, message) {
}

void PauseDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == ' ')
		close();
	else
		InfoDialog::handleKeyDown(state);
}

enum {
	kZipCmd = 'ZIPM',
	kTransCmd = 'TRAN',
	kWaterCmd = 'WATR',
	kDropCmd = 'DROP',
	kMapCmd = 'SMAP',
	kMenuCmd = 'MENU',
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kQuitCmd = 'QUIT'
};

#if defined(ENABLE_MYST) || defined(ENABLE_RIVEN)

MohawkOptionsDialog::MohawkOptionsDialog() :
		GUI::Dialog(0, 0, 360, 200) {
	new GUI::ButtonWidget(this, 95, 160, 120, 25, _("~O~K"), nullptr, GUI::kOKCmd);
	new GUI::ButtonWidget(this, 225, 160, 120, 25, _("~C~ancel"), nullptr, GUI::kCloseCmd);
}

MohawkOptionsDialog::~MohawkOptionsDialog() {
}

void MohawkOptionsDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	// Center the dialog
	_x = (screenW - getWidth()) / 2;
	_y = (screenH - getHeight()) / 2;

	GUI::Dialog::reflowLayout();
}


void MohawkOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
		case GUI::kCloseCmd:
			close();
			break;
		default:
			GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

#endif

#ifdef ENABLE_MYST

MystOptionsDialog::MystOptionsDialog(MohawkEngine_Myst* vm) :
		MohawkOptionsDialog(),
		_vm(vm),
		_canDropPage(false),
		_canShowMap(false),
		_canReturnToMenu(false),
		_loadSlot(-1),
		_saveSlot(-1) {

	_loadButton = new GUI::ButtonWidget(this, 245, 25, 100, 25, _("~L~oad"), nullptr, kLoadCmd);
	_saveButton = new GUI::ButtonWidget(this, 245, 60, 100, 25, _("~S~ave"), nullptr, kSaveCmd);
	_quitButton = new GUI::ButtonWidget(this, 245, 95, 100, 25, _("~Q~uit"), nullptr, kQuitCmd);

	// I18N: Option for fast scene switching
	_zipModeCheckbox = new GUI::CheckboxWidget(this, 15, 10, 220, 15, _("~Z~ip Mode Activated"), nullptr, kZipCmd);
	_transitionsCheckbox = new GUI::CheckboxWidget(this, 15, 30, 220, 15, _("~T~ransitions Enabled"), nullptr, kTransCmd);
	// I18N: Drop book page
	_dropPageButton = new GUI::ButtonWidget(this, 15, 60, 100, 25, _("~D~rop Page"), nullptr, kDropCmd);

	// Myst ME only has maps
	if (_vm->getFeatures() & GF_ME)
		_showMapButton = new GUI::ButtonWidget(this, 15, 95, 100, 25, _("Show ~M~ap"), nullptr, kMapCmd);
	else
		_showMapButton = nullptr;

	// Myst demo only has a menu
	if (_vm->getFeatures() & GF_DEMO)
		_returnToMenuButton = new GUI::ButtonWidget(this, 15, 95, 100, 25, _("Main Men~u~"), nullptr, kMenuCmd);
	else
		_returnToMenuButton = nullptr;

	_loadDialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
	_saveDialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
}

MystOptionsDialog::~MystOptionsDialog() {
	delete _loadDialog;
	delete _saveDialog;
}

void MystOptionsDialog::open() {
	MohawkOptionsDialog::open();

	_dropPageButton->setEnabled(_canDropPage);

	if (_showMapButton) {
		_showMapButton->setEnabled(_canShowMap);
	}

	if (_returnToMenuButton) {
		// Return to menu button is not enabled on the menu
		_returnToMenuButton->setEnabled(_canReturnToMenu);
	}

	// Zip mode is disabled in the demo
	if (_vm->getFeatures() & GF_DEMO)
		_zipModeCheckbox->setEnabled(false);

	_zipModeCheckbox->setState(_vm->_gameState->_globals.zipMode);
	_transitionsCheckbox->setState(_vm->_gameState->_globals.transitions);

	if (_vm->getFeatures() & GF_25TH) {
		// The 25th anniversary version has a main menu, no need to show these buttons here
		_loadButton->setVisible(false);
		_saveButton->setVisible(false);
		_quitButton->setVisible(false);
	}

	_loadSlot = -1;
	_saveSlot = -1;
	_loadButton->setEnabled(_vm->canLoadGameStateCurrently());
	_saveButton->setEnabled(_vm->canSaveGameStateCurrently());
}

void MystOptionsDialog::save() {
	_saveSlot = _saveDialog->runModalWithCurrentTarget();

	if (_saveSlot >= 0) {
		_saveDescription = _saveDialog->getResultString();
		if (_saveDescription.empty()) {
			// If the user was lazy and entered no save name, come up with a default name.
			_saveDescription = _saveDialog->createDefaultSaveDescription(_saveSlot);
		}

		close();
	}
}

void MystOptionsDialog::load() {
	// Do not load the game state from insite the dialog loop to
	// avoid mouse cursor glitches (see bug #7164). Instead store
	// the slot to load and let the code exectuting the dialog do
	// the load after the dialog finished running.
	_loadSlot = _loadDialog->runModalWithCurrentTarget();

	if (_loadSlot >= 0)
		close();
}

void MystOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kLoadCmd:
		load();
		break;
	case kSaveCmd:
		save();
		break;
	case kDropCmd:
		setResult(kActionDropPage);
		close();
		break;
	case kMapCmd:
		setResult(kActionShowMap);
		close();
		break;
	case kMenuCmd:
		setResult(kActionGoToMenu);
		close();
		break;
	case kQuitCmd: {
		setResult(kActionShowCredits);
		close();
	}
		break;
	case GUI::kOKCmd:
		_vm->_gameState->_globals.zipMode = _zipModeCheckbox->getState();
		_vm->_gameState->_globals.transitions = _transitionsCheckbox->getState();
		setResult(kActionNone);
		close();
		break;
	default:
		MohawkOptionsDialog::handleCommand(sender, cmd, data);
	}
}

void MystOptionsDialog::setCanDropPage(bool canDropPage) {
	_canDropPage = canDropPage;
}

void MystOptionsDialog::setCanShowMap(bool canShowMap) {
	_canShowMap = canShowMap;
}

void MystOptionsDialog::setCanReturnToMenu(bool canReturnToMenu) {
	_canReturnToMenu = canReturnToMenu;
}

#endif

#ifdef ENABLE_RIVEN

RivenOptionsDialog::RivenOptionsDialog(MohawkEngine_Riven* vm) :
		MohawkOptionsDialog(),
		_vm(vm) {
	_zipModeCheckbox = new GUI::CheckboxWidget(this, 15, 10, 220, 15, _("~Z~ip Mode Activated"), nullptr, kZipCmd);
	_waterEffectCheckbox = new GUI::CheckboxWidget(this, 15, 35, 220, 15, _("~W~ater Effect Enabled"), nullptr, kWaterCmd);

	_transitionModeCaption = new GUI::StaticTextWidget(this, 15, 60, 90, 20, _("Transitions:"), Graphics::kTextAlignRight);
	_transitionModePopUp = new GUI::PopUpWidget(this, 115, 60, 120, 20);
	_transitionModePopUp->appendEntry(_("Disabled"), kRivenTransitionModeDisabled);
	_transitionModePopUp->appendEntry(_("Fastest"), kRivenTransitionModeFastest);
	_transitionModePopUp->appendEntry(_("Normal"), kRivenTransitionModeNormal);
	_transitionModePopUp->appendEntry(_("Best"), kRivenTransitionModeBest);
}

RivenOptionsDialog::~RivenOptionsDialog() {
}

void RivenOptionsDialog::open() {
	MohawkOptionsDialog::open();

	_zipModeCheckbox->setState(_vm->_vars["azip"] != 0);
	_waterEffectCheckbox->setState(_vm->_vars["waterenabled"] != 0);
	_transitionModePopUp->setSelectedTag(_vm->_vars["transitionmode"]);
}

void RivenOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case GUI::kOKCmd:
		_vm->_vars["azip"] = _zipModeCheckbox->getState() ? 1 : 0;
		_vm->_vars["waterenabled"] = _waterEffectCheckbox->getState() ? 1 : 0;
		_vm->_vars["transitionmode"] = _transitionModePopUp->getSelectedTag();
		setResult(1);
		close();
		break;
	default:
		MohawkOptionsDialog::handleCommand(sender, cmd, data);
	}
}

#endif

} // End of namespace Mohawk
