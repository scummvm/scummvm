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
 * $URL$
 * $Id$
 *
 */

#ifndef DRACI_WALKING_H
#define DRACI_WALKING_H

#include "common/array.h"
#include "common/rect.h"

namespace Draci {

class Sprite;

typedef Common::Array<Common::Point> WalkingPath;

class WalkingMap {
public:
	WalkingMap() : _realWidth(0), _realHeight(0), _deltaX(1), _deltaY(1),
		_mapWidth(0), _mapHeight(0), _byteWidth(0), _data(NULL) { }

	void load(const byte *data, uint length);

	bool getPixel(int x, int y) const;
	bool isWalkable(int x, int y) const;

	Sprite *newOverlayFromMap(byte colour) const;
	Common::Point findNearestWalkable(int x, int y, Common::Rect searchRect) const;

	bool findShortestPath(Common::Point p1, Common::Point p2, WalkingPath *path) const;
	void obliquePath(const WalkingPath& path, WalkingPath *obliquedPath);
	Sprite *newOverlayFromPath(const WalkingPath &path, byte colour) const;
	Common::Point getDelta() const { return Common::Point(_deltaX, _deltaY); }

private:
	int _realWidth, _realHeight;
	int _deltaX, _deltaY;
	int _mapWidth, _mapHeight;
	int _byteWidth;

	// We don't own the pointer.  It points to the BArchive cache for this room.
	const byte *_data;

	// 4 possible directions to walk from a pixel.
	static int kDirections[][2];

	void drawOverlayRectangle(const Common::Point &p, byte colour, byte *buf) const;
	int pointsBetween(const Common::Point &p1, const Common::Point &p2) const;
	Common::Point interpolate(const Common::Point &p1, const Common::Point &p2, int i, int n) const;
	bool lineIsCovered(const Common::Point &p1, const Common::Point &p2) const;

	// Returns true if the number of vertices on the path was decreased.
	bool managedToOblique(WalkingPath *path) const;
};

/*
 * Enumerates the directions the dragon can look into when arrived.
 */
enum SightDirection {
	kDirectionLast, kDirectionMouse, kDirectionUnknown,
	kDirectionRight, kDirectionLeft, kDirectionIntelligent
};

/**
  * Enumerates the animations for the dragon's movement.
  */
enum Movement {
	kMoveUndefined = -1,
	kMoveDown, kMoveUp, kMoveRight, kMoveLeft,
	kMoveRightDown, kMoveRightUp, kMoveLeftDown, kMoveLeftUp,
	kMoveDownRight, kMoveUpRight, kMoveDownLeft, kMoveUpLeft,
	kMoveLeftRight, kMoveRightLeft, kMoveUpStopLeft, kMoveUpStopRight,
	kSpeakRight, kSpeakLeft, kStopRight, kStopLeft
};

class DraciEngine;
struct GPL2Program;

class WalkingState {
public:
	explicit WalkingState(DraciEngine *vm) : _vm(vm) { stopWalking(); }
	~WalkingState() {}

	void stopWalking();
	void startWalking(const Common::Point &p1, const Common::Point &p2,
		const Common::Point &mouse, SightDirection dir,
		const Common::Point &delta, const WalkingPath& path);
	const WalkingPath& getPath() const { return _path; }

	void setCallback(const GPL2Program *program, uint16 offset);
	void callback();

	bool isActive() const { return _path.size() > 0; }

	// Advances the hero along the path and changes animation accordingly.
	// Walking MUST be active when calling this method.  When the hero has
	// arrived to the target, clears the path and returns false, but leaves
	// the callback untouched (the caller must call it).
	bool continueWalking();

private:
	DraciEngine *_vm;

	WalkingPath _path;
	Common::Point _mouse;
	SightDirection _dir;

	const GPL2Program *_callback;
	uint16 _callbackOffset;

	// Return one of the 4 animations kMove{Down,Up,Right,Left}
	// corresponding to the walking from here to there.
	static Movement animationForDirection(const Common::Point &here, const Common::Point &there);

	// Returns either animation that needs to be played between given two
	// animations (e.g., kMoveRightDown after kMoveRight and before
	// kMoveDown), or kMoveUndefined if none animation is to be played.
	static Movement transitionBetweenAnimations(Movement previous, Movement next);
};

} // End of namespace Draci

#endif // DRACI_WALKING_H
