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

#ifndef DARKSEED_DIALOGS_H
#define DARKSEED_DIALOGS_H

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

namespace Darkseed {

struct PopUpOptionsItem {
	const char *label;
	int configValue;
};

#define POPUP_OPTIONS_ITEMS_TERMINATOR {nullptr, 0}

struct PopUpOptionsMap {
	const char *guioFlag;
	const char *label;
	const char *tooltip;
	const char *configOption;
	int defaultState;
	PopUpOptionsItem items[10];
};

#define POPUP_OPTIONS_TERMINATOR                                                  \
	{                                                                             \
		nullptr, nullptr, nullptr, nullptr, 0, { POPUP_OPTIONS_ITEMS_TERMINATOR } \
	}

class OptionsWidget : public GUI::OptionsContainerWidget {
public:
	explicit OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	Common::String _guiOptions;
	Common::HashMap<Common::String, GUI::CheckboxWidget *> _checkboxes;
	Common::HashMap<Common::String, GUI::PopUpWidget *> _popUps;
};

static const ADExtraGuiOptionsMap optionsList[] = {
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


static const PopUpOptionsMap popUpOptionsList[] = {
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

#endif // DARKSEED_DIALOGS_H
