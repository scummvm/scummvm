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

#ifndef SCI_DIALOGS_H
#define SCI_DIALOGS_H

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

namespace Sci {

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

#define POPUP_OPTIONS_TERMINATOR { nullptr, nullptr, nullptr, nullptr, 0, { POPUP_OPTIONS_ITEMS_TERMINATOR } }

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

extern const ADExtraGuiOptionsMap optionsList[];
extern const PopUpOptionsMap popUpOptionsList[];

} // End of namespace Sci

#endif // SCI_DIALOGS_H
