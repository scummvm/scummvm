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

#ifndef ILLUSIONS_DUCKMAN_SPECIALCODE_H
#define ILLUSIONS_DUCKMAN_SPECIALCODE_H

#include "illusions/illusions.h"
#include "illusions/specialcode.h"
#include "common/algorithm.h"

namespace Illusions {

class IllusionsEngine_Duckman;
class DuckmanCredits;
class DuckmanInventory;
class PropertyTimers;

typedef Common::Functor1<OpCall&, void> SpecialCodeFunction;

class DuckmanSpecialCode : public SpecialCode {
public:
	DuckmanSpecialCode(IllusionsEngine_Duckman *vm);
	~DuckmanSpecialCode() override;
	void init() override;
	void run(uint32 specialCodeId, OpCall &opCall) override;
public:
	typedef Common::HashMap<uint32, SpecialCodeFunction*> SpecialCodeMap;
	typedef SpecialCodeMap::iterator SpecialCodeMapIterator;

	IllusionsEngine_Duckman *_vm;
	SpecialCodeMap _specialCodeMap;

	PropertyTimers *_propertyTimers;
	DuckmanInventory *_inventory;

	uint _chinesePuzzleIndex;
	byte _chinesePuzzleAnswers[3];
	Common::Point _teleporterPosition;
	int16 _counter;
	bool _wasCursorHoldingElvisPoster;
	int16 _savedTempMasterSfxVolume;
	int16 _lastRandomSoundIndex;

	DuckmanCredits *_credits;

	// Special code interface functions
	void runSpecialCode(uint32 specialCodeId, OpCall &opCall);
	void spcStartScreenShaker(OpCall &opCall);
	void spcSetCursorHandMode(OpCall &opCall);
	void spcResetChinesePuzzle(OpCall &opCall);
	void spcAddChinesePuzzleAnswer(OpCall &opCall);
	void spcOpenInventory(OpCall &opCall);
	void spcPutBackInventoryItem(OpCall &opCall);
	void spcClearInventorySlot(OpCall &opCall);
	void spcAddPropertyTimer(OpCall &opCall);
	void spcSetPropertyTimer(OpCall &opCall);
	void spcRemovePropertyTimer(OpCall &opCall);
	void spcInitTeleporterPosition(OpCall &opCall);
	void spcUpdateTeleporterPosition(OpCall &opCall);
	void spcCenterNewspaper(OpCall &opCall);
	void spcStopScreenShaker(OpCall &opCall);
	void spcIncrCounter(OpCall &opCall);
	void spcUpdateObject272Sequence(OpCall &opCall);
	void spcPlayRandomSound(OpCall &opCall);
	void spcHoldGlowingElvisPoster(OpCall &opCall);
	void spcStartCredits(OpCall &opCall);
	void spcSetCursorInventoryMode(OpCall &opCall);
	void spcCenterCurrentScreenText(OpCall &opCall);
	void spcSetDefaultTextCoords(OpCall &opCall);
	void spcSetTextDuration(OpCall &opCall);
	void spcSetTempMasterSfxVolume(OpCall &opCall);
	void spcRestoreTempMasterSfxVolume(OpCall &opCall);

	void updateTeleporterProperties();

	void startCredits();
	int updateCredits(uint flags);
	char *readNextCreditsLine();
	Common::Point getCreditsItemPosition(int index);
	void charToWChar(char *text, uint16 *wtext, uint size);

};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
