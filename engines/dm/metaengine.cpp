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

/*
 * Based on the Reverse Engineering work of Christophe Fontanel,
 * maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
 */

#include "common/config-manager.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "dm/dm.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace DM {

class DMMetaEngine : public AdvancedMetaEngine<DMADGameDescription> {
public:
	const char *getName() const override {
		return "dm";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const DMADGameDescription *desc) const override {
		*engine = new DM::DMEngine(syst, (const DMADGameDescription*)desc);
		return Common::kNoError;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return
			(f == kSupportsListSaves) ||
			(f == kSupportsLoadingDuringStartup) ||
			(f == kSavesSupportThumbnail) ||
			(f == kSavesSupportMetaInfo) ||
			(f == kSimpleSavesNames) ||
			(f == kSavesSupportCreationDate);
	}

	int getMaximumSaveSlot() const override { return 99; }

	SaveStateList listSaves(const char *target) const override {
		Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
		SaveGameHeader header;
		Common::String pattern = target;
		pattern += ".###";

		Common::StringArray filenames = saveFileMan->listSavefiles(pattern.c_str());

		SaveStateList saveList;

		for (const auto &filename : filenames) {
			// Obtain the last 3 digits of the filename, since they correspond to the save slot
			int slotNum = atoi(filename.c_str() + filename.size() - 3);

			if ((slotNum >= 0) && (slotNum <= 999)) {
				Common::InSaveFile *in = saveFileMan->openForLoading(filename.c_str());
				if (in) {
					if (DM::readSaveGameHeader(in, &header))
						saveList.push_back(SaveStateDescriptor(this, slotNum, header._descr.getDescription()));
					delete in;
				}
			}
		}

		// Sort saves based on slot number.
		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::String filename = Common::String::format("%s.%03u", target, slot);
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

		if (in) {
			DM::SaveGameHeader header;

			bool successfulRead = DM::readSaveGameHeader(in, &header);
			delete in;

			if (successfulRead) {
				SaveStateDescriptor desc(this, slot, header._descr.getDescription());

				return header._descr;
			}
		}

		return SaveStateDescriptor();
	}

	bool removeSaveState(const char *target, int slot) const override { return false; }
	Common::KeymapArray initKeymaps(const char *target) const override;
};


Common::KeymapArray DMMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace DM;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "dm-default", _("Default keymappings"));
	Keymap *choiceSelectionKeyMap = new Keymap(Keymap::kKeymapTypeGame, "choice-selection", _("Dialog choice selection keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Select / Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	// I18N: (Game name: Dungeon Master) The player has a team of 4 characters called "champions" which they choose themselves. One of them can be assigned the "leader". This action toggles the inventory screen of the leader champion.
	act = new Action(kStandardActionRightClick, _("Toggle leader inventory"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	// I18N: (Game name: Dungeon Master) The game has multi-choice dialogs. If there is only one choice, then this action can be used to select it.
	act = new Action("SELECTCHOICE", _("Select dialog choice (only works if there is a single choice)"));
	act->setCustomEngineActionEvent(kActionSelectChoice);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_X");
	choiceSelectionKeyMap->addAction(act);

	// I18N: (Game name: Dungeon Master) The player has a team of 4 characters called "champions" which they choose themselves. This action toggles the inventory screen of champion 1.
	act = new Action("CHAMPION1INV", _("Toggle champion 1 inventory"));
	act->setCustomEngineActionEvent(kActionToggleInventoryChampion0);
	act->addDefaultInputMapping("F1"); // F1 (<CSI>1~) Atari ST: Code = 0x3B00
	gameKeyMap->addAction(act);

	// I18N: (Game name: Dungeon Master) The player has a team of 4 characters called "champions" which they choose themselves. This action toggles the inventory screen of champion 2.
	act = new Action("CHAMPION2INV", _("Toggle champion 2 inventory"));
	act->setCustomEngineActionEvent(kActionToggleInventoryChampion1);
	act->addDefaultInputMapping("F2"); // F2 (<CSI>2~) Atari ST: Code = 0x3C00
	gameKeyMap->addAction(act);

	// I18N: (Game name: Dungeon Master) The player has a team of 4 characters called "champions" which they choose themselves. This action toggles the inventory screen of champion 3.
	act = new Action("CHAMPION3INV", _("Toggle champion 3 inventory"));
	act->setCustomEngineActionEvent(kActionToggleInventoryChampion2);
	act->addDefaultInputMapping("F3"); // F3 (<CSI>3~) Atari ST: Code = 0x3D00
	gameKeyMap->addAction(act);

	// I18N: (Game name: Dungeon Master) The player has a team of 4 characters called "champions" which they choose themselves. This action toggles the inventory screen of champion 4.
	act = new Action("CHAMPION4INV", _("Toggle champion 4 inventory"));
	act->setCustomEngineActionEvent(kActionToggleInventoryChampion3);
	act->addDefaultInputMapping("F4"); // F4 (<CSI>4~) Atari ST: Code = 0x3E00
	gameKeyMap->addAction(act);

	act = new Action("SAVEGAME", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("C+s"); // CTRL-S       Atari ST: Code = 0x0013
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("FREEZE", _("Pause"));
	act->setCustomEngineActionEvent(kActionFreezeGame);
	act->addDefaultInputMapping("ESCAPE"); // Esc (0x1B)   Atari ST: Code = 0x001B
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	act = new Action("TURNLEFT", _("Turn left"));
	act->setCustomEngineActionEvent(kActionTurnLeft);
	act->addDefaultInputMapping("KP4"); // Numeric pad 4 Atari ST: Code = 0x5200
	act->addDefaultInputMapping("q"); // Added for convenience
	act->addDefaultInputMapping("S+q"); // Added for convenience
	act->addDefaultInputMapping("DELETE"); // Del (0x7F)
	act->addDefaultInputMapping("S+DELETE"); // Shift Del (0x7F)
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("MOVEFORWARD", _("Move forward"));
	act->setCustomEngineActionEvent(kActionMoveForward);
	act->addDefaultInputMapping("KP4"); // Numeric pad 5 Atari ST: Code = 0x4800
	act->addDefaultInputMapping("w"); // Added for convenience
	act->addDefaultInputMapping("S+w"); // Added for convenience
	act->addDefaultInputMapping("UP"); // Up Arrow (<CSI>A)
	act->addDefaultInputMapping("S+UP"); // Shift Up Arrow (<CSI>T)
	act->addDefaultInputMapping("JOY_UP");
	gameKeyMap->addAction(act);

	act = new Action("TURNRIGHT", _("Turn right"));
	act->setCustomEngineActionEvent(kActionTurnRight);
	act->addDefaultInputMapping("KP6"); // Numeric pad 6 Atari ST: Code = 0x4700
	act->addDefaultInputMapping("e"); // Added for convenience
	act->addDefaultInputMapping("S+e"); // Added for convenience
	act->addDefaultInputMapping("HELP"); // Help (<CSI>?~)
	act->addDefaultInputMapping("S+HELP"); // Shift Help (<CSI>?~)
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("MOVELEFT", _("Strafe left"));
	act->setCustomEngineActionEvent(kActionMoveLeft);
	act->addDefaultInputMapping("KP1"); // Numeric pad 1 Atari ST: Code = 0x4B00
	act->addDefaultInputMapping("a"); // Added for convenience
	act->addDefaultInputMapping("S+a"); // Added for convenience
	act->addDefaultInputMapping("LEFT"); // Backward Arrow (<CSI>D)
	act->addDefaultInputMapping("S+LEFT"); // Shift Backward Arrow (<CSI>V)
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeyMap->addAction(act);

	act = new Action("MOVEBACKWARD", _("Move backward"));
	act->setCustomEngineActionEvent(kActionMoveBackward);
	act->addDefaultInputMapping("KP2"); // Numeric pad 2 Atari ST: Code = 0x5000
	act->addDefaultInputMapping("s"); // Added for convenience
	act->addDefaultInputMapping("S+s"); // Added for convenience
	act->addDefaultInputMapping("DOWN"); // Down Arrow (<CSI>B)
	act->addDefaultInputMapping("S+DOWN"); // Shift Down Arrow (<CSI>S)
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeyMap->addAction(act);

	act = new Action("MOVERIGHT", _("Strafe right"));
	act->setCustomEngineActionEvent(kActionMoveRight);
	act->addDefaultInputMapping("KP3"); // Numeric pad 3 Atari ST: Code = 0x4D00.
	act->addDefaultInputMapping("d"); // Added for convenience
	act->addDefaultInputMapping("S+d"); // Added for convenience
	act->addDefaultInputMapping("RIGHT"); // Forward Arrow (<CSI>C)
	act->addDefaultInputMapping("S+RIGHT"); // Shift Forward Arrow (<CSI>U)
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeyMap->addAction(act);

	// I18N: (Game name: Dungeon Master) The player has a team of 4 characters called "champions" which they choose themselves. The champions can be put to sleep. This action wakes up the sleeping champion.
	act = new Action("WAKEUPCHAMPION", _("Wake up champion"));
	act->setCustomEngineActionEvent(kActionWakeUp);
	act->addDefaultInputMapping("RETURN");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(3);

	keymaps[0] = engineKeyMap;
	keymaps[1] = choiceSelectionKeyMap;
	keymaps[2] = gameKeyMap;

	choiceSelectionKeyMap->setEnabled(false);

	return keymaps;
}

} // End of namespace DM

#if PLUGIN_ENABLED_DYNAMIC(DM)
	REGISTER_PLUGIN_DYNAMIC(DM, PLUGIN_TYPE_ENGINE, DM::DMMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DM, PLUGIN_TYPE_ENGINE, DM::DMMetaEngine);
#endif
