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

#include "sword25/sword25.h"
#include "sword25/kernel/persistenceservice.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Sword25 {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ENGLISH_SPEECH,
		{
			_s("Use English speech"),
			_s("Use English speech instead of German for every language other than German"),
			"english_speech",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

uint32 Sword25Engine::getGameFlags() const { return _gameDescription->flags; }

} // End of namespace Sword25

class Sword25MetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "sword25";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Sword25::optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override { return Sword25::PersistenceService::getSlotCount(); }
	SaveStateList listSaves(const char *target) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error Sword25MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Sword25::Sword25Engine(syst, desc);
	return Common::kNoError;
}

bool Sword25MetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSimpleSavesNames);
}

SaveStateList Sword25MetaEngine::listSaves(const char *target) const {
	Common::String pattern = target;
	pattern = pattern + ".###";
	SaveStateList saveList;

	Sword25::PersistenceService ps;
	Sword25::setGameTarget(target);

	ps.reloadSlots();

	for (uint i = 0; i < ps.getSlotCount(); ++i) {
		if (ps.isSlotOccupied(i)) {
			Common::String desc = ps.getSavegameDescription(i);
			saveList.push_back(SaveStateDescriptor(this, i, desc));
		}
	}

	return saveList;
}

Common::KeymapArray Sword25MetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Sword25;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "sword25-default", _("Default keymappings"));
	Keymap *debugKeyMap = new Keymap(Keymap::kKeymapTypeGame, "debug", _("Debug keymappings"));

	Common::Action *act;

	act = new Action(kStandardActionLeftClick, _("Move / Interact / Skip dialog"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("SKIPDLG", _("Examine / Skip dialog"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionPause, _("Pause"));
	act->setKeyEvent(KeyState(KEYCODE_p, 'p'));
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Action("REVEALHOTSPOTS", _("Reveal all interactive hotspots (hold)"));
	act->setKeyEvent(KeyState(KEYCODE_SPACE, ASCII_SPACE));
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeyMap->addAction(act);

	if (debugChannelSet(-1, kDebugInternalDebugger)) {

		act = new Action("QUIT", _("Quit game"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		debugKeyMap->addAction(act);

		act = new Action("DEBUGCONSOLE", _("Toggle debug console"));
		act->setKeyEvent(KeyState(KEYCODE_F1, KEYCODE_F1));
		act->addDefaultInputMapping("F1");
		debugKeyMap->addAction(act);

		act = new Action("SHOWINFO", _("Toggle info text (FPS, memory usage, coordinates, etc.)"));
		act->setKeyEvent(KeyState(KEYCODE_F2, KEYCODE_F2));
		act->addDefaultInputMapping("F2");
		debugKeyMap->addAction(act);

		act = new Action("TOGGLE_VSYNC", _("Toggle VSync mode"));
		act->setKeyEvent(KeyState(KEYCODE_F3, KEYCODE_F3));
		act->addDefaultInputMapping("F3");
		debugKeyMap->addAction(act);

		act = new Action("SHOW_WALK_REGIONS", _("Toggle walk regions"));
		act->setKeyEvent(KeyState(KEYCODE_F4, KEYCODE_F4));
		act->addDefaultInputMapping("F4");
		debugKeyMap->addAction(act);

		act = new Action("SHOW_ITEM_REGIONS", _("Toggle item regions"));
		act->setKeyEvent(KeyState(KEYCODE_F5, KEYCODE_F5));
		act->addDefaultInputMapping("F5");
		debugKeyMap->addAction(act);

		act = new Action("TAKE_SCREENSHOT", _("Take and save screenshot"));
		act->setKeyEvent(KeyState(KEYCODE_F6, KEYCODE_F6));
		act->addDefaultInputMapping("F6");
		debugKeyMap->addAction(act);

	}
	
	KeymapArray keymaps(2);
	keymaps[0] = engineKeyMap;
	keymaps[1] = debugKeyMap;

	return keymaps;

}

#if PLUGIN_ENABLED_DYNAMIC(SWORD25)
	REGISTER_PLUGIN_DYNAMIC(SWORD25, PLUGIN_TYPE_ENGINE, Sword25MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SWORD25, PLUGIN_TYPE_ENGINE, Sword25MetaEngine);
#endif
