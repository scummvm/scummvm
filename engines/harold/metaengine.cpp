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

#include "common/events.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"

#include "harold/bcb/bcb.h"
#include "harold/detection.h"

class HaroldMetaEngine : public AdvancedMetaEngine<BCB::HaroldGameDescription> {
public:
	const char *getName() const override {
		return "harold";
	}

	int getMaximumSaveSlot() const override { return 8; }
	bool hasFeature(MetaEngineFeature f) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const BCB::HaroldGameDescription *gd) const override;
};

bool HaroldMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f);
}

Common::KeymapArray HaroldMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace BCB;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "bcb-default", _("J.B. Harold: Blue Chicago Blues"));
	Action *act;

	act = new Action("INTERACT", _("Interact"));
	act->setCustomEngineActionEvent(kBCBActionInteract);
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("ESCAPE", _("Escape"));
	act->setCustomEngineActionEvent(kBCBActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("SKIP", _("Skip / Advance"));
	act->setCustomEngineActionEvent(kBCBActionSkip);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	act = new Action("MOVE_CHOICE", _("Move"));
	act->setCustomEngineActionEvent(kBCBActionChoice9);
	act->addDefaultInputMapping("9");
	act->addDefaultInputMapping("KP9");
	engineKeyMap->addAction(act);

	static const char *const choiceActionIds[] = {
		"CHOICE1", "CHOICE2", "CHOICE3", "CHOICE4",
		"CHOICE5", "CHOICE6", "CHOICE7", "CHOICE8"};
	static const char *const choiceActionLabels[] = {
		_s("Choice 1"), _s("Choice 2"), _s("Choice 3"),
		_s("Choice 4"), _s("Choice 5"), _s("Choice 6"),
		_s("Choice 7"), _s("Choice 8")};

	for (int choiceIndex = 1; choiceIndex <= 8; ++choiceIndex) {
		act = new Action(choiceActionIds[choiceIndex - 1], _(choiceActionLabels[choiceIndex - 1]));
		act->setCustomEngineActionEvent((Common::CustomEventType)(kBCBActionChoice1 + choiceIndex - 1));
		act->addDefaultInputMapping(Common::String::format("%d", choiceIndex));
		act->addDefaultInputMapping(Common::String::format("KP%d", choiceIndex));
		engineKeyMap->addAction(act);
	}

	return Keymap::arrayOf(engineKeyMap);
}

Common::Error HaroldMetaEngine::createInstance(OSystem *syst, Engine **engine, const BCB::HaroldGameDescription *gd) const {
	switch (gd->gameType) {
#ifdef ENABLE_BCB
	case BCB::kHaroldGameBCB:
		*engine = new BCB::BCBEngine(syst, gd);
		return Common::kNoError;
#endif
	default:
		return Common::kUnsupportedGameidError;
	}
}

#if PLUGIN_ENABLED_DYNAMIC(HAROLD)
REGISTER_PLUGIN_DYNAMIC(HAROLD, PLUGIN_TYPE_ENGINE, HaroldMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HAROLD, PLUGIN_TYPE_ENGINE, HaroldMetaEngine);
#endif
