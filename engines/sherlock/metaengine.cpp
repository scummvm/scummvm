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

#include "sherlock/sherlock.h"
#include "sherlock/saveload.h"
#include "sherlock/scalpel/scalpel.h"
#include "sherlock/tattoo/tattoo.h"

#include "common/system.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"

#include "sherlock/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "sherlock/fixed_text.h"
#include "sherlock/scalpel/scalpel_fixed_text.h"
//#include "sherlock/scalpel/scalpel_fixed_text_tables.h"

namespace Sherlock {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVES,
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
		GAMEOPTION_FADE_STYLE,
		{
			_s("Pixellated scene transitions"),
			_s("When changing scenes, a randomized pixel transition is done"),
			"fade_style",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_HELP_STYLE,
		{
			_s("Don't show hotspots when moving mouse"),
			_s("Only show hotspot names after you actually click on a hotspot or action button"),
			"help_style",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_PORTRAITS_ON,
		{
			_s("Show character portraits"),
			_s("Show portraits for the characters when conversing"),
			"portraits_on",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_WINDOW_STYLE,
		{
			_s("Slide dialogs into view"),
			_s("Slide UI dialogs into view, rather than simply showing them immediately"),
			"window_style",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_TRANSPARENT_WINDOWS,
		{
			_s("Transparent windows"),
			_s("Show windows with a partially transparent background"),
			"transparent_windows",
			true,
			0,
			0
		}
	},

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

GameType SherlockEngine::getGameID() const {
	return _gameDescription->gameID;
}

Common::Platform SherlockEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

Common::Language SherlockEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

static const SHERLOCKActions fileActions[] = {
	kActionScalpelFiles1,
	kActionScalpelFiles2,
	kActionScalpelFiles3,
	kActionScalpelFiles4,
	kActionScalpelFiles5,
	kActionScalpelFiles6,
	kActionScalpelFiles7,
	kActionScalpelFiles8,
	kActionScalpelFiles9
};

static const char *fileIds[] = {
	"FILE1",
	"FILE2",
	"FILE3",
	"FILE4",
	"FILE5",
	"FILE6",
	"FILE7",
	"FILE8",
	"FILE9"
};

} // End of namespace Sherlock


class SherlockMetaEngine : public AdvancedMetaEngine<Sherlock::SherlockGameDescription> {
public:
	const char *getName() const override {
		return "sherlock";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Sherlock::optionsList;
	}

	/**
	 * Creates an instance of the game engine
	 */
	Common::Error createInstance(OSystem *syst, Engine **engine, const Sherlock::SherlockGameDescription *desc) const override;

	/**
	 * Returns a list of features the game's MetaEngine support
	 */
	bool hasFeature(MetaEngineFeature f) const override;

	/**
	 * Return a list of savegames
	 */
	SaveStateList listSaves(const char *target) const override;

	/**
	 * Returns the maximum number of allowed save slots
	 */
	int getMaximumSaveSlot() const override;

	/**
	 * Deletes a savegame in the specified slot
	 */
	bool removeSaveState(const char *target, int slot) const override;

	/**
	 * Given a specified savegame slot, returns extended information for the save
	 */
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	/**
	 * Returns keymaps for the game
	 */
	Common::KeymapArray initKeymaps(const char *target) const override;
};

Common::Error SherlockMetaEngine::createInstance(OSystem *syst, Engine **engine, const Sherlock::SherlockGameDescription *gd) const {
	switch (gd->gameID) {
	case Sherlock::GType_SerratedScalpel:
		*engine = new Sherlock::Scalpel::ScalpelEngine(syst, gd);
		break;
	case Sherlock::GType_RoseTattoo:
		*engine = new Sherlock::Tattoo::TattooEngine(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

bool SherlockMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSimpleSavesNames);
}

bool Sherlock::SherlockEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool Sherlock::SherlockEngine::isDemo() const {
	return _gameDescription->desc.flags & ADGF_DEMO;
}

SaveStateList SherlockMetaEngine::listSaves(const char *target) const {
	return Sherlock::SaveManager::getSavegameList(this, target);
}

int SherlockMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVEGAME_SLOTS;
}

bool SherlockMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Sherlock::SaveManager(nullptr, target).generateSaveName(slot);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor SherlockMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Sherlock::SaveManager(nullptr, target).generateSaveName(slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {
		Sherlock::SherlockSavegameHeader header;
		if (!Sherlock::SaveManager::readSavegameHeader(f, header, false)) {
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

Common::String getString(int fixedTextId, const Sherlock::Scalpel::FixedTextLanguageEntry *curLanguageEntry, const char *target) {

	Common::String str = curLanguageEntry->fixedTextArray[fixedTextId];
	str.toLowercase();

	return str;
}

Common::String getKey(int fixedTextId, const Sherlock::Scalpel::FixedTextLanguageEntry *curLanguageEntry, const char *target, int pos = 0) {

	Common::String str = getString(fixedTextId, curLanguageEntry, target);

	return str.substr(pos, 1);
}

Common::KeymapArray SherlockMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Sherlock;

	KeymapArray keymaps;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "sherlock-default", _("Default keymappings"));

	Action *act;

	if (ConfMan.get("gameid", target) == "scalpel") {
		using namespace Scalpel;

		Language curLanguage = parseLanguage(ConfMan.get("language", target));
		const FixedTextLanguageEntry *curLanguageEntry = fixedTextLanguages;

		while (curLanguageEntry->language != UNK_LANG) {
			if (curLanguageEntry->language == curLanguage)
				break; // found current language
			curLanguageEntry++;
		}

		Keymap *scalpelQuitKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-quit", _("Quit keymappings"));
		Keymap *animationKeymap = new Keymap(Keymap::kKeymapTypeGame, "animation", _("Animation keymappings"));
		Keymap *scalpelScrollKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-scroll", _("Scroll keymappings"));
		Keymap *scalpelKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel", _("Main user interface keymappings"));
		Keymap *scalpelDartsKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-darts", _("Darts minigame keymappings"));
		Keymap *scalpelJournalKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-journal", _("Journal keymappings"));
		Keymap *scalpelTalkKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-talk", _("Talk keymappings"));
		Keymap *scalpelInvKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-inv", _("Inventory keymappings"));
		Keymap *scalpelFilesKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-files", _("Files keymappings"));
		Keymap *scalpelQuitDialogKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-quit-dialog", _("Quit dialog keymappings"));
		Keymap *scalpelMapKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-map", _("Map keymappings"));
		Keymap *scalpelSettingsKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-settings", _("Settings keymappings"));
		Keymap *scalpel3D0MovieKeymap = new Keymap(Keymap::kKeymapTypeGame, "scalpel-3d0-movie", _("Movie keymappings"));

		act = new Common::Action(kStandardActionLeftClick, _("Move / Interact / Select"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		engineKeymap->addAction(act);

		act = new Common::Action(kStandardActionRightClick, _("Skip / Deselect verb"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		engineKeymap->addAction(act);

		//
		// Quit keymap
		//

		act = new Action("QUIT", _("Quit game"));
		act->setCustomEngineActionEvent(kActionScalpelQuit);
		act->addDefaultInputMapping("A+x");
		scalpelQuitKeymap->addAction(act);

		//
		// Scroll keymap
		//

		act = new Action("SCROLLUP", _("Scroll up")); // scroll up in journal, talk, and files UI all use the same key in all the languages hence using the same action for all three.
		act->setCustomEngineActionEvent(kActionScalpelScrollUp);
		act->addDefaultInputMapping(getKey(kFixedText_Window_Up, curLanguageEntry, target));
		act->addDefaultInputMapping("JOY_UP");
		scalpelScrollKeymap->addAction(act);

		act = new Action("SCROLLDOWN", _("Scroll down"));  // scroll down in journal, talk, and files UI all use the same key in all the languages hence using the same action for all three.
		act->setCustomEngineActionEvent(kActionScalpelScrollDown);
		act->addDefaultInputMapping(getKey(kFixedText_Window_Down, curLanguageEntry, target));
		act->addDefaultInputMapping("JOY_DOWN");
		scalpelScrollKeymap->addAction(act);

		//
		// Animation keymap
		//

		act = new Action("SKIP", _("Skip"));
		act->setCustomEngineActionEvent(kActionSkipAnim);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_Y");
		animationKeymap->addAction(act);

		//
		// Darts minigame keymap
		//

		act = new Action("EXIT", _("Exit"));
		act->setCustomEngineActionEvent(kActionScalpelDartsExit);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_B");
		scalpelDartsKeymap->addAction(act);

		//
		// Scalpel main Game keymap
		//

		act = new Action("LOOK", _("Look"));
		act->setCustomEngineActionEvent(kActionScalpelLook);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 0));
		scalpelKeymap->addAction(act);

		act = new Action("MOVE", _("Move"));
		act->setCustomEngineActionEvent(kActionScalpelMove);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 1));
		scalpelKeymap->addAction(act);

		act = new Action("TALK", _("Talk"));
		act->setCustomEngineActionEvent(kActionScalpelTalk);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 2));
		scalpelKeymap->addAction(act);

		act = new Action("PICKUP", _("Pick up"));
		act->setCustomEngineActionEvent(kActionScalpelPickUp);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 3));
		scalpelKeymap->addAction(act);

		act = new Action("OPEN", _("Open"));
		act->setCustomEngineActionEvent(kActionScalpelOpen);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 4));
		scalpelKeymap->addAction(act);

		act = new Action("CLOSE", _("Close"));
		act->setCustomEngineActionEvent(kActionScalpelClose);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 5));
		scalpelKeymap->addAction(act);

		act = new Action("INVENTORY", _("Inventory"));
		act->setCustomEngineActionEvent(kActionScalpelInventory);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 6));
		scalpelKeymap->addAction(act);

		act = new Action("USE", _("Use"));
		act->setCustomEngineActionEvent(kActionScalpelUse);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 7));
		scalpelKeymap->addAction(act);

		act = new Action("GIVE", _("Give"));
		act->setCustomEngineActionEvent(kActionScalpelGive);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 8));
		scalpelKeymap->addAction(act);

		act = new Action("SETUP", _("Setup"));
		act->setCustomEngineActionEvent(kActionScalpelSetup);
		act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 11));
		scalpelKeymap->addAction(act);

		// 3DO version has different actions in the main UI
		if (parsePlatform(ConfMan.get("platform")) == kPlatform3DO) {
			act = new Action("LOAD", _("Load game"));
			act->setCustomEngineActionEvent(kActionScalpelLoad);
			act->addDefaultInputMapping("a");
			scalpelKeymap->addAction(act);

			act = new Action("SAVE", _("Save game"));
			act->setCustomEngineActionEvent(kActionScalpelSave);
			act->addDefaultInputMapping("v");
			scalpelKeymap->addAction(act);
		} else {
			act = new Action("JOURNAL", _("Journal"));
			act->setCustomEngineActionEvent(kActionScalpelJournal);
			act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 9));
			scalpelKeymap->addAction(act);

			act = new Action("FILES", _("Files"));
			act->setCustomEngineActionEvent(kActionScalpelFiles);
			act->addDefaultInputMapping(getKey(kFixedText_Game_Hotkeys, curLanguageEntry, target, 10));
			scalpelKeymap->addAction(act);
		}

		//
		// 3DO Movie keymap
		//

		if (parsePlatform(ConfMan.get("platform")) == kPlatform3DO) {
			act = new Action("SKIPMOVIE", _("Skip movie"));
			act->setCustomEngineActionEvent(kActionScalpelSkipMovie);
			act->addDefaultInputMapping("ESCAPE");
			act->addDefaultInputMapping("JOY_Y");
			scalpel3D0MovieKeymap->addAction(act);
		}

		//
		// Journal keymap
		//

		act = new Action("JOURNALEXIT", _("Exit"));
		act->setCustomEngineActionEvent(kActionScalpelJournalExit);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping(getKey(kFixedText_Journal_Exit, curLanguageEntry, target));
		scalpelJournalKeymap->addAction(act);

		// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has a journal, this action is used to go back 10 pages in the journal
		act = new Action("BACK10", _("Go back 10 pages"));
		act->setCustomEngineActionEvent(kActionScalpelJournalBack10);
		act->addDefaultInputMapping(getKey(kFixedText_Journal_Back10, curLanguageEntry, target));
		scalpelJournalKeymap->addAction(act);

		// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has a journal, this action is used to go forward 10 pages in the journal
		act = new Action("AHEAD10", _("Go forward 10 pages"));
		act->setCustomEngineActionEvent(kActionScalpelJournalAhead10);
		act->addDefaultInputMapping(getKey(kFixedText_Journal_Ahead10, curLanguageEntry, target));
		act->addDefaultInputMapping("JOY_RIGHT");
		scalpelJournalKeymap->addAction(act);

		// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has a journal, this action is used to search the journal
		act = new Action("SEARCH", _("Search"));
		act->setCustomEngineActionEvent(kActionScalpelJournalSearch);
		act->addDefaultInputMapping(getKey(kFixedText_Journal_Search, curLanguageEntry, target));
		act->addDefaultInputMapping("JOY_X");
		scalpelJournalKeymap->addAction(act);

		// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has a journal, this action is used to go to the first page of the journal
		act = new Action("FIRST_PAGE", _("First page"));
		act->setCustomEngineActionEvent(kActionScalpelJournalFirstPage);
		act->addDefaultInputMapping(getKey(kFixedText_Journal_FirstPage, curLanguageEntry, target));
		act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
		scalpelJournalKeymap->addAction(act);

		// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has a journal, this action is used to go to the last page of the journal
		act = new Action("LAST_PAGE", _("Last page"));
		act->setCustomEngineActionEvent(kActionScalpelJournalLastPage);
		act->addDefaultInputMapping(getKey(kFixedText_Journal_LastPage, curLanguageEntry, target));
		act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
		scalpelJournalKeymap->addAction(act);

		//
		// Talk keymap
		//

		act = new Action("TALKEXIT", _("Exit"));
		act->setCustomEngineActionEvent(kActionScalpelTalkExit);
		act->addDefaultInputMapping(getKey(kFixedText_Window_Exit, curLanguageEntry, target));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		scalpelTalkKeymap->addAction(act);

		//
		// Inventory keymap
		//

		act = new Action("INVEXIT", _("Exit"));
		act->setCustomEngineActionEvent(kActionScalpelInvExit);
		act->addDefaultInputMapping(getKey(kFixedText_Inventory_Exit, curLanguageEntry, target));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		scalpelInvKeymap->addAction(act);

		act = new Action("LOOK", _("Look"));
		act->setCustomEngineActionEvent(kActionScalpelInvLook);
		act->addDefaultInputMapping(getKey(kFixedText_Inventory_Look, curLanguageEntry, target));
		scalpelInvKeymap->addAction(act);

		act = new Action("USE", _("Use"));
		act->setCustomEngineActionEvent(kActionScalpelInvUse);
		act->addDefaultInputMapping(getKey(kFixedText_Inventory_Use, curLanguageEntry, target));
		scalpelInvKeymap->addAction(act);

		act = new Action("GIVE", _("Give"));
		act->setCustomEngineActionEvent(kActionScalpelInvGive);
		act->addDefaultInputMapping(getKey(kFixedText_Inventory_Give, curLanguageEntry, target));
		scalpelInvKeymap->addAction(act);

		// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has an inventory, this action is go back a page (one page displays 6 items) in the inventory
		act = new Action("PAGE_LEFT", _("Previous page"));
		act->setCustomEngineActionEvent(kActionScalpelInvPageLeft);
		act->addDefaultInputMapping("COMMA");
		act->addDefaultInputMapping("JOY_UP");
		scalpelInvKeymap->addAction(act);

		act = new Action("LEFT", _("Scroll left"));
		act->setCustomEngineActionEvent(kActionScalpelInvLeft);
		act->addDefaultInputMapping("MINUS");
		act->addDefaultInputMapping("JOY_LEFT");
		scalpelInvKeymap->addAction(act);

		act = new Action("RIGHT", _("Scroll right"));
		act->setCustomEngineActionEvent(kActionScalpelInvRight);
		act->addDefaultInputMapping("PLUS");
		act->addDefaultInputMapping("JOY_RIGHT");
		scalpelInvKeymap->addAction(act);

		// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has an inventory, this action is go ahead a page (one page displays 6 items) in the inventory
		act = new Action("PAGE_RIGHT", _("Next page"));
		act->setCustomEngineActionEvent(kActionScalpelInvPageRight);
		act->addDefaultInputMapping("PERIOD");
		act->addDefaultInputMapping("JOY_DOWN");
		scalpelInvKeymap->addAction(act);

		//
		// Files keymap
		//

		act = new Action("EXIT", _("Exit"));
		act->setCustomEngineActionEvent(kActionScalpelFilesExit);
		act->addDefaultInputMapping(getKey(kFixedText_LoadSave_Exit, curLanguageEntry, target));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		scalpelFilesKeymap->addAction(act);

		act = new Action("LOAD", _("Load game"));
		act->setCustomEngineActionEvent(kActionScalpelFilesLoad);
		act->addDefaultInputMapping(getKey(kFixedText_LoadSave_Load, curLanguageEntry, target));
		scalpelFilesKeymap->addAction(act);

		act = new Action("SAVE", _("Save game"));
		act->setCustomEngineActionEvent(kActionScalpelFilesSave);
		act->addDefaultInputMapping(getKey(kFixedText_LoadSave_Save, curLanguageEntry, target));
		scalpelFilesKeymap->addAction(act);

		act = new Action("QUIT", _("Quit game"));
		act->setCustomEngineActionEvent(kActionScalpelFilesQuit);
		act->addDefaultInputMapping(getKey(kFixedText_LoadSave_Quit, curLanguageEntry, target));
		scalpelFilesKeymap->addAction(act);

		String fileLabel[9];

		for (int i = 0; i < 9; ++i) {
			fileLabel[i] = String::format("Save game slot %d", i + 1);

			act = new Action(fileIds[i], _(fileLabel[i]));
			act->setCustomEngineActionEvent(fileActions[i]);
			act->addDefaultInputMapping(String::format("%d", i + 1));
			scalpelFilesKeymap->addAction(act);
		}

		//
		// Quit dialog keymap
		//

		act = new Action("YES", _("Yes"));
		act->setCustomEngineActionEvent(kActionScalpelQuitDialogYes);
		act->addDefaultInputMapping(getKey(kFixedText_QuitGame_Yes, curLanguageEntry, target));
		scalpelQuitDialogKeymap->addAction(act);

		act = new Action("NO", _("No"));
		act->setCustomEngineActionEvent(kActionScalpelQuitDialogNo);
		act->addDefaultInputMapping(getKey(kFixedText_QuitGame_No, curLanguageEntry, target));
		act->addDefaultInputMapping("ESCAPE");
		scalpelQuitDialogKeymap->addAction(act);

		act = new Action("QUITDLGSELECT", _("Select option"));
		act->setCustomEngineActionEvent(kActionScalpelQuitDialogSelect);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("SPACE");
		scalpelQuitDialogKeymap->addAction(act);

		//
		// Map keymap
		//

		act = new Action("MAPSELECT", _("Select"));
		act->setCustomEngineActionEvent(kActionScalpelMapSelect);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_A");
		scalpelMapKeymap->addAction(act);

		//
		// Settings keymap
		//

		act = new Action("SETTINGSSELECT", _("Select"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsSelect);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_A");
		scalpelSettingsKeymap->addAction(act);

		act = new Action("SETTINGSEXIT", _("Exit"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsExit);
		act->addDefaultInputMapping(getKey(kFixedText_Settings_Exit, curLanguageEntry, target));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		scalpelSettingsKeymap->addAction(act);

		act = new Action("TOGGLEMUSIC", _("Toggle music"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsToggleMusic);
		act->addDefaultInputMapping(getKey(kFixedText_Settings_MusicOn, curLanguageEntry, target)); // Same key for on/off in all languages
		scalpelSettingsKeymap->addAction(act);

		act = new Action("TOGGLEPORTRAITS", _("Toggle portraits"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsTogglePortraits);
		act->addDefaultInputMapping(getKey(kFixedText_Settings_PortraitsOn, curLanguageEntry, target)); // Same key for on/off in all languages
		scalpelSettingsKeymap->addAction(act);

		act = new Action("FONT", _("Change font style"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsChangeFontStyle);
		act->addDefaultInputMapping(getKey(kFixedText_Settings_NewFontStyle, curLanguageEntry, target));
		scalpelSettingsKeymap->addAction(act);

		act = new Action("TOGGLESOUNDS", _("Toggle sound effects"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsToggleSoundEffects);
		act->addDefaultInputMapping(getKey(kFixedText_Settings_SoundEffectsOn, curLanguageEntry, target)); // Same key for on/off in all languages
		scalpelSettingsKeymap->addAction(act);

		act = new Action("WINDOW", _("Toggle windows mode"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsToggleWindowsMode);
		act->addDefaultInputMapping(getKey(kFixedText_Settings_WindowsSlide, curLanguageEntry, target)); // Same key for various window modes in all languages
		scalpelSettingsKeymap->addAction(act);

		// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has an auto-help feature, this action is used to change the auto-help location between left and right
		act = new Action("AUTOHELP", _("Change auto-help location"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsChangeAutohelpLoc);
		act->addDefaultInputMapping(getKey(kFixedText_Settings_AutoHelpRight, curLanguageEntry, target)); // Same key for various auto-help location in all languages
		scalpelSettingsKeymap->addAction(act);

		act = new Action("TOGGLEVOICES", _("Toggle voices"));
		act->setCustomEngineActionEvent(kActionScalpelSettingsToggleVoices);
		act->addDefaultInputMapping(getKey(kFixedText_Settings_VoicesOn, curLanguageEntry, target)); // Same key for on/off in all languages
		scalpelSettingsKeymap->addAction(act);

		// German version has different keys for fade modes, every other language uses the same key for all fade modes
		if (parseLanguage(ConfMan.get("language", target)) == Common::DE_DEU) {
			// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has a fade animation when switching scenes, this action is used to change the fade mode to "Fade by pixel"
			act = new Action("FADEBYPIXEL", _("Fade by pixel"));
			act->setCustomEngineActionEvent(kActionScalpelSettingsFadeByPixels);
			act->addDefaultInputMapping(getKey(kFixedText_Settings_FadeByPixel, curLanguageEntry, target));
			scalpelSettingsKeymap->addAction(act);

			// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has a fade animation when switching scenes, this action is used to change the fade mode to "Fade directly"
			act = new Action("FADEDIRECTLY", _("Fade directly"));
			act->setCustomEngineActionEvent(kActionScalpelSettingsFadeDirectly);
			act->addDefaultInputMapping(getKey(kFixedText_Settings_FadeDirectly, curLanguageEntry, target));
			scalpelSettingsKeymap->addAction(act);
		} else {
			// I18N: (Game: The Lost Files of Sherlock Holmes: The Case of the Serrated Scalpel) The game has a fade animation when switching scenes, this action is used to change the fade mode
			act = new Action("FADE", _("Change fade mode"));
			act->setCustomEngineActionEvent(kActionScalpelSettingsChangeFadeMode);
			act->addDefaultInputMapping(getKey(kFixedText_Settings_FadeByPixel, curLanguageEntry, target));
			scalpelSettingsKeymap->addAction(act);
		}

		keymaps.push_back(engineKeymap);
		keymaps.push_back(scalpelQuitKeymap);
		keymaps.push_back(scalpelScrollKeymap);
		keymaps.push_back(animationKeymap);
		keymaps.push_back(scalpelKeymap);
		keymaps.push_back(scalpelDartsKeymap);
		keymaps.push_back(scalpelJournalKeymap);
		keymaps.push_back(scalpelTalkKeymap);
		keymaps.push_back(scalpelInvKeymap);
		keymaps.push_back(scalpelFilesKeymap);
		keymaps.push_back(scalpelQuitDialogKeymap);
		keymaps.push_back(scalpelMapKeymap);
		keymaps.push_back(scalpelSettingsKeymap);

		scalpelScrollKeymap->setEnabled(false);
		animationKeymap->setEnabled(false);
		scalpelDartsKeymap->setEnabled(false);
		scalpelJournalKeymap->setEnabled(false);
		scalpelTalkKeymap->setEnabled(false);
		scalpelInvKeymap->setEnabled(false);
		scalpelFilesKeymap->setEnabled(false);
		scalpelQuitDialogKeymap->setEnabled(false);
		scalpelMapKeymap->setEnabled(false);
		scalpelSettingsKeymap->setEnabled(false);

		if (parsePlatform(ConfMan.get("platform")) == kPlatform3DO) {
			keymaps.push_back(scalpel3D0MovieKeymap);
			scalpel3D0MovieKeymap->setEnabled(false);
		}

	} else if (ConfMan.get("gameid", target) == "rosetattoo") {

		Keymap *tattooKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo", _("Main user interface keymappings"));
		Keymap *tattooExitKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-exit", _("Exit keymappings"));
		Keymap *tattooPrologKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-prolog", _("Movie keymappings"));
		Keymap *tattooJournalKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-journal", _("Journal keymappings"));
		Keymap *tattooJournalSearchKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-journal-search", _("Journal search keymappings"));
		Keymap *tattooDartsKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-darts", _("Darts minigame keymappings"));
		Keymap *tattooMapKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-map", _("Map keymappings"));
		Keymap *tattooScrollingKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-scrolling", _("Menu scrolling keymappings"));
		Keymap *tattooFilesKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-files", _("save / load menu keymappings"));
		Keymap *tattooFilesNameKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-files-name", _("Save files name editing keymappings"));
		Keymap *tattooFoolscapKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-foolscap", _("Foolscap puzzle keymappings"));
		Keymap *tattooInvKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-inv", _("Inventory keymappings"));
		Keymap *tattooOptionsKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-options", _("Options menu keymappings"));
		Keymap *tattooPasswordKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-password", _("Password entry keymappings"));
		Keymap *tattooQuitDialogKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-quit-dialog", _("Quit dialog keymappings"));
		Keymap *tattooTalkKeymap = new Keymap(Keymap::kKeymapTypeGame, "tattoo-talk", _("Talk options keymappings"));

		act = new Common::Action(kStandardActionLeftClick, _("Move / Examine / Select"));
		act->setLeftClickEvent();
		act->addDefaultInputMapping("MOUSE_LEFT");
		act->addDefaultInputMapping("JOY_A");
		engineKeymap->addAction(act);

		act = new Common::Action(kStandardActionRightClick, _("Open action menu / Close menu"));
		act->setRightClickEvent();
		act->addDefaultInputMapping("MOUSE_RIGHT");
		act->addDefaultInputMapping("JOY_B");
		engineKeymap->addAction(act);

		//
		// tattoo main Game keymap
		//

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) This action changes the speed of the game (walking, animations, etc.)
		act = new Action("SPEED", _("Change speed"));
		act->setCustomEngineActionEvent(kActionTattooChangeSpeed);
		act->addDefaultInputMapping("s");
		act->addDefaultInputMapping("JOY_X");
		tattooKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) This action is used to look at an object or npc in the game
		act = new Action("LOOK", _("Look"));
		act->setCustomEngineActionEvent(kActionTattooLook);
		act->addDefaultInputMapping("l");
		act->addDefaultInputMapping("JOY_Y");
		tattooKeymap->addAction(act);

		act = new Action("SAVE", _("Save game"));
		act->setCustomEngineActionEvent(kActionTattooSave);
		act->addDefaultInputMapping("F5");
		tattooKeymap->addAction(act);

		act = new Action("LOAD", _("Load game"));
		act->setCustomEngineActionEvent(kActionTattooLoad);
		act->addDefaultInputMapping("F7");
		tattooKeymap->addAction(act);

		act = new Action("JOURNAL", _("Open journal"));
		act->setCustomEngineActionEvent(kActionTattooJournal);
		act->addDefaultInputMapping("F1");
		act->addDefaultInputMapping("JOY_UP");
		tattooKeymap->addAction(act);

		act = new Action("INV", _("Open inventory"));
		act->setCustomEngineActionEvent(kActionTattooInv);
		act->addDefaultInputMapping("TAB");
		act->addDefaultInputMapping("F3");
		act->addDefaultInputMapping("JOY_DOWN");
		tattooKeymap->addAction(act);

		act = new Action("OPTIONS", _("Open options menu"));
		act->setCustomEngineActionEvent(kActionTattooOptions);
		act->addDefaultInputMapping("F4");
		act->addDefaultInputMapping("JOY_LEFT");
		tattooKeymap->addAction(act);

		act = new Action("QUIT", _("Quit game"));
		act->setCustomEngineActionEvent(kActionTattooQuit);
		act->addDefaultInputMapping("F10");
		act->addDefaultInputMapping("JOY_RIGHT");
		tattooKeymap->addAction(act);

		//
		// Exit keymap
		//

		act = new Action("EXIT", _("Exit / Close window"));
		act->setCustomEngineActionEvent(kActionTattooExit);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		tattooExitKeymap->addAction(act);

		//
		// Prolog keymap
		//

		act = new Action("SKIPPROLOG", _("Skip intro"));
		act->setCustomEngineActionEvent(kActionTattooSkipProlog);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		tattooPrologKeymap->addAction(act);

		//
		// Journal keymap
		//

		act = new Action("OPTIONSLEFT", _("Change selected option to the left"));
		act->setCustomEngineActionEvent(kActionTattooJournalOptionsLeft);
		act->addDefaultInputMapping("S+TAB");
		act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
		tattooJournalKeymap->addAction(act);

		act = new Action("OPTIONSRIGHT", _("Change selected option to the right"));
		act->setCustomEngineActionEvent(kActionTattooJournalOptionsRight);
		act->addDefaultInputMapping("TAB");
		act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
		tattooJournalKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a journal, this action is used to go forward 10 pages in the journal
		act = new Action("PAGEFORWARD10", _("Go forward 10 pages"));
		act->setCustomEngineActionEvent(kActionTattooJournalForward10);
		act->addDefaultInputMapping("S+PAGEDOWN");
		act->addDefaultInputMapping("JOY_RIGHT");
		tattooJournalKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a journal, this action is used to go forward 1 page in the journal
		act = new Action("PAGEFORWARD1", _("Next page"));
		act->setCustomEngineActionEvent(kActionTattooJournalForward1);
		act->addDefaultInputMapping("PAGEDOWN");
		act->addDefaultInputMapping("KP3");
		act->addDefaultInputMapping("JOY_DOWN");
		tattooJournalKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a journal, this action is used to go back 10 pages in the journal
		act = new Action("PAGEBACK10", _("Go back 10 pages"));
		act->setCustomEngineActionEvent(kActionTattooJournalBack10);
		act->addDefaultInputMapping("S+PAGEUP");
		act->addDefaultInputMapping("JOY_LEFT");
		tattooJournalKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a journal, this action is used to go back 1 page in the journal
		act = new Action("PAGEBACK1", _("Previous page"));
		act->setCustomEngineActionEvent(kActionTattooJournalBack1);
		act->addDefaultInputMapping("PAGEUP");
		act->addDefaultInputMapping("KP9");
		act->addDefaultInputMapping("JOY_UP");
		tattooJournalKeymap->addAction(act);

		act = new Action("GOSTART", _("First page"));
		act->setCustomEngineActionEvent(kActionTattooJournalStart);
		act->addDefaultInputMapping("HOME");
		act->addDefaultInputMapping("KP7");
		act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
		tattooJournalKeymap->addAction(act);

		act = new Action("GOEND", _("Last page"));
		act->setCustomEngineActionEvent(kActionTattooJournalEnd);
		act->addDefaultInputMapping("END");
		act->addDefaultInputMapping("KP1");
		act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
		tattooJournalKeymap->addAction(act);

		act = new Action("SELECT", _("Select option"));
		act->setCustomEngineActionEvent(kActionTattooJournalSelect);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("JOY_X");
		tattooJournalKeymap->addAction(act);

		//
		// Journal search keymap
		//

		act = new Action("OPTIONSLEFT", _("Change selected option to the left"));
		act->setCustomEngineActionEvent(kActionTattooJournalSearchOptionsLeft);
		act->addDefaultInputMapping("S+TAB");
		act->addDefaultInputMapping("JOY_LEFT");
		tattooJournalSearchKeymap->addAction(act);

		act = new Action("OPTIONSRIGHT", _("Change selected option to the right"));
		act->setCustomEngineActionEvent(kActionTattooJournalSearchOptionsRight);
		act->addDefaultInputMapping("TAB");
		act->addDefaultInputMapping("JOY_RIGHT");
		tattooJournalSearchKeymap->addAction(act);

		act = new Action("SEARCH", _("Search"));
		act->setCustomEngineActionEvent(kActionTattooJournalSearch);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("JOY_X");
		tattooJournalSearchKeymap->addAction(act);

		//
		// Darts keymap
		//

		act = new Action("SKIPGAME", _("Skip darts minigame"));
		act->setCustomEngineActionEvent(kActionTattooSkipDarts);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		tattooDartsKeymap->addAction(act);

		//
		// Map keymap
		//

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a map, this action is used to scroll to the top left of the map
		act = new Action("TOPLEFT", _("Scroll to top left"));
		act->setCustomEngineActionEvent(kActionTattooMapTopLeft);
		act->addDefaultInputMapping("HOME");
		act->addDefaultInputMapping("KP7");
		act->addDefaultInputMapping("JOY_LEFT");
		tattooMapKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a map, this action is used to scroll to the bottom right of the map
		act = new Action("BOTTOMRIGHT", _("Scroll to bottom right"));
		act->setCustomEngineActionEvent(kActionTattooMapBottomRight);
		act->addDefaultInputMapping("END");
		act->addDefaultInputMapping("KP1");
		act->addDefaultInputMapping("JOY_RIGHT");
		tattooMapKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a map, this action is used to scroll up in the map
		act = new Action("UP", _("Scroll up"));
		act->setCustomEngineActionEvent(kActionTattooMapUp);
		act->addDefaultInputMapping("PAGEUP");
		act->addDefaultInputMapping("KP9");
		act->addDefaultInputMapping("JOY_UP");
		tattooMapKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a map, this action is used to scroll down in the map
		act = new Action("DOWN", _("Scroll down"));
		act->setCustomEngineActionEvent(kActionTattooMapDown);
		act->addDefaultInputMapping("PAGEDOWN");
		act->addDefaultInputMapping("KP3");
		act->addDefaultInputMapping("JOY_DOWN");
		tattooMapKeymap->addAction(act);

		act = new Action("SELECT", _("Select"));
		act->setCustomEngineActionEvent(kActionTattooMapSelect);
		act->addDefaultInputMapping("SPACE");
		act->addDefaultInputMapping("JOY_X");
		tattooMapKeymap->addAction(act);

		//
		// Scrolling keymap
		//

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has multiple widgets for various purposes (eg. inventory, save files, etc.), this action is used to scroll up in the widget
		act = new Action("SCROLLUP", _("Scroll up"));
		act->setCustomEngineActionEvent(kActionTattooWidgetScrollUp);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("KP8");
		act->addDefaultInputMapping("JOY_UP");
		tattooScrollingKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has multiple widgets for various purposes (eg. inventory, save files, etc.), this action is used to scroll down in the widget
		act = new Action("SCROLLDOWN", _("Scroll down"));
		act->setCustomEngineActionEvent(kActionTattooWidgetScrollDown);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("KP2");
		act->addDefaultInputMapping("JOY_DOWN");
		tattooScrollingKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has multiple widgets for various purposes (eg. inventory, save files, etc.), this action is used to scroll up by a page in the widget
		act = new Action("SCROLLPGUP", _("Previous page"));
		act->setCustomEngineActionEvent(kActionTattooWidgetScrollPageUp);
		act->addDefaultInputMapping("PAGEUP");
		act->addDefaultInputMapping("KP9");
		tattooScrollingKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has multiple widgets for various purposes (eg. inventory, save files, etc.), this action is used to scroll down by a page in the widget
		act = new Action("SCROLLPGDOWN", _("Next page"));
		act->setCustomEngineActionEvent(kActionTattooWidgetScrollPageDown);
		act->addDefaultInputMapping("PAGEDOWN");
		act->addDefaultInputMapping("KP3");
		tattooScrollingKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has multiple widgets for various purposes (eg. inventory, save files, etc.), this action is used to scroll to the end of the widget
		act = new Action("SCROLLEND", _("Scroll to end"));
		act->setCustomEngineActionEvent(kActionTattooWidgetScrollEnd);
		act->addDefaultInputMapping("END");
		act->addDefaultInputMapping("KP1");
		tattooScrollingKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has multiple widgets for various purposes (eg. inventory, save files, etc.), this action is used to scroll to the start of the widget
		act = new Action("SCROLLSTART", _("Scroll to start"));
		act->setCustomEngineActionEvent(kActionTattooWidgetScrollStart);
		act->addDefaultInputMapping("HOME");
		act->addDefaultInputMapping("KP7");
		tattooScrollingKeymap->addAction(act);

		//
		// Files keymap
		//

		act = new Action("NEXT", _("Go to next save file slot"));
		act->setCustomEngineActionEvent(kActionTattooFilesNextSlot);
		act->addDefaultInputMapping("TAB");
		act->addDefaultInputMapping("JOY_RIGHT");
		tattooFilesKeymap->addAction(act);

		act = new Action("NEXTPGE", _("Go to next page of save files"));
		act->setCustomEngineActionEvent(kActionTattooFilesNextPageSlot);
		act->addDefaultInputMapping("S+TAB");
		tattooFilesKeymap->addAction(act);

		act = new Action("SELECT", _("Select save file"));
		act->setCustomEngineActionEvent(kActionTattooFilesSelect);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_X");
		tattooFilesKeymap->addAction(act);

		//
		// Files name keymap
		//

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) cursor refers to the text cursor in the save file name input field
		act = new Action("LEFT", _("Move cursor left"));
		act->setCustomEngineActionEvent(kActionTattooFilesNameLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
		act->addDefaultInputMapping("JOY_LEFT");
		tattooFilesNameKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) cursor refers to the text cursor in the save file name input field
		act = new Action("RIGHT", _("Move cursor right"));
		act->setCustomEngineActionEvent(kActionTattooFilesNameRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		act->addDefaultInputMapping("JOY_RIGHT");
		tattooFilesNameKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) This action is used to go to the start of the save file name input field
		act = new Action("GOSTART", _("Go to start of line"));
		act->setCustomEngineActionEvent(kActionTattooFilesNameStart);
		act->addDefaultInputMapping("HOME");
		act->addDefaultInputMapping("KP7");
		tattooFilesNameKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) This action is used to go to the end of the save file name input field
		act = new Action("GOEND", _("Go to end of line"));
		act->setCustomEngineActionEvent(kActionTattooFilesNameEnd);
		act->addDefaultInputMapping("END");
		act->addDefaultInputMapping("KP1");
		tattooFilesNameKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) save file name input field has 2 modes, insert and overwrite, this action toggles between them
		act = new Action("INSERTMODE", _("Toggle insert mode"));
		act->setCustomEngineActionEvent(kActionTattooFilesNameToggleInsertMode);
		act->addDefaultInputMapping("INSERT");
		act->addDefaultInputMapping("JOY_UP");
		tattooFilesNameKeymap->addAction(act);

		//
		// Foolscap puzzle keymap
		//

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) The game has a foolscap puzzle, where the player has to decode a hidden message on a piece of paper called a foolscap. this action is used to exit the puzzle
		act = new Action("EXIT", _("Close puzzle"));
		act->setCustomEngineActionEvent(kActionTattooFoolscapExit);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		tattooFoolscapKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) cursor refers to the text cursor in the foolscap puzzle input field
		act = new Action("UP", _("Move cursor up"));
		act->setCustomEngineActionEvent(kActionTattooFoolscapUp);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("KP8");
		act->addDefaultInputMapping("JOY_UP");
		tattooFoolscapKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) cursor refers to the text cursor in the foolscap puzzle input field
		act = new Action("DOWN", _("Move cursor down"));
		act->setCustomEngineActionEvent(kActionTattooFoolscapDown);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("KP2");
		act->addDefaultInputMapping("JOY_DOWN");
		tattooFoolscapKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) cursor refers to the text cursor in the foolscap puzzle input field
		act = new Action("LEFT", _("Move cursor left"));
		act->setCustomEngineActionEvent(kActionTattooFoolscapLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
		act->addDefaultInputMapping("JOY_LEFT");
		tattooFoolscapKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) cursor refers to the text cursor in the foolscap puzzle input field
		act = new Action("RIGHT", _("Move cursor right"));
		act->setCustomEngineActionEvent(kActionTattooFoolscapRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		act->addDefaultInputMapping("JOY_RIGHT");
		tattooFoolscapKeymap->addAction(act);

		//
		// Inventory keymap
		//

		act = new Action("EXIT", _("Exit inventory / Close verb menu / Cancel item use action"));
		act->setCustomEngineActionEvent(kActionTattooInvExit);
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		tattooInvKeymap->addAction(act);

		//
		// Options menu keymap
		//

		act = new Action("SELECT", _("Select / Interact"));
		act->setCustomEngineActionEvent(kActionTattooOptionsSelect);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("KP_ENTER");
		act->addDefaultInputMapping("SPACE");
		tattooOptionsKeymap->addAction(act);

		//
		// Password entry keymap
		//

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) cursor refers to the text cursor in the password input field
		act = new Action("LEFT", _("Move cursor left"));
		act->setCustomEngineActionEvent(kActionTattooPasswordLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
		act->addDefaultInputMapping("JOY_LEFT");
		tattooPasswordKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) cursor refers to the text cursor in the password input field
		act = new Action("RIGHT", _("Move cursor right"));
		act->setCustomEngineActionEvent(kActionTattooPasswordRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		act->addDefaultInputMapping("JOY_RIGHT");
		tattooPasswordKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) This action is used to go to the start of the password input field
		act = new Action("GOSTART", _("Go to the line start"));
		act->setCustomEngineActionEvent(kActionTattooPasswordStart);
		act->addDefaultInputMapping("HOME");
		act->addDefaultInputMapping("KP7");
		tattooPasswordKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) This action is used to go to the end of the password input field
		act = new Action("GOEND", _("Go to the line end"));
		act->setCustomEngineActionEvent(kActionTattooPasswordEnd);
		act->addDefaultInputMapping("END");
		act->addDefaultInputMapping("KP1");
		tattooPasswordKeymap->addAction(act);

		// I18N: (Game name: The Lost Files of Sherlock Holmes: The Case of the Rose Tattoo) save password input field has 2 modes, insert and overwrite, this action toggles between them
		act = new Action("INSERTMODE", _("Toggle insert mode"));
		act->setCustomEngineActionEvent(kActionTattooPasswordToggleInsertMode);
		act->addDefaultInputMapping("INSERT");
		act->addDefaultInputMapping("JOY_UP");
		tattooPasswordKeymap->addAction(act);

		//
		// Quit dialog keymap
		//

		act = new Action("NEXTOPTION", _("Select next option"));
		act->setCustomEngineActionEvent(kActionTattooQuitDialogNextOption);
		act->addDefaultInputMapping("TAB");
		tattooQuitDialogKeymap->addAction(act);

		act = new Action("YES", _("Yes"));
		act->setCustomEngineActionEvent(kActionTattooQuitDialogYes);
		act->addDefaultInputMapping("y");
		act->addDefaultInputMapping("JOY_X");
		tattooQuitDialogKeymap->addAction(act);

		act = new Action("NO", _("No"));
		act->setCustomEngineActionEvent(kActionTattooQuitDialogNo);
		act->addDefaultInputMapping("n");
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		tattooQuitDialogKeymap->addAction(act);

		//
		// Talk keymap
		//

		act = new Action("NEXT", _("Go to next talk option"));
		act->setCustomEngineActionEvent(kActionTattooTalkNext);
		act->addDefaultInputMapping("TAB");
		act->addDefaultInputMapping("JOY_DOWN");
		tattooTalkKeymap->addAction(act);

		act = new Action("PREVIOUS", _("Go to previous talk option"));
		act->setCustomEngineActionEvent(kActionTattooTalkPrevious);
		act->addDefaultInputMapping("S+TAB");
		act->addDefaultInputMapping("JOY_UP");
		tattooTalkKeymap->addAction(act);

		keymaps.push_back(tattooExitKeymap);
		keymaps.push_back(engineKeymap);
		keymaps.push_back(tattooKeymap);
		keymaps.push_back(tattooPrologKeymap);
		keymaps.push_back(tattooJournalKeymap);
		keymaps.push_back(tattooJournalSearchKeymap);
		keymaps.push_back(tattooDartsKeymap);
		keymaps.push_back(tattooMapKeymap);
		keymaps.push_back(tattooScrollingKeymap);
		keymaps.push_back(tattooFilesKeymap);
		keymaps.push_back(tattooFilesNameKeymap);
		keymaps.push_back(tattooFoolscapKeymap);
		keymaps.push_back(tattooInvKeymap);
		keymaps.push_back(tattooOptionsKeymap);
		keymaps.push_back(tattooPasswordKeymap);
		keymaps.push_back(tattooQuitDialogKeymap);
		keymaps.push_back(tattooTalkKeymap);

		tattooExitKeymap->setEnabled(false);
		tattooPrologKeymap->setEnabled(false);
		tattooJournalKeymap->setEnabled(false);
		tattooJournalSearchKeymap->setEnabled(false);
		tattooDartsKeymap->setEnabled(false);
		tattooMapKeymap->setEnabled(false);
		tattooScrollingKeymap->setEnabled(false);
		tattooFilesKeymap->setEnabled(false);
		tattooFilesNameKeymap->setEnabled(false);
		tattooFoolscapKeymap->setEnabled(false);
		tattooInvKeymap->setEnabled(false);
		tattooOptionsKeymap->setEnabled(false);
		tattooPasswordKeymap->setEnabled(false);
		tattooQuitDialogKeymap->setEnabled(false);
		tattooTalkKeymap->setEnabled(false);
	}

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(SHERLOCK)
	REGISTER_PLUGIN_DYNAMIC(SHERLOCK, PLUGIN_TYPE_ENGINE, SherlockMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SHERLOCK, PLUGIN_TYPE_ENGINE, SherlockMetaEngine);
#endif
