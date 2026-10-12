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

#include "audio/mixer.h"

#include "engines/metaengine.h"
#include "engines/savestate.h"

#include "graphics/screen.h"

#include "gui/saveload.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/state/scene.h"

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
	takeAction(kActionShow, "SaveQueryDialog", 0.0);
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

// The saves get listed by their names; the second chance save isn't one of them
uint Manager::fillSaveList(const Common::String &listName) {
	takeAction(kActionSetText, listName, Common::String());

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
		if (action.value >= 0) {
			askToSaveFirst(action);
		} else if (!ConfMan.getBool("originalsaveload")) {
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
	case kActionSetText:
		// Save names must work as file names; a bad one gets undone
		if (!action.text.contains('<') && !action.text.contains('>') && !action.text.contains(':') && !action.text.contains('*') &&
				!action.text.contains('?') && !action.text.contains('"') && !action.text.contains('/') && !action.text.contains('\\') &&
				!action.text.contains('|') && !action.text.equalsIgnoreCase(kNewGameName)) {
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

		requestSave(_typedSaveName);
		_lastSaveName = _typedSaveName;
		_typedSaveName.clear();
		break;
	}
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
