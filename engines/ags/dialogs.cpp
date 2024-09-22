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
#include "ags/detection.h"
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

	GUI::CheckboxWidget *_overrideSavesCheckbox;
	GUI::CheckboxWidget *_forceTextAACheckbox;
	GUI::CheckboxWidget *_displayFPSCheckbox;
};

AGSOptionsWidget::AGSOptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
	OptionsContainerWidget(boss, name, "AGSGameOptionsDialog", domain) {

	// Language
	GUI::StaticTextWidget *textWidget = new GUI::StaticTextWidget(widgetsBoss(), _dialogLayout + ".translation_desc", _("Game language:"), _("Language to use for multilingual games"));
	textWidget->setAlign(Graphics::kTextAlignRight);

	_langPopUp = new GUI::PopUpWidget(widgetsBoss(), _dialogLayout + ".translation");
	_langPopUp->appendEntry(_("<default>"), (uint32) - 1);

	_traFileNames = AGSMetaEngine::getGameTranslations(_domain);

	int i = 0;
	for (Common::StringArray::iterator iter = _traFileNames.begin(); iter != _traFileNames.end(); ++iter) {
		_langPopUp->appendEntry(*iter, i++);
	}

	// Override game save management
	if (Common::checkGameGUIOption(GAMEOPTION_NO_AUTOSAVE, ConfMan.get("guioptions", domain)) ||
		Common::checkGameGUIOption(GAMEOPTION_NO_SAVELOAD, ConfMan.get("guioptions", domain))) {
		_overrideSavesCheckbox = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".savesOvr", _("Enable ScummVM save management"), _("Never disable ScummVM save management and autosaves.\nNOTE: This could cause save duplication and other oddities"));
	} else
		_overrideSavesCheckbox = nullptr;

	// Force font antialiasing
	_forceTextAACheckbox = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".textAA", _("Force antialiased text"), _("Use antialiasing to draw text even if the game does not ask for it"));

	// Display fps
	_displayFPSCheckbox = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".displayFPS", _("Show FPS"), _("Show the current FPS-rate, while you play."));
}

void AGSOptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);
	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(0, 0, 0, 0);

	layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal).addPadding(0, 0, 0, 0);
	layouts.addWidget("translation_desc", "OptionsLabel");
	layouts.addWidget("translation", "PopUp").closeLayout();

	layouts.addWidget("savesOvr", "Checkbox");
	layouts.addWidget("textAA", "Checkbox");
	layouts.addWidget("displayFPS", "Checkbox");

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

	Common::String saveOverride;
	gameConfig->tryGetVal("save_override", saveOverride);
	if (!saveOverride.empty()) {
		bool val;
		if (_overrideSavesCheckbox && parseBool(saveOverride, val))
			_overrideSavesCheckbox->setState(val);
	}

	Common::String forceTextAA;
	gameConfig->tryGetVal("force_text_aa", forceTextAA);
	if (!forceTextAA.empty()) {
		bool val;
		if (parseBool(forceTextAA, val))
			_forceTextAACheckbox->setState(val);
	}

	Common::String displayFPS;
	gameConfig->tryGetVal("display_fps", displayFPS);
	if (!displayFPS.empty()) {
		bool val;
		if (parseBool(displayFPS, val))
			_displayFPSCheckbox->setState(val);
	}
}

bool AGSOptionsWidget::save() {
	uint langIndex = _langPopUp->getSelectedTag();
	if (langIndex < _traFileNames.size())
		ConfMan.set("translation", _traFileNames[langIndex], _domain);
	else
		ConfMan.removeKey("translation", _domain);

	if (_overrideSavesCheckbox)
		ConfMan.setBool("save_override", _overrideSavesCheckbox->getState(), _domain);
	ConfMan.setBool("force_text_aa", _forceTextAACheckbox->getState(), _domain);
	ConfMan.setBool("display_fps", _displayFPSCheckbox->getState(), _domain);

	return true;
}

} // namespace AGS3

GUI::OptionsContainerWidget *AGSMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new AGS3::AGSOptionsWidget(boss, name, target);
}
