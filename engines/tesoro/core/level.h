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

#ifndef TESORO_CORE_LEVEL_H
#define TESORO_CORE_LEVEL_H

#include "common/array.h"
#include "common/stream.h"

#include "tesoro/core/animbank.h"
#include "tesoro/core/camera.h"
#include "tesoro/core/object.h"
#include "tesoro/core/room.h"

namespace Tesoro {

class WalkFloor;

/**
 * One scene file (.LVL).
 * A level is an object itself, so it sits in the game's main object list and takes part in the frame passes.
 */
class Level : public Object {
public:
	Level() {}
	~Level() override;

	/**
	 * Reads the level stored at an archive entry offset.
	 */
	bool load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode, int32 now);

	uint roomCount() const { return _rooms.size(); }
	Room &room(uint i) { return _rooms[i]; }
	const Room &room(uint i) const { return _rooms[i]; }
	uint bankCount() const { return _levelPlayers.size(); }
	const Common::Array<AnimPlayer> &levelPlayers() const { return _levelPlayers; }

	ObjectList &objects() { return _objects; }
	ObjectList &movedObjects() { return _movedObjects; }

	Camera &camera() { return _camera; }
	const Camera &camera() const { return _camera; }

	WalkFloor *floor() const;

	/** The level's single kind 1 object, if it has one. */
	Object *specialObject() const { return _special; }

	/** Game clock when the level was built. */
	int32 loadTime() const { return _loadTime; }

	/** Records a collidable object that has moved, once. */
	void objectMoved(Object *object);

	/**
	 * The collision pass: updates bounds of moved objects, finds which hitter touches which object
	 * (nearest in time first), tells each hitter about it and forgets the moves.
	 */
	void collisionPass(int32 now);

	int32 frameTime() const { return _frameTime; }
	void setFrameTime(int32 now) { _frameTime = now; }

	/** First pass step: the collision pass, or the one-shot verb after a reload of the pictures. */
	int update() override;

	/** Binds every object to this level in list order. */
	void attachObjects();

	Object *findObject(uint64 key);

	/**
	 * Runs the camera towards each room's focus
	 * and updates the visible, active objects of that room.
	 * */
	void drawRooms(int32 now, Display &display);

protected:

	virtual Object *createObject(const ObjectRecord &record, Room &room);

private:
	Common::Array<AnimPlayer> _levelPlayers;
	Common::Array<Room> _rooms;
	ObjectList _objects;
	ObjectList _movedObjects;
	Common::Array<Object *> _owned;
	Camera _camera;
	Object *_special = nullptr;
	int32 _frameTime = 0;

	/** A touch found by the collision pass. */
	struct Touch {
		Object *hitter;
		Object *other;
		int32 time;		// fraction of the movement, 0x10000 being all of it
		int16 depth;
	};
	Common::Array<Touch> _touches;

	void resolveOverlap(Object *hitter, Object *other);
	/** The step the object is showing, null if it has no animation. */
	static const AnimStep *shapeOf(Object *object);
	/** The contact time of two objects that both have outlines; -1 for no touch. */
	static int32 shapedContactTime(Object *a, Object *b, const AnimStep *aShape, const AnimStep *bShape, int32 time, int32 dx, int32 dy);

	int32 _loadTime = 0;
	bool _deferredReloaded = false;
};

} // End of namespace Tesoro

#endif
