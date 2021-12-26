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

#ifndef WORLD_ACTORS_ROLLINGTHUNDERPROCESS_H
#define WORLD_ACTORS_ROLLINGTHUNDERPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/world/actors/animation.h"

namespace Ultima {
namespace Ultima8 {

class Actor;

/**
 * A process to roll out, shoot at the player, and roll back.
 * Only used in No Regret.
 */
class RollingThunderProcess : public Process {
public:
	RollingThunderProcess();
	RollingThunderProcess(Actor *actor);

	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

private:
	void sleepFor60Ticks();

	bool checkForSpiderBomb();

	bool checkTimer();

	bool fireDistance(Direction dir, int32 x, int32 y, int32 z) const;

	bool checkDir(Animation::Sequence anim, Direction &outdir) const;

	uint16 _target;
	uint32 _timer;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
