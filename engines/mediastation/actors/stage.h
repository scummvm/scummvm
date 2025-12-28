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
#include "mediastation/graphics.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

// Cylindrical wrapping allows content on a stage to wrap around like a cylinder - for example, when you scroll past the
// right edge, content from the left edge appears, creating the illusion of an infinite looping world.
enum CylindricalWrapMode : int {
	kWrapNone = 0,        // No offset (default)
	kWrapRight = 1,       // Right wrap (X + extent.x)
	kWrapBottom = 2,      // Bottom wrap (Y + extent.y)
	kWrapLeftTop = 3,     // Left + Top wrap (X - extent.x, Y - extent.y)
	kWrapLeft = 4,        // Left wrap (X - extent.x)
	kWrapRightBottom = 5, // Right + Bottom wrap (X + extent.x, Y + extent.y)
	kWrapTop = 6,         // Top wrap (Y - extent.y)
	kWrapLeftBottom = 7,  // Left + Bottom wrap (X - extent.x, Y + extent.y)
	kWrapRightTop = 8     // Right + Top wrap (X + extent.x, Y - extent.y)
};

class CameraActor;
class StageActor : public SpatialEntity {
public:
	StageActor();
	virtual ~StageActor();

	virtual void draw(DisplayContext &displayContext) override;
	void drawUsingStage(DisplayContext &displayContext);

	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual void preload(const Common::Rect &rect) override;
	virtual bool isVisible() const override { return _children.size() > 0; }
	virtual bool isRectInMemory(const Common::Rect &rect) override;

	void addChildSpatialEntity(SpatialEntity *entity);
	void removeChildSpatialEntity(SpatialEntity *entity);
	void addCamera(CameraActor *camera);
	void removeCamera(CameraActor *camera);
	void setCurrentCamera(CameraActor *camera);
	CameraActor *getCurrentCamera() const { return _currentCamera; }

	uint16 queryChildrenAboutMouseEvents(
		const Common::Point &point,
		uint16 eventMask,
		MouseActorState &state,
		CylindricalWrapMode wrapMode = kWrapNone);
	uint16 findActorToAcceptMouseEventsObject(
		const Common::Point &point,
		uint16 eventMask,
		MouseActorState &state,
		bool inBounds);
	uint16 findActorToAcceptMouseEventsCamera(
		const Common::Point &point,
		uint16 eventMask,
		MouseActorState &state,
		bool inBounds);
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
	virtual void invalidateLocalBounds() override;
	virtual void invalidateRect(const Common::Rect &rect);

	bool cylindricalX() const { return _cylindricalX; }
	bool cylindricalY() const { return _cylindricalY; }
	Common::Point extent() const { return _extent; }

protected:
	// Whether or not cameras on this stage should wrap around this stage.
	bool _cylindricalX = false;
	bool _cylindricalY = false;
	Common::Point _extent;
	SpatialEntity *_pendingParent = nullptr;
	CameraActor *_currentCamera = nullptr;

	void addActorToStage(uint actorId);
	void removeActorFromStage(uint actorId);
	bool isRectInMemoryTest(const Common::Rect &rect, CylindricalWrapMode wrapMode);
	void preloadTest(const Common::Rect &rect, CylindricalWrapMode wrapMode);

	bool assertHasNoParent(const SpatialEntity *entity);
	bool assertHasParentThatIsNotMe(const SpatialEntity *entity) { return !assertIsMyChild(entity); }
	bool assertIsMyChild(const SpatialEntity *entity) { return this == entity->getParentStage();}
	void removeAllChildren();

	virtual void invalidateLocalZIndex() override;
	void invalidateUsingCameras(const Common::Rect &rect);
	void invalidateObject(const Common::Rect &rect, const Common::Rect &visibleRegion);
	// The function is called this in the original; not sure why though.
	void invalidateTest(const Common::Rect &rect, const Common::Rect &clip, const Common::Point &originAdjustment);

	virtual void loadIsComplete() override;

	static int compareSpatialActorByZIndex(const SpatialEntity *a, const SpatialEntity *b);

	Common::SortedArray<SpatialEntity *, const SpatialEntity *> _children;
	Common::SortedArray<CameraActor *, const SpatialEntity *> _cameras;
};

class RootStage : public StageActor {
public:
	friend class StageDirector;
	RootStage() : StageActor() { _id = ROOT_STAGE_ACTOR_ID; };

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

	void drawAll(DisplayContext &displayContext);
	void drawDirtyRegion(DisplayContext &displayContext);

	Region _dirtyRegion;

private:
	static const uint ROOT_STAGE_ACTOR_ID = 2;
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
