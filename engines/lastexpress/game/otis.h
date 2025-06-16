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

#ifndef LASTEXPRESS_OTIS_H
#define LASTEXPRESS_OTIS_H

#include "lastexpress/lastexpress.h"

namespace LastExpress {

class LastExpressEngine;

struct Seq;
struct Node;

class OtisManager {

public:
	OtisManager(LastExpressEngine *engine);
	~OtisManager() {}

	void wipeLooseSprites();
	void wipeGSysInfo(int character);
	void wipeAllGSysInfo();
	bool fDirection(int nodeIdx);
	bool rDirection(int nodeIdx);
	bool doorView();
	bool corrRender(int nodeIdx);
	bool walkingRender();
	int checkMouse(int32 cursorX, int32 cursorY);
	void startSeq(int character, int direction, bool loadSequence);
	void getNewSeqName(int character, int direction, char *outSeqName, char *outSecondarySeqName);
	void drawLooseSprites();
	void refreshSequences();
	int findFrame(int character, Seq *sequence, int position, bool doProcessing);
	void initCurFrame(int character);
	bool mainWalkTooFar(int character);
	int getFudge();
	void updateCharacter(int character);
	void doNewSprite(int character, bool keepPreviousFrame, bool dontPlaySound);
	void doSeqChange(int character);
	void doNextSeq(int character);
	void doNoSeq(int character);
	void updateAll();
	void goUpdateAll();
	void adjustOtisTrueTime();

private:
	LastExpressEngine *_engine = nullptr;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_OTIS_H
