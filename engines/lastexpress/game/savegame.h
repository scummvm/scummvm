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

#ifndef LASTEXPRESS_SAVELOAD_H
#define LASTEXPRESS_SAVELOAD_H

#include "lastexpress/lastexpress.h"

namespace LastExpress {

class LastExpressEngine;
class CVCRFile;
struct SVCRFileHeader;
struct SVCRSavePointHeader;

class SaveManager {
public:
	SaveManager(LastExpressEngine *engine);
	~SaveManager() {}

	void writeSavePoint(CVCRFile *file, int saveType, int entityIndex, int value);
	void readSavePoint(CVCRFile *savegameData, int *saveType, uint8 *entity, int *saveEvent, bool skipSoundLoading);
	void validateSaveFile(bool flag);
	bool checkFileHeader(SVCRFileHeader *fileHeader);
	bool checkSavePointHeader(SVCRSavePointHeader *savePointHeader);
	void continueGame();
	void startRewoundGame();
	bool fileExists(const char *filename);
	bool removeSavegame(const char *filename);
	bool renameSavegame(const char *oldName, const char *newName);

private:
	LastExpressEngine *_engine = nullptr;

};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SAVELOAD_H
