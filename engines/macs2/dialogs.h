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

#ifndef MACS2_DIALOGS_H
#define MACS2_DIALOGS_H

#include "gui/ThemeEval.h"
#include "gui/widget.h"

namespace Macs2 {

class Macs2OptionsWidget : public GUI::OptionsContainerWidget {
public:
	Macs2OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~Macs2OptionsWidget() override {}

	void load() override;
	bool save() override;

private:
	enum {
		kEnhancementGroup1Cmd = 'ENH1',
		kEnhancementGroup2Cmd = 'ENH2',
		kEnhancementGroup3Cmd = 'ENH3',
		kEnhancementGroup4Cmd = 'ENH4'
	};

	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	GUI::CheckboxWidget *_originalMenusCheckbox = nullptr;
#ifdef USE_TTS
	GUI::CheckboxWidget *_ttsCheckbox = nullptr;
#endif
	Common::Array<GUI::CheckboxWidget *> _enhancementsCheckboxes;
};

} // End of namespace Macs2

#endif // MACS2_DIALOGS_H
