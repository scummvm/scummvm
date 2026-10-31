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
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "igor/detection.h"
#include "igor/igor.h"
#include "igor/metaengine.h"

namespace Igor {

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
	{
		GAMEOPTION_SKIP_COPY_PROTECTION,
		{
			_s("Skip copy protection"),
			_s("Skips original copy protection screen."),
			"skip_copy_protection",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Igor

const char *IgorMetaEngine::getName() const {
	return "igor";
}

void IgorMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	if (Igor::g_engine && Igor::g_engine->copyOptionsMenuThumbnail(thumb))
		return;
	MetaEngine::getSavegameThumbnail(thumb);
}

const ADExtraGuiOptionsMap *IgorMetaEngine::getAdvancedExtraGuiOptions() const {
	return Igor::optionsList;
}

Common::KeymapArray IgorMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *mouseKeymap = new Keymap(Keymap::kKeymapTypeGame, "igor-default", _("Default keymappings"));
	// The keyboard actions live in their own keymap: it is suspended while the player types text.
	Keymap *keyKeymap = new Keymap(Keymap::kKeymapTypeGame, "igor-keys", _("Keyboard keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	mouseKeymap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	mouseKeymap->addAction(act);

	act = new Common::Action(kStandardActionOpenMainMenu, _("Game menu"));
	act->setKeyEvent(KeyState(KEYCODE_F5, ASCII_F5));
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	act->allowKbdRepeats();
	mouseKeymap->addAction(act);

	struct KeyAction {
		const char *id;
		const char *desc;
		int action;
		const char *keys[3];
		bool repeat;
	};
	// The quit confirmation answers with the initial of "yes" in the game's language.
	const char *yesKey = (ConfMan.hasKey("language", target) && parseLanguage(ConfMan.get("language", target)) == ES_ESP) ? "s" : "y";
	// I18N: Escape is a key
	const KeyAction keyActions[] = {
		{ "ESCAPE", _s("Escape"), Igor::kActionEscape, { "ESCAPE", "JOY_X", nullptr }, true },
		{ "CONFIRM", _s("Confirm"), Igor::kActionConfirm, { "RETURN", "KP_ENTER", nullptr }, true },
		{ "OPTIONS", _s("Options menu"), Igor::kActionOptions, { "SPACE", "JOY_Y", nullptr }, true },
		{ "PAUSE", _s("Pause"), Igor::kActionPause, { "p", nullptr, nullptr }, true },
		{ "FASTMODE", _s("Toggle fast mode"), Igor::kActionFastMode, { "C+f", nullptr, nullptr }, false },
		{ "KEYRIGHTCLICK", _s("Right click with the keyboard"), Igor::kActionKeyRightClick, { "INSERT", nullptr, nullptr }, true },
		{ "UP", _s("Up"), Igor::kActionUp, { "UP", "KP8", "JOY_UP" }, true },
		{ "DOWN", _s("Down"), Igor::kActionDown, { "DOWN", "KP2", "JOY_DOWN" }, true },
		{ "LEFT", _s("Left"), Igor::kActionLeft, { "LEFT", "KP4", "JOY_LEFT" }, true },
		{ "RIGHT", _s("Right"), Igor::kActionRight, { "RIGHT", "KP6", "JOY_RIGHT" }, true },
		{ "UPLEFT", _s("Up and left"), Igor::kActionUpLeft, { "HOME", "KP7", nullptr }, true },
		{ "UPRIGHT", _s("Up and right"), Igor::kActionUpRight, { "PAGEUP", "KP9", nullptr }, true },
		{ "DOWNLEFT", _s("Down and left"), Igor::kActionDownLeft, { "END", "KP1", nullptr }, true },
		{ "DOWNRIGHT", _s("Down and right"), Igor::kActionDownRight, { "PAGEDOWN", "KP3", nullptr }, true },
		{ "YES", _s("Confirm quit"), Igor::kActionYes, { yesKey, nullptr, nullptr }, true },
		{ "NO", _s("Cancel quit"), Igor::kActionNo, { "n", nullptr, nullptr }, true }
	};
	for (uint i = 0; i < ARRAYSIZE(keyActions); ++i) {
		act = new Common::Action(keyActions[i].id, _(keyActions[i].desc));
		act->setCustomEngineActionEvent(keyActions[i].action);
		for (uint j = 0; j < ARRAYSIZE(keyActions[i].keys) && keyActions[i].keys[j]; ++j)
			act->addDefaultInputMapping(keyActions[i].keys[j]);
		if (keyActions[i].repeat)
			act->allowKbdRepeats();
		keyKeymap->addAction(act);
	}

	KeymapArray keymaps(2);
	keymaps[0] = mouseKeymap;
	keymaps[1] = keyKeymap;
	return keymaps;
}

Common::Error IgorMetaEngine::createInstance(OSystem *syst, Engine **engine, const Igor::IgorGameDescription *desc) const {
	*engine = new Igor::IgorEngine(syst, desc);
	return Common::kNoError;
}

bool IgorMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(IGOR)
REGISTER_PLUGIN_DYNAMIC(IGOR, PLUGIN_TYPE_ENGINE, IgorMetaEngine);
#else
REGISTER_PLUGIN_STATIC(IGOR, PLUGIN_TYPE_ENGINE, IgorMetaEngine);
#endif
