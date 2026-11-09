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

/**
 * One scene file (.LVL): its animation banks, its rooms and the objects standing in them. A level is an object
 * itself, so it sits in the game's main object list and takes part in the frame passes.
 */
class WalkFloor;

class Level : public Object {
public:
	Level() {}
	~Level() override;

	/**
	 * Reads the level stored at an archive entry offset. mode is the animation load mode (AnimBank::kLoadNow or
	 * kLoadDeferred), now the game clock. Objects of kinds that are not ported yet are skipped with a warning.
	 */
	bool load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode, int32 now);

	uint roomCount() const { return _rooms.size(); }
	Room &room(uint i) { return _rooms[i]; }
	const Room &room(uint i) const { return _rooms[i]; }
	uint bankCount() const { return _levelPlayers.size(); }
	const Common::Array<AnimPlayer> &levelPlayers() const { return _levelPlayers; }

	/** Objects of the level in file order. */
	ObjectList &objects() { return _objects; }

	Camera &camera() { return _camera; }
	const Camera &camera() const { return _camera; }

	/** The level's single kind 1 object, if it has one. */
	Object *specialObject() const { return _special; }

	/** Game clock when the level was built. */
	int32 loadTime() const { return _loadTime; }

	/** Binds every object to this level (see attachAll). */
	void attachObjects();

	/** Object with this name key, or null. Does not disturb the object list's cursor. */
	Object *findObject(uint64 key);

protected:

	/** Builds the object of one level record, null for kinds not ported yet. */
	virtual Object *createObject(const ObjectRecord &record, Room &room);

private:
	Common::Array<AnimPlayer> _levelPlayers;
	Common::Array<Room> _rooms;
	ObjectList _objects;
	Common::Array<Object *> _owned;	// every object the level built, deleted with it
	Camera _camera;
	Object *_special = nullptr;

	int32 _loadTime = 0;
	bool _deferredReloaded = false;
};

} // End of namespace Tesoro

#endif
