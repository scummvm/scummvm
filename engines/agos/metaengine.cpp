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

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/compression/installshield_cab.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"

#include "agos/intern.h"
#include "agos/agos.h"
#include "agos/detection.h"

namespace AGOS {

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
		GAMEOPTION_OPL3_MODE,
		{
			_s("AdLib OPL3 mode"),
			_s("When AdLib is selected, OPL3 features will be used. Depending on the game, this will prevent cut-off notes, add extra notes or instruments and/or add stereo."),
			"opl3_mode",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DOS_TEMPOS,
		{
			_s("Use DOS version music tempos"),
			_s("Selecting this option will play the music using the tempos used by the DOS version of the game. Otherwise, the faster tempos of the Windows version will be used."),
			"dos_music_tempos",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_WINDOWS_TEMPOS,
		{
			_s("Use DOS version music tempos"),
			_s("Selecting this option will play the music using the tempos used by the DOS version of the game. Otherwise, the faster tempos of the Windows version will be used."),
			"dos_music_tempos",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_PREFER_DIGITAL_SFX,
		{
			_s("Prefer digital sound effects"),
			_s("Prefer digital sound effects instead of synthesized ones"),
			"prefer_digitalsfx",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_DISABLE_FADE_EFFECTS,
		{
			_s("Disable fade-out effects"),
			_s("Don't fade every screen to black when leaving a room."),
			"disable_fade_effects",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace AGOS

class AgosMetaEngine : public AdvancedMetaEngine<AGOS::AGOSGameDescription> {
public:
	const char *getName() const override {
		return "agos";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return AGOS::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const AGOS::AGOSGameDescription *desc) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool AgosMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSimpleSavesNames);
}

bool AGOS::AGOSEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher);
}

Common::Error AgosMetaEngine::createInstance(OSystem *syst, Engine **engine, const AGOS::AGOSGameDescription *gd) const {
	switch (gd->gameType) {
	case AGOS::GType_PN:
		*engine = new AGOS::AGOSEngine_PN(syst, gd);
		break;
	case AGOS::GType_ELVIRA1:
		*engine = new AGOS::AGOSEngine_Elvira1(syst, gd);
		break;
	case AGOS::GType_ELVIRA2:
		*engine = new AGOS::AGOSEngine_Elvira2(syst, gd);
		break;
	case AGOS::GType_WW:
		*engine = new AGOS::AGOSEngine_Waxworks(syst, gd);
		break;
	case AGOS::GType_SIMON1:
		*engine = new AGOS::AGOSEngine_Simon1(syst, gd);
		break;
	case AGOS::GType_SIMON2:
		*engine = new AGOS::AGOSEngine_Simon2(syst, gd);
		break;
#ifdef ENABLE_AGOS2
	case AGOS::GType_FF:
		if (gd->features & AGOS::GF_DEMO)
			*engine = new AGOS::AGOSEngine_FeebleDemo(syst, gd);
		else
			*engine = new AGOS::AGOSEngine_Feeble(syst, gd);
		break;
	case AGOS::GType_PP:
		if (gd->gameId == AGOS::GID_DIMP)
			*engine = new AGOS::AGOSEngine_DIMP(syst, gd);
		else
			*engine = new AGOS::AGOSEngine_PuzzlePack(syst, gd);
		break;
#else
	case AGOS::GType_FF:
	case AGOS::GType_PP:
		return Common::Error(Common::kUnsupportedGameidError, _s("AGOS 2 support is not compiled in"));
#endif
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

SaveStateList AgosMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				saveDesc = file->c_str();
				saveList.push_back(SaveStateDescriptor(this, slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int AgosMetaEngine::getMaximumSaveSlot() const { return 999; }

Common::KeymapArray AgosMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace AGOS;

	Common::String gameId = ConfMan.get("gameid", target);

	// I18N: "AGOS main" refers to the main keymappings for the agos engine.
	// It is never disabled and it is not game specific
	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "agos-main", _("AGOS main"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *yesNoKeymap = new Keymap(Keymap::kKeymapTypeGame, "game-Yes/No", _("Yes/No keymapping"));
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

	act = new Action("EXTCUTSCN", _("Exit cutscene"));
	act->setCustomEngineActionEvent(kActionExitCutscene);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	if (gameId == "simon2" || gameId == "feeble")
		act->addDefaultInputMapping("F5");
	engineKeyMap->addAction(act);

	act = new Action("PAUSE", _("Pause"));
	act->setCustomEngineActionEvent(kActionPause);
	act->addDefaultInputMapping("p");
	gameKeyMap->addAction(act);

	act = new Action("MUSICDOWN", _("Music volume down"));
	act->setCustomEngineActionEvent(kActionMusicDown);
	act->addDefaultInputMapping("MINUS");
	gameKeyMap->addAction(act);

	act = new Action("MUSICUP", _("Music volume up"));
	act->setCustomEngineActionEvent(kActionMusicUp);
	act->addDefaultInputMapping("S+EQUALS");
	act->addDefaultInputMapping("PLUS");
	gameKeyMap->addAction(act);

	act = new Action("MUTEMSC", _("Toggle music on/off"));
	act->setCustomEngineActionEvent(kActionToggleMusic);
	act->addDefaultInputMapping("m");
	gameKeyMap->addAction(act);

	act = new Action("SNDEFFECT", _("Toggle sound effects on/off"));
	act->setCustomEngineActionEvent(kActionToggleSoundEffects);
	act->addDefaultInputMapping("s");
	gameKeyMap->addAction(act);

	act = new Action("FSTMODE", _("Toggle fast mode on/off"));
	act->setCustomEngineActionEvent(kActionToggleFastMode);
	act->addDefaultInputMapping("C+f");
	gameKeyMap->addAction(act);

	if (gameId == "waxworks" ||
			gameId == "elvira1" ||
			gameId == "elvira2" ||
			gameId == "swampy" ||
			gameId == "puzzle" ||
			gameId == "jumble" ||
			gameId == "dimp") {
		act = new Action("WLKFORWARD", _("Walk forward")); // KEYCODE_UP
		act->setCustomEngineActionEvent(kActionWalkForward);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("JOY_UP");
		gameKeyMap->addAction(act);

		act = new Action("TRNBACK", _("Turn backward")); // KEYCODE_DOWN
		act->setCustomEngineActionEvent(kActionTurnBack);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("JOY_DOWN");
		gameKeyMap->addAction(act);

		act = new Action("TRNLEFT", _("Turn left")); // KEYCODE_LEFT
		act->setCustomEngineActionEvent(kActionTurnLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

		act = new Action("TRNRIGHT", _("Turn right")); // KEYCODE_RIGHT
		act->setCustomEngineActionEvent(kActionTurnRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);
	}

	if (gameId == "simon1" || gameId == "simon2") {
		act = new Action("TXTFAST", _("Text speed - Fast"));
		act->setCustomEngineActionEvent(kActionTextSpeedFast);
		act->addDefaultInputMapping("F1");
		gameKeyMap->addAction(act);

		act = new Action("TXTMEDIUM", _("Text speed - Medium"));
		act->setCustomEngineActionEvent(kActionTextSpeedMedium);
		act->addDefaultInputMapping("F2");
		gameKeyMap->addAction(act);

		act = new Action("TXTSLOW", _("Text speed - Slow"));
		act->setCustomEngineActionEvent(kActionTextSpeedSlow);
		act->addDefaultInputMapping("F3");
		gameKeyMap->addAction(act);

		act = new Action("SHOWOBJINTERACT", _("Show objects to interact"));
		act->setCustomEngineActionEvent(kActionShowObjects);
		act->addDefaultInputMapping("F10");
		act->addDefaultInputMapping("JOY_UP");
		gameKeyMap->addAction(act);

		if (gameId == "simon2") {
			act = new Action("BACKGRNDSND", _("Toggle background sounds on/off"));
			act->setCustomEngineActionEvent(kActionToggleBackgroundSound);
			act->addDefaultInputMapping("b");
			gameKeyMap->addAction(act);
		}
	}

	if (gameId == "feeble") {
		// I18N: Characters are game actors
		act = new Action("SWTCHCHARACTER", _("Switch characters"));
		act->setCustomEngineActionEvent(kActionToggleSwitchCharacter);
		act->addDefaultInputMapping("F7");
		act->addDefaultInputMapping("JOY_X");
		gameKeyMap->addAction(act);

		act = new Action("TOGGLEHITBOX", _("Toggle hitbox names on/off"));
		act->setCustomEngineActionEvent(kActionToggleHitboxName);
		act->addDefaultInputMapping("F9");
		gameKeyMap->addAction(act);
	}

	if (gameId == "feeble" || gameId == "simon2") {
		act = new Action("TOGGLESUB", _("Switches between speech only and combined speech and subtitles"));
		act->setCustomEngineActionEvent(kActionToggleSubtitle);
		act->addDefaultInputMapping("t");
		act->addDefaultInputMapping("JOY_LEFT");
		gameKeyMap->addAction(act);

		act = new Action("TOGGLESPEECH", _("Switches between subtitles only and combined speech and subtitles"));
		act->setCustomEngineActionEvent(kActionToggleSpeech);
		act->addDefaultInputMapping("v");
		act->addDefaultInputMapping("JOY_RIGHT");
		gameKeyMap->addAction(act);
	}

	if (gameId == "swampy" ||
			gameId == "puzzle" ||
			gameId == "jumble") {
		act = new Action("HIGHSPEED", _("High speed mode on/off in Swampy Adventures"));
		act->setCustomEngineActionEvent(kActionSpeed_GTYPEPP);
		act->addDefaultInputMapping("F12");
		gameKeyMap->addAction(act);
	}

	act = new Action("KEYYES", _("Press \"Yes\" key"));
	act->setCustomEngineActionEvent(kActionKeyYes);
	act->addDefaultInputMapping("JOY_A");
	yesNoKeymap->addAction(act);

	act = new Action("KEYNO", _("Press \"No\" key"));
	act->setCustomEngineActionEvent(kActionKeyNo);
	act->addDefaultInputMapping("JOY_B");
	yesNoKeymap->addAction(act);

	KeymapArray keymaps(3);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;
	keymaps[2] = yesNoKeymap;

	yesNoKeymap->setEnabled(false);
	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(AGOS)
	REGISTER_PLUGIN_DYNAMIC(AGOS, PLUGIN_TYPE_ENGINE, AgosMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(AGOS, PLUGIN_TYPE_ENGINE, AgosMetaEngine);
#endif

namespace AGOS {

int AGOSEngine::getGameId() const {
	return _gameDescription->gameId;
}

int AGOSEngine::getGameType() const {
	return _gameDescription->gameType;
}

uint32 AGOSEngine::getFeatures() const {
	return _gameDescription->features;
}

const char *AGOSEngine::getExtra() const {
	return _gameDescription->desc.extra;
}

Common::Language AGOSEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform AGOSEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

const char *AGOSEngine::getFileName(int type) const {
	// Required if the InstallShield cab is been used
	if (getGameType() == GType_PP) {
		if (type == GAME_BASEFILE)
			return gss->base_filename;
	}

	// Required if the InstallShield cab is been used
	if (getGameType() == GType_FF && getPlatform() == Common::kPlatformWindows) {
		if (type == GAME_BASEFILE)
			return gss->base_filename;
		if (type == GAME_RESTFILE)
			return gss->restore_filename;
		if (type == GAME_TBLFILE)
			return gss->tbl_filename;
	}

	for (int i = 0; _gameDescription->desc.filesDescriptions[i].fileType; i++) {
		if (_gameDescription->desc.filesDescriptions[i].fileType == type)
			return _gameDescription->desc.filesDescriptions[i].fileName;
	}
	return nullptr;
}

#ifdef ENABLE_AGOS2
void AGOSEngine::loadArchives() {
	const ADGameFileDescription *ag;

	if (getFeatures() & GF_PACKED) {
		for (ag = _gameDescription->desc.filesDescriptions; ag->fileName; ag++) {
			if (ag->fileType != GAME_CABFILE)
				continue;

			if (!SearchMan.hasArchive(ag->fileName)) {
				// Assumes the cabinet file is named data1.cab
				Common::Archive *cabinet = Common::makeInstallShieldArchive("data");
				if (cabinet)
					SearchMan.add(ag->fileName, cabinet);
			}
		}
	}
}
#endif

} // End of namespace AGOS
