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

#include "groovie/groovie.h"
#include "groovie/saveload.h"

#include "common/system.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"
#include "groovie/detection.h"

namespace Groovie {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_T7G_FAST_MOVIE_SPEED,
		{
			_s("Fast movie speed"),
			_s("Play movies at an increased speed"),
			"fast_movie_speed",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_EASIER_AI,
		{
			_s("Easier AI"),
			_s("Decrease the difficulty of AI puzzles"),
			"easier_ai",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_EASIER_AI_DEFAULT,
		{
			_s("Easier AI"),
			_s("Decrease the difficulty of AI puzzles"),
			"easier_ai",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_FINAL_HOUR,
		{
			_s("Updated Credits Music"),
			_s("Play the song The Final Hour during the credits instead of reusing MIDI songs"),
			"credits_music",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_SLIMHOTSPOTS,
		{
			_s("Slim Left/Right Hotspots"),
			_s("Shrinks the hotspots on the left and right sides for exiting puzzles"),
			"slim_hotspots",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_SPEEDRUN,
		{
			_s("Speedrun Mode"),
			_s("Affects the controls for fast forwarding the game"),
			"speedrun_mode",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class GroovieMetaEngine : public AdvancedMetaEngine<GroovieGameDescription> {
public:
	const char *getName() const override {
		return "groovie";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const GroovieGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	int getAutosaveSlot() const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error GroovieMetaEngine::createInstance(OSystem *syst, Engine **engine, const GroovieGameDescription *gd) const {
#ifndef ENABLE_GROOVIE2
	if (gd->version != kGroovieT7G)
		return Common::Error(Common::kUnsupportedGameidError, _s("GroovieV2 support is not compiled in"));
#endif

	*engine = new GroovieEngine(syst,gd);
	return Common::kNoError;
}

bool GroovieMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesSupportMetaInfo);
}

SaveStateList GroovieMetaEngine::listSaves(const char *target) const {
	return SaveLoad::listValidSaves(target);
}

int GroovieMetaEngine::getMaximumSaveSlot() const {
	return SaveLoad::getMaximumSlot();
}

bool GroovieMetaEngine::removeSaveState(const char *target, int slot) const {
	if (!SaveLoad::isSlotValid(slot)) {
		// Invalid slot, do nothing
		return false;
	}

	Common::String filename = SaveLoad::getSlotSaveName(target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor GroovieMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc;

	Common::InSaveFile *savefile = SaveLoad::openForLoading(target, slot, &desc);
	delete savefile;

	return desc;
}

int GroovieMetaEngine::getAutosaveSlot() const {
	return GroovieEngine::AUTOSAVE_SLOT;
}

Common::KeymapArray GroovieMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Groovie;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "groovie-engine", _("Groovie engine"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("SKIPORFAST", _("Skip or fast forward scene"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("SPACE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	return Keymap::arrayOf(engineKeyMap);
}

} // End of namespace Groovie

#if PLUGIN_ENABLED_DYNAMIC(GROOVIE)
	REGISTER_PLUGIN_DYNAMIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#endif
