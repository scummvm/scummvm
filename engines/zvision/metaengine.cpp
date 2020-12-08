/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "engines/advancedDetector.h"

#include "zvision/zvision.h"
#include "zvision/file/save_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

namespace ZVision {

ZVisionGameId ZVision::getGameId() const {
	return _gameDescription->gameId;
}
Common::Language ZVision::getLanguage() const {
	return _gameDescription->desc.language;
}
uint32 ZVision::getFeatures() const {
	return _gameDescription->desc.flags;
}

} // End of namespace ZVision

class ZVisionMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "zvision";
	}

    bool hasFeature(MetaEngineFeature f) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
};

bool ZVisionMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool ZVision::ZVision::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error ZVision::ZVision::loadGameState(int slot) {
	return _saveManager->loadGame(slot);
}

Common::Error ZVision::ZVision::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	_saveManager->saveGame(slot, desc, false);
	return Common::kNoError;
}

bool ZVision::ZVision::canLoadGameStateCurrently() {
	return !_videoIsPlaying;
}

bool ZVision::ZVision::canSaveGameStateCurrently() {
	Location currentLocation = _scriptManager->getCurrentLocation();
	return !_videoIsPlaying && currentLocation.world != 'g' && !(currentLocation.room == 'j' || currentLocation.room == 'a');
}

Common::KeymapArray ZVisionMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace ZVision;

	Keymap *mainKeymap = new Keymap(Keymap::kKeymapTypeGame, mainKeymapId, "Z-Vision");

	Action *act;

	act = new Action("LCLK", _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	mainKeymap->addAction(act);

	act = new Action("RCLK", _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	mainKeymap->addAction(act);

	Keymap *gameKeymap = new Keymap(Keymap::kKeymapTypeGame, gameKeymapId, "Z-Vision - Game");

	act = new Action(kStandardActionMoveUp, _("Look Up"));
	act->setCustomEngineActionEvent(kZVisionActionUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	gameKeymap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Look Down"));
	act->setCustomEngineActionEvent(kZVisionActionDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeymap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Turn Left"));
	act->setCustomEngineActionEvent(kZVisionActionLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeymap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Turn Right"));
	act->setCustomEngineActionEvent(kZVisionActionRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeymap->addAction(act);

	act = new Action("FPS", _("Show FPS"));
	act->setCustomEngineActionEvent(kZVisionActionShowFPS);
	act->addDefaultInputMapping("F10");
	gameKeymap->addAction(act);

	act = new Action("HELP", _("Help"));
	act->setKeyEvent(KEYCODE_F1);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	gameKeymap->addAction(act);

	act = new Action("INV", _("Inventory"));
	act->setKeyEvent(KEYCODE_F5);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeymap->addAction(act);

	act = new Action("SPELL", _("Spellbook"));
	act->setKeyEvent(KEYCODE_F6);
	act->addDefaultInputMapping("F6");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeymap->addAction(act);

	act = new Action("SCORE", _("Score"));
	act->setKeyEvent(KEYCODE_F7);
	act->addDefaultInputMapping("F7");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	gameKeymap->addAction(act);

	act = new Action("AWAY", _("Put away object"));
	act->setKeyEvent(KEYCODE_F8);
	act->addDefaultInputMapping("F8");
	act->addDefaultInputMapping("JOY_X");
	gameKeymap->addAction(act);

	act = new Action("COIN", _("Extract coin"));
	act->setKeyEvent(KEYCODE_F9);
	act->addDefaultInputMapping("F9");
	act->addDefaultInputMapping("JOY_Y");
	gameKeymap->addAction(act);

	act = new Action(kStandardActionSave, _("Save"));
	act->setCustomEngineActionEvent(kZVisionActionSave);
	act->addDefaultInputMapping("C+s");
	gameKeymap->addAction(act);

	act = new Action(kStandardActionLoad, _("Restore"));
	act->setCustomEngineActionEvent(kZVisionActionRestore);
	act->addDefaultInputMapping("C+r");
	gameKeymap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kZVisionActionQuit);
	act->addDefaultInputMapping("C+q");
	gameKeymap->addAction(act);

	act = new Action(kStandardActionOpenSettings, _("Preferences"));
	act->setCustomEngineActionEvent(kZVisionActionPreferences);
	act->addDefaultInputMapping("C+p");
	gameKeymap->addAction(act);

	Keymap *cutscenesKeymap = new Keymap(Keymap::kKeymapTypeGame, cutscenesKeymapId, "Z-Vision - Cutscenes");

	act = new Action(kStandardActionSkip, _("Skip cutscene"));
	act->setCustomEngineActionEvent(kZVisionActionSkipCutscene);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_Y");
	cutscenesKeymap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kZVisionActionQuit);
	act->addDefaultInputMapping("C+q");
	cutscenesKeymap->addAction(act);

	KeymapArray keymaps(3);
	keymaps[0] = mainKeymap;
	keymaps[1] = gameKeymap;
	keymaps[2] = cutscenesKeymap;

	return keymaps;
}

Common::Error ZVisionMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new ZVision::ZVision(syst, (const ZVision::ZVisionGameDescription *)desc);
	return Common::kNoError;
}

SaveStateList ZVisionMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	ZVision::SaveGameHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());

	SaveStateList saveList;
	// We only use readSaveGameHeader() here, which doesn't need an engine callback
	ZVision::SaveManager *zvisionSaveMan = new ZVision::SaveManager(NULL);

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (zvisionSaveMan->readSaveGameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
				}
				delete in;
			}
		}
	}

	delete zvisionSaveMan;

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int ZVisionMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void ZVisionMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	saveFileMan->removeSavefile(Common::String::format("%s.%03u", target, slot));
}

SaveStateDescriptor ZVisionMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03u", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		ZVision::SaveGameHeader header;

		// We only use readSaveGameHeader() here, which doesn't need an engine callback
		ZVision::SaveManager *zvisionSaveMan = new ZVision::SaveManager(NULL);
		bool successfulRead = zvisionSaveMan->readSaveGameHeader(in, header, false);
		delete zvisionSaveMan;
		delete in;

		if (successfulRead) {
			SaveStateDescriptor desc(slot, header.saveName);

			// Do not allow save slot 0 (used for auto-saving) to be deleted or
			// overwritten.
			desc.setDeletableFlag(slot != 0);
			desc.setWriteProtectedFlag(slot == 0);

			desc.setThumbnail(header.thumbnail);

			if (header.version >= 1) {
				int day = header.saveDay;
				int month = header.saveMonth;
				int year = header.saveYear;

				desc.setSaveDate(year, month, day);

				int hour = header.saveHour;
				int minutes = header.saveMinutes;

				desc.setSaveTime(hour, minutes);
			}

			if (header.version >= 2) {
				desc.setPlayTime(header.playTime * 1000);
			}

			return desc;
		}
	}

	return SaveStateDescriptor();
}

#if PLUGIN_ENABLED_DYNAMIC(ZVISION)
	REGISTER_PLUGIN_DYNAMIC(ZVISION, PLUGIN_TYPE_ENGINE, ZVisionMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ZVISION, PLUGIN_TYPE_ENGINE, ZVisionMetaEngine);
#endif
