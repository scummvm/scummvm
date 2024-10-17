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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "engines/advancedDetector.h"

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "graphics/surface.h"

#include "cge2/cge2.h"
#include "cge2/detection.h"

namespace CGE2 {

static const ADExtraGuiOptionsMap optionsList[] = {
		{
			GAMEOPTION_COLOR_BLIND_DEFAULT_OFF,
			{
				_s("Color Blind Mode"),
				_s("Enable Color Blind Mode by default"),
				"enable_color_blind",
				false,
				0,
				0
			}
		},

#ifdef USE_TTS
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
#endif

		AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CGE2MetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "cge2";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error CGE2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new CGE2::CGE2Engine(syst, desc);
	return Common::kNoError;
}

bool CGE2MetaEngine::hasFeature(MetaEngineFeature f) const {
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

int CGE2MetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList CGE2MetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {

			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				CGE2::SavegameHeader header;

				// Check to see if it's a ScummVM savegame or not
				char buffer[kSavegameStrSize + 1];
				file->read(buffer, kSavegameStrSize + 1);

				if (!strncmp(buffer, kSavegameStr, kSavegameStrSize + 1)) {
					// Valid savegame
					if (CGE2::CGE2Engine::readSavegameHeader(file, header)) {
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

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor CGE2MetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		CGE2::SavegameHeader header;

		// Check to see if it's a ScummVM savegame or not
		char buffer[kSavegameStrSize + 1];
		f->read(buffer, kSavegameStrSize + 1);

		bool hasHeader = !strncmp(buffer, kSavegameStr, kSavegameStrSize + 1) &&
			CGE2::CGE2Engine::readSavegameHeader(f, header, false);
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

			if (header.playTime) {
				desc.setPlayTime(header.playTime * 1000);
			}

			return desc;
		}
	}

	return SaveStateDescriptor();
}

bool CGE2MetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::KeymapArray CGE2MetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "Sfinx", _("Game Keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keymap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keymap->addAction(act);

	// I18N: This closes the Dialogue/text box.
	act = new Common::Action("CLOSEBOX", _("Close the Dialogue box"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	keymap->addAction(act);

	act = new Common::Action(kStandardActionSave, _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	keymap->addAction(act);

	act = new Common::Action(kStandardActionLoad, _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F7");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	keymap->addAction(act);

	// I18N: 3-4 dialogs of game version info, (translation) credits, etc.
	act = new Common::Action("INFO", _("Game Info"));
	act->setCustomEngineActionEvent(kActionInfo);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	keymap->addAction(act);

	// I18N: This opens a Quit Prompt where you have to choose
	// [Confirm] or [Continue Playing] lines with Left Click.
	act = new Common::Action("QUIT", _("Quit Prompt"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("A+x");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	keymap->addAction(act);

	return Keymap::arrayOf(keymap);
}
} // End of namespace CGE2

#if PLUGIN_ENABLED_DYNAMIC(CGE2)
	REGISTER_PLUGIN_DYNAMIC(CGE2, PLUGIN_TYPE_ENGINE, CGE2::CGE2MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CGE2, PLUGIN_TYPE_ENGINE, CGE2::CGE2MetaEngine);
#endif
