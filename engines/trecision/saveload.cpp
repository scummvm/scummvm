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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gui/saveload.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/translation.h"

#include "trecision/dialog.h"
#include "trecision/logic.h"
#include "trecision/sound.h"
#include "trecision/3d.h"
#include "trecision/actor.h"
#include "trecision/graphics.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {
void TrecisionEngine::loadSaveSlots(Common::StringArray &saveNames) {
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	for (uint i = 0; i < _inventory.size(); ++i) {
		Common::String saveFileName = getSaveStateName(i + 1);
		Common::InSaveFile *saveFile = saveFileMan->openForLoading(saveFileName);
		ExtendedSavegameHeader header;

		if (!saveFile) {
			saveNames.push_back(_sysText[kMessageEmptySpot]);
			_inventory[i] = iEMPTYSLOT;
			continue;
		}

		const byte version = saveFile->readByte();

		if (saveFile && version == SAVE_VERSION_ORIGINAL) {
			// Original saved game, convert
			char buf[40];
			saveFile->read(buf, 40);
			buf[39] = '\0';
			saveNames.push_back(buf);

			uint16 *thumbnailBuf = _icons + (READICON + 1 + i) * ICONDX * ICONDY;
			saveFile->read((void *)thumbnailBuf, ICONDX * ICONDY * sizeof(uint16));
			_graphicsMgr->updatePixelFormat(thumbnailBuf, ICONDX * ICONDY);

			_inventory[i] = LASTICON + i;
		} else if (saveFile && version == SAVE_VERSION_SCUMMVM) {
			const bool headerRead = MetaEngine::readSavegameHeader(saveFile, &header, false);
			if (headerRead) {
				saveNames.push_back(header.description);

				Graphics::Surface *thumbnail = convertScummVMThumbnail(header.thumbnail);
				uint16 *thumbnailBuf = _icons + (READICON + 1 + i) * ICONDX * ICONDY;
				memcpy(thumbnailBuf, thumbnail->getPixels(), ICONDX * ICONDY * 2);
				thumbnail->free();
				delete thumbnail;

				_inventory[i] = LASTICON + i;
			} else {
				saveNames.push_back(_sysText[kMessageEmptySpot]);
				_inventory[i] = iEMPTYSLOT;
			}
		} else {
			saveNames.push_back(_sysText[kMessageEmptySpot]);
			_inventory[i] = iEMPTYSLOT;
		}

		delete saveFile;
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
		MASKCOL,
		_sysText[kMessageSavePosition]);
	drawText.draw();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_graphicsMgr->clearScreenBufferInventory();
	_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	_gameQueue.initQueue();
	_animQueue.initQueue();
	_characterQueue.initQueue();

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
				_graphicsMgr->clearScreenBufferInventoryDescriptions();

				posx = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				LenText = textLength(saveNames[CurPos]);

				posx = CLIP(posx - (LenText / 2), 2, MAXX - 2 - LenText);
				drawText.set(
					Common::Rect(posx, FIRSTLINE + ICONDY + 10, LenText + posx, CARHEI + (FIRSTLINE + ICONDY + 10)),
					Common::Rect(0, 0, LenText, CARHEI),
					MOUSECOL,
					MASKCOL,
					saveNames[CurPos].c_str());
				drawText.draw();

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (_mouseLeftBtn)
				break;
		} else {
			if (OldPos != -1) {
				_graphicsMgr->clearScreenBufferInventoryDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			OldPos = -1;
			CurPos = -1;

			if (_mouseLeftBtn || _mouseRightBtn) {
				skipSave = true;
				break;
			}
		}
	}

	if (!skipSave) {
		if (_inventory[CurPos] == iEMPTYSLOT) {
			saveNames[CurPos].clear();

			_graphicsMgr->clearScreenBufferInventoryDescriptions();
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
				_graphicsMgr->clearScreenBufferInventoryDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);

				goto insave;
			}

			if (ch == 8) // Backspace
				saveNames[CurPos].deleteLastChar();
			else if (ch == 13) // Enter
				break;
			else if (saveNames[CurPos].size() < 39 && Common::isPrint(ch))
				saveNames[CurPos] += ch;

			_graphicsMgr->clearScreenBufferInventoryDescriptions();

			saveNames[CurPos] += '_'; // add blinking cursor

			posx = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
			LenText = textLength(saveNames[CurPos]);

			posx = CLIP(posx - (LenText / 2), 2, MAXX - 2 - LenText);
			drawText.set(
				Common::Rect(posx, FIRSTLINE + ICONDY + 10, LenText + posx, CARHEI + (FIRSTLINE + ICONDY + 10)),
				Common::Rect(0, 0, LenText, CARHEI),
				MOUSECOL,
				MASKCOL,
				saveNames[CurPos].c_str());

			if ((readTime() / 8) & 1)
				_blinkLastDTextChar = 0x0000;

			drawText.draw();
			_blinkLastDTextChar = MASKCOL;

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

	_graphicsMgr->clearScreenBufferTopDescription();
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

		performLoad(saveSlot - 1, skipLoad);

		return !skipLoad;
	}

	_graphicsMgr->clearScreenBufferTop();

	_graphicsMgr->showCursor();

	SDText drawText;
	drawText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageLoadPosition]);
	drawText.draw();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_graphicsMgr->clearScreenBufferInventory();
	_graphicsMgr->copyToScreen(0, TOP + AREA, MAXX, TOP);

	_gameQueue.initQueue();
	_animQueue.initQueue();
	_characterQueue.initQueue();

	freeKey();

	// Reset the inventory and turn it into save slots
	_inventory.clear();
	_iconBase = 0;

	loadSaveSlots(saveNames);

	bool skipLoad = false;
	int8 CurPos = -1;
	int8 OldPos = -1;

	for (;;) {
		checkSystem();
		getKey();

		if (_mousePos.y >= FIRSTLINE &&
			_mousePos.y < (FIRSTLINE + ICONDY) &&
			_mousePos.x >= ICONMARGSX &&
			(_mousePos.x < (MAXX - ICONMARGDX))) {
			OldPos = CurPos;
			CurPos = (_mousePos.x - ICONMARGSX) / ICONDX;

			if (OldPos != CurPos) {
				_graphicsMgr->clearScreenBufferInventoryDescriptions();

				uint16 posX = ICONMARGSX + ((CurPos) * (ICONDX)) + ICONDX / 2;
				uint16 lenText = textLength(saveNames[CurPos]);
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
					MASKCOL,
					saveNames[CurPos].c_str());
				drawText.draw();

				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			if (_mouseLeftBtn && (_inventory[CurPos] != iEMPTYSLOT))
				break;
		} else {
			if (OldPos != -1) {
				_graphicsMgr->clearScreenBufferInventoryDescriptions();
				_graphicsMgr->copyToScreen(0, FIRSTLINE + ICONDY + 10, MAXX, CARHEI);
			}

			OldPos = -1;
			CurPos = -1;

			if (_mouseLeftBtn || _mouseRightBtn) {
				retval = false;
				skipLoad = true;
				break;
			}
		}
	}

	performLoad(CurPos, skipLoad);

	if (skipLoad) {
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
		ser.skip(40, SAVE_VERSION_ORIGINAL, SAVE_VERSION_ORIGINAL);                               // description
		ser.skip(ICONDX * ICONDY * sizeof(uint16), SAVE_VERSION_ORIGINAL, SAVE_VERSION_ORIGINAL); // thumbnail
	}

	ser.syncAsUint16LE(_curRoom);
	ser.syncAsByte(unused); // _inventorySize
	ser.syncAsByte(unused); // _cyberInventorySize
	ser.syncAsByte(_iconBase);
	ser.syncAsSint16LE(_flagSkipTalk);
	ser.syncAsSint16LE(unused); // _flagSkipEnable
	ser.syncAsSint16LE(unused); // _flagMouseEnabled
	ser.syncAsSint16LE(unused); // _flagScreenRefreshed
	ser.syncAsSint16LE(_flagPaintCharacter);
	ser.syncAsSint16LE(_flagSomeoneSpeaks);
	ser.syncAsSint16LE(_flagCharacterSpeak);
	ser.syncAsSint16LE(_flagInventoryLocked);
	ser.syncAsSint16LE(_flagUseWithStarted);
	ser.syncAsSint16LE(unused); // FlagMousePolling
	ser.syncAsSint16LE(unused); // FlagDialogSolitaire
	ser.syncAsSint16LE(_flagCharacterExists);

	syncInventory(ser);
	_actor->syncGameStream(ser);
	_pathFind->syncGameStream(ser);

	for (int a = 0; a < MAXROOMS; a++) {
		ser.syncBytes((byte *)_room[a]._baseName, 4);
		for (int i = 0; i < MAXACTIONINROOM; i++)
			ser.syncAsUint16LE(_room[a]._actions[i]);
		ser.syncAsByte(_room[a]._flag);
		ser.syncAsUint16LE(_room[a]._bkgAnim);
	}

	for (int a = 0; a < MAXOBJ; a++) {
		ser.syncAsUint16LE(_obj[a]._lim.left);
		ser.syncAsUint16LE(_obj[a]._lim.top);
		ser.syncAsUint16LE(_obj[a]._lim.right);
		ser.syncAsUint16LE(_obj[a]._lim.bottom);
		ser.syncAsUint16LE(_obj[a]._name);
		ser.syncAsUint16LE(_obj[a]._examine);
		ser.syncAsUint16LE(_obj[a]._action);
		ser.syncAsUint16LE(_obj[a]._anim);
		ser.syncAsByte(_obj[a]._mode);
		ser.syncAsByte(_obj[a]._flag);
		ser.syncAsByte(_obj[a]._goRoom);
		ser.syncAsByte(_obj[a]._nbox);
		ser.syncAsByte(_obj[a]._ninv);
		ser.syncAsSByte(_obj[a]._position);
	}

	for (int a = 0; a < MAXINVENTORY; a++) {
		ser.syncAsUint16LE(_inventoryObj[a]._name);
		ser.syncAsUint16LE(_inventoryObj[a]._examine);
		ser.syncAsUint16LE(_inventoryObj[a]._action);
		ser.syncAsUint16LE(_inventoryObj[a]._anim);
		ser.syncAsByte(_inventoryObj[a]._flag);
	}

	_animMgr->syncGameStream(ser);
	_soundMgr->syncGameStream(ser);
	_dialogMgr->syncGameStream(ser);
	_logicMgr->syncGameStream(ser);

	return true;
}

} // End of namespace Trecision
