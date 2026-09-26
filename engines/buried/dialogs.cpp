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
#include "gui/widget.h"
#include "gui/widgets/popup.h"

#include "buried/dialogs.h"
#include "buried/subtitle_manager.h"

namespace Buried {

BuriedOptionsWidget::BuriedOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain)
	: OptionsContainerWidget(boss, name, /* dialogLayout= */"BuriedGameOptionsDialog", domain) {

	// "Subtitle font size:" label
	_fontSizeDesc = new GUI::StaticTextWidget(
		widgetsBoss(),
		/* name= */"BuriedGameOptionsDialog.FontSizeDesc",
		/* text= */ _("Subtitles font size:"));
	_fontSizeDesc->setAlign(Graphics::kTextAlignRight);

	// Dropdown populated with named presets
	_fontSizePopUp = new GUI::PopUpWidget(widgetsBoss(), "BuriedGameOptionsDialog.FontSize");
	_fontSizePopUp->appendEntry(_("Small"), kFontSizeSmall);
	_fontSizePopUp->appendEntry(_("Medium"), kFontSizeMedium);
	_fontSizePopUp->appendEntry(_("Large"), kFontSizeLarge);
	_fontSizePopUp->appendEntry(_("Extra large"), kFontSizeXLarge);
}

static void defineFontSizeRowLayout(GUI::ThemeEval &layouts) {
	layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal).addPadding(0, 0, 2, 0);
	layouts.addWidget(/* name= */ "FontSizeDesc", /* type= */ "OptionsLabel");
	layouts.addWidget(/* name= */ "FontSize", /* type= */ "PopUp");
	layouts.closeLayout();
}

void BuriedOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(0, 0, 0, 0);

	defineFontSizeRowLayout(layouts);

	layouts.closeLayout();
	layouts.closeDialog();
}

// Reads the subtitle font size setting from the active ScummVM configuration domain ("subtitle_font_size").
static int getSavedSubtitleFontSize(const Common::String &domain) {
	if (ConfMan.hasKey(kSubtitleFontSizeKey, domain)) {
		return ConfMan.getInt(kSubtitleFontSizeKey, domain);
	}
	return kFontSizeMedium;
}

// Writes the subtitle font size setting to the active ScummVM configuration domain ("subtitle_font_size").
static void saveSubtitleFontSize(const Common::String &domain, const int fontSize) {
	ConfMan.setInt(kSubtitleFontSizeKey, fontSize, domain);
}

void BuriedOptionsWidget::load() {
	const int fontSize = getSavedSubtitleFontSize(_domain);
	_fontSizePopUp->setSelectedTag(fontSize);

	// If the saved value doesn't match any preset, fall back to Medium
	if ((int32)_fontSizePopUp->getSelectedTag() == -1) {
		_fontSizePopUp->setSelectedTag(kFontSizeMedium);
	}
}

bool BuriedOptionsWidget::save() {
	uint32 selectedTag = _fontSizePopUp->getSelectedTag();
	if ((int32) selectedTag == -1) {
		selectedTag = kDefaultSubtitleFontSize;
	}

	saveSubtitleFontSize(_domain, (int) selectedTag);
	return true;
}

} // End of namespace Buried
