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
#include "gui/message.h"
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
#include "mohawk/myst_metaengine.h"
#endif

#ifdef ENABLE_RIVEN
#include "mohawk/riven.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_metaengine.h"
#endif

namespace Mohawk {

// This used to have GUI::Dialog("MohawkDummyDialog"), but that doesn't work with the gui branch merge :P (Sorry, Tanoku!)
InfoDialog::InfoDialog(MohawkEngine *vm, const Common::U32String &message) : _vm(vm), GUI::Dialog(0, 0, 1, 1), _message(message) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;

	_text = new GUI::StaticTextWidget(this, 0, 0, 10, 10, _message, Graphics::kTextAlignCenter);
}

void InfoDialog::setInfoText(const Common::U32String &message) {
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

PauseDialog::PauseDialog(MohawkEngine *vm, const Common::U32String &message) : InfoDialog(vm, message) {
}

void PauseDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == ' ')
		close();
	else
		InfoDialog::handleKeyDown(state);
}

enum {
	kDropCmd = 'DROP',
	kMapCmd  = 'SMAP',
	kMenuCmd = 'MENU'
};

#ifdef ENABLE_MYST

MystOptionsWidget::MystOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "MystOptionsDialog", false, domain),
		_zipModeCheckbox(nullptr),
		_transitionsCheckbox(nullptr),
		_mystFlyByCheckbox(nullptr),
		_languagePopUp(nullptr),
		_dropPageButton(nullptr),
		_showMapButton(nullptr),
		_returnToMenuButton(nullptr) {
	Common::String guiOptions = ConfMan.get("guioptions", _domain);
	bool isDemo = checkGameGUIOption(GAMEOPTION_DEMO, guiOptions);
	bool isME = checkGameGUIOption(GAMEOPTION_ME, guiOptions);

	if (!isDemo) {
		// I18N: Option for fast scene switching
		_zipModeCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "MystOptionsDialog.ZipMode", _("~Z~ip Mode Activated"));
	}

	_transitionsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "MystOptionsDialog.Transistions", _("~T~ransitions Enabled"));

	if (isME) {
		_mystFlyByCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "MystOptionsDialog.PlayMystFlyBy", _("Play the Myst fly by movie"),
		                                             _("The Myst fly by movie was not played by the original engine."));
	}

	if (isInGame()) {
		MohawkEngine_Myst *vm = static_cast<MohawkEngine_Myst *>(g_engine);
		assert(vm);

		// I18N: Drop book page
		_dropPageButton = new GUI::ButtonWidget(widgetsBoss(), "MystOptionsDialog.DropPage", _("~D~rop Page"), Common::U32String(), kDropCmd);

		// Myst ME only has maps
		if (vm->isGameVariant(GF_ME)) {
			_showMapButton = new GUI::ButtonWidget(widgetsBoss(), "MystOptionsDialog.ShowMap", _("Show ~M~ap"), Common::U32String(), kMapCmd);
		}

		// Myst demo only has a menu
		if (vm->isGameVariant(GF_DEMO)) {
			_returnToMenuButton = new GUI::ButtonWidget(widgetsBoss(), "MystOptionsDialog.MainMenu", _("Main Men~u~"), Common::U32String(), kMenuCmd);
		}

		if (vm->isGameVariant(GF_25TH)) {
			GUI::StaticTextWidget *languageCaption = new GUI::StaticTextWidget(widgetsBoss(), "MystOptionsDialog.LanguageDesc", _("Language:"));
			languageCaption->setAlign(Graphics::kTextAlignRight);

			_languagePopUp = new GUI::PopUpWidget(widgetsBoss(), "MystOptionsDialog.Language");

			const MystLanguage *languages = MohawkMetaEngine_Myst::listLanguages();
			while (languages->language != Common::UNK_LANG) {
				_languagePopUp->appendEntry(Common::getLanguageDescription(languages->language), languages->language);
				languages++;
			}
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
	                    .addPadding(0, 0, 0, 0)
	                    .addWidget("LanguageDesc", "OptionsLabel")
	                    .addWidget("Language", "PopUp")
	                .closeLayout()
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

	if (_languagePopUp) {
		Common::Language language = Common::parseLanguage(ConfMan.get("language", _domain));
		const MystLanguage *languageDesc = MohawkEngine_Myst::getLanguageDesc(language);
		if (languageDesc) {
			_languagePopUp->setSelectedTag(languageDesc->language);
		}
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

	if (_languagePopUp) {
		MohawkEngine_Myst *vm = static_cast<MohawkEngine_Myst *>(g_engine);
		assert(vm);

		int32 selectedLanguage = _languagePopUp->getSelectedTag();
		const MystLanguage *languageDesc = nullptr;
		if (selectedLanguage >= 0) {
			languageDesc = MohawkEngine_Myst::getLanguageDesc(static_cast<Common::Language>(selectedLanguage));
		}

		Common::Language newLanguage = Common::UNK_LANG;
		if (languageDesc != nullptr) {
			newLanguage = languageDesc->language;
			ConfMan.set("language", Common::getLanguageCode(languageDesc->language));
		}

		Common::Language currentLanguage = vm->getLanguage();
		if (newLanguage != currentLanguage && vm->isGameStarted()) {
			GUI::MessageDialog dialog(_("The new language will be applied after restarting the game."));
			dialog.runModal();
		}
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
		vm->saveAutosaveIfEnabled();
		vm->runCredits();
		break;
	default:
		MainMenuDialog::handleCommand(sender, cmd, data);
		break;
	}
}

#endif

#ifdef ENABLE_RIVEN

RivenOptionsWidget::RivenOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "RivenOptionsDialog", false, domain),
		_languagePopUp(nullptr) {
	Common::String guiOptions = ConfMan.get("guioptions", domain);
	bool is25th = checkGameGUIOption(GAMEOPTION_25TH, guiOptions);

	_zipModeCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "RivenOptionsDialog.ZipMode", _("~Z~ip Mode Activated"));
	_waterEffectCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "RivenOptionsDialog.WaterEffect", _("~W~ater Effect Enabled"));

	GUI::StaticTextWidget *transitionModeCaption = new GUI::StaticTextWidget(widgetsBoss(), "RivenOptionsDialog.TransistionsDesc", _("Transitions:"));
	transitionModeCaption->setAlign(Graphics::kTextAlignRight);

	_transitionModePopUp = new GUI::PopUpWidget(widgetsBoss(), "RivenOptionsDialog.Transistions");
	_transitionModePopUp->appendEntry(_("Disabled"), kRivenTransitionModeDisabled);
	_transitionModePopUp->appendEntry(_("Fastest"), kRivenTransitionModeFastest);
	_transitionModePopUp->appendEntry(_("Normal"), kRivenTransitionModeNormal);
	_transitionModePopUp->appendEntry(_("Best"), kRivenTransitionModeBest);

	// Only the 25th anniversary edition is multi-language
	// Only allow changing the language at run-time, so that there is only one
	//  language selection drop down in the edit game dialog.
	if (is25th && g_engine) {
		bool canChangeLanguage = true;
		MohawkEngine_Riven *vm = static_cast<MohawkEngine_Riven *>(g_engine);
		canChangeLanguage = vm->isInteractive() || vm->isInMainMenu();

		GUI::StaticTextWidget *languageCaption = new GUI::StaticTextWidget(widgetsBoss(), "RivenOptionsDialog.LanguageDesc", _("Language:"));
		languageCaption->setAlign(Graphics::kTextAlignRight);

		_languagePopUp = new GUI::PopUpWidget(widgetsBoss(), "RivenOptionsDialog.Language");
		_languagePopUp->setEnabled(canChangeLanguage);

		const RivenLanguage *languages = MohawkMetaEngine_Riven::listLanguages();
		while (languages->language != Common::UNK_LANG) {
			_languagePopUp->appendEntry(Common::getLanguageDescription(languages->language), languages->language);
			languages++;
		}
	}
}

RivenOptionsWidget::~RivenOptionsWidget() {
}

void RivenOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
	        .addLayout(GUI::ThemeLayout::kLayoutVertical)
	            .addPadding(16, 16, 16, 16)
	            .addWidget("ZipMode", "Checkbox")
	            .addWidget("WaterEffect", "Checkbox")
	            .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	                .addPadding(0, 0, 0, 0)
	                .addWidget("TransistionsDesc", "OptionsLabel")
	                .addWidget("Transistions", "PopUp")
	            .closeLayout()
	            .addLayout(GUI::ThemeLayout::kLayoutHorizontal)
	                .addPadding(0, 0, 0, 0)
	                .addWidget("LanguageDesc", "OptionsLabel")
	                .addWidget("Language", "PopUp")
	            .closeLayout()
	        .closeLayout()
	    .closeDialog();
}

void RivenOptionsWidget::load() {
	_zipModeCheckbox->setState(ConfMan.getBool("zip_mode", _domain));
	_waterEffectCheckbox->setState(ConfMan.getBool("water_effects", _domain));

	uint32 transitions = ConfMan.getInt("transition_mode", _domain);
	_transitionModePopUp->setSelectedTag(RivenGraphics::sanitizeTransitionMode(transitions));

	if (_languagePopUp) {
		Common::Language language = Common::parseLanguage(ConfMan.get("language", _domain));
		const RivenLanguage *languageDesc = MohawkEngine_Riven::getLanguageDesc(language);
		if (languageDesc) {
			_languagePopUp->setSelectedTag(languageDesc->language);
		}
	}
}

bool RivenOptionsWidget::save() {
	ConfMan.setBool("zip_mode", _zipModeCheckbox->getState(), _domain);
	ConfMan.setBool("water_effects", _waterEffectCheckbox->getState(), _domain);
	ConfMan.setInt("transition_mode", _transitionModePopUp->getSelectedTag(), _domain);

	if (_languagePopUp) {
		int32 selectedLanguage = _languagePopUp->getSelectedTag();
		const RivenLanguage *languageDesc = nullptr;
		if (selectedLanguage >= 0) {
			languageDesc = MohawkEngine_Riven::getLanguageDesc(static_cast<Common::Language>(selectedLanguage));
		}

		if (languageDesc != nullptr) {
			ConfMan.set("language", Common::getLanguageCode(languageDesc->language));
		}
	}

	return true;
}

#endif

} // End of namespace Mohawk
