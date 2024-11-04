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

#include "draci/draci.h"
#include "draci/saveload.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "base/plugins.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "engines/metaengine.h"

class DraciMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "draci";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override { return 99; }
	SaveStateList listSaves(const char *target) const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool DraciMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList DraciMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern("draci.s##");

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Draci::DraciSavegameHeader header;
				if (Draci::readSavegameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(this, slotNum, header.saveName));
				}
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool DraciMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(Draci::DraciEngine::getSavegameFile(slot));
}

SaveStateDescriptor DraciMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
		Draci::DraciEngine::getSavegameFile(slot));

	if (f) {
		Draci::DraciSavegameHeader header;
		if (!Draci::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}

		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header.saveName);
		desc.setThumbnail(header.thumbnail);

		int day = (header.date >> 24) & 0xFF;
		int month = (header.date >> 16) & 0xFF;
		int year = header.date & 0xFFFF;
		desc.setSaveDate(year, month, day);

		int hour = (header.time >> 8) & 0xFF;
		int minutes = header.time & 0xFF;
		desc.setSaveTime(hour, minutes);

		desc.setPlayTime(header.playtime * 1000);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::Error DraciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Draci::DraciEngine(syst, desc);
	return Common::kNoError;
}

Common::KeymapArray DraciMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Draci;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "draci-default", _("Default keymappings"));
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

	act = new Action("ESCAPE", _("Skip intro/Exit map or inventory"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("MAP", _("Open map"));
	act->setCustomEngineActionEvent(kActionMap);
	act->addDefaultInputMapping("m");
	act->addDefaultInputMapping("JOY_A");
	gameKeyMap->addAction(act);

	// I18N: shows where the game actor is able to move
	act = new Action("WALKMAP", _("Show walking map"));
	act->setCustomEngineActionEvent(kActionShowWalkMap);
	act->addDefaultInputMapping("w");
	act->addDefaultInputMapping("JOY_LEFT_STICK");
	gameKeyMap->addAction(act);

	act = new Action("TOGGLEWALKSPEED", _("Toggle walk speed"));
	act->setCustomEngineActionEvent(kActionToggleWalkSpeed);
	act->addDefaultInputMapping("q");
	act->addDefaultInputMapping("JOY_RIGHT_STICK");
	gameKeyMap->addAction(act);

	act = new Action("INV", _("Inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("JOY_B");
	gameKeyMap->addAction(act);

	act = new Action("MAINMENU", _("Open main menu"));
	act->setCustomEngineActionEvent(kActionOpenMainMenu);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("POINTERORITEM", _("Toggle between mouse pointer and the last game item"));
	act->setCustomEngineActionEvent(kActionTogglePointerItem);
	act->addDefaultInputMapping("SLASH");
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeyMap->addAction(act);

	act = new Action("PREVITEM", _("Previous item in inventory"));
	act->setCustomEngineActionEvent(kActionInvRotatePrevious);
	act->addDefaultInputMapping("COMMA");
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeyMap->addAction(act);

	act = new Action("NEXTITEM", _("Next item in inventory"));
	act->setCustomEngineActionEvent(kActionInvRotateNext);
	act->addDefaultInputMapping("PERIOD");
	act->addDefaultInputMapping("JOY_UP");
	gameKeyMap->addAction(act);


	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(DRACI)
	REGISTER_PLUGIN_DYNAMIC(DRACI, PLUGIN_TYPE_ENGINE, DraciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRACI, PLUGIN_TYPE_ENGINE, DraciMetaEngine);
#endif
