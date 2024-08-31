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

#include "scumm/scumm_v4.h"
#include "scumm/object.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v4, x)

void ScummEngine_v4::setupOpcodes() {
	ScummEngine_v5::setupOpcodes();

	OPCODE(0x25, o5_drawObject);
	OPCODE(0x45, o5_drawObject);
	OPCODE(0x65, o5_drawObject);
	OPCODE(0xa5, o5_drawObject);
	OPCODE(0xc5, o5_drawObject);
	OPCODE(0xe5, o5_drawObject);

	OPCODE(0x50, o4_pickupObject);
	OPCODE(0xd0, o4_pickupObject);

	OPCODE(0x5c, o4_oldRoomEffect);
	OPCODE(0xdc, o4_oldRoomEffect);

	OPCODE(0x0f, o4_ifState);
	OPCODE(0x4f, o4_ifState);
	OPCODE(0x8f, o4_ifState);
	OPCODE(0xcf, o4_ifState);

	OPCODE(0x2f, o4_ifNotState);
	OPCODE(0x6f, o4_ifNotState);
	OPCODE(0xaf, o4_ifNotState);
	OPCODE(0xef, o4_ifNotState);

	OPCODE(0xa7, o4_saveLoadVars);

	OPCODE(0x22, o4_saveLoadGame);
	OPCODE(0xa2, o4_saveLoadGame);

	// Disable some opcodes which are unused in v4.
	_opcodes[0x3b].setProc(nullptr, nullptr);
	_opcodes[0x4c].setProc(nullptr, nullptr);
	_opcodes[0xbb].setProc(nullptr, nullptr);
}

void ScummEngine_v4::o4_ifState() {
	int a = getVarOrDirectWord(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	// WORKAROUND bug #5709 (also occurs in original): Some old versions of
	// Indy3 sometimes fail to allocate IQ points correctly. To quote:
	// "About the points error leaving Castle Brunwald: It seems to "reversed"!
	// When you get caught, free yourself and escape, you get 25 IQ points even
	// though you're not supposed to. However if you escape WITHOUT getting
	// caught, you get 0 IQ points (supposed to get 25 IQ points)."
	// This workaround is meant to address that.
	//
	// See also the similar ScummEngine_v5::o5_startScript() workaround.
	if (_game.id == GID_INDY3 && a == 367 && _currentScript != 0xFF &&
	    vm.slot[_currentScript].number == 363 && _currentRoom == 25 &&
	    enhancementEnabled(kEnhMinorBugFixes)) {
		// Buggy script compares it with '1'
		b = 0;
	}

	jumpRelative(getState(a) == b);
}

void ScummEngine_v4::o4_ifNotState() {
	int a = getVarOrDirectWord(PARAM_1);
	int b = getVarOrDirectByte(PARAM_2);

	jumpRelative(getState(a) != b);
}

void ScummEngine_v4::o4_pickupObject() {
	int obj = getVarOrDirectWord(PARAM_1);

	if (obj < 1) {
		error("pickupObjectOld received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	// Don't take an object twice
		return;

	// debug(0, "adding %d from %d to inventoryOld", obj, _currentRoom);
	addObjectToInventory(obj, _roomResource);
	markObjectRectAsDirty(obj);
	putOwner(obj, VAR(VAR_EGO));
	putClass(obj, kObjectClassUntouchable, 1);
	putState(obj, 1);
	clearDrawObjectQueue();
	runInventoryScript(1);
}

void ScummEngine_v4::o4_oldRoomEffect() {
	int a;

	_opcode = fetchScriptByte();
	if ((_opcode & 0x1F) == 3) {
		a = getVarOrDirectWord(PARAM_1);

		if (_game.platform == Common::kPlatformFMTowns && _game.version == 3) {
			if (a == 4) {
				_textSurface.fillRect(Common::Rect(0, 0, _textSurface.w * _textSurfaceMultiplier, _textSurface.h * _textSurfaceMultiplier), 0);
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
				if (_townsScreen)
					_townsScreen->clearLayer(1);
#endif
				return;
			}
		}

		if (a) {
			_switchRoomEffect = (byte)(a & 0xFF);
			_switchRoomEffect2 = (byte)(a >> 8);
		} else {
			fadeIn(_newEffect);
		}
	}
}

void ScummEngine_v4::o4_saveLoadVars() {
	if (fetchScriptByte() == 1)
		saveVars();
	else
		loadVars();
}

enum StringIds {
	// The string IDs used by Indy3 to store the episode resp. series IQ points.
	// Note that we save the episode IQ points but load the series IQ points,
	// which matches the original Indy3 save/load code. See also the notes
	// on bug #7547.
	STRINGID_IQ_EPISODE = 7,
	STRINGID_IQ_SERIES = 9,
	// The string IDs of the first savegame name, used as an offset to determine
	// the IDs of all savenames.
	// Loom is the only game whose savenames start with a different ID.
	STRINGID_SAVENAME1 = 10,
	STRINGID_SAVENAME1_LOOM = 9
};

void ScummEngine_v4::saveVars() {
	int a, b;

	while ((_opcode = fetchScriptByte()) != 0) {
		switch (_opcode & 0x1F) {
		case 0x01: // write a range of variables
			getResultPos();
			a = _resultVarNumber;
			getResultPos();
			b = _resultVarNumber;
			debug(0, "stub saveVars: vars %d -> %d", a, b);
			break;
		case 0x02: // write a range of string variables
			a = getVarOrDirectByte(PARAM_1);
			b = getVarOrDirectByte(PARAM_2);

			if (a == STRINGID_IQ_EPISODE && b == STRINGID_IQ_EPISODE) {
				if (_game.id == GID_INDY3) {
					// This will not be invoked by the Mac version. The other versions
					// have a much more script-driven handling of the IQ points and do
					// not clearly distinguish the strings for the episode and series IQ.
					// The STRINGID_IQ_EPISODE will often actually handle the series IQ.
					byte *ptr = getResourceAddress(rtString, STRINGID_IQ_EPISODE);
					if (ptr) {
						int size = getResourceSize(rtString, STRINGID_IQ_EPISODE);
						if (size < 73)
							warning("ScummEngine_v4::saveVars(): error writing iq points file");
						else
							saveIQPoints(ptr, 73);
					}
				}
				break;
			}
			break;
		case 0x03: // open file
			a = resStrLen(_scriptPointer);
			strncpy(_saveLoadVarsFilename, (const char *)_scriptPointer, a);
			_saveLoadVarsFilename[a] = '\0';
			_scriptPointer += a + 1;
			break;
		case 0x04:
			return;
		case 0x1F: // close file
			_saveLoadVarsFilename[0] = '\0';
			return;
		default:
			break;
		}
	}
}

void ScummEngine_v4::loadVars() {
	int a, b;

	while ((_opcode = fetchScriptByte()) != 0) {
		switch (_opcode & 0x1F) {
		case 0x01: // read a range of variables
			getResultPos();
			a = _resultVarNumber;
			getResultPos();
			b = _resultVarNumber;
			debug(0, "stub loadVars: vars %d -> %d", a, b);
			break;
		case 0x02: // read a range of string variables
			a = getVarOrDirectByte(PARAM_1);
			b = getVarOrDirectByte(PARAM_2);

			int slot;
			int slotSize;
			byte *slotContent;
			int savegameId;
			bool availSaves[100];

			if (a == STRINGID_IQ_SERIES && b == STRINGID_IQ_SERIES) {
				// Zak256 loads the IQ script-slot but does not use it -> ignore it
				if (_game.id == GID_INDY3) {
					byte *ptr = getResourceAddress(rtString, STRINGID_IQ_EPISODE);
					if (ptr) {
						int size = getResourceSize(rtString, STRINGID_IQ_EPISODE);
						loadIQPoints(ptr, size);
					}
				}
				break;
			}

			listSavegames(availSaves, ARRAYSIZE(availSaves));
			for (slot = a; slot <= b; ++slot) {
				slotSize = getResourceSize(rtString, slot);
				slotContent = getResourceAddress(rtString, slot);

				// load savegame names
				savegameId = slot - a + 1;
				Common::String name;
				if (availSaves[savegameId] && getSavegameName(savegameId, name)) {
					int pos;
					const char *ptr = name.c_str();
					// slotContent ends with {'\0','@'} -> max. length = slotSize-2
					for (pos = 0; pos < slotSize - 2; ++pos) {
						if (!ptr[pos])
							break;
						// replace special characters
						if (ptr[pos] >= 32 && ptr[pos] <= 122 && ptr[pos] != 64)
							slotContent[pos] = ptr[pos];
						else
							slotContent[pos] = '_';
					}
					slotContent[pos] = '\0';
				} else {
					slotContent[0] = '\0';
				}
			}
			break;
		case 0x03: // open file
			a = resStrLen(_scriptPointer);
			strncpy(_saveLoadVarsFilename, (const char *)_scriptPointer, a);
			_saveLoadVarsFilename[a] = '\0';
			_scriptPointer += a + 1;
			break;
		case 0x04:
			return;
		case 0x1F: // close file
			_saveLoadVarsFilename[0] = '\0';
			return;
		default:
			break;
		}
	}
}

/**
 * IQ Point calculation for Indy3.
 * The scripts that perform this task are
 * - script-9 (save/load dialog initialization, loads room 14),
 * - room-14-204 (load series IQ string),
 * - room-14-205 (save series IQ string),
 * - room-14-206 (calculate series IQ string).
 * Unfortunately script-9 contains lots of GUI stuff so calling this script
 * directly is not possible. The other scripts depend on script-9.
 */
void ScummEngine_v4::updateIQPoints() {
	// IQString[0..72] corresponds to each puzzle's IQ.
	// IQString[73] indicates that the IQ-file was loaded successfully and is always 0 when
	// the IQ is calculated, hence it will be ignored here.
	const int NUM_PUZZLES = 73;
	byte seriesIQString[NUM_PUZZLES];
	byte *episodeIQString;
	int episodeIQStringSize;

	// Load string with series IQ points.
	// IMPORTANT: the resource string STRINGID_IQ_SERIES is only valid while
	// the original save/load dialog is executed, so do not use it here.
	memset(seriesIQString, 0, sizeof(seriesIQString));
	loadIQPoints(seriesIQString, sizeof(seriesIQString));

	// Load string with IQ points given per puzzle from currently active game.
	episodeIQString = getResourceAddress(rtString, STRINGID_IQ_EPISODE);
	if (!episodeIQString)
		return;
	episodeIQStringSize = getResourceSize(rtString, STRINGID_IQ_EPISODE);
	if (episodeIQStringSize < NUM_PUZZLES)
		return;

	// Merge episode and series IQ strings and calculate total series and episode IQ.
	int seriesIQ = 0;
	int episodeIQ = 0;
	// iterate over puzzles
	for (int i = 0; i < NUM_PUZZLES; ++i) {
		if (episodeIQString[i] != 0 && episodeIQString[i] != 0x40) {
			seriesIQString[i] = episodeIQString[i];
			episodeIQ += episodeIQString[i];
		}
		if (seriesIQString[i] != 0 && seriesIQString[i] != 0x40) {
			seriesIQ += seriesIQString[i];
			if (_game.platform != Common::kPlatformMacintosh) {
				// This might look very strange, but it is necessary to match the behavior
				// of the non-Mac original interpreters that have a much more script-driven
				// handling of the IQ points and do not clearly distinguish the strings for
				// the episode and series IQ. The STRINGID_IQ_EPISODE string is supposed to
				// contain series IQ data here.
				episodeIQString[i] = seriesIQString[i];
			}
		}
	}

	if (_game.platform == Common::kPlatformMacintosh)
		VAR(244) = episodeIQ;
	VAR(245) = seriesIQ;

	// save series IQ string
	saveIQPoints(seriesIQString, sizeof(seriesIQString));
}

void ScummEngine_v4::clearSeriesIQPoints() {
	Common::OutSaveFile *file;
	Common::String filename = _targetName + ".iq";

	file = _saveFileMan->openForSaving(filename);
	if (file != nullptr) {
		int size = getResourceSize(rtString, STRINGID_IQ_EPISODE);

		for (int i = 0; i < size; i++)
			file->writeByte(0);

		delete file;

		updateIQPoints();
	}
}

void ScummEngine_v4::saveIQPoints(const byte *ptr, int size) {
	// save Indy3 IQ-points
	Common::OutSaveFile *file;
	Common::String filename = _targetName + ".iq";

	file = _saveFileMan->openForSaving(filename);
	if (file != nullptr && ptr != nullptr && size > 0) {
		file->write(ptr, size);
		delete file;
	}
}

void ScummEngine_v4::loadIQPoints(byte *ptr, int size) {
	// load Indy3 IQ-points
	Common::InSaveFile *file;
	Common::String filename = _targetName + ".iq";

	file = _saveFileMan->openForLoading(filename);
	if (file != nullptr) {
		byte *tmp = new byte[size]();
		int nread = file->read(tmp, size);
		if (nread >= 73)
			memcpy(ptr, tmp, nread);
		else
			warning("ScummEngine_v4::loadIQPoints(): read %d bytes, expected >= 73", nread);
		delete[] tmp;
		delete file;
	}
}

void ScummEngine_v4::o4_saveLoadGame() {
	getResultPos();
	byte slot;
	byte a = getVarOrDirectByte(PARAM_1);
	byte result = 0;
	Common::String dummyName;

	int saveRoom = 50;

	if (_game.id == GID_INDY3)
		saveRoom = 14;
	else if (_game.id == GID_LOOM)
		saveRoom = 70;

	_mainMenuIsActive = true;

	if ((_game.id == GID_MANIAC && _game.version <= 1) || (_game.id == GID_ZAK && _game.platform == Common::kPlatformC64)) {
		// Convert V0/V1 load/save screen (they support only one savegame per disk)
		// 1 Load
		// 2 Save
		slot = 1;
		switch (a) {
		case 1:
			_opcode = 0x40;
			break;
		case 2:
			_opcode = 0x80;
			break;
		default:
			error("o4_saveLoadGame: unknown param %d", a);
		}
	} else {
		slot = a & 0x1F;
		// Slot numbers in older games start with 0, in newer games with 1
		if (_game.version <= 2)
			slot++;
		_opcode = a & 0xE0;
	}

	switch (_opcode) {
	case 0x00: // Num slots available
		result = 100;
		break;
	case 0x20: // Drive
		if (_game.version <= 3) {
			// 0 = ???
			// [1,2] = Disk drive [A:,B:]
			// 3 = Hard drive
			result = 3;
		} else {
			// Set current drive
			result = 1;
		}
		break;
	case 0x40: // load
		_lastLoadedRoom = -1;

		// The original interpreter allowed you to change the name of the
		// savegame before loading it. As weird as that is, let's allow it...
		if (_game.version == 3) {
			char *ptr;
			int firstSlot = (_game.id == GID_LOOM) ? STRINGID_SAVENAME1_LOOM : STRINGID_SAVENAME1;
			ptr = (char *)getStringAddress(slot + firstSlot - 1);
			if (ptr) {
				if (!changeSavegameName(slot, ptr)) {
					warning("o4_saveLoadGame: Couldn't change savegame name");
				}
			}
		}

		if (loadState(slot, false))
			result = 3; // Success
		else
			result = 5; // Failed to load

		// If the loaded state loads a different room from the save menu room
		// it means that we are loading a game saved from the GMM. To correctly
		// handle this, we run the boot script, we reload the state, and then signal
		// the ScummEngine_v3::scummLoop_handleSaveLoad() function that we need to
		// perform the post load fixes.
		if (result == 3 && _currentRoom != saveRoom) {
			_loadFromLauncher = true;
			runBootscript();
			loadState(slot, false);
			_mainMenuIsActive = false;
			return;
		}

		break;
	case 0x80: // Save
		_lastLoadedRoom = -1;
		if (_game.version <= 3) {
			char name[32];
			if (_game.version <= 2) {
				// V2 and below use a hardcoded name for savestates
				Common::sprintf_s(name, "Game %c", 'A' + slot - 1);
			} else {
				// Use the name entered by the user...
				char* ptr;
				int firstSlot = (_game.id == GID_LOOM) ? STRINGID_SAVENAME1_LOOM : STRINGID_SAVENAME1;
				ptr = (char *)getStringAddress(slot + firstSlot - 1);
				Common::strlcpy(name, ptr, sizeof(name));
			}

			_saveLoadDescription = name;
			if (saveState(slot, false, dummyName))
				result = 0;
			else
				result = 2;
		} else {
			result = 2; // Failed to save
		}
		break;
	case 0xC0: // Test if the save file exists
		{
		Common::InSaveFile *file = nullptr;
		bool availableSaves[100];
		bool atLeastOneSaveExists = false;

		listSavegames(availableSaves, ARRAYSIZE(availableSaves));
		Common::String filename = makeSavegameName(slot, false);

		for (int i = 0; i < ARRAYSIZE(availableSaves); i++) {
			if (availableSaves[i]) {
				atLeastOneSaveExists = true;
				break;
			}
		}

		if (availableSaves[slot] && (file = _saveFileMan->openForLoading(filename))) {
			result = 6; // Save file exists
			delete file;
		} else if (_game.id == GID_LOOM && _game.platform == Common::kPlatformFMTowns && slot == 0 && atLeastOneSaveExists) {
			// LOOM Towns explicitly sets the result to 6 if the selected slot is 0;
			// also, it needs to have at least one savegame available, otherwise it would lead
			// to the game reaching towards a non-existent string, and crashing as a consequence.
			result = 6;
		} else
			result = (_game.id == GID_LOOM && _game.platform == Common::kPlatformFMTowns) ? 8 : 7; // Save file does not exist
		}

		break;
	default:
		error("o4_saveLoadGame: unknown subopcode %d", _opcode);
	}

	_mainMenuIsActive = false;

	setResult(result);

	// Did the script move? Update its state!
	updateScriptPtr();
	getScriptBaseAddress();
	resetScriptPointer();
}

} // End of namespace Scumm
