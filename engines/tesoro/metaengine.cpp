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
#include "backends/keymapper/standard-actions.h"
#include "common/translation.h"

#include "tesoro/detection.h"
#include "tesoro/metaengine.h"
#include "tesoro/tesoro.h"

const char *TesoroMetaEngine::getName() const {
	return "tesoro";
}

Common::Error TesoroMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Tesoro::TesoroEngine(syst, desc);
	return Common::kNoError;
}

bool TesoroMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

Common::KeymapArray TesoroMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *keymap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "tesoro-default", _("Default keymappings"));

	Common::Action *act = new Common::Action(Common::kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keymap->addAction(act);

	act = new Common::Action(Common::kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keymap->addAction(act);

	act = new Common::Action("FAST_MODE", _("Fast mode"));
	act->setCustomEngineActionEvent(Tesoro::kActionFastMode);
	act->addDefaultInputMapping("C+f");
	keymap->addAction(act);

	act = new Common::Action("MENU_KEY", _("Menu / skip movie"));
	act->setCustomEngineActionEvent(Tesoro::kActionMenuKey);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_START");
	keymap->addAction(act);

	return Common::Keymap::arrayOf(keymap);
}

void TesoroMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	// The scene kept for the save list; without one (no game running) the screen is taken as usual.
	if (Tesoro::g_engine && Tesoro::g_engine->sceneThumbnail().getPixels())
		thumb.copyFrom(Tesoro::g_engine->sceneThumbnail());
	else
		AdvancedMetaEngine<ADGameDescription>::getSavegameThumbnail(thumb);
}

#if PLUGIN_ENABLED_DYNAMIC(TESORO)
REGISTER_PLUGIN_DYNAMIC(TESORO, PLUGIN_TYPE_ENGINE, TesoroMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TESORO, PLUGIN_TYPE_ENGINE, TesoroMetaEngine);
#endif
