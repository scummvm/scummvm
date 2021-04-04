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

#include "engines/dialogs.h"

#ifndef NANCY_DIALOGS_H
#define NANCY_DIALOGS_H

namespace Nancy {

class NancyOptionsWidget : public GUI::OptionsContainerWidget {
public:
	NancyOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);
	virtual ~NancyOptionsWidget() override {};

	void load() override;
	bool save() override;

private:
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	bool isInGame() const;

	GUI::CheckboxWidget *_playerSpeechCheckbox;
	GUI::CheckboxWidget *_characterSpeechCheckbox;
	GUI::CheckboxWidget *_originalMenusCheckbox;
};

} // End of namespace Nancy

#endif // NANCY_DIALOGS_H
