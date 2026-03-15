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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "lilliput/lilliput.h"
#include "lilliput/detection.h"

namespace Lilliput {

uint32 LilliputEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

const char *LilliputEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

} // End of namespace Lilliput

namespace Lilliput {

class LilliputMetaEngine : public AdvancedMetaEngine<LilliputGameDescription> {
public:
	const char *getName() const override {
		return "lilliput";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const LilliputGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("%s-##.SAV", target);
		else
			return Common::String::format("%s-%02d.SAV", target, saveGameIdx);
	}
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error LilliputMetaEngine::createInstance(OSystem *syst, Engine **engine, const LilliputGameDescription *gd) const {
	*engine = new LilliputEngine(syst,gd);
	((LilliputEngine *)*engine)->initGame(gd);
	return Common::kNoError;
}

bool LilliputMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

int LilliputMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList LilliputMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += "-##.SAV";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	char slot[3];
	int slotNum;

	for (const auto &filename : filenames) {
		slot[0] = filename.c_str()[filename.size() - 6];
		slot[1] = filename.c_str()[filename.size() - 5];
		slot[2] = '\0';
		// Obtain the last 2 digits of the filename (without extension), since they correspond to the save slot
		slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *file = saveFileMan->openForLoading(filename);
			if (file) {
				int saveVersion = file->readByte();

				if (saveVersion != kSavegameVersion) {
					warning("Savegame of incompatible version");
					delete file;
					continue;
				}

				// read name
				uint16 nameSize = file->readUint16BE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char name[256];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(this, slotNum, name));
				delete file;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor LilliputMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(getSavegameFile(slot, target));

	if (file) {
		int saveVersion = file->readByte();

		if (saveVersion != kSavegameVersion) {
			warning("Savegame of incompatible version");
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16BE();
		Common::String saveName;
		for (uint32 i = 0; i < saveNameLength; ++i) {
			char curChr = file->readByte();
			saveName += curChr;
		}

		SaveStateDescriptor desc(this, slot, saveName);

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*file, thumbnail)) {
			delete file;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		uint32 saveDate = file->readUint32BE();
		uint16 saveTime = file->readUint16BE();

		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;

		desc.setSaveTime(hour, minutes);

		delete file;
		return desc;
	}
	return SaveStateDescriptor();
}

bool LilliputMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(getSavegameFile(slot, target));
}

Common::KeymapArray LilliputMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Lilliput;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "lilliput-default", "Default keymappings");
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

	act = new Action("SAVE", _("Save Game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	act = new Action("LOAD", _("Load Game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F2");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("TOGGLE_SOUND", _("Toggle Sound"));
	act->setCustomEngineActionEvent(kActionToggleSound);
	act->addDefaultInputMapping("F3");
	act->addDefaultInputMapping("JOY_B");
	gameKeyMap->addAction(act);

	act = new Action("RESTART", _("Restart Game"));
	act->setCustomEngineActionEvent(kActionRestart);
	act->addDefaultInputMapping("F4");
	act->addDefaultInputMapping("JOY_BACK");
	gameKeyMap->addAction(act);

	act = new Action("PAUSE", _("Pause Game"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_START");
	gameKeyMap->addAction(act);

	act = new Action("BOW", _("Bow"));
	act->setCustomEngineActionEvent(kActionBow);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("EYE", _("Eye"));
	act->setCustomEngineActionEvent(kActionEye);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping("u");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("DO", _("Do"));
	act->setCustomEngineActionEvent(kActionDo);
	act->addDefaultInputMapping("d");
	gameKeyMap->addAction(act);

	act = new Action("WHO", _("Who"));
	act->setCustomEngineActionEvent(kActionWho);
	act->addDefaultInputMapping("w");
	gameKeyMap->addAction(act);

	act = new Action("MAP", _("Map"));
	act->setCustomEngineActionEvent(kActionMap);
	act->addDefaultInputMapping("m");
	gameKeyMap->addAction(act);

	act = new Action("FOLLOW", _("Follow"));
	act->setCustomEngineActionEvent(kActionFollow);
	act->addDefaultInputMapping("f");
	gameKeyMap->addAction(act);

	act = new Action("RVN", _("Rvn"));
	act->setCustomEngineActionEvent(kActionRvn);
	act->addDefaultInputMapping("r");
	gameKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("q");
	gameKeyMap->addAction(act);

	act = new Action("UNIT_I", _("Unit I"));
	act->setCustomEngineActionEvent(kActionUnitI);
	act->addDefaultInputMapping("c");
	gameKeyMap->addAction(act);

	act = new Action("UNIT_II", _("Unit II"));
	act->setCustomEngineActionEvent(kActionUnitII);
	act->addDefaultInputMapping("v");
	gameKeyMap->addAction(act);

	act = new Action("UNIT_III", _("Unit III"));
	act->setCustomEngineActionEvent(kActionUnitIII);
	act->addDefaultInputMapping("b");
	gameKeyMap->addAction(act);

	act = new Action("UNIT_IV", _("Unit IV"));
	act->setCustomEngineActionEvent(kActionUnitIV);
	act->addDefaultInputMapping("n");
	gameKeyMap->addAction(act);

	act = new Action("FORM", _("Form"));
	act->setCustomEngineActionEvent(kActionForm);
	act->addDefaultInputMapping("f");
	gameKeyMap->addAction(act);

	act = new Action("ORDER", _("Order"));
	act->setCustomEngineActionEvent(kActionOrder);
	act->addDefaultInputMapping("o");
	gameKeyMap->addAction(act);

	act = new Action("STD", _("Standard"));
	act->setCustomEngineActionEvent(kActionStd);
	act->addDefaultInputMapping("t");
	gameKeyMap->addAction(act);

	act = new Action("MIL_SPACE", _("Military Space"));
	act->setCustomEngineActionEvent(kActionMilSpace);
	act->addDefaultInputMapping("SPACE");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

} // End of namespace Lilliput

#if PLUGIN_ENABLED_DYNAMIC(LILLIPUT)
	REGISTER_PLUGIN_DYNAMIC(LILLIPUT, PLUGIN_TYPE_ENGINE, Lilliput::LilliputMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LILLIPUT, PLUGIN_TYPE_ENGINE, Lilliput::LilliputMetaEngine);
#endif

namespace Lilliput {

void LilliputEngine::initGame(const LilliputGameDescription *gd) {
	_gameType = gd->gameType;
	_platform = gd->desc.platform;
}

} // End of namespace Lilliput
