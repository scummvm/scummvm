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
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"

#include "lure/lure.h"
#include "lure/detection.h"

namespace Lure {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Enable copy protection"),
			_s("Enable any copy protection that would otherwise be bypassed by default."),
			"copy_protection",
			false,
			0,
			0
		},
	},
#ifdef USE_TTS
	{
		GAMEOPTION_TTS_NARRATOR,
		{
			_s("TTS Narrator"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_narrator",
			false,
			0,
			0
		}
	},
#endif

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

uint32 LureEngine::getFeatures() const { return _gameDescription->features; }
Common::Language LureEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform LureEngine::getPlatform() const { return _gameDescription->desc.platform; }

LureLanguage LureEngine::getLureLanguage() const {
	switch (_gameDescription->desc.language) {
	case Common::IT_ITA: return LANG_IT_ITA;
	case Common::FR_FRA: return LANG_FR_FRA;
	case Common::DE_DEU: return LANG_DE_DEU;
	case Common::ES_ESP: return LANG_ES_ESP;
	case Common::RU_RUS: return LANG_RU_RUS;
	case Common::EN_ANY:
		return isKonami() ? LANG_EN_KONAMI : LANG_EN_ANY;
	case Common::UNK_LANG: return LANG_UNKNOWN;
	default:
		error("Unknown game language");
	}
}

} // End of namespace Lure

class LureMetaEngine : public AdvancedMetaEngine<Lure::LureGameDescription> {
public:
	const char *getName() const override {
		return "lure";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Lure::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const Lure::LureGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool LureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSimpleSavesNames) ||
		(f == kSupportsDeleteSave);
}

bool Lure::LureEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error LureMetaEngine::createInstance(OSystem *syst, Engine **engine, const Lure::LureGameDescription *desc) const {
	*engine = new Lure::LureEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList LureMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = "lure.###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				saveDesc = Lure::getSaveName(in);
				saveList.push_back(SaveStateDescriptor(this, slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int LureMetaEngine::getMaximumSaveSlot() const { return 999; }

bool LureMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = target;
	filename += Common::String::format(".%03d", slot);

	return g_system->getSavefileManager()->removeSavefile(filename);
}

Common::KeymapArray LureMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Lure;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "lure-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *fightKeyMap = new Keymap(Keymap::kKeymapTypeGame, "fight-shortcut", _("Fight sequence keymappings"));
	Keymap *indexKeyMap = new Keymap(Keymap::kKeymapTypeGame, "index-shortcut", _("Index keymappings"));
	Keymap *yesNoKeyMap = new Keymap(Keymap::kKeymapTypeGame, "yesno-shortcut", _("Yes/No keymappings"));

	Common::Action *act;

	{
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
	}

	{
		act = new Common::Action("SAVEGAME", _("Save game"));
		act->setCustomEngineActionEvent(kActionSaveGame);
		act->addDefaultInputMapping("F5");
		act->addDefaultInputMapping("JOY_Y");
		gameKeyMap->addAction(act);

		act = new Common::Action("RESTOREGAME", _("Restore game"));
		act->setCustomEngineActionEvent(kActionRestoreGame);
		act->addDefaultInputMapping("F7");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Common::Action("RESTARTGAME", _("Restart game"));
		act->setCustomEngineActionEvent(kActionRestartGame);
		act->addDefaultInputMapping("F9");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Common::Action("ESC", _("Escape"));
		act->setCustomEngineActionEvent(kActionEscape);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_X");
		gameKeyMap->addAction(act);
	}

	{
		// I18N: Move actor left during fight
		act = new Common::Action("MOVELEFT", _("Move left"));
		act->setCustomEngineActionEvent(kActionFightMoveLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("JOY_LEFT");
		fightKeyMap->addAction(act);

		
		// I18N: Move actor right during fight
		act = new Common::Action("MOVERIGHT", _("Move right"));
		act->setCustomEngineActionEvent(kActionFightMoveRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("JOY_RIGHT");
		fightKeyMap->addAction(act);

		// I18N: Shift Cursor during fight to top left
		act = new Common::Action("CURSORLEFTTOP", _("Shift Cursor - Top left"));
		act->setCustomEngineActionEvent(kActionFightCursorLeftTop);
		act->addDefaultInputMapping("KP7");
		fightKeyMap->addAction(act);

		// I18N: Shift Cursor during fight to middle left
		act = new Common::Action("CURSORLEFTMIDDLE", _("Shift Cursor -  Middle left"));
		act->setCustomEngineActionEvent(kActionFightCursorLeftTop);
		act->addDefaultInputMapping("KP4");
		fightKeyMap->addAction(act);

		// I18N: Shift Cursor during fight to bottom left
		act = new Common::Action("CURSORLEFTBOTTOM", _("Shift Cursor - Bottom left"));
		act->setCustomEngineActionEvent(kActionFightCursorLeftTop);
		act->addDefaultInputMapping("KP1");
		fightKeyMap->addAction(act);

		// I18N: Shift Cursor during fight to top right
		act = new Common::Action("CURSORRIGHTTOP", _("Shift Cursor - Top right"));
		act->setCustomEngineActionEvent(kActionFightCursorLeftTop);
		act->addDefaultInputMapping("KP9");
		fightKeyMap->addAction(act);

		// I18N: Shift Cursor during fight to middle right
		act = new Common::Action("CURSORRIGHTMIDDLE", _("Shift Cursor - Middle right"));
		act->setCustomEngineActionEvent(kActionFightCursorLeftTop);
		act->addDefaultInputMapping("KP6");
		fightKeyMap->addAction(act);

		// I18N: Shift Cursor during fight to bottom right
		act = new Common::Action("CURSORRIGHTBOTTOM", _("Shift Cursor - Bottom right"));
		act->setCustomEngineActionEvent(kActionFightCursorLeftTop);
		act->addDefaultInputMapping("KP3");
		fightKeyMap->addAction(act);
	}

	{
		act = new Common::Action("INDEXPREVIOUS", _("Go to next index"));
		act->setCustomEngineActionEvent(kActionIndexNext);
		act->addDefaultInputMapping("KP2");
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("JOY_DOWN");
		indexKeyMap->addAction(act);

		act = new Common::Action("INDEXNEXT", _("Go to previous index"));
		act->setCustomEngineActionEvent(kActionIndexPrevious);
		act->addDefaultInputMapping("KP8");
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("JOY_UP");
		indexKeyMap->addAction(act);

		act = new Common::Action("INDEXSELECT", _("Select index"));
		act->setCustomEngineActionEvent(kActionIndexSelect);
		act->addDefaultInputMapping("KP3");
		act->addDefaultInputMapping("JOY_CENTER");
		indexKeyMap->addAction(act);
	}

	{
		String s = ConfMan.get("language", target);
		Language l = Common::parseLanguage(s);

		act = new Common::Action("YES", _("Press \"Yes\" key"));
		act->setCustomEngineActionEvent(kActionYes);
		act->addDefaultInputMapping("JOY_LEFT_STICK");
		if (l == Common::FR_FRA)
			act->addDefaultInputMapping("o");
		else if ((l == Common::DE_DEU) || (l == Common::NL_NLD))
			act->addDefaultInputMapping("j");
		else if ((l == Common::ES_ESP) || (l == Common::IT_ITA))
			act->addDefaultInputMapping("s");
		else if (l == Common::RU_RUS)
			act->addDefaultInputMapping("l");
		else
			act->addDefaultInputMapping("y");
		yesNoKeyMap->addAction(act);

		act = new Common::Action("NO", _("Press \"No\" key"));
		act->setCustomEngineActionEvent(kActionNo);
		act->addDefaultInputMapping("JOY_RIGHT_STICK");
		if (l == Common::RU_RUS)
			act->addDefaultInputMapping("y");
		else
			act->addDefaultInputMapping("n");
		yesNoKeyMap->addAction(act);
	}

	KeymapArray keymaps(5);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;
	keymaps[2] = fightKeyMap;
	keymaps[3] = indexKeyMap;
	keymaps[4] = yesNoKeyMap;

	yesNoKeyMap->setEnabled(false);

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(LURE)
	REGISTER_PLUGIN_DYNAMIC(LURE, PLUGIN_TYPE_ENGINE, LureMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LURE, PLUGIN_TYPE_ENGINE, LureMetaEngine);
#endif
