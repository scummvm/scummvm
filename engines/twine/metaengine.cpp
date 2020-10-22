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

#include "twine/keyboard.h"
#include "twine/twine.h"

namespace TwinE {

class TwinEMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "TwinE";
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return false;
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		if (desc) {
			*engine = new TwinE::TwinEEngine(syst, desc->language, desc->flags);
		}
		return desc != nullptr;
	}

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;
};

Common::KeymapArray TwinEMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "twine", "Little Big Adventure");

	Action *act;

	act = new Action("PAUSE", _("Pause"));
	act->setCustomEngineActionEvent(TwinEActionType::Pause);
	act->addDefaultInputMapping("p");
	engineKeyMap->addAction(act);

	act = new Action("NEXTROOM", _("Debug Next Room"));
	act->setCustomEngineActionEvent(TwinEActionType::NextRoom);
	act->addDefaultInputMapping("r");
	engineKeyMap->addAction(act);

	act = new Action("PREVIOUSROOM", _("Debug Previous Room"));
	act->setCustomEngineActionEvent(TwinEActionType::PreviousRoom);
	act->addDefaultInputMapping("f");
	engineKeyMap->addAction(act);

	act = new Action("APPLYCELLINGGRID", _("Debug Apply Celling Grid"));
	act->setCustomEngineActionEvent(TwinEActionType::ApplyCellingGrid);
	act->addDefaultInputMapping("t");
	engineKeyMap->addAction(act);

	act = new Action("INCREASECELLINGGRIDINDEX", _("Debug Increase Celling Grid Index"));
	act->setCustomEngineActionEvent(TwinEActionType::IncreaseCellingGridIndex);
	act->addDefaultInputMapping("g");
	engineKeyMap->addAction(act);

	act = new Action("DECREASECELLINGGRIDINDEX", _("Debug Decrease Celling Grid Index"));
	act->setCustomEngineActionEvent(TwinEActionType::DecreaseCellingGridIndex);
	act->addDefaultInputMapping("b");
	engineKeyMap->addAction(act);

	act = new Action("DEBUGGRIDCAMERAPRESSUP", _("Debug Grid Camera Up"));
	act->setCustomEngineActionEvent(TwinEActionType::DebugGridCameraPressUp);
	act->addDefaultInputMapping("s");
	engineKeyMap->addAction(act);

	act = new Action("DEBUGGRIDCAMERAPRESSDOWN", _("Debug Grid Camera Down"));
	act->setCustomEngineActionEvent(TwinEActionType::DebugGridCameraPressDown);
	act->addDefaultInputMapping("x");
	engineKeyMap->addAction(act);

	act = new Action("DEBUGGRIDCAMERAPRESSLEFT", _("Debug Grid Camera Left"));
	act->setCustomEngineActionEvent(TwinEActionType::DebugGridCameraPressLeft);
	act->addDefaultInputMapping("y");
	act->addDefaultInputMapping("z");
	engineKeyMap->addAction(act);

	act = new Action("DEBUGGRIDCAMERAPRESSRIGHT", _("Debug Grid Camera Right"));
	act->setCustomEngineActionEvent(TwinEActionType::DebugGridCameraPressRight);
	act->addDefaultInputMapping("c");
	engineKeyMap->addAction(act);

	act = new Action("NORMALBEHAVIOUR", _("Normal Behaviour"));
	act->setCustomEngineActionEvent(TwinEActionType::QuickBehaviourNormal);
	act->addDefaultInputMapping("F1");
	engineKeyMap->addAction(act);

	act = new Action("ATHLETICBEHAVIOUR", _("Athletic Behaviour"));
	act->setCustomEngineActionEvent(TwinEActionType::QuickBehaviourAthletic);
	act->addDefaultInputMapping("F2");
	engineKeyMap->addAction(act);

	act = new Action("AGGRESSIVEBEHAVIOUR", _("Aggressive Behaviour"));
	act->setCustomEngineActionEvent(TwinEActionType::QuickBehaviourAggressive);
	act->addDefaultInputMapping("F3");
	engineKeyMap->addAction(act);

	act = new Action("DISCREETBEHAVIOUR", _("Discreet Behaviour"));
	act->setCustomEngineActionEvent(TwinEActionType::QuickBehaviourDiscreet);
	act->addDefaultInputMapping("F4");
	engineKeyMap->addAction(act);

	act = new Action("BEHAVIOURACTION", _("Behaviour Action"));
	act->setCustomEngineActionEvent(TwinEActionType::ExecuteBehaviourAction);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("CHANGEBEHAVIOUR", _("Change Behaviour"));
	act->setCustomEngineActionEvent(TwinEActionType::BehaviourMenu);
	act->addDefaultInputMapping("CTRL");
	engineKeyMap->addAction(act);

	act = new Action("OPTIONSMENU", _("Options Menu"));
	act->setCustomEngineActionEvent(TwinEActionType::OptionsMenu);
	act->addDefaultInputMapping("F6");
	engineKeyMap->addAction(act);

	act = new Action("CENTER", _("Center"));
	act->setCustomEngineActionEvent(TwinEActionType::RecenterScreenOnTwinsen);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	engineKeyMap->addAction(act);

	act = new Action("USESELECTEDOBJECT", _("Use Selected Object"));
	act->setCustomEngineActionEvent(TwinEActionType::UseSelectedObject);
	act->addDefaultInputMapping("SHIFT+RETURN");
	act->addDefaultInputMapping("SHIFT+KP_ENTER");
	engineKeyMap->addAction(act);

	act = new Action("THROWMAGICBALL", _("Throw Magic Ball"));
	act->setCustomEngineActionEvent(TwinEActionType::ThrowMagicBall);
	act->addDefaultInputMapping("ALT");
	engineKeyMap->addAction(act);

	act = new Action("MOVEFORWARD", _("Move Forward"));
	act->setCustomEngineActionEvent(TwinEActionType::MoveForward);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("KP8");
	engineKeyMap->addAction(act);

	act = new Action("MOVEBACKWARD", _("Move Backward"));
	act->setCustomEngineActionEvent(TwinEActionType::MoveBackward);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("KP2");
	engineKeyMap->addAction(act);

	act = new Action("TurnRight", _("Turn Right"));
	act->setCustomEngineActionEvent(TwinEActionType::TurnRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("KP6");
	engineKeyMap->addAction(act);

	act = new Action("TurnLeft", _("Turn Left"));
	act->setCustomEngineActionEvent(TwinEActionType::TurnLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("KP4");
	engineKeyMap->addAction(act);

	act = new Action("USEPROTOPACK", _("Use Protopack"));
	act->setCustomEngineActionEvent(TwinEActionType::UseProtoPack);
	act->addDefaultInputMapping("j");
	engineKeyMap->addAction(act);

	act = new Action("OPENHOLOMAP", _("Open Holomap"));
	act->setCustomEngineActionEvent(TwinEActionType::OpenHolomap);
	act->addDefaultInputMapping("h");
	engineKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(TwinEActionType::InventoryMenu);
	act->addDefaultInputMapping("LSHIFT");
	act->addDefaultInputMapping("RSHIFT");
	act->addDefaultInputMapping("i");
	engineKeyMap->addAction(act);

	act = new Action("SPECIALACTION", _("Special Action"));
	act->setCustomEngineActionEvent(TwinEActionType::SpecialAction);
	act->addDefaultInputMapping("w");
	engineKeyMap->addAction(act);

	act = new Action("ESCAPE", _("Escape"));
	act->setCustomEngineActionEvent(TwinEActionType::Escape);
	act->addDefaultInputMapping("ESCAPE");
	engineKeyMap->addAction(act);

	act = new Action("PAGEUP", _("Page Up"));
	act->setCustomEngineActionEvent(TwinEActionType::PageUp);
	act->addDefaultInputMapping("PAGEUP");
	engineKeyMap->addAction(act);

	const int delta = (int)TwinEActionType::Max - (int)engineKeyMap->getActions().size();
	if (delta != 0) {
		error("Registered key map actions differs from TwinEActionType by %i", delta);
	}

	return Keymap::arrayOf(engineKeyMap);
}

} // namespace TwinE

#if PLUGIN_ENABLED_DYNAMIC(TWINE)
REGISTER_PLUGIN_DYNAMIC(TWINE, PLUGIN_TYPE_ENGINE, TwinE::TwinEMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TWINE, PLUGIN_TYPE_ENGINE, TwinE::TwinEMetaEngine);
#endif
