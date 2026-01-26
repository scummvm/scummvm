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

#include "base/plugins.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "engines/advancedDetector.h"

#include "supernova/supernova.h"
#include "supernova/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_IMPROVED,
		{
			_s("Improved mode"),
			_s("Removes some repetitive actions, adds possibility to change verbs by keyboard"),
			"improved",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_TTS,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_enabled",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class SupernovaMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "supernova";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	bool removeSaveState(const char *target, int slot) const override;
	int getMaximumSaveSlot() const override {
		return 99;
	}
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		const char *prefix = target;
		if (!strncmp(target, "msn1", 4))
			prefix = "msn_save";
		if (!strncmp(target, "msn2", 4))
			prefix = "ms2_save";
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("%s.###", prefix);
		else
			return Common::String::format("%s.%03d", prefix, saveGameIdx);
	}

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool SupernovaMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch (f) {
	case kSupportsLoadingDuringStartup:
	case kSupportsListSaves:
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

Common::Error SupernovaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Supernova::SupernovaEngine(syst);
	return Common::kNoError;
}

SaveStateList SupernovaMetaEngine::listSaves(const char *target) const {
	Common::StringArray filenames;
	const Common::String pattern = getSavegameFilePattern(target);

	filenames = g_system->getSavefileManager()->listSavefiles(pattern);

	SaveStateList saveFileList;
	for (Common::StringArray::const_iterator file = filenames.begin();
		 file != filenames.end(); ++file) {
		int saveSlot = atoi(file->c_str() + file->size() - 3);
		if (saveSlot >= 0 && saveSlot <= getMaximumSaveSlot()) {
			Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(*file);
			if (savefile) {
				uint saveHeader = savefile->readUint32LE();
				if ((saveHeader == SAVEGAME_HEADER && !strncmp(target, "msn1", 4)) ||
					(saveHeader == SAVEGAME_HEADER2 && !strncmp(target, "msn2", 4))) {
					byte saveVersion = savefile->readByte();
					if (saveVersion <= SAVEGAME_VERSION) {
						int saveFileDescSize = savefile->readSint16LE();
						char* saveFileDesc = new char[saveFileDescSize];
						savefile->read(saveFileDesc, saveFileDescSize);
						saveFileList.push_back(SaveStateDescriptor(this, saveSlot, saveFileDesc));
						delete [] saveFileDesc;
					}
				}
				delete savefile;
			}
		}
	}

	Common::sort(saveFileList.begin(), saveFileList.end(), SaveStateDescriptorSlotComparator());
	return saveFileList;
}

bool SupernovaMetaEngine::removeSaveState(const char *target, int slot) const {
	return g_system->getSavefileManager()->removeSavefile(getSavegameFile(slot, target));
}

SaveStateDescriptor SupernovaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::InSaveFile *savefile = g_system->getSavefileManager()->openForLoading(getSavegameFile(slot, target));

	if (savefile) {
		uint saveHeader = savefile->readUint32LE();
		if ((!strncmp(target, "msn1", 4) && saveHeader != SAVEGAME_HEADER) ||
			(!strncmp(target, "msn2", 4) && saveHeader != SAVEGAME_HEADER2)) {
			delete savefile;
			return SaveStateDescriptor();
		}
		byte saveVersion = savefile->readByte();
		if (saveVersion > SAVEGAME_VERSION){
			delete savefile;
			return SaveStateDescriptor();
		}

		int descriptionSize = savefile->readSint16LE();
		char* description = new char[descriptionSize];
		savefile->read(description, descriptionSize);
		SaveStateDescriptor desc(this, slot, description);
		delete [] description;

		uint32 saveDate = savefile->readUint32LE();
		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;
		desc.setSaveDate(year, month, day);

		uint16 saveTime = savefile->readUint16LE();
		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;
		desc.setSaveTime(hour, minutes);

		uint32 playTime =savefile->readUint32LE();
		desc.setPlayTime(playTime * 1000);

		if (Graphics::checkThumbnailHeader(*savefile)) {
			Graphics::Surface *thumbnail;
			if (!Graphics::loadThumbnail(*savefile, thumbnail)) {
				delete savefile;
				return SaveStateDescriptor();
			}
			desc.setThumbnail(thumbnail);
		}

		delete savefile;

		return desc;
	}

	return SaveStateDescriptor();
}

Common::KeymapArray SupernovaMetaEngine::initKeymaps(const char *target) const {

	using namespace Common;
	using namespace Supernova;

	Common::String gameId = ConfMan.get("gameid", target);

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "supernova-default", _("Default keymappings"));
	Keymap *menuKeymap = new Keymap(Keymap::kKeymapTypeGame, "menu", _("Menu keymappings"));
	Keymap *cutsceneKeymap = new Keymap(Keymap::kKeymapTypeGame, "cutscene", _("Cutscene keymappings"));
	Keymap *improvedKeymap = new Keymap(Keymap::kKeymapTypeGame, "improved-mode", _("Improved mode keymappings"));
	Keymap *textReaderKeymap = new Keymap(Keymap::kKeymapTypeGame, "text-reader", _("Text reader keymappings"));
	Keymap *computerKeymap = new Keymap(Keymap::kKeymapTypeGame, "computer", _("Computer keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Perform default action"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeymap->addAction(act);

	act = new Common::Action("HELP", _("Help"));
	act->setCustomEngineActionEvent(kActionHelp);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeymap->addAction(act);

	act = new Common::Action("INSTRUCTIONS", _("Instructions"));
	act->setCustomEngineActionEvent(kActionInstr);
	act->addDefaultInputMapping("F2");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeymap->addAction(act);

	act = new Common::Action("INFORMATION", _("Information"));
	act->setCustomEngineActionEvent(kActionInfo);
	act->addDefaultInputMapping("F3");
	act->addDefaultInputMapping("JOY_UP");
	engineKeymap->addAction(act);

	act = new Common::Action("TEXTSPEED", _("Text speed"));
	act->setCustomEngineActionEvent(kActionSpeed);
	act->addDefaultInputMapping("F4");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeymap->addAction(act);

	act = new Common::Action("PAUSE", _("Pause"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("F5");
	engineKeymap->addAction(act);

	act = new Common::Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("A+x");
	act->addDefaultInputMapping("JOY_Y");
	engineKeymap->addAction(act);

	act = new Common::Action("UP", _("Up"));
	act->setCustomEngineActionEvent(kActionUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	textReaderKeymap->addAction(act);

	act = new Common::Action("DOWN", _("Down"));
	act->setCustomEngineActionEvent(kActionDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	textReaderKeymap->addAction(act);

	act = new Common::Action("PGUP", _("Previous page"));
	act->setCustomEngineActionEvent(kActionPgUp);
	act->addDefaultInputMapping("PAGEUP");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	textReaderKeymap->addAction(act);

	act = new Common::Action("PGDOWN", _("Next page"));
	act->setCustomEngineActionEvent(kActionPgDown);
	act->addDefaultInputMapping("PAGEDOWN");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	textReaderKeymap->addAction(act);

	act = new Common::Action("EXIT", _("Exit"));
	act->setCustomEngineActionEvent(kActionExit);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	textReaderKeymap->addAction(act);

	act = new Common::Action("SELECT", _("Select"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	menuKeymap->addAction(act);

	act = new Common::Action("SKIP", _("Skip cutscene"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	cutsceneKeymap->addAction(act);

	act = new Common::Action("SKPL", _("Skip line"));
	act->setCustomEngineActionEvent(kActionSkipLine);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_A");
	cutsceneKeymap->addAction(act);

	act = new Common::Action("GO", _("Go"));
	act->setCustomEngineActionEvent(kActionGo);
	act->addDefaultInputMapping("1");
	improvedKeymap->addAction(act);

	act = new Common::Action("LOOK", _("Look"));
	act->setCustomEngineActionEvent(kActionLook);
	act->addDefaultInputMapping("2");
	improvedKeymap->addAction(act);

	act = new Common::Action("TAKE", _("Take"));
	act->setCustomEngineActionEvent(kActionTake);
	act->addDefaultInputMapping("3");
	improvedKeymap->addAction(act);

	act = new Common::Action("OPEN", _("Open"));
	act->setCustomEngineActionEvent(kActionOpen);
	act->addDefaultInputMapping("4");
	improvedKeymap->addAction(act);

	act = new Common::Action("CLOSE", _("Close"));
	act->setCustomEngineActionEvent(kActionClose);
	act->addDefaultInputMapping("5");
	improvedKeymap->addAction(act);

	act = new Common::Action("PUSH", _("Push"));
	act->setCustomEngineActionEvent(kActionPush);
	act->addDefaultInputMapping("6");
	improvedKeymap->addAction(act);

	act = new Common::Action("PULL", _("Pull"));
	act->setCustomEngineActionEvent(kActionPull);
	act->addDefaultInputMapping("7");
	improvedKeymap->addAction(act);

	act = new Common::Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping("8");
	improvedKeymap->addAction(act);

	act = new Common::Action("TALK", _("Talk"));
	act->setCustomEngineActionEvent(kActionTalk);
	act->addDefaultInputMapping("9");
	improvedKeymap->addAction(act);

	act = new Common::Action("GIVE", _("Give"));
	act->setCustomEngineActionEvent(kActionGive);
	act->addDefaultInputMapping("0");
	improvedKeymap->addAction(act);

	if (gameId == "msn1") {

		act = new Common::Action("EXIT", _("Exit"));
		act->setCustomEngineActionEvent(kActionExit);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_B");
		computerKeymap->addAction(act);

		act = new Common::Action("OFFICE", _("Office manager"));
		act->setCustomEngineActionEvent(kActionOfficeManager);
		act->addDefaultInputMapping("1");
		act->addDefaultInputMapping("JOY_LEFT");
		computerKeymap->addAction(act);

		act = new Common::Action("PHONE", _("Phone"));
		act->setCustomEngineActionEvent(kActionPhone);
		act->addDefaultInputMapping("2");
		act->addDefaultInputMapping("JOY_UP");
		computerKeymap->addAction(act);

		act = new Common::Action("PROTEXT", _("ProText"));
		act->setCustomEngineActionEvent(kActionProText);
		act->addDefaultInputMapping("3");
		act->addDefaultInputMapping("JOY_RIGHT");
		computerKeymap->addAction(act);

		act = new Common::Action("CALCULATA", _("Calculata"));
		act->setCustomEngineActionEvent(kActionCalculata);
		act->addDefaultInputMapping("4");
		act->addDefaultInputMapping("JOY_DOWN");
		computerKeymap->addAction(act);

	}

	Common::KeymapArray keymaps(6);
	keymaps[0] = engineKeymap;
	keymaps[1] = menuKeymap;
	keymaps[2] = cutsceneKeymap;
	keymaps[3] = improvedKeymap;
	keymaps[4] = textReaderKeymap;
	keymaps[5] = computerKeymap;

	menuKeymap->setEnabled(false);
	cutsceneKeymap->setEnabled(false);
	textReaderKeymap->setEnabled(false);
	computerKeymap->setEnabled(false);

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(SUPERNOVA)
	REGISTER_PLUGIN_DYNAMIC(SUPERNOVA, PLUGIN_TYPE_ENGINE, SupernovaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SUPERNOVA, PLUGIN_TYPE_ENGINE, SupernovaMetaEngine);
#endif
