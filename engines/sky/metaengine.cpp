/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/metaengine.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/system.h"
#include "common/savefile.h"
#include "common/translation.h"
#include "common/file.h"
#include "common/fs.h"

#include "sky/control.h"
#include "sky/sky.h"

class SkyMetaEngine : public MetaEngine {
    const char *getName() const override {
        return "sky";
    }

    bool hasFeature(MetaEngineFeature f) const override;

    Common::Error createInstance(OSystem *syst, Engine **engine) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;

    Common::KeymapArray initKeymaps(const char *target) const override;
};

bool SkyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
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

	act = new Action("LCLK", _("Walk / Look / Talk"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	mainKeymap->addAction(act);

	act = new Action("RCLK", _("Use"));
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

	act = new Action("FAST", _("Toggle fast mode"));
	act->setCustomEngineActionEvent(kSkyActionToggleFastMode);
	act->addDefaultInputMapping("C+f");
	shortcutsKeymap->addAction(act);

	act = new Action("RFAST", _("Toggle really fast mode"));
	act->setCustomEngineActionEvent(kSkyActionToggleReallyFastMode);
	act->addDefaultInputMapping("C+g");
	shortcutsKeymap->addAction(act);

	KeymapArray keymaps(2);
	keymaps[0] = mainKeymap;
	keymaps[1] = shortcutsKeymap;

	return keymaps;
}

Common::Error SkyMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);
	*engine = new Sky::SkyEngine(syst);
	return Common::kNoError;
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
			saveList.push_back(SaveStateDescriptor(slotNum,
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
	if (slot == 0)	// do not delete the auto save
		return;

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	char fName[20];
	sprintf(fName,"SKY-VM.%03d", slot);
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
	savenames[slot] = "";

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

bool SkyEngine::canLoadGameStateCurrently() {
	return _systemVars->pastIntro && _skyControl->loadSaveAllowed();
}

bool SkyEngine::canSaveGameStateCurrently() {
	return _systemVars->pastIntro && _skyControl->loadSaveAllowed();
}

} // End of namespace Sky
