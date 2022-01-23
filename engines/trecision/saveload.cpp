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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "gui/saveload.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "trecision/actor.h"
#include "trecision/animmanager.h"
#include "trecision/dialog.h"
#include "trecision/graphics.h"
#include "trecision/logic.h"
#include "trecision/pathfinding3d.h"
#include "trecision/sound.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {

void TrecisionEngine::loadSaveSlots(Common::StringArray &saveNames) {
	for (uint i = 0; i < ICONSHOWN; ++i) {
		SaveStateDescriptor saveState = getMetaEngine()->querySaveMetaInfos(_targetName.c_str(), i + 1);
		if (saveState.getSaveSlot() == -1) {
			saveNames.push_back(_sysText[kMessageEmptySpot]);
			_inventory.push_back(EMPTYSLOT);
		} else {
			saveNames.push_back(saveState.getDescription());
			_inventory.push_back(EMPTYSLOT + i + 1);
			_graphicsMgr->setSaveSlotThumbnail(i, saveState.getThumbnail());
		}
	}

	refreshInventory(0, 0);
}

bool TrecisionEngine::dataSave() {
	const Common::Array<byte> savedInventory = _inventory;
	const uint8 savedIconBase = _iconBase;
	Common::StringArray saveNames;
	saveNames.reserve(MAXSAVEFILE);
	uint16 posx, LenText;
	bool ret = true;

	_actor->actorStop();
	_pathFind->nextStep();

	if (!ConfMan.getBool("originalsaveload")) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
		int saveSlot = dialog->runModalWithCurrentTarget();
		Common::String saveName = dialog->getResultString();
		bool skipSave = saveSlot == -1;
		delete dialog;

		// Remove the mouse click event from the save/load dialog
		eventLoop();
		_mouseLeftBtn = _mouseRightBtn = false;

		if (!skipSave)
			saveGameState(saveSlot, saveName);

		return skipSave;
	}

	_graphicsMgr->clearScreenBufferTop();

	SDText drawText;
	drawText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		_sysText[kMessageSavePosition]);
	drawText.draw(this);

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_graphicsMgr->clearScreenBufferInventory();
	_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	_scheduler->resetQueues();

	freeKey();

	// Reset the inventory and turn it into save slots
	_inventory.clear();
	_iconBase = 0;

insave:

	int8 CurPos = -1;
	int8 OldPos = -1;
	bool skipSave = false;

	loadSaveSlots(saveNames);

	for (;;) {
		checkSystem();
		getKey();

		int16 mx = _mousePos.x;
		int16 my = _mousePos.y;

		if (my >= FIRSTLINE &&
			my < FIRSTLINE + ICONDY &&
			mx >= ICONMARGSX &&
			mx < MAXX - ICONMARGDX) {
			OldPos = CurPos;
			CurPos = ((mx - ICONMARGSX) / ICONDX);

			if (OldPos != CurPos) {
				_graphicsMgr->clearScreenBufferSaveSlotDescriptions();

				posx = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				LenText = textLength(saveNames[CurPos]);

				posx = CLIP(posx - (LenText / 2), 2, MAXX - 2 - LenText);
				drawText.set(
					Common::Rect(posx, FIRSTLINE + ICONDY + 10, LenText + posx, CARHEI + (FIRSTLINE + ICONDY + 10)),
					Common::Rect(0, 0, LenText, CARHEI),
					MOUSECOL,
					saveNames[CurPos].c_str());
				drawText.draw(this);

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (_mouseLeftBtn) {
				_mouseLeftBtn = false;
				break;
			}
		} else {
			if (OldPos != -1) {
				_graphicsMgr->clearScreenBufferSaveSlotDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			OldPos = -1;
			CurPos = -1;

			if (_mouseLeftBtn || _mouseRightBtn) {
				_mouseLeftBtn = _mouseRightBtn = false;
				skipSave = true;
				break;
			}
		}
	}

	if (!skipSave) {
		if (_inventory[CurPos] == EMPTYSLOT) {
			saveNames[CurPos].clear();

			_graphicsMgr->clearScreenBufferSaveSlotDescriptions();
			_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		for (;;) {
			_keybInput = true;
			checkSystem();
			uint16 ch = getKey();
			freeKey();

			_keybInput = false;

			if (ch == 0x1B) {
				ch = 0;
				_graphicsMgr->clearScreenBufferSaveSlotDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);

				goto insave;
			}

			if (ch == 8) // Backspace
				saveNames[CurPos].deleteLastChar();
			else if (ch == 13) // Enter
				break;
			else if (saveNames[CurPos].size() < 39 && Common::isPrint(ch))
				saveNames[CurPos] += ch;

			_graphicsMgr->clearScreenBufferSaveSlotDescriptions();

			saveNames[CurPos] += '_'; // add blinking cursor

			posx = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
			LenText = textLength(saveNames[CurPos]);

			posx = CLIP(posx - (LenText / 2), 2, MAXX - 2 - LenText);
			drawText.set(
				Common::Rect(posx, FIRSTLINE + ICONDY + 10, LenText + posx, CARHEI + (FIRSTLINE + ICONDY + 10)),
				Common::Rect(0, 0, LenText, CARHEI),
				MOUSECOL,
				saveNames[CurPos].c_str());

			const bool hideLastChar = (readTime() / 8) & 1;
			drawText.draw(this, hideLastChar);

			saveNames[CurPos].deleteLastChar(); // remove blinking cursor

			_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
		}

		_graphicsMgr->clearScreenBufferInventory();

		ret = false;

		// Restore the inventory
		_inventory = savedInventory;
		_curInventory = 0;
		_iconBase = savedIconBase;

		saveGameState(CurPos + 1, saveNames[CurPos]);
	}

	_graphicsMgr->clearScreenBufferInventory();
	_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

	_graphicsMgr->clearScreenBufferTop();
	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	// Restore the inventory
	_inventory = savedInventory;
	_curInventory = 0;
	_iconBase = savedIconBase;

	return ret;
}

bool TrecisionEngine::dataLoad() {
	const Common::Array<byte> savedInventory = _inventory;
	const uint8 savedIconBase = _iconBase;
	Common::StringArray saveNames;
	saveNames.reserve(MAXSAVEFILE);
	bool retval = true;

	if (!ConfMan.getBool("originalsaveload")) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);
		int saveSlot = dialog->runModalWithCurrentTarget();
		bool skipLoad = saveSlot == -1;
		delete dialog;

		// Remove the mouse click event from the save/load dialog
		eventLoop();
		_mouseLeftBtn = _mouseRightBtn = false;

		if (!skipLoad)
			loadGameState(saveSlot);

		return !skipLoad;
	}

	_graphicsMgr->clearScreenBufferTop();

	_graphicsMgr->showCursor();

	SDText drawText;
	drawText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		_sysText[kMessageLoadPosition]);
	drawText.draw(this);

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_graphicsMgr->clearScreenBufferInventory();
	_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	_scheduler->resetQueues();

	freeKey();

	// Reset the inventory and turn it into save slots
	_inventory.clear();
	_iconBase = 0;

	loadSaveSlots(saveNames);

	bool skipLoad = false;
	int8 curPos = -1;
	int8 oldPos = -1;

	for (;;) {
		checkSystem();
		getKey();

		if (_mousePos.y >= FIRSTLINE &&
			_mousePos.y < (FIRSTLINE + ICONDY) &&
			_mousePos.x >= ICONMARGSX &&
			(_mousePos.x < (MAXX - ICONMARGDX))) {
			oldPos = curPos;
			curPos = (_mousePos.x - ICONMARGSX) / ICONDX;

			if (oldPos != curPos) {
				_graphicsMgr->clearScreenBufferSaveSlotDescriptions();

				uint16 posX = ICONMARGSX + ((curPos) * (ICONDX)) + ICONDX / 2;
				uint16 lenText = textLength(saveNames[curPos]);
				if (posX - (lenText / 2) < 2)
					posX = 2;
				else
					posX = posX - (lenText / 2);
				if (posX + lenText > MAXX - 2)
					posX = MAXX - 2 - lenText;

				drawText.set(
					Common::Rect(posX, FIRSTLINE + ICONDY + 10, lenText + posX, CARHEI + (FIRSTLINE + ICONDY + 10)),
					Common::Rect(0, 0, lenText, CARHEI),
					MOUSECOL,
					saveNames[curPos].c_str());
				drawText.draw(this);

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (_mouseLeftBtn && (_inventory[curPos] != EMPTYSLOT)) {
				_mouseLeftBtn = false;
				break;
			}
		} else {
			if (oldPos != -1) {
				_graphicsMgr->clearScreenBufferSaveSlotDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			oldPos = -1;
			curPos = -1;

			if (_mouseLeftBtn || _mouseRightBtn) {
				_mouseLeftBtn = _mouseRightBtn = false;
				retval = false;
				skipLoad = true;
				break;
			}
		}
	}

	if (!skipLoad) {
		loadGameState(curPos + 1);
	} else {
		_actor->actorStop();
		_pathFind->nextStep();
		checkSystem();

		_graphicsMgr->clearScreenBufferInventory();
		_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

		_graphicsMgr->clearScreenBufferTop();
		_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

		if (_flagScriptActive) {
			_graphicsMgr->hideCursor();
		}

		// Restore the inventory
		_inventory = savedInventory;
		_curInventory = 0;
		_iconBase = savedIconBase;
	}

	return retval;
}

Common::Error TrecisionEngine::loadGameStream(Common::SeekableReadStream *stream) {
	const byte version = stream->readByte();
	Common::Serializer ser(stream, nullptr);
	ser.setVersion(version);
	syncGameStream(ser);

	_graphicsMgr->clearScreenBufferInventory();

	_flagNoPaintScreen = true;
	_curStack = 0;
	_flagScriptActive = false;

	_oldRoom = _curRoom;
	changeRoom(_curRoom);

	_actor->actorStop();
	_pathFind->nextStep();
	checkSystem();

	_graphicsMgr->clearScreenBufferInventory();
	_graphicsMgr->copyToScreen(0, FIRSTLINE, MAXX, TOP);

	_graphicsMgr->clearScreenBufferTop();
	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	if (_flagScriptActive) {
		_graphicsMgr->hideCursor();
	}

	return Common::kNoError;
}

Common::Error TrecisionEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	const byte version = SAVE_VERSION_SCUMMVM;
	Common::Serializer ser(nullptr, stream);
	ser.setVersion(version);
	stream->writeByte(version);
	syncGameStream(ser);
	return Common::kNoError;
}

bool TrecisionEngine::syncGameStream(Common::Serializer &ser) {
	uint16 unused = 0;

	if (ser.isLoading()) {
		ser.skip(40, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX);                               // description
		ser.skip(ICONDX * ICONDY * sizeof(uint16), SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // thumbnail
	}

	ser.syncAsUint16LE(_curRoom);
	ser.syncAsByte(unused, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // _inventorySize
	ser.syncAsByte(unused, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // _cyberInventorySize
	ser.syncAsByte(_iconBase);
	ser.syncAsSint16LE(_flagSkipTalk);
	ser.syncAsSint16LE(unused, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // _flagSkipEnable
	ser.syncAsSint16LE(unused, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // _flagMouseEnabled
	ser.syncAsSint16LE(unused, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // _flagScreenRefreshed
	ser.syncAsSint16LE(_flagPaintCharacter);
	ser.syncAsSint16LE(_flagSomeoneSpeaks);
	ser.syncAsSint16LE(_flagCharacterSpeak);
	ser.syncAsSint16LE(_flagInventoryLocked);
	ser.syncAsSint16LE(_flagUseWithStarted);
	ser.syncAsSint16LE(unused, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // _flagMousePolling
	ser.syncAsSint16LE(unused, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // _flagDialogSolitaire
	ser.syncAsSint16LE(unused);	// _flagCharacterExists

	syncInventory(ser);
	_actor->syncGameStream(ser);
	_pathFind->syncGameStream(ser);

	for (int i = 0; i < MAXROOMS; i++)
		_room[i].syncGameStream(ser);

	for (int i = 0; i < MAXOBJ; i++)
		_obj[i].syncGameStream(ser);

	for (int i = 0; i < MAXINVENTORY; i++)
		_inventoryObj[i].syncGameStream(ser);

	_animMgr->syncGameStream(ser);
	ser.skip(NUMSAMPLES * 2, SAVE_VERSION_ORIGINAL_MIN, SAVE_VERSION_ORIGINAL_MAX); // SoundManager::syncGameStream()
	_dialogMgr->syncGameStream(ser);
	_logicMgr->syncGameStream(ser);

	return true;
}

} // End of namespace Trecision
