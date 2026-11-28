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

#ifndef TESORO_KINDS_INTERACTIVE_H
#define TESORO_KINDS_INTERACTIVE_H

#include "tesoro/core/object.h"
#include "tesoro/core/speechtable.h"
#include "tesoro/core/walkarea.h"

namespace Tesoro {

class World;

/**
 * A scene object the player can act on. Object kind 9.
 */
class InteractiveObject : public Object, public WalkTarget {
public:
	enum { kVerbs = 7 };

	/** Built from its level record. */
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world, uint16 stateSize = 0, uint dataStart = 0);

	int update() override;
	int setVerbAndClick(int verb) override;
	int doubleClick(int verb) override { return setVerbAndClick(verb); }
	bool acceptsPointer() override { return true; }
	int showHoverText() override;
	int clearHoverText() override;

	/** Jim reaches this object, faces it and says whatever line is appropriate. */
	int arrived() override;

	/** The click itself: Jim walks to the hotspot's approach point. */
	virtual void onClick();

	/** Returns 3 when it started a line. */
	int verb(int verb) override;
	/** An inventory item (its number) used on the object. */
	virtual int useItem(int item);

	void updateAnimation(int32 now) override { _anim.tickClearMovement(now); }
	void draw(const Camera &camera, Display &display) override { drawAnimation(camera, display, true); }

	int32 approachX() const override { return _approachX; }
	int32 approachY() const override { return _approachY; }
	int facing() const override { return _facing; }
	bool pending() const { return _pending; }

protected:
	int answer(const SpeechTable::Line *line);
	const SpeechTable::Line *defaultLine(const char *name);
	/** Runs the verb the pointer carries. */
	int runVerb();

	World *_world = nullptr;
	bool _pending = false;
	bool _answered = false;
	int _facing = 0;
	int32 _approachX = 0, _approachY = 0;
	int _verb = 0;
	const SpeechTable::Line *_hoverName = nullptr;
	const SpeechTable::Line *_verbLine[kVerbs] = {};
};

} // End of namespace Tesoro

#endif
