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

#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "tinsel/bmv.h"
#include "tinsel/cursor.h"
#include "tinsel/tinsel.h"
#include "tinsel/savescn.h"	// needed by TinselMetaEngine::

#include "tinsel/detection.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_CROP_HEIGHT_480_TO_432,
		{
			_s("Remove Black Bars"),
			_s("The game originally renders at 640x432 which is then presented letterboxed at 640x480. Enabling this option removes the forced letterbox effect, so that the game fits better on screens with an aspect ratio wider than 4:3."),
			"crop_black_bars",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

namespace Tinsel {

uint32 TinselEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 TinselEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Language TinselEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform TinselEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 TinselEngine::getVersion() const {
	return _gameDescription->version;
}

bool TinselEngine::getIsADGFDemo() const {
	return (bool)(_gameDescription->desc.flags & ADGF_DEMO);
}

bool TinselEngine::isV1CD() const {
	return (bool)(_gameDescription->desc.flags & ADGF_CD);
}

} // End of namespace Tinsel

class TinselMetaEngine : public AdvancedMetaEngine<Tinsel::TinselGameDescription> {
public:
	const char *getName() const  override{
		return "tinsel";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const Tinsel::TinselGameDescription *desc) const override;

	bool hasFeature(MetaEngineFeature f) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	bool removeSaveState(const char *target, int slot) const override;

	// TODO: Add getSavegameFile(). See comments in loadGameState and removeSaveState
	
	void registerDefaultSettings(const Common::String &target) const override;
	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
};

bool TinselMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesSupportCreationDate);
}

bool Tinsel::TinselEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime);
}

SaveStateDescriptor TinselMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName;
	fileName = Common::String::format("%s.%03u", target, slot);

	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (!file) {
		return SaveStateDescriptor();
	}

	file->readUint32LE();		// skip id
	file->readUint32LE();		// skip size
	uint32 ver = file->readUint32LE();
	char saveDesc[SG_DESC_LEN];
	file->read(saveDesc, sizeof(saveDesc));

	saveDesc[SG_DESC_LEN - 1] = 0;
	SaveStateDescriptor desc(this, slot, saveDesc);

	int8 tm_year = file->readUint16LE();
	int8 tm_mon = file->readSByte();
	int8 tm_mday = file->readSByte();
	int8 tm_hour = file->readSByte();
	int8 tm_min = file->readSByte();
	file->readSByte(); // skip secs

	desc.setSaveDate(1900 + tm_year, 1 + tm_mon, tm_mday);
	desc.setSaveTime(tm_hour, tm_min);

	if (ver >= 3) {
		uint32 playTime = file->readUint32LE(); // playTime in seconds
		desc.setPlayTime(playTime);
	}

	delete file;
	return desc;
}

namespace Tinsel {
extern int getList(Common::SaveFileManager *saveFileMan, const Common::String &target);
}

SaveStateList TinselMetaEngine::listSaves(const char *target) const {
	Common::String pattern = target;
	pattern = pattern + ".###";
	Common::StringArray files = g_system->getSavefileManager()->listSavefiles(pattern);

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = files.begin(); file != files.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		const Common::String &fname = *file;
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fname);
		if (in) {
			in->readUint32LE();		// skip id
			in->readUint32LE();		// skip size
			in->readUint32LE();		// skip version
			char saveDesc[SG_DESC_LEN];
			in->read(saveDesc, sizeof(saveDesc));

			saveDesc[SG_DESC_LEN - 1] = 0;

			saveList.push_back(SaveStateDescriptor(this, slotNum, saveDesc));
			delete in;
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

Common::Error TinselMetaEngine::createInstance(OSystem *syst, Engine **engine, const Tinsel::TinselGameDescription *desc) const {
	*engine = new Tinsel::TinselEngine(syst,desc);
	return Common::kNoError;
}

int TinselMetaEngine::getMaximumSaveSlot() const { return 99; }

bool TinselMetaEngine::removeSaveState(const char *target, int slot) const {
	Tinsel::setNeedLoad();
	// Same issue here as with loadGameState(): we need the physical savegame
	// slot. Refer to bug #5819.
	int listSlot = -1;
	const int numStates = Tinsel::getList(g_system->getSavefileManager(), target);
	for (int i = 0; i < numStates; ++i) {
		const char *fileName = Tinsel::ListEntry(i, Tinsel::LE_NAME);
		const int saveSlot = atoi(fileName + strlen(fileName) - 3);

		if (saveSlot == slot) {
			listSlot = i;
			break;
		}
	}

	bool success = g_system->getSavefileManager()->removeSavefile(Tinsel::ListEntry(listSlot, Tinsel::LE_NAME));
	Tinsel::setNeedLoad();
	Tinsel::getList(g_system->getSavefileManager(), target);
	return success;
}

void TinselMetaEngine::registerDefaultSettings(const Common::String &target) const {
	ConfMan.registerDefault("crop_black_bars", false); // show the black bars by default (original behaviour)
}

const ADExtraGuiOptionsMap *TinselMetaEngine::getAdvancedExtraGuiOptions() const {
	return optionsList;
}

Common::KeymapArray TinselMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Tinsel;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "tinsel-default", _("Default keymappings"));
	Keymap *gameKeyMap = new Keymap(Keymap::kKeymapTypeGame, "game-shortcuts", _("Game keymappings"));
	Keymap *saveloadKeyMap = new Keymap(Keymap::kKeymapTypeGame, "saveload-shortcuts", _("Save/Load keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Action("ESCAPE", _("Escape"));
	act->setCustomEngineActionEvent(kActionEscape);
	act->addDefaultInputMapping("ESCAPE");
	engineKeyMap->addAction(act);

	act = new Action("MOVEUP", _("Move up"));
	act->setCustomEngineActionEvent(kActionMoveUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("KP8");
	gameKeyMap->addAction(act);

	act = new Action("MOVEDOWN", _("Move down"));
	act->setCustomEngineActionEvent(kActionMoveDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("KP2");
	gameKeyMap->addAction(act);

	act = new Action("MOVELEFT", _("Move left"));
	act->setCustomEngineActionEvent(kActionMoveLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("KP4");
	gameKeyMap->addAction(act);

	act = new Action("MOVERIGHT", _("Move right"));
	act->setCustomEngineActionEvent(kActionMoveRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("KP6");
	gameKeyMap->addAction(act);

	// I18N: Walk to where cursor points
	act = new Action("WALK", _("Walk to"));
	act->setCustomEngineActionEvent(kActionWalkTo);
	act->addDefaultInputMapping("SPACE");
	gameKeyMap->addAction(act);

	// I18N: Perform action on object where cursor points
	act = new Action("ACTION", _("Action"));
	act->setCustomEngineActionEvent(kActionAction);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	gameKeyMap->addAction(act);

	act = new Action("LOOK", _("Look"));
	act->setCustomEngineActionEvent(kActionLook);
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("LCTRL");
	act->addDefaultInputMapping("RCTRL");
	gameKeyMap->addAction(act);

	act = new Action("OPTIONSDIALOG", _("Options Dialog"));
	act->setCustomEngineActionEvent(kActionOptionsDialog);
	act->addDefaultInputMapping("F1");
	act->addDefaultInputMapping("1");
	gameKeyMap->addAction(act);

	act = new Action("INVENTORY", _("Inventory"));
	act->setCustomEngineActionEvent(kActionInventory);
	act->addDefaultInputMapping("F2");
	gameKeyMap->addAction(act);

	act = new Action("NOTEBOOK", _("Notebook"));
	act->setCustomEngineActionEvent(kActionNotebook);
	act->addDefaultInputMapping("F3");
	gameKeyMap->addAction(act);

	act = new Action("SAVE", _("Save"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("5");
	gameKeyMap->addAction(act);

	act = new Action("LOAD", _("Load"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("7");
	act->addDefaultInputMapping("F7");
	gameKeyMap->addAction(act);

	act = new Action("QUIT", _("Quit"));
	act->setCustomEngineActionEvent(kActionQuit);
	act->addDefaultInputMapping("C+q");
	act->addDefaultInputMapping("A+q");
	gameKeyMap->addAction(act);

	act = new Action("PGUPMENU", _("Page Up in save/load menu"));
	act->setCustomEngineActionEvent(kActionPageUp);
	act->addDefaultInputMapping("PAGEUP");
	act->addDefaultInputMapping("KP9");
	saveloadKeyMap->addAction(act);

	act = new Action("PGDOWNMENU", _("Page down in save/load menu"));
	act->setCustomEngineActionEvent(kActionPageDown);
	act->addDefaultInputMapping("PAGEDOWN");
	act->addDefaultInputMapping("KP3");
	saveloadKeyMap->addAction(act);

	act = new Action("STARTOFMENU", _("Go to start of save/load menu"));
	act->setCustomEngineActionEvent(kActionHome);
	act->addDefaultInputMapping("HOME");
	act->addDefaultInputMapping("KP7");
	saveloadKeyMap->addAction(act);

	act = new Action("ENDOFMENU", _("Go to end of save/load menu"));
	act->setCustomEngineActionEvent(kActionEnd);
	act->addDefaultInputMapping("END");
	act->addDefaultInputMapping("KP1");
	saveloadKeyMap->addAction(act);

	KeymapArray keymaps(3);
	keymaps[0] = engineKeyMap;
	keymaps[1] = gameKeyMap;
	keymaps[2] = saveloadKeyMap;
	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(TINSEL)
	REGISTER_PLUGIN_DYNAMIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TINSEL, PLUGIN_TYPE_ENGINE, TinselMetaEngine);
#endif

namespace Tinsel {

Common::Error TinselEngine::loadGameState(int slot) {
	// FIXME: Hopefully this is only used when loading games via
	// the launcher, since we do a hacky savegame slot to savelist
	// entry mapping here.
	//
	// You might wonder why is needed and here is the answer:
	// The save/load dialog of the GMM operates with the physical
	// savegame slots, while Tinsel internally uses entry numbers in
	// a savelist (which is sorted latest to first). Now to allow
	// proper loading of (especially Discworld2) saves we need to
	// get a savelist entry number instead of the physical slot.
	//
	// There are different possible solutions:
	//
	// One way to fix this would be to pass the filename instead of
	// the savelist entry number to RestoreGame, though it could make
	// problems how DW2 handles CD switches. Normally DW2 would pass
	// '-2' as slot when it changes CDs.
	//
	// Another way would be to convert all of Tinsel to use physical
	// slot numbers instead of savelist entry numbers for loading.
	// This would also allow '-2' as slot for CD changes without
	// any major hackery.

	int listSlot = -1;
	const int numStates = Tinsel::getList();
	for (int i = 0; i < numStates; ++i) {
		const char *fileName = Tinsel::ListEntry(i, Tinsel::LE_NAME);
		const int saveSlot = atoi(fileName + strlen(fileName) - 3);

		if (saveSlot == slot) {
			listSlot = i;
			break;
		}
	}

	if (listSlot == -1)
		return Common::kUnknownError;	// TODO: proper error code

	RestoreGame(listSlot);
	return Common::kNoError;	// TODO: return success/failure
}

#if 0
Common::Error TinselEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String saveName = _vm->getSavegameFilename((int16)(slot + 1));
	char saveDesc[SG_DESC_LEN];
	Common::strlcpy(saveDesc, desc, SG_DESC_LEN);
	SaveGame((char *)saveName.c_str(), saveDesc);
	ProcessSRQueue();			// This shouldn't be needed, but for some reason it is...
	return Common::kNoError;	// TODO: return success/failure
}
#endif

bool TinselEngine::canLoadGameStateCurrently(Common::U32String *msg) { return !_bmv->MoviePlaying(); }

#if 0
bool TinselEngine::canSaveGameStateCurrently(Common::U32String *msg) { return isCursorShown(); }
#endif

} // End of namespace Tinsel
