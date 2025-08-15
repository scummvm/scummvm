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

#include "chamber/chamber.h"
#include "engines/advancedDetector.h"

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Chamber {

Common::Language ChamberEngine::getLanguage() const {
	return _gameDescription->language;
}

} // end of namespace Chamber

class ChamberMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "chamber";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error ChamberMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Chamber::ChamberEngine(syst, desc);
	return Common::kNoError;
}


Common::KeymapArray ChamberMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Chamber;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "chamber-default", _("Default keymappings"));
	Keymap *gameKeymap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *quitDialogKeymap = new Keymap(Keymap::kKeymapTypeGame, "quit-dialog", _("Quit dialog keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Select / Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Select / Interact"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeymap->addAction(act);

	act = new Action("INTERACT", _("Select / Interact"));
	act->setCustomEngineActionEvent(kActionInteract);
	act->addDefaultInputMapping("SPACE");
	gameKeymap->addAction(act);

	// Only the EN_USA version has a quit dialog. Input handling does a similar check.
	if (parseLanguage(ConfMan.get("language")) == EN_USA) {
		act = new Action("QUIT", _("Quit"));
		act->setCustomEngineActionEvent(kActionQuit);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_X");
		gameKeymap->addAction(act);
	}

	act = new Action("YES", _("Yes"));
	act->setCustomEngineActionEvent(kActionYes);
	act->addDefaultInputMapping("y");
	act->addDefaultInputMapping("JOY_A");
	quitDialogKeymap->addAction(act);

	act = new Action("NO", _("No"));
	act->setCustomEngineActionEvent(kActionNo);
	act->addDefaultInputMapping("n");
	act->addDefaultInputMapping("JOY_B");
	quitDialogKeymap->addAction(act);

	Common::KeymapArray keymaps(3);

	keymaps[0] = engineKeymap;
	keymaps[1] = gameKeymap;
	keymaps[2] = quitDialogKeymap;

	quitDialogKeymap->setEnabled(false);

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(CHAMBER)
REGISTER_PLUGIN_DYNAMIC(CHAMBER, PLUGIN_TYPE_ENGINE, ChamberMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CHAMBER, PLUGIN_TYPE_ENGINE, ChamberMetaEngine);
#endif
