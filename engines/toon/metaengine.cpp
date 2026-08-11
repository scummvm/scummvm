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
 *
 * This file is dual-licensed.
 * In addition to the GPLv3 license mentioned above, MojoTouch has
 * exclusively licensed this code on March 23th, 2024, to be used in
 * closed-source products.
 * Therefore, any contributions (commits) to it will also be dual-licensed.
 *
 */

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "base/plugins.h"
#include "graphics/thumbnail.h"
#include "toon/toon.h"

class ToonMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "toon";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool ToonMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool ToonMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::KeymapArray ToonMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Toon;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "toon-default", _("Default keymappings"));
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

	act = new Action("ESCAPE", _("Skip intro"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_BACK");
	gameKeyMap->addAction(act);

	// I18N: Skips current line being spoken by a character
	act = new Action("STOPCURRENTVOICE", _("Stop current voice"));
	act->setCustomEngineActionEvent(kActionStopCurrentVoice);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Action("SAVEGAME", _("Save game"));
	act->setCustomEngineActionEvent(kActionSaveGame);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("LOADGAME", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoadGame);
	act->addDefaultInputMapping("F6");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("SUBTITLES", _("Toggle subtitles"));
	act->setCustomEngineActionEvent(kActionSubtitles);
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("JOY_UP");
	gameKeyMap->addAction(act);

	act = new Action("MUTEMUSIC", _("Mute music"));
	act->setCustomEngineActionEvent(kActionMuteMusic);
	act->addDefaultInputMapping("m");
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeyMap->addAction(act);

	act = new Action("SPEECHMUTE", _("Mute speech"));
	act->setCustomEngineActionEvent(kActionSpeechMute);
	act->addDefaultInputMapping("d");
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeyMap->addAction(act);

	act = new Action("SFXMUTE", _("Mute sound effects"));
	act->setCustomEngineActionEvent(kActionSFXMute);
	act->addDefaultInputMapping("s");
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeyMap->addAction(act);

	act = new Action("SHOWOPTIONS", _("Show options"));
	act->setCustomEngineActionEvent(kActionShowOptions);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

int ToonMetaEngine::getMaximumSaveSlot() const { return MAX_SAVE_SLOT; }

SaveStateList ToonMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= MAX_SAVE_SLOT) {
			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				int32 version = file->readSint32BE();
				if ( (version < 4) || (version > TOON_SAVEGAME_VERSION) ) {
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

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor ToonMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {

		int32 version = file->readSint32BE();
		if ( (version < 4) || (version > TOON_SAVEGAME_VERSION) ) {
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16BE();
		char saveName[256];
		file->read(saveName, saveNameLength);
		saveName[saveNameLength] = 0;

		SaveStateDescriptor desc(this, slot, saveName);

		Graphics::Surface *thumbnail = nullptr;
		if (!Graphics::loadThumbnail(*file, thumbnail, false)) {
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

		if (version >= 5) {
			uint32 playTimeMsec = file->readUint32BE();
			desc.setPlayTime(playTimeMsec);
		}

		delete file;
		return desc;
	}

	return SaveStateDescriptor();
}

Common::Error ToonMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Toon::ToonEngine(syst, desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(TOON)
	REGISTER_PLUGIN_DYNAMIC(TOON, PLUGIN_TYPE_ENGINE, ToonMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TOON, PLUGIN_TYPE_ENGINE, ToonMetaEngine);
#endif
