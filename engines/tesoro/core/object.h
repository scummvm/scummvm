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

#ifndef TESORO_CORE_OBJECT_H
#define TESORO_CORE_OBJECT_H

#include "common/array.h"

#include "tesoro/core/animset.h"
#include "tesoro/core/camera.h"
#include "tesoro/core/objectlist.h"

namespace Tesoro {

class Display;
class Room;

struct ObjectRect {
	int32 x1 = 0, y1 = 0, x2 = 0, y2 = 0;
};

/** Structure describing an object as it appears in the level file. */
struct ObjectRecord {
	int32 x = 0, y = 0;
	int16 sortOffset = 0;
	char name[13] = {};
	Common::Array<byte> banks;	// which of the level's animation banks the object uses
	uint32 kind = 0;			// which class the level factory builds
	Common::Array<byte> extra;	// data the kind reads when it is built

	/** Byte i of the kind's data. The bank indices follow the data in the file, so short data reads on into them. */
	byte dataByte(uint i) const {
		if (i < extra.size())
			return extra[i];
		i -= extra.size();
		return i < banks.size() ? banks[i] : 0;
	}
};

/** Every thing in the level is an object, including a Level. */
class Object {
public:
	enum Flags {
		kProcessed = 0x1,
		kActive = 0x2,
		kVisible = 0x4,
		kCollidable = 0x8,
		kHasAnimation = 0x10,
		kInRoomList = 0x100,
		kPassDone = 0x200,
		kHitter = 0x400
	};

	virtual ~Object() {}

	/** Plain object: active and visible, drawn at sort order 5. */
	void initPlain();

	/**
	 * Object with animations, built from its level record.
	 * roomSortOffset: draw order offset added to the object's own.
	 * levelPlayers: the level's animation players; the record picks which ones the object copies.
	 * stateSize: bytes of saved state the object keeps (0 for none).
	 */
	void initAnimated(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, uint16 stateSize);

	virtual void setActive(bool active) { _flags = (_flags & ~kActive) | (active ? kActive : 0); }
	void setVisible(bool visible) { _flags = (_flags & ~kVisible) | (visible ? kVisible : 0); }

	/** Advances the animation */
	virtual void updateAnimation(int32 now);

	/** Ticks the animation and works out the hit rectangle and the area the object moved through. */
	virtual void updateBounds(int32 now);

	/** Called after updateBounds(), draws the object as seen by the camera. */
	virtual void draw(const Camera &camera, Display &display) {}

	uint32 flags() const { return _flags; }
	void setFlags(uint32 flags) { _flags = flags; }
	int32 x() const { return _x; }
	int32 y() const { return _y; }
	int32 previousX() const { return _previousX; }
	int32 previousY() const { return _previousY; }
	int16 sortValue() const { return _sortValue; }
	void setSortValue(int16 sort) { _sortValue = sort; }
	uint64 nameKey() const { return _nameKey; }

	const ObjectRect &hitRect() const { return _hitRect; }
	const ObjectRect &movedArea() const { return _movedArea; }

	/** The animation player the object is shown by, or null if it has none. */
	AnimPlayer *currentPlayer() { return _anim.bankCount() ? &_anim.current() : nullptr; }

	AnimSet &animation() { return _anim; }
	const AnimSet &animation() const { return _anim; }

	Room *room() const { return _room; }
	void setRoom(Room *room) { _room = room; }

	void setPosition(int32 x, int32 y) { _x = x; _y = y; }

	/**
	 * Moves the object to (x, y): the position it is heading to always changes, the current one only with setCurrent.
	 * A collidable object is entered in its level's list of moved objects.
	 */
	void moveTo(int32 x, int32 y, bool setCurrent);

protected:
	/**
	 * Draws the image of the step the current player is on, positioned from the camera.
	 * With skipPastRight an image whose left edge would be further right than the screen
	 * is not sent at all.
	 */
	void drawAnimation(const Camera &camera, Display &display, bool skipPastRight);

	Room *_room = nullptr;
	uint32 _flags = 0;
	int32 _x = 0, _y = 0;
	int32 _previousX = 0, _previousY = 0;
	int16 _sortValue = 0;
	uint64 _nameKey = 0;
	byte _kind = 0;
	uint16 _stateSize = 0;
	bool _hasStateBlob = false;
	Common::Array<byte> _stateBlob;
	ObjectRect _hitRect;
	ObjectRect _movedArea;
	AnimSet _anim;
};

} // End of namespace Tesoro

#endif
