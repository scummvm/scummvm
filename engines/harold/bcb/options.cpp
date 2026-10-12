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

#include "harold/bcb/options.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/str-array.h"
#include "common/translation.h"
#include "gui/ThemeEval.h"
#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"
#include "harold/bcb/translation.h"

namespace BCB {

static const char *const kTextLanguageConfigKey = "bcb_text_language";

class BCBOptionsWidget : public GUI::OptionsContainerWidget {
public:
	BCBOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);

	void load() override;
	bool save() override;

private:
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	Common::StringArray _languageCodes;
	GUI::PopUpWidget *_textLanguagePopUp;
};

BCBOptionsWidget::BCBOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain)
	: OptionsContainerWidget(boss, name, "BCBGameOptionsDialog", domain),
	  _textLanguagePopUp(nullptr) {

	GUI::StaticTextWidget *languageLabel = new GUI::StaticTextWidget(
		widgetsBoss(),
		_dialogLayout + ".text_language_desc",
		_("Translation:"),
		_("Select a text translation package. Original language is used when no package is selected."));
	languageLabel->setAlign(Graphics::kTextAlignRight);

	_textLanguagePopUp = new GUI::PopUpWidget(
		widgetsBoss(),
		_dialogLayout + ".text_language",
		_("Select a text translation package. Original language is used when no package is selected."));

	_languageCodes.push_back("");
	_textLanguagePopUp->appendEntry(_("Original"), 0);

	const Common::FSNode gameDataDir(ConfMan.getPath("path", domain));
	Common::Array<TranslationPackageDefinition> packages;
	listAvailableTranslationPackages(gameDataDir, packages);
	for (uint i = 0; i < packages.size(); ++i) {
		const TranslationPackageDefinition &package = packages[i];
		_languageCodes.push_back(package.code);
		_textLanguagePopUp->appendEntry(package.displayName.c_str(), _languageCodes.size() - 1);
	}
}

void BCBOptionsWidget::load() {
	uint32 selectedTag = 0;
	Common::ConfigManager::Domain *gameConfig = ConfMan.getDomain(_domain);
	if (gameConfig) {
		Common::String selectedCode;
		gameConfig->tryGetVal(kTextLanguageConfigKey, selectedCode);
		for (uint i = 0; i < _languageCodes.size(); ++i) {
			if (_languageCodes[i].equalsIgnoreCase(selectedCode)) {
				selectedTag = i;
				break;
			}
		}
	}

	_textLanguagePopUp->setSelectedTag(selectedTag);
}

bool BCBOptionsWidget::save() {
	const uint32 selectedTag = _textLanguagePopUp->getSelectedTag();
	if (selectedTag > 0 && selectedTag < _languageCodes.size())
		ConfMan.set(kTextLanguageConfigKey, _languageCodes[selectedTag], _domain);
	else
		ConfMan.removeKey(kTextLanguageConfigKey, _domain);

	return true;
}

void BCBOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	const bool lowResGui = g_gui.useLowResGUI();
	const int labelWidth = lowResGui ? 125 : 115;
	const int leftInset = 1;
	const int rowSpacing = lowResGui ? 6 : 10;
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(leftInset, 0, 0, 0);
	layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal, rowSpacing, GUI::ThemeLayout::kItemAlignCenter).addPadding(0, 0, 0, 0);
	layouts.addWidget("text_language_desc", "", labelWidth, lineHeight, Graphics::kTextAlignEnd);
	layouts.addWidget("text_language", "PopUp").closeLayout();
	layouts.closeLayout().closeDialog();
}

GUI::OptionsContainerWidget *createTranslationOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) {
	return new BCBOptionsWidget(boss, name, target);
}

} // End of namespace BCB
