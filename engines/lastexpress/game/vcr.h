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

#ifndef LASTEXPRESS_VCR_H
#define LASTEXPRESS_VCR_H

#include "lastexpress/lastexpress.h"
#include "lastexpress/data/cvcrfile.h"

namespace LastExpress {

class LastExpressEngine;
class CVCRFile;

struct SVCRFileHeader;
struct SVCRSavePointHeader;

class VCR {

public:
	VCR(LastExpressEngine *engine);
	~VCR() {}

	void virginSaveFile();
	void writeSavePoint(int type, int entity, int event);
	void selectFromName(const char *filename);
	void shuffleGames();
	void setCurrentGameColor(int index);
	void init(bool doSaveGameFlag, int saveType, int32 time);
	void autoRewind(int saveType, int32 time);
	void free();
	bool isVirgin(int savegameIndex);
	bool currentEndsGame();
	bool makePermanent();
	int switchGames();
	void storeSettings();
	void loadSettings();
	void rewind();
	void forward();
	void stop();
	void seekToTime(int32 time);
	void updateCurGame(int32 fromTime, int32 toTime, bool searchEntry);
	void go();

protected:
	LastExpressEngine *_engine;

	int32 _currentSavePointInVCR = 0;

};

} // End of namespace LastExpress

#endif // LASTEXPRESS_VCR_H
