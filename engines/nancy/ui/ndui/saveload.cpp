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

#include "audio/mixer.h"

#include "engines/metaengine.h"
#include "engines/savestate.h"

#include "graphics/screen.h"

#include "gui/saveload.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/script/types.h"

#include "engines/nancy/ui/ndui/manager.h"
#include "engines/nancy/ui/ndui/panel.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// The load dialog lists this as a save, for starting over
static const char *const kNewGameName = "_StartNewGame_";

// The picture of a save, as the save and load dialogs ask for it
static const char *const kScreenshotTexture = "Engine_LoadSave";
static const int kScreenshotWidth = 256;
static const int kScreenshotHeight = 154;

// Leaving the game or loading another one first asks whether to save, then goes
// on with what the player was doing
void Manager::askToSaveFirst(const Action &action) {
	_pendingAction = action;
	_pendingAction.hasValue = true;
	_pendingAction.value = -1;
	_hasPendingAction = true;

	if (!_isSavingAllowed && findPanel("ExitQueryDialog")) {
		takeAction(kActionShow, "ExitQueryDialog", 0.0);
	} else {
		takeAction(kActionShow, "SaveQueryDialog", 0.0);
	}
}

void Manager::runPendingAction() {
	if (_hasPendingAction) {
		_hasPendingAction = false;
		takeAction(_pendingAction);
	}
}

void Manager::onSaveQueryAction(const Action &action) {
	switch (action.type) {
	case kActionShow:
		takeAction(kActionHide, "SaveQueryDialog", 0.0);
		takeAction(kActionShow, "Engine_Saver", 0.0);
		break;
	case kActionHide:
		takeAction(kActionHide, "SaveQueryDialog", 0.0);
		runPendingAction();
		break;
	case kActionDisable:
		takeAction(kActionHide, "SaveQueryDialog", 0.0);
		_hasPendingAction = false;
		break;
	default:
		break;
	}
}

// The answer to whether to go on without saving
void Manager::onExitQueryAction(const Action &action) {
	switch (action.type) {
	case kActionEnable:
		takeAction(kActionHide, "ExitQueryDialog", 0.0);
		runPendingAction();
		break;
	case kActionDisable:
		takeAction(kActionHide, "ExitQueryDialog", 0.0);
		_hasPendingAction = false;
		break;
	default:
		break;
	}
}

// The saves get listed by their names; the second chance save isn't one of them
uint Manager::fillSaveList(const Common::String &listName) {
	takeAction(kActionSetText, listName, Common::String());

	// Resorting to Danger lists its profiles, the one saved last first
	if (g_nancy->getGameType() == kGameTypeNancyDossier2) {
		Common::Array<Common::String> names;
		getProfileNames(names);
		for (const Common::String &name : names) {
			takeAction(kActionAddText, listName, name);
		}

		return names.size();
	}

	uint numSaves = 0;
	SaveStateList saves = g_nancy->getMetaEngine()->listSaves(ConfMan.getActiveDomainName().c_str());
	for (const SaveStateDescriptor &save : saves) {
		if (save.getSaveSlot() == g_nancy->getMetaEngine()->getMaximumSaveSlot() || save.getSaveSlot() == g_nancy->getAutosaveSlot()) {
			continue;
		}

		takeAction(kActionAddText, listName, save.getDescription());
		++numSaves;
	}

	return numSaves;
}

// A name that is in use gets saved over; a new one gets the first free slot
void Manager::requestSave(const Common::String &name) {
	int slot = findSaveSlot(name);
	for (int i = 1; slot == -1 && i < g_nancy->getMetaEngine()->getMaximumSaveSlot(); ++i) {
		if (i != g_nancy->getAutosaveSlot() && !g_nancy->getMetaEngine()->querySaveMetaInfos(ConfMan.getActiveDomainName().c_str(), i).isValid()) {
			slot = i;
		}
	}

	if (slot != -1) {
		if (_request.type == kRequestLoad || _request.type == kRequestNewGame || _request.type == kRequestQuit) {
			_requestAfterSave = _request;
		}

		_request.type = kRequestSave;
		_request.slot = slot;
		_request.name = name;
	} else {
		warning("No free save slot for %s", name.c_str());
	}
}

void Manager::requestLoad(const Common::String &name) {
	int slot = findSaveSlot(name);
	if (slot != -1) {
		_request.type = kRequestLoad;
		_request.slot = slot;
	} else {
		warning("There is no saved game named %s", name.c_str());
	}
}

int Manager::findSaveSlot(const Common::String &name) const {
	SaveStateList saves = g_nancy->getMetaEngine()->listSaves(ConfMan.getActiveDomainName().c_str());
	for (const SaveStateDescriptor &save : saves) {
		if (save.getSaveSlot() != g_nancy->getMetaEngine()->getMaximumSaveSlot() && save.getSaveSlot() != g_nancy->getAutosaveSlot() &&
				save.getDescription().equalsIgnoreCase(name)) {
			return save.getSaveSlot();
		}
	}

	return -1;
}

bool Manager::deleteSave(const Common::String &name) {
	const int slot = findSaveSlot(name);
	return slot != -1 && g_nancy->getMetaEngine()->removeSaveState(ConfMan.getActiveDomainName().c_str(), slot);
}

// Gives a saved game another name, which is the description in the header at
// the end of the file; the rest stays as it was
bool Manager::renameSave(const Common::String &oldName, const Common::String &newName) {
	const int slot = findSaveSlot(oldName);
	if (slot == -1 || hasSave(newName)) {
		return false;
	}

	const Common::String fileName = g_nancy->getSaveStateName(slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);
	if (!in) {
		return false;
	}

	Common::Array<byte> data;
	data.resize(in->size());
	const bool isRead = in->read(data.data(), data.size()) == data.size();
	delete in;

	// The header: its id (6 bytes), version (1), date (4), time (2), play time
	// (4), then the description as a byte of length and its letters. The file
	// ends with where the header starts.
	const uint32 kDescriptionOffset = 6 + 1 + 4 + 2 + 4;
	if (!isRead || data.size() < 4) {
		return false;
	}

	const uint32 headerPos = READ_LE_UINT32(data.data() + data.size() - 4);
	if (headerPos + kDescriptionOffset + 1 > data.size() - 4 || memcmp(data.data() + headerPos, "SVMCR", 6) != 0) {
		return false;
	}

	const uint32 lengthPos = headerPos + kDescriptionOffset;
	const uint32 oldLength = data[lengthPos];
	if (lengthPos + 1 + oldLength > data.size() - 4) {
		return false;
	}

	const Common::String description = newName.substr(0, 0xFF);

	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(fileName);
	if (!out) {
		return false;
	}

	out->write(data.data(), lengthPos);
	out->writeByte((byte)description.size());
	out->writeString(description);
	out->write(data.data() + lengthPos + 1 + oldLength, data.size() - (lengthPos + 1 + oldLength));
	out->finalize();
	const bool isWritten = !out->err();
	delete out;
	return isWritten;
}

void Manager::getSaveNames(Common::Array<Common::String> &names) const {
	SaveStateList saves = g_nancy->getMetaEngine()->listSaves(ConfMan.getActiveDomainName().c_str());
	for (const SaveStateDescriptor &save : saves) {
		if (save.getSaveSlot() != g_nancy->getMetaEngine()->getMaximumSaveSlot() && save.getSaveSlot() != g_nancy->getAutosaveSlot()) {
			names.push_back(save.getDescription());
		}
	}
}

// The dialogs show a scaled down picture of the game
void Manager::setSaveScreenshot(const Graphics::ManagedSurface &image, const Common::Rect &src) {
	Graphics::ManagedSurface *texture = _textures.getValOrDefault(kScreenshotTexture, nullptr);
	if (!texture) {
		texture = new Graphics::ManagedSurface();
		_textures.setVal(kScreenshotTexture, texture);
	}

	texture->create(kScreenshotWidth, kScreenshotHeight, g_nancy->_graphics->getTransparentPixelFormat());
	texture->blitFrom(image, src, Common::Rect(kScreenshotWidth, kScreenshotHeight));
}

// The ScummVM dialog, when picked over the game's own. Returns the slot, or -1
// when the player backs out of it
int Manager::runScummVMSaveLoadDialog(bool isSave, Common::String &saveName) {
	PauseToken pause = g_nancy->pauseEngine();

	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(isSave);
	int slot = dialog->runModalWithCurrentTarget();
	saveName = dialog->getResultString();
	delete dialog;

	g_nancy->_input->forceCleanInput();

	return slot;
}

void Manager::onLoaderAction(const Action &action) {
	switch (action.type) {
	case kActionShow:
		// The profile screen of Resorting to Danger is the load dialog
		if (action.value >= 0) {
			askToSaveFirst(action);
		} else if (!ConfMan.getBool("originalsaveload") && g_nancy->getGameType() != kGameTypeNancyDossier2) {
			Common::String saveName;
			int slot = runScummVMSaveLoadDialog(false, saveName);
			if (slot >= 0) {
				_request.type = kRequestLoad;
				_request.slot = slot;
			}
		} else {
			takeAction(fillSaveList("LoadList") > 0 ? kActionEnable : kActionDisable, "LoadButton", 0.0);
			takeAction(kActionSetValue, "LoadList", 0.0);
			takeAction(kActionShow, "LoadDialog", 0.0);
		}

		return;
	case kActionSetText: {
		_selectedSave = action.text;
		int slot = findSaveSlot(_selectedSave);
		if (_selectedSave == kNewGameName || slot == -1) {
			return;
		}

		SaveStateDescriptor save = g_nancy->getMetaEngine()->querySaveMetaInfos(ConfMan.getActiveDomainName().c_str(), slot);
		takeAction(kActionSetText, "LoadGameDetails", save.getSaveDate() + " " + save.getSaveTime());

		if (save.getThumbnail()) {
			Graphics::ManagedSurface thumbnail;
			thumbnail.copyFrom(*save.getThumbnail());
			setSaveScreenshot(thumbnail, Common::Rect(thumbnail.w, thumbnail.h));
			takeAction(kActionSetTexture, "LoadScreenshot", kScreenshotTexture);
		}

		return;
	}
	case kActionSetValue:
		if (_selectedSave == kNewGameName) {
			_request.type = kRequestNewGame;
		} else {
			_request.type = kRequestLoad;
			_request.slot = findSaveSlot(_selectedSave);
		}

		// In Resorting to Danger, loading a game or starting a new one allows saving
		// again, which the main menu turns off
		if (g_nancy->getGameType() == kGameTypeNancyDossier2) {
			_isSavingAllowed = true;
		}

		_lastSaveName = _selectedSave;
		break;
	case kActionHide:
		break;
	default:
		return;
	}

	_selectedSave.clear();
	takeAction(kActionHide, "LoadDialog", 0.0);
	takeAction(kActionSetText, "LoadList", Common::String());
}

void Manager::onSaverAction(const Action &action) {
	switch (action.type) {
	case kActionShow: {
		if (!ConfMan.getBool("originalsaveload")) {
			Common::String saveName;
			int slot = runScummVMSaveLoadDialog(true, saveName);
			if (slot >= 0) {
				// A quit or load that was put off to save first is a request
				// of its own, so it waits for the save
				_request.type = kRequestSave;
				_request.slot = slot;
				_request.name = saveName;
				_request.hasFollowUpAction = true;
			} else {
				runPendingAction();
			}

			break;
		}

		fillSaveList("SaveList");
		_typedSaveName = _lastSaveName;

		Action setName;
		setName.type = kActionSetText;
		setName.target = "SaveName";
		setName.hasValue = true;
		setName.value = 1;
		setName.text = _lastSaveName;
		takeAction(setName);

		takeAction(kActionShow, "SaveDialog", 0.0);

		// The picture is of the viewport, as it was before the dialog came up
		const Graphics::Screen *screen = g_nancy->_graphics->getScreen();
		Graphics::ManagedSurface screenCopy;
		screenCopy.copyFrom(*screen);
		setSaveScreenshot(screenCopy, g_nancy->_graphics->gameToScreen(NancySceneState.getViewport().getScreenPosition()));
		takeAction(kActionSetTexture, "SaveScreenshot", kScreenshotTexture);
		break;
	}
	case kActionHide:
		takeAction(kActionHide, "SaveDialog", 0.0);
		takeAction(kActionSetText, "SaveList", Common::String());
		_typedSaveName.clear();
		runPendingAction();
		break;
	case kActionEnable:
		_isSavingAllowed = true;
		break;
	case kActionDisable:
		_isSavingAllowed = false;
		break;
	case kActionSetText:
		// Save names must work as file names; a bad one gets undone
		if (isValidSaveName(action.text)) {
			_typedSaveName = action.text;
		} else {
			takeAction(kActionSetText, "SaveName", _typedSaveName);
		}

		break;
	case kActionSetValue: {
		if (_typedSaveName.empty()) {
			break;
		}

		takeAction(kActionHide, "SaveDialog", 0.0);
		takeAction(kActionSetText, "SaveList", Common::String());

		// Nancy20 asks first before saving over a game of the same name. The
		// answer comes back with a value of 0.
		if (action.value > 0 && findPanel("SaveOverwriteDialog") && findSaveSlot(_typedSaveName) != -1) {
			takeAction(kActionShow, "SaveOverwriteDialog", 0.0);
			break;
		}

		requestSave(_typedSaveName);
		_lastSaveName = _typedSaveName;
		_typedSaveName.clear();
		break;
	}
	default:
		break;
	}
}

bool Manager::isValidSaveName(const Common::String &name) const {
	return !name.contains('<') && !name.contains('>') && !name.contains(':') && !name.contains('*') &&
		!name.contains('?') && !name.contains('"') && !name.contains('/') && !name.contains('\\') &&
		!name.contains('|') && !name.equalsIgnoreCase(kNewGameName);
}

// The profiles, the one played last first. The one of the arcade mode isn't
// one of a player, and neither are the saves Resorting to Danger makes when a
// player types a secret name.
void Manager::getProfileNames(Common::Array<Common::String> &names) const {
	const ScriptBootData *boot = (const ScriptBootData *)g_nancy->getEngineData("ScriptBootData");
	assert(boot);

	struct Profile {
		Common::String name;
		uint64 time = 0;
	};

	Common::Array<Profile> profiles;
	SaveStateList saves = g_nancy->getMetaEngine()->listSaves(ConfMan.getActiveDomainName().c_str());
	for (const SaveStateDescriptor &save : saves) {
		if (save.getSaveSlot() == g_nancy->getMetaEngine()->getMaximumSaveSlot() || save.getSaveSlot() == g_nancy->getAutosaveSlot() ||
				(!boot->arcadeSave.empty() && save.getDescription().equalsIgnoreCase(boot->arcadeSave))) {
			continue;
		}

		bool isSpecialSave = false;
		for (const Common::String &specialSave : boot->specialSaves) {
			isSpecialSave |= save.getDescription().equalsIgnoreCase(specialSave);
		}

		if (isSpecialSave) {
			continue;
		}

		Profile profile;
		profile.name = save.getDescription();

		Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(g_nancy->getSaveStateName(save.getSaveSlot()));
		ExtendedSavegameHeader header;
		if (file && MetaEngine::readSavegameHeader(file, &header)) {
			uint16 year;
			uint8 month, day, hour, minute;
			MetaEngine::decodeSavegameDate(&header, year, month, day);
			MetaEngine::decodeSavegameTime(&header, hour, minute);
			profile.time = ((((uint64)year * 13 + month) * 32 + day) * 24 + hour) * 60 + minute;
		}

		delete file;

		uint i = 0;
		while (i < profiles.size() && profiles[i].time >= profile.time) {
			++i;
		}

		profiles.insert_at(i, profile);
	}

	for (const Profile &profile : profiles) {
		names.push_back(profile.name);
	}
}

// The name of the player is kept by a record of the scene, which the scripts
// are compiled in for whenever this game runs
void Manager::sendToPlayerName(const char *action, const Common::String &text) {
#ifdef ENABLE_NANCY_V4
	const ScriptBootData *boot = (const ScriptBootData *)g_nancy->getEngineData("ScriptBootData");
	assert(boot);

	Script::Message message;
	message.action = action;
	message.target = boot->currentPlayerObject;
	message.sender = "Engine_Loader";
	message.text = text;

	bool wasTaken = false;
	NancySceneState.receiveMessage(message, wasTaken);
#endif
}

// Ten buttons show the profiles, the one being played disabled. The one played
// last becomes the profile when there is none yet; there being no profile at
// all lets the scripts ask for the first one.
void Manager::fillProfileList() {
	const ScriptBootData *boot = (const ScriptBootData *)g_nancy->getEngineData("ScriptBootData");
	assert(boot);

	const uint kNumProfileButtons = 10;

	_listedProfiles.clear();
	getProfileNames(_listedProfiles);
	if (_listedProfiles.empty()) {
		sendToPlayerName("enable", Common::String());
		return;
	}

	if (_listedProfiles.size() > kNumProfileButtons) {
		_listedProfiles.resize(kNumProfileButtons);
	}

	ScriptSavedData *saved = (ScriptSavedData *)NancySceneState.getPuzzleData(ScriptSavedData::getTag());
	for (uint i = 0; i < _listedProfiles.size(); ++i) {
		const Common::String button = boot->profileButtonPrefix + Common::String::format("%u", i);

		Action setName;
		setName.type = kActionSetText;
		setName.target = button;
		setName.hasValue = true;
		setName.text = _listedProfiles[i];
		takeAction(setName);

		if (i == 0 && !saved->values.contains(boot->currentPlayerObject)) {
			takeAction(kActionDisable, button, 0.0);

			sendToPlayerName("setText", _listedProfiles[i]);
		} else {
			takeAction(_listedProfiles[i].equalsIgnoreCase(g_nancy->_profile.current) ? kActionDisable : kActionEnable, button, 0.0);
		}
	}

	takeAction(_listedProfiles.size() < kNumProfileButtons ? kActionEnable : kActionDisable, boot->newProfileButton, 0.0);
	for (uint i = _listedProfiles.size(); i < kNumProfileButtons; ++i) {
		takeAction(kActionDisable, boot->profileButtonPrefix + Common::String::format("%u", i), 0.0);
	}
}

void Manager::onProfileLoaderAction(const Action &action) {
	const ScriptBootData *boot = (const ScriptBootData *)g_nancy->getEngineData("ScriptBootData");
	assert(boot);

	switch (action.type) {
	case kActionShow:
		fillProfileList();
		break;
	case kActionHide:
		_pickedProfile.clear();
		break;
	case kActionCollapse: {
		// Deletes the profile that was named, which the player isn't anymore
		const int slot = _pickedProfile.empty() ? -1 : findSaveSlot(_pickedProfile);
		if (slot != -1 && g_nancy->getMetaEngine()->removeSaveState(ConfMan.getActiveDomainName().c_str(), slot)) {
			_pickedProfile.clear();
			ScriptSavedData *saved = (ScriptSavedData *)NancySceneState.getPuzzleData(ScriptSavedData::getTag());
			saved->values.erase(boot->currentPlayerObject);
		}

		break;
	}
	case kActionSetText:
		_pickedProfile = action.text;
		break;
	case kActionSetValue:
		_isProfileLoadRequested = true;
		break;
	case kActionGetText:
		// A button of the list picks the profile it shows
		if (action.value >= 0 && action.value < _listedProfiles.size()) {
			Action setName;
			setName.type = kActionSetText;
			setName.target = "Engine_Loader";
			setName.hasValue = true;
			setName.text = _listedProfiles[(uint)action.value];
			takeAction(setName);
			takeAction(kActionSetValue, "Engine_Loader", 0.0);
		}

		break;
	default:
		break;
	}
}

// A profile that has no saved game yet is a new game, which gets saved under
// its name once it runs. "_StartNewGame_" starts one that isn't saved.
void Manager::runProfileLoad() {
	_isProfileLoadRequested = false;

	const ScriptBootData *boot = (const ScriptBootData *)g_nancy->getEngineData("ScriptBootData");
	assert(boot);

	NancyEngine::ProfileState &profile = g_nancy->_profile;
	const bool isNewGameName = _pickedProfile.equalsIgnoreCase(kNewGameName);
	const int slot = isNewGameName || _pickedProfile.empty() ? -1 : findSaveSlot(_pickedProfile);
	if (slot == -1) {
		profile.isNewProfileGame = true;
		profile.newProfile = isNewGameName ? Common::String() : _pickedProfile;
		_request.type = kRequestNewGame;
	} else {
		_request.type = kRequestLoad;
		_request.slot = slot;
	}

	// Coming from a profile of a player, the profiles show up again
	if (!profile.current.empty() && !profile.current.equalsIgnoreCase(boot->arcadeSave)) {
		profile.hasPendingPopup = true;
		profile.isPopupForArcade = _pickedProfile.equalsIgnoreCase(boot->arcadeSave);
	}

	profile.current = _pickedProfile;
	_pickedProfile.clear();
}

void Manager::onProfileSaverAction(const Action &action) {
	if (action.type == kActionSetText) {
		if (isValidSaveName(action.text)) {
			_profileToSave = action.text;
		}
	} else if (action.type == kActionSetValue && !_profileToSave.empty()) {
		requestSave(_profileToSave);
		g_nancy->_profile.current = _profileToSave;
		_profileToSave.clear();
	}
}

// The answer to whether to save over a game: yes saves, no goes back to the
// dialog for saving
void Manager::onSaveOverwriteAction(const Action &action) {
	switch (action.type) {
	case kActionEnable:
		takeAction(kActionHide, "SaveOverwriteDialog", 0.0);
		takeAction(kActionSetValue, "Engine_Saver", 0.0);
		break;
	case kActionDisable:
		takeAction(kActionHide, "SaveOverwriteDialog", 0.0);
		takeAction(kActionShow, "Engine_Saver", 0.0);
		break;
	default:
		break;
	}
}

void Manager::onGameWindowAction(const Action &action) {
	switch (action.type) {
	case kActionHide:
		// Quitting
		if (action.value >= 0) {
			askToSaveFirst(action);
		} else {
			_request.type = kRequestQuit;
		}

		break;
	case kActionSetValue:
		ConfMan.setInt("matte_color", (int)action.value);
		applyMatteColor();
		break;
	case kActionEnable:
	case kActionDisable:
		// The dialog that dims the screen disables the game window, which pauses
		// the game. Nancy16 does nothing for it.
		if (g_nancy->getGameType() >= kGameTypeNancy17) {
			NancySceneState.setGamePaused(action.type == kActionDisable);
		}

		break;
	case kActionQuery: {
		const Graphics::PixelFormat &format = g_nancy->_graphics->getTransparentPixelFormat();
		bool isCurrent = ConfMan.hasKey("matte_color") &&
			g_nancy->_graphics->getFontColor(ConfMan.getInt("matte_color"), format) == g_nancy->_graphics->getFontColor((uint)action.value, format);
		takeAction(kActionSetValue, action.sender, isCurrent ? 1.0 : 0.0);
		break;
	}
	default:
		break;
	}
}

// The matte is black until a font color gets picked for it
void Manager::applyMatteColor() {
	uint32 color = 0xFF000000;
	if (ConfMan.hasKey("matte_color")) {
		color = g_nancy->_graphics->getFontColor(ConfMan.getInt("matte_color"), kARGBFormat);
	}

	NancySceneState.getFrame().setBlankColor(color);
}

// The sliders go from 0 to 100, and have to read back what they set
void Manager::onVolumeAction(const Action &action, const char *configKey, uint16 sampleChannel) {
	if (action.type == kActionQuery) {
		takeAction(kActionSetValue, action.sender, (double)((ConfMan.getInt(configKey) * 100 + Audio::Mixer::kMaxMixerVolume / 2) / Audio::Mixer::kMaxMixerVolume));
	} else if (action.type == kActionSetValue) {
		ConfMan.setInt(configKey, (int)action.value * Audio::Mixer::kMaxMixerVolume / 100);
		g_nancy->syncSoundSettings();
	} else if (action.type == kActionSetText && g_nancy->getGameType() >= kGameTypeNancy19 && !action.text.empty()) {
		// Nancy19 names a sound to play once a slider is let go of
		SoundDescription sound;
		sound.name = action.text;
		sound.channelID = sampleChannel;
		sound.numLoops = 1;
		sound.volume = 100;

		g_nancy->_sound->loadSound(sound);
		g_nancy->_sound->playSound(sound);
	}
}

// The value is the event flag
void Manager::onFlagsAction(const Action &action) {
	int16 label = (int16)action.value;

	switch (action.type) {
	case kActionEnable:
		NancySceneState.setEventFlag(label, g_nancy->_true);
		break;
	case kActionDisable:
		NancySceneState.setEventFlag(label, g_nancy->_false);
		break;
	case kActionToggleEnabled:
		NancySceneState.setEventFlag(label, NancySceneState.getEventFlag(label, g_nancy->_true) ? g_nancy->_false : g_nancy->_true);
		break;
	case kActionQuery:
		takeAction(kActionSetValue, action.sender, NancySceneState.getEventFlag(label, g_nancy->_true) ? 1.0 : 0.0);
		break;
	default:
		break;
	}
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
