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
#include "pink/audio_info_mgr.h"

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
		kPlayingSequence = 2,
		kInventory = 3,
		kPDA = 4,
		kPlayingExitSequence = 6,
		kUndefined = 7
	};

	void deserialize(Archive &archive) override;

	void toConsole() const override;

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	void init(bool paused) override;

	void start(bool isHandler);

	void update();

	void loadPDA(const Common::String &pageName);

	void onKeyboardButtonClick(Common::KeyCode code);
	void onLeftButtonClick(Common::Point point);
	void onLeftButtonUp();
	virtual void onRightButtonClick(Common::Point point);

	void onMouseMove(Common::Point point);

	void onMouseOverWithItem(Common::Point point, const Common::String &itemName, Pink::CursorMgr *cursorMgr) override;
	void onMouseOver(Common::Point point, CursorMgr *mgr) override;

	void onLeftClickMessage() override;
	virtual void onVariableSet() {}
	void onInventoryClosed(bool isItemClicked);
	void onWalkEnd(const Common::String &stopName);
	void onPDAClose();

	bool isInteractingWith(const Actor *actor) const;

	void setNextExecutors(const Common::String &nextModule, const Common::String &nextPage);

	State getState() const { return _state; }

	AudioInfoMgr *getAudioInfoMgr() { return &_audioInfoMgr; }

	Actor *getActorByPoint(Common::Point point);

	Actor *findActor(const Common::String &name);

protected:
	void forceUpdateCursor();

	virtual void updateCursor(Common::Point point);

	virtual void sendUseClickMessage(Actor *actor);
	void sendLeftClickMessage(Actor *actor);

	virtual WalkLocation *getWalkDestination();

	void startInventory(bool paused);
	bool startWalk();

	void cancelInteraction();

	Actor *_recipient;

	CursorMgr *_cursorMgr;
	WalkMgr *_walkMgr;
	Sequencer *_sequencer;

	AudioInfoMgr _audioInfoMgr;

	State _state;
	State _nextState;
	State _stateBeforeInventory;
	State _stateBeforePDA;

	bool _isHaveItem;
};


class ParlSqPink : public LeadActor {
public:
	void toConsole() const override;

protected:
	WalkLocation *getWalkDestination() override;
};

class PubPink : public LeadActor {
public:
	void toConsole() const override;

	void onRightButtonClick(Common::Point point) override;

	void onLeftClickMessage() override;
	void onVariableSet() override;

protected:
	void updateCursor(Common::Point point) override;

	void sendUseClickMessage(Actor *actor) override;

	WalkLocation *getWalkDestination() override;

private:
	bool playingMiniGame();
};


} // End of namespace Pink

#endif
