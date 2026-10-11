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

#include "common/config-manager.h"
#include "common/translation.h"
#include "gui/ThemeEval.h"
#include "gui/widgets/popup.h"
#include "ultima/shared/engine/graphics_options.h"

namespace Ultima {
namespace Shared {

GraphicsOptionsWidget::GraphicsOptionsWidget(GUI::GuiObject *boss, const Common::String &name,
		const Common::String &domain, const char *const *modes, int modeCount,
		const ExtraGuiOptions &options, bool onTitleScreen) :
		GUI::OptionsContainerWidget(boss, name, "UltimaGraphicsOptionsDialog", domain),
		_modes(modes), _modeCount(modeCount), _options(options) {
	const Common::U32String tooltip = onTitleScreen ?
		_("Which of the game's renderers to use. This can also be changed from the game's title screen.") :
		_("Which of the game's renderers to use.");

	GUI::StaticTextWidget *label = new GUI::StaticTextWidget(widgetsBoss(),
		_dialogLayout + ".graphics_mode_desc", _("Graphics mode:"), tooltip);
	label->setAlign(Graphics::kTextAlignLeft);

	_modePopUp = new GUI::PopUpWidget(widgetsBoss(), _dialogLayout + ".graphics_mode", tooltip);

	for (int i = 0; i < _modeCount; ++i)
		_modePopUp->appendEntry(_(_modes[i]), i);

	for (uint i = 0; i < _options.size(); ++i) {
		_checkboxes.push_back(new GUI::CheckboxWidget(widgetsBoss(),
			_dialogLayout + Common::String::format(".customOption%dCheckbox", i + 1),
			_(_options[i].label), _(_options[i].tooltip)));
	}
}

void GraphicsOptionsWidget::load() {
	// A missing or stale value means the original graphics
	int mode = 0;
	if (ConfMan.hasKey(GRAPHICS_MODE_KEY, _domain))
		mode = ConfMan.getInt(GRAPHICS_MODE_KEY, _domain);
	if (mode < 0 || mode >= _modeCount)
		mode = 0;

	_modePopUp->setSelectedTag(mode);

	// The options that are checkboxes start from what the game says unless set
	for (uint i = 0; i < _options.size(); ++i) {
		bool isChecked = _options[i].defaultState;
		if (ConfMan.hasKey(_options[i].configOption, _domain))
			isChecked = ConfMan.getBool(_options[i].configOption, _domain);
		_checkboxes[i]->setState(isChecked);
	}
}

bool GraphicsOptionsWidget::save() {
	ConfMan.setInt(GRAPHICS_MODE_KEY, _modePopUp->getSelectedTag(), _domain);

	for (uint i = 0; i < _options.size(); ++i)
		ConfMan.setBool(_options[i].configOption, _checkboxes[i]->getState(), _domain);

	return true;
}

void GraphicsOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName,
		const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);

	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(16, 0, 0, 0);
		layouts.addWidget("graphics_mode_desc", "OptionsLabel");
		layouts.addWidget("graphics_mode", "PopUp");

		for (uint i = 0; i < _options.size(); ++i)
			layouts.addWidget(Common::String::format("customOption%dCheckbox", i + 1), "Checkbox");
	layouts.closeLayout().closeDialog();
}

} // namespace Shared
} // namespace Ultima
