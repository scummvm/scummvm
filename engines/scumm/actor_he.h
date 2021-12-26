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

struct AuxBlock {
	bool visible;
	Common::Rect r;

	void reset() {
		visible = false;
		r.left = r.top = 0;
		r.right = r.bottom = -1;
	}

	void clear() {
		reset();
		r.right = 0;
		r.bottom = 0;
	}
};

struct AuxEntry {
	int actorNum;
	int subIndex;
};

class ActorHE : public Actor {
public:
	ActorHE(ScummEngine *scumm, int id) : Actor(scumm, id) {}

	void initActor(int mode) override;

	void hideActor() override;

	void drawActorToBackBuf(int x, int y);

	void setHEFlag(int bit, int set);

	void setUserCondition(int slot, int set);
	bool isUserConditionSet(int slot) const;

	void setTalkCondition(int slot);
	bool isTalkConditionSet(int slot) const;

public:
	/** This rect is used to clip actor drawing. */
	Common::Rect _clipOverride;

	bool _heNoTalkAnimation;
	bool _heTalking;
	byte _heFlags;

	AuxBlock _auxBlock;

	struct {
		int16 posX;
		int16 posY;
		int16 color;
		byte sentence[128];
	} _heTalkQueue[16];


	void prepareDrawActorCostume(BaseCostumeRenderer *bcr) override;
	void setActorCostume(int c) override;
};

} // End of namespace Scumm

#endif
