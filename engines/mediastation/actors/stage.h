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

#ifndef MEDIASTATION_STAGE_H
#define MEDIASTATION_STAGE_H

#include "common/events.h"

#include "mediastation/actor.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

class StageActor : public SpatialEntity {
public:
	StageActor();
	virtual ~StageActor();

	virtual void draw(const Common::Array<Common::Rect> &dirtyRegion) override;
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual bool isVisible() const override { return _children.size() > 0; }

	void addChildSpatialEntity(SpatialEntity *entity);
	void removeChildSpatialEntity(SpatialEntity *entity);

	virtual uint16 findActorToAcceptMouseEvents(
		const Common::Point &point,
		uint16 eventMask,
		MouseActorState &state,
		bool inBounds) override;
	virtual uint16 findActorToAcceptKeyboardEvents(
		uint16 asciiCode,
		uint16 eventMask,
		MouseActorState &state) override;
	virtual void currentMousePosition(Common::Point &point);
	virtual void setMousePosition(int16 x, int16 y) override;

	void invalidateZIndexOf(const SpatialEntity *entity);

protected:
	bool _cylindricalX = false;
	bool _cylindricalY = false;
	SpatialEntity *_pendingChild = nullptr;

	void addActorToStage(uint actorId);
	void removeActorFromStage(uint actorId);

	bool assertHasNoParent(const SpatialEntity *entity);
	bool assertHasParentThatIsNotMe(const SpatialEntity *entity) { return !assertIsMyChild(entity); }
	bool assertIsMyChild(const SpatialEntity *entity) { return this == entity->getParentStage();}
	void removeAllChildren();

	virtual void invalidateLocalBounds() override;
	virtual void invalidateLocalZIndex() override;
	virtual void invalidateRect(const Common::Rect &rect);
	virtual void loadIsComplete() override;

	static int compareSpatialActorByZIndex(const SpatialEntity *a, const SpatialEntity *b);

	Common::SortedArray<SpatialEntity *, const SpatialEntity *> _children;
};

class RootStage : public StageActor {
public:
	friend class StageDirector;
	RootStage() : StageActor() { _id = ROOT_STAGE_ACTOR_ID; };

	const Common::Array<Common::Rect> &getDirtyRegion() { return _dirtyRegion; }
	void clearDirtyRegion() { _dirtyRegion.clear(); }

	virtual uint16 findActorToAcceptMouseEvents(
		const Common::Point &point,
		uint16 eventMask,
		MouseActorState &state,
		bool inBounds) override;
	virtual void currentMousePosition(Common::Point &point) override;
	virtual void setMousePosition(int16 x, int16 y) override;
	virtual void invalidateRect(const Common::Rect &rect) override;
	virtual void deleteChildrenFromContextId(uint contextId);

	virtual void mouseEnteredEvent(const Common::Event &event) override;
	virtual void mouseExitedEvent(const Common::Event &event) override;
	virtual void mouseOutOfFocusEvent(const Common::Event &event) override;

	void drawAll();
	void drawDirtyRegion();
	void invalidateAll();

private:
	static const uint ROOT_STAGE_ACTOR_ID = 2;
	Common::Array<Common::Rect> _dirtyRegion;
	bool _isMouseInside = false;
};

class StageDirector {
public:
	StageDirector();
	~StageDirector();

	RootStage *getRootStage() const { return _rootStage; }

	void drawAll();
	void drawDirtyRegion();
	void clearDirtyRegion();

	void handleKeyboardEvent(const Common::Event &event);
	void handleMouseDownEvent(const Common::Event &event);
	void handleMouseUpEvent(const Common::Event &event);
	void handleMouseMovedEvent(const Common::Event &event);
	void handleMouseEnterExitEvent(const Common::Event &event);
	void handleMouseOutOfFocusEvent(const Common::Event &event);
	void sendMouseEnterExitEvent(uint16 flags, MouseActorState &state, const Common::Event &event);

private:
	RootStage *_rootStage = nullptr;
};

} // End of namespace MediaStation

#endif
