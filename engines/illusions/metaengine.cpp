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

#include "illusions/illusions.h"
#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/duckman/illusions_duckman.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/config-manager.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

#include "illusions/detection.h"

namespace Illusions {

int IllusionsEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Language IllusionsEngine::getGameLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Illusions

class IllusionsMetaEngine : public AdvancedMetaEngine<Illusions::IllusionsGameDescription> {
public:
	const char *getName() const override {
		return "illusions";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Illusions::IllusionsGameDescription *desc) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool IllusionsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesSupportCreationDate);
}

bool IllusionsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::KeymapArray IllusionsMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Illusions;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "illusions-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Common::Action *act;

	act = new Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	act->addDefaultInputMapping("RETURN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("BACKSPACE");
	engineKeyMap->addAction(act);

	act = new Action("CRSRUP", _("Move cursor up"));
	act->setCustomEngineActionEvent(kActionCursorUp);
	act->addDefaultInputMapping("JOY_UP");
	act->addDefaultInputMapping("UP");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("CRSRDOWN", _("Move cursor down"));
	act->setCustomEngineActionEvent(kActionCursorDown);
	act->addDefaultInputMapping("JOY_DOWN");
	act->addDefaultInputMapping("DOWN");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("CRSRLEFT", _("Move cursor left"));
	act->setCustomEngineActionEvent(kActionCursorLeft);
	act->addDefaultInputMapping("JOY_LEFT");
	act->addDefaultInputMapping("LEFT");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("CRSRRIGHT", _("Move cursor right"));
	act->setCustomEngineActionEvent(kActionCursorRight);
	act->addDefaultInputMapping("JOY_RIGHT");
	act->addDefaultInputMapping("RIGHT");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("ABORT", _("Abort"));
	act->setCustomEngineActionEvent(kActionAbort);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("SKIP", _("Skip"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("SPACE");
	gameKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Open inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("TAB");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	act = new Action("ENCHEAT", _("Enable cheat mode"));
	act->setCustomEngineActionEvent(kActionCheatMode);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

int IllusionsMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

SaveStateList IllusionsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Illusions::IllusionsEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".???";
	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);
		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Illusions::IllusionsEngine::readSaveHeader(in, header) == Illusions::IllusionsEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(this, slotNum, header.description));
				}
				delete in;
			}
		}
	}
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor IllusionsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Illusions::IllusionsEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());
	if (in) {
		Illusions::IllusionsEngine::SaveHeader header;
		Illusions::IllusionsEngine::kReadSaveHeaderError error;
		error = Illusions::IllusionsEngine::readSaveHeader(in, header, false);
		delete in;
		if (error == Illusions::IllusionsEngine::kRSHENoError) {
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

Common::Error IllusionsMetaEngine::createInstance(OSystem *syst, Engine **engine, const Illusions::IllusionsGameDescription *gd) const {
	switch (gd->gameId) {
	case Illusions::kGameIdBBDOU:
		*engine = new Illusions::IllusionsEngine_BBDOU(syst, gd);
		break;
	case Illusions::kGameIdDuckman:
		*engine = new Illusions::IllusionsEngine_Duckman(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(ILLUSIONS)
	REGISTER_PLUGIN_DYNAMIC(ILLUSIONS, PLUGIN_TYPE_ENGINE, IllusionsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ILLUSIONS, PLUGIN_TYPE_ENGINE, IllusionsMetaEngine);
#endif
