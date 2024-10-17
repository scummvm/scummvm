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
#include "common/fs.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"

#include "tucker/tucker.h"

class TuckerMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "tucker";
	}

	bool hasFeature(MetaEngineFeature f) const override {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsLoadingDuringStartup:
		case kSupportsDeleteSave:
		case kSavesSupportMetaInfo:
		case kSavesSupportThumbnail:
		case kSavesSupportCreationDate:
		case kSavesSupportPlayTime:
			return true;
		default:
			return false;
		}
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new Tucker::TuckerEngine(syst, desc->language, desc->flags);
		return Common::kNoError;
	}

	SaveStateList listSaves(const char *target) const override {
		Common::String pattern = Tucker::generateGameStateFileName(target, 0, true);
		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
		Tucker::TuckerEngine::SavegameHeader header;
		SaveStateList saveList;

		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			int slot;
			const char *ext = strrchr(file->c_str(), '.');
			if (ext && (slot = atoi(ext + 1)) >= 0 && slot <= Tucker::kLastSaveSlot) {
				Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);
				if (in) {
					if (Tucker::TuckerEngine::readSavegameHeader(in, header) == Tucker::TuckerEngine::kSavegameNoError) {
						saveList.push_back(SaveStateDescriptor(this, slot, header.description));
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
		return Tucker::kLastSaveSlot;
	}

	int getAutosaveSlot() const override {
		return Tucker::kAutoSaveSlot;
	}

	bool removeSaveState(const char *target, int slot) const override {
		Common::String filename = Tucker::generateGameStateFileName(target, slot);
		return g_system->getSavefileManager()->removeSavefile(filename);
	}

	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override {
		Common::String fileName = Common::String::format("%s.%d", target, slot);
		Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

		if (!file) {
			return SaveStateDescriptor();
		}

		Tucker::TuckerEngine::SavegameHeader header;
		Tucker::TuckerEngine::SavegameError savegameError = Tucker::TuckerEngine::readSavegameHeader(file, header, false);
		if (savegameError) {
			delete file;
			return SaveStateDescriptor();
		}

		SaveStateDescriptor desc(this, slot, header.description);

		if (slot == Tucker::kAutoSaveSlot) {
			bool autosaveAllowed = Tucker::TuckerEngine::isAutosaveAllowed(target);
			desc.setDeletableFlag(!autosaveAllowed);
			desc.setWriteProtectedFlag(autosaveAllowed);
		}

		if (header.version >= 2) {
			// creation/play time
			if (header.saveDate) {
				int day   = (header.saveDate >> 24) & 0xFF;
				int month = (header.saveDate >> 16) & 0xFF;
				int year  =  header.saveDate        & 0xFFFF;
				desc.setSaveDate(year, month, day);
			}

			if (header.saveTime) {
				int hour    = (header.saveTime >> 16) & 0xFF;
				int minutes = (header.saveTime >>  8) & 0xFF;
				desc.setSaveTime(hour, minutes);
			}

			if (header.playTime) {
				desc.setPlayTime(header.playTime * 1000);
			}

			// thumbnail
			if (header.thumbnail) {
				desc.setThumbnail(header.thumbnail);
			}
		}

		delete file;
		return desc;
	}

	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		return Tucker::generateGameStateFileName(target, saveGameIdx, saveGameIdx == kSavegameFilePattern);
	}
	
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::KeymapArray TuckerMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Tucker;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "tucker-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Common::Action("PAUSE", _("Pause"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_X");
	gameKeyMap->addAction(act);

	act = new Common::Action("SKIP_SPEECH", _("Skip speech"));
	act->setCustomEngineActionEvent(kActionSkipSpeech);
	act->addDefaultInputMapping("PERIOD");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	gameKeyMap->addAction(act);

	act = new Common::Action("FAST_MODE", _("Fast mode"));
	act->setCustomEngineActionEvent(kActionFastMode);
	act->addDefaultInputMapping("C+f");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	gameKeyMap->addAction(act);

	// I18N: Panel contains all actor actions and the artstyle is toggled
	act = new Common::Action("TOGGLE_PANEL_STYLE", _("Toggle panel style"));
	act->setCustomEngineActionEvent(kActionTogglePanelStyle);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Common::Action("TOGGLE_TEXT_SPEECH", _("Toggle text speech"));
	act->setCustomEngineActionEvent(kActionToggleTextSpeech);
	act->addDefaultInputMapping("F2");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Common::Action("HELP", _("Help"));
	act->setCustomEngineActionEvent(kActionHelp);
	act->addDefaultInputMapping("F3");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	act = new Common::Action("ESCAPE", _("Escape"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_BACK");
	gameKeyMap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}


#if PLUGIN_ENABLED_DYNAMIC(TUCKER)
	REGISTER_PLUGIN_DYNAMIC(TUCKER, PLUGIN_TYPE_ENGINE, TuckerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TUCKER, PLUGIN_TYPE_ENGINE, TuckerMetaEngine);
#endif
