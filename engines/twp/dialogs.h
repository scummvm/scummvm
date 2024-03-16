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

#ifndef TWP_DIALOGS_H
#define TWP_DIALOGS_H

#include "gui/dialog.h"
#include "engines/dialogs.h"

namespace Twp {

class TwpOptionsContainerWidget : public GUI::OptionsContainerWidget {
public:
	TwpOptionsContainerWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	~TwpOptionsContainerWidget() override = default;

	void load() override;
	bool save() override;

private:
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

private:
	GUI::CheckboxWidget *_enableToiletPaperOverGUICheckbox = nullptr;
	GUI::CheckboxWidget *_enableAnnoyingInJokesGUICheckbox = nullptr;
	GUI::CheckboxWidget *_enableInvertVerbColorsGUICheckbox = nullptr;
	GUI::CheckboxWidget *_enableRetroFontsGUICheckbox = nullptr;
	GUI::CheckboxWidget *_enableRetroVerbsGUICheckbox = nullptr;
	GUI::CheckboxWidget *_enableClassicSentenceGUICheckbox = nullptr;
	GUI::CheckboxWidget *_enableDLC = nullptr;
};

} // namespace Twp

#endif
