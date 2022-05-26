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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"
#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "mm/detection.h"
#include "mm/mm1/mm1.h"
#include "mm/xeen/xeen.h"
#include "mm/xeen/worldofxeen/worldofxeen.h"
#include "mm/xeen/swordsofxeen/swordsofxeen.h"

#define MAX_SAVES 99

class MMMetaEngine : public AdvancedMetaEngine {
private:
	Common::KeymapArray mm1InitKeymaps(const char *target) const;
public:
	const char *getName() const override {
		return "mm";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool MMMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool MM::Xeen::XeenEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error MMMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const MM::MightAndMagicGameDescription *gd = (const MM::MightAndMagicGameDescription *)desc;

	switch (gd->gameID) {
	case MM::GType_MightAndMagic1:
		*engine = new MM::MM1::MM1Engine(syst, gd);
		break;
	case MM::GType_Clouds:
	case MM::GType_DarkSide:
	case MM::GType_WorldOfXeen:
		*engine = new MM::Xeen::WorldOfXeen::WorldOfXeenEngine(syst, gd);
		break;
	case MM::GType_Swords:
		*engine = new MM::Xeen::SwordsOfXeen::SwordsOfXeenEngine(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

SaveStateList MMMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.###", target);
	MM::Xeen::XeenSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot <= MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (MM::Xeen::SavesManager::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));

				delete in;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int MMMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void MMMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor MMMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		MM::Xeen::XeenSavegameHeader header;
		if (!MM::Xeen::SavesManager::readSavegameHeader(f, header, false)) {
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

Common::KeymapArray MMMetaEngine::initKeymaps(const char *target) const {
	return mm1InitKeymaps(target);
}

Common::KeymapArray MMMetaEngine::mm1InitKeymaps(const char *target) const {
	using namespace Common;
	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "griffon", "The Griffon Legend");
	Action *act;

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_FORWARDS);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_BACKWARDS);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Turn Left"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_TURN_LEFT);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Turn Right"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_TURN_RIGHT);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("STRAFE_LEFT", _("Strafe Left"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_STRAFE_LEFT);
	act->addDefaultInputMapping("C+LEFT");
	engineKeyMap->addAction(act);

	act = new Action("STRAFE_RIGHT", _("Strafe Right"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_STRAFE_RIGHT);
	act->addDefaultInputMapping("C+RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("ORDER", _("Order Party"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_ORDER);
	act->addDefaultInputMapping("o");
	engineKeyMap->addAction(act);

	act = new Action("PROTECT", _("Protect"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_PROTECT);
	act->addDefaultInputMapping("p");
	engineKeyMap->addAction(act);

	act = new Action("REST", _("Rest"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_REST);
	act->addDefaultInputMapping("r");
	engineKeyMap->addAction(act);

	act = new Action("SEARCH", _("Search"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_SEARCH);
	act->addDefaultInputMapping("s");
	engineKeyMap->addAction(act);

	act = new Action("BASH", _("Bash"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_BASH);
	act->addDefaultInputMapping("b");
	engineKeyMap->addAction(act);

	act = new Action("UNLOCK", _("Unlock"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_UNLOCK);
	act->addDefaultInputMapping("u");
	engineKeyMap->addAction(act);

	act = new Action("QUICKREF", _("Quick Reference"));
	act->setCustomEngineActionEvent(MM::MM1::ACTION_QUICKREF);
	act->addDefaultInputMapping("q");
	engineKeyMap->addAction(act);

	const char *const PARTY_STRINGS[] = {
		"PARTY1", "Party Member 1",
		"PARTY2", "Party Member 2",
		"PARTY3", "Party Member 3",
		"PARTY4", "Party Member 4",
		"PARTY5", "Party Member 5",
		"PARTY6", "Party Member 6",
	};
	char key[2] = "1";
	for (int i = 0; i < 6; ++i) {
		key[0] = '1' + i;
		act = new Action(PARTY_STRINGS[i * 2], _(PARTY_STRINGS[i * 2 + 1]));
		act->setCustomEngineActionEvent(MM::MM1::ACTION_QUICKREF);
		act->addDefaultInputMapping(key);
		engineKeyMap->addAction(act);
	}

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(MM)
	REGISTER_PLUGIN_DYNAMIC(MM, PLUGIN_TYPE_ENGINE, MMMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MM, PLUGIN_TYPE_ENGINE, MMMetaEngine);
#endif
