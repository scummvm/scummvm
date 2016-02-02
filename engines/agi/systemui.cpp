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

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/text.h"
#include "agi/keyboard.h"
#include "agi/systemui.h"

namespace Agi {

SystemUI::SystemUI(AgiEngine *vm, GfxMgr *gfx, TextMgr *text) {
	_vm = vm;
	_gfx = gfx;
	_text = text;

	_textStatusScore = "Score:%v3 of %v7";
	_textStatusSoundOn = "Sound:on";
	_textStatusSoundOff = "Sound:off";

	_textPause = "      Game paused.\nPress Enter to continue.";
	_textRestart = "Press ENTER to restart\nthe game.\n\nPress ESC to continue\nthis game.";
	_textQuit = "Press ENTER to quit.\nPress ESC to keep playing.";

	_textInventoryYouAreCarrying = "You are carrying:";
	_textInventoryNothing = "nothing";
	_textInventorySelectItems = "Press ENTER to select, ESC to cancel";
	_textInventoryReturnToGame = "Press a key to return to the game";

	_textSaveGameSelectSlot = "Use the arrow keys to select the slot in which you wish to save the game. Press ENTER to save in the slot, ESC to not save a game.";
	_textSaveGameEnterDescription = "How would you like to describe this saved game?\n\n";
	_textSaveGameVerify = "About to save the game\ndescribed as:\n\n%s\n\nin file:\n%s\n\nPress ENTER to continue.\nPress ESC to cancel.";

	_textRestoreGameNoSlots = "There are no games to\nrestore in\n\n ScummVM saved game directory\n\nPress ENTER to continue.";
	_textRestoreGameSelectSlot = "Use the arrow keys to select the game which you wish to restore. Press ENTER to restore the game, ESC to not restore a game.";
	_textRestoreGameError = "Error in restoring game.\nPress ENTER to quit.";
	_textRestoreGameVerify = "About to restore the game\ndescribed as:\n\n%s\n\nfrom file:\n%s\n\nPress ENTER to continue.\nPress ESC to cancel.";

	// Replace with translated text, when needed
	switch (_vm->getLanguage()) {
	case Common::RU_RUS:
		_textStatusScore = "\x91\xE7\xA5\xE2: %v3 \xA8\xA7 %v7";
		_textStatusSoundOn = "\x87\xA2\xE3\xAA: \xA2\xAA\xAB";
		_textStatusSoundOff = "\x87\xA2\xE3\xAA: \xA2\xEB\xAA\xAB";

		_textPause = "  \x88\xA3\xE0\xA0 \xAE\xE1\xE2\xA0\xAD\xAE\xA2\xAB\xA5\xAD\xA0\nENTER - \xAF\xE0\xAE\xA4\xAE\xAB\xA6\xA5\xAD\xA8\xA5.";
		//_textPause = "  \x88\xa3\xe0\xa0 \xae\xe1\xe2\xa0\xad\xae\xa2\xab\xa5\xad\xa0.  \n\n\n"; <- mouse
		// pause button text "\x8f\xe0\xae\xa4\xae\xab\xa6\xa8\xe2\xec"
		_textRestart = "ENTER - \xAF\xA5\xE0\xA5\xA7\xA0\xAF\xE3\xE1\xE2\xA8\xE2\xEC \xA8\xA3\xE0\xE3.\n\nESC - \xAF\xE0\xAE\xA4\xAE\xAB\xA6\xA5\xAD\xA8\xA5 \xA8\xA3\xE0\xEB.";
		_textQuit = "ENTER-\xA2\xEB\xE5\xAE\xA4 \xA8\xA7 \xA8\xA3\xE0\xEB.\nESC - \xA8\xA3\xE0\xA0\xE2\xEC \xA4\xA0\xAB\xEC\xE8\xA5.";

		_textInventoryYouAreCarrying = "   \x93 \xA2\xA0\xE1 \xA5\xE1\xE2\xEC\x3A   ";
		_textInventoryNothing = "\xAD\xA8\xE7\xA5\xA3\xAE";
		_textInventorySelectItems = "ENTER - \xA2\xEB\xA1\xAE\xE0, ESC - \xAE\xE2\xAC\xA5\xAD\xA0.";
		_textInventoryReturnToGame = "\x8B\xEE\xA1\xA0\xEF \xAA\xAB\xA0\xA2\xA8\xE8\xA0 - \xA2\xAE\xA7\xA2\xE0\xA0\xE2 \xA2 \xA8\xA3\xE0\xE3";

		_textSaveGameSelectSlot = "\x91 \xAF\xAE\xAC\xAE\xE9\xEC\xEE \xAA\xAB\xA0\xA2\xA8\xE8 \xAA\xE3\xE0\xE1\xAE\xE0\xA0 \xA2\xEB\xA1\xA5\xE0\xA8\xE2\xA5 \xE1\xE2\xE0\xAE\xAA\xE3, \xA2 \xAA\xAE\xE2\xAE\xE0\xE3\xEE \xA2\xEB \xA6\xA5\xAB\xA0\xA5\xE2\xA5 \xA7\xA0\xAF\xA8\xE1\xA0\xE2\xEC \xA8\xA3\xE0\xE3. \x8D\xA0\xA6\xAC\xA8\xE2\xA5 ENTER \xA4\xAB\xEF \xA7\xA0\xAF\xA8\xE1\xA8 \xA8\xA3\xE0\xEB, ESC - \xAE\xE2\xAC\xA5\xAD\xA0 \xA7\xA0\xAF\xA8\xE1\xA8.";
		_textSaveGameEnterDescription = "\x8A\xA0\xAA \xA2\xEB \xA6\xA5\xAB\xA0\xA5\xE2\xA5 \xAD\xA0\xA7\xA2\xA0\xE2\xEC \xED\xE2\xE3 \xA7\xA0\xAF\xA8\xE1\xEB\xA2\xA0\xA5\xAC\xE3\xEE \xA8\xA3\xE0\xE3?\n\n";
		_textSaveGameVerify = "\x83\xAE\xE2\xAE\xA2 \xAA \xA7\xA0\xAF\xA8\xE1\xA8 \xA8\xA3\xE0\xEB, \n\xAE\xAF\xA8\xE1\xA0\xAD\xAD\xAE\xA9 \xAA\xA0\xAA:\n\n%s\n\n\xA2 \xE4\xA0\xA9\xAB:\n%s\n\n\x84\xAB\xEF \xAF\xE0\xAE\xA4\xAE\xAB\xA6\xA5\xAD\xA8\xEF \xAD\xA0\xA6\xAC\xA8\xE2\xA5 ENTER.\nESC - \xAE\xE2\xAC\xA5\xAD\xA0.";

		_textRestoreGameNoSlots = "\x82 \xAA\xA0\xE2\xA0\xAB\xAE\xA3\xA5\n\n ScummVM saved game directory\n\n\xAD\xA5\xE2 \xA7\xA0\xAF\xA8\xE1\xA0\xAD\xAD\xEB\xE5 \xA8\xA3\xE0.\n\nENTER - \xAF\xE0\xAE\xA4\xAE\xAB\xA6\xA5\xAD\xA8\xA5.";
		_textRestoreGameSelectSlot = "\x91 \xAF\xAE\xAC\xAE\xE9\xEC\xEE \xAA\xAB\xA0\xA2\xA8\xE8 \xAA\xE3\xE0\xE1\xAE\xE0\xA0 \xA2\xEB\xA1\xA5\xE0\xA8\xE2\xA5 \xA8\xA3\xE0\xE3, \xAA\xAE\xE2\xAE\xE0\xE3\xEE \xA2\xEB \xA6\xA5\xAB\xA0\xA5\xE2\xA5 \xE1\xE7\xA8\xE2\xA0\xE2\xEC. \x8D\xA0\xA6\xAC\xA8\xE2\xA5 ENTER \xA4\xAB\xEF \xE1\xE7\xA8\xE2\xEB\xA2\xA0\xAD\xA8\xEF \xA8\xA3\xE0\xEB, ESC - \xA4\xAB\xEF \xAE\xE2\xAC\xA5\xAD\xEB.";
		_textRestoreGameError = "\x8E\xE8\xA8\xA1\xAA\xA0 \xA2 \xA7\xA0\xAF\xA8\xE1\xA0\xAD\xAD\xAE\xA9 \xA8\xA3\xE0\xA5.\nENTER - \xA2\xEB\xE5\xAE\xA4.";
		_textRestoreGameVerify = "\x83\xAE\xE2\xAE\xA2 \xAA \xE1\xE7\xA8\xE2\xEB\xA2\xA0\xAD\xA8\xEE \xA8\xA3\xE0\xEB\x2C\n\xAE\xAF\xA8\xE1\xA0\xAD\xAD\xAE\xA9 \xAA\xA0\xAA.\n\n%s\n\n\xA8\xA7 \xE4\xA0\xA9\xAB\xA0:\n%s\n\n\x84\xAB\xEF \xAF\xE0\xAE\xA4\xAE\xAB\xA6\xA5\xAD\xA8\xEF \xAD\xA0\xA6\xAC\xA8\xE2\xA5 ENTER.\nESC - \xAE\xE2\xAC\xA5\xAD\xA0.";
		break;
	default:
		break;
	}
}

SystemUI::~SystemUI() {
	clearSavedGameSlots();
}

const char *SystemUI::getStatusTextScore() {
	return _textStatusScore;
}
const char *SystemUI::getStatusTextSoundOn() {
	return _textStatusSoundOn;
}
const char *SystemUI::getStatusTextSoundOff() {
	return _textStatusSoundOff;
}

void SystemUI::pauseDialog() {
	_vm->_text->messageBox(_textPause);
}

bool SystemUI::restartDialog() {
	return _vm->_text->messageBox(_textRestart);
}

bool SystemUI::quitDialog() {
	return _vm->_text->messageBox(_textQuit);
}

const char *SystemUI::getInventoryTextNothing() {
	return _textInventoryNothing;
}
const char *SystemUI::getInventoryTextYouAreCarrying() {
	return _textInventoryYouAreCarrying;
}
const char *SystemUI::getInventoryTextSelectItems() {
	return _textInventorySelectItems;
}
const char *SystemUI::getInventoryTextReturnToGame() {
	return _textInventoryReturnToGame;
}

int16 SystemUI::figureOutAutomaticSaveGameSlot(const char *automaticSaveDescription) {
	int16 matchedGameSlotId = -1;
	int16 freshGameSlotId   = -1;

	// Fill saved game slot cache
	readSavedGameSlots(false, false); // don't filter, but also don't include auto-save slot

	// Walk through saved game slots
	// check, if description matches and return the slot
	// if no match can be found, return the first non-existant slot
	// if all slots exist, return -1
	figureOutAutomaticSavedGameSlot(automaticSaveDescription, matchedGameSlotId, freshGameSlotId);

	if (matchedGameSlotId >= 0)
		return matchedGameSlotId; // return matched slot

	if (freshGameSlotId >= 0)
		return freshGameSlotId; // return first non-existant slot

	return -1; // no slots exist, not match found
}

int16 SystemUI::figureOutAutomaticRestoreGameSlot(const char *automaticSaveDescription) {
	int16 matchedGameSlotId = -1;
	int16 freshGameSlotId   = -1;

	// Fill saved game slot cache
	readSavedGameSlots(true, false); // filter non-existant/invalid saves, also don't include auto-save slot

	// Walk through saved game slots
	// check, if description matches and return the slot. Otherwise return -1
	figureOutAutomaticSavedGameSlot(automaticSaveDescription, matchedGameSlotId, freshGameSlotId);

	if (matchedGameSlotId >= 0)
		return matchedGameSlotId; // return matched slot
	return -1; // no match found
}

int16 SystemUI::askForSaveGameSlot() {
	int16 saveGameSlotNr = -1;

	// Fill saved game slot cache
	readSavedGameSlots(false, false); // don't filter, but also don't include auto-save slot

	saveGameSlotNr = askForSavedGameSlot(_textSaveGameSelectSlot);

	if (saveGameSlotNr < 0) {
		// User cancelled? exit now
		return -1;
	}

	// return actual slot number of the saved game
	return _savedGameArray[saveGameSlotNr].slotId;
}

bool SystemUI::askForSaveGameDescription(int16 slotId, Common::String &newDescription) {
	// Let user enter new description
	bool previousEditState = _text->inputGetEditStatus();
	byte previousEditCursor = _text->inputGetCursorChar();

	_text->drawMessageBox(_textSaveGameEnterDescription, 0, 31, true);

	_text->charPos_Push();
	_text->charAttrib_Push();
	_text->inputEditOn();

	_text->charPos_SetInsideWindow(3, 0);
	_text->charAttrib_Set(15, 0);
	_text->clearBlockInsideWindow(3, 0, 31, 0); // input line is supposed to be black
	_text->inputSetCursorChar('_');

	// figure out the current description of the slot
	_text->stringSet("");
	for (uint16 slotNr = 0; slotNr < _savedGameArray.size(); slotNr++) {
		if (_savedGameArray[slotNr].slotId == slotId) {
			// found slotId
			if (_savedGameArray[slotNr].isValid) {
				// and also valid, so use its description
				_text->stringSet(_savedGameArray[slotNr].description);
			}
		}
	}

	_vm->cycleInnerLoopActive(CYCLE_INNERLOOP_GETSTRING);
	_text->stringEdit(30); // only allow up to 30 characters

	_text->charAttrib_Pop();
	_text->charPos_Pop();
	_text->inputSetCursorChar(previousEditCursor);
	if (!previousEditState) {
		_text->inputEditOff();
	}

	_text->closeWindow();

	if (!_text->stringWasEntered()) {
		// User cancelled? exit now
		return false;
	}

	// Now verify that the user really wants to do this
	if (!askForSavedGameVerification(_textSaveGameVerify, (char *)_text->_inputString, slotId)) {
		return false;
	}

	newDescription.clear();
	newDescription += (char *)_text->_inputString;
	return true;
}

int16 SystemUI::askForRestoreGameSlot() {
	int16 restoreGameSlotNr = -1;

	// Fill saved game slot cache
	readSavedGameSlots(true, true); // filter empty/corrupt slots, but including auto-save slot

	if (_savedGameArray.size() == 0) {
		// no saved games
		_vm->_text->messageBox(_textRestoreGameNoSlots);
		return -1;
	}

	restoreGameSlotNr = askForSavedGameSlot(_textRestoreGameSelectSlot);

	// User cancelled? exit now
	if (restoreGameSlotNr < 0)
		return -1;

	SystemUISavedGameEntry *selectedSavedGameEntry = &_savedGameArray[restoreGameSlotNr];

	// Check, if selected saved game was marked as valid
	if (!selectedSavedGameEntry->isValid) {
		_vm->_text->messageBox(_textRestoreGameError);
		return -1;
	}

	// Now verify that the user really wants to do this
	if (!askForSavedGameVerification(_textRestoreGameVerify, selectedSavedGameEntry->description, selectedSavedGameEntry->slotId)) {
		return -1;
	}

	// return actual slot number of the saved game
	return _savedGameArray[restoreGameSlotNr].slotId;
}

int16 SystemUI::askForSavedGameSlot(const char *slotListText) {
	int16 messageBoxHeight = 0;
	int16 slotsCount = _savedGameArray.size();

	if (slotsCount > SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN) {
		messageBoxHeight = 5 + SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN;
	} else {
		messageBoxHeight = 5 + slotsCount;
	}
	_text->drawMessageBox(slotListText, messageBoxHeight, 34, true);

	drawSavedGameSlots();
	drawSavedGameSlotSelector(true);

	_vm->cycleInnerLoopActive(CYCLE_INNERLOOP_SYSTEMUI_SELECTSAVEDGAMESLOT);
	do {
		_vm->mainCycle();
	} while (_vm->cycleInnerLoopIsActive() && !(_vm->shouldQuit() || _vm->_restartGame));

	_text->closeWindow();

	return _savedGameSelectedSlotNr;
}

void SystemUI::savedGameSlot_CharPress(int16 newChar) {
	int16 slotCount = _savedGameArray.size();
	int16 newUpmostSlotNr = _savedGameUpmostSlotNr;
	int16 newSelectedSlotNr = _savedGameSelectedSlotNr;
	bool  slotsScrolled = false;

	switch (newChar) {
	case AGI_KEY_ENTER:
		_vm->cycleInnerLoopInactive(); // exit savedGameSlot-loop
		return;
		break;

	case AGI_KEY_ESCAPE:
		_savedGameSelectedSlotNr = -1;
		_vm->cycleInnerLoopInactive(); // exit savedGameSlot-loop
		return;
		break;
	case AGI_KEY_UP: // previous slot
		newSelectedSlotNr--;
		break;
	case AGI_KEY_DOWN: // next slot
		newSelectedSlotNr++;
		break;
	// FEATURE: any scroll functionality was not available in original AGI
	//  Original AGI was in fact limited to a total of 12 save slots
	case AGI_KEY_PAGE_UP: // scroll up
		newUpmostSlotNr -= SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN;
		break;
	case AGI_KEY_PAGE_DOWN: // scroll down
		newUpmostSlotNr += SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN;
		break;
	case AGI_KEY_HOME: // scroll all the way up
		newUpmostSlotNr = 0;
		break;
	case AGI_KEY_END: // scroll all the way down
		newUpmostSlotNr = (slotCount - 1) - (SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN - 1);
		break;

	default:
		break;
	}

	if (newUpmostSlotNr != _savedGameUpmostSlotNr) {
		// Make sure, upmost slot number is valid
		if (newUpmostSlotNr < 0) {
			newUpmostSlotNr = 0;
		}
		if (newUpmostSlotNr + (SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN - 1) >= slotCount) {
			newUpmostSlotNr = (slotCount - 1) - (SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN - 1);
			if (newUpmostSlotNr < 0)
				newUpmostSlotNr = 0;
		}

		if (newUpmostSlotNr != _savedGameUpmostSlotNr) {
			// Still different? then actually force a slot number change in any case
			slotsScrolled = true;

			// also adjust selected slot number now
			int16 slotDifference = _savedGameSelectedSlotNr - _savedGameUpmostSlotNr;
			newSelectedSlotNr = newUpmostSlotNr + slotDifference;
		}
	}

	if ((newSelectedSlotNr != _savedGameSelectedSlotNr) || slotsScrolled) {
		// slot number was changed

		// Make slot number valid and scroll in case it's needed
		if (newSelectedSlotNr < 0) {
			newSelectedSlotNr = slotCount - 1;
			newUpmostSlotNr = newSelectedSlotNr - SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN;
			if (newUpmostSlotNr < 0)
				newUpmostSlotNr = 0;
		}
		if (newSelectedSlotNr >= slotCount) {
			newSelectedSlotNr = 0;
			newUpmostSlotNr = 0;
		}

		if (newSelectedSlotNr < newUpmostSlotNr) {
			// scroll up when needed
			newUpmostSlotNr = newSelectedSlotNr;
		}

		if (newSelectedSlotNr >= newUpmostSlotNr + SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN) {
			// scroll down when needed
			newUpmostSlotNr = newSelectedSlotNr - (SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN - 1);
		}

		bool drawSlots = false;

		// remove selector
		drawSavedGameSlotSelector(false);

		if (newUpmostSlotNr != _savedGameUpmostSlotNr) {
			drawSlots = true;
		}

		_savedGameUpmostSlotNr   = newUpmostSlotNr;
		_savedGameSelectedSlotNr = newSelectedSlotNr;
		if (drawSlots) {
			drawSavedGameSlots();
		}
		drawSavedGameSlotSelector(true);
	}
}

void SystemUI::clearSavedGameSlots() {
	_savedGameArray.clear();
	_savedGameUpmostSlotNr = 0;
	_savedGameSelectedSlotNr = 0;
}

void SystemUI::createSavedGameDisplayText(char *destDisplayText, const char *actualDescription, int16 slotId, bool fillUpWithSpaces) {
	char  fillUpChar = fillUpWithSpaces ? ' ' : 0x00;
	char  slotIdChar[3];
	int16 actualDescriptionLen = 0;

	// clear with spaces
	memset(destDisplayText, fillUpChar, SYSTEMUI_SAVEDGAME_DISPLAYTEXT_LEN);

	// create fixed prefix (" 1:", "10:", etc.)
	sprintf(slotIdChar, "%02d", slotId);
	memcpy(destDisplayText, slotIdChar, 2);
	destDisplayText[2] = ':';

	actualDescriptionLen = strlen(actualDescription);
	if (actualDescriptionLen > (SYSTEMUI_SAVEDGAME_DISPLAYTEXT_LEN - SYSTEMUI_SAVEDGAME_DISPLAYTEXT_PREFIX_LEN)) {
		actualDescriptionLen = SYSTEMUI_SAVEDGAME_DISPLAYTEXT_LEN - SYSTEMUI_SAVEDGAME_DISPLAYTEXT_PREFIX_LEN;
	}

	if (actualDescriptionLen > 0) {
		memcpy(destDisplayText + SYSTEMUI_SAVEDGAME_DISPLAYTEXT_PREFIX_LEN, actualDescription, actualDescriptionLen);
	}
	destDisplayText[SYSTEMUI_SAVEDGAME_DISPLAYTEXT_LEN] = 0; // terminator
}

void SystemUI::readSavedGameSlots(bool filterNonexistant, bool withAutoSaveSlot) {
	SavedGameSlotIdArray slotIdArray;
	int16 lastSlotId = -1;
	int16 curSlotId = 0;
	int16 loopSlotId = 0;
	SystemUISavedGameEntry savedGameEntry;
	Common::String saveDescription;
	uint32         saveDate = 0;
	uint16         saveTime = 0;
	bool           saveIsValid = false;

	int16  mostRecentSlotNr = -1;
	uint32 mostRecentSlotSaveDate = 0;
	uint16 mostRecentSlotSaveTime = 0;

	clearSavedGameSlots();

	// we assume that the Slot-Ids are in order
	slotIdArray = _vm->getSavegameSlotIds();
	slotIdArray.push_back(SYSTEMUI_SAVEDGAME_MAXIMUM_SLOTS); // so that the loop will process all slots

	SavedGameSlotIdArray::iterator it;
	SavedGameSlotIdArray::iterator end = slotIdArray.end();;

	for (it = slotIdArray.begin(); it != end; it++) {
		curSlotId = *it;

		assert(curSlotId > lastSlotId); // safety check

		if (curSlotId == 0) {
			// Skip over auto-save slot, if not requested
			if (!withAutoSaveSlot)
				continue;
		}

		// only allow slot-ids 000 up to 099
		if (curSlotId >= SYSTEMUI_SAVEDGAME_MAXIMUM_SLOTS)
			curSlotId = SYSTEMUI_SAVEDGAME_MAXIMUM_SLOTS;

		if (!filterNonexistant) {
			// add slot-ids from last one up to current one (not including the current one)
			lastSlotId++;
			for (loopSlotId = lastSlotId; loopSlotId < curSlotId; loopSlotId++) {
				if (loopSlotId == 0) {
					// Skip over auto-save slot, if not requested
					if (!withAutoSaveSlot)
						continue;
				}

				savedGameEntry.slotId  = loopSlotId;
				savedGameEntry.exists  = false;
				savedGameEntry.isValid = false;
				memset(savedGameEntry.description, 0, sizeof(savedGameEntry.description));
				createSavedGameDisplayText(savedGameEntry.displayText, "", loopSlotId, true);

				_savedGameArray.push_back(savedGameEntry);
			}
		}

		if (curSlotId >= SYSTEMUI_SAVEDGAME_MAXIMUM_SLOTS)
			break; // force an exit, limit reached

		savedGameEntry.slotId = curSlotId;
		if (_vm->getSavegameInformation(curSlotId, saveDescription, saveDate, saveTime, saveIsValid)) {
			if (saveIsValid) {
				// saved game is valid
				// check, if this is the latest slot
				if (saveDate > mostRecentSlotSaveDate) {
					mostRecentSlotNr = _savedGameArray.size();
					mostRecentSlotSaveDate = saveDate;
					mostRecentSlotSaveTime = saveTime;
				} else if ((saveDate == mostRecentSlotSaveDate) && (saveTime >= mostRecentSlotSaveTime)) {
					// larger or equal is on purpose, so that we use the last slot in case there are multiple saves
					// with the exact same date+time
					mostRecentSlotNr = _savedGameArray.size();
					mostRecentSlotSaveDate = saveDate;
					mostRecentSlotSaveTime = saveTime;
				}
			}
		} else {
			// slot doesn't exist
			if (filterNonexistant) {
				continue;
			}
		}

		savedGameEntry.exists  = true;
		savedGameEntry.isValid = saveIsValid;
		memset(savedGameEntry.description, 0, sizeof(savedGameEntry.description));
		strncpy(savedGameEntry.description, saveDescription.c_str(), SYSTEMUI_SAVEDGAME_DESCRIPTION_LEN);
		createSavedGameDisplayText(savedGameEntry.displayText, saveDescription.c_str(), curSlotId, true);

		_savedGameArray.push_back(savedGameEntry);

		lastSlotId = curSlotId;
	}

	if (mostRecentSlotNr >= 0) {
		// valid slot found, we use it as default
		// Sierra AGI seems to have done the same
		_savedGameSelectedSlotNr = mostRecentSlotNr;
		if (mostRecentSlotNr < SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN) {
			// available without scrolling, so keep upmost slot 0
			_savedGameUpmostSlotNr = 0;
		} else {
			// we need to scroll, try to have the slot in the middle
			int16 slotCount = _savedGameArray.size();

			_savedGameUpmostSlotNr = mostRecentSlotNr - (SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN / 2);
			if ((_savedGameUpmostSlotNr + (SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN - 1)) >= slotCount) {
				// current upmost would result in empty lines, because it's at the end, push it upwards
				_savedGameUpmostSlotNr = (slotCount - 1) - (SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN - 1);
			}
		}
	}
}

void SystemUI::figureOutAutomaticSavedGameSlot(const char *automaticSaveDescription, int16 &matchedGameSlotId, int16 &freshGameSlotId) {
	bool foundFresh = false;

	for (uint16 slotNr = 0; slotNr < _savedGameArray.size(); slotNr++) {
		SystemUISavedGameEntry *savedGameEntry = &_savedGameArray[slotNr];

		if (savedGameEntry->isValid) {
			// valid saved game
			if (strcmp(savedGameEntry->description, automaticSaveDescription) == 0) {
				// we got a match
				matchedGameSlotId = savedGameEntry->slotId;
				return;
			}
		}
		if (!foundFresh) {
			// no new slot found yet
			if (!savedGameEntry->exists) {
				// and current slot doesn't exist
				if (slotNr) {
					// and slot is not the auto-save slot -> remember this slot
					freshGameSlotId = savedGameEntry->slotId;
					foundFresh = true;
				}
			}
		}
	}
	return;
}

void SystemUI::drawSavedGameSlots() {
	int16 slotsToDrawCount = _savedGameArray.size() - _savedGameUpmostSlotNr;
	int16 slotNr = 0;

	if (slotsToDrawCount > SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN) {
		slotsToDrawCount = SYSTEMUI_SAVEDGAME_SLOTS_ON_SCREEN;
	}
	_text->charAttrib_Push();
	_text->charAttrib_Set(0, 15);

	for (slotNr = 0; slotNr < slotsToDrawCount; slotNr++) {
		_text->displayTextInsideWindow("-", 5 + slotNr, 1);
		_text->displayTextInsideWindow(_savedGameArray[_savedGameUpmostSlotNr + slotNr].displayText, 5 + slotNr, 3);
	}
	_text->charAttrib_Pop();
}

void SystemUI::drawSavedGameSlotSelector(bool active) {
	int16 windowRow = 5 + (_savedGameSelectedSlotNr - _savedGameUpmostSlotNr);

	_text->charAttrib_Push();
	_text->charAttrib_Set(0, 15);
	if (active) {
		_text->displayTextInsideWindow("\x1a", windowRow, 0);
	} else {
		_text->displayTextInsideWindow(" ", windowRow, 0);
	}
	_text->charAttrib_Pop();
}

bool SystemUI::askForSavedGameVerification(const char *verifyText, const char *actualDescription, int16 slotId) {
	char displayDescription[SYSTEMUI_SAVEDGAME_DISPLAYTEXT_LEN + 1];
	Common::String userActionVerify;
	Common::String savedGameFilename = _vm->getSavegameFilename(slotId);
	int16 userKey = 0;

	createSavedGameDisplayText(displayDescription, actualDescription, slotId, false);
	userActionVerify = Common::String::format(verifyText, displayDescription, savedGameFilename.c_str());

	_text->drawMessageBox(userActionVerify.c_str(), 0, 35);

	userKey = _vm->waitKey();

	_text->closeWindow();

	switch (userKey) {
	case AGI_KEY_ENTER:
	case AGI_KEY_LEFT:
		return true;
		break;
	default:
		break;
	}
	return false;
}

} // End of namespace Agi
