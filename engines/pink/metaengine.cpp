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

#include "common/system.h"

#include "engines/advancedDetector.h"

#include "pink/pink.h"

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Pink {

Common::Language PinkEngine::getLanguage() const {
	return _desc->language;
}

} // End of Namespace Pink

class PinkMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "pink";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override { return 99; }
	SaveStateList listSaves(const char *target) const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool PinkMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList PinkMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = Common::String::format("%s.s##", target);
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::ScopedPtr<Common::InSaveFile> in(saveFileMan->openForLoading(*file));
			if (in) {
				SaveStateDescriptor desc;
				desc.setSaveSlot(slotNum);
				if (Pink::readSaveHeader(*in.get(), desc))
					saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool PinkMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(Pink::generateSaveName(slot, target));
}

SaveStateDescriptor PinkMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(Pink::generateSaveName(slot, target)));

	if (f) {
		SaveStateDescriptor desc;
		if (!Pink::readSaveHeader(*f.get(), desc, false))
			return SaveStateDescriptor();

		return desc;
	}

	return SaveStateDescriptor();
}

Common::Error PinkMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Pink::PinkEngine(syst, desc);
	return Common::kNoError;
}

Common::KeymapArray PinkMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Pink;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "pink-default", "Default keymappings");

	Action *act;

	String gameId = ConfMan.get("gameid", target);

	act = new Action(kStandardActionLeftClick, _("Interact / Select"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	if (gameId == "peril") {
		// I18N: (Game Name: Pink Panther) The player normally clicks on a target (character/object) to move toward it and interact (talk,use etc.) with it. This action makes the player still walk to the target but not start interacting when they arrive.
		act = new Action(kStandardActionRightClick, _("Cancel interaction"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		engineKeyMap->addAction(act);
	}

	// I18N: (Game Name: Pink Panther) The player normally clicks on a target (character/object) to move toward it and interact (talk,use etc.) with it. This action skips the walking animation and immediately start interacting.
	act = new Action("SKIPWALK", _("Skip walk"));
	act->setCustomEngineActionEvent(kActionSkipWalk);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	// I18N: (Game Name: Pink Panther) The player normally clicks on a target (character/object) to move toward it and interact (talk,use etc.) with it. This action skips the walking animation and also prevents the interaction, instead instantly placing the player next to the target.
	act = new Action("SKIPWALKANDCANCEL", _("Skip walk and cancel interaction"));
	act->setCustomEngineActionEvent(kActionSkipWalkAndCancelInteraction);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	// I18N: (Game Name: Pink Panther) This action fully skips the current running sequence (cutscene, dialog, interaction, etc).
	act = new Action("SKIPSEQUENCE", _("Skip sequence"));
	act->setCustomEngineActionEvent(kActionSkipSequence);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	// I18N: (Game Name: Pink Panther) This action skips part of the current running sequence (cutscene, dialog, interaction, etc).
	act = new Action("SKIPSUBSEQUENCE", _("Skip sub-sequence"));
	act->setCustomEngineActionEvent(kActionSkipSubSequence);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	// I18N: (Game Name: Pink Panther) This action restarts the current running sequence (cutscene, dialog, interaction, etc).
	act = new Action("RESTARTSEQUENCE", _("Restart sequence"));
	act->setCustomEngineActionEvent(kActionRestartSequence);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(PINK)
	REGISTER_PLUGIN_DYNAMIC(PINK, PLUGIN_TYPE_ENGINE, PinkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PINK, PLUGIN_TYPE_ENGINE, PinkMetaEngine);
#endif

