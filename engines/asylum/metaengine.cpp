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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/advancedDetector.h"
#include "base/plugins.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "common/translation.h"

#include "asylum/asylum.h"
#include "asylum/shared.h"

class AsylumMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "asylum";
	}

	const char *getOriginalCopyright() const {
		return "Sanitarium (c) ASC Games";
	}

	bool hasFeature(MetaEngineFeature f) const;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool AsylumMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

Common::Error AsylumMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Asylum::AsylumEngine(syst, desc);
	}
	return desc ? Common::kNoError : Common::kUnsupportedGameidError;
}

Common::KeymapArray AsylumMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Asylum;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "asylum", "Sanitarium");

	Action *act;

	act = new Action("VERSION", _("Show version"));
	act->setCustomEngineActionEvent(kAsylumActionShowVersion);
	act->addDefaultInputMapping("v");
	engineKeyMap->addAction(act);

	act = new Action("LOAD", _("Quick load"));
	act->setCustomEngineActionEvent(kAsylumActionQuickLoad);
	act->addDefaultInputMapping("S+l");
	engineKeyMap->addAction(act);

	act = new Action("SAVE", _("Quick save"));
	act->setCustomEngineActionEvent(kAsylumActionQuickSave);
	act->addDefaultInputMapping("S+s");
	engineKeyMap->addAction(act);

	act = new Action("SARAH", _("Switch to Sarah"));
	act->setCustomEngineActionEvent(kAsylumActionSwitchToSarah);
	act->addDefaultInputMapping("s");
	engineKeyMap->addAction(act);

	act = new Action("GRIMWALL", _("Switch to Grimwall"));
	act->setCustomEngineActionEvent(kAsylumActionSwitchToGrimwall);
	act->addDefaultInputMapping("g");
	engineKeyMap->addAction(act);

	act = new Action("OLMEC", _("Switch to Olmec"));
	act->setCustomEngineActionEvent(kAsylumActionSwitchToOlmec);
	act->addDefaultInputMapping("o");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(ASYLUM)
	REGISTER_PLUGIN_DYNAMIC(ASYLUM, PLUGIN_TYPE_ENGINE, AsylumMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ASYLUM, PLUGIN_TYPE_ENGINE, AsylumMetaEngine);
#endif
