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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WORLD_ACTORS_PATHFINDERPROCESS_H
#define WORLD_ACTORS_PATHFINDERPROCESS_H

#include "ultima/ultima8/kernel/process.h"

#include "ultima/ultima8/world/actors/pathfinder.h"

namespace Ultima {
namespace Ultima8 {

class Actor;

class PathfinderProcess : public Process {
public:
	PathfinderProcess();
	PathfinderProcess(Actor *actor, ObjId item, bool hit = false);
	PathfinderProcess(Actor *actor, int32 x, int32 y, int32 z);
	~PathfinderProcess() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;
	void terminate() override;

//	virtual void terminate();

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	int32 _targetX, _targetY, _targetZ;
	ObjId _targetItem;
	bool _hitMode;

	Std::vector<PathfindingAction> _path;
	unsigned int _currentStep;

public:
	static const uint16 PATHFINDER_PROC_TYPE;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
