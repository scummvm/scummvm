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
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "bagel/metaengine.h"
#include "bagel/detection.h"
#include "bagel/spacebar/spacebar.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_key1;
	const char *_key2;
	const char *_joy;
};

namespace SpaceBar {

static const KeybindingRecord MINIMAL_KEYS[] = {
	{ KEYBIND_WAIT, "WAIT", _s("Wait"), "SPACE", nullptr, nullptr },
	{ KEYBIND_CHEAT714, "CHEAT714", _s("Soldier 714 Eye Cheat"), "SCROLLOCK", nullptr, nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

} // namespace SpaceBar

namespace HodjNPodj {

static const KeybindingRecord MINIMAL_KEYS[] = {
#ifdef TODO
	{ KEYBIND_SELECT, "SELECT", _s("Select"), "SPACE", "RETURN", "JOY_A" },
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Escape"), "ESCAPE", nullptr, "JOY_B" },
	{ KEYBIND_UP, "UP", _s("Up"), "UP", nullptr, "JOY_UP"},
	{ KEYBIND_DOWN, "DOWN", _s("Down"), "DOWN", nullptr, "JOY_DOWN"},
	{ KEYBIND_LEFT, "LEFT", _s("Left"), "LEFT", nullptr, "JOY_LEFT"},
	{ KEYBIND_RIGHT, "RIGHT", _s("Right"), "RIGHT", nullptr, "JOY_RIGHT"},
	{ KEYBIND_PAGEUP, "PAGEUP", _s("Page Up"), "PAGEUP", "KP9", "JOY_RIGHT"},
	{ KEYBIND_PAGEDOWN, "PAGEDOWN", _s("Page Down"), "PAGEDOWN", "KP3", "JOY_RIGHT"},
	{ KEYBIND_HOME, "HOME", _s("Home"), "HOME", "KP7", "JOY_RIGHT"},
	{ KEYBIND_END, "END", _s("End"), "END", "KP2", "JOY_RIGHT"},
#endif
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

} // namespace HodjNPodj

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // namespace Bagel

const char *BagelMetaEngine::getName() const {
	return "bagel";
}

const ADExtraGuiOptionsMap *BagelMetaEngine::getAdvancedExtraGuiOptions() const {
	return Bagel::optionsList;
}

Common::Error BagelMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (!strcmp(desc->gameId, "spacebar"))
		*engine = new Bagel::SpaceBar::SpaceBarEngine(syst, desc);
	else
		*engine = new Bagel::HodjNPodj::HodjNPodjEngine(syst, desc);

	return Common::kNoError;
}

bool BagelMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
	       (f == kSupportsLoadingDuringStartup);
}

Common::KeymapArray BagelMetaEngine::initKeymaps(const char *target) const {
	Common::KeymapArray keymapArray;

	// The current keymaps are only applicable for Space Bar
	const Common::ConfigManager::Domain *domain = ConfMan.getDomain(target);
	Common::String gameId = domain->getVal("gameid");

	const Bagel::KeybindingRecord *keys = (gameId == "spacebar") ?
		Bagel::SpaceBar::MINIMAL_KEYS :
		Bagel::HodjNPodj::MINIMAL_KEYS;

	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "bagel", _s("General Keys"));
	keymapArray.push_back(keyMap);

	for (const Bagel::KeybindingRecord *r = keys; r->_id; ++r) {
		Common::Action *act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		act->addDefaultInputMapping(r->_key1);
		if (r->_key2)
			act->addDefaultInputMapping(r->_key2);
		if (r->_joy)
			act->addDefaultInputMapping(r->_joy);

		if (r->_action == Bagel::KEYBIND_UP || r->_action == Bagel::KEYBIND_DOWN ||
				r->_action == Bagel::KEYBIND_LEFT || r->_action == Bagel::KEYBIND_RIGHT)
			act->allowKbdRepeats();

		keyMap->addAction(act);
	}

	return keymapArray;
}

#if PLUGIN_ENABLED_DYNAMIC(BAGEL)
REGISTER_PLUGIN_DYNAMIC(BAGEL, PLUGIN_TYPE_ENGINE, BagelMetaEngine);
#else
REGISTER_PLUGIN_STATIC(BAGEL, PLUGIN_TYPE_ENGINE, BagelMetaEngine);
#endif
