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

#include "hopkins/hopkins.h"

#include "base/plugins.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/translation.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "hopkins/detection.h"

#define MAX_SAVES 99

namespace Hopkins {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_GORE_DEFAULT_OFF,
		{
			_s("Gore Mode"),
			_s("Enable Gore Mode when available"),
			"enable_gore",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_GORE_DEFAULT_ON,
		{
			_s("Gore Mode"),
			_s("Enable Gore Mode when available"),
			"enable_gore",
			true,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

uint32 HopkinsEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language HopkinsEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform HopkinsEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

bool HopkinsEngine::getIsDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

const Common::String &HopkinsEngine::getTargetName() const {
	return _targetName;
}

} // End of namespace Hopkins

class HopkinsMetaEngine : public AdvancedMetaEngine<Hopkins::HopkinsGameDescription> {
public:
	const char *getName() const override {
		return "hopkins";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Hopkins::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Hopkins::HopkinsGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool HopkinsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Hopkins::HopkinsEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error HopkinsMetaEngine::createInstance(OSystem *syst, Engine **engine, const Hopkins::HopkinsGameDescription *desc) const {
	*engine = new Hopkins::HopkinsEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList HopkinsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);

	filenames = saveFileMan->listSavefiles(pattern);

	Hopkins::hopkinsSavegameHeader header;

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (Hopkins::SaveLoadManager::readSavegameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));
				}

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int HopkinsMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void HopkinsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor HopkinsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Hopkins::hopkinsSavegameHeader header;
		if (!Hopkins::SaveLoadManager::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}

		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);
		desc.setThumbnail(header._thumbnail);
		desc.setSaveDate(header._year, header._month, header._day);
		desc.setSaveTime(header._hour, header._minute);
		desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::KeymapArray HopkinsMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Hopkins;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "hopkins-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("ESCAPE", _("Exit/Skip"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_BACK");
	gameKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Open inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("TAB");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("SAVE", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F7");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("OPTIONS", _("Option dialog"));
	act->setCustomEngineActionEvent(kActionOptions);
	act->addDefaultInputMapping("o");
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);


	
	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(HOPKINS)
	REGISTER_PLUGIN_DYNAMIC(HOPKINS, PLUGIN_TYPE_ENGINE, HopkinsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(HOPKINS, PLUGIN_TYPE_ENGINE, HopkinsMetaEngine);
#endif
