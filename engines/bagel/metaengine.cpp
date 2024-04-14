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

namespace Bagel {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_key;
	const char *_joy;
};

static const KeybindingRecord MINIMAL_KEYS[] = {
	{ KEYBIND_WAIT, "WAIT", _s("Wait"), "SPACE", nullptr },
	{ KEYBIND_CHEAT714, "CHEAT714", _s("Soldier 714 Eye Cheat"), "SCROLLOCK", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

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

} // End of namespace Bagel

const char *BagelMetaEngine::getName() const {
	return "bagel";
}

const ADExtraGuiOptionsMap *BagelMetaEngine::getAdvancedExtraGuiOptions() const {
	return Bagel::optionsList;
}

Common::Error BagelMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Bagel::SpaceBar::SpaceBarEngine(syst, desc);
	return Common::kNoError;
}

bool BagelMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
	       (f == kSupportsLoadingDuringStartup);
}

Common::KeymapArray BagelMetaEngine::initKeymaps(const char *target) const {
	Common::KeymapArray keymapArray;
	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "bagel", _s("General Keys"));
	keymapArray.push_back(keyMap);

	for (const Bagel::KeybindingRecord *r = Bagel::MINIMAL_KEYS; r->_id; ++r) {
		Common::Action *act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		act->addDefaultInputMapping(r->_key);
		if (r->_joy)
			act->addDefaultInputMapping(r->_joy);

		keyMap->addAction(act);
	}

	return keymapArray;
}

#if PLUGIN_ENABLED_DYNAMIC(BAGEL)
REGISTER_PLUGIN_DYNAMIC(BAGEL, PLUGIN_TYPE_ENGINE, BagelMetaEngine);
#else
REGISTER_PLUGIN_STATIC(BAGEL, PLUGIN_TYPE_ENGINE, BagelMetaEngine);
#endif
