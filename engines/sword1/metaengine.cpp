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

#include "engines/advancedDetector.h"

#include "sword1/sword1.h"
#include "sword1/control.h"
#include "sword1/logic.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/dialogs.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "gui/ThemeEval.h"
#include "gui/widget.h"
#include "gui/widgets/popup.h"

namespace Sword1 {

#define GAMEOPTION_WINDOWS_AUDIO_MODE     GUIO_GAMEOPTIONS1
#define GAMEOPTION_MULTILANGUAGE          GUIO_GAMEOPTIONS2
#define GAMEOPTION_MULTILANGUAGE_EXTENDED GUIO_GAMEOPTIONS3

class Sword1OptionsWidget : public GUI::OptionsContainerWidget {
public:
	explicit Sword1OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain);

	// OptionsContainerWidget API
	void load() override;
	bool save() override;

private:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	Common::StringArray _availableLangCodes = {"en", "de", "fr", "it", "es", "pt", "cs"};
	Common::StringArray _availableLangs = {_("English"), _("German"), _("French"), _("Italian"), _("Spanish"), _("Brazilian Portuguese"), _("Czech")};
	uint32 _numAvailableLangs = 0;
	bool _atLeastOneAdditionalOpt = false;

	GUI::PopUpWidget *_langPopUp;
	GUI::CheckboxWidget *_windowsAudioMode;
};

Sword1OptionsWidget::Sword1OptionsWidget(GuiObject *boss, const Common::String &name, const Common::String &domain) :
	OptionsContainerWidget(boss, name, "Sword1GameOptionsDialog", domain) {

	if (Common::checkGameGUIOption(GAMEOPTION_MULTILANGUAGE, ConfMan.get("guioptions", domain))) {
		_numAvailableLangs = 5;
	} else if (Common::checkGameGUIOption(GAMEOPTION_MULTILANGUAGE_EXTENDED, ConfMan.get("guioptions", domain))) {
		_numAvailableLangs = 7;
	}

	// Language
	if (Common::checkGameGUIOption(GAMEOPTION_MULTILANGUAGE, ConfMan.get("guioptions", domain)) ||
		Common::checkGameGUIOption(GAMEOPTION_MULTILANGUAGE_EXTENDED, ConfMan.get("guioptions", domain))) {
		GUI::StaticTextWidget *textWidget = new GUI::StaticTextWidget(
			widgetsBoss(),
			_dialogLayout + ".subtitles_lang_desc",
			_("Text language:"),
			_("Set the language for the subtitles. This will not affect voices.")
		);

		textWidget->setAlign(Graphics::kTextAlignLeft);

		_langPopUp = new GUI::PopUpWidget(
			widgetsBoss(),
			_dialogLayout + ".subtitles_lang",
			_("Set the language for the subtitles. This will not affect voices.")
		);

		_langPopUp->appendEntry(_("<default>"), (uint32)-1);

		for (uint32 i = 0; i < _numAvailableLangs; i++) {
			_langPopUp->appendEntry(_availableLangs[i], i);
		}

		_atLeastOneAdditionalOpt = true;
	} else {
		_langPopUp = nullptr;
	}

	// Windows audio mode
	if (Common::checkGameGUIOption(GAMEOPTION_WINDOWS_AUDIO_MODE, ConfMan.get("guioptions", domain))) {
		_windowsAudioMode = new GUI::CheckboxWidget(
			widgetsBoss(),
			_dialogLayout + ".windows_audio_mode",
			_("Simulate the audio engine from the Windows executable"),
			_("Makes the game use softer (logarithmic) audio curves, but removes fade-in and fade-out for "
			  "sound effects, fade-in for music, and automatic music volume attenuation for when speech is playing"));

		_atLeastOneAdditionalOpt = true;
	} else {
		_windowsAudioMode = nullptr;
	}

	if (_atLeastOneAdditionalOpt) {
		GUI::StaticTextWidget *additionalOptsWidget = new GUI::StaticTextWidget(
			widgetsBoss(),
			_dialogLayout + ".additional_opts_label",
			_("Additional options:"));

		additionalOptsWidget->setAlign(Graphics::kTextAlignLeft);
	}
}

void Sword1OptionsWidget::load() {
	Common::ConfigManager::Domain *gameConfig = ConfMan.getDomain(_domain);
	if (!gameConfig)
		return;

	if (_langPopUp) {
		uint32 curLangIndex = (uint32)-1;
		Common::String curLang;
		gameConfig->tryGetVal("subtitles_language_override", curLang);
		if (!curLang.empty()) {
			for (uint i = 0; i < _numAvailableLangs; ++i) {
				if (_availableLangCodes[i].equalsIgnoreCase(curLang)) {
					curLangIndex = i;
					break;
				}
			}
		}

		_langPopUp->setSelectedTag(curLangIndex);
	}

	if (_windowsAudioMode) {
		Common::String windowsAudioMode;
		gameConfig->tryGetVal("windows_audio_mode", windowsAudioMode);
		if (!windowsAudioMode.empty()) {
			bool val;
			if (parseBool(windowsAudioMode, val))
				_windowsAudioMode->setState(val);
		}
	}
}

bool Sword1OptionsWidget::save() {
	if (_langPopUp) {
		uint langIndex = _langPopUp->getSelectedTag();
		if (langIndex < _numAvailableLangs)
			ConfMan.set("subtitles_language_override", _availableLangCodes[langIndex], _domain);
		else
			ConfMan.removeKey("subtitles_language_override", _domain);
	}

	if (_windowsAudioMode)
		ConfMan.setBool("windows_audio_mode", _windowsAudioMode->getState(), _domain);

	return true;
}

void Sword1OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout);

	layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(16, 0, 0, 0); // Layout 1

		layouts.addWidget("additional_opts_label", "OptionsLabel");
		layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(8, 0, 4, 0); // Layout 2

			layouts.addWidget("subtitles_lang_desc", "OptionsLabel");
			layouts.addWidget("subtitles_lang", "PopUp");

			// This third layout is added for further separation from the dropdown list
			layouts.addLayout(GUI::ThemeLayout::kLayoutVertical).addPadding(0, 0, 0, 0); // Layout 3

				if (_langPopUp) // Don't draw padding if there's no lang selection
					layouts.addPadding(0, 0, 8, 0);

				layouts.addWidget("windows_audio_mode", "Checkbox");

			layouts.closeLayout(); // Close layout 3

		layouts.closeLayout(); // Close layout 2

	layouts.closeLayout().closeDialog(); // Close layout 1
}

} // End of namespace Sword1

class SwordMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "sword1";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("sword1.###");
		else
			return Common::String::format("sword1.%03d", saveGameIdx);
	}

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool SwordMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate) ||
	    (f == kSavesSupportPlayTime);
}

bool Sword1::SwordEngine::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsReturnToLauncher) ||
	    (f == kSupportsSavingDuringRuntime) ||
	    (f == kSupportsLoadingDuringRuntime);
}

GUI::OptionsContainerWidget *SwordMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new Sword1::Sword1OptionsWidget(boss, name, target);
}

Common::Error SwordMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Sword1::SwordEngine(syst, desc);
	return Common::kNoError;
}

Common::KeymapArray SwordMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Sword1;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "sword1-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("ESCAPE", _("Exit/Skip"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_BACK");
	gameKeyMap->addAction(act);

	act = new Action("PAUSE", _("Pause game"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit Game"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("C+q");
	act->addDefaultInputMapping("JOY_CENTER");
	gameKeyMap->addAction(act);

	act = new Action("MAINPANEL", _("Main Menu"));
	act->setCustomEngineActionEvent(kActionMainPanel);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

SaveStateList SwordMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;
	char saveName[40];

	Common::StringArray filenames = saveFileMan->listSavefiles("sword1.###");

	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				in->readUint32LE(); // header
				in->read(saveName, 40);
				saveList.push_back(SaveStateDescriptor(this, slotNum, saveName));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int SwordMetaEngine::getMaximumSaveSlot() const { return 999; }

bool SwordMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(Common::String::format("sword1.%03d", slot));
}

SaveStateDescriptor SwordMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("sword1.%03d", slot);
	char name[40];
	uint32 playTime = 0;
	byte versionSave;

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		in->skip(4);        // header
		in->read(name, sizeof(name));
		in->read(&versionSave, 1);      // version

		SaveStateDescriptor desc(this, slot, name);

		if (versionSave < 2) // These older version of the savegames used a flag to signal presence of thumbnail
			in->skip(1);

		if (Graphics::checkThumbnailHeader(*in)) {
			Graphics::Surface *thumbnail;
			if (!Graphics::loadThumbnail(*in, thumbnail)) {
				delete in;
				return SaveStateDescriptor();
			}
			desc.setThumbnail(thumbnail);
		}

		uint32 saveDate = in->readUint32BE();
		uint16 saveTime = in->readUint16BE();
		if (versionSave > 1) // Previous versions did not have playtime data
			playTime = in->readUint32BE();

		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;

		desc.setSaveTime(hour, minutes);

		if (versionSave > 1) {
			desc.setPlayTime(playTime * 1000);
		} else { //We have no playtime data
			desc.setPlayTime(0);
		}

		delete in;

		return desc;
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(SWORD1)
	REGISTER_PLUGIN_DYNAMIC(SWORD1, PLUGIN_TYPE_ENGINE, SwordMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SWORD1, PLUGIN_TYPE_ENGINE, SwordMetaEngine);
#endif

namespace Sword1 {

Common::Error SwordEngine::loadGameState(int slot) {
	_systemVars.controlPanelMode = CP_NORMAL;
	_control->restoreGameFromFile(slot);
	reinitialize();
	_control->doRestore();
	reinitRes();
	return Common::kNoError;    // TODO: return success/failure
}

bool SwordEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return (mouseIsActive() && !_control->isPanelShown()); // Disable GMM loading when game panel is shown
}

Common::Error SwordEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	_control->setSaveDescription(slot, desc.c_str());
	_control->saveGameToFile(slot);
	return Common::kNoError;    // TODO: return success/failure
}

bool SwordEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return (mouseIsActive() && !_control->isPanelShown() && Logic::_scriptVars[SCREEN] != 91);
}

} // End of namespace Sword1
