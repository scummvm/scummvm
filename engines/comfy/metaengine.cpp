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

#include "comfy/metaengine.h"
#include "comfy/detection.h"
#include "comfy/comfy.h"

namespace Comfy {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_FORCE_LANGUAGE_SETUP,
		{
			_s("Force language setup screen on start-up"),
			_s("Show the original language setup screen every time the game starts"),
			"force_language_setup",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Comfy

const char *ComfyMetaEngine::getName() const {
	return "comfy";
}

const ADExtraGuiOptionsMap *ComfyMetaEngine::getAdvancedExtraGuiOptions() const {
	return Comfy::optionsList;
}

Common::Error ComfyMetaEngine::createInstance(OSystem *syst, Engine **engine, const Comfy::ComfyGameDescription *desc) const {
	*engine = new Comfy::ComfyEngine(syst, desc);
	return Common::kNoError;
}

bool ComfyMetaEngine::hasFeature(MetaEngineFeature) const {
	return false;
}

Common::KeymapArray ComfyMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	static const char *actionIds[COMFY_KEYBOARD_CONTACT_COUNT] = {
		"SUN", "ROLL", "RED", "DRUM", "SNAILY", "PURPLE", "PIANO", "POWER",
		"ORANGE", "MOON", "JUMPY", "YELLOW", "STOP", "HANDSET", "BLUE", "TRUMPET",
		"BUDDY", "GREEN", "FLUTE", "CONTACT19", "COMFY", "CLOUD", "FEELY", "MUSIC"
	};
	static const char *actionNames[COMFY_KEYBOARD_CONTACT_COUNT] = {
		_s("Sun"), _s("Rolling cylinder"), _s("Red"), _s("Drum"), _s("Snaily phone"), _s("Purple"),
		_s("Piano"), _s("Power"), _s("Orange"), _s("Moon"), _s("Jumpy phone"), _s("Yellow"),
		_s("Stop"), _s("Handset"), _s("Blue"), _s("Trumpet"), _s("Buddy phone"), _s("Green"),
		_s("Flute"), _s("Keyboard contact 19"), _s("Comfy phone"), _s("Rainy cloud"),
		_s("Feely phone"), _s("Music")
	};
	static const char *defaultMappings[COMFY_KEYBOARD_CONTACT_COUNT] = {
		"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "q", "w",
		"e", "r", "t", "y", "u", "i", "o", "p", "a", "s", "d", "f"
	};

	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "comfy-keyboard", _s("Comfy Keyboard"));
	for (uint i = 0; i < COMFY_KEYBOARD_CONTACT_COUNT; i++) {
		Action *action = new Action(actionIds[i], _(actionNames[i]));
		action->setCustomEngineActionEvent(Comfy::kActionKeyboardContact0 + i);
		action->addDefaultInputMapping(defaultMappings[i]);
		keymap->addAction(action);
	}

	return Keymap::arrayOf(keymap);
}

#if PLUGIN_ENABLED_DYNAMIC(COMFY)
REGISTER_PLUGIN_DYNAMIC(COMFY, PLUGIN_TYPE_ENGINE, ComfyMetaEngine);
#else
REGISTER_PLUGIN_STATIC(COMFY, PLUGIN_TYPE_ENGINE, ComfyMetaEngine);
#endif
