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
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"
#include "vcruise/vcruise.h"

namespace Graphics {

struct Surface;

} // End of namespace Graphics

namespace VCruise {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_INCREASE_DRAG_DISTANCE,
		{
			_s("Improved click sensitivity"),
			_s("Allows the mouse to be moved a short distance without cancelling click interactions.  If this is off, any mouse movement while the mouse button is held cancels a click interaction."),
			"vcruise_increase_drag_distance",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_FAST_ANIMATIONS,
		{
			_s("Faster animations"),
			_s("Speeds up animations."),
			"vcruise_fast_animations",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_LAUNCH_DEBUG,
		{
			_s("Start with debugger"),
			_s("Starts with the debugger dashboard active."),
			"vcruise_debug",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_SKIP_MENU,
		{
			_s("Skip main menu"),
			_s("Starts a new game upon launching instead of going to the main menu."),
			"vcruise_skip_menu",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_USE_4BIT_GRAPHICS,
		{
			_s("Use 16-color graphics"),
			_s("Uses 16-color graphics."),
			"vcruise_use_4bit",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_PRELOAD_SOUNDS,
		{
			_s("Preload sounds"),
			_s("Preload sounds. May improve performance on slow hard drives."),
			"vcruise_preload_sounds",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FAST_VIDEO_DECODER,
		{
			_s("Faster video decoder (lower quality)"),
			_s("Reduce video decoding CPU usage at the cost of quality."),
			"vcruise_fast_video_decoder",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace VCruise

class VCruiseMetaEngine : public AdvancedMetaEngine<VCruise::VCruiseGameDescription> {
public:
	const char *getName() const override {
		return "vcruise";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return VCruise::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const VCruise::VCruiseGameDescription *desc) const override;

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;
};

bool VCruiseMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch (f) {
	case kSupportsLoadingDuringStartup:
		return true;
	default:
		break;
	}

	return checkExtendedSaves(f);
}

Common::Error VCruiseMetaEngine::createInstance(OSystem *syst, Engine **engine, const VCruise::VCruiseGameDescription *desc) const {
	*engine = new VCruise::VCruiseEngine(syst, reinterpret_cast<const VCruise::VCruiseGameDescription *>(desc));
	return Common::kNoError;
}

Common::Array<Common::Keymap *> VCruiseMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *keymap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "vcruise", "V-Cruise");
	Common::Action *act;

	act = new Common::Action(Common::kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_ESCAPE", _("Escape"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_HELP", _("Display help screen"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventHelp);
	act->addDefaultInputMapping("F1");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_SAVE_GAME", _("Save game"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventSaveGame);
	act->addDefaultInputMapping("F2");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_LOAD_GAME", _("Load game"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventLoadGame);
	act->addDefaultInputMapping("F3");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_SOUND_SETTINGS", _("Open sound settings"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventSoundSettings);
	act->addDefaultInputMapping("F4");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_QUIT", _("Quit game"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventQuit);
	act->addDefaultInputMapping("F10");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_PAUSE", _("Pause game"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventPause);
	act->addDefaultInputMapping("SPACE");
	keymap->addAction(act);


	act = new Common::Action("VCRUISE_MUSIC_TOGGLE", _("Toggle music on/off"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventMusicToggle);
	act->addDefaultInputMapping("F5");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_SOUND_TOGGLE", _("Toggle sound effects on/off"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventSoundToggle);
	act->addDefaultInputMapping("F6");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_MUSIC_VOLUME_DOWN", _("Music volume down"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventMusicVolumeDown);
	act->addDefaultInputMapping("F7");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_MUSIC_VOLUME_UP", _("Music volume up"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventMusicVolumeUp);
	act->addDefaultInputMapping("F8");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_SOUND_EFFECTS_VOLUME_DOWN", _("Sound effect volume down"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventSoundVolumeUp);
	act->addDefaultInputMapping("F11");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_SOUND_EFFECTS_VOLUME_UP", _("Sound effect volume up"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventSoundVolumeDown);
	act->addDefaultInputMapping("F12");
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_SKIP_ANIMATION", _("Skip current animation"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventSkipAnimation);
	keymap->addAction(act);

	act = new Common::Action("VCRUISE_PUT_ITEM", _("Cycle item in scene (debug cheat)"));
	act->setCustomEngineActionEvent(VCruise::kKeymappedEventPutItem);
	keymap->addAction(act);

	return Common::Keymap::arrayOf(keymap);
}

#if PLUGIN_ENABLED_DYNAMIC(VCRUISE)
REGISTER_PLUGIN_DYNAMIC(VCRUISE, PLUGIN_TYPE_ENGINE, VCruiseMetaEngine);
#else
REGISTER_PLUGIN_STATIC(VCRUISE, PLUGIN_TYPE_ENGINE, VCruiseMetaEngine);
#endif
