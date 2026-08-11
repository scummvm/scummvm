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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/lab.h"
#include "engines/advancedDetector.h"

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

namespace Lab {

Common::Platform LabEngine::getPlatform() const {
	return _gameDescription->platform;
}

uint32 LabEngine::getFeatures() const {
	return _gameDescription->flags | _extraGameFeatures;
}

} // End of namespace Lab

class LabMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "lab";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new Lab::LabEngine(syst, desc);
		return Common::kNoError;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool LabMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool Lab::LabEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList LabMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Lab::SaveGameHeader header;
	Common::String pattern = target;
	pattern += ".###";

	Common::StringArray filenames = saveFileMan->listSavefiles(pattern.c_str());
	SaveStateList saveList;

	for (const auto &filename : filenames) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(filename.c_str() + filename.size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(filename);
			if (in) {
				if (Lab::readSaveGameHeader(in, header))
					saveList.push_back(SaveStateDescriptor(this, slotNum, header._descr.getDescription()));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int LabMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

bool LabMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	return saveFileMan->removeSavefile(Common::String::format("%s.%03u", target, slot));
}

SaveStateDescriptor LabMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03u", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Lab::SaveGameHeader header;

		bool successfulRead = Lab::readSaveGameHeader(in, header, false);
		delete in;

		if (successfulRead) {
			SaveStateDescriptor desc(this, slot, header._descr.getDescription());
			return header._descr;
		}
	}

	return SaveStateDescriptor();
}

Common::KeymapArray LabMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Lab;

	Keymap *engineKeymap = new Keymap(Keymap::kKeymapTypeGame, "lab-default", _("Default keymappings"));
	Keymap *exitKeymap = new Keymap(Keymap::kKeymapTypeGame, "exit", _("Exit keymappings"));
	Keymap *quitDialogKeymap = new Keymap(Keymap::kKeymapTypeGame, "quit-dialog", _("Quit dialog keymappings"));
	Keymap *gameKeymap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *invKeymap = new Keymap(Keymap::kKeymapTypeGame, "inventory", _("Inventory keymappings"));
	Keymap *introKeymap = new Keymap(Keymap::kKeymapTypeGame, "intro", _("Intro keymappings"));
	Keymap *mapKeymap = new Keymap(Keymap::kKeymapTypeGame, "map", _("Map keymappings"));
	Keymap *journalKeymap = new Keymap(Keymap::kKeymapTypeGame, "journal", _("Journal keymappings"));

	Common::Action *act;

	act = new Common::Action(kStandardActionLeftClick, _("Interact"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeymap->addAction(act);

	act = new Common::Action(kStandardActionRightClick, _("Exit"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeymap->addAction(act);

	act = new Common::Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("q");
	act->addDefaultInputMapping("x");
	act->addDefaultInputMapping("JOY_DOWN");
	engineKeymap->addAction(act);

	act = new Common::Action("RAISESOUND", _("Raise the sound volume"));
	act->setCustomEngineActionEvent(kActionSoundRaise);
	act->addDefaultInputMapping("RIGHTBRACKET");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeymap->addAction(act);

	act = new Common::Action("LOWERSOUND", _("Lower the sound volume"));
	act->setCustomEngineActionEvent(kActionSoundLower);
	act->addDefaultInputMapping("LEFTBRACKET");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeymap->addAction(act);

	act = new Common::Action("EXIT", _("Exit"));
	act->setCustomEngineActionEvent(kActionExit);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	exitKeymap->addAction(act);

	act = new Common::Action("YES", _("Yes"));
	act->setCustomEngineActionEvent(kActionQuitDialogYes);
	act->addDefaultInputMapping("y");
	act->addDefaultInputMapping("q");
	act->addDefaultInputMapping("JOY_A");
	quitDialogKeymap->addAction(act);

	act = new Common::Action("NO", _("No"));
	act->setCustomEngineActionEvent(kActionQuitDialogNo);
	act->addDefaultInputMapping("JOY_B");
	quitDialogKeymap->addAction(act);

	act = new Common::Action("SKIP_INTRO", _("Skip intro"));
	act->setCustomEngineActionEvent(kActionSkipIntro);
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_X");
	introKeymap->addAction(act);

	act = new Common::Action("INTERACT", _("Interact with object with chosen action"));
	act->setCustomEngineActionEvent(kActionInteract);
	act->addDefaultInputMapping("RETURN");
	gameKeymap->addAction(act);

	act = new Common::Action("TAKE", _("Take object"));
	act->setCustomEngineActionEvent(kActionTake);
	act->addDefaultInputMapping("t");
	act->addDefaultInputMapping("1");
	gameKeymap->addAction(act);

	act = new Common::Action("MOVE", _("Move or manipulate object"));
	act->setCustomEngineActionEvent(kActionMove);
	act->addDefaultInputMapping("m");
	act->addDefaultInputMapping("2");
	gameKeymap->addAction(act);

	act = new Common::Action("OPEN", _("Open door or object"));
	act->setCustomEngineActionEvent(kActionOpen);
	act->addDefaultInputMapping("o");
	act->addDefaultInputMapping("3");
	gameKeymap->addAction(act);

	act = new Common::Action("CLOSE", _("Close door or object"));
	act->setCustomEngineActionEvent(kActionClose);
	act->addDefaultInputMapping("c");
	act->addDefaultInputMapping("4");
	gameKeymap->addAction(act);

	act = new Common::Action("LOOK", _("Look at object close-up"));
	act->setCustomEngineActionEvent(kActionLook);
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("5");
	gameKeymap->addAction(act);

	act = new Common::Action("INVENTORY", _("Switch to inventory display"));
	act->setCustomEngineActionEvent(kActionInv);
	act->addDefaultInputMapping("i");
	act->addDefaultInputMapping("6");
	gameKeymap->addAction(act);

	act = new Common::Action("LEFT", _("Turn left"));
	act->setCustomEngineActionEvent(kActionLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("7");
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeymap->addAction(act);

	act = new Common::Action("FORWARD", _("Walk forward"));
	act->setCustomEngineActionEvent(kActionForward);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("8");
	act->addDefaultInputMapping("JOY_UP");
	gameKeymap->addAction(act);

	act = new Common::Action("RIGHT", _("Turn right"));
	act->setCustomEngineActionEvent(kActionRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("9");
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeymap->addAction(act);

	act = new Common::Action("MAP", _("Show map"));
	act->setCustomEngineActionEvent(kActionMap);
	act->addDefaultInputMapping("p");
	act->addDefaultInputMapping("0");
	gameKeymap->addAction(act);

	act = new Common::Action("FOCUS", _("Move focus to interactive object"));
	act->setCustomEngineActionEvent(kActionFocusOnNextInteractiveItem);
	act->addDefaultInputMapping("TAB");
	act->addDefaultInputMapping("JOY_X");
	gameKeymap->addAction(act);

	if (parsePlatform(ConfMan.get("platform")) == Common::kPlatformWindows) {

		act = new Common::Action("STARTBREADCRUMB", _("Start dropping virtual bread crumbs"));
		act->setCustomEngineActionEvent(kActionDropBreadcrumb);
		act->addDefaultInputMapping("b");
		act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
		gameKeymap->addAction(act);

		act = new Common::Action("FOLLOWBREADCRUMBS", _("Follow virtual bread crumbs"));
		act->setCustomEngineActionEvent(kActionFollowBreadcrumbs);
		act->addDefaultInputMapping("f");
		act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
		gameKeymap->addAction(act);

		act = new Common::Action("RUNFOLLOWINGBREADCRUMBS", _("Run while following virtual bread crumbs"));
		act->setCustomEngineActionEvent(kActionRunWhileFollowingBreadcrumbs);
		act->addDefaultInputMapping("r");
		act->addDefaultInputMapping("JOY_X");
		gameKeymap->addAction(act);
	}

	act = new Common::Action("MAINDISPLAY", _("Switch to main display"));
	act->setCustomEngineActionEvent(kActionMainDisplay);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("1");
	invKeymap->addAction(act);

	act = new Common::Action("SAVELOAD", _("Open save / load dialog"));
	act->setCustomEngineActionEvent(kActionSaveLoad);
	act->addDefaultInputMapping("g");
	act->addDefaultInputMapping("2");
	invKeymap->addAction(act);

	act = new Common::Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping("u");
	act->addDefaultInputMapping("3");
	invKeymap->addAction(act);

	act = new Common::Action("LOOKSCENE", _("Look at scene"));
	act->setCustomEngineActionEvent(kActionInvLook);
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("4");
	invKeymap->addAction(act);

	act = new Common::Action("PREV", _("Previous inventory item"));
	act->setCustomEngineActionEvent(kActionPrev);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("5");
	act->addDefaultInputMapping("JOY_LEFT");
	invKeymap->addAction(act);

	act = new Common::Action("NEXT", _("Next inventory item"));
	act->setCustomEngineActionEvent(kActionNext);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("6");
	act->addDefaultInputMapping("JOY_RIGHT");
	invKeymap->addAction(act);


	if (parsePlatform(ConfMan.get("platform")) == Common::kPlatformWindows) {

		act = new Common::Action("STARTBREADCRUMB", _("Start dropping virtual bread crumbs"));
		act->setCustomEngineActionEvent(kActionDropBreadcrumb);
		act->addDefaultInputMapping("b");
		act->addDefaultInputMapping("7");
		act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
		invKeymap->addAction(act);

		act = new Common::Action("FOLLOWBREADCRUMBS", _("Follow virtual bread crumbs"));
		act->setCustomEngineActionEvent(kActionFollowBreadcrumbs);
		act->addDefaultInputMapping("f");
		act->addDefaultInputMapping("8");
		act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
		invKeymap->addAction(act);

		act = new Common::Action("RUNFOLLOWINGBREADCRUMBS", _("Run while following virtual bread crumbs"));
		act->setCustomEngineActionEvent(kActionRunWhileFollowingBreadcrumbs);
		act->addDefaultInputMapping("r");
		act->addDefaultInputMapping("JOY_X");
		invKeymap->addAction(act);
	}

	act = new Common::Action("EXITMAP", _("Exit map display"));
	act->setCustomEngineActionEvent(kActionMapExit);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("1");
	mapKeymap->addAction(act);

	act = new Common::Action("UPLEVEL", _("Up one level"));
	act->setCustomEngineActionEvent(kActionUpperFloor);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("2");
	mapKeymap->addAction(act);

	act = new Common::Action("DOWNLEVEL", _("Down one level"));
	act->setCustomEngineActionEvent(kActionLowerFloor);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("3");
	mapKeymap->addAction(act);

	act = new Common::Action("JOURNALBACK", _("Go back in journal"));
	act->setCustomEngineActionEvent(kActionJournalBack);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("1");
	journalKeymap->addAction(act);

	act = new Common::Action("EXITJOURNAL", _("Exit journal"));
	act->setCustomEngineActionEvent(kActionJournalExit);
	act->addDefaultInputMapping("2");
	journalKeymap->addAction(act);

	act = new Common::Action("JOURNALFORWARD", _("Go forward in journal"));
	act->setCustomEngineActionEvent(kActionJournalForward);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("3");
	journalKeymap->addAction(act);

	KeymapArray keymaps(8);

	keymaps[0] = engineKeymap;
	keymaps[1] = exitKeymap;
	keymaps[2] = gameKeymap;
	keymaps[3] = invKeymap;
	keymaps[4] = introKeymap;
	keymaps[5] = mapKeymap;
	keymaps[6] = quitDialogKeymap;
	keymaps[7] = journalKeymap;

	invKeymap->setEnabled(false);
	introKeymap->setEnabled(false);
	mapKeymap->setEnabled(false);
	quitDialogKeymap->setEnabled(false);
	journalKeymap->setEnabled(false);

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(LAB)
	REGISTER_PLUGIN_DYNAMIC(LAB, PLUGIN_TYPE_ENGINE, LabMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LAB, PLUGIN_TYPE_ENGINE, LabMetaEngine);
#endif
