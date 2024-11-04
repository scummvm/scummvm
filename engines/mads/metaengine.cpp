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
 *
 */

#include "mads/mads.h"

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/surface.h"

#include "mads/events.h"
#include "mads/game.h"
#include "mads/detection.h"

#define MAX_SAVES 99

namespace MADS {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Enable copy protection"),
			_s("Enable any copy protection that would otherwise be bypassed by default."),
			"copy_protection",
			false,
			0,
			0
		},
	},

	{
		GAMEOPTION_EASY_MOUSE,
		{
			_s("Easy mouse interface"),
			_s("Shows object names when hovering the mouse over them"),
			"EasyMouse",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_ANIMATED_INVENTORY,
		{
			_s("Animated inventory items"),
			_s("Animated inventory items"),
			"InvObjectsAnimated",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_ANIMATED_INTERFACE,
		{
			_s("Animated game interface"),
			_s("Animated game interface"),
			"TextWindowAnimated",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_NAUGHTY_MODE,
		{
			_s("Naughty game mode"),
			_s("Naughty game mode"),
			"NaughtyMode",
			true,
			0,
			0
		}
	},

	/*{
		GAMEOPTION_GRAPHICS_DITHERING,
		{
			_s("Graphics dithering"),
			_s("Graphics dithering"),
			"GraphicsDithering",
			true,
			0,
			0
		}
	},*/

#ifdef USE_TTS
	{
		GAMEOPTION_TTS_NARRATOR,
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

uint32 MADSEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 MADSEngine::getGameFeatures() const {
	return _gameDescription->features;
}

uint32 MADSEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

bool MADSEngine::isDemo() const {
	return (bool)(_gameDescription->desc.flags & ADGF_DEMO);
}

Common::Language MADSEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform MADSEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

} // End of namespace MADS

class MADSMetaEngine : public AdvancedMetaEngine<MADS::MADSGameDescription> {
public:
	const char *getName() const override {
		return "mads";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return MADS::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const MADS::MADSGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool MADSMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSimpleSavesNames);
}

bool MADS::MADSEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error MADSMetaEngine::createInstance(OSystem *syst, Engine **engine, const MADS::MADSGameDescription *desc) const {
	*engine = new MADS::MADSEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList MADSMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = Common::String::format("%s.0##", target);
	MADS::MADSSavegameHeader header;

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const char *ext = strrchr(file->c_str(), '.');
		int slot = ext ? atoi(ext + 1) : -1;

		if (slot >= 0 && slot < MAX_SAVES) {
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);

			if (in) {
				if (MADS::Game::readSavegameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(this, slot, header._saveName));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int MADSMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

bool MADSMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor MADSMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		MADS::MADSSavegameHeader header;
		if (!MADS::Game::readSavegameHeader(f, header, false)) {
			delete f;
			return SaveStateDescriptor();
		}
		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, header._saveName);
		desc.setThumbnail(header._thumbnail);
		desc.setSaveDate(header._year, header._month, header._day);
		desc.setSaveTime(header._hour, header._minute);
		desc.setPlayTime(header._totalFrames * GAME_FRAME_TIME);

		return desc;
	}

	return SaveStateDescriptor();
}

Common::KeymapArray MADSMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace MADS;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "mads-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *menuKeyMap = new Keymap(Keymap::kKeymapTypeGame, "menu-shortcuts", _("Start menu keymappings"));

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

	act = new Action("ESCAPE", _("Escape"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	engineKeyMap->addAction(act);

	{
		act = new Action("GAMEMENU", _("Open game menu"));
		act->setCustomEngineActionEvent(kActionGameMenu);
		act->addDefaultInputMapping("F1");
		act->addDefaultInputMapping("JOY_Y");
		gameKeyMap->addAction(act);

		act = new Action("SAVE", _("Save game"));
		act->setCustomEngineActionEvent(kActionSave);
		act->addDefaultInputMapping("F5");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("RESTORE", _("Restore game"));
		act->setCustomEngineActionEvent(kActionRestore);
		act->addDefaultInputMapping("F7");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		gameKeyMap->addAction(act);

		act = new Action("SCROLLUP", _("Scroll bar up"));
		act->setCustomEngineActionEvent(kActionScrollUp);
		act->addDefaultInputMapping("PAGEUP");
		act->addDefaultInputMapping("JOY_UP");
		gameKeyMap->addAction(act);

		act = new Action("SCROLLDN", _("Scroll bar down"));
		act->setCustomEngineActionEvent(kActionScrollDown);
		act->addDefaultInputMapping("PAGEDOWN");
		act->addDefaultInputMapping("JOY_DOWN");
		gameKeyMap->addAction(act);
	}

	{
		act = new Action("START", _("Start game"));
		act->setCustomEngineActionEvent(kActionStartGame);
		act->addDefaultInputMapping("F1");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		menuKeyMap->addAction(act);

		act = new Action("RESUME", _("Resume game"));
		act->setCustomEngineActionEvent(kActionResumeGame);
		act->addDefaultInputMapping("F2");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		menuKeyMap->addAction(act);

		act = new Action("INTRO", _("Show intro"));
		act->setCustomEngineActionEvent(kActionShowIntro);
		act->addDefaultInputMapping("F3");
		act->addDefaultInputMapping("JOY_LEFT");
		menuKeyMap->addAction(act);

		act = new Action("CREDITS", _("Show credits"));
		act->setCustomEngineActionEvent(kActionCredits);
		act->addDefaultInputMapping("F4");
		act->addDefaultInputMapping("JOY_UP");
		menuKeyMap->addAction(act);

		act = new Action("QUOTES", _("Show quotes"));
		act->setCustomEngineActionEvent(kActionQuotes);
		act->addDefaultInputMapping("F5");
		act->addDefaultInputMapping("JOY_RIGHT");
		menuKeyMap->addAction(act);

		act = new Action("EXIT", _("Exit game"));
		act->setCustomEngineActionEvent(kActionEscape);
		act->addDefaultInputMapping("F6");
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_X");
		menuKeyMap->addAction(act);

		act = new Action("RESTARTANIM", _("Restart animation"));
		act->setCustomEngineActionEvent(kActionRestartAnimation);
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_Y");
		menuKeyMap->addAction(act);
	}

	KeymapArray keymaps(3);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;
	keymaps[2] = menuKeyMap;

	menuKeyMap->setEnabled(false);

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(MADS)
	REGISTER_PLUGIN_DYNAMIC(MADS, PLUGIN_TYPE_ENGINE, MADSMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MADS, PLUGIN_TYPE_ENGINE, MADSMetaEngine);
#endif
