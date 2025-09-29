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
#include "backends/keymapper/standard-actions.h"

#include "darkseed/metaengine.h"
#include "darkseed/detection.h"
#include "darkseed/darkseed.h"
#include "darkseed/dialogs.h"

const char *DarkseedMetaEngine::getName() const {
	return "darkseed";
}

Common::KeymapArray DarkseedMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "darkseed", "Darkseed");

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Action"));
	act->setCustomEngineActionEvent(Darkseed::kDarkseedActionSelect);
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action("CHANGECOMMAND", _("Change command"));
	act->setCustomEngineActionEvent(Darkseed::kDarkseedActionChangeCommand);
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("TIMEADVANCE", _("Time advance"));
	act->setCustomEngineActionEvent(Darkseed::kDarkseedActionTimeAdvance);
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("JOY_Y");
	engineKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit game"));
	act->setCustomEngineActionEvent(Darkseed::kDarkseedActionQuit);
	act->addDefaultInputMapping("C+q");
	engineKeyMap->addAction(act);

	act = new Action("SKIPCUTSCENE", _("Skip Cutscene"));
	act->setCustomEngineActionEvent(Darkseed::kDarkseedActionSkipCutscene);
	act->addDefaultInputMapping("SPACE");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

Common::Error DarkseedMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Darkseed::DarkseedEngine(syst, desc);
	return Common::kNoError;
}

bool DarkseedMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

void DarkseedMetaEngine::registerDefaultSettings(const Common::String &target) const {
	for (const ADExtraGuiOptionsMap *entry = Darkseed::optionsList; entry->guioFlag; ++entry)
		ConfMan.registerDefault(entry->option.configOption, entry->option.defaultState);

	for (const Darkseed::PopUpOptionsMap *entry = Darkseed::popUpOptionsList; entry->guioFlag; ++entry)
		ConfMan.registerDefault(entry->configOption, entry->defaultState);
}

GUI::OptionsContainerWidget *DarkseedMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new Darkseed::OptionsWidget(boss, name, target);
}

#if PLUGIN_ENABLED_DYNAMIC(DARKSEED)
	REGISTER_PLUGIN_DYNAMIC(DARKSEED, PLUGIN_TYPE_ENGINE, DarkseedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DARKSEED, PLUGIN_TYPE_ENGINE, DarkseedMetaEngine);
#endif
