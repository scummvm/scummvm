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

#include "base/plugins.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"

#include "darkseed/detection.h"
#include "darkseed/dialogs.h"

namespace Darkseed {

OptionsWidget::OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) : OptionsContainerWidget(boss, name, "DarkseedGameOptionsDialog", domain) {
	_guiOptions = ConfMan.get("guioptions", domain);

	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			_checkboxes[entry->option.configOption] = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + "." + entry->option.configOption, _(entry->option.label), _(entry->option.tooltip));

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions)) {
			GUI::StaticTextWidget *textWidget = new GUI::StaticTextWidget(widgetsBoss(), _dialogLayout + "." + entry->configOption + "_desc", _(entry->label), _(entry->tooltip));
			textWidget->setAlign(Graphics::kTextAlignRight);

			_popUps[entry->configOption] = new GUI::PopUpWidget(widgetsBoss(), _dialogLayout + "." + entry->configOption);

			for (uint i = 0; entry->items[i].label; ++i)
				_popUps[entry->configOption]->appendEntry(_(entry->items[i].label), entry->items[i].configValue);
		}
}

void OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(0, 0, 0, 0);

	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		layouts.addWidget(entry->option.configOption, "Checkbox");

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry) {
		layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal).addPadding(0, 0, 0, 0);
		layouts.addWidget(Common::String(entry->configOption) + "_desc", "OptionsLabel");
		layouts.addWidget(entry->configOption, "PopUp").closeLayout();
	}

	layouts.closeLayout().closeDialog();
}

void OptionsWidget::load() {
	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			_checkboxes[entry->option.configOption]->setState(ConfMan.getBool(entry->option.configOption, _domain));

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			_popUps[entry->configOption]->setSelectedTag(ConfMan.getInt(entry->configOption, _domain));
}

bool OptionsWidget::save() {
	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			ConfMan.setBool(entry->option.configOption, _checkboxes[entry->option.configOption]->getState(), _domain);

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry)
		if (checkGameGUIOption(entry->guioFlag, _guiOptions))
			ConfMan.setInt(entry->configOption, _popUps[entry->configOption]->getSelectedTag(), _domain);

	return true;
}

const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FLOPPY_MUSIC,
		{
			_s("Use floppy version music"),
			_s("Use the music from the floppy version. The floppy version's music files must be copied to the SOUND directory."),
			"use_floppy_music",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};


const PopUpOptionsMap popUpOptionsList[] = {
	{
		GAMEOPTION_SFX_MODE,
		_s("SFX mode:"),
		_s("Determines if the game should use CD version SFX only, CD SFX with additional floppy SFX, or floppy SFX only. Floppy SFX are only available if floppy music is used."),
		"sfx_mode",
		SFX_MODE_CD_ONLY,
		{
			{
				_s("CD version SFX only"),
				SFX_MODE_CD_ONLY
			},
			{
				_s("CD + extra floppy SFX"),
				SFX_MODE_CD_PLUS_FLOPPY
			},
			{
				_s("Floppy version SFX only"),
				SFX_MODE_FLOPPY_ONLY
			},
			POPUP_OPTIONS_ITEMS_TERMINATOR
		}
	},
	POPUP_OPTIONS_TERMINATOR
};

} // End of namespace Darkseed
