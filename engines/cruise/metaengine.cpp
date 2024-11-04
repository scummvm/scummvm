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

#include "base/plugins.h"
#include "common/savefile.h"
#include "common/system.h"
#include "engines/advancedDetector.h"

#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"


#include "cruise/cruise.h"
#include "cruise/saveload.h"
#include "cruise/detection.h"

namespace Cruise {

const char *CruiseEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Language CruiseEngine::getLanguage() const {
	return _gameDescription->desc.language;
}
Common::Platform CruiseEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace Cruise

class CruiseMetaEngine : public AdvancedMetaEngine<Cruise::CRUISEGameDescription> {
public:
	const char *getName() const override {
		return "cruise";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override { return 99; }

	SaveStateList listSaves(const char *target) const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Cruise::CRUISEGameDescription *desc) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool CruiseMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList CruiseMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern("cruise.s##");

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Cruise::CruiseSavegameHeader header;
				if (Cruise::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(this, slotNum, header.saveName));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool CruiseMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(Cruise::CruiseEngine::getSavegameFile(slot));
}

SaveStateDescriptor CruiseMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
		Cruise::CruiseEngine::getSavegameFile(slot));

	if (f) {
		Cruise::CruiseSavegameHeader header;
		if (!Cruise::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}

		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header.saveName);
		desc.setThumbnail(header.thumbnail);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::Error CruiseMetaEngine::createInstance(OSystem *syst, Engine **engine, const Cruise::CRUISEGameDescription *desc) const {
	*engine = new Cruise::CruiseEngine(syst,desc);
	return Common::kNoError;
}

Common::KeymapArray CruiseMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Cruise;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "cruise-default", _("Default keymappings"));
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

	// I18N: Game runs at faster speed
	act = new Action("FASTMODE", _("Fast mode"));
	act->setCustomEngineActionEvent(kActionFastMode);
	act->addDefaultInputMapping("C+f");
	act->addDefaultInputMapping("JOY_CENTER");
	gameKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("F9");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	// I18N: Opens menu with player commands
	act = new Action("PLAYERMENU", _("Player menu"));
	act->setCustomEngineActionEvent(kActionPlayerMenu);
	act->addDefaultInputMapping("F10");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	act = new Action("INCGAMESPEED", _("Increase game speed"));
	act->setCustomEngineActionEvent(kActionIncreaseGameSpeed);
	act->addDefaultInputMapping("KP_PLUS");
	act->addDefaultInputMapping("JOY_UP");
	gameKeyMap->addAction(act);

	act = new Action("DECGAMESPEED", _("Decrease game speed"));
	act->setCustomEngineActionEvent(kActionDecreaseGameSpeed);
	act->addDefaultInputMapping("KP_MINUS");
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeyMap->addAction(act);

	act = new Action("PAUSE", _("Pause"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_RIGHT_STICK");
	gameKeyMap->addAction(act);

	act = new Action("ESC", _("Escape"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("EXITGAME", _("Exit game"));
	act->setCustomEngineActionEvent(kActionExit);
	act->addDefaultInputMapping("x");
	act->addDefaultInputMapping("JOY_LEFT_STICK");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(CRUISE)
	REGISTER_PLUGIN_DYNAMIC(CRUISE, PLUGIN_TYPE_ENGINE, CruiseMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CRUISE, PLUGIN_TYPE_ENGINE, CruiseMetaEngine);
#endif
