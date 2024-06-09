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

#ifndef WORLD_ACTORS_TARGETEDANIMPROCESS_H
#define WORLD_ACTORS_TARGETEDANIMPROCESS_H

#include "ultima/ultima8/world/actors/actor_anim_process.h"
#include "ultima/ultima8/world/actors/animation.h"
#include "ultima/ultima8/misc/classtype.h"
#include "ultima/ultima8/misc/point3.h"

namespace Ultima {
namespace Ultima8 {

class TargetedAnimProcess : public ActorAnimProcess {
public:
	TargetedAnimProcess();
	//! note: this probably needs some more parameters
	TargetedAnimProcess(Actor *actor, Animation::Sequence action, Direction dir,
	                    const Point3 &pt);

	ENABLE_RUNTIME_CLASSTYPE()

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	bool init() override;

	Point3 _pt;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
