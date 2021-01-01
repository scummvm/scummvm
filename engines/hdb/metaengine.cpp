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

#include "base/plugins.h"

#include "common/debug.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"
#include "graphics/thumbnail.h"

#include "hdb/hdb.h"
#include "hdb/input.h"
#include "hdb/detection.h"

namespace HDB {

const char *HDBGame::getGameId() const { return _gameDescription->gameId; }
Common::Platform HDBGame::getPlatform() const { return _gameDescription->platform; }

const char *HDBGame::getGameFile() const {
	return _gameDescription->filesDescriptions[0].fileName;
}

uint32 HDBGame::getGameFlags() const {
	return _gameDescription->flags;
}

bool HDBGame::isDemo() const {
	return (getGameFlags() & ADGF_DEMO);
}

bool HDBGame::isPPC() const {
	return (getPlatform() == Common::kPlatformPocketPC);
}

bool HDBGame::isHandango() const {
	return (getGameFlags() & GF_HANDANGO);
}

} // End of namespace HDB

class HDBMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "hdb";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override;

	void removeSaveState(const char *target, int slot) const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool HDBMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportPlayTime);
}

bool HDB::HDBGame::hasFeature(Engine::EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime);
}

void HDBMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int HDBMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList HDBMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::ScopedPtr<Common::InSaveFile> in(saveFileMan->openForLoading(*file));
			if (in) {
				SaveStateDescriptor desc;
				char mapName[32];
				Graphics::Surface *thumbnail;

				if (!Graphics::loadThumbnail(*in, thumbnail)) {
					warning("Error loading thumbnail for %s", file->c_str());
				}
				desc.setThumbnail(thumbnail);

				uint32 timeSeconds = in->readUint32LE();
				in->read(mapName, 32);

				debug(1, "mapName: %s playtime: %d", mapName, timeSeconds);

				desc.setSaveSlot(slotNum);
				desc.setPlayTime(timeSeconds * 1000);

				if (slotNum < 8)
					desc.setDescription(Common::String::format("Auto: %s", mapName));
				else
					desc.setDescription(mapName);

				saveList.push_back(desc);
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor HDBMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::ScopedPtr<Common::InSaveFile> in(g_system->getSavefileManager()->openForLoading(Common::String::format("%s.%03d", target, slot)));

	if (in) {
		SaveStateDescriptor desc;
		char mapName[32];
		Graphics::Surface *thumbnail;

		if (!Graphics::loadThumbnail(*in, thumbnail)) {
			warning("Error loading thumbnail");
		}
		desc.setThumbnail(thumbnail);

		uint32 timeSeconds = in->readUint32LE();
		in->read(mapName, 32);

		desc.setSaveSlot(slot);
		desc.setPlayTime(timeSeconds * 1000);
		desc.setDescription(mapName);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::KeymapArray HDBMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace HDB;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "hdb", "Hyperspace Delivery Boy!");

	Action *act;

	act = new Action("LCLK", _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Move up"));
	act->setCustomEngineActionEvent(kHDBActionUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Move down"));
	act->setCustomEngineActionEvent(kHDBActionDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Move left"));
	act->setCustomEngineActionEvent(kHDBActionLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Move right"));
	act->setCustomEngineActionEvent(kHDBActionRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kHDBActionUse);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("CLEAR", _("Clear waypoints"));
	act->setCustomEngineActionEvent(kHDBActionClearWaypoints);
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

#if 0
	act = new Action("INV", _("Inventory"));
	act->setCustomEngineActionEvent(kHDBActionInventory);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);
#endif

	act = new Action(kStandardActionPause, _("Pause"));
	act->setCustomEngineActionEvent(kHDBActionPause);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionOpenMainMenu, _("Menu"));
	act->setCustomEngineActionEvent(kHDBActionMenu);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("DEBUG", _("Debug"));
	act->setCustomEngineActionEvent(kHDBActionDebug);
	act->addDefaultInputMapping("F1");
	engineKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kHDBActionQuit);
	act->addDefaultInputMapping("F10");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

Common::Error HDBMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new HDB::HDBGame(syst, desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(HDB)
	REGISTER_PLUGIN_DYNAMIC(HDB, PLUGIN_TYPE_ENGINE, HDBMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(HDB, PLUGIN_TYPE_ENGINE, HDBMetaEngine);
#endif
