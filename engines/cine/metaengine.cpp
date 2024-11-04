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

#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/util.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "cine/cine.h"
#include "cine/various.h"

#include "cine/detection.h"

namespace Cine {

static const ADExtraGuiOptionsMap optionsList[] = {
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
		GAMEOPTION_TRANSPARENT_DIALOG_BOXES,
		{
			_s("Use transparent dialog boxes in 16 color scenes"),
			_s("Use transparent dialog boxes in 16 color scenes even if the original game version did not support them"),
			"transparentdialogboxes",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

#define MAX_SAVEGAMES (ARRAYSIZE(Cine::currentSaveName))
#define SAVEGAME_NAME_LEN (sizeof(Cine::currentSaveName[0]))
#define SAVELIST_SIZE (MAX_SAVEGAMES * SAVEGAME_NAME_LEN)

bool CineEngine::mayHave256Colors() const { return getGameType() == Cine::GType_OS && getPlatform() == Common::kPlatformDOS; }
int CineEngine::getGameType() const { return _gameDescription->gameType; }
uint32 CineEngine::getFeatures() const { return _gameDescription->features; }
Common::Language CineEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform CineEngine::getPlatform() const { return _gameDescription->desc.platform; }

} // End of namespace Cine

class CineMetaEngine : public AdvancedMetaEngine<Cine::CINEGameDescription> {
public:
	const char *getName() const override {
		return "cine";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Cine::optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const Cine::CINEGameDescription *desc) const override;

	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target = nullptr) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool CineMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsLoadingDuringStartup) ||
		checkExtendedSaves(f);
}

bool Cine::CineEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error CineMetaEngine::createInstance(OSystem *syst, Engine **engine, const Cine::CINEGameDescription *desc) const {
	*engine = new Cine::CineEngine(syst,desc);
	return Common::kNoError;
}

SaveStateList CineMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;

	Common::String pattern;

	Common::StringArray::const_iterator file;

	Common::String filename = target;
	filename += ".dir";
	Common::InSaveFile *in = saveFileMan->openForLoading(filename);
	bool foundAutosave = false;
	if (in) {
		typedef char CommandeType[SAVEGAME_NAME_LEN];
		CommandeType saveNames[MAX_SAVEGAMES];

		// Initialize all savegames' descriptions to empty strings
		// so that if the savegames' descriptions can only be partially read from file
		// then the missing ones are correctly set to empty strings.
		memset(saveNames, 0, sizeof(saveNames));

		in->read(saveNames, SAVELIST_SIZE);
		CommandeType saveDesc;

		pattern = target;
		pattern += ".#*";
		Common::StringArray filenames = saveFileMan->listSavefiles(pattern);

		for (file = filenames.begin(); file != filenames.end(); ++file) {
			// Obtain the extension part of the filename, since it corresponds to the save slot number
			Common::String ext = Common::lastPathComponent(*file, '.');
			int slotNum = (int)ext.asUint64();

			if (ext.equals(Common::String::format("%d", slotNum)) &&
				slotNum >= 0 && slotNum < MAX_SAVEGAMES) {
				// Copy the savegame description making sure it ends with a trailing zero
				strncpy(saveDesc, saveNames[slotNum], SAVEGAME_NAME_LEN);
				saveDesc[sizeof(CommandeType) - 1] = 0;

				SaveStateDescriptor saveStateDesc(this, slotNum, saveDesc);

				if (saveStateDesc.getDescription().empty()) {
					if (slotNum == getAutosaveSlot()) {
						saveStateDesc.setDescription(_("Unnamed autosave"));
					} else {
						saveStateDesc.setDescription(_("Unnamed savegame"));
					}
				}

				if (slotNum == getAutosaveSlot()) {
					foundAutosave = true;
				}

				saveList.push_back(saveStateDesc);
			}
		}
	}

	delete in;

	// No saving on empty autosave slot
	if (!foundAutosave) {
		SaveStateDescriptor desc(this, getAutosaveSlot(), _("Empty autosave"));
		saveList.push_back(desc);
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int CineMetaEngine::getMaximumSaveSlot() const { return MAX_SAVEGAMES - 1; }

Common::String CineMetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	return Common::String::format("%s.%d", target == nullptr ? getName() : target, saveGameIdx);
}

SaveStateDescriptor CineMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	if (slot < 0 || slot > getMaximumSaveSlot()) {
		// HACK: Try to make SaveLoadChooserGrid::open() not use save slot
		// numbers over the maximum save slot number for "New save".
		SaveStateDescriptor desc;
		desc.setWriteProtectedFlag(true);
		return desc;
	}

	Common::ScopedPtr<Common::InSaveFile> f(g_system->getSavefileManager()->openForLoading(
		getSavegameFile(slot, target)));

	if (f) {
		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, Common::U32String());

		ExtendedSavegameHeader header;
		if (readSavegameHeader(f.get(), &header, false)) {
			parseSavegameHeader(&header, &desc);
			desc.setThumbnail(header.thumbnail);
		} else {
			// Load savegame descriptions from index file
			typedef char CommandeType[SAVEGAME_NAME_LEN];
			CommandeType saveNames[MAX_SAVEGAMES];
			memset(saveNames, 0, sizeof(saveNames));

			Common::InSaveFile *in;
			in = g_system->getSavefileManager()->openForLoading(Common::String::format("%s.dir", target));

			if (in) {
				in->read(saveNames, SAVELIST_SIZE);
				delete in;
			}

			saveNames[slot][SAVEGAME_NAME_LEN - 1] = 0;
			Common::String saveNameStr((const char *)saveNames[slot]);
			desc.setDescription(saveNameStr);
		}

		if (desc.getDescription().empty()) {
			desc.setDescription(_("Unnamed savegame"));
		}

		return desc;
	}

	// No saving on empty autosave slot
	if (slot == getAutosaveSlot()) {
		SaveStateDescriptor desc(this, slot, _("Empty autosave"));
		desc.setAutosave(true);
		return desc;
	}

	return SaveStateDescriptor();
}

Common::KeymapArray CineMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Cine;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "cine-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *mouseKeyMap = new Keymap(Keymap::kKeymapTypeGame, "mouse-shortcuts", _("Key to mouse keymappings"));
	Keymap *introKeyMap = new Keymap(Keymap::kKeymapTypeGame, "intro-shortcuts", _("Exit screen keymappings"));

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

	act = new Action("SKIPSONY", _("Exit Sony intro screen"));
	act->setCustomEngineActionEvent(kActionExitSonyScreen);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_A");
	introKeyMap->addAction(act);

	act = new Action("MOUSELEFT", _("Select option / Click in game"));
	act->setCustomEngineActionEvent(kActionMouseLeft);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	act->addDefaultInputMapping("KP5");
	mouseKeyMap->addAction(act);

	act = new Action("MOUSERIGHT", _("Open action menu / Close menu"));
	act->setCustomEngineActionEvent(kActionMouseRight);
	act->addDefaultInputMapping("ESCAPE");
	mouseKeyMap->addAction(act);

	act = new Action("DEFAULTSPEED", _("Default game speed"));
	act->setCustomEngineActionEvent(kActionGameSpeedDefault);
	act->addDefaultInputMapping("KP0");
	gameKeyMap->addAction(act);

	act = new Action("SLOWERSPEED", _("Slower game speed"));
	act->setCustomEngineActionEvent(kActionGameSpeedSlower);
	act->addDefaultInputMapping("MINUS");
	act->addDefaultInputMapping("KP_MINUS");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("FASTERSPEED", _("Faster game speed"));
	act->setCustomEngineActionEvent(kActionGameSpeedFaster);
	act->addDefaultInputMapping("PLUS");
	act->addDefaultInputMapping("KP_PLUS");
	act->addDefaultInputMapping("S+EQUALS");
	act->allowKbdRepeats();
	gameKeyMap->addAction(act);

	act = new Action("EXAMINE", _("Examine"));
	act->setCustomEngineActionEvent(kActionExamine);
	act->addDefaultInputMapping("F1");
	gameKeyMap->addAction(act);

	act = new Action("TAKE", _("Take"));
	act->setCustomEngineActionEvent(kActionTake);
	act->addDefaultInputMapping("F2");
	gameKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("F3");
	gameKeyMap->addAction(act);

	act = new Action("USE", _("Use"));
	act->setCustomEngineActionEvent(kActionUse);
	act->addDefaultInputMapping("F4");
	gameKeyMap->addAction(act);

	act = new Action("ACTIVATE", _("Activate"));
	act->setCustomEngineActionEvent(kActionActivate);
	act->addDefaultInputMapping("F5");
	gameKeyMap->addAction(act);

	act = new Action("SPEAK", _("Speak"));
	act->setCustomEngineActionEvent(kActionSpeak);
	act->addDefaultInputMapping("F6");
	gameKeyMap->addAction(act);

	act = new Action("ACTMENU", _("Action menu"));
	act->setCustomEngineActionEvent(kActionActionMenu);
	act->addDefaultInputMapping("F9");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	gameKeyMap->addAction(act);

	act = new Action("SYSMENU", _("System menu"));
	act->setCustomEngineActionEvent(kActionSystemMenu);
	act->addDefaultInputMapping("F10");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	gameKeyMap->addAction(act);

	// I18N: Opens collision map of where the actor can freely move
	act = new Action("COLLISIONPAGE", _("Show collisions"));
	act->setCustomEngineActionEvent(kActionCollisionPage);
	act->addDefaultInputMapping("F11");
	act->addDefaultInputMapping("JOY_Y");
	gameKeyMap->addAction(act);

	// I18N: Move Actor to upwards direction
	act = new Action("MOVEUP", _("Move up"));
	act->setCustomEngineActionEvent(kActionMoveUp);
	act->addDefaultInputMapping("KP8");
	act->addDefaultInputMapping("JOY_UP");
	gameKeyMap->addAction(act);

	// I18N: Move Actor to downwards direction
	act = new Action("MOVEDOWN", _("Move down"));
	act->setCustomEngineActionEvent(kActionMoveDown);
	act->addDefaultInputMapping("KP2");
	act->addDefaultInputMapping("JOY_DOWN");
	gameKeyMap->addAction(act);

	// I18N: Move Actor to left direction
	act = new Action("MOVELEFT", _("Move left"));
	act->setCustomEngineActionEvent(kActionMoveLeft);
	act->addDefaultInputMapping("KP4");
	act->addDefaultInputMapping("JOY_LEFT");
	gameKeyMap->addAction(act);

	// I18N: Move Actor to right direction
	act = new Action("MOVERIGHT", _("Move right"));
	act->setCustomEngineActionEvent(kActionMoveRight);
	act->addDefaultInputMapping("KP6");
	act->addDefaultInputMapping("JOY_RIGHT");
	gameKeyMap->addAction(act);

	// I18N: Move Actor to top-left direction
	act = new Action("MOVEUPLEFT", _("Move up-left"));
	act->setCustomEngineActionEvent(kActionMoveUpLeft);
	act->addDefaultInputMapping("KP7");
	gameKeyMap->addAction(act);

	// I18N: Move Actor to top-right direction
	act = new Action("MOVEUPRIGHT", _("Move up-right"));
	act->setCustomEngineActionEvent(kActionMoveUpRight);
	act->addDefaultInputMapping("KP9");
	gameKeyMap->addAction(act);

	// I18N: Move Actor to bottom-left direction
	act = new Action("MOVEDOWNLEFT", _("Move down-left"));
	act->setCustomEngineActionEvent(kActionMoveDownLeft);
	act->addDefaultInputMapping("KP1");
	gameKeyMap->addAction(act);

	// I18N: Move Actor to bottom-right direction
	act = new Action("MOVEDOWNRIGHT", _("Move down-right"));
	act->setCustomEngineActionEvent(kActionMoveDownRight);
	act->addDefaultInputMapping("KP3");
	gameKeyMap->addAction(act);

	act = new Action("MENUUP", _("Menu option up"));
	act->setCustomEngineActionEvent(kActionMenuOptionUp);
	act->addDefaultInputMapping("UP");
	gameKeyMap->addAction(act);

	act = new Action("MENUDOWN", _("Menu option down"));
	act->setCustomEngineActionEvent(kActionMenuOptionDown);
	act->addDefaultInputMapping("DOWN");
	gameKeyMap->addAction(act);


	KeymapArray keymaps(4);
	keymaps[0] = engineKeyMap;
	keymaps[1] = mouseKeyMap;
	keymaps[2] = gameKeyMap;
	keymaps[3] = introKeyMap;

	introKeyMap->setEnabled(false);

	return keymaps;
}

bool CineMetaEngine::removeSaveState(const char *target, int slot) const {
	if (slot < 0 || slot >= MAX_SAVEGAMES) {
		return false;
	}

	// Load savegame descriptions from index file
	typedef char CommandeType[SAVEGAME_NAME_LEN];
	CommandeType saveNames[MAX_SAVEGAMES];

	// Initialize all savegames' descriptions to empty strings
	// so that if the savegames' descriptions can only be partially read from file
	// then the missing ones are correctly set to empty strings.
	memset(saveNames, 0, sizeof(saveNames));

	Common::InSaveFile *in;
	in = g_system->getSavefileManager()->openForLoading(Common::String::format("%s.dir", target));

	if (!in)
		return false;

	in->read(saveNames, SAVELIST_SIZE);
	delete in;

	// Set description for selected slot
	char slotName[SAVEGAME_NAME_LEN];
	slotName[0] = 0;
	Common::strlcpy(saveNames[slot], slotName, SAVEGAME_NAME_LEN);

	// Update savegame descriptions
	Common::String indexFile = Common::String::format("%s.dir", target);
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(indexFile);
	if (!out) {
		warning("Unable to open file %s for saving", indexFile.c_str());
		return false;
	}

	out->write(saveNames, SAVELIST_SIZE);
	delete out;

	// Delete save file
	Common::String saveFileName = getSavegameFile(slot, target);

	return g_system->getSavefileManager()->removeSavefile(saveFileName);
}

#if PLUGIN_ENABLED_DYNAMIC(CINE)
	REGISTER_PLUGIN_DYNAMIC(CINE, PLUGIN_TYPE_ENGINE, CineMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CINE, PLUGIN_TYPE_ENGINE, CineMetaEngine);
#endif

namespace Cine {

Common::Error CineEngine::loadGameState(int slot) {
	bool gameLoaded = makeLoad(getSaveStateName(slot));

	return gameLoaded ? Common::kNoError : Common::kUnknownError;
}

Common::Error CineEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	if (slot < 0 || slot >= MAX_SAVEGAMES) {
		return Common::kCreatingFileFailed;
	}

	// Load savegame descriptions from index file
	loadSaveDirectory();

	// Set description for selected slot making sure it ends with a trailing zero
	strncpy(currentSaveName[slot], desc.c_str(), sizeof(CommandeType));
	currentSaveName[slot][sizeof(CommandeType) - 1] = 0;

	// Update savegame descriptions
	Common::String indexFile = _targetName + ".dir";

	Common::OutSaveFile *fHandle = _saveFileMan->openForSaving(indexFile);
	if (!fHandle) {
		warning("Unable to open file %s for saving", indexFile.c_str());
		return Common::kUnknownError;
	}

	fHandle->write(currentSaveName, SAVELIST_SIZE);
	delete fHandle;

	// Save game
	makeSave(getSaveStateName(slot), getTotalPlayTime() / 1000, desc, isAutosave);

	checkDataDisk(-1);

	return Common::kNoError;
}

Common::String CineEngine::getSaveStateName(int slot) const {
	return getMetaEngine()->getSavegameFile(slot, _targetName.c_str());
}

bool CineEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return (!disableSystemMenu && !inMenu);
}

bool CineEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return (allowPlayerInput && !disableSystemMenu && !inMenu);
}

} // End of namespace Cine
