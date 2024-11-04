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

#include "bbvs/bbvs.h"

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Bbvs {

bool BbvsEngine::isDemo() const {
	return _gameDescription->flags & ADGF_DEMO;
}

bool BbvsEngine::isLoogieDemo() const {
	return _gameDescription->flags & GF_LOOGIE_DEMO;
}

bool BbvsEngine::isLoogieAltDemo() const {
	return _gameDescription->flags & GF_LOOGIE_ALT_DEMO;
}

} // End of namespace Bbvs

class BbvsMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "bbvs";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;

	// Disable autosave (see mirrored method in bbvs.h for detailed explanation)
	int getAutosaveSlot() const override { return -1; }

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool BbvsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate) ||
		(f == kSimpleSavesNames);
}

bool BbvsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::KeymapArray BbvsMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Bbvs;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "bbvs-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *escapeKeyMap = new Keymap(Keymap::kKeymapTypeGame, "escape-shortcuts", _("Escape keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Common::Action("ESCAPE", _("Escape"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("JOY_X");
	act->addDefaultInputMapping("ESCAPE");
	escapeKeyMap->addAction(act);

	act = new Common::Action("INVENTORY", _("Open inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_y");
	gameKeyMap->addAction(act);

	act = new Common::Action("LOOK", _("Look"));
	act->setCustomEngineActionEvent(kActionLook);
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeyMap->addAction(act);

	act = new Common::Action("TALK", _("Talk"));
	act->setCustomEngineActionEvent(kActionTalk);
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeyMap->addAction(act);

	act = new Common::Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping("u");
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeyMap->addAction(act);

	act = new Common::Action("WALK", _("Walk"));
	act->setCustomEngineActionEvent(kActionWalk);
	act->addDefaultInputMapping("w");
	act->addDefaultInputMapping("JOY_UP");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(3);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;
	keymaps[2] = escapeKeyMap;

	return keymaps;
}

int BbvsMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

SaveStateList BbvsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Bbvs::BbvsEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".###";
	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Bbvs::BbvsEngine::readSaveHeader(in, header) == Bbvs::BbvsEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(this, slotNum, header.description));
				}
				delete in;
			}
		}
	}
	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor BbvsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Bbvs::BbvsEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());
	if (in) {
		Bbvs::BbvsEngine::SaveHeader header;
		Bbvs::BbvsEngine::kReadSaveHeaderError error;
		error = Bbvs::BbvsEngine::readSaveHeader(in, header, false);
		delete in;
		if (error == Bbvs::BbvsEngine::kRSHENoError) {
			SaveStateDescriptor desc(this, slot, header.description);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveDate & 0xFFFF, (header.saveDate >> 16) & 0xFF, (header.saveDate >> 24) & 0xFF);
			desc.setSaveTime((header.saveTime >> 16) & 0xFF, (header.saveTime >> 8) & 0xFF);
			desc.setPlayTime(header.playTime * 1000);
			return desc;
		}
	}
	return SaveStateDescriptor();
}

Common::Error BbvsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Bbvs::BbvsEngine(syst, desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(BBVS)
	REGISTER_PLUGIN_DYNAMIC(BBVS, PLUGIN_TYPE_ENGINE, BbvsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(BBVS, PLUGIN_TYPE_ENGINE, BbvsMetaEngine);
#endif
