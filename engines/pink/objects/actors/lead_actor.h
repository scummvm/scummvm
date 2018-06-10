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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PINK_LEAD_ACTOR_H
#define PINK_LEAD_ACTOR_H

#include "common/rect.h"
#include "common/keyboard.h"

#include "pink/objects/actors/actor.h"

namespace Pink {

class CursorMgr;
class WalkMgr;
class WalkLocation;
class Sequencer;

class SupportingActor;
class InventoryItem;

class LeadActor : public Actor {
public:
	LeadActor();

	enum State {
		kReady = 0,
		kMoving = 1,
		kInDialog1 = 2, //???
		kInventory = 3,
		kPDA = 4,
		kInDialog2 = 5,//???
		kPlayingVideo = 6, // ???
		kUnk_Loading = 7// ????
	};

	void deserialize(Archive &archive) override;

	void toConsole() override;

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	virtual void init(bool unk);

	void start(bool isHandler);

	void update();

	void loadPDA(const Common::String &pageName);

	void onKeyboardButtonClick(Common::KeyCode code);
	void onLeftButtonClick(const Common::Point point);

	void onMouseMove(const Common::Point point);

	void onMouseOver(const Common::Point point, CursorMgr *mgr) override;

	virtual void onClick();
	virtual void onVariableSet() {}
	void onInventoryClosed(bool isItemClicked);
	void onWalkEnd();
	void onPDAClose();

	bool isInteractingWith(Actor *actor);

	void setNextExecutors (const Common::String &nextModule, const Common::String &nextPage);

	State getState() const { return _state; }

protected:
	void forceUpdateCursor();

	virtual void updateCursor(const Common::Point point);

	virtual bool sendUseClickMessage(Actor *actor);
	bool sendLeftClickMessage(Actor *actor);

	virtual WalkLocation *getWalkDestination();

	Actor *_recipient;

	CursorMgr *_cursorMgr;
	WalkMgr *_walkMgr;
	Sequencer *_sequencer;

	State _state;
	State _nextState;
	State _stateCopy;
	State _stateBeforePDA;

	bool _isHaveItem;
};


class ParlSqPink : public LeadActor {
public:
	void toConsole() override;

protected:
	WalkLocation *getWalkDestination() override;
};

class PubPink : public LeadActor {
public:
	PubPink();

	void toConsole() override;

	void onClick() override;
	void onVariableSet() override;

protected:
	void updateCursor(Common::Point point) override;

	bool sendUseClickMessage(Actor *actor) override;

	WalkLocation *getWalkDestination() override;

private:
	bool playingMiniGame();

	int _round;
};


} // End of namespace Pink

#endif
