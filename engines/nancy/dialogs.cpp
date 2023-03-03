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

#include "common/formats/winexe.h"
#include "common/translation.h"
#include "common/stream.h"

#include "gui/widgets/tab.h"
#include "gui/widgets/edittext.h"

#include "gui/ThemeEval.h"

#include "engines/nancy/dialogs.h"
#include "engines/nancy/iff.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {

NancyOptionsWidget::NancyOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "NancyOptionsDialog", false, domain) {
	_playerSpeechCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "NancyOptionsDialog.PlayerSpeech", _("Player Speech"), _("Enable player speech. Only works if speech is enabled in the Audio settings."));
	_characterSpeechCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "NancyOptionsDialog.CharacterSpeech", _("Character Speech"), _("Enable NPC speech. Only works if speech is enabled in the Audio settings."));
	_originalMenusCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "NancyOptionsDialog.OriginalMenus", _("Use original menus"), _("Use the original engine's main, save/load, and setup menus. ScummVM's Global Main Menu can still be accessed through its keymap."));

	// I18N: Second Chance is the name of the original engine's autosave system
	_secondChanceCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "NancyOptionsDialog.SecondChance", _("Enable Second Chance"), _("Enable the Second Chance feature, which automatically saves at specific scenes. Enabling this disables timed autosaves."));

	new GUI::StaticTextWidget(widgetsBoss(), "NancyOptionsDialog.SpeechSettingsLabel", _("Speech Options"));
	new GUI::StaticTextWidget(widgetsBoss(), "NancyOptionsDialog.EngineSettingsLabel", _("Engine Options"));
}

void NancyOptionsWidget::load() {
	_playerSpeechCheckbox->setState(ConfMan.getBool("player_speech", _domain));
	_characterSpeechCheckbox->setState(ConfMan.getBool("character_speech", _domain));
	_originalMenusCheckbox->setState(ConfMan.getBool("original_menus", _domain));
	_secondChanceCheckbox->setState(ConfMan.getBool("second_chance", _domain));
}

bool NancyOptionsWidget::save() {
	ConfMan.setBool("player_speech", _playerSpeechCheckbox->getState(), _domain);
	ConfMan.setBool("character_speech", _characterSpeechCheckbox->getState(), _domain);
	ConfMan.setBool("original_menus", _originalMenusCheckbox->getState(), _domain);
	ConfMan.setBool("second_chance", _secondChanceCheckbox->getState(), _domain);

	return true;
}

void NancyOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
			.addPadding(16, 16, 16, 16)
			.addWidget("SpeechSettingsLabel", "OptionsLabel")
			.addWidget("PlayerSpeech", "Checkbox")
			.addWidget("CharacterSpeech", "Checkbox")
			.addSpace(16)
			.addWidget("EngineSettingsLabel", "OptionsLabel")
			.addWidget("OriginalMenus", "Checkbox")
			.addWidget("SecondChance", "Checkbox")
		.closeLayout()
	.closeDialog();
}

bool NancyOptionsWidget::isInGame() const {
	return _domain.equals(ConfMan.getActiveDomainName());
}

} // End of namespace Nancy
