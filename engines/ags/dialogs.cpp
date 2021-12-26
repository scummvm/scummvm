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
#include "common/file.h"
#include "common/md5.h"
#include "common/str-array.h"
#include "common/translation.h"
#include "common/util.h"
#include "ags/metaengine.h"

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

namespace AGS3 {

class AGSOptionsWidget : public GUI::OptionsContainerWidget {
public:
	explicit AGSOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;

	GUI::PopUpWidget *_langPopUp;
	Common::StringArray _traFileNames;

	GUI::CheckboxWidget *_forceTextAACheckbox;
};

AGSOptionsWidget::AGSOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
	OptionsContainerWidget(boss, name, "AGSGameOptionsDialog", false, domain) {

	// Language
	GUI::StaticTextWidget *textWidget = new GUI::StaticTextWidget(widgetsBoss(), _dialogLayout + ".translation_desc", _("Game language:"), _("Language to use for multilingual games"));
	textWidget->setAlign(Graphics::kTextAlignRight);

	_langPopUp = new GUI::PopUpWidget(widgetsBoss(), _dialogLayout + ".translation");
	_langPopUp->appendEntry(_("<default>"), (uint32) - 1);

	Common::String path = ConfMan.get("path", _domain);
	Common::FSDirectory dir(path);
	Common::ArchiveMemberList traFileList;
	dir.listMatchingMembers(traFileList, "*.tra");

	int i = 0;
	for (Common::ArchiveMemberList::iterator iter = traFileList.begin(); iter != traFileList.end(); ++iter) {
		Common::String traFileName = (*iter)->getName();
		traFileName.erase(traFileName.size() - 4); // remove .tra extension
		_traFileNames.push_back(traFileName);
		_langPopUp->appendEntry(traFileName, i++);
	}

	// Force font antialiasing
	_forceTextAACheckbox = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".textAA", _("Force antialiased text"), _("Use antialiasing to draw text even if the game does not ask for it"));
}

void AGSOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(16, 16, 16, 16);

	layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal).addPadding(0, 0, 0, 0);
	layouts.addWidget("translation_desc", "OptionsLabel");
	layouts.addWidget("translation", "PopUp").closeLayout();

	layouts.addWidget("textAA", "Checkbox");

	layouts.closeLayout().closeDialog();
}

void AGSOptionsWidget::load() {
	Common::ConfigManager::Domain *gameConfig = ConfMan.getDomain(_domain);
	if (!gameConfig)
		return;

	uint32 curLangIndex = (uint32) - 1;
	Common::String curLang;
	gameConfig->tryGetVal("translation", curLang);
	if (!curLang.empty()) {
		for (uint i = 0; i < _traFileNames.size(); ++i) {
			if (_traFileNames[i].equalsIgnoreCase(curLang)) {
				curLangIndex = i;
				break;
			}
		}
	}
	_langPopUp->setSelectedTag(curLangIndex);

	Common::String forceTextAA;
	gameConfig->tryGetVal("force_text_aa", forceTextAA);
	if (!forceTextAA.empty()) {
		bool val;
		if (parseBool(forceTextAA, val))
			_forceTextAACheckbox->setState(val);
	}
}

bool AGSOptionsWidget::save() {
	uint langIndex = _langPopUp->getSelectedTag();
	if (langIndex < _traFileNames.size())
		ConfMan.set("translation", _traFileNames[langIndex], _domain);
	else
		ConfMan.removeKey("translation", _domain);

	ConfMan.setBool("force_text_aa", _forceTextAACheckbox->getState(), _domain);

	return true;
}

} // namespace AGS3

GUI::OptionsContainerWidget *AGSMetaEngine::buildEngineOptionsWidgetDynamic(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new AGS3::AGSOptionsWidget(boss, name, target);
}
