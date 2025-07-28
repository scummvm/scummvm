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

#include "common/translation.h"

#include "engines/advancedDetector.h"
#include "prince/prince.h"
#include "prince/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Prince {

#ifdef USE_TTS

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_TTS_OBJECTS,
		{
			_s("Enable Text to Speech for Objects and Options"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_enabled_objects",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_TTS_SPEECH,
		{
			_s("Enable Text to Speech for Subtitles"),
			_s("Use TTS to read the subtitles (if TTS is available)"),
			"tts_enabled_speech",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_TTS_MISSING_VOICE,
		{
			_s("Enable Text to Speech for Missing Voiceovers"),
			_s("Use TTS to read the subtitles of missing voiceovers (if TTS is available)"),
			"tts_enabled_missing_voice",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

#endif

int PrinceEngine::getGameType() const {
	return _gameDescription->gameType;
}

const char *PrinceEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

uint32 PrinceEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language PrinceEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

} // End of namespace Prince

class PrinceMetaEngine : public AdvancedMetaEngine<Prince::PrinceGameDescription> {
public:
	const char *getName() const override {
		return "prince";
	}

#ifdef USE_TTS
	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Prince::optionsList;
	}
#endif

	Common::Error createInstance(OSystem *syst, Engine **engine, const Prince::PrinceGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override { return 99; }
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool PrinceMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSimpleSavesNames);
}

bool Prince::PrinceEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsReturnToLauncher);
}

SaveStateList PrinceMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (const auto &filename : filenames) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(filename.c_str() + filename.size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {

			Common::InSaveFile *file = saveFileMan->openForLoading(filename);
			if (file) {
				Prince::SavegameHeader header;

				// Check to see if it's a ScummVM savegame or not
				char buffer[kSavegameStrSize + 1];
				file->read(buffer, kSavegameStrSize + 1);

				if (!strncmp(buffer, kSavegameStr, kSavegameStrSize + 1)) {
					// Valid savegame
					if (Prince::PrinceEngine::readSavegameHeader(file, header)) {
						saveList.push_back(SaveStateDescriptor(this, slotNum, header.saveName));
					}
				} else {
					// Must be an original format savegame
					saveList.push_back(SaveStateDescriptor(this, slotNum, "Unknown"));
				}

				delete file;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor PrinceMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		Prince::SavegameHeader header;

		// Check to see if it's a ScummVM savegame or not
		char buffer[kSavegameStrSize + 1];
		f->read(buffer, kSavegameStrSize + 1);

		bool hasHeader = !strncmp(buffer, kSavegameStr, kSavegameStrSize + 1) &&
			Prince::PrinceEngine::readSavegameHeader(f, header, false);
		delete f;

		if (!hasHeader) {
			// Original savegame perhaps?
			SaveStateDescriptor desc(this, slot, "Unknown");
			return desc;
		} else {
			// Create the return descriptor
			SaveStateDescriptor desc(this, slot, header.saveName);
			desc.setThumbnail(header.thumbnail);
			desc.setSaveDate(header.saveYear, header.saveMonth, header.saveDay);
			desc.setSaveTime(header.saveHour, header.saveMinutes);
			desc.setPlayTime(header.playTime * 1000);

			return desc;
		}
	}

	return SaveStateDescriptor();
}

bool PrinceMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::Error PrinceMetaEngine::createInstance(OSystem *syst, Engine **engine, const Prince::PrinceGameDescription *desc) const {
	*engine = new Prince::PrinceEngine(syst,desc);
	return Common::kNoError;
}

Common::KeymapArray PrinceMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Prince;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "prince-default", _("Default keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Move / Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Open interaction menu"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Common::Action("SAVE", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F2");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	// I18N: This refers to the Z key on a keyboard.
	act = new Action("Z", _("Z key")); // TODO: Rename this action to better reflect its use in the prison cell minigame near the end of the game.
	act->setCustomEngineActionEvent(kActionZ);
	act->addDefaultInputMapping("z");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	// I18N: This refers to the X key on a keyboard.
	act = new Action("X", _("X key")); // TODO: Rename this action to better reflect its use in the prison cell minigame near the end of the game.
	act->setCustomEngineActionEvent(kActionX);
	act->addDefaultInputMapping("x");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("SKIP", _("Skip"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(PRINCE)
	REGISTER_PLUGIN_DYNAMIC(PRINCE, PLUGIN_TYPE_ENGINE, PrinceMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PRINCE, PLUGIN_TYPE_ENGINE, PrinceMetaEngine);
#endif
