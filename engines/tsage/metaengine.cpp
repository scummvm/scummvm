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

#include "common/config-manager.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"

#include "base/plugins.h"

#include "tsage/tsage.h"
#include "tsage/detection.h"

namespace TsAGE {

const char *TSageEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 TSageEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 TSageEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Language TSageEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Path TSageEngine::getPrimaryFilename() const {
	return Common::Path(_gameDescription->desc.filesDescriptions[0].fileName);
}

} // End of namespace TsAGE

enum {
	MAX_SAVES = 100
};

class TSageMetaEngine : public AdvancedMetaEngine<TsAGE::tSageGameDescription> {
public:
	const char *getName() const override {
		return "tsage";
	}

	bool hasFeature(MetaEngineFeature f) const override {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsDeleteSave:
		case kSupportsLoadingDuringStartup:
		case kSavesSupportMetaInfo:
		case kSavesSupportThumbnail:
		case kSavesSupportCreationDate:
		case kSavesSupportPlayTime:
		case kSimpleSavesNames:
			return true;
		default:
			return false;
		}
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const TsAGE::tSageGameDescription *desc) const override {
		*engine = new TsAGE::TSageEngine(syst, desc);
		return Common::kNoError;
	}

	static Common::String generateGameStateFileName(const char *target, int slot) {
		return Common::String::format("%s.%03d", target, slot);
	}

	SaveStateList listSaves(const char *target) const override {
		Common::String pattern = target;
		pattern += ".###";

		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
		TsAGE::tSageSavegameHeader header;

		SaveStateList saveList;
		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			const char *ext = strrchr(file->c_str(), '.');
			int slot = ext ? atoi(ext + 1) : -1;

			if (slot >= 0 && slot < MAX_SAVES) {
				Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

				if (in) {
					if (TsAGE::Saver::readSavegameHeader(in, header)) {
						saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));
					}

					delete in;
				}
			}
		}

		// Sort saves based on slot number.
		Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
		return saveList;
	}

	int getMaximumSaveSlot() const override {
		return MAX_SAVES - 1;
	}

	bool removeSaveState(const char *target, int slot) const override {
		Common::String filename = Common::String::format("%s.%03d", target, slot);
		return g_system->getSavefileManager()->removeSavefile(filename);
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
			generateGameStateFileName(target, slot));

		if (f) {
			TsAGE::tSageSavegameHeader header;
			if (!TsAGE::Saver::readSavegameHeader(f, header, false)) {
				delete f;
				return SaveStateDescriptor();
			}

			delete f;

			// Create the return descriptor
			SaveStateDescriptor desc(this, slot, header._saveName);
			desc.setThumbnail(header._thumbnail);
			desc.setSaveDate(header._saveYear, header._saveMonth, header._saveDay);
			desc.setSaveTime(header._saveHour, header._saveMinutes);
			desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

			return desc;
		}

		return SaveStateDescriptor();
	}

	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::KeymapArray TSageMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace TsAGE;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "tsage-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Common::Action *act;

	Common::String gameId = ConfMan.get("gameid", target);
	Common::String extra = ConfMan.get("extra", target);
	const bool isDemo = extra.contains("Demo");

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
	act->addDefaultInputMapping("ESCAPE");
	gameKeyMap->addAction(act);

	// I18N: Return refers to return/enter key
	act = new Common::Action("RETURN", _("Return"));
	act->setCustomEngineActionEvent(kActionReturn);
	act->addDefaultInputMapping("RETURN");
	gameKeyMap->addAction(act);

	act = new Common::Action("WALK", _("Walk"));
	act->setCustomEngineActionEvent(kActionWalk);
	act->addDefaultInputMapping("w");
	gameKeyMap->addAction(act);

	act = new Common::Action("LOOK", _("Look"));
	act->setCustomEngineActionEvent(kActionLook);
	act->addDefaultInputMapping("l");
	gameKeyMap->addAction(act);

	act = new Common::Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping("u");
	gameKeyMap->addAction(act);

	act = new Common::Action("TALK", _("Talk"));
	act->setCustomEngineActionEvent(kActionTalk);
	act->addDefaultInputMapping("t");
	gameKeyMap->addAction(act);

	act = new Common::Action("HELP", _("View Help"));
	act->setCustomEngineActionEvent(kActionHelp);
	act->addDefaultInputMapping("F1");
	gameKeyMap->addAction(act);

	act = new Common::Action("SOUNDOPTIONS", _("Sound options"));
	act->setCustomEngineActionEvent(kActionSoundOptions);
	act->addDefaultInputMapping("F2");
	gameKeyMap->addAction(act);

	act = new Common::Action("QUITGAME", _("Quit game"));
	act->setCustomEngineActionEvent(kActionQuitGame);
	act->addDefaultInputMapping("F3");
	gameKeyMap->addAction(act);

	if (!isDemo) {
		act = new Common::Action("RESTARTGAME", _("Restart game"));
		act->setCustomEngineActionEvent(kActionRestartGame);
		act->addDefaultInputMapping("F4");
		gameKeyMap->addAction(act);

		act = new Common::Action("SAVEGAME", _("Save game"));
		act->setCustomEngineActionEvent(kActionSaveGame);
		act->addDefaultInputMapping("F5");
		gameKeyMap->addAction(act);

		act = new Common::Action("RESTOREGAME", _("Restore game"));
		act->setCustomEngineActionEvent(kActionRestoreGame);
		act->addDefaultInputMapping("F7");
		gameKeyMap->addAction(act);

		act = new Common::Action("PAUSEGAME", _("Pause game"));
		act->setCustomEngineActionEvent(kActionPauseGame);
		act->addDefaultInputMapping("F10");
		gameKeyMap->addAction(act);
	}

	if (gameId == "ringworld2") {
		act = new Common::Action("CREDITS", _("Show credits"));
		act->setCustomEngineActionEvent(kActionCredits);
		act->addDefaultInputMapping("F8");
		gameKeyMap->addAction(act);

		act = new Common::Action("UP", _("Crawl North"));
		act->setCustomEngineActionEvent(kActionMoveUpCrawlNorth);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("KP8");
		act->allowKbdRepeats();
		gameKeyMap->addAction(act);

		act = new Common::Action("DOWN", _("Crawl South"));
		act->setCustomEngineActionEvent(kActionMoveDownCrawlSouth);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("KP2");
		act->allowKbdRepeats();
		gameKeyMap->addAction(act);

		act = new Common::Action("LEFT", _("Turn Left/Crawl West"));
		act->setCustomEngineActionEvent(kActionMoveLeftCrawlWest);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
		act->allowKbdRepeats();
		gameKeyMap->addAction(act);

		act = new Common::Action("RIGHT", _("Turn Right/Crawl East"));
		act->setCustomEngineActionEvent(kActionMoveRightCrawlEast);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		act->allowKbdRepeats();
		gameKeyMap->addAction(act);

		act = new Common::Action("INCREASESPEED", _("Increase speed"));
		act->setCustomEngineActionEvent(kActionIncreaseSpeed);
		act->addDefaultInputMapping("KP9");
		gameKeyMap->addAction(act);

		act = new Common::Action("DECREASESPEED", _("Decrease speed"));
		act->setCustomEngineActionEvent(kActionDecreaseSpeed);
		act->addDefaultInputMapping("KP3");
		gameKeyMap->addAction(act);

		act = new Common::Action("MINIMUMSPEED", _("Minimum speed"));
		act->setCustomEngineActionEvent(kActionMinimumSpeed);
		act->addDefaultInputMapping("KP1");
		gameKeyMap->addAction(act);

		act = new Common::Action("MAXIMUMSPEED", _("Maximum speed"));
		act->setCustomEngineActionEvent(kActionMaximumSpeed);
		act->addDefaultInputMapping("KP7");
		gameKeyMap->addAction(act);

		act = new Common::Action("LOWSPEED", _("Low speed"));
		act->setCustomEngineActionEvent(kActionLowSpeed);
		act->addDefaultInputMapping("KP_PERIOD");
		gameKeyMap->addAction(act);

		act = new Common::Action("MEDIUMSPEED", _("Medium speed"));
		act->setCustomEngineActionEvent(kActionMediumSpeed);
		act->addDefaultInputMapping("KP0");
		gameKeyMap->addAction(act);

		act = new Common::Action("DRAWCARDS", _("Draw Cards"));
		act->setCustomEngineActionEvent(kActionDrawCards);
		act->addDefaultInputMapping("SPACE");
		gameKeyMap->addAction(act);
	}

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(TSAGE)
	REGISTER_PLUGIN_DYNAMIC(TSAGE, PLUGIN_TYPE_ENGINE, TSageMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TSAGE, PLUGIN_TYPE_ENGINE, TSageMetaEngine);
#endif
