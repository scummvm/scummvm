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

#ifndef TESORO_KINDS_WANDERINGSPRITE_H
#define TESORO_KINDS_WANDERINGSPRITE_H

#include "tesoro/kinds/spriteobject.h"

namespace Tesoro {

class World;

/**
 * Object kind 16: an animated sprite that waits a random while, then travels along a path taken from the animation of
 * a named anchor object of its level.
 */
class WanderingSprite : public SpriteObject {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host);

	void updateAnimation(int32 now) override;
	void draw(const Camera &camera, Display &display) override;

protected:
	void onAttach() override;

private:
	const AnimStep *pathStep() const;
	void arrive(const AnimStep &path);

	World *_world = nullptr;
	char _anchorName[13] = {};
	Object *_anchor = nullptr;
	int8 _pathLow = 0, _pathHigh = 0;
	uint32 _minWait = 0, _maxWait = 0;
	int32 _stamp = 0;
	int32 _wait = 0;
	bool _travelling = false;
	uint32 _path = 0;
	int32 _point = 0;
	int32 _covered = 0;
	int32 _scale = 0;
	bool _warned = false;
};

} // End of namespace Tesoro

#endif
