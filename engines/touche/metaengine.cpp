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
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "base/plugins.h"

#include "touche/touche.h"

class ToucheMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "touche";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (!target)
			target = getName();
		return Touche::generateGameStateFileName(target, saveGameIdx, saveGameIdx == kSavegameFilePattern);
	}

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool ToucheMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Touche::ToucheEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}

Common::Error ToucheMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Touche::ToucheEngine(syst, desc->language);
	return Common::kNoError;
}

SaveStateList ToucheMetaEngine::listSaves(const char *target) const {
	Common::String pattern = Touche::generateGameStateFileName(target, 0, true);
	Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
	bool slotsTable[Touche::kMaxSaveStates];
	memset(slotsTable, 0, sizeof(slotsTable));
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		int slot = Touche::getGameStateFileSlot(file->c_str());
		if (slot >= 0 && slot < Touche::kMaxSaveStates) {
			slotsTable[slot] = true;
		}
	}
	for (int slot = 0; slot < Touche::kMaxSaveStates; ++slot) {
		if (slotsTable[slot]) {
			Common::String file = Touche::generateGameStateFileName(target, slot);
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(file);
			if (in) {
				char description[64];
				Touche::readGameStateDescription(in, description, sizeof(description) - 1);
				if (description[0]) {
					saveList.push_back(SaveStateDescriptor(this, slot, description));
				}
				delete in;
			}
		}
	}
	return saveList;
}

int ToucheMetaEngine::getMaximumSaveSlot() const {
	return Touche::kMaxSaveStates - 1;
}

bool ToucheMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Touche::generateGameStateFileName(target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

Common::KeymapArray ToucheMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Touche;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "touche-default", "Default keymappings");
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

	{
		act = new Action("SKIPORQUIT", _("Skip sequence/open quit dialogue"));
		act->setCustomEngineActionEvent(kToucheActionSkipOrQuit);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("SKIPDILOG", _("Skip Dialogue"));
		act->setCustomEngineActionEvent(kToucheActionSkipDialogue);
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_X");
		gameKeyMap->addAction(act);

		act = new Action("OPTIONS", _("Open options menu"));
		act->setCustomEngineActionEvent(kToucheActionOpenOptions);
		act->addDefaultInputMapping("F5");
		act->addDefaultInputMapping("JOY_Y");
		gameKeyMap->addAction(act);

		// I18N: The actor walking pace is increased
		act = new Action("ENABLEFASTWALK", _("Enable fast walk"));
		act->setCustomEngineActionEvent(kToucheActionEnableFastWalk);
		act->addDefaultInputMapping("F9");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

		// I18N: The actor walking pace is decreased
		act = new Action("DISABLEFASTWALK", _("Disable fast walk"));
		act->setCustomEngineActionEvent(kToucheActionDisableFastWalk);
		act->addDefaultInputMapping("F10");
		act->addDefaultInputMapping("JOY_LEFT_STICK");
		gameKeyMap->addAction(act);

		act = new Action("TGGLFASTMODE", _("Toggle fast mode"));
		act->setCustomEngineActionEvent(kToucheActionToggleFastMode);
		act->addDefaultInputMapping("C+f");
		act->addDefaultInputMapping("JOY_RIGHT_STICK");
		gameKeyMap->addAction(act);

		act = new Action("TGGLTALKTEXT", _("Toggle between voice/text/text and voice"));
		act->setCustomEngineActionEvent(kToucheActionToggleTalkTextMode);
		act->addDefaultInputMapping("t");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

		String s = ConfMan.get("language", target);
		Language l = Common::parseLanguage(s);

		act = new Action("YES", _("Press \"Yes\" Key"));
		act->setCustomEngineActionEvent(kToucheActionYes);
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		switch (l) {
		case FR_FRA:
			act->addDefaultInputMapping("o");
			break;
		case DE_DEU:
			act->addDefaultInputMapping("j");
			break;
		case ES_ESP:
			act->addDefaultInputMapping("s");
			break;
		case PL_POL:
			act->addDefaultInputMapping("s");
			act->addDefaultInputMapping("t");
			break;
		default:
			act->addDefaultInputMapping("y");
			break;
		}
		gameKeyMap->addAction(act);
	}

	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(TOUCHE)
	REGISTER_PLUGIN_DYNAMIC(TOUCHE, PLUGIN_TYPE_ENGINE, ToucheMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TOUCHE, PLUGIN_TYPE_ENGINE, ToucheMetaEngine);
#endif
