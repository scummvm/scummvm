/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"
#include "base/plugins.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "twine/detection.h"

#include "twine/input.h"
#include "twine/twine.h"

namespace TwinE {

class TwinEMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "TwinE";
	}

	int getMaximumSaveSlot() const override {
		return 6;
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		if (desc) {
			TwineGameType gameType = TwineGameType::GType_LBA;
			const Common::String gameId = desc->gameId;
			if (gameId == "lba") {
				gameType = TwineGameType::GType_LBA;
			} else if (gameId == "lba2") {
				gameType = TwineGameType::GType_LBA2;
			}
			*engine = new TwinE::TwinEEngine(syst, desc->language, desc->flags, gameType);
		}
		return desc != nullptr;
	}

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;
};

static const ExtraGuiOption OptWallCollision = {
	_s("Enable wall collisions"),
	_s("Enable the original wall collision damage"),
	"wallcollision",
	false
};

static const ExtraGuiOption OptCrossFade = {
	_s("Enable cross fade"),
	_s("Enable cross fading of images and scenes"),
	"crossfade",
	false
};

// this only changes the menu and doesn't change the autosave behaviour - as scummvm is handling this now
static const ExtraGuiOption OptDisableSaveMenu = {
	_s("Disable save menu"),
	_s("The original only had autosaves. This allows you to save whenever you want."),
	"useautosaving",
	false
};

static const ExtraGuiOption OptDebug = {
	_s("Enable debug mode"),
	_s("Enable the debug mode"),
	"debug",
	false
};

static const ExtraGuiOption OptUseCD = {
	_s("Enable audio CD"),
	_s("Enable the original audio cd track"),
	"usecd",
	false
};

static const ExtraGuiOption OptSound = {
	_s("Enable sound"),
	_s("Enable the sound for the game"),
	"sound",
	true
};

static const ExtraGuiOption OptVoices = {
	_s("Enable voices"),
	_s("Enable the voices for the game"),
	"voice",
	true
};

static const ExtraGuiOption OptText = {
	_s("Enable text"),
	_s("Enable the text for the game"),
	"displaytext",
	true
};

static const ExtraGuiOption OptMovies = {
	_s("Enable movies"),
	_s("Enable the cutscenes for the game"),
	"movie",
	true
};

static const ExtraGuiOption OptUSAVersion = {
	_s("Use the USA version"),
	_s("Enable the USA specific version flags"),
	"version",
	false
};

const ExtraGuiOptions TwinEMetaEngine::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	options.push_back(OptWallCollision);
	options.push_back(OptCrossFade);
	options.push_back(OptDisableSaveMenu);
	options.push_back(OptDebug);
	options.push_back(OptUseCD);
	options.push_back(OptSound);
	options.push_back(OptMovies);
	options.push_back(OptUSAVersion);
	options.push_back(OptVoices);
	options.push_back(OptText);
	return options;
}

Common::KeymapArray TwinEMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	Action *act;

	KeymapArray array(3);

	{
		Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, mainKeyMapId, "Little Big Adventure");
		act = new Action("PAUSE", _("Pause"));
		act->setCustomEngineActionEvent(TwinEActionType::Pause);
		act->addDefaultInputMapping("p");
		gameKeyMap->addAction(act);

		act = new Action("NEXTROOM", _("Debug Next Room"));
		act->setCustomEngineActionEvent(TwinEActionType::NextRoom);
		act->addDefaultInputMapping("r");
		gameKeyMap->addAction(act);

		act = new Action("PREVIOUSROOM", _("Debug Previous Room"));
		act->setCustomEngineActionEvent(TwinEActionType::PreviousRoom);
		act->addDefaultInputMapping("f");
		gameKeyMap->addAction(act);

		act = new Action("APPLYCELLINGGRID", _("Debug Apply Celling Grid"));
		act->setCustomEngineActionEvent(TwinEActionType::ApplyCellingGrid);
		act->addDefaultInputMapping("t");
		gameKeyMap->addAction(act);

		act = new Action("INCREASECELLINGGRIDINDEX", _("Debug Increase Celling Grid Index"));
		act->setCustomEngineActionEvent(TwinEActionType::IncreaseCellingGridIndex);
		act->addDefaultInputMapping("g");
		gameKeyMap->addAction(act);

		act = new Action("DECREASECELLINGGRIDINDEX", _("Debug Decrease Celling Grid Index"));
		act->setCustomEngineActionEvent(TwinEActionType::DecreaseCellingGridIndex);
		act->addDefaultInputMapping("b");
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

		act = new Action("DEBUGMENU", _("Debug Menu"));
		act->setCustomEngineActionEvent(TwinEActionType::DebugMenu);
		act->addDefaultInputMapping("MOUSE_RIGHT");
		gameKeyMap->addAction(act);

		act = new Action("DEBUGMENUEXEC", _("Debug Menu Execute"));
		act->setCustomEngineActionEvent(TwinEActionType::DebugMenuActivate);
		act->addDefaultInputMapping("MOUSE_LEFT");
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
		gameKeyMap->addAction(act);

		act = new Action("OPTIONSMENU", _("Options Menu"));
		act->setCustomEngineActionEvent(TwinEActionType::OptionsMenu);
		act->addDefaultInputMapping("F6");
		gameKeyMap->addAction(act);

		act = new Action("CENTER", _("Center"));
		act->setCustomEngineActionEvent(TwinEActionType::RecenterScreenOnTwinsen);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
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
		gameKeyMap->addAction(act);

		act = new Action("MOVEFORWARD", _("Move Forward"));
		act->setCustomEngineActionEvent(TwinEActionType::MoveForward);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("KP8");
		gameKeyMap->addAction(act);

		act = new Action("MOVEBACKWARD", _("Move Backward"));
		act->setCustomEngineActionEvent(TwinEActionType::MoveBackward);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("KP2");
		gameKeyMap->addAction(act);

		act = new Action("TURNRIGHT", _("Turn Right"));
		act->setCustomEngineActionEvent(TwinEActionType::TurnRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		gameKeyMap->addAction(act);

		act = new Action("TURNLEFT", _("Turn Left"));
		act->setCustomEngineActionEvent(TwinEActionType::TurnLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
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
		act->addDefaultInputMapping("i");
		gameKeyMap->addAction(act);

		act = new Action("SPECIALACTION", _("Special Action"));
		act->setCustomEngineActionEvent(TwinEActionType::SpecialAction);
		act->addDefaultInputMapping("w");
		gameKeyMap->addAction(act);

		act = new Action("ESCAPE", _("Escape"));
		act->setCustomEngineActionEvent(TwinEActionType::Escape);
		act->addDefaultInputMapping("ESCAPE");
		gameKeyMap->addAction(act);

		array[0] = gameKeyMap;
	}

	{
		Keymap *uiKeyMap = new Keymap(Keymap::kKeymapTypeGame, uiKeyMapId, "Little Big Adventure UI");

		act = new Action("ACCEPT", _("Accept"));
		act->setCustomEngineActionEvent(TwinEActionType::UIEnter);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		uiKeyMap->addAction(act);

		act = new Action("ABORT", _("Abort"));
		act->setCustomEngineActionEvent(TwinEActionType::UIAbort);
		act->addDefaultInputMapping("ESCAPE");
		uiKeyMap->addAction(act);

		act = new Action("UP", _("Up"));
		act->setCustomEngineActionEvent(TwinEActionType::UIUp);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("KP8");
		uiKeyMap->addAction(act);

		act = new Action("DOWN", _("Down"));
		act->setCustomEngineActionEvent(TwinEActionType::UIDown);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("KP2");
		uiKeyMap->addAction(act);

		act = new Action("RIGHT", _("Right"));
		act->setCustomEngineActionEvent(TwinEActionType::UIRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		uiKeyMap->addAction(act);

		act = new Action("LEFT", _("Left"));
		act->setCustomEngineActionEvent(TwinEActionType::UILeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
		uiKeyMap->addAction(act);

		act = new Action("NEXTPAGE", _("Next Page"));
		act->setCustomEngineActionEvent(TwinEActionType::UINextPage);
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("PAGEDOWN");
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
		cutsceneKeyMap->addAction(act);

		array[2] = cutsceneKeyMap;
	}

	return array;
}

} // namespace TwinE

#if PLUGIN_ENABLED_DYNAMIC(TWINE)
REGISTER_PLUGIN_DYNAMIC(TWINE, PLUGIN_TYPE_ENGINE, TwinE::TwinEMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TWINE, PLUGIN_TYPE_ENGINE, TwinE::TwinEMetaEngine);
#endif
