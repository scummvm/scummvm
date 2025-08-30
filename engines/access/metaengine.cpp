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


#include "common/savefile.h"
#include "common/system.h"

#include "engines/advancedDetector.h"

#include "graphics/surface.h"

#include "access/access.h"
#include "access/amazon/amazon_game.h"
#include "access/martian/martian_game.h"

#include "access/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/translation.h"

#define MAX_SAVES 99

namespace Access {

uint32 AccessEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 AccessEngine::getGameFeatures() const {
	return _gameDescription->features;
}

uint32 AccessEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

bool AccessEngine::isCD() const {
	return (bool)(_gameDescription->desc.flags & ADGF_CD);
}

bool AccessEngine::isDemo() const {
	return (bool)(_gameDescription->desc.flags & ADGF_DEMO);
}

Common::Language AccessEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform AccessEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace Access

class AccessMetaEngine : public AdvancedMetaEngine<Access::AccessGameDescription> {
public:
	const char *getName() const override {
		return "access";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const Access::AccessGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool AccessMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool Access::AccessEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error AccessMetaEngine::createInstance(OSystem *syst, Engine **engine, const Access::AccessGameDescription *gd) const {
	switch (gd->gameID) {
	case Access::kGameAmazon:
		*engine = new Access::Amazon::AmazonEngine(syst, gd);
		break;
	case Access::kGameMartianMemorandum:
		*engine = new Access::Martian::MartianEngine(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

SaveStateList AccessMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);
	Access::AccessSavegameHeader header;
	SaveStateList saveList;

	for (const auto &filename : filenames) {
		const char *ext = strrchr(filename.c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename);

			if (in) {
				if (Access::AccessEngine::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));

				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int AccessMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

bool AccessMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor AccessMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Access::AccessSavegameHeader header;
		if (!Access::AccessEngine::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}

		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);
		desc.setThumbnail(header._thumbnail);
		desc.setSaveDate(header._year, header._month, header._day);
		desc.setSaveTime(header._hour, header._minute);
		desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::KeymapArray AccessMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Access;

	// Get the game ID for the target
	const Common::String currDomain = ConfMan.getActiveDomainName();
	ConfMan.setActiveDomain(target);
	const Common::String gameId = ConfMan.get("gameid");
	ConfMan.setActiveDomain(currDomain);

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "access-default", _("Default keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Move / Interact / Skip"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Skip"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("SKIP", _("Skip movie"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("UP", _("Move up"));
	act->setCustomEngineActionEvent(kActionMoveUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("KP8");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action("DOWN", _("Move down"));
	act->setCustomEngineActionEvent(kActionMoveDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("KP2");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action("LEFT", _("Move left"));
	act->setCustomEngineActionEvent(kActionMoveLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("KP4");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action("RIGHT", _("Move right"));
	act->setCustomEngineActionEvent(kActionMoveRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("KP6");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("UPLEFT", _("Move up-left"));
	act->setCustomEngineActionEvent(kActionMoveUpLeft);
	act->addDefaultInputMapping("KP7");
	engineKeyMap->addAction(act);

	act = new Action("UPRIGHT", _("Move up-right"));
	act->setCustomEngineActionEvent(kActionMoveUpRight);
	act->addDefaultInputMapping("KP9");
	engineKeyMap->addAction(act);

	act = new Action("DOWNLEFT", _("Move down-left"));
	act->setCustomEngineActionEvent(kActionMoveDownLeft);
	act->addDefaultInputMapping("KP1");
	engineKeyMap->addAction(act);

	act = new Action("DOWNRIGHT", _("Move down-right"));
	act->setCustomEngineActionEvent(kActionMoveDownRight);
	act->addDefaultInputMapping("KP3");
	engineKeyMap->addAction(act);

	act = new Action("LOOK", _("Look"));
	act->setCustomEngineActionEvent(kActionLook);
	act->addDefaultInputMapping("F1");
	if (strcmp(target, "amazon") == 0)
		act->addDefaultInputMapping("F2");
	engineKeyMap->addAction(act);

	if (gameId.equals("martian")) {
		act = new Action("OPEN", _("Open"));
		act->setCustomEngineActionEvent(kActionOpen);
		act->addDefaultInputMapping("F2");
		engineKeyMap->addAction(act);

		act = new Action("MOVE", _("Move"));
		act->setCustomEngineActionEvent(kActionMove);
		act->addDefaultInputMapping("F3");
		engineKeyMap->addAction(act);

		act = new Action("GET", _("Get"));
		act->setCustomEngineActionEvent(kActionTake);
		act->addDefaultInputMapping("F4");
		engineKeyMap->addAction(act);

		act = new Action("USE", _("Use"));
		act->setCustomEngineActionEvent(kActionUse);
		act->addDefaultInputMapping("F5");
		engineKeyMap->addAction(act);

		act = new Action("GOTO", _("Goto"));
		act->setCustomEngineActionEvent(kActionWalk);
		act->addDefaultInputMapping("F6");
		engineKeyMap->addAction(act);

		act = new Action("TALK", _("Talk"));
		act->setCustomEngineActionEvent(kActionTalk);
		act->addDefaultInputMapping("F7");
		engineKeyMap->addAction(act);

		act = new Action("TRAVEL", _("Travel"));
		act->setCustomEngineActionEvent(kActionTravel);
		act->addDefaultInputMapping("F8");
		engineKeyMap->addAction(act);
	} else {
		// Amazon keymaps
		act = new Action("USE", _("Use"));
		act->setCustomEngineActionEvent(kActionUse);
		act->addDefaultInputMapping("F3");
		engineKeyMap->addAction(act);

		act = new Action("TAKE", _("Take"));
		act->setCustomEngineActionEvent(kActionTake);
		act->addDefaultInputMapping("F4");
		engineKeyMap->addAction(act);

		act = new Action("INVENTORY", _("Inventory"));
		act->setCustomEngineActionEvent(kActionInventory);
		act->addDefaultInputMapping("F5");
		engineKeyMap->addAction(act);

		act = new Action("CLIMB", _("Climb"));
		act->setCustomEngineActionEvent(kActionClimb);
		act->addDefaultInputMapping("F6");
		engineKeyMap->addAction(act);

		act = new Action("TALK", _("Talk"));
		act->setCustomEngineActionEvent(kActionTalk);
		act->addDefaultInputMapping("F7");
		engineKeyMap->addAction(act);

		act = new Action("WALK", _("Walk"));
		act->setCustomEngineActionEvent(kActionWalk);
		act->addDefaultInputMapping("F8");
		engineKeyMap->addAction(act);
	}

	act = new Action("HELP", _("Help"));
	act->setCustomEngineActionEvent(kActionHelp);
	act->addDefaultInputMapping("F9");
	engineKeyMap->addAction(act);

	act = new Action("SAVELOAD", _("Open save/load menu"));
	act->setCustomEngineActionEvent(kActionSaveLoad);
	act->addDefaultInputMapping("F10");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(ACCESS)
	REGISTER_PLUGIN_DYNAMIC(ACCESS, PLUGIN_TYPE_ENGINE, AccessMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ACCESS, PLUGIN_TYPE_ENGINE, AccessMetaEngine);
#endif
