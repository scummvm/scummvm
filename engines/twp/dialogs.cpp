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

#include "common/translation.h"
#include "gui/gui-manager.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/popup.h"
#include "gui/ThemeEval.h"
#include "twp/dialogs.h"

namespace Twp {

static const char *lang_items[] = {"en", "fr", "it", "de", "es"};

TwpOptionsContainerWidget::TwpOptionsContainerWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) : OptionsContainerWidget(boss, name, "TwpGameOptionsDialog", false, domain) {
	GUI::StaticTextWidget *text = new GUI::StaticTextWidget(widgetsBoss(), "TwpGameOptionsDialog.VideoLabel", _("Video:"));
	text->setAlign(Graphics::TextAlign::kTextAlignStart);

	_enableToiletPaperOverGUICheckbox = new GUI::CheckboxWidget(widgetsBoss(), "TwpGameOptionsDialog.VideoCheck1",
																// I18N: Setting to switch toiled paper to be shown as "over".
																_("Toilet paper over"),
																_("The toilet paper in some toilets will be shown “over”.\nIt’s a joke option that has no effects on the gameplay.."));
	_enableAnnoyingInJokesGUICheckbox = new GUI::CheckboxWidget(widgetsBoss(), "TwpGameOptionsDialog.VideoCheck2",
																// I18N: Setting to enable or disable additional jokes in the game.
																_("Annoying in-jokes"),
																_("The game will include in-jokes and references to past adventure games, in the form of both dialogues and objects.\nThere is a game achievement that can be obtained only if the in-jokes option is switched on."));

	text = new GUI::StaticTextWidget(widgetsBoss(), "TwpGameOptionsDialog.ControlsLabel", _("Controls:"));
	text->setAlign(Graphics::TextAlign::kTextAlignStart);

	// I18N: Setting to invert verb colors or keep the original verb colors.
	_enableInvertVerbColorsGUICheckbox = new GUI::CheckboxWidget(widgetsBoss(), "TwpGameOptionsDialog.ControlsCheck1", _("Invert verb colors"), _(""));
	// I18N: Setting to use retro or modern fonts.
	_enableRetroFontsGUICheckbox = new GUI::CheckboxWidget(widgetsBoss(), "TwpGameOptionsDialog.ControlsCheck2", _("Retro Fonts"), _(""));
	// I18N: Setting to use retro or modern verbs.
	_enableRetroVerbsGUICheckbox = new GUI::CheckboxWidget(widgetsBoss(), "TwpGameOptionsDialog.ControlsCheck3", _("Retro Verbs"), _(""));
	// I18N: Setting to use classic sentence or modern sentence.
	_enableClassicSentenceGUICheckbox = new GUI::CheckboxWidget(widgetsBoss(), "TwpGameOptionsDialog.ControlsCheck4", _("Classic Sentence"), _(""));

	// I18N: Settings to enable or disable Ransome unbeeped DLC.
	_enableDLC = new GUI::CheckboxWidget(widgetsBoss(), "TwpGameOptionsDialog.TextCheck1", _("Ransome *unbeeped* (DLC)"), _(""));

	_langGUIDropdown = new GUI::PopUpWidget(widgetsBoss(), "TwpGameOptionsDialog.LangDropDown");
	_langGUIDropdown->appendEntry(_("English"));
	_langGUIDropdown->appendEntry(_("French"));
	_langGUIDropdown->appendEntry(_("Italian"));
	_langGUIDropdown->appendEntry(_("German"));
	_langGUIDropdown->appendEntry(_("Spanish"));
}

void TwpOptionsContainerWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(0, 0, 8, 8);

	layouts.addPadding(0, 0, 8, 8)
		.addSpace(10)
		.addWidget("VideoLabel", "OptionsLabel")
		.addWidget("VideoCheck1", "Checkbox")
		.addWidget("VideoCheck2", "Checkbox")
		.addWidget("ControlsLabel", "OptionsLabel")
		.addWidget("ControlsCheck1", "Checkbox")
		.addWidget("ControlsCheck2", "Checkbox")
		.addWidget("ControlsCheck3", "Checkbox")
		.addWidget("ControlsCheck4", "Checkbox")
		.addWidget("LangDropDown", "PopUp")
		.addWidget("TextCheck1", "Checkbox");

	layouts.closeLayout().closeDialog();
}

void TwpOptionsContainerWidget::load() {
	_enableToiletPaperOverGUICheckbox->setState(ConfMan.getBool("toiletPaperOver", _domain));
	_enableAnnoyingInJokesGUICheckbox->setState(ConfMan.getBool("annoyingInJokes", _domain));
	_enableInvertVerbColorsGUICheckbox->setState(ConfMan.getBool("invertVerbHighlight", _domain));
	_enableRetroFontsGUICheckbox->setState(ConfMan.getBool("retroFonts", _domain));
	_enableRetroVerbsGUICheckbox->setState(ConfMan.getBool("retroVerbs", _domain));
	_enableClassicSentenceGUICheckbox->setState(ConfMan.getBool("hudSentence", _domain));
	_enableDLC->setState(ConfMan.getBool("ransomeUnbeeped", _domain));
	Common::String lang = ConfMan.get("language", _domain);
	int index = 0;
	for (int i = 0; i < ARRAYSIZE(lang_items); i++) {
		if (lang == lang_items[i]) {
			index = i;
			break;
		}
	}
	_langGUIDropdown->setSelected(index);
}

bool TwpOptionsContainerWidget::save() {
	ConfMan.setBool("toiletPaperOver", _enableToiletPaperOverGUICheckbox->getState(), _domain);
	ConfMan.setBool("annoyingInJokes", _enableAnnoyingInJokesGUICheckbox->getState(), _domain);
	ConfMan.setBool("invertVerbHighlight", _enableInvertVerbColorsGUICheckbox->getState(), _domain);
	ConfMan.setBool("retroFonts", _enableRetroFontsGUICheckbox->getState(), _domain);
	ConfMan.setBool("retroVerbs", _enableRetroVerbsGUICheckbox->getState(), _domain);
	ConfMan.setBool("hudSentence", _enableClassicSentenceGUICheckbox->getState(), _domain);
	ConfMan.setBool("ransomeUnbeeped", _enableDLC->getState(), _domain);
	ConfMan.set("language", lang_items[_langGUIDropdown->getSelected()], _domain);
	return true;
}

} // namespace Twp
