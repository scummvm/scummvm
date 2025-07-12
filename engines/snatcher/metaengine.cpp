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

#include "snatcher/detection.h"
#include "snatcher/saveload.h"
#include "snatcher/snatcher.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

const ADExtraGuiOptionsMap gameGuiOptions[] = {
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class SnatcherMetaEngine : public AdvancedMetaEngine<SnatcherGameDescription> {
public:
    const char *getName() const override {
		return "snatcher";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override;
    bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const SnatcherGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	int getAutosaveSlot() const override { return 0; }

	Common::KeymapArray initKeymaps(const char *target) const override;
};

const ADExtraGuiOptionsMap *SnatcherMetaEngine::getAdvancedExtraGuiOptions() const {
	return gameGuiOptions;
}

bool SnatcherMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSimpleSavesNames);
}

bool Snatcher::SnatcherEngine::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsReturnToLauncher) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime) ||
	    (f == kSupportsChangingOptionsDuringRuntime);
}

Common::Error SnatcherMetaEngine::createInstance(OSystem *syst, Engine **engine, const SnatcherGameDescription *desc) const {
	const SnatcherGameDescription *gd = (const SnatcherGameDescription*)desc;
	Snatcher::GameDescription dsc = gd->ex_desc;

	Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));
	if (platform != Common::kPlatformUnknown)
		dsc.platform = platform;

	if (dsc.lang == Common::UNK_LANG) {
		Common::Language lang = Common::parseLanguage(ConfMan.get("language"));
		if (lang != Common::UNK_LANG)
			dsc.lang = lang;
		else
			dsc.lang = Common::EN_ANY;
	}

	switch (dsc.gameID) {
	case Snatcher::GI_SNATCHER:
		*engine = new Snatcher::SnatcherEngine(syst, dsc);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

SaveStateList SnatcherMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Snatcher::SaveLoadManager::SaveHeader header;
	Common::String pattern(target);
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slot = atoi(file->substr(file->size() - 3, 3).c_str());
		if (slot >= 0 && slot <= getMaximumSaveSlot()) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				if (Snatcher::SaveLoadManager::readSaveHeader(in, header))
					saveList.push_back(SaveStateDescriptor(this, slot, header.desc));
				delete in;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int SnatcherMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

bool SnatcherMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Snatcher::SaveLoadManager::getSavegameFilename(slot, target);
	g_system->getSavefileManager()->removeSavefile(filename);
	return true;
}

SaveStateDescriptor SnatcherMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Snatcher::SaveLoadManager::getSavegameFilename(slot, target);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);

	Common::String d;
	SaveStateDescriptor desc(this, slot, d);
	Snatcher::SaveLoadManager::SaveHeader header;

	if (in && Snatcher::SaveLoadManager::readSaveHeader(in, header)) {
		desc.setDescription(header.desc);
		desc.setThumbnail(header.thumbnail);
		desc.setPlayTime(header.totalPlayTime * 1000);
		desc.setSaveDate(header.td.tm_year + 1900, header.td.tm_mon + 1, header.td.tm_mday);
		desc.setSaveTime(header.td.tm_hour, header.td.tm_min);
	}

	// We don't allow the autosave slot to be overwritten.
	desc.setWriteProtectedFlag(slot == getAutosaveSlot());
	delete in;

	return desc;
}

const char *const kLightGunKeymapName = "snlightgun";
const char *const kControllerKeymapName = "sncontroller";

void addKeymapAction(Common::Keymap *const keyMap, const char *actionId, const Common::U32String &actionDesc, const Common::Functor0Mem<void, Common::Action>::FuncType setEventProc, const Common::String &mapping1, const Common::String &mapping2) {
	Common::Action *act = new Common::Action(actionId, actionDesc);
	Common::Functor0Mem<void, Common::Action>(act, setEventProc)();
	act->addDefaultInputMapping(mapping1);
	act->addDefaultInputMapping(mapping2);
	keyMap->addAction(act);
}

void addKeymapAction(Common::Keymap *const keyMap, const char *actionId, const Common::U32String &actionDesc, Common::KeyState eventKeyState, const Common::String &mapping1, const Common::String &mapping2) {
	Common::Action *act = new Common::Action(actionId, actionDesc);
	act->setKeyEvent(eventKeyState);
	act->addDefaultInputMapping(mapping1);
	act->addDefaultInputMapping(mapping2);
	keyMap->addAction(act);
}

Common::KeymapArray SnatcherMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *keyMap1 = new Common::Keymap(Common::Keymap::kKeymapTypeGame, kLightGunKeymapName, "Snatcher - Lightgun");
	Common::Keymap *keyMap2 = new Common::Keymap(Common::Keymap::kKeymapTypeGame, kControllerKeymapName, "Snatcher - Controller");
	Common::KeymapArray res;

	addKeymapAction(keyMap1, Common::kStandardActionLeftClick, _("Lightgun Fire Button"), &Common::Action::setLeftClickEvent, "MOUSE_LEFT", "JOY_LEFT_SHOULDER");
	addKeymapAction(keyMap1, Common::kStandardActionRightClick, _("Lightgun Start Button"), &Common::Action::setRightClickEvent, "MOUSE_RIGHT", "JOY_RIGHT_SHOULDER");
	res.push_back(keyMap1);

	addKeymapAction(keyMap2, "CTRL_A", _("Controller Button A"), Common::KeyState(Common::KEYCODE_a, 'a'), "a", "JOY_A");
	addKeymapAction(keyMap2, "CTRL_B", _("Controller Button B"), Common::KeyState(Common::KEYCODE_b, 'b'), "b", "JOY_B");
	addKeymapAction(keyMap2, "CTRL_C", _("Controller Button C"), Common::KeyState(Common::KEYCODE_c, 'c'), "c", "JOY_X");
	addKeymapAction(keyMap2, "CTRL_START", _("Controller Start Button"), Common::KeyState(Common::KEYCODE_SPACE, Common::ASCII_SPACE), "SPACE", "JOY_START");
	addKeymapAction(keyMap2, "CTRL_UP", _("Controller Up"), Common::KeyState(Common::KEYCODE_UP), "UP", "JOY_UP");
	addKeymapAction(keyMap2, "CTRL_DOWN", _("Controller Down"), Common::KeyState(Common::KEYCODE_DOWN), "DOWN", "JOY_DOWN");
	addKeymapAction(keyMap2, "CTRL_LEFT", _("Controller Left"), Common::KeyState(Common::KEYCODE_LEFT), "LEFT", "JOY_LEFT");
	addKeymapAction(keyMap2, "CTRL_RIGHT", _("Controller Right"), Common::KeyState(Common::KEYCODE_RIGHT), "RIGHT", "JOY_RIGHT");
	addKeymapAction(keyMap2, "CTRL_UP_LEFT", _("Controller Up Left"), Common::KeyState(Common::KEYCODE_KP7), "UP_LEFT", "");
	addKeymapAction(keyMap2, "CTRL_UP_RIGHT", _("Controller Up Right"), Common::KeyState(Common::KEYCODE_KP9), "UP_RIGHT", "");
	addKeymapAction(keyMap2, "CTRL_DOWN_LEFT", _("Controller Down Left"), Common::KeyState(Common::KEYCODE_KP1), "DOWN_LEFT", "");
	addKeymapAction(keyMap2, "CTRL_DOWN_RIGHT", _("Controller Down Right"), Common::KeyState(Common::KEYCODE_KP3), "DOWN_RIGHT", "");
	res.push_back(keyMap2);

	return res;
}

#if PLUGIN_ENABLED_DYNAMIC(SNATCHER)
	REGISTER_PLUGIN_DYNAMIC(SNATCHER, PLUGIN_TYPE_ENGINE, SnatcherMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SNATCHER, PLUGIN_TYPE_ENGINE, SnatcherMetaEngine);
#endif
