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
#include "backends/keymapper/keymapper.h"
#include "awe/metaengine.h"
#include "awe/detection.h"
#include "awe/awe.h"

namespace Awe {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Enable title and copy protection screens (if present)"),
			_s("Displays title and copy protection screens that would otherwise be bypassed by default."),
			"copy_protection",
			false,
			0,
			0
		},
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};


struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_key1;
	const char *_key2;
	const char *_joy;
};

static const KeybindingRecord GAME_KEYS[] = {
	{ KEYBIND_UP, "UP", _s("Up"), "UP", nullptr, "JOY_UP"},
	{ KEYBIND_DOWN, "DOWN", _s("Down"), "DOWN", nullptr, "JOY_DOWN"},
	{ KEYBIND_LEFT, "LEFT", _s("Left"), "LEFT", nullptr, "JOY_LEFT"},
	{ KEYBIND_RIGHT, "RIGHT", _s("Right"), "RIGHT", nullptr, "JOY_RIGHT"},
	{ KEYBIND_SELECT, "SELECT", _s("Select/Kick/Run"), "SPACE", "RETURN", "JOY_A" },
	{ KEYBIND_JUMP, "JUMP", _s("Jump"), "LSHIFT", "RSHIFT", "JOY_B" },
	{ KEYBIND_CODE, "CODE", _s("Enter Level Code"), "c", nullptr, "JOY_X" },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

} // End of namespace Awe

const char *AweMetaEngine::getName() const {
	return "awe";
}

const ADExtraGuiOptionsMap *AweMetaEngine::getAdvancedExtraGuiOptions() const {
	return Awe::optionsList;
}

Common::Error AweMetaEngine::createInstance(OSystem *syst, Engine **engine, const Awe::AweGameDescription *desc) const {
	*engine = new Awe::AweEngine(syst, (const Awe::AweGameDescription *)desc);
	return Common::kNoError;
}

bool AweMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

Common::Array<Common::Keymap *> AweMetaEngine::initKeymaps(const char *target) const {
	Common::KeymapArray keymapArray;
	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "got", _s("Game Keys"));
	keymapArray.push_back(keyMap);

	for (const Awe::KeybindingRecord *r = Awe::GAME_KEYS; r->_id; ++r) {
		Common::Action *act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		act->allowKbdRepeats();

		act->addDefaultInputMapping(r->_key1);
		if (r->_key2)
			act->addDefaultInputMapping(r->_key2);
		if (r->_joy)
			act->addDefaultInputMapping(r->_joy);

		keyMap->addAction(act);
	}

	return keymapArray;
}

#if PLUGIN_ENABLED_DYNAMIC(AWE)
REGISTER_PLUGIN_DYNAMIC(AWE, PLUGIN_TYPE_ENGINE, AweMetaEngine);
#else
REGISTER_PLUGIN_STATIC(AWE, PLUGIN_TYPE_ENGINE, AweMetaEngine);
#endif
