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

#include "lastexpress/game/savegame.h"

#include "lastexpress/data/cvcrfile.h"

#include "lastexpress/game/logic.h"

#include "lastexpress/menu/menu.h"

#include "lastexpress/debug.h"
#include "lastexpress/lastexpress.h"

#include "common/savefile.h"

namespace LastExpress {

SaveManager::SaveManager(LastExpressEngine *engine) {
	_engine = engine;
}

void SaveManager::writeSavePoint(CVCRFile *file, int saveType, int character, int value) {
	int32 originalFilePos;
	int32 paddingSize;
	int32 posAfterWriting;
	SVCRSavePointHeader savePointHeader;
	byte emptyHeader[15];

	savePointHeader.type = saveType;
	savePointHeader.magicNumber = 0xE660E660;
	savePointHeader.size = 0;
	savePointHeader.time = _engine->getLogicManager()->_gameTime;
	savePointHeader.partNumber = _engine->getLogicManager()->_globals[kGlobalChapter];
	savePointHeader.latestGameEvent = value;
	savePointHeader.emptyField1 = 0;
	savePointHeader.emptyField2 = 0;

	originalFilePos = file->tell();
	file->write(&savePointHeader, sizeof(SVCRSavePointHeader), 1, 0);
	file->flush();
	file->writeRLE(&character, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_gameTime, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_timeSpeed, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_realTime, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_activeNode, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_closeUp, 1, 1);
	file->writeRLE(&_engine->getLogicManager()->_nodeReturn, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_nodeReturn2, 4, 1);
	file->writeRLE(&_engine->getLogicManager()->_activeItem, 4, 1);

	file->writeRLE(_engine->getLogicManager()->_blockedViews, 4, 1000);
	file->writeRLE(_engine->getLogicManager()->_blockedX, 4, 16);
	file->writeRLE(_engine->getLogicManager()->_softBlockedX, 4, 16);
	file->writeRLE(_engine->getLogicManager()->_globals, 4, 128);
	file->writeRLE(_engine->getLogicManager()->_doneNIS, 1, 512);

	// Handle complex types (which were fine in the original, but not within a crossplatform context)...
	byte *inventoryBuffer = (byte *)malloc(32 * 7); // 32 items, 7 bytes each
	byte *doorsBuffer = (byte *)malloc(128 * 5); // 128 objects, 5 bytes each
	byte *charactersBuffer = (byte *)malloc(40 * 1262); // 128 objects, 5 bytes each

	assert(inventoryBuffer && doorsBuffer && charactersBuffer);

	// Copy Item data...
	for (int i = 0; i < 32; i++) {
		int offset = i * 7;
		inventoryBuffer[offset] = _engine->getLogicManager()->_items[i].mnum;
		WRITE_LE_UINT16(&inventoryBuffer[offset + 1], _engine->getLogicManager()->_items[i].closeUp);
		inventoryBuffer[offset + 3] = _engine->getLogicManager()->_items[i].useable;
		inventoryBuffer[offset + 4] = _engine->getLogicManager()->_items[i].haveIt;
		inventoryBuffer[offset + 5] = _engine->getLogicManager()->_items[i].inPocket;
		inventoryBuffer[offset + 6] = _engine->getLogicManager()->_items[i].floating;
	}

	file->writeRLE(inventoryBuffer, 7, 32);

	// Copy Door data...
	for (int i = 0; i < 128; i++) {
		int offset = i * 5;
		doorsBuffer[offset] = _engine->getLogicManager()->_doors[i].who;
		doorsBuffer[offset + 1] = _engine->getLogicManager()->_doors[i].status;
		doorsBuffer[offset + 2] = _engine->getLogicManager()->_doors[i].windowCursor;
		doorsBuffer[offset + 3] = _engine->getLogicManager()->_doors[i].handleCursor;
		doorsBuffer[offset + 4] = _engine->getLogicManager()->_doors[i].model;
	}

	file->writeRLE(doorsBuffer, 5, 128);

	// Copy Character data...
	for (int i = 0; i < 40; i++) {
		int offset = 0;
		Character characterStruct = getCharacter(i);

		// First copy CallParams (9 sets of 32 integers)...
		for (int j = 0; j < 9; j++) {
			for (int k = 0; k < 32; k++) {
				WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], characterStruct.callParams[j].parameters[k]);
				offset += 4;
			}
		}

		// Copy callbacks array (16 bytes)...
		for (int j = 0; j < 16; j++) {
			charactersBuffer[i * 1262 + offset++] = characterStruct.callbacks[j];
		}

		// Copy currentCall (1 byte)...
		charactersBuffer[i * 1262 + offset++] = characterStruct.currentCall;

		// Copy characterPosition (3 uint16)...
		WRITE_LE_UINT16(&charactersBuffer[i * 1262 + offset], characterStruct.characterPosition.position); offset += 2;
		WRITE_LE_UINT16(&charactersBuffer[i * 1262 + offset], characterStruct.characterPosition.location); offset += 2;
		WRITE_LE_UINT16(&charactersBuffer[i * 1262 + offset], characterStruct.characterPosition.car); offset += 2;

		// Copy the remaining basic fields...
		charactersBuffer[i * 1262 + offset++] = characterStruct.walkCounter;
		charactersBuffer[i * 1262 + offset++] = characterStruct.attachedConductor;
		charactersBuffer[i * 1262 + offset++] = characterStruct.inventoryItem;
		charactersBuffer[i * 1262 + offset++] = characterStruct.direction;

		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.waitedTicksUntilCycleRestart); offset += 2;
		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.currentFrameSeq1); offset += 2;
		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.currentFrameSeq2); offset += 2;
		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.elapsedFrames); offset += 2;
		WRITE_LE_INT16(&charactersBuffer[i * 1262 + offset], characterStruct.walkStepSize); offset += 2;

		charactersBuffer[i * 1262 + offset++] = characterStruct.clothes;
		charactersBuffer[i * 1262 + offset++] = characterStruct.position2;
		charactersBuffer[i * 1262 + offset++] = characterStruct.car2;
		charactersBuffer[i * 1262 + offset++] = characterStruct.doProcessEntity;
		charactersBuffer[i * 1262 + offset++] = characterStruct.needsPosFudge;
		charactersBuffer[i * 1262 + offset++] = characterStruct.needsSecondaryPosFudge;
		charactersBuffer[i * 1262 + offset++] = characterStruct.directionSwitch;

		// Copy string fields
		memcpy(&charactersBuffer[i * 1262 + offset], characterStruct.sequenceName, 13); offset += 13;
		memcpy(&charactersBuffer[i * 1262 + offset], characterStruct.sequenceName2, 13); offset += 13;
		memcpy(&charactersBuffer[i * 1262 + offset], characterStruct.sequenceNamePrefix, 7); offset += 7;
		memcpy(&charactersBuffer[i * 1262 + offset], characterStruct.sequenceNameCopy, 13); offset += 13;

		// Set pointers to zero (each 4 bytes)
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // frame1
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // frame2
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // sequence1
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // sequence2
		WRITE_LE_UINT32(&charactersBuffer[i * 1262 + offset], 0); offset += 4; // sequence3

		// At this point, offset should equal 1262!
		assert(offset == 1262);
	}

	file->writeRLE(charactersBuffer, 1262, 40);

	free(inventoryBuffer);
	free(doorsBuffer);
	free(charactersBuffer);

	_engine->getSoundManager()->saveSoundInfo(file);
	_engine->getMessageManager()->saveMessages(file);

	savePointHeader.size = file->flush();

	if ((savePointHeader.size & 0xF) != 0) {
		memset(emptyHeader, 0, sizeof(emptyHeader));
		paddingSize = ((~(savePointHeader.size & 0xFF) & 0xF) + 1);
		file->write(&emptyHeader, 1, paddingSize, 0);
		savePointHeader.size += paddingSize;
	}

	posAfterWriting = file->tell();
	file->seek(originalFilePos, 0);
	checkSavePointHeader(&savePointHeader);
	file->write(&savePointHeader, 32, 1, false);
	file->seek(posAfterWriting, 0);
}

void SaveManager::readSavePoint(CVCRFile *file, int *saveType, uint8 *character, int *saveEvent, bool skipSoundLoading) {
	int latestGameEvent;
	int32 originalPos;
	int32 posDiff;
	SVCRSavePointHeader savePointHeader;

	if (saveType && character && saveEvent) {
		*saveType = 1;
		*character = kCharacterCath;
		*saveEvent = 0;

		file->read(&savePointHeader, sizeof(SVCRSavePointHeader), 1, false, true);

		if (checkSavePointHeader(&savePointHeader)) {
			latestGameEvent = savePointHeader.latestGameEvent;
			*saveType = savePointHeader.type;
			*saveEvent = latestGameEvent;
			file->flush();
			originalPos = file->tell();

			// The original treats the "character" arg as uint8, but then asks
			// for a four bytes integer, causing a stack corruption around it.
			// This is our workaround...
			int32 intCharacter;
			file->readRLE(&intCharacter, 4, 1);
			*character = (uint8)(intCharacter & 0xFF);

			if (*character >= 40) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_gameTime, 4, 1);
			if (_engine->getLogicManager()->_gameTime < 1061100) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			if (_engine->getLogicManager()->_gameTime > 4941000) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_timeSpeed, 4, 1);
			if (_engine->getLogicManager()->_gameTime > 4941000) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_realTime, 4, 1);

			file->readRLE(&_engine->getLogicManager()->_activeNode, 4, 1);
			if (_engine->getLogicManager()->_activeNode >= 2500) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_closeUp, 1, 1);
			if (_engine->getLogicManager()->_closeUp > 1) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_nodeReturn, 4, 1);
			if (_engine->getLogicManager()->_nodeReturn >= 2500) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_nodeReturn2, 4, 1);
			if (_engine->getLogicManager()->_nodeReturn2 >= 2500) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(&_engine->getLogicManager()->_activeItem, 4, 1);
			if (_engine->getLogicManager()->_activeItem >= 32) {
				error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
			}

			file->readRLE(_engine->getLogicManager()->_blockedViews, 4, 1000);
			file->readRLE(_engine->getLogicManager()->_blockedX, 4, 16);
			file->readRLE(_engine->getLogicManager()->_softBlockedX, 4, 16);
			file->readRLE(_engine->getLogicManager()->_globals, 4, 128);
			file->readRLE(_engine->getLogicManager()->_doneNIS, 1, 512);

			// Handle complex types (which were fine in the original, but not within a crossplatform context)...
			byte *inventoryBuffer = (byte *)malloc(32 * 7);     // 32 items, 7 bytes each
			byte *doorsBuffer = (byte *)malloc(128 * 5);      // 128 objects, 5 bytes each
			byte *charactersBuffer = (byte *)malloc(40 * 1262); // 40 characters, 1262 bytes each

			assert(inventoryBuffer && doorsBuffer && charactersBuffer);

			// Read data from file
			file->readRLE(inventoryBuffer, 7, 32);
			file->readRLE(doorsBuffer, 5, 128);
			file->readRLE(charactersBuffer, 1262, 40);

			// Copy Item data from buffer to structures
			for (int i = 0; i < 32; i++) {
				int offset = i * 7;
				_engine->getLogicManager()->_items[i].mnum = inventoryBuffer[offset];
				_engine->getLogicManager()->_items[i].closeUp = READ_LE_UINT16(&inventoryBuffer[offset + 1]);
				_engine->getLogicManager()->_items[i].useable = inventoryBuffer[offset + 3];
				_engine->getLogicManager()->_items[i].haveIt = inventoryBuffer[offset + 4];
				_engine->getLogicManager()->_items[i].inPocket = inventoryBuffer[offset + 5];
				_engine->getLogicManager()->_items[i].floating = inventoryBuffer[offset + 6];
			}

			// Copy Door data from buffer to structures
			for (int i = 0; i < 128; i++) {
				int offset = i * 5;
				_engine->getLogicManager()->_doors[i].who = doorsBuffer[offset];
				_engine->getLogicManager()->_doors[i].status = doorsBuffer[offset + 1];
				_engine->getLogicManager()->_doors[i].windowCursor = doorsBuffer[offset + 2];
				_engine->getLogicManager()->_doors[i].handleCursor = doorsBuffer[offset + 3];
				_engine->getLogicManager()->_doors[i].model = doorsBuffer[offset + 4];
			}

			// Copy Character data from buffer to structures
			for (int i = 0; i < 40; i++) {
				int offset = 0;
				Character *characterStruct = &getCharacter(i);

				// Copy CallParams (9 sets of 32 integers)
				for (int j = 0; j < 9; j++) {
					for (int k = 0; k < 32; k++) {
						characterStruct->callParams[j].parameters[k] = READ_LE_UINT32(&charactersBuffer[i * 1262 + offset]);
						offset += 4;
					}
				}

				// Copy callbacks array (16 bytes)
				for (int j = 0; j < 16; j++) {
					characterStruct->callbacks[j] = charactersBuffer[i * 1262 + offset]; offset++;
				}

				// Copy currentCall (1 byte)
				characterStruct->currentCall = charactersBuffer[i * 1262 + offset]; offset++;

				// Copy characterPosition (3 uint16)
				characterStruct->characterPosition.position = READ_LE_UINT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->characterPosition.location = READ_LE_UINT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->characterPosition.car = READ_LE_UINT16(&charactersBuffer[i * 1262 + offset]); offset += 2;

				// Copy the remaining basic fields
				characterStruct->walkCounter = charactersBuffer[i * 1262 + offset++];
				characterStruct->attachedConductor = charactersBuffer[i * 1262 + offset++];
				characterStruct->inventoryItem = charactersBuffer[i * 1262 + offset++];
				characterStruct->direction = charactersBuffer[i * 1262 + offset++];

				characterStruct->waitedTicksUntilCycleRestart = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->currentFrameSeq1 = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->currentFrameSeq2 = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->elapsedFrames = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;
				characterStruct->walkStepSize = READ_LE_INT16(&charactersBuffer[i * 1262 + offset]); offset += 2;

				characterStruct->clothes = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->position2 = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->car2 = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->doProcessEntity = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->needsPosFudge = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->needsSecondaryPosFudge = charactersBuffer[i * 1262 + offset]; offset++;
				characterStruct->directionSwitch = charactersBuffer[i * 1262 + offset]; offset++;

				// Copy string fields
				memcpy(characterStruct->sequenceName, &charactersBuffer[i * 1262 + offset], 13); offset += 13;
				memcpy(characterStruct->sequenceName2, &charactersBuffer[i * 1262 + offset], 13); offset += 13;
				memcpy(characterStruct->sequenceNamePrefix, &charactersBuffer[i * 1262 + offset], 7); offset += 7;
				memcpy(characterStruct->sequenceNameCopy, &charactersBuffer[i * 1262 + offset], 13); offset += 13;

				// Skip pointer data...
				offset += 4; // frame1
				offset += 4; // frame2
				offset += 4; // sequence1
				offset += 4; // sequence2
				offset += 4; // sequence3

				// At this point, offset should equal 1262!
				assert(offset == 1262);
			}

			free(inventoryBuffer);
			free(doorsBuffer);
			free(charactersBuffer);

			_engine->getSoundManager()->loadSoundInfo(file, skipSoundLoading);
			_engine->getMessageManager()->loadMessages(file);

			_engine->getLogicManager()->_globals[kGlobalChapter] = savePointHeader.partNumber;

			file->flush();

			posDiff = (file->tell() - originalPos) & 0xFF;
			if ((posDiff & 0xF) != 0)
				file->seek(((~posDiff & 0xF) + 1), 1);

			for (int i = 0; i < 40; i++) {
				getCharacter(i).frame1 = nullptr;
				getCharacter(i).frame2 = nullptr;
				getCharacter(i).sequence1 = nullptr;
				getCharacter(i).sequence2 = nullptr;
				getCharacter(i).sequence3 = nullptr;
			}
		}
	}
}

void SaveManager::validateSaveFile(bool flag) {
	SVCRSavePointHeader savePointHeader;
	SVCRFileHeader fileHeader;

	CVCRFile *tempFile = new CVCRFile(_engine);
	CVCRFile *saveFile = new CVCRFile(_engine);

	bool hasValidationError = false;

	if (flag) {
		if (_engine->_savePointHeaders)
			_engine->getMemoryManager()->freeMem(_engine->_savePointHeaders);

		_engine->_savePointHeaders = (SVCRSavePointHeader *)_engine->getMemoryManager()->allocMem(
			sizeof(SVCRSavePointHeader), _engine->_savegameFilename, kCharacterMaster
		);

		if (!_engine->_savePointHeaders) {
			error("Out of memory");
		}

		_engine->_savePointHeaders->time = _engine->isDemo() ? 2241000 : 1037700;
		_engine->_savePointHeaders->partNumber = _engine->isDemo() ? 3 : 1;
	}

	saveFile->open(_engine->_savegameFilename, CVCRMODE_RB);

	saveFile->seek(0, SEEK_END);
	int fileSize = saveFile->tell();
	saveFile->seek(0, SEEK_SET);

	if (fileSize >= (int32)sizeof(SVCRFileHeader)) {
		saveFile->read(&fileHeader, sizeof(SVCRFileHeader), 1, false, true);
		if (checkFileHeader(&fileHeader)) {
			if (flag) {
				if (_engine->_savePointHeaders)
					_engine->getMemoryManager()->freeMem(_engine->_savePointHeaders);

				_engine->_savePointHeaders = (SVCRSavePointHeader *)_engine->getMemoryManager()->allocMem(
					sizeof(SVCRSavePointHeader) * (fileHeader.numVCRGames + 1),
					_engine->_savegameFilename,
					kCharacterMaster
				);

				if (!_engine->_savePointHeaders) {
					error("Out of memory");
				}

				_engine->_savePointHeaders->time = _engine->isDemo() ? 2241000 : 1037700;
				_engine->_savePointHeaders->partNumber = _engine->isDemo() ? 3 : 1;
			}

			for (int i = 0; fileSize >= (int32)sizeof(SVCRFileHeader) && i < fileHeader.numVCRGames; ++i) {
				_engine->getSoundManager()->soundThread();

				saveFile->read(&savePointHeader, sizeof(SVCRSavePointHeader), 1, false, true);
				if (flag) {
					memcpy(&_engine->_savePointHeaders[i + 1], &savePointHeader, sizeof(savePointHeader));
				}

				fileSize -= sizeof(SVCRSavePointHeader);
				if (fileSize >= 0) {
					if (checkSavePointHeader(&savePointHeader)) {
						fileSize -= savePointHeader.size;
						if (fileSize >= 0) {
							saveFile->seek(savePointHeader.size, SEEK_CUR);
						} else {
							hasValidationError = true;
						}
					} else {
						fileSize = 0;
						hasValidationError = true;
					}
				} else {
					hasValidationError = true;
				}
			}

			saveFile->close();
		} else {
			hasValidationError = true;
			saveFile->close();
		}
	} else {
		hasValidationError = true;
		saveFile->close();
	}

	if (hasValidationError) {
		saveFile->open(_engine->_savegameFilename, CVCRMODE_RB);
		saveFile->seek(0, SEEK_END);
		fileSize = saveFile->tell();
		saveFile->seek(0, SEEK_SET);

		if (fileSize < (int32)sizeof(SVCRFileHeader)) {
			if (fileSize) {
				error("Attempting to salvage corrupt save game file \"%s\"", _engine->_savegameFilename);
			}

			saveFile->close();
			_engine->getVCR()->virginSaveFile();

			delete tempFile;
			delete saveFile;

			return;
		}

		saveFile->read(&fileHeader, sizeof(SVCRFileHeader), 1, false, true);
		if (!checkFileHeader(&fileHeader)) {
			error("Attempting to salvage corrupt save game file \"%s\"", _engine->_savegameFilename);
			saveFile->close();
			_engine->getVCR()->virginSaveFile();

			delete tempFile;
			delete saveFile;

			return;
		}

		if (flag) {
			if (_engine->_savePointHeaders)
				_engine->getMemoryManager()->freeMem(_engine->_savePointHeaders);

			_engine->_savePointHeaders = (SVCRSavePointHeader *)_engine->getMemoryManager()->allocMem(
				sizeof(SVCRSavePointHeader) * (fileHeader.numVCRGames + 1),
				_engine->_savegameFilename,
				kCharacterMaster
			);

			if (!_engine->_savePointHeaders) {
				error("Out of memory");
			}

			_engine->_savePointHeaders->time = _engine->isDemo() ? 2241000 : 1037700;
			_engine->_savePointHeaders->partNumber = _engine->isDemo() ? 3 : 1;
		}

		int offset = sizeof(SVCRFileHeader);
		tempFile->open("temp.egg", CVCRMODE_WB);
		tempFile->seek(sizeof(SVCRFileHeader), 0);
		tempFile->close();
		fileHeader.realWritePos = sizeof(SVCRFileHeader);
		fileHeader.nextWritePos = sizeof(SVCRFileHeader);
		fileHeader.lastIsTemporary = 0;
		int numSavePoints = 0;

		for (int j = 0; true; j++) {
			if (fileSize < (int32)sizeof(SVCRFileHeader) || j >= fileHeader.numVCRGames) {
				saveFile->close();
				fileHeader.numVCRGames = numSavePoints;
				tempFile->open("temp.egg", CVCRMODE_RWB);
				tempFile->seek(0, SEEK_SET);
				tempFile->write(&fileHeader, sizeof(SVCRFileHeader), 1, false);
				tempFile->seek(offset, SEEK_SET);
				tempFile->close();

				if (removeSavegame(_engine->_savegameFilename)) {
					error("Error deleting file \"%s\"", _engine->_savegameFilename);
				} else if (renameSavegame("temp.egg", _engine->_savegameFilename)) {
					error("Error renaming file \"%s\" to \"%s\"", "temp.egg", _engine->_savegameFilename);
				}

				delete tempFile;
				delete saveFile;

				return;
			}

			_engine->getSoundManager()->soundThread();
			saveFile->read(&savePointHeader, sizeof(SVCRSavePointHeader), 1, false, true);
			if (flag) {
				memcpy(&_engine->_savePointHeaders[j + 1], &savePointHeader, sizeof(savePointHeader));
			}

			fileSize -= sizeof(SVCRSavePointHeader);
			if (fileSize < 0)
				break;

			if (checkSavePointHeader(&savePointHeader)) {
				fileSize -= savePointHeader.size;
				if (fileSize < 0)
					break;

				numSavePoints++;

				byte *tempMem = (byte *)malloc(savePointHeader.size);

				saveFile->read(tempMem, savePointHeader.size, 1, false, true);
				tempFile->open("temp.egg", CVCRMODE_RWB);
				tempFile->seek(offset, SEEK_SET);

				if (savePointHeader.type != 3 && savePointHeader.type != 4)
					fileHeader.realWritePos = offset;

				tempFile->write(&savePointHeader, sizeof(SVCRSavePointHeader), 1, false);
				tempFile->write(tempMem, savePointHeader.size, 1, false);
				tempFile->close();

				free(tempMem);

				offset += savePointHeader.size + sizeof(SVCRSavePointHeader);
				if (savePointHeader.type == 3 || savePointHeader.type == 4) {
					fileHeader.lastIsTemporary = 1;
				} else {
					fileHeader.lastIsTemporary = 0;
					fileHeader.nextWritePos = offset;
				}
			} else {
				fileSize = 0;
				error("Attempting to salvage corrupt save game file \"%s\"", _engine->_savegameFilename);
			}
		}

		error("Attempting to salvage corrupt save game file \"%s\"", _engine->_savegameFilename);
	}

	delete tempFile;
	delete saveFile;
}

bool SaveManager::checkFileHeader(SVCRFileHeader *fileHeader) {
	if (fileHeader->magicNumber == 0x12001200 &&
		fileHeader->numVCRGames >= 0 &&
		fileHeader->nextWritePos >= (int32)sizeof(SVCRFileHeader) &&
		fileHeader->realWritePos >= (int32)sizeof(SVCRFileHeader) &&
		fileHeader->lastIsTemporary < 2 &&
		fileHeader->brightness <= 6 &&
		fileHeader->volume < 8) {

		if (fileHeader->saveVersion == 9) {
			return true;
		} else {
			error("Save game file \"%s\" is incompatible with this version of the game", _engine->_savegameFilename);
			return false;
		}
	} else {
		error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
		return false;
	}
}

bool SaveManager::checkSavePointHeader(SVCRSavePointHeader *savePointHeader) {
	if ((uint32)savePointHeader->magicNumber == 0xE660E660) {
		if (savePointHeader->type > 0 && savePointHeader->type <= 5) {
			if (savePointHeader->time >= 1061100 && savePointHeader->time <= 4941000) {
				if (savePointHeader->size > 0 && (savePointHeader->size & 0xF) == 0 && savePointHeader->partNumber > 0)
					return true;
			}
		}
	}

	error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);

	return false;
}

void SaveManager::continueGame() {
	uint8 character;
	int saveEvent;
	int saveType;
	SVCRFileHeader header;

	_engine->_savegame->open(_engine->_savegameFilename, CVCRMODE_RB);
	_engine->_savegame->read(&header, sizeof(SVCRFileHeader), 1, false, true);

	if (checkFileHeader(&header)) {
		_engine->_savegame->seek(header.realWritePos, SEEK_SET);
		readSavePoint(_engine->_savegame, &saveType, &character, &saveEvent, header.lastIsTemporary);
		_engine->getLogicManager()->_lastSavegameSessionTicks = _engine->getLogicManager()->_realTime;

		if (header.lastIsTemporary) {
			_engine->getSoundManager()->destroyAllSound();
			readSavePoint(_engine->_savegame, &saveType, &character, &saveEvent, 0);
		}

		_engine->_savegame->close();
		_engine->getOtisManager()->wipeAllGSysInfo();
		_engine->getLogicManager()->CONS_All(false, character);
	} else {
		_engine->_savegame->close();
	}
}

void SaveManager::startRewoundGame() {
	SVCRFileHeader header;
	SVCRSavePointHeader savePointHeader;
	int saveEvent;
	int saveType;
	uint8 character = 0;

	CVCRFile *saveFile = new CVCRFile(_engine);
	byte *buf = (byte *)malloc(0x2000);

	_engine->_fightSkipCounter = 0;

	_engine->_savegame->open(_engine->_savegameNames[_engine->_currentGameFileColorId], CVCRMODE_RB);
	_engine->_savegameFilename = _engine->_savegameTempNames[_engine->_currentGameFileColorId];
	saveFile->open(_engine->_savegameFilename, CVCRMODE_WB);

	header.nextWritePos = sizeof(SVCRFileHeader);
	header.numVCRGames = _engine->_currentSavePoint;
	header.realWritePos = sizeof(SVCRFileHeader);
	header.magicNumber = 0x12001200;
	header.lastIsTemporary = 0;
	header.brightness = _engine->getGraphicsManager()->getGammaLevel();
	header.volume = _engine->getSoundManager()->getMasterVolume();
	header.saveVersion = 9;
	saveFile->write(&header, sizeof(header), 1, false);

	_engine->_savegame->seek(sizeof(header), SEEK_SET);

	if (_engine->_currentSavePoint > 1) {
		int count = _engine->_currentSavePoint - 1;
		do {
			_engine->_savegame->read(&savePointHeader, sizeof(savePointHeader), 1, false, true);
			saveFile->write(&savePointHeader, sizeof(savePointHeader), 1, false);
			for (; savePointHeader.size > 0x2000; savePointHeader.size -= 0x2000) {
				_engine->_savegame->read(buf, 0x2000, 1, false, true);
				saveFile->write(buf, 0x2000, 1, false);
			}
			_engine->_savegame->read(buf, savePointHeader.size, 1, false, true);
			saveFile->write(buf, savePointHeader.size, 1, false);
			--count;
		} while (count);
	}

	free(buf);
	buf = nullptr;

	if (_engine->_currentSavePoint) {
		readSavePoint(_engine->_savegame, &saveType, &character, &saveEvent, false);
		_engine->_savegame->close();
		_engine->getLogicManager()->_lastSavegameSessionTicks = _engine->getLogicManager()->_realTime;
		header.realWritePos = saveFile->tell();

		writeSavePoint(saveFile, saveType, character, saveEvent);

		header.nextWritePos = saveFile->tell();
		checkFileHeader(&header);
		saveFile->seek(0, SEEK_SET);
		saveFile->write(&header, sizeof(header), 1, false);
	}

	_engine->_savegame->close();
	saveFile->close();
	delete saveFile;

	_engine->_gracePeriodIndex = _engine->_currentSavePoint;
	_engine->_gracePeriodTimer = _engine->getLogicManager()->_globals[kGlobalJacket] < 2 ? 225 : 450;

	if (_engine->_currentSavePoint) {
		_engine->getOtisManager()->wipeAllGSysInfo();
		_engine->getLogicManager()->CONS_All(false, character);
	} else {
		_engine->startNewGame();
	}
}

bool SaveManager::fileExists(const char *filename) {
	return g_system->getSavefileManager()->exists(_engine->getTargetName() + "-" + Common::String(filename));
}

bool SaveManager::removeSavegame(const char *filename) {
	return !g_system->getSavefileManager()->removeSavefile(_engine->getTargetName() + "-" + Common::String(filename));
}

bool SaveManager::renameSavegame(const char *oldName, const char *newName) {
	return !g_system->getSavefileManager()->renameSavefile(
		_engine->getTargetName() + "-" + Common::String(oldName),
		_engine->getTargetName() + "-" + Common::String(newName), false);
}

} // End of namespace LastExpress
