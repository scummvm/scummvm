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

#include "engines/advancedDetector.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "efh/detection.h"
#include "efh/efh.h"

namespace Efh {

#ifdef USE_TTS

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_TTS,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read text in the game (if TTS is available)"),
			"tts_enabled",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

#endif

uint32 EfhEngine::getFeatures() const {
	return _gameDescription->flags;
}

const char *EfhEngine::getGameId() const {
	return _gameDescription->gameId;
}

void EfhEngine::initGame(const ADGameDescription *gd) {
	_platform = gd->platform;
}

bool EfhEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *EfhEngine::getCopyrightString() const {
	return "Escape From Hell (C) Electronic Arts, 1990";
}

Common::Platform EfhEngine::getPlatform() const {
	return _platform;
}
} // End of namespace Efh

namespace Efh {

class EfhMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "efh";
	}

#ifdef USE_TTS
	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Efh::optionsList;
	}
#endif

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	int getMaximumSaveSlot() const override;
	SaveStateList listSaves(const char *target) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error EfhMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	*engine = new EfhEngine(syst, gd);
	((EfhEngine *)*engine)->initGame(gd);
	return Common::kNoError;
}

bool EfhMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate);
}

int EfhMetaEngine::getMaximumSaveSlot() const {
	return 99;
}

SaveStateList EfhMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	char slot[3];
	for (const auto &filename : filenames) {
		slot[0] = filename.c_str()[filename.size() - 2];
		slot[1] = filename.c_str()[filename.size() - 1];
		slot[2] = '\0';
		// Obtain the last 2 digits of the filename (without extension), since they correspond to the save slot
		int slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *file = saveFileMan->openForLoading(filename);
			if (file) {
				uint32 sign = file->readUint32LE();
				uint8 saveVersion = file->readByte();

				if (sign != EFH_SAVE_HEADER || saveVersion > kSavegameVersion) {
					warning("Incompatible savegame");
					delete file;
					continue;
				}

				// read name
				uint16 nameSize = file->readUint16LE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char name[256];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(this, slotNum, name));
				delete file;
			}
		}
	}

	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor EfhMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {
		uint32 sign = file->readUint32LE();
		uint8 saveVersion = file->readByte();

		if (sign != EFH_SAVE_HEADER || saveVersion > kSavegameVersion) {
			warning("Incompatible savegame");
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16LE();
		Common::String saveName;
		for (uint32 i = 0; i < saveNameLength; ++i) {
			char curChr = file->readByte();
			saveName += curChr;
		}

		SaveStateDescriptor desc(this, slot, saveName);

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*file, thumbnail)) {
			delete file;
			return SaveStateDescriptor();
		}
		desc.setThumbnail(thumbnail);

		// Read in save date/time
		int16 year = file->readSint16LE();
		int16 month = file->readSint16LE();
		int16 day = file->readSint16LE();
		int16 hour = file->readSint16LE();
		int16 minute = file->readSint16LE();
		desc.setSaveDate(year, month, day);
		desc.setSaveTime(hour, minute);

		desc.setDeletableFlag(slot != 0);
		desc.setWriteProtectedFlag(slot == 0);

		delete file;
		return desc;
	}
	return SaveStateDescriptor();
}

bool EfhMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

struct EfhSubMenuOption {
	EFHAction _action;
	const char *_id;
	const char *_inputKey;
	const char *_desc;
};

static const EfhSubMenuOption _efhSubMenuOptions[] = {
	{ kActionOption1, "OPTION1", "a", _s("Select option 1") },
	{ kActionOption2, "OPTION2", "b", _s("Select option 2") },
	{ kActionOption3, "OPTION3", "c", _s("Select option 3") },
	{ kActionOption4, "OPTION4", "d", _s("Select option 4") },
	{ kActionOption5, "OPTION5", "e", _s("Select option 5") },
	{ kActionOption6, "OPTION6", "f", _s("Select option 6") },
	{ kActionOption7, "OPTION7", "g", _s("Select option 7") },
	{ kActionOption8, "OPTION8", "h", _s("Select option 8") },
	{ kActionOption9, "OPTION9", "i", _s("Select option 9") },
	{ kActionOption10, "OPTION10", "j", _s("Select option 10") },
	{ kActionOption11, "OPTION11", "k", _s("Select option 11") },
	{ kActionOption12, "OPTION12", "l", _s("Select option 12") },
	{ kActionOption13, "OPTION13", "m", _s("Select option 13") },
	{ kActionOption14, "OPTION14", "n", _s("Select option 14") },

	{ kActionNone, nullptr, nullptr, nullptr }
};

Common::KeymapArray EfhMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "efh-default", _("Default keymappings"));
	Keymap *menuKeymap = new Keymap(Keymap::kKeymapTypeGame, "menu", _("Menu keymappings"));
	Keymap *quitKeymap = new Keymap(Keymap::kKeymapTypeGame, "efh-quit", _("Quit keymappings"));
	Keymap *skipVideoKeymap = new Keymap(Keymap::kKeymapTypeGame, "skip-video", _("Skip video keymappings"));
	Keymap *skipSongKeymap = new Keymap(Keymap::kKeymapTypeGame, "skip-song", _("Skip song keymappings"));
	Keymap *statusMenuNavigationKeymap = new Keymap(Keymap::kKeymapTypeGame, "status-menu-navigation", _("Status menu navigation keymappings"));
	Keymap *statusMenuKeymap = new Keymap(Keymap::kKeymapTypeGame, "status-menu", _("Status menu keymappings"));
	Keymap *statusMenuSubMenuKeymap = new Keymap(Keymap::kKeymapTypeGame, "status-menu-submenu", _("Status menu submenu keymappings"));
	Keymap *interactionKeymap = new Keymap(Keymap::kKeymapTypeGame, "interaction", _("Interaction keymappings"));
	Keymap *fightKeymap = new Keymap(Keymap::kKeymapTypeGame, "fight", _("Fight keymappings"));
	Keymap *characterSelectionKeymap = new Keymap(Keymap::kKeymapTypeGame, "character-selection", _("Character selection keymappings"));
	Keymap *enemySelectionKeymap = new Keymap(Keymap::kKeymapTypeGame, "enemy-selection", _("Enemy selection keymappings"));
	Keymap *deathMenuKeymap = new Keymap(Keymap::kKeymapTypeGame, "death-menu", _("Death menu keymappings"));

	Action *act;

	act = new Action(kStandardActionSave, _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("C+s");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	engineKeymap->addAction(act);

	act = new Action(kStandardActionLoad, _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("F7");
	act->addDefaultInputMapping("C+l");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	engineKeymap->addAction(act);

	act = new Action("MOVEUP", _("Move up"));
	act->setCustomEngineActionEvent(kActionMoveUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("KP8");
	act->addDefaultInputMapping("JOY_UP");
	engineKeymap->addAction(act);

	act = new Action("MOVEDOWN", _("Move down"));
	act->setCustomEngineActionEvent(kActionMoveDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("KP2");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeymap->addAction(act);

	act = new Action("MOVELEFT", _("Move left"));
	act->setCustomEngineActionEvent(kActionMoveLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("KP4");
	act->addDefaultInputMapping("JOY_LEFT");
	engineKeymap->addAction(act);

	act = new Action("MOVERIGHT", _("Move right"));
	act->setCustomEngineActionEvent(kActionMoveRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("KP6");
	act->addDefaultInputMapping("JOY_RIGHT");
	engineKeymap->addAction(act);

	act = new Action("MOVEUPLEFT", _("Move up-left"));
	act->setCustomEngineActionEvent(kActionMoveUpLeft);
	act->addDefaultInputMapping("HOME");
	act->addDefaultInputMapping("KP7");
	engineKeymap->addAction(act);

	act = new Action("MOVEUPRIGHT", _("Move up-right"));
	act->setCustomEngineActionEvent(kActionMoveUpRight);
	act->addDefaultInputMapping("PAGEUP");
	act->addDefaultInputMapping("KP9");
	engineKeymap->addAction(act);

	act = new Action("MOVEDOWNLEFT", _("Move down-left"));
	act->setCustomEngineActionEvent(kActionMoveDownLeft);
	act->addDefaultInputMapping("END");
	act->addDefaultInputMapping("KP1");
	engineKeymap->addAction(act);

	act = new Action("MOVEDOWNRIGHT", _("Move down-right"));
	act->setCustomEngineActionEvent(kActionMoveDownRight);
	act->addDefaultInputMapping("PAGEDOWN");
	act->addDefaultInputMapping("KP3");
	engineKeymap->addAction(act);

	// I18N: (Game name: Escape from hell) Character is a hero in-game
	act = new Action("CHARACTER1STATUS", _("Character 1 status"));
	act->setCustomEngineActionEvent(kActionCharacter1Status);
	act->addDefaultInputMapping("1");
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	// I18N: (Game name: Escape from hell) Character is a hero in-game
	act = new Action("CHARACTER2STATUS", _("Character 2 status"));
	act->setCustomEngineActionEvent(kActionCharacter2Status);
	act->addDefaultInputMapping("2");
	act->addDefaultInputMapping("F2");
	act->addDefaultInputMapping("JOY_X");
	engineKeymap->addAction(act);

	// I18N: (Game name: Escape from hell) Character is a hero in-game
	act = new Action("CHARACTER3STATUS", _("Character 3 status"));
	act->setCustomEngineActionEvent(kActionCharacter3Status);
	act->addDefaultInputMapping("3");
	act->addDefaultInputMapping("F3");
	act->addDefaultInputMapping("JOY_Y");
	engineKeymap->addAction(act);

	act = new Action("YES", _("Yes"));
	act->setCustomEngineActionEvent(kActionYes);
	act->addDefaultInputMapping("y");
	act->addDefaultInputMapping("JOY_A");
	menuKeymap->addAction(act);

	act = new Action("NO", _("No"));
	act->setCustomEngineActionEvent(kActionNo);
	act->addDefaultInputMapping("JOY_B");
	menuKeymap->addAction(act);

	act = new Action("QUIT", _("Quit game"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("C+q");
	act->addDefaultInputMapping("C+x");
	quitKeymap->addAction(act);

	act = new Action("SKIP", _("Skip"));
	act->setCustomEngineActionEvent(kActionSkipVideo);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_A");
	skipVideoKeymap->addAction(act);

	act = new Action("SKIPSONG", _("Skip song"));
	act->setCustomEngineActionEvent(kActionSkipSong);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_A");
	skipSongKeymap->addAction(act);

	act = new Action("SKIPSONGANDINTRO", _("Skip song and intro"));
	act->setCustomEngineActionEvent(kActionSkipSongAndIntro);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	skipSongKeymap->addAction(act);

	act = new Action("SELECT", _("Select"));
	act->setCustomEngineActionEvent(kActionSelect);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_A");
	statusMenuNavigationKeymap->addAction(act);

	act = new Action("SCROLLDOWN", _("Scroll down"));
	act->setCustomEngineActionEvent(kActionScrollDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("2");
	act->addDefaultInputMapping("6");
	act->addDefaultInputMapping("KP2");
	act->addDefaultInputMapping("KP6");
	act->addDefaultInputMapping("JOY_DOWN");
	act->addDefaultInputMapping("JOY_RIGHT");
	statusMenuNavigationKeymap->addAction(act);

	act = new Action("SCROLLUP", _("Scroll up"));
	act->setCustomEngineActionEvent(kActionScrollUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("8");
	act->addDefaultInputMapping("4");
	act->addDefaultInputMapping("KP8");
	act->addDefaultInputMapping("KP4");
	act->addDefaultInputMapping("JOY_UP");
	act->addDefaultInputMapping("JOY_LEFT");
	statusMenuNavigationKeymap->addAction(act);

	act = new Action("ACTIVE", _("Active skills"));
	act->setCustomEngineActionEvent(kActionActive);
	act->addDefaultInputMapping("a");
	statusMenuKeymap->addAction(act);

	act = new Action("DROP", _("Drop"));
	act->setCustomEngineActionEvent(kActionDrop);
	act->addDefaultInputMapping("d");
	statusMenuKeymap->addAction(act);

	act = new Action("EQUIP", _("Equip"));
	act->setCustomEngineActionEvent(kActionEquip);
	act->addDefaultInputMapping("e");
	statusMenuKeymap->addAction(act);

	act = new Action("GIVE", _("Give"));
	act->setCustomEngineActionEvent(kActionGive);
	act->addDefaultInputMapping("g");
	statusMenuKeymap->addAction(act);

	act = new Action("INFO", _("Info"));
	act->setCustomEngineActionEvent(kActionInfo);
	act->addDefaultInputMapping("i");
	statusMenuKeymap->addAction(act);

	act = new Action("PASSIVE", _("Passive skills"));
	act->setCustomEngineActionEvent(kActionPassive);
	act->addDefaultInputMapping("p");
	statusMenuKeymap->addAction(act);

	act = new Action("TRADE", _("Trade"));
	act->setCustomEngineActionEvent(kActionTrade);
	act->addDefaultInputMapping("t");
	statusMenuKeymap->addAction(act);

	act = new Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping("u");
	statusMenuKeymap->addAction(act);

	act = new Action("EXIT", _("Exit status menu"));
	act->setCustomEngineActionEvent(kActionExitStatusMenu);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("JOY_B");
	statusMenuKeymap->addAction(act);

	for (const EfhSubMenuOption *i = _efhSubMenuOptions; i->_id; ++i) {
		act = new Action(i->_id, _(i->_desc));
		act->setCustomEngineActionEvent(i->_action);
		act->addDefaultInputMapping(i->_inputKey);
		statusMenuSubMenuKeymap->addAction(act);
	}

	act = new Action("EXIT", _("Exit submenu"));
	act->setCustomEngineActionEvent(kActionExitSubMenu);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	statusMenuSubMenuKeymap->addAction(act);

	act = new Action("STARTFIGHT", _("Attack"));
	act->setCustomEngineActionEvent(kActionStartFight);
	act->addDefaultInputMapping("a");
	act->addDefaultInputMapping("JOY_UP");
	interactionKeymap->addAction(act);

	act = new Action("LEAVE", _("Leave"));
	act->setCustomEngineActionEvent(kActionLeave);
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_DOWN");
	interactionKeymap->addAction(act);

	act = new Action("STATUS", _("Status"));
	act->setCustomEngineActionEvent(kActionStatus);
	act->addDefaultInputMapping("s");
	act->addDefaultInputMapping("JOY_LEFT");
	interactionKeymap->addAction(act);

	act = new Action("TALK", _("Talk"));
	act->setCustomEngineActionEvent(kActionTalk);
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("JOY_RIGHT");
	interactionKeymap->addAction(act);

	act = new Action("ATTACK", _("Attack"));
	act->setCustomEngineActionEvent(kActionAttack);
	act->addDefaultInputMapping("a");
	act->addDefaultInputMapping("JOY_UP");
	fightKeymap->addAction(act);

	act = new Action("DEFEND", _("Defend"));
	act->setCustomEngineActionEvent(kActionDefend);
	act->addDefaultInputMapping("d");
	act->addDefaultInputMapping("JOY_DOWN");
	fightKeymap->addAction(act);

	act = new Action("HIDE", _("Hide"));
	act->setCustomEngineActionEvent(kActionHide);
	act->addDefaultInputMapping("h");
	act->addDefaultInputMapping("JOY_LEFT");
	fightKeymap->addAction(act);

	act = new Action("RUN", _("Run"));
	act->setCustomEngineActionEvent(kActionRun);
	act->addDefaultInputMapping("r");
	act->addDefaultInputMapping("JOY_RIGHT");
	fightKeymap->addAction(act);

	act = new Action("STATUS", _("Status"));
	act->setCustomEngineActionEvent(kActionTeamStatus);
	act->addDefaultInputMapping("s");
	act->addDefaultInputMapping("JOY_Y");
	fightKeymap->addAction(act);

	// I18N: (Game name: Escape from hell) This action is used to view the terrain during a fight so that the player can see where they can move if they decide to run.
	act = new Action("TERRAIN", _("View terrain"));
	act->setCustomEngineActionEvent(kActionTerrain);
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("JOY_X");
	fightKeymap->addAction(act);

	// I18N: (Game name: Escape from hell) Character is a hero in-game
	act = new Action("CHARACTER1", _("Select character 1"));
	act->setCustomEngineActionEvent(kActionCharacter1);
	act->addDefaultInputMapping("1");
	act->addDefaultInputMapping("JOY_UP");
	characterSelectionKeymap->addAction(act);

	// I18N: (Game name: Escape from hell) Character is a hero in-game
	act = new Action("CHARACTER2", _("Select character 2"));
	act->setCustomEngineActionEvent(kActionCharacter2);
	act->addDefaultInputMapping("2");
	act->addDefaultInputMapping("JOY_RIGHT");
	characterSelectionKeymap->addAction(act);

	// I18N: (Game name: Escape from hell) Character is a hero in-game
	act = new Action("CHARACTER3", _("Select character 3"));
	act->setCustomEngineActionEvent(kActionCharacter3);
	act->addDefaultInputMapping("3");
	act->addDefaultInputMapping("JOY_DOWN");
	characterSelectionKeymap->addAction(act);

	act = new Action("CHARACTERSELECTIONCANCEL", _("Cancel character selection"));
	act->setCustomEngineActionEvent(kActionCancelCharacterSelection);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("0");
	act->addDefaultInputMapping("JOY_B");
	characterSelectionKeymap->addAction(act);

	act = new Action("ENEMY1", _("Select enemy 1"));
	act->setCustomEngineActionEvent(kActionEnemy1);
	act->addDefaultInputMapping("a");
	act->addDefaultInputMapping("JOY_UP");
	enemySelectionKeymap->addAction(act);

	act = new Action("ENEMY2", _("Select enemy 2"));
	act->setCustomEngineActionEvent(kActionEnemy2);
	act->addDefaultInputMapping("b");
	act->addDefaultInputMapping("JOY_RIGHT");
	enemySelectionKeymap->addAction(act);

	act = new Action("ENEMY3", _("Select enemy 3"));
	act->setCustomEngineActionEvent(kActionEnemy3);
	act->addDefaultInputMapping("c");
	act->addDefaultInputMapping("JOY_DOWN");
	enemySelectionKeymap->addAction(act);

	act = new Action("ENEMY4", _("Select enemy 4"));
	act->setCustomEngineActionEvent(kActionEnemy4);
	act->addDefaultInputMapping("d");
	act->addDefaultInputMapping("JOY_LEFT");
	enemySelectionKeymap->addAction(act);

	act = new Action("ENEMY5", _("Select enemy 5"));
	act->setCustomEngineActionEvent(kActionEnemy5);
	act->addDefaultInputMapping("e");
	act->addDefaultInputMapping("JOY_Y");
	enemySelectionKeymap->addAction(act);

	act = new Action("CANCEL", _("Cancel enemy selection"));
	act->setCustomEngineActionEvent(kActionCancelEnemySelection);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	enemySelectionKeymap->addAction(act);

	act = new Action(kStandardActionLoad, _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("JOY_LEFT");
	deathMenuKeymap->addAction(act);

	act = new Action(kStandardActionSave, _("Quit game"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("q");
	act->addDefaultInputMapping("JOY_RIGHT");
	deathMenuKeymap->addAction(act);

	act = new Action("RESET", _("Restart game"));
	act->setCustomEngineActionEvent(kActionreset);
	act->addDefaultInputMapping("r");
	act->addDefaultInputMapping("JOY_UP");
	deathMenuKeymap->addAction(act);

	if (ConfMan.getBool("dump_scripts")) {
		act = new Action("SOUND13", _("Play sound type 13"));
		act->setCustomEngineActionEvent(kActionSound13);
		act->addDefaultInputMapping("4");
		engineKeymap->addAction(act);

		act = new Action("SOUND14", _("Play sound type 14"));
		act->setCustomEngineActionEvent(kActionSound14);
		act->addDefaultInputMapping("5");
		engineKeymap->addAction(act);

		act = new Action("SOUND15", _("Play sound type 15"));
		act->setCustomEngineActionEvent(kActionSound15);
		act->addDefaultInputMapping("6");
		engineKeymap->addAction(act);

		act = new Action("SOUND5", _("Play sound type 5"));
		act->setCustomEngineActionEvent(kActionSound5);
		act->addDefaultInputMapping("7");
		engineKeymap->addAction(act);

		act = new Action("SOUND10", _("Play sound type 10"));
		act->setCustomEngineActionEvent(kActionSound10);
		act->addDefaultInputMapping("8");
		engineKeymap->addAction(act);

		act = new Action("SOUND9", _("Play sound type 9"));
		act->setCustomEngineActionEvent(kActionSound9);
		act->addDefaultInputMapping("9");
		engineKeymap->addAction(act);

		act = new Action("SOUND16", _("Play sound type 16"));
		act->setCustomEngineActionEvent(kActionSound16);
		act->addDefaultInputMapping("0");
		engineKeymap->addAction(act);
	}

	KeymapArray keymap(13);

	keymap[0] = engineKeymap;
	keymap[1] = menuKeymap;
	keymap[2] = quitKeymap;
	keymap[3] = skipVideoKeymap;
	keymap[4] = skipSongKeymap;
	keymap[5] = statusMenuNavigationKeymap;
	keymap[6] = statusMenuKeymap;
	keymap[7] = statusMenuSubMenuKeymap;
	keymap[8] = interactionKeymap;
	keymap[9] = fightKeymap;
	keymap[10] = characterSelectionKeymap;
	keymap[11] = enemySelectionKeymap;
	keymap[12] = deathMenuKeymap;

	menuKeymap->setEnabled(false);
	skipVideoKeymap->setEnabled(false);
	skipSongKeymap->setEnabled(false);
	statusMenuNavigationKeymap->setEnabled(false);
	statusMenuKeymap->setEnabled(false);
	statusMenuSubMenuKeymap->setEnabled(false);
	interactionKeymap->setEnabled(false);
	fightKeymap->setEnabled(false);
	characterSelectionKeymap->setEnabled(false);
	enemySelectionKeymap->setEnabled(false);
	deathMenuKeymap->setEnabled(false);

	return keymap;
}

} // End of namespace Efh

#if PLUGIN_ENABLED_DYNAMIC(EFH)
	REGISTER_PLUGIN_DYNAMIC(EFH, PLUGIN_TYPE_ENGINE, Efh::EfhMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(EFH, PLUGIN_TYPE_ENGINE, Efh::EfhMetaEngine);
#endif
