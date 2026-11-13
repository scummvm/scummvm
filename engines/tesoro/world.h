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

#ifndef TESORO_WORLD_H
#define TESORO_WORLD_H

#include "common/scummsys.h"
#include "tesoro/core/walkarea.h"

namespace Tesoro {

class Jim;
class Level;
class Object;
class TesoroEngine;

/**
 * The game's world as the objects see it: the table of levels (loaded when first needed), the main object list, the
 * spoken lines, Jim, the pointer and what it carries, the sound.
 */
class World {
public:
	enum { kLevelCount = 47 };

	explicit World(TesoroEngine &engine) : _engine(engine) {}
	~World();

	/** Reads a level file and builds its objects, hosted by this world. */
	Level *loadLevelFile(const char *name, int mode, int index = -1);
	/** The level at a place of the game's list, loaded on first use (null when it cannot be read). */
	Level *level(int index);
	/** The place of a level file's name in the game's list, -1 if it is not one of them. */
	int levelIndexOf(const char *name) const;

	/** The protagonist */
	Jim *jim();

	int32 now();
	void setHoverText(const char *text);

	/** The main list takes each object once: never insert the same object twice. */
	void insertInMainList(Object *object, int priority);
	void insertInMainListFront(Object *object, int priority);
	void removeFromMainList(Object *object);

	bool hasWalkArea();
	/** The size of things at a point (0x100 is life size; 0x100 too without a floor). */
	int32 scaleAt(int32 x, int32 y);
	bool findPath(int32 destX, int32 destY, WalkTarget *target);
	bool hasWalkAreaOf(Object *other);

	/** The level Jim is in, or null. */
	Level *jimLevel();

	void walkJimToPointer(int32 mouseX, int32 mouseY);

private:
	/** The objects the game starts with switched off, once a level has been loaded. */
	void applyStartState(Level &where, int index);

	TesoroEngine &_engine;
	Level *_levels[kLevelCount] = {};
	Jim *_jim = nullptr;
};

} // End of namespace Tesoro

#endif
