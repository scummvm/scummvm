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

#include "base/plugins.h"

#include "common/system.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"

#include "playground3d/playground3d.h"

class Playground3dMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "playground3d";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription * /* desc */) const override {
		*engine = new Playground3d::Playground3dEngine(syst);
		return Common::kNoError;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return false;
	}

	Common::KeymapArray initKeymaps(const char *target) const override {
		Common::Keymap *keymap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "playground3d", _("Default keymappings"));

		Common::Action *act;

		act = new Common::Action(Common::kStandardActionLeftClick, _("Switch test"));
		act->setCustomEngineActionEvent(Playground3d::kActionSwitchTest);
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		keymap->addAction(act);

		act = new Common::Action("FOG", _("Enable/Disable fog"));
		act->setCustomEngineActionEvent(Playground3d::kActionEnableFog);
		act->addDefaultInputMapping("f");
		act->addDefaultInputMapping("JOY_X");
		keymap->addAction(act);

		act = new Common::Action("SCISSOR", _("Enable/Disable scissor"));
		act->setCustomEngineActionEvent(Playground3d::kActionEnableScissor);
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_Y");
		keymap->addAction(act);

		Common::KeymapArray keymaps = { keymap };
		return keymaps;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(PLAYGROUND3D)
	REGISTER_PLUGIN_DYNAMIC(PLAYGROUND3D, PLUGIN_TYPE_ENGINE, Playground3dMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PLAYGROUND3D, PLUGIN_TYPE_ENGINE, Playground3dMetaEngine);
#endif
