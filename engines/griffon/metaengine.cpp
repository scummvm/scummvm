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
#include "common/config-manager.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "griffon/griffon.h"

class GriffonMetaEngine: public AdvancedMetaEngine {
public:
    const char *getName() const override {
		return "griffon";
	}

    int getMaximumSaveSlot() const override {
		return ConfMan.getInt("autosave_period") ? 4 : 3;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	virtual int getAutosaveSlot() const override {
		return 4;
	}
	Common::String getSavegameFile(int saveGameIdx, const char *target = nullptr) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;

};

Common::String GriffonMetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	if (saveGameIdx == kSavegameFilePattern) {
		// Pattern requested
		return Common::String::format("%s.s##", target == nullptr ? getEngineId() : target);
	} else {
		// Specific filename requested
		return Common::String::format("%s.s%02d", target == nullptr ? getEngineId() : target, saveGameIdx);
	}
}

bool Griffon::GriffonEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error GriffonMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Griffon::GriffonEngine(syst);
	return Common::kNoError;
}

Common::KeymapArray GriffonMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "griffon", "The Griffon Legend");

	Action *act;

	act = new Action(kStandardActionSkip, _("Menu / Skip"));
	act->setCustomEngineActionEvent(Griffon::kGriffonMenu);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action("RETURN", _("Confirm"));
	act->setCustomEngineActionEvent(Griffon::kGriffonConfirm);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setCustomEngineActionEvent(Griffon::kGriffonUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setCustomEngineActionEvent(Griffon::kGriffonDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setCustomEngineActionEvent(Griffon::kGriffonLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setCustomEngineActionEvent(Griffon::kGriffonRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeyMap->addAction(act);

	act = new Action("ATTACK", _("Attack"));
	act->setCustomEngineActionEvent(Griffon::kGriffonAttack);
	act->addDefaultInputMapping("LCTRL");
	act->addDefaultInputMapping("RCTRL");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(Griffon::kGriffonInventory);
	act->addDefaultInputMapping("LALT");
	act->addDefaultInputMapping("RALT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("SPEEDUP", _("Speed Up Cutscene"));
	act->setCustomEngineActionEvent(Griffon::kGriffonCutsceneSpeedUp);
	act->addDefaultInputMapping("LSHIFT");
	act->addDefaultInputMapping("RSHIFT");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(GRIFFON)
	REGISTER_PLUGIN_DYNAMIC(GRIFFON, PLUGIN_TYPE_ENGINE, GriffonMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GRIFFON, PLUGIN_TYPE_ENGINE, GriffonMetaEngine);
#endif
