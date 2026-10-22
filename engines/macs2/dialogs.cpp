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

#include "macs2/dialogs.h"
#include "common/config-manager.h"
#include "common/gui_options.h"
#include "common/translation.h"
#include "engines/enhancements.h"
#include "gui/widget.h"
#include "macs2/detection.h"

namespace Macs2 {

Macs2OptionsWidget::Macs2OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
		OptionsContainerWidget(boss, name, "Macs2GameOptionsDialog", domain) {

	if (Common::checkGameGUIOption(GAMEOPTION_TRANSLATION, ConfMan.get("guioptions", domain))) {
		GUI::StaticTextWidget *label = new GUI::StaticTextWidget(widgetsBoss(), _dialogLayout + ".translationLabel", _("Translation:"));
		label->setAlign(Graphics::kTextAlignRight);
		_translationPopUp = new GUI::PopUpWidget(widgetsBoss(), _dialogLayout + ".translation");
		_translationPopUp->appendEntry(_("Original"), 0);
		_translationPackages = listTranslationPackages(Common::FSNode(ConfMan.getPath("path", domain)), ConfMan.get("gameid", domain));
		for (uint i = 0; i < _translationPackages.size(); ++i)
			_translationPopUp->appendEntry(_translationPackages[i].displayName, i + 1);
	}

	_originalMenusCheckbox = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".originalMenus",
		_("Use original save/load screens"),
		_("Use the original save/load screens instead of the ScummVM ones"));

#ifdef USE_TTS
	_ttsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".ttsEnabled",
		_("Enable Text to Speech"),
		_("Use TTS to read the dialogue (if TTS is available)"));
#endif

	// Enhancements
	GUI::StaticTextWidget *text = new GUI::StaticTextWidget(widgetsBoss(), _dialogLayout + ".enhancementsLabel", _("Enhancements:"));
	text->setAlign(Graphics::TextAlign::kTextAlignStart);

	// these don't exist yet
#if 0
	GUI::CheckboxWidget *enh1 = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".enhancementGroup1",
		_("Fix original bugs"),
		_("Fixes bugs which were present in the original release, and noticeable graphical/audio glitches."),
		kEnhancementGroup1Cmd);
#endif
	GUI::CheckboxWidget *enh2 = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".enhancementGroup2",
		_("Audio-visual improvements"),
		_("Optional generated dialogue speech (SPEECH/*.wav) and other audio/visual polish."),
		kEnhancementGroup2Cmd);
#if 0
	GUI::CheckboxWidget *enh3 = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".enhancementGroup3",
		_("Restored content"),
		_("Restores dialogs, graphics, and audio elements which were originally cut in the original release."),
		kEnhancementGroup3Cmd);
#endif
	GUI::CheckboxWidget *enh4 = new GUI::CheckboxWidget(widgetsBoss(), _dialogLayout + ".enhancementGroup4",
		_("Modern UI/UX adjustments"),
		_("Activates some modern comforts for a smoother user experience."),
		kEnhancementGroup4Cmd);

#if 0
	_enhancementsCheckboxes.push_back(enh1);
#endif
	_enhancementsCheckboxes.push_back(enh2);
#if 0
	_enhancementsCheckboxes.push_back(enh3);
#endif
	_enhancementsCheckboxes.push_back(enh4);
}

void Macs2OptionsWidget::load() {
	if (_translationPopUp) {
		_translationPopUp->setSelectedTag(0);
		const Common::String code = ConfMan.get(kTranslationConfigKey, _domain);
		for (uint i = 0; i < _translationPackages.size(); ++i) {
			if (_translationPackages[i].code.equalsIgnoreCase(code)) {
				_translationPopUp->setSelectedTag(i + 1);
				break;
			}
		}
	}

	_originalMenusCheckbox->setState(ConfMan.getBool("original_menus", _domain));

#ifdef USE_TTS
	_ttsCheckbox->setState(ConfMan.getBool("tts_enabled", _domain));
#endif

	int32 enhancementsFlags = (int32)ConfMan.getInt("enhancements", _domain);
	enhancementsFlags &= ~kEnhGameBreakingBugFixes; // Always active

	for (uint i = 0; i < _enhancementsCheckboxes.size(); i++) {
		if (!_enhancementsCheckboxes[i])
			continue;

		int32 targetFlags = 0;
		switch (_enhancementsCheckboxes[i]->getCmd()) {
		case kEnhancementGroup1Cmd:
			targetFlags = kEnhGrp1;
			break;
		case kEnhancementGroup2Cmd:
			targetFlags = kEnhGrp2;
			break;
		case kEnhancementGroup3Cmd:
			targetFlags = kEnhGrp3;
			break;
		case kEnhancementGroup4Cmd:
			targetFlags = kEnhGrp4;
			break;
		default:
			break;
		}
		_enhancementsCheckboxes[i]->setState(enhancementsFlags & targetFlags);
	}
}

bool Macs2OptionsWidget::save() {
	if (_translationPopUp) {
		const uint32 selected = _translationPopUp->getSelectedTag();
		if (selected > 0 && selected <= _translationPackages.size())
			ConfMan.set(kTranslationConfigKey, _translationPackages[selected - 1].code, _domain);
		else
			ConfMan.removeKey(kTranslationConfigKey, _domain);
	}

	ConfMan.setBool("original_menus", _originalMenusCheckbox->getState(), _domain);

#ifdef USE_TTS
	ConfMan.setBool("tts_enabled", _ttsCheckbox->getState(), _domain);
#endif

	int32 enhancementsFlags = kEnhGameBreakingBugFixes; // Always active

	for (uint i = 0; i < _enhancementsCheckboxes.size(); i++) {
		if (!_enhancementsCheckboxes[i])
			continue;

		switch (_enhancementsCheckboxes[i]->getCmd()) {
		case kEnhancementGroup1Cmd:
			if (_enhancementsCheckboxes[i]->getState())
				enhancementsFlags |= kEnhGrp1;
			break;
		case kEnhancementGroup2Cmd:
			if (_enhancementsCheckboxes[i]->getState())
				enhancementsFlags |= kEnhGrp2;
			break;
		case kEnhancementGroup3Cmd:
			if (_enhancementsCheckboxes[i]->getState())
				enhancementsFlags |= kEnhGrp3;
			break;
		case kEnhancementGroup4Cmd:
			if (_enhancementsCheckboxes[i]->getState())
				enhancementsFlags |= kEnhGrp4;
			break;
		default:
			break;
		}
	}

	ConfMan.setInt("enhancements", enhancementsFlags, _domain);
	return true;
}

void Macs2OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical, 5)
			.addPadding(0, 0, 0, 0);
	if (_translationPopUp) {
		layouts.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
			.addWidget("translationLabel", "OptionsLabel")
			.addWidget("translation", "PopUp")
		.closeLayout();
	}
	layouts
			.addWidget("originalMenus", "Checkbox")
#ifdef USE_TTS
			.addWidget("ttsEnabled", "Checkbox")
#endif
			.addPadding(0, 0, 8, 8)
			.addSpace(10)
			.addWidget("enhancementsLabel", "OptionsLabel")
			.addWidget("enhancementGroup1", "Checkbox")
			.addWidget("enhancementGroup2", "Checkbox")
			.addWidget("enhancementGroup3", "Checkbox")
			.addWidget("enhancementGroup4", "Checkbox")
		.closeLayout()
	.closeDialog();
}

} // End of namespace Macs2
