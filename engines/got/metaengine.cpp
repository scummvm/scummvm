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
#include "got/metaengine.h"
#include "got/detection.h"
#include "got/got.h"

namespace Got {

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

struct KeybindingRecord {
    KeybindingAction _action;
    const char *_id;
    const char *_desc;
    const char *_key;
    const char *_joy;
};

static const KeybindingRecord GAME_KEYS[] = {
    { KEYBIND_UP, "UP", _s("Up"), "UP", "JOY_UP"},
    { KEYBIND_DOWN, "DOWN", _s("Down"), "DOWN", "JOY_DOWN"},
    { KEYBIND_LEFT, "LEFT", _s("Left"), "LEFT", "JOY_LEFT"},
    { KEYBIND_RIGHT, "RIGHT", _s("Right"), "RIGHT", "JOY_RIGHT"},
    { KEYBIND_FIRE, "FIRE", _s("Fire"), "LALT", "JOY_A" },
    { KEYBIND_MAGIC, "MAGIC", _s("Magic"), "LCTRL", "JOY_B" },
    { KEYBIND_SELECT, "SELECT", _s("Select"), "SPACE", "JOY_X" },
    // I18N: ESC key
    { KEYBIND_ESCAPE, "ESCAPE", _s("Escape"), "ESCAPE", "JOY_Y" },
    { KEYBIND_THOR_DIES, "THOR_DIES", _s("Thor Dies"), "d", nullptr },
    { KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

} // End of namespace Got

const char *GotMetaEngine::getName() const {
    return "got";
}

const ADExtraGuiOptionsMap *GotMetaEngine::getAdvancedExtraGuiOptions() const {
    return Got::optionsList;
}

Common::Error GotMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
    *engine = new Got::GotEngine(syst, desc);
    return Common::kNoError;
}

bool GotMetaEngine::hasFeature(MetaEngineFeature f) const {
    return checkExtendedSaves(f) ||
           (f == kSupportsLoadingDuringStartup);
}

Common::Array<Common::Keymap *> GotMetaEngine::initKeymaps(const char *target) const {
    Common::KeymapArray keymapArray;
    Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "got", _s("Game Keys"));
    keymapArray.push_back(keyMap);

    for (const Got::KeybindingRecord *r = Got::GAME_KEYS; r->_id; ++r) {
	    Common::Action *act = new Common::Action(r->_id, _(r->_desc));
        act->setCustomEngineActionEvent(r->_action);
        act->addDefaultInputMapping(r->_key);
        if (r->_joy)
            act->addDefaultInputMapping(r->_joy);
        if (r->_action == Got::KEYBIND_SELECT)
            act->addDefaultInputMapping("RETURN");

        keyMap->addAction(act);
    }

    return keymapArray;
}


#if PLUGIN_ENABLED_DYNAMIC(GOT)
REGISTER_PLUGIN_DYNAMIC(GOT, PLUGIN_TYPE_ENGINE, GotMetaEngine);
#else
REGISTER_PLUGIN_STATIC(GOT, PLUGIN_TYPE_ENGINE, GotMetaEngine);
#endif
