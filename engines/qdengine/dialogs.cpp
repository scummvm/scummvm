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

#include "gui/ThemeEval.h"

#include "qdengine/dialogs.h"

namespace QDEngine {

QdOptionsWidget::QdOptionsWidget(GuiObject* boss, const Common::String& name, const Common::String& domain) : OptionsContainerWidget(boss, name, "QdGameOptionsDialog", domain),
	_16bppModeCheckbox(nullptr) {
	_16bppModeCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "QdGameOptionsDialog.16bppMode", _("Enable 16 bits per pixel mode"), _("Enable this if backend does not support 32bpp and/or to debug graphics"));
}

QdOptionsWidget::~QdOptionsWidget() {
}

void QdOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
			.addPadding(0, 0, 0, 0)
			.addWidget("16bppMode", "Checkbox")
		.closeLayout()
	.closeDialog();
}

void QdOptionsWidget::load() {
	if (_16bppModeCheckbox)
		_16bppModeCheckbox->setState(ConfMan.getBool("16bpp"));
}

bool QdOptionsWidget::save() {
	if (_16bppModeCheckbox)
		ConfMan.setBool("16bpp", _16bppModeCheckbox->getState());

	return true;
}

} // End of namespace QDEngine
