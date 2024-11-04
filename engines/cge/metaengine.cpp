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

#include "graphics/thumbnail.h"

#include "engines/advancedDetector.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "cge/cge.h"
#include "cge/detection.h"

namespace CGE {

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
		GAMEOPTION_TTS,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read text in the game (if TTS is available)"),
			"tts_enabled",
			false,
			0,
			0
		}
	},
#endif

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CGEMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "cge";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool CGEMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool CGEMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

int CGEMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList CGEMetaEngine::listSaves(const char *target) const {
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
				CGE::SavegameHeader header;

				// Check to see if it's a ScummVM savegame or not
				char buffer[kSavegameStrSize + 1];
				file->read(buffer, kSavegameStrSize + 1);

				if (!strncmp(buffer, CGE::savegameStr, kSavegameStrSize + 1)) {
					// Valid savegame
					if (CGE::CGEEngine::readSavegameHeader(file, header)) {
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

SaveStateDescriptor CGEMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(fileName);

	if (f) {
		CGE::SavegameHeader header;

		// Check to see if it's a ScummVM savegame or not
		char buffer[kSavegameStrSize + 1];
		f->read(buffer, kSavegameStrSize + 1);

		bool hasHeader = !strncmp(buffer, CGE::savegameStr, kSavegameStrSize + 1) &&
			CGE::CGEEngine::readSavegameHeader(f, header, false);
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

Common::Error CGEMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new CGE::CGEEngine(syst, desc);
	return Common::kNoError;
}

Common::KeymapArray CGEMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "Soltys", _("Game Keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keymap->addAction(act);

	act = new Action(kStandardActionSkip, _("Exit/Skip"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	keymap->addAction(act);

	act = new Action(kStandardActionSave, _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	keymap->addAction(act);

	act = new Action(kStandardActionLoad, _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F7");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	keymap->addAction(act);

	// I18N: 3-4 dialogs of game version info, (translation) credits, etc.
	act = new Action("INFO", _("Game Info"));
	act->setCustomEngineActionEvent(kActionInfo);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	keymap->addAction(act);

	// I18N: This opens a Quit Prompt where you have to choose
	// [Confirm] or [Continue Playing] lines with Left Click.
	act = new Action("QUIT", _("Quit Prompt"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("A+x");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	keymap->addAction(act);

	// I18N: Here ALTered Item refers to the dice that has been altered/changed.
	// In order to escape the dice game loop press Right/Left Alt
	act = new Action("ALTITEM", _("ALTered Item"));
	act->setCustomEngineActionEvent(kActionAltDice);
	act->addDefaultInputMapping("LALT");
	act->addDefaultInputMapping("RALT");
	act->addDefaultInputMapping("JOY_Y");
	keymap->addAction(act);

	act = new Action("INV1", _("Inventory Item 1 (Select/Deselect)"));
	act->setCustomEngineActionEvent(kActionInv1);
	act->addDefaultInputMapping("1");
	keymap->addAction(act);

	act = new Action("INV2", _("Inventory Item 2 (Select/Deselect)"));
	act->setCustomEngineActionEvent(kActionInv2);
	act->addDefaultInputMapping("2");
	keymap->addAction(act);

	act = new Action("INV3", _("Inventory Item 3 (Select/Deselect)"));
	act->setCustomEngineActionEvent(kActionInv3);
	act->addDefaultInputMapping("3");
	keymap->addAction(act);

	act = new Action("INV4", _("Inventory Item 4 (Select/Deselect)"));
	act->setCustomEngineActionEvent(kActionInv4);
	act->addDefaultInputMapping("4");
	keymap->addAction(act);

	act = new Action("INV5", _("Inventory Item 5 (Select/Deselect)"));
	act->setCustomEngineActionEvent(kActionInv5);
	act->addDefaultInputMapping("5");
	keymap->addAction(act);

	act = new Action("INV6", _("Inventory Item 6 (Select/Deselect)"));
	act->setCustomEngineActionEvent(kActionInv6);
	act->addDefaultInputMapping("6");
	keymap->addAction(act);

	act = new Action("INV7", _("Inventory Item 7 (Select/Deselect)"));
	act->setCustomEngineActionEvent(kActionInv7);
	act->addDefaultInputMapping("7");
	keymap->addAction(act);

	act = new Action("INV8", _("Inventory Item 8 (Select/Deselect)"));
	act->setCustomEngineActionEvent(kActionInv8);
	act->addDefaultInputMapping("8");
	keymap->addAction(act);

	// I18N: There are 24 single-room/screen locations in the game.
	// You switch between them from numbered buttons on interface.
	// Sets the current access to only the first Location
	act = new Action("DEBUGLOC1", _("DEBUG: Access to Location 1"));
	act->setCustomEngineActionEvent(kActionLevel0);
	act->addDefaultInputMapping("A+0");
	keymap->addAction(act);

	// I18N: Sets the current access to Locations 1 to 8.
	act = new Action("DEBUGLOC18", _("DEBUG: Access to Locations 1-8"));
	act->setCustomEngineActionEvent(kActionLevel1);
	act->addDefaultInputMapping("A+1");
	keymap->addAction(act);

	// I18N: Sets the current access to Locations 1 to 16.
	act = new Action("DEBUGLOC116", _("DEBUG: Access to Locations 1-16"));
	act->setCustomEngineActionEvent(kActionLevel2);
	act->addDefaultInputMapping("A+2");
	keymap->addAction(act);

	// I18N: Sets the current access to Locations 1 to 23.
	act = new Action("DEBUGLOC123", _("DEBUG: Access to Locations 1-23"));
	act->setCustomEngineActionEvent(kActionLevel3);
	act->addDefaultInputMapping("A+3");
	keymap->addAction(act);

	// I18N: Sets the current access to Locations 1 to 24.
	act = new Action("DEBUGLOC124", _("DEBUG: Access to Locations 1-24"));
	act->setCustomEngineActionEvent(kActionLevel4);
	act->addDefaultInputMapping("A+4");
	keymap->addAction(act);

	return Keymap::arrayOf(keymap);
}

} // End of namespace CGE

#if PLUGIN_ENABLED_DYNAMIC(CGE)
REGISTER_PLUGIN_DYNAMIC(CGE, PLUGIN_TYPE_ENGINE, CGE::CGEMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CGE, PLUGIN_TYPE_ENGINE, CGE::CGEMetaEngine);
#endif
