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

#include "lastexpress/lastexpress.h"
#include "lastexpress/data/cvcrfile.h"
#include "lastexpress/menu/clock.h"
#include "lastexpress/game/vcr.h"

#include "common/config-manager.h"
#include "common/savefile.h"

#include "engines/metaengine.h"

namespace LastExpress {

VCR::VCR(LastExpressEngine *engine) {
	_engine = engine;
}

void VCR::virginSaveFile() {
	CVCRFile *saveFile = new CVCRFile(_engine);
	SVCRFileHeader fileHeader;

	assert(_engine->_savegameFilename);

	fileHeader.magicNumber = 0x12001200;
	fileHeader.numVCRGames = 0;
	fileHeader.nextWritePos = sizeof(SVCRFileHeader);
	fileHeader.realWritePos = sizeof(SVCRFileHeader);
	fileHeader.lastIsTemporary = 0;
	fileHeader.brightness = _engine->getGraphicsManager()->getGammaLevel();
	fileHeader.saveVersion = 9;
	fileHeader.volume = _engine->_soundMan->getMasterVolume();

	saveFile->open(_engine->_savegameFilename, CVCRMODE_WB);
	saveFile->write(&fileHeader, sizeof(SVCRFileHeader), 1, 0);
	saveFile->close();
	delete saveFile;
}

void VCR::writeSavePoint(int type, int entity, int event) {
	SVCRSavePointHeader savePointHeader;
	SVCRFileHeader fileHeader;

	CVCRFile *saveFile = new CVCRFile(_engine);

	if (_engine->getLogicManager()->_activeNode <= 30) {
		delete saveFile;
		return;
	}

	if (_engine->_savegameTempNames[_engine->_currentGameFileColorId] == _engine->_savegameFilename && !_engine->_gracePeriodTimer) {
		makePermanent();
	}

	saveFile->open(_engine->_savegameFilename, CVCRMODE_RWB);
	saveFile->read(&fileHeader, 32, 1, 0, 1);

	if (!_engine->getSaveManager()->checkFileHeader(&fileHeader)) {
		saveFile->close();
		delete saveFile;
		return;
	}

	saveFile->seek(fileHeader.nextWritePos, 0);

	if (fileHeader.numVCRGames > 0) {
		saveFile->seek(fileHeader.realWritePos, 0);
		saveFile->read(&savePointHeader, 32, 1, 0, 1);

		if ((!_engine->getSaveManager()->checkSavePointHeader(&savePointHeader)) ||
			(savePointHeader.time > _engine->getLogicManager()->_gameTime) ||
			(type == 5 && savePointHeader.time == _engine->getLogicManager()->_gameTime)) {
			saveFile->close();
			delete saveFile;
			return;
		}

		saveFile->seek(fileHeader.nextWritePos, 0);
		if ((type == 1 || type == 2) && savePointHeader.type == 5 && (_engine->getLogicManager()->_gameTime - savePointHeader.time) < 2700) {
			saveFile->seek(fileHeader.realWritePos, 0);
			fileHeader.numVCRGames--;
		}
	}

	if (type != 3 && type != 4)
		fileHeader.realWritePos = saveFile->tell();

	_engine->getSaveManager()->writeSavePoint(saveFile, type, entity, event);

	if (!fileHeader.lastIsTemporary)
		++fileHeader.numVCRGames;

	if (type == 3 || type == 4) {
		fileHeader.lastIsTemporary = 1;
	} else {
		fileHeader.lastIsTemporary = 0;
		fileHeader.nextWritePos = saveFile->tell();
		_engine->getLogicManager()->_lastSavegameSessionTicks = _engine->getLogicManager()->_realTime;
	}

	_engine->getSaveManager()->checkFileHeader(&fileHeader);

	saveFile->seek(0, 0);
	saveFile->write(&fileHeader, sizeof(SVCRFileHeader), 1, 0);
	saveFile->close();
	delete saveFile;
}

void VCR::selectFromName(const char *filename) {
	 // Search through valid savegame names...
	for (int saveNameIndex = 0; saveNameIndex < 6; saveNameIndex++) {
		Common::String curSaveName = _engine->getTargetName() + "-" + _engine->_savegameNames[saveNameIndex];

		if (Common::String(filename).equalsIgnoreCase(curSaveName)) {
			// Found a match!
			setCurrentGameColor(saveNameIndex);
			return;
		}
	}
}

void VCR::shuffleGames() {
	int currentSlot;

	// Process all save game slots
	for (currentSlot = 0; currentSlot < ARRAYSIZE(_engine->_savegameNames); currentSlot++) {

		const char *currentFile = _engine->_savegameNames[currentSlot];

		Common::InSaveFile *saveFile = _engine->getSaveFileManager()->openForLoading(_engine->getTargetName() + "-" + Common::String(currentFile));

		bool slotFilled = false;

		// Check if current slot has a valid save
		if (saveFile) {
			if (saveFile->size() <= 32) {
				delete saveFile;

				// Remove invalid/corrupted save files
				if (_engine->getSaveManager()->removeSavegame(currentFile) != 0) {
					error("Error deleting file \"%s\"", currentFile);
				}

				// Also remove the timestamp
				Common::String tsName = currentFile;
				tsName.chop(4);
				tsName = _engine->getTargetName() + "-" + tsName + ".timestamp";

				if (g_system->getSavefileManager()->exists(tsName))
					g_system->getSavefileManager()->removeSavefile(tsName);
			} else {
				slotFilled = true;
				delete saveFile;
			}
		}

		// If slot is empty, try to find a valid save to move here
		if (!slotFilled && currentSlot < ARRAYSIZE(_engine->_savegameNames) - 1) {
			for (const char **candidateFile = &_engine->_savegameNames[currentSlot + 1];
				 candidateFile < _engine->_savegameTempNames;
				 candidateFile++) {

				Common::InSaveFile *candidateSave = _engine->getSaveFileManager()->openForLoading(_engine->getTargetName() + "-" + Common::String(*candidateFile));

				if (candidateSave) {
					if (candidateSave->size() > 32) {
						delete candidateSave;

						// Move this valid save to the empty slot
						if (_engine->getSaveManager()->renameSavegame(*candidateFile, currentFile) != 0) {
							error("Error renaming file \"%s\" to \"%s\"", *candidateFile, currentFile);
						}

						slotFilled = true;
						break;
					} else {
						delete candidateSave;

						// Remove invalid candidate files
						if (_engine->getSaveManager()->removeSavegame(*candidateFile) != 0) {
							error("Error deleting file \"%s\"", *candidateFile);
						}

						// Also remove the timestamp
						Common::String tsName = *candidateFile;
						tsName.chop(4);
						tsName = _engine->getTargetName() + "-" + tsName + ".timestamp";

						if (g_system->getSavefileManager()->exists(tsName))
							g_system->getSavefileManager()->removeSavefile(tsName);
					}
				}
			}
		}
	}
	
	// Set the current game color based on the most recent save...
	Common::String currentSaveName = _engine->getTargetName() + "-" + Common::String(_engine->_savegameNames[currentSlot % 6]);
	if (_engine->_currentGameFileColorId == -1 || !_engine->getSaveFileManager()->exists(currentSaveName)) {
		setCurrentGameColor(0); // Default color
		int32  newestSaveSecs = 0;
		uint32 newestSaveTime = 0;
		uint32 newestSaveDate = 0;

		// Find the most recently modified save file
		for (int i = 0; i < 6; i++) {
			Common::String tsFilename = _engine->_savegameNames[i];
			tsFilename.chop(4);
			tsFilename = _engine->getTargetName() + "-" + tsFilename + ".timestamp";

			Common::InSaveFile *saveFile = _engine->getSaveFileManager()->openForLoading(tsFilename);

			if (saveFile) {
				int32 eshSecs = saveFile->readSint32LE();

				ExtendedSavegameHeader esh;
				if (_engine->getMetaEngine()->readSavegameHeader(saveFile, &esh)) {
					if (esh.date > newestSaveDate ||
						(esh.date == newestSaveDate && esh.time > newestSaveTime) ||
						(esh.date == newestSaveDate && esh.time == newestSaveTime && eshSecs > newestSaveSecs)) {
						newestSaveSecs = eshSecs;
						newestSaveTime = esh.time;
						newestSaveDate = esh.date;

						setCurrentGameColor(i);
					}

					// SwapBytesVCR(i);
				}
			}

			delete saveFile;
		}
	}
}

void VCR::setCurrentGameColor(int index) {
	_engine->_currentGameFileColorId = index;
	_engine->_savegameFilename = _engine->_savegameNames[index];
}

void VCR::init(bool doSaveGameFlag, int saveType, int32 time) {
	int cdNum;
	SVCRFileHeader header;
	char path[80];

	int32 chosenTime = 0;
	bool flag = true;
	bool writeSavePoint = false;

	if (_engine->_savegame && _engine->_savegame->fileIsOpen())
		_engine->_savegame->close();

	if (_engine->_gracePeriodTimer) {
		if (_engine->isDemo()) {
			time = 0;
			_engine->_gracePeriodTimer = 0;
			doSaveGameFlag = false;

			if (_engine->getSaveManager()->removeSavegame(_engine->_savegameFilename)) {
				error("Error deleting file \"%s\"", _engine->_savegameFilename);
			}

			_engine->_currentSavePoint = _engine->_gracePeriodIndex;
			flag = false;
			_engine->_savegameFilename = _engine->_savegameNames[_engine->_currentGameFileColorId];
		} else {
			chosenTime = 0;

			if (_engine->getLogicManager()->_globals[kGlobalChapter] <= 1) {
				cdNum = 1;
			} else {
				cdNum = (_engine->getLogicManager()->_globals[kGlobalChapter] > 3) + 2;
			}

			if (_engine->getArchiveManager()->isCDAvailable(cdNum, path, sizeof(path))) {
				writeSavePoint = 0;
				_engine->_gracePeriodTimer = 0;

				if (_engine->getSaveManager()->removeSavegame(_engine->_savegameFilename)) {
					error("Error deleting file \"%s\"", _engine->_savegameFilename);
				}

				flag = false;
				_engine->_currentSavePoint = _engine->_gracePeriodIndex;
				_engine->_savegameFilename = _engine->_savegameNames[_engine->_currentGameFileColorId];
			} else {
				writeSavePoint = false;
			}
		}
	} else {
		if (_engine->_savegameFilename == _engine->_savegameTempNames[_engine->_currentGameFileColorId])
			_engine->getVCR()->makePermanent();

		if (!_engine->isDemo()) {
			writeSavePoint = doSaveGameFlag;
			chosenTime = time;
		}
	}

	if (!_engine->getSaveManager()->fileExists(_engine->_savegameFilename))
		_engine->getVCR()->virginSaveFile();

	if (_engine->isDemo()) {
		if (doSaveGameFlag)
			_engine->getVCR()->writeSavePoint(3, kCharacterCath, 0);
	} else {
		if (writeSavePoint)
			_engine->getVCR()->writeSavePoint(3, kCharacterCath, 0);
	}

	if (!_engine->_gracePeriodTimer &&
		_engine->getSaveManager()->fileExists(_engine->_savegameTempNames[_engine->_currentGameFileColorId]) &&
		_engine->getSaveManager()->removeSavegame(_engine->_savegameTempNames[_engine->_currentGameFileColorId])
		) {
		error("Error deleting file \"%s\"", _engine->_savegameTempNames[_engine->_currentGameFileColorId]);
	}

	_engine->getSaveManager()->validateSaveFile(true);

	if (!_engine->_savegame)
		_engine->_savegame = new CVCRFile(_engine);

	_engine->_savegame->open(_engine->_savegameFilename, CVCRMODE_RB);
	_engine->_savegame->read(&header, sizeof(SVCRFileHeader), 1, false, true);

	if (!_engine->getSaveManager()->checkFileHeader(&header)) {
		_engine->_savegame->close();
		error("Save game file \"%s\" is corrupt", _engine->_savegameFilename);
	}

	_engine->_lastSavePointIdInFile = header.numVCRGames;
	_engine->_gameTimeOfLastSavePointInFile = _engine->_savePointHeaders[header.numVCRGames].time;

	if (flag)
		_engine->_currentSavePoint = _engine->_lastSavePointIdInFile;

	if (!_engine->_gracePeriodTimer)
		_engine->_gracePeriodIndex = 0;

	if (!_engine->getLogicManager()->_globals[kGlobalChapter])
		_engine->getLogicManager()->_globals[kGlobalChapter] = 1;

	_engine->getLogicManager()->_gameTime = _engine->_savePointHeaders[_engine->_currentSavePoint].time;
	_engine->getLogicManager()->_globals[kGlobalChapter] = _engine->_savePointHeaders[_engine->_currentSavePoint].partNumber;

	if (_engine->_gameTimeOfLastSavePointInFile >= 1061100) {
		_engine->getClock()->startClock(_engine->getLogicManager()->_gameTime);

		if (_engine->isDemo()) {
			// Demo: use modified time parameter
			_engine->getVCR()->autoRewind(saveType, time);
		} else {
			_engine->getVCR()->autoRewind(saveType, chosenTime);
		}
	}
}

void VCR::autoRewind(int saveType, int32 time) {
	int selectedIdx = 0;

	if (time) {
		switch (saveType) {
		case 0:
			if (_engine->_currentSavePoint <= time) {
				selectedIdx = 1;
			} else {
				selectedIdx = _engine->_currentSavePoint - time;
			}

			break;
		case 1:
			if (time < 1061100)
				break;

			if (!_engine->_currentSavePoint)
				return;

			selectedIdx = _engine->_currentSavePoint;
			do {
				if (_engine->_savePointHeaders[selectedIdx].time <= time)
					break;

				selectedIdx--;
			} while (selectedIdx);

			break;
		case 2:
			if (!_engine->_currentSavePoint)
				return;

			selectedIdx = _engine->_currentSavePoint;
			do {
				if (_engine->_savePointHeaders[selectedIdx].latestGameEvent == time)
					break;

				selectedIdx--;
			} while (selectedIdx);

			break;
		case 3:
			selectedIdx = _engine->_currentSavePoint;
			if (_engine->_currentSavePoint > 1) {
				do {
					if (_engine->_savePointHeaders[selectedIdx].latestGameEvent == time)
						break;

					selectedIdx--;
				} while (selectedIdx != 1);
			}

			selectedIdx--;
			break;
		default:
			break;
		}

		if (selectedIdx) {
			_currentSavePointInVCR = selectedIdx;
			_engine->getClock()->setClock(_engine->_savePointHeaders[selectedIdx].time);
		}
	}
}

void VCR::free() {
	_engine->getClock()->endClock();

	if (_engine->_savePointHeaders) {
		_engine->getMemoryManager()->freeMem(_engine->_savePointHeaders);
		_engine->_savePointHeaders = nullptr;
	}

	_engine->_savegame->close();
}

bool VCR::isVirgin(int savegameIndex) {
	SVCRFileHeader header;
	CVCRFile *file = new CVCRFile(_engine);

	if (_engine->getSaveManager()->fileExists(_engine->_savegameNames[savegameIndex]) &&
		file->open(_engine->_savegameNames[savegameIndex], CVCRMODE_RB)) {
		if (file->read(&header, sizeof(SVCRFileHeader), 1, false, false) &&
			_engine->getSaveManager()->checkFileHeader(&header) && header.numVCRGames > 0) {
			file->close();
			delete file;

			return false;
		}
	}

	file->close();
	delete file;

	return true;
}

bool VCR::currentEndsGame() {
	SVCRSavePointHeader *header = &_engine->_savePointHeaders[_engine->_currentSavePoint];
	int32 latestGameEvent = header->latestGameEvent;

	return _engine->_lastSavePointIdInFile == _engine->_currentSavePoint && header->type == 2 && 
		(  latestGameEvent == kEventAnnaKilled
		|| latestGameEvent == kEventKronosHostageAnnaNoFirebird
		|| latestGameEvent == kEventKahinaPunchBaggageCarEntrance
		|| latestGameEvent == kEventKahinaPunchBlue
		|| latestGameEvent == kEventKahinaPunchYellow
		|| latestGameEvent == kEventKahinaPunchSalon
		|| latestGameEvent == kEventKahinaPunchKitchen
		|| latestGameEvent == kEventKahinaPunchBaggageCar
		|| latestGameEvent == kEventKahinaPunchCar
		|| latestGameEvent == kEventKahinaPunchSuite4
		|| latestGameEvent == kEventKahinaPunchRestaurant
		|| latestGameEvent == kEventKahinaPunch
		|| latestGameEvent == kEventKronosGiveFirebird
		|| latestGameEvent == kEventAugustFindCorpse
		|| latestGameEvent == kEventMertensBloodJacket
		|| latestGameEvent == kEventMertensCorpseFloor
		|| latestGameEvent == kEventMertensCorpseBed
		|| latestGameEvent == kEventCoudertBloodJacket
		|| latestGameEvent == kEventGendarmesArrestation
		|| latestGameEvent == kEventAbbotDrinkGiveDetonator
		|| latestGameEvent == kEventMilosCorpseFloor
		|| latestGameEvent == kEventLocomotiveAnnaStopsTrain
		|| latestGameEvent == kEventTrainStopped
		|| latestGameEvent == kEventCathVesnaRestaurantKilled
		|| latestGameEvent == kEventCathVesnaTrainTopKilled
		|| latestGameEvent == kEventLocomotiveConductorsDiscovered
		|| latestGameEvent == kEventViennaAugustUnloadGuns
		|| latestGameEvent == kEventViennaKronosFirebird
		|| latestGameEvent == kEventVergesAnnaDead
		|| latestGameEvent == kEventTrainExplosionBridge
		|| latestGameEvent == kEventKronosBringNothing );
}

bool VCR::makePermanent() {
	if (_engine->getSaveManager()->removeSavegame(_engine->_savegameNames[_engine->_currentGameFileColorId])) {
		error("Error deleting file \"%s\"", _engine->_savegameNames[_engine->_currentGameFileColorId]);
		return false;
	} else if (_engine->getSaveManager()->renameSavegame(_engine->_savegameFilename, _engine->_savegameNames[_engine->_currentGameFileColorId])) {
		error("Error renaming file \"%s\" to \"%s\"", _engine->_savegameFilename, _engine->_savegameNames[_engine->_currentGameFileColorId]);
		return false;
	} else {
		_engine->_savegameFilename = _engine->_savegameNames[_engine->_currentGameFileColorId];
		return true;
	}
}

int VCR::switchGames() {
	int index = 0;

	if (!isVirgin(_engine->_currentGameFileColorId)) {
		index = (_engine->_currentGameFileColorId + 1) % 6;
	}

	_engine->_currentGameFileColorId = index;
	_engine->_savegameFilename = _engine->_savegameNames[index];

	if (!_engine->getSaveManager()->fileExists(_engine->_savegameFilename))
		virginSaveFile();

	_engine->getLogicManager()->_gameTime = 0;
	_engine->getClock()->turnOnClock(false);

	if (_engine->_savePointHeaders) {
		_engine->getMemoryManager()->freeMem(_engine->_savePointHeaders);
		_engine->_savePointHeaders = nullptr;
	}

	_engine->_savegame->close();
	storeSettings();
	init(false, 0, 0);

	return _engine->_currentGameFileColorId;
}

void VCR::storeSettings() {
	SVCRFileHeader header;
	CVCRFile *file = new CVCRFile(_engine);

	file->open(_engine->_savegameFilename, CVCRMODE_RWB);

	if (file->read(&header, sizeof(SVCRFileHeader), 1, false, false) == 1) {
		if (!_engine->getSaveManager()->checkFileHeader(&header)) {
			file->close();
			delete file;
			return;
		}

		header.brightness = _engine->getGraphicsManager()->getGammaLevel();
		header.volume = _engine->getSoundManager()->getMasterVolume();

		file->seek(0, SEEK_SET);
		file->write(&header, sizeof(SVCRFileHeader), 1, false);
		file->close();
	} else {
		file->close();
		virginSaveFile();
	}

	delete file;
}

void VCR::loadSettings() {
	SVCRFileHeader header;
	CVCRFile *file = new CVCRFile(_engine);

	if (_engine->getSaveManager()->fileExists(_engine->_savegameFilename)) {
		file->open(_engine->_savegameFilename, CVCRMODE_RB);

		if (file->read(&header, sizeof(SVCRFileHeader), 1, false, false) == 1) {
			if (_engine->getSaveManager()->checkFileHeader(&header)) {
				file->seek(0, SEEK_SET);
				_engine->getGraphicsManager()->setGammaLevel(header.brightness);
				_engine->getSoundManager()->setMasterVolume(header.volume);
			}

			file->close();
		} else {
			file->close();
			virginSaveFile();
		}
	}

	delete file;

}

void VCR::rewind() {
	if (_engine->_currentSavePoint) {
		_currentSavePointInVCR = 0;
		_engine->getClock()->setClock(_engine->_savePointHeaders->time);
	}
}

void VCR::forward() {
	if (_engine->_lastSavePointIdInFile > _engine->_currentSavePoint) {
		_currentSavePointInVCR = _engine->_lastSavePointIdInFile;
		_engine->getClock()->setClock(_engine->_savePointHeaders[_engine->_lastSavePointIdInFile].time);
	}
}

void VCR::stop() {
	_currentSavePointInVCR = _engine->_currentSavePoint;
	_engine->getClock()->stopClock(_engine->_savePointHeaders[_engine->_currentSavePoint].time);
}

void VCR::seekToTime(int32 time) {
	int bestIdx = 0;
	int32 minDiff = ABS<int32>(_engine->_savePointHeaders[0].time - time);

	for (int i = 0; i <= _engine->_lastSavePointIdInFile; i++) {
		int32 curDiff = ABS<int32>(_engine->_savePointHeaders[i].time - time);
		if (curDiff < minDiff) {
			minDiff = curDiff;
			bestIdx = i;
		}
	}

	_currentSavePointInVCR = bestIdx;
	_engine->getClock()->setClock(_engine->_savePointHeaders[bestIdx].time);
}

void VCR::updateCurGame(int32 fromTime, int32 toTime, bool searchEntry) {
	int32 minTimeDiff = 0x7FFFFFFF;
	int newMenuIdx = 0;

	if (toTime != fromTime) {
		newMenuIdx = _engine->_currentSavePoint;

		if (toTime >= fromTime) {
			if (searchEntry) {
				for (int idx = _engine->_currentSavePoint; idx >= 0; --idx) {
					int32 gameTime = _engine->_savePointHeaders[idx].time;
					if (gameTime <= fromTime && minTimeDiff >= fromTime - gameTime) {
						minTimeDiff = fromTime - gameTime;
						newMenuIdx = idx;
					}
				}
			} else {
				newMenuIdx = _engine->_currentSavePoint - 1;
			}
		} else if (searchEntry) {
			for (int idx = _engine->_currentSavePoint; idx <= _engine->_lastSavePointIdInFile; ++idx) {
				int32 gameTime = _engine->_savePointHeaders[idx].time;
				if (gameTime >= fromTime && minTimeDiff > gameTime - fromTime) {
					minTimeDiff = gameTime - fromTime;
					newMenuIdx = idx;
				}
			}
		} else {
			newMenuIdx = _engine->_currentSavePoint + 1;
		}

		_engine->_currentSavePoint = newMenuIdx;
		_engine->getMenu()->updateEgg();
	}

	if (_engine->_currentSavePoint == _currentSavePointInVCR &&
		_engine->_savePointHeaders[newMenuIdx].partNumber != _engine->getLogicManager()->_globals[kGlobalChapter]) {
		_engine->getLogicManager()->_globals[kGlobalChapter] = _engine->_savePointHeaders[_engine->_currentSavePoint].partNumber;
	}
}

void VCR::go() {
	free();

	if (_engine->_savegameTempNames[_engine->_currentGameFileColorId] == _engine->_savegameFilename) {
		if (_engine->_lastSavePointIdInFile == _engine->_currentSavePoint) {
			_engine->getSaveManager()->continueGame();
			return;
		}
		_engine->getSaveManager()->startRewoundGame();
		return;
	}

	if (_engine->_lastSavePointIdInFile != _engine->_currentSavePoint) {
		_engine->getSaveManager()->startRewoundGame();
		return;
	}

	_engine->_gracePeriodTimer = 0;
	if (_engine->_currentSavePoint) {
		_engine->getSaveManager()->continueGame();
		return;
	}

	_engine->startNewGame();
}

} // End of namespace LastExpress
