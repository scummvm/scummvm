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

#include "engines/metaengine.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/gui_options.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "common/file.h"
#include "common/fs.h"

#include "gui/message.h"

#include "sky/control.h"
#include "sky/sky.h"

class SkyMetaEngine : public MetaEngine {
	const char *getName() const override {
		return "sky";
	}

	bool hasFeature(MetaEngineFeature f) const override;

	Common::Error createInstance(OSystem *syst, Engine **engine,
	                             const DetectedGame &gameDescriptor, const void *metaEngineDescriptor) override;

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	Common::KeymapArray initKeymaps(const char *target) const override;
	Common::String getSavegameFile(int saveGameIdx, const char *target) const override {
		if (saveGameIdx == kSavegameFilePattern)
			return Common::String::format("SKY-VM.###");
		else
			return Common::String::format("SKY-VM.%03d", saveGameIdx);
	}
};

bool SkyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo);
}

bool Sky::SkyEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::KeymapArray SkyMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Sky;

	Keymap *mainKeymap = new Keymap(Keymap::kKeymapTypeGame, "sky-main", "Beneath a Steel Sky");

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Walk / Look / Talk"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	mainKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Use"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	mainKeymap->addAction(act);

	act = new Action("CONFIRM", _("Confirm"));
	act->setCustomEngineActionEvent(kSkyActionConfirm);
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("KP_ENTER");
	mainKeymap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip / Close"));
	act->setCustomEngineActionEvent(kSkyActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	mainKeymap->addAction(act);

	Keymap *shortcutsKeymap = new Keymap(Keymap::kKeymapTypeGame, SkyEngine::shortcutsKeymapId, "Beneath a Steel Sky - Shortcuts");

	act = new Action(kStandardActionOpenMainMenu, _("Open control panel"));
	act->setCustomEngineActionEvent(kSkyActionOpenControlPanel);
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_X");
	shortcutsKeymap->addAction(act);

	act = new Action("SKPL", _("Skip line"));
	act->setCustomEngineActionEvent(kSkyActionSkipLine);
	act->addDefaultInputMapping("PERIOD");
	shortcutsKeymap->addAction(act);

	act = new Action(kStandardActionPause, _("Pause"));
	act->setCustomEngineActionEvent(kSkyActionPause);
	act->addDefaultInputMapping("p");
	shortcutsKeymap->addAction(act);

	act = new Action("FAST", _("Toggle fast mode on/off"));
	act->setCustomEngineActionEvent(kSkyActionToggleFastMode);
	act->addDefaultInputMapping("C+f");
	shortcutsKeymap->addAction(act);

	act = new Action("RFAST", _("Toggle really fast mode on/off"));
	act->setCustomEngineActionEvent(kSkyActionToggleReallyFastMode);
	act->addDefaultInputMapping("C+g");
	shortcutsKeymap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = mainKeymap;
	keymaps[1] = shortcutsKeymap;

	return keymaps;
}

Common::Error SkyMetaEngine::createInstance(OSystem *syst, Engine **engine,
	const DetectedGame &gameDescriptor, const void *metaEngineDescriptor) {
	assert(engine);
	*engine = new Sky::SkyEngine(syst);
	return Common::kNoError;
}

static const ExtraGuiOption skyExtraGuiOption = {
	_s("Floppy intro"),
	_s("Use the floppy version's intro (CD version only)"),
	"alt_intro",
	false,
	0,
	0
};

const ExtraGuiOptions SkyMetaEngine::getExtraGuiOptions(const Common::String &target) const {
	Common::String guiOptions;
	ExtraGuiOptions options;

	if (target.empty()) {
		options.push_back(skyExtraGuiOption);
		return options;
	}

	if (ConfMan.hasKey("guioptions", target)) {
		guiOptions = ConfMan.get("guioptions", target);
		guiOptions = parseGameGUIOptions(guiOptions);
	}

	if (!guiOptions.contains(GUIO_NOSPEECH))
		options.push_back(skyExtraGuiOption);
	return options;
}

SaveStateList SkyMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SaveStateList saveList;

	// Load the descriptions
	Common::StringArray savenames;
	savenames.resize(MAX_SAVE_GAMES+1);

	Common::InSaveFile *inf;
	inf = saveFileMan->openForLoading("SKY-VM.SAV");
	if (inf != NULL) {
		char *tmpBuf =  new char[MAX_SAVE_GAMES * MAX_TEXT_LEN];
		char *tmpPtr = tmpBuf;
		inf->read(tmpBuf, MAX_SAVE_GAMES * MAX_TEXT_LEN);
		for (int i = 0; i < MAX_SAVE_GAMES; ++i) {
			savenames[i] = tmpPtr;
			tmpPtr += savenames[i].size() + 1;
		}
		delete inf;
		delete[] tmpBuf;
	}

	// Find all saves
	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles("SKY-VM.###");

	// Prepare the list of savestates by looping over all matching savefiles
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Extract the extension
		Common::String ext = file->c_str() + file->size() - 3;
		ext.toUppercase();
		int slotNum = atoi(ext.c_str());
		Common::InSaveFile *in = saveFileMan->openForLoading(*file);
		if (in) {
			saveList.push_back(SaveStateDescriptor(this, slotNum,
				(slotNum == 0) ? _("Autosave") : Common::U32String(savenames[slotNum - 1])));
			delete in;
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int SkyMetaEngine::getMaximumSaveSlot() const { return MAX_SAVE_GAMES; }

void SkyMetaEngine::removeSaveState(const char *target, int slot) const {
	if (slot == 0)	{
		// Do not delete the auto save
		// Note: Setting the autosave slot as write protected (with setWriteProtectedFlag())
		//       does not disable the delete action on the slot.
		const Common::U32String message = _("WARNING: Deleting the autosave slot is not supported by this engine");
		GUI::MessageDialog warn(message);
		warn.runModal();
		return;
	}

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	char fName[20];
	Common::sprintf_s(fName,"SKY-VM.%03d", slot);
	saveFileMan->removeSavefile(fName);

	// Load current save game descriptions
	Common::StringArray savenames;
	savenames.resize(MAX_SAVE_GAMES+1);
	Common::InSaveFile *inf;
	inf = saveFileMan->openForLoading("SKY-VM.SAV");
	if (inf != NULL) {
		char *tmpBuf =  new char[MAX_SAVE_GAMES * MAX_TEXT_LEN];
		char *tmpPtr = tmpBuf;
		inf->read(tmpBuf, MAX_SAVE_GAMES * MAX_TEXT_LEN);
		for (int i = 0; i < MAX_SAVE_GAMES; ++i) {
			savenames[i] = tmpPtr;
			tmpPtr += savenames[i].size() + 1;
		}
		delete inf;
		delete[] tmpBuf;
	}

	// Update the save game description at the given slot
	savenames[slot - 1] = "";

	// Save the updated descriptions
	Common::OutSaveFile *outf;

	outf = saveFileMan->openForSaving("SKY-VM.SAV");
	bool ioFailed = true;
	if (outf) {
		for (uint16 cnt = 0; cnt < MAX_SAVE_GAMES; cnt++) {
			outf->write(savenames[cnt].c_str(), savenames[cnt].size() + 1);
		}
		outf->finalize();
		if (!outf->err())
			ioFailed = false;
		delete outf;
	}
	if (ioFailed)
		warning("Unable to store Savegame names to file SKY-VM.SAV. (%s)", saveFileMan->popErrorDesc().c_str());
}

SaveStateDescriptor SkyMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	if (slot > 0) {
		// Search current save game descriptions
		// for the description of the specified slot, if any
		Common::String tmpSavename;
		Common::InSaveFile *inf;
		inf = saveFileMan->openForLoading("SKY-VM.SAV");
		if (inf != NULL) {
			char *tmpBuf =  new char[MAX_SAVE_GAMES * MAX_TEXT_LEN];
			char *tmpPtr = tmpBuf;
			inf->read(tmpBuf, MAX_SAVE_GAMES * MAX_TEXT_LEN);
			for (int i = 0; i < MAX_SAVE_GAMES; ++i) {
				tmpSavename = tmpPtr;
				tmpPtr += tmpSavename.size() + 1;
				if (i == slot - 1) {
					break;
				}
			}
			delete inf;
			delete[] tmpBuf;
		}

		// Make sure the file exists
		// Note: there can be valid saved file names with empty savename
		char fName[20];
		Common::sprintf_s(fName,"SKY-VM.%03d", slot);
		Common::InSaveFile *in = saveFileMan->openForLoading(fName);
		if (in) {
			delete in;
			SaveStateDescriptor descriptor(this, slot, tmpSavename);
			return descriptor;
		}
	}

	// Reaching here, means we selected an empty save slot, that does not correspond to a save file
	SaveStateDescriptor emptySave;
	// Do not allow save slot 0 (used for auto-saving) to be overwritten.
	if (slot == 0) {
		emptySave.setAutosave(true);
		emptySave.setWriteProtectedFlag(true);
	} else {
		emptySave.setWriteProtectedFlag(false);
	}
	return emptySave;
}

#if PLUGIN_ENABLED_DYNAMIC(SKY)
	REGISTER_PLUGIN_DYNAMIC(SKY, PLUGIN_TYPE_ENGINE, SkyMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SKY, PLUGIN_TYPE_ENGINE, SkyMetaEngine);
#endif

namespace Sky {

Common::Error SkyEngine::loadGameState(int slot) {
	// We don't need to offset "slot" here. Both loadGameState and quickXRestore
	// are called with the ScummVM Save File Manager's "slot" as argument
	uint16 result = _skyControl->quickXRestore(slot);
	return (result == GAME_RESTORED) ? Common::kNoError : Common::kUnknownError;
}

/**
* Manually saving a game should save it into ScummVM Save File Managers slots 1 or greater.
* ScummVM Save file manager's slot 0 is reserved for the autosave.
* However, natively, the index 0 (_selectedGame) is the first manually saved game.
* @param slot is the save slot on the ScummVM file manager's list
*
*/
Common::Error SkyEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	// prevent writing to autosave slot when user selects it manually
	// ie. from the ScummVM in-game menu Save feature
	// This also secures _selectedGame which is unsigned integer (uint16)
	// from overflowing in the subtraction below
	if (slot < 0 || (!isAutosave && slot == 0)) {
		return Common::kWritePermissionDenied;
	}
	// Set the save slot and save the game
	// _selectedGame value is one unit lower than the ScummVM's Save File Manager's slot value
	// Note that *_selectedGame* value 0 corresponds to a manually saved game (the first in order)
	//   whereas *slot* value 0 corresponds to the autosave
	if (slot > 0) {
		// We don't care for updating the _selectedGame when slot == 0
		// (in the case of autosave) but we do include the check for slot > 0
		// to guard from overflow, which would be bad practice to allow.
		_skyControl->_selectedGame = slot - 1;
	}
	if (_skyControl->saveGameToFile(false, nullptr, isAutosave) != GAME_SAVED)
		return Common::kWritePermissionDenied;

	// Load current save game descriptions
	Common::StringArray saveGameTexts;
	saveGameTexts.resize(MAX_SAVE_GAMES+1);
	_skyControl->loadDescriptions(saveGameTexts);

	// Update the save game description at the given slot
	if (!isAutosave) {
		saveGameTexts[_skyControl->_selectedGame] = desc;
	}

	// Save the updated descriptions
	_skyControl->saveDescriptions(saveGameTexts);

	return Common::kNoError;
}

bool SkyEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return _systemVars->pastIntro
	    && _skyControl->loadSaveAllowed()
	    && !_skyControl->isControlPanelOpen();
}

bool SkyEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _systemVars->pastIntro
	    && _skyControl->loadSaveAllowed()
	    && !_skyControl->isControlPanelOpen();
}

} // End of namespace Sky
