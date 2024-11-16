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


#ifndef SCUMM_ACTOR_HE_H
#define SCUMM_ACTOR_HE_H

#include "scumm/actor.h"

namespace Scumm {

#define ACTOR_GENERAL_FLAG_IGNORE_ERASE 0x00000001

#define HE100_CHORE_REDIRECT_INIT        1024
#define HE100_CHORE_REDIRECT_WALK        1025
#define HE100_CHORE_REDIRECT_STAND       1026
#define HE100_CHORE_REDIRECT_START_TALK  1027
#define HE100_CHORE_REDIRECT_STOP_TALK   1028

 struct HEEraseAuxEntry {
	int actor;
	int32 x1, y1, x2, y2;
};

struct HEAnimAuxEntry {
	int actor;
	int auxIndex;
};

struct HEAnimAuxData {
	HEAnimAuxData() : auxDefaultSearchBlock(nullptr), externalDataPtr(nullptr), auxDataBlock(nullptr), actor(0) {} 
	byte *auxDefaultSearchBlock;
	byte *externalDataPtr;
	const byte *auxDataBlock;
	int actor;
};

struct HEAuxFileRelInfo {
	int globFileOffset;
	Common::File fileHandle;
	int globType;
	int globNum;
	int roomNum;
};

class ActorHE : public Actor {
public:
	ActorHE(ScummEngine *scumm, int id);

	void initActor(int mode) override;

	void hideActor() override;

	void drawActorToBackBuf(int x, int y);

	void setActorEraseType(int eraseValue);

	void setCondition(int slot, int set);
	bool isConditionSet(int slot) const;

	void setUserCondition(int slot, int set);
	bool isUserConditionSet(int slot) const;

	void setTalkCondition(int slot);
	bool isTalkConditionSet(int slot) const;

	void clearActorUpdateInfo();
	void setActorUpdateArea(int x1, int y1, int x2, int y2);

public:
	/** This rect is used to clip actor drawing. */
	Common::Rect _clipOverride;

	bool _heNoTalkAnimation = false;
	bool _heTalking = false;
	byte _generalFlags = 0;

	int _auxActor = 0;
	int32 _auxEraseX1 = 0, _auxEraseY1 = 0, _auxEraseX2 = 0, _auxEraseY2 = 0;

	// 80 is the maximum number of strips that any hi-res HE game is going to have
	int _screenUpdateTableMin[80] = {};
	int _screenUpdateTableMax[80] = {};

	struct {
		int16 posX;
		int16 posY;
		int16 color;
		byte sentence[128];
	} _heTalkQueue[16] = {};


	void prepareDrawActorCostume(BaseCostumeRenderer *bcr) override;
	void setActorCostume(int c) override;
};

} // End of namespace Scumm

#endif
