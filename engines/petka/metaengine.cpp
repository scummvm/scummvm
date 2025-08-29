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
#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "petka/petka.h"

#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

class PetkaMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "petka";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override { return 17; }
	SaveStateList listSaves(const char *target) const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool PetkaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

SaveStateList PetkaMetaEngine::listSaves(const char *target) const {
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
				if (Petka::readSaveHeader(*in.get(), desc))
					saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool PetkaMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(Petka::generateSaveName(slot, target));
}

SaveStateDescriptor PetkaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(Petka::generateSaveName(slot, target)));

	if (f) {
		SaveStateDescriptor desc;
		if (!Petka::readSaveHeader(*f.get(), desc, false))
			return SaveStateDescriptor();

		desc.setSaveSlot(slot);
		return desc;
	}

	return SaveStateDescriptor();
}

Common::Error PetkaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Petka::PetkaEngine(syst, desc);
	return Common::kNoError;
}

Common::KeymapArray PetkaMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Petka;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "petka-default", _("Default keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Interact / Skip dialog"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	// I18N: (Game Name: Red Comrades 1: Save the Galaxy) The game has multiple actions that you can perform(take, use, etc.), the action menu allows you to select one of them.
	act = new Action(kStandardActionRightClick, _("Open action menu / Close current interface"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeymap->addAction(act);

	act = new Action("LOOK", _("Look"));
	act->setCustomEngineActionEvent(kActionCursorLook);
	act->addDefaultInputMapping("1");
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeymap->addAction(act);

	act = new Action("WALK", _("Walk"));
	act->setCustomEngineActionEvent(kActionCursorWalk);
	act->addDefaultInputMapping("2");
	act->addDefaultInputMapping("w");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeymap->addAction(act);

	act = new Action("TAKE", _("Take"));
	act->setCustomEngineActionEvent(kActionCursorTake);
	act->addDefaultInputMapping("3");
	act->addDefaultInputMapping("g");
	act->addDefaultInputMapping("JOY_UP");
	engineKeymap->addAction(act);

	act = new Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionCursorUse);
	act->addDefaultInputMapping("4");
	act->addDefaultInputMapping("u");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeymap->addAction(act);

	act = new Action("TALK", _("Talk"));
	act->setCustomEngineActionEvent(kActionCursorTalk);
	act->addDefaultInputMapping("5");
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("JOY_X");
	engineKeymap->addAction(act);

	// I18N: (Game Name: Red Comrades 1: Save the Galaxy) The game features two characters, but all actions are normally performed by Petka. This action allows you to "use" an object specifically with Chapayev instead.
	act = new Action("CHAPAYEV", _("Chapayev use object"));
	act->setCustomEngineActionEvent(kActionCursorChapayev);
	act->addDefaultInputMapping("6");
	act->addDefaultInputMapping("c");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	engineKeymap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeymap->addAction(act);

	act = new Action("MAP", _("Map"));
	act->setCustomEngineActionEvent(kActionMap);
	act->addDefaultInputMapping("TAB");
	act->addDefaultInputMapping("m");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeymap->addAction(act);

	act = new Action("OPTIONS", _("Show / Hide options screen"));
	act->setCustomEngineActionEvent(kActionOptions);
	act->addDefaultInputMapping("o");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	engineKeymap->addAction(act);

	act = new Action("PREVIOUS_INTERFACE", _("Close current interface"));
	act->setCustomEngineActionEvent(kActionPrevInterface);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	engineKeymap->addAction(act);

	act = new Action("SAVE", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F2");
	engineKeymap->addAction(act);

	act = new Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F3");
	engineKeymap->addAction(act);

	return Keymap::arrayOf(engineKeymap);
}

#if PLUGIN_ENABLED_DYNAMIC(PETKA)
	REGISTER_PLUGIN_DYNAMIC(PETKA, PLUGIN_TYPE_ENGINE, PetkaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PETKA, PLUGIN_TYPE_ENGINE, PetkaMetaEngine);
#endif
