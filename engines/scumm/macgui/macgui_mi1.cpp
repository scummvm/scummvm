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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/macresman.h"

#include "engines/engine.h"

#include "graphics/maccursor.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/surface.h"

#include "scumm/scumm.h"
#include "scumm/detection.h"
#include "scumm/macgui/macgui_impl.h"
#include "scumm/macgui/macgui_mi1.h"
#include "scumm/music.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"

namespace Scumm {

// ===========================================================================
// The Mac Monkey Island 1 GUI.
// ===========================================================================

MacMI1Gui::MacMI1Gui(ScummEngine *vm, const Common::Path &resourceFile) : MacGuiImpl(vm, resourceFile) {
	readStrings();
}

MacMI1Gui::~MacMI1Gui() {
}

void MacMI1Gui::readStrings() {
	Common::MacResManager resource;
	resource.open(_resourceFile);
	uint32 strsLen = resource.getResLength(MKTAG('S', 'T', 'R', 'S'), 0);

	if (strsLen <= 0)
		return;

	Common::SeekableReadStream *strsStream = resource.getResource(MKTAG('S', 'T', 'R', 'S'), 0);
	uint8 *strsBlock = (uint8 *)malloc(strsLen);
	strsStream->read(strsBlock, strsLen);

	uint8 *strsData = strsBlock;

	// Most of these are debug strings. We parse the entire STRS block anyway,
	// for any future need.

	// Debug strings
	for (int i = 0; i < 93; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	// "\x14", "About Monkey Island...<B;(-"
	for (int i = 0; i < 2; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "Are you sure you want to quit?", "Are you sure you want to quit?",
	// "Are you sure you want to restart this game from the beginning?",
	// "Are you sure you want to quit?"
	for (int i = 0; i < 4; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	// "macPixHead is NULL", "Monkey Island"
	for (int i = 0; i < 2; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "CopyBits error:"
	_strsStrings.emplace_back(readCString(strsData));

	// "Open Game File...", "Save Game File as..." "Game file"
	for (int i = 0; i < 3; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// "This disk is full.  The game was not saved."
	_strsStrings.emplace_back(readCString(strsData));

	// "An error occured while saving.  The game was not saved.  Please try saving the game to another disk."
	_strsStrings.emplace_back(readCString(strsData));

	// "At this size, you will not be able to fit the whole game on your screen."
	_strsStrings.emplace_back(readCString(strsData));

	// Debug strings
	for (int i = 0; i < 3; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	for (int i = 0; i < 2; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// All the "About" dialog strings
	for (int i = 0; i < 32; i++) {
		_strsStrings.emplace_back(readPascalString(strsData));
	}

	// Remaining debug strings
	for (int i = 0; i < 75; i++) {
		_strsStrings.emplace_back(readCString(strsData));
	}

	free(strsBlock);
	delete strsStream;
}

const Graphics::Font *MacMI1Gui::getFontByScummId(int32 id) {
	switch (id) {
	case 0:
		return getFont(kLoomFontLarge);

	default:
		error("MacMI1Gui::getFontByScummId: Invalid font id %d", id);
	}
}

bool MacMI1Gui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	return MacGuiImpl::getFontParams(fontId, id, size, slant);
}

void MacMI1Gui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	if (_vm->_game.id == GID_MONKEY) {
		_windowManager->replaceCursor(Graphics::MacGUIConstants::kMacCursorArrow);
	} else if (_vm->_game.version == 5) {
		Common::MacResManager resource;
		Graphics::MacCursor macCursor;

		resource.open(_resourceFile);

		Common::SeekableReadStream *curs = resource.getResource(MKTAG('C', 'U', 'R', 'S'), 128);

		if (macCursor.readFromStream(*curs)) {
			width = macCursor.getWidth();
			height = macCursor.getHeight();
			hotspotX = macCursor.getHotspotX();
			hotspotY = macCursor.getHotspotY();
			animate = 0;

			_windowManager->replaceCursor(Graphics::MacGUIConstants::kMacCursorCustom, &macCursor);
		}

		delete curs;
		resource.close();
	}
}

bool MacMI1Gui::handleMenu(int id, Common::String &name) {
	if (MacGuiImpl::handleMenu(id, name))
		return true;

	switch (id) {
	case 204:   // Fix color map
		break; // Do a no-op

	case 205:   // Options
		runOptionsDialog();
		break;

	case 206:   // Quit
		if (runQuitDialog())
			_vm->quitGame();
		break;

	default:
		warning("Unknown menu command: %d", id);
		break;
	}

	return false;
}

void MacMI1Gui::runAboutDialog() {
	// TODO
}

// A standard file picker dialog doesn't really make sense in ScummVM, so we
// make something that just looks similar to one.

bool MacMI1Gui::runOpenDialog(int &saveSlotToHandle) {
	Common::Rect bounds(88, 28, 448, 210);

	MacDialogWindow *window = createWindow(bounds);

	window->addButton(Common::Rect(254, 137, 334, 157), "Open", true);
	window->addButton(Common::Rect(254, 106, 334, 126), "Cancel", true);
	window->addButton(Common::Rect(254, 62, 334, 82), "Desktop", false);
	window->addButton(Common::Rect(254, 34, 334, 54), "Eject", false);

	window->drawDottedHLine(253, 93, 334);

	bool availSlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, availSlots, slotIds, ARRAYSIZE(availSlots));

	drawFakePathList(window, Common::Rect(14, 8, 232, 26), _folderIcon, "Monkey Island", Graphics::kTextAlignLeft);
	drawFakeDriveLabel(window, Common::Rect(242, 10, 340, 28), _hardDriveIcon, "ScummVM", Graphics::kTextAlignLeft);

	window->addListBox(Common::Rect(14, 31, 232, 161), savegameNames, true);

	window->setDefaultWidget(0);

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0 || clicked == 6) {
			saveSlotToHandle =
				window->getWidgetValue(6) < ARRAYSIZE(slotIds) ?
				slotIds[window->getWidgetValue(6)] : -1;
			ret = true;
			break;
		}

		if (clicked == 1)
			break;
	}

	delete window;
	return ret;
}

bool MacMI1Gui::runSaveDialog(int &saveSlotToHandle, Common::String &name) {
	Common::Rect bounds(110, 27, 470, 231);

	MacDialogWindow *window = createWindow(bounds);

	window->addButton(Common::Rect(254, 163, 334, 183), "Save", true);
	window->addButton(Common::Rect(254, 132, 334, 152), "Cancel", true);
	window->addButton(Common::Rect(254, 90, 334, 110), "New", false);
	window->addButton(Common::Rect(254, 62, 334, 82), "Desktop", false);
	window->addButton(Common::Rect(254, 34, 334, 54), "Eject", false);

	bool busySlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, busySlots, slotIds, ARRAYSIZE(busySlots));

	Common::String saveGameFileAsResStr = _strsStrings[103].c_str();
	Common::String gameFileResStr = _strsStrings[104].c_str();

	int firstAvailableSlot = -1;
	for (int i = 1; i < ARRAYSIZE(busySlots); i++) { // Skip the autosave slot
		if (!busySlots[i]) {
			firstAvailableSlot = i;
			break;
		}
	}

	drawFakePathList(window, Common::Rect(14, 8, 232, 26), _folderIcon, "Monkey Island", Graphics::kTextAlignLeft);
	drawFakeDriveLabel(window, Common::Rect(242, 10, 340, 28), _hardDriveIcon, "ScummVM", Graphics::kTextAlignLeft);

	window->addListBox(Common::Rect(14, 31, 232, 129), savegameNames, true, true);

	MacGuiImpl::MacEditText *editText = window->addEditText(Common::Rect(16, 159, 229, 175), gameFileResStr, true);

	Graphics::Surface *s = window->innerSurface();
	const Graphics::Font *font = getFont(kSystemFont);

	s->frameRect(Common::Rect(14, 156, 232, 178), kBlack);

	window->drawDottedHLine(253, 121, 334);

	font->drawString(s, saveGameFileAsResStr, 14, 138, 218, kBlack, Graphics::kTextAlignLeft, 4);

	window->setDefaultWidget(0);
	editText->selectAll();

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0) {
			ret = true;
			name = editText->getText();
			saveSlotToHandle = firstAvailableSlot;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == -2) {
			// Cycle through deferred actions
			for (uint i = 0; i < deferredActionsIds.size(); i++) {
				// Edit text widget
				if (deferredActionsIds[i] == 4) {
					MacGuiImpl::MacWidget *wid = window->getWidget(deferredActionsIds[i]);

					// Disable "Save" button when text is empty
					window->getWidget(0)->setEnabled(!wid->getText().empty());
				}
			}
		}
	}

	delete window;
	return ret;
}

bool MacMI1Gui::runOptionsDialog() {
	// Widgets:
	//
	// 0 - Okay button
	// 1 - Cancel button
	// 2 - Sound checkbox
	// 3 - Music checkbox
	// 4 - Picture (text speed background)
	// 5 - Picture (text speed handle)
	// 6 - Scrolling checkbox
	// 7 - Full Animation checkbox
	// 8 - Picture (music quality background)
	// 9 - Picture (music quality handle)
	// 10 - "Machine Speed:  ^0" text
	// 11 - Text speed slider (manually created)
	// 12 - Music quality slider (manually created)

	int sound = 1;
	int music = 1;
	if (_vm->VAR(167) == 2) {
		sound = music = 0;
	} else if (_vm->VAR(167) == 1) {
		music = 0;
	}

	int scrolling = _vm->_snapScroll == 0;
	int fullAnimation = _vm->VAR(_vm->VAR_MACHINE_SPEED) == 1 ? 0 : 1;
	int textSpeed = _vm->_defaultTextSpeed;
	int musicQuality = ConfMan.hasKey("mac_snd_quality") ? ConfMan.getInt("mac_snd_quality") : 0;
	int musicQualityOption = (musicQuality == 0) ? 1 : (musicQuality - 1) % 3;
	musicQuality = (musicQuality == 0) ? (_vm->VAR(_vm->VAR_SOUNDCARD) == 10 ? 0 : 2) : (musicQuality - 1) / 3;

	MacDialogWindow *window = createDialog(1000);

	window->setWidgetValue(2, sound);
	window->setWidgetValue(3, music);
	window->setWidgetValue(6, scrolling);
	window->setWidgetValue(7, fullAnimation);

	if (!sound)
		window->setWidgetEnabled(3, false);

	window->addPictureSlider(4, 5, true, 5, 105, 0, 9);
	window->setWidgetValue(11, textSpeed);

	window->addPictureSlider(8, 9, true, 5, 69, 0, 2, 6, 4);
	window->setWidgetValue(12, musicQualityOption);

	// Machine rating
	window->addSubstitution(Common::String::format("%d", _vm->VAR(53)));

	// When quitting, the default action is not to not apply options
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0) {
			ret = true;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == 2)
			window->setWidgetEnabled(3, window->getWidgetValue(2) != 0);
	}

	if (ret) {
		// Update settings

		// TEXT SPEED
		_vm->_defaultTextSpeed = CLIP<int>(window->getWidgetValue(11), 0, 9);
		ConfMan.setInt("original_gui_text_speed", _vm->_defaultTextSpeed);
		_vm->setTalkSpeed(_vm->_defaultTextSpeed);

		// SOUND&MUSIC ACTIVATION
		// 0 - Sound&Music on
		// 1 - Sound on, music off
		// 2 - Sound&Music off
		int musicVariableValue = 0;

		if (window->getWidgetValue(2) == 0) {
			musicVariableValue = 2;
		} else if (window->getWidgetValue(2) == 1 && window->getWidgetValue(3) == 0) {
			musicVariableValue = 1;
		}

		_vm->VAR(167) = musicVariableValue;

		if (musicVariableValue != 0) {
			if (_vm->VAR(169) != 0) {
				_vm->_sound->stopSound(_vm->VAR(169));
				_vm->VAR(169) = 0;
			}
		}

		// SCROLLING ACTIVATION
		_vm->_snapScroll = window->getWidgetValue(6) == 0;

		if (_vm->VAR_CAMERA_FAST_X != 0xFF)
			_vm->VAR(_vm->VAR_CAMERA_FAST_X) = _vm->_snapScroll;

		// FULL ANIMATION ACTIVATION
		_vm->VAR(_vm->VAR_MACHINE_SPEED) = window->getWidgetValue(7) == 1 ? 0 : 1;

		// MUSIC QUALITY SELECTOR
		musicQuality = musicQuality * 3 + 1 + window->getWidgetValue(12);
		_vm->_musicEngine->setQuality(musicQuality);
		ConfMan.setInt("mac_snd_quality", musicQuality);

		_vm->syncSoundSettings();
		ConfMan.flushToDisk();
	}

	delete window;
	return ret;
}

void MacMI1Gui::resetAfterLoad() {
	reset();
	_windowManager->replaceCursor(Graphics::MacGUIConstants::kMacCursorArrow);
}

bool MacMI1Gui::handleEvent(Common::Event event) {
	if (MacGuiImpl::handleEvent(event))
		return true;

	if (_vm->isPaused())
		return false;

	if (_vm->_userPut <= 0)
		return false;

	return false;
}

} // End of namespace Scumm
