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

#ifndef AGOS_DIALOGS_H
#define AGOS_DIALOGS_H

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

namespace AGOS {

struct PopUpOptionsItem {
	const char *label;
	int configValue;
};

#define POPUP_OPTIONS_ITEMS_TERMINATOR { nullptr, 0 }

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
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Enable copy protection"),
			_s("Enable any copy protection that would otherwise be bypassed by default."),
			"copy_protection",
			false,
			0,
			0
		},
	},
	{
		GAMEOPTION_OPL3_MODE,
		{
			_s("AdLib OPL3 mode"),
			_s("When AdLib is selected, OPL3 features will be used. Depending on the game, this will prevent cut-off notes, add extra notes or instruments and/or add stereo."),
			"opl3_mode",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DOS_TEMPOS,
		{
			_s("Use DOS version music tempos"),
			_s("Selecting this option will play the music using the tempos used by the DOS version of the game. Otherwise, the faster tempos of the Windows version will be used."),
			"dos_music_tempos",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_PREFER_DIGITAL_SFX,
		{
			_s("Prefer digital sound effects"),
			_s("Prefer digital sound effects instead of synthesized ones"),
			"prefer_digitalsfx",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_DISABLE_FADE_EFFECTS,
		{
			_s("Disable fade-out effects"),
			_s("Don't fade every screen to black when leaving a room."),
			"disable_fade_effects",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

static const PopUpOptionsMap popUpOptionsList[] = {
	{
		GAMEOPTION_MIDI_MODE,
		_s("MIDI mode:"),
		_s("When using external MIDI devices (e.g. through USB-MIDI), select your device here"),
		"midi_mode",
		kMidiModeStandard,
		{
			{
				_s("Standard (GM / MT-32)"),
				kMidiModeStandard
			},
			{
				_s("Casio MT-540"),
				kMidiModeMT540
			},
			{
				_s("Casio CT-460 / CSM-1"),
				kMidiModeCT460
			},
			POPUP_OPTIONS_ITEMS_TERMINATOR
		}
	},
	POPUP_OPTIONS_TERMINATOR
};

} // End of namespace AGOS

#endif // AGOS_DIALOGS_H
