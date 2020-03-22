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
#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "common/gui_options.h"
#include "common/system.h"
#include "common/translation.h"

#ifdef ENABLE_MYST
#include "mohawk/myst.h"
#include "mohawk/myst_actions.h"
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

MystOptionsWidget::MystOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "MystOptionsDialog", false, domain),
		_zipModeCheckbox(nullptr),
		_transitionsCheckbox(nullptr),
		_mystFlyByCheckbox(nullptr),
		_dropPageButton(nullptr),
		_showMapButton(nullptr),
		_returnToMenuButton(nullptr) {

	if (!checkGameGUIOption(GAMEOPTION_DEMO, ConfMan.get("guioptions", _domain))) {
		// I18N: Option for fast scene switching
		_zipModeCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "MystOptionsDialog.ZipMode", _("~Z~ip Mode Activated"));
	}

	_transitionsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "MystOptionsDialog.Transistions", _("~T~ransitions Enabled"));

	if (checkGameGUIOption(GAMEOPTION_ME, ConfMan.get("guioptions", _domain))) {
		_mystFlyByCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "MystOptionsDialog.PlayMystFlyBy", _("Play the Myst fly by movie"),
		                                             _("The Myst fly by movie was not played by the original engine."));
	}

	if (isInGame()) {
		MohawkEngine_Myst *vm = static_cast<MohawkEngine_Myst *>(g_engine);
		assert(vm);

		// I18N: Drop book page
		_dropPageButton = new GUI::ButtonWidget(widgetsBoss(), "MystOptionsDialog.DropPage", _("~D~rop Page"), nullptr, kDropCmd);

		// Myst ME only has maps
		if (vm->getFeatures() & GF_ME) {
			_showMapButton = new GUI::ButtonWidget(widgetsBoss(), "MystOptionsDialog.ShowMap", _("Show ~M~ap"), nullptr, kMapCmd);
		}

		// Myst demo only has a menu
		if (vm->getFeatures() & GF_DEMO) {
			_returnToMenuButton = new GUI::ButtonWidget(widgetsBoss(), "MystOptionsDialog.MainMenu", _("Main Men~u~"), nullptr, kMenuCmd);
		}
	}
}

MystOptionsWidget::~MystOptionsWidget() {
}

void MystOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
	            .addLayout(GUI::ThemeLayout::kLayoutVertical)
	                .addPadding(16, 16, 16, 16)
	                .addWidget("ZipMode", "Checkbox")
	                .addWidget("Transistions", "Checkbox")
	                .addWidget("PlayMystFlyBy", "Checkbox")
	                .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	                    .addPadding(0, 0, 16, 0)
	                    .addSpace()
	                    .addWidget("DropPage", "Button")
	                    .addWidget("ShowMap",  "Button")
	                    .addWidget("MainMenu", "Button")
	                    .addSpace()
	                .closeLayout()
	            .closeLayout()
	        .closeDialog();
}

bool MystOptionsWidget::isInGame() const {
	return _domain.equals(ConfMan.getActiveDomainName());
}

void MystOptionsWidget::load() {
	if (_zipModeCheckbox) {
		_zipModeCheckbox->setState(ConfMan.getBool("zip_mode", _domain));
	}

	_transitionsCheckbox->setState(ConfMan.getBool("transition_mode", _domain));

	if (_mystFlyByCheckbox) {
		_mystFlyByCheckbox->setState(ConfMan.getBool("playmystflyby", _domain));
	}

	if (isInGame()) {
		MohawkEngine_Myst *vm = static_cast<MohawkEngine_Myst *>(g_engine);
		assert(vm);

		_dropPageButton->setEnabled(vm->canDoAction(kMystActionDropPage));

		if (_showMapButton) {
			_showMapButton->setEnabled(vm->canDoAction(kMystActionShowMap));
		}

		if (_returnToMenuButton) {
			// Return to menu button is not enabled on the menu
			_returnToMenuButton->setEnabled(vm->canDoAction(kMystActionOpenMainMenu));
		}
	}
}

bool MystOptionsWidget::save() {
	if (_zipModeCheckbox) {
		ConfMan.setBool("zip_mode", _zipModeCheckbox->getState(), _domain);
	}

	ConfMan.setBool("transition_mode", _transitionsCheckbox->getState(), _domain);

	if (_mystFlyByCheckbox) {
		ConfMan.setBool("playmystflyby", _mystFlyByCheckbox->getState(), _domain);
	}

	return true;
}

void MystOptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	assert(_parentDialog);

	GUI::CommandSender dialog(_parentDialog);

	switch (cmd) {
	case kDropCmd:
		dialog.sendCommand(GUI::kCloseWithResultCmd, kMystActionDropPage);
		break;
	case kMapCmd:
		dialog.sendCommand(GUI::kCloseWithResultCmd, kMystActionShowMap);
		break;
	case kMenuCmd:
		dialog.sendCommand(GUI::kCloseWithResultCmd, kMystActionOpenMainMenu);
		break;
	default:
		OptionsContainerWidget::handleCommand(sender, cmd, data);
	}
}

MystMenuDialog::MystMenuDialog(Engine *engine) :
		MainMenuDialog(engine) {
}

MystMenuDialog::~MystMenuDialog() {
}

void MystMenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	MohawkEngine_Myst *vm = static_cast<MohawkEngine_Myst *>(_engine);
	assert(vm);

	switch (cmd) {
	case kOptionsCmd: {
		GUI::ConfigDialog configDialog;
		int result = configDialog.runModal();
		if (result > kMystActionNone && result <= kMystActionLast) {
			close();
			MystEventAction action = static_cast<MystEventAction>(result);
			vm->scheduleAction(action);
		}
		break;
	}
	case kQuitCmd:
		close();
		vm->runCredits();
		break;
	default:
		MainMenuDialog::handleCommand(sender, cmd, data);
		break;
	}
}

#endif

#ifdef ENABLE_RIVEN

RivenOptionsDialog::RivenOptionsDialog() : MohawkOptionsDialog() {
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

bool RivenOptionsDialog::getZipMode() const {
	return _zipModeCheckbox->getState();
}

void RivenOptionsDialog::setZipMode(bool enabled) {
	_zipModeCheckbox->setState(enabled);
}

bool RivenOptionsDialog::getWaterEffect() const {
	return _waterEffectCheckbox->getState();
}

void RivenOptionsDialog::setWaterEffect(bool enabled) {
	_waterEffectCheckbox->setState(enabled);
}

uint32 RivenOptionsDialog::getTransitions() const {
	return _transitionModePopUp->getSelectedTag();
}

void RivenOptionsDialog::setTransitions(uint32 mode) {
	_transitionModePopUp->setSelectedTag(mode);
}

void RivenOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case GUI::kOKCmd:
		setResult(1);
		close();
		break;
	default:
		MohawkOptionsDialog::handleCommand(sender, cmd, data);
	}
}

#endif

} // End of namespace Mohawk
