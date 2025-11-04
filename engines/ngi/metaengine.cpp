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

#include "engines/advancedDetector.h"
#include "engines/metaengine.h"
#include "common/file.h"

#include "graphics/surface.h"

#include "ngi/ngi.h"
#include "ngi/detection.h"
#include "ngi/gameloader.h"

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace NGI {

uint32 NGIEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

bool NGIEngine::isDemo() {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

Common::Language NGIEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

const char *NGIEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

int NGIEngine::getGameGID() const {
	return _gameDescription->gameId;
}

} // End of namspace Fullpipe

class NGIMetaEngine : public AdvancedMetaEngine<NGI::NGIGameDescription> {
public:
	const char *getName() const override {
		return "ngi";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override { return 99; }

	Common::Error createInstance(OSystem *syst, Engine **engine, const NGI::NGIGameDescription *desc) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;

    Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
	    if (saveGameIdx == kSavegameFilePattern) {
	    	// Pattern requested
	    	const char *pattern = "%s.s##";
	    	return Common::String::format(pattern, "fullpipe");
	    } else {
	    	// Specific filename requested
	    	const char *pattern = "%s.s%02d";
	    	return Common::String::format(pattern, "fullpipe", saveGameIdx);
	    }
	}
};

bool NGIMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) || (f == kSupportsLoadingDuringStartup);
}

bool NGI::NGIEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}


Common::Error NGIMetaEngine::createInstance(OSystem *syst, Engine **engine, const NGI::NGIGameDescription *desc) const {
	*engine = new NGI::NGIEngine(syst,desc);
	return Common::kNoError;
}

Common::KeymapArray NGIMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace NGI;

	String extra = ConfMan.get("extra", target);
	const bool isDemo = extra.contains("Demo");

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "ngi-default", _("Default keymappings"));
	Keymap *mapKeymap = new Keymap(Keymap::kKeymapTypeGame, "map", _("Map keymappings"));
	Keymap *introKeymap = new Keymap(Keymap::kKeymapTypeGame, "intro", _("Intro keymappings"));
	Keymap *creditsKeymap = new Keymap(Keymap::kKeymapTypeGame, "credits", _("Credits keymappings"));
	// I18N: Keymap for various dialogs such as save game dialog.
	Keymap *dialogKeymap = new Keymap(Keymap::kKeymapTypeGame, "dialog", _("Dialog keymappings"));

	Common::Action *act;

	act = new Action(kStandardActionLeftClick, _("Move / Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Deselect item"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeymap->addAction(act);

	act = new Action("PAUSE", _("Pause"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	engineKeymap->addAction(act);

	// I18N: Stops all currently playing sounds.
	act = new Action("STOPSOUNDS", _("Stop sounds"));
	act->setCustomEngineActionEvent(kActionStopSounds);
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeymap->addAction(act);

	act = new Action("TOGGLEMUTE", _("Toggle mute"));
	act->setCustomEngineActionEvent(kActionToggleMute);
	act->addDefaultInputMapping("u");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeymap->addAction(act);

	act = new Action("TOGGLEINV", _("Toggle inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_X");
	engineKeymap->addAction(act);

	// I18N: Opens the help screen.
	act = new Action("TOGGLEHELP", _("Help"));
	act->setCustomEngineActionEvent(kActionHelp);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeymap->addAction(act);

	act = new Action("SKIP", _("Skip intro"));
	act->setCustomEngineActionEvent(kActionSkipIntro);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_X");
	introKeymap->addAction(act);

	act = new Action("SKIPCREDITS", _("Skip credits"));
	act->setCustomEngineActionEvent(kActionSkipCredits);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	creditsKeymap->addAction(act);

	// I18N: Closes any open dialogs, such as save game dialog.
	act = new Action("CLOSEDLG", _("Close dialog"));
	act->setCustomEngineActionEvent(kActionCloseDialog);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	dialogKeymap->addAction(act);

	// Demo has no map
	if (!(isDemo && parseLanguage(ConfMan.get("language", target)) == RU_RUS)) {
		act = new Action("OPENMAP", _("Open map"));
		act->setCustomEngineActionEvent(kActionMapOpen);
		act->addDefaultInputMapping("TAB");
		act->addDefaultInputMapping("JOY_RIGHT");
		engineKeymap->addAction(act);

		act = new Action("CLOSEMAP", _("Close map"));
		act->setCustomEngineActionEvent(kActionMapClose);
		act->addDefaultInputMapping("TAB");
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_RIGHT");
		mapKeymap->addAction(act);
	}

	KeymapArray keymaps(5);
	keymaps[0] = engineKeymap;
	keymaps[1] = mapKeymap;
	keymaps[2] = introKeymap;
	keymaps[3] = creditsKeymap;
	keymaps[4] = dialogKeymap;

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(NGI)
	REGISTER_PLUGIN_DYNAMIC(NGI, PLUGIN_TYPE_ENGINE, NGIMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(NGI, PLUGIN_TYPE_ENGINE, NGIMetaEngine);
#endif
