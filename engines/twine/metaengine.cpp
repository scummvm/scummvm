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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "base/plugins.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "twine/achievements_tables.h"
#include "twine/detection.h"
#include "twine/input.h"
#include "twine/renderer/screens.h"
#include "twine/twine.h"

namespace TwinE {

static const ADExtraGuiOptionsMap twineOptionsList[] = {
	{
		GAMEOPTION_WALL_COLLISION,
		{
			_s("Enable wall collisions"),
			_s("Enable the original wall collision damage"),
			"wallcollision",
			false,
			0,
			0
		}
	},
	{
		// this only changes the menu and doesn't change the autosave behaviour - as scummvm is handling this now
		GAMEOPTION_DISABLE_SAVE_MENU,
		{
			_s("Disable save menu"),
			_s("The original only had autosaves. This allows you to save whenever you want."),
			"useautosaving",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DEBUG,
		{
			_s("Enable debug mode"),
			_s("Enable the debug mode"),
			"debug",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_SOUND,
		{
			_s("Enable sound"),
			_s("Enable the sound for the game"),
			"sound",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_TEXT,
		{
			_s("Enable text"),
			_s("Enable the text for the game"),
			"subtitles",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_MOVIES,
		{
			_s("Enable movies"),
			_s("Enable the cutscenes for the game"),
			"movie",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_MOUSE,
		{
			_s("Enable mouse"),
			_s("Enable the mouse for the UI"),
			"mouse",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_USA_VERSION,
		{
			_s("Use the USA version"),
			_s("Enable the USA specific version flags"),
			"version",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_HIGH_RESOLUTION,
		{
			_s("Enable high resolution"),
			_s("Enable a higher resolution for the game"),
			"usehighres",
			false,
			0,
			0
		}
	},
#ifdef USE_TTS
	{
		GAMEOPTION_TEXT_TO_SPEECH,
		{
			_s("TTS Narrator"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_narrator",
			false,
			0,
			0
		}
	},
#endif
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class TwinEMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "twine";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return twineOptionsList;
	}

	int getMaximumSaveSlot() const override {
		return 6;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		TwineGameType gameType = TwineGameType::GType_LBA;
		const Common::String gameId = desc->gameId;
		if (gameId == "lba") {
			gameType = TwineGameType::GType_LBA;
		} else if (gameId == "lba2") {
			gameType = TwineGameType::GType_LBA2;
		} else if (gameId == "lbashow") {
			gameType = TwineGameType::GType_LBASHOW;
		}
		*engine = new TwinE::TwinEEngine(syst, desc->language, desc->flags, desc->platform, gameType);
		return Common::kNoError;
	}

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;

	const Common::AchievementDescriptionList *getAchievementDescriptionList() const override {
		return TwinE::achievementDescriptionList;
	}

	void getSavegameThumbnail(Graphics::Surface &thumb) override;
};

void TwinEMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	TwinEEngine *engine = (TwinEEngine *)g_engine;
	const Graphics::ManagedSurface &managedSurface = engine->_workVideoBuffer;
	const Graphics::Surface &screenSurface = managedSurface.rawSurface();
	Graphics::Palette *pal = &engine->_screens->_ptrPal;
	if (engine->_screens->_flagPalettePcx) {
		pal = &engine->_screens->_palettePcx;
	}
	::createThumbnail(&thumb, (const uint8 *)screenSurface.getPixels(), screenSurface.w, screenSurface.h, pal->data());
}

//
// unused:
// JOY_LEFT_TRIGGER
// JOY_RIGHT_TRIGGER
// JOY_RIGHT_SHOULDER
//
// JOY_RIGHT_STICK_X
// JOY_RIGHT_STICK_Y
// JOY_LEFT_STICK_X
// JOY_LEFT_STICK_Y
//
Common::KeymapArray TwinEMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	Action *act;

	KeymapArray array(4);

	{
		Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, mainKeyMapId, "Little Big Adventure");
		act = new Action("PAUSE", _("Pause"));
		act->setCustomEngineActionEvent(TwinEActionType::Pause);
		act->addDefaultInputMapping("p");
		gameKeyMap->addAction(act);

		act = new Action("DEBUGGRIDCAMERAPRESSUP", _("Debug Grid Camera Up"));
		act->setCustomEngineActionEvent(TwinEActionType::DebugGridCameraPressUp);
		act->addDefaultInputMapping("s");
		gameKeyMap->addAction(act);

		act = new Action("DEBUGGRIDCAMERAPRESSDOWN", _("Debug Grid Camera Down"));
		act->setCustomEngineActionEvent(TwinEActionType::DebugGridCameraPressDown);
		act->addDefaultInputMapping("x");
		gameKeyMap->addAction(act);

		act = new Action("DEBUGGRIDCAMERAPRESSLEFT", _("Debug Grid Camera Left"));
		act->setCustomEngineActionEvent(TwinEActionType::DebugGridCameraPressLeft);
		act->addDefaultInputMapping("y");
		act->addDefaultInputMapping("z");
		gameKeyMap->addAction(act);

		act = new Action("DEBUGGRIDCAMERAPRESSRIGHT", _("Debug Grid Camera Right"));
		act->setCustomEngineActionEvent(TwinEActionType::DebugGridCameraPressRight);
		act->addDefaultInputMapping("c");
		gameKeyMap->addAction(act);

		act = new Action("CHANGETONORMALBEHAVIOUR", _("Normal Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::ChangeBehaviourNormal);
		act->addDefaultInputMapping("1");
		gameKeyMap->addAction(act);

		act = new Action("CHANGETOATHLETICBEHAVIOUR", _("Athletic Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::ChangeBehaviourAthletic);
		act->addDefaultInputMapping("2");
		gameKeyMap->addAction(act);

		act = new Action("CHANGETOAGGRESSIVEBEHAVIOUR", _("Aggressive Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::ChangeBehaviourAggressive);
		act->addDefaultInputMapping("3");
		gameKeyMap->addAction(act);

		act = new Action("CHANGETODISCREETBEHAVIOUR", _("Discreet Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::ChangeBehaviourDiscreet);
		act->addDefaultInputMapping("4");
		gameKeyMap->addAction(act);

		act = new Action("NORMALBEHAVIOUR", _("Normal Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::QuickBehaviourNormal);
		act->addDefaultInputMapping("F1");
		gameKeyMap->addAction(act);

		act = new Action("ATHLETICBEHAVIOUR", _("Athletic Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::QuickBehaviourAthletic);
		act->addDefaultInputMapping("F2");
		gameKeyMap->addAction(act);

		act = new Action("AGGRESSIVEBEHAVIOUR", _("Aggressive Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::QuickBehaviourAggressive);
		act->addDefaultInputMapping("F3");
		gameKeyMap->addAction(act);

		act = new Action("DISCREETBEHAVIOUR", _("Discreet Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::QuickBehaviourDiscreet);
		act->addDefaultInputMapping("F4");
		gameKeyMap->addAction(act);

		act = new Action("BEHAVIOURACTION", _("Behaviour Action"));
		act->setCustomEngineActionEvent(TwinEActionType::ExecuteBehaviourAction);
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_A");
		gameKeyMap->addAction(act);

		act = new Action("CHANGEBEHAVIOUR", _("Change Behaviour"));
		act->setCustomEngineActionEvent(TwinEActionType::BehaviourMenu);
		act->addDefaultInputMapping("LCTRL");
		act->addDefaultInputMapping("RCTRL");
		act->addDefaultInputMapping("JOY_X");
		gameKeyMap->addAction(act);

		act = new Action("MENU", _("Global Main Menu"));
		act->addDefaultInputMapping("F5");
		act->setEvent(EVENT_MAINMENU);
		gameKeyMap->addAction(act);

		act = new Action("OPTIONSMENU", _("Options Menu"));
		act->setCustomEngineActionEvent(TwinEActionType::OptionsMenu);
		act->addDefaultInputMapping("F6");
		gameKeyMap->addAction(act);

		act = new Action("CENTER", _("Center"));
		act->setCustomEngineActionEvent(TwinEActionType::RecenterScreenOnTwinsen);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("JOY_RIGHT_STICK");
		gameKeyMap->addAction(act);

		act = new Action("USESELECTEDOBJECT", _("Use Selected Object"));
		act->setCustomEngineActionEvent(TwinEActionType::UseSelectedObject);
		act->addDefaultInputMapping("S+RETURN");
		act->addDefaultInputMapping("S+KP_ENTER");
		gameKeyMap->addAction(act);

		act = new Action("THROWMAGICBALL", _("Throw Magic Ball"));
		act->setCustomEngineActionEvent(TwinEActionType::ThrowMagicBall);
		act->addDefaultInputMapping("LALT");
		act->addDefaultInputMapping("RALT");
		act->addDefaultInputMapping("JOY_LEFT_STICK");
		gameKeyMap->addAction(act);

		act = new Action("MOVEFORWARD", _("Move Forward"));
		act->setCustomEngineActionEvent(TwinEActionType::MoveForward);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("KP8");
		act->addDefaultInputMapping("JOY_UP");
		gameKeyMap->addAction(act);

		act = new Action("MOVEBACKWARD", _("Move Backwards"));
		act->setCustomEngineActionEvent(TwinEActionType::MoveBackward);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("KP2");
		act->addDefaultInputMapping("JOY_DOWN");
		gameKeyMap->addAction(act);

		act = new Action("TURNRIGHT", _("Turn Right"));
		act->setCustomEngineActionEvent(TwinEActionType::TurnRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);

		act = new Action("TURNLEFT", _("Turn Left"));
		act->setCustomEngineActionEvent(TwinEActionType::TurnLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

		act = new Action("USEPROTOPACK", _("Use Protopack"));
		act->setCustomEngineActionEvent(TwinEActionType::UseProtoPack);
		act->addDefaultInputMapping("j");
		gameKeyMap->addAction(act);

		act = new Action("OPENHOLOMAP", _("Open Holomap"));
		act->setCustomEngineActionEvent(TwinEActionType::OpenHolomap);
		act->addDefaultInputMapping("h");
		gameKeyMap->addAction(act);

		act = new Action("INVENTORY", _("Inventory"));
		act->setCustomEngineActionEvent(TwinEActionType::InventoryMenu);
		act->addDefaultInputMapping("LSHIFT");
		act->addDefaultInputMapping("RSHIFT");
		act->addDefaultInputMapping("JOY_Y");
		act->addDefaultInputMapping("i");
		gameKeyMap->addAction(act);

		act = new Action("SPECIALACTION", _("Special Action"));
		act->setCustomEngineActionEvent(TwinEActionType::SpecialAction);
		act->addDefaultInputMapping("w");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("SCENERYZOOM", _("Scenery Zoom"));
		act->setCustomEngineActionEvent(TwinEActionType::SceneryZoom);
		gameKeyMap->addAction(act);

		act = new Action("ESCAPE", _("Escape"));
		act->setCustomEngineActionEvent(TwinEActionType::Escape);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_B");
		act->addDefaultInputMapping("JOY_BACK");
		gameKeyMap->addAction(act);

		// TODO: lba2 has shortcuts for the inventory items
		// J: Protopack/Jetpack
		// P: Mecha-Penguin
		// H: Holomap
		// X: Dodges
		// 1: Magic Ball
		// 2: Darts
		// 3: Blowpipe/Blowtron
		// 4: Conch Shell
		// 5: Glove
		// 6: Laser Gun
		// 7: Saber

		array[0] = gameKeyMap;
	}

	{
		Keymap *uiKeyMap = new Keymap(Keymap::kKeymapTypeGame, uiKeyMapId, "Little Big Adventure UI");

		act = new Action("ACCEPT", _("Accept"));
		act->setCustomEngineActionEvent(TwinEActionType::UIEnter);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		uiKeyMap->addAction(act);

		act = new Action("ABORT", _("Abort"));
		act->setCustomEngineActionEvent(TwinEActionType::UIAbort);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_BACK");
		act->addDefaultInputMapping("JOY_B");
		uiKeyMap->addAction(act);

		act = new Action("UP", _("Up"));
		act->setCustomEngineActionEvent(TwinEActionType::UIUp);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("KP8");
		act->addDefaultInputMapping("JOY_UP");
		uiKeyMap->addAction(act);

		act = new Action("DOWN", _("Down"));
		act->setCustomEngineActionEvent(TwinEActionType::UIDown);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("KP2");
		act->addDefaultInputMapping("JOY_DOWN");
		uiKeyMap->addAction(act);

		act = new Action("RIGHT", _("Right"));
		act->setCustomEngineActionEvent(TwinEActionType::UIRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		act->addDefaultInputMapping("MOUSE_WHEEL_UP");
		act->addDefaultInputMapping("JOY_RIGHT");
		uiKeyMap->addAction(act);

		act = new Action("LEFT", _("Left"));
		act->setCustomEngineActionEvent(TwinEActionType::UILeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
		act->addDefaultInputMapping("MOUSE_WHEEL_DOWN");
		act->addDefaultInputMapping("JOY_LEFT");
		uiKeyMap->addAction(act);

		act = new Action("NEXTPAGE", _("Next Page"));
		act->setCustomEngineActionEvent(TwinEActionType::UINextPage);
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("PAGEDOWN");
		act->addDefaultInputMapping("JOY_B");
		act->addDefaultInputMapping("JOY_BACK");
		uiKeyMap->addAction(act);

		array[1] = uiKeyMap;
	}

	{
		Keymap *cutsceneKeyMap = new Keymap(Keymap::kKeymapTypeGame, cutsceneKeyMapId, "Little Big Adventure Cutscenes");

		act = new Action("ABORT", _("Abort"));
		act->setCustomEngineActionEvent(TwinEActionType::CutsceneAbort);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_B");
		act->addDefaultInputMapping("JOY_BACK");
		cutsceneKeyMap->addAction(act);

		array[2] = cutsceneKeyMap;
	}

	{
		Keymap *holomapKeyMap = new Keymap(Keymap::kKeymapTypeGame, holomapKeyMapId, "Little Big Adventure Holomap");

		act = new Action("ABORT", _("Abort"));
		act->setCustomEngineActionEvent(TwinEActionType::HolomapAbort);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_B");
		act->addDefaultInputMapping("JOY_BACK");
		holomapKeyMap->addAction(act);

		act = new Action("UP", _("Up"));
		act->setCustomEngineActionEvent(TwinEActionType::HolomapUp);
		act->addDefaultInputMapping("C+UP");
		act->addDefaultInputMapping("KP8");
		act->addDefaultInputMapping("MOUSE_WHEEL_UP");
		act->addDefaultInputMapping("JOY_UP");
		holomapKeyMap->addAction(act);

		act = new Action("DOWN", _("Down"));
		act->setCustomEngineActionEvent(TwinEActionType::HolomapDown);
		act->addDefaultInputMapping("C+DOWN");
		act->addDefaultInputMapping("KP2");
		act->addDefaultInputMapping("MOUSE_WHEEL_DOWN");
		act->addDefaultInputMapping("JOY_DOWN");
		holomapKeyMap->addAction(act);

		act = new Action("RIGHT", _("Right"));
		act->setCustomEngineActionEvent(TwinEActionType::HolomapRight);
		act->addDefaultInputMapping("C+RIGHT");
		act->addDefaultInputMapping("KP6");
		act->addDefaultInputMapping("JOY_RIGHT");
		holomapKeyMap->addAction(act);

		act = new Action("LEFT", _("Left"));
		act->setCustomEngineActionEvent(TwinEActionType::HolomapLeft);
		act->addDefaultInputMapping("C+LEFT");
		act->addDefaultInputMapping("KP4");
		act->addDefaultInputMapping("JOY_LEFT");
		holomapKeyMap->addAction(act);

		act = new Action("PREV", _("Previous location"));
		act->setCustomEngineActionEvent(TwinEActionType::HolomapPrev);
		act->addDefaultInputMapping("LEFT");
		holomapKeyMap->addAction(act);

		act = new Action("NEXT", _("Next location"));
		act->setCustomEngineActionEvent(TwinEActionType::HolomapNext);
		act->addDefaultInputMapping("RIGHT");
		holomapKeyMap->addAction(act);

		array[3] = holomapKeyMap;
	}

	return array;
}

} // namespace TwinE

#if PLUGIN_ENABLED_DYNAMIC(TWINE)
REGISTER_PLUGIN_DYNAMIC(TWINE, PLUGIN_TYPE_ENGINE, TwinE::TwinEMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TWINE, PLUGIN_TYPE_ENGINE, TwinE::TwinEMetaEngine);
#endif
