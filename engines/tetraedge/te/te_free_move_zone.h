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

#ifndef TETRAEDGE_TE_TE_FREE_MOVE_ZONE_H
#define TETRAEDGE_TE_TE_FREE_MOVE_ZONE_H

#include "common/array.h"

#include "tetraedge/te/te_pick_mesh2.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_act_zone.h"
#include "tetraedge/te/te_timer.h"

namespace Tetraedge {

namespace micropather {
	class MicroPather;
}

// TODO: should these structs be moved to their own headers?
struct TeBlocker {
	Common::String _s;
	TeVector2f32 _pts[2];
	long _x;
};

struct TeRectBlocker {
	Common::String _s;
	TeVector2f32 _pts[4];
	long _x;
};


class TeFreeMoveZone : public TePickMesh2 {
public:
	struct CollidePoint {
		TeVector3f32 _point;
		float _distance;
	};

	class TeFreeMoveZoneGraph {
		friend class TeFreeMoveZone;
		TeVector2s32 _size;
		Common::Array<char> _flags;
		float _f;
		TeFreeMoveZone *_owner;
	};

	TeFreeMoveZone();

	float bordersDistance() const { return _graph->_f; }
	void buildAStar();
	void calcGridMatrix();
	void clear();
	Common::Array<TeVector3f32> collisions(const TeVector3f32 &v1, const TeVector3f32 &v2);
	TeVector3f32 correctCharacterPosition(const TeVector3f32 &pos, bool *flagout, bool f);

	static void deserialize(Common::ReadStream &stream, TeFreeMoveZone &dest, Common::Array<TeBlocker> *blockers,
               Common::Array<TeRectBlocker> *rectblockers, Common::Array<TeActZone> *actzones);

private:
	Common::Array<TeActZone> *_actzones;
	Common::Array<TeBlocker> *_blockers;
	Common::Array<TeRectBlocker> *_rectBlockers;

	TeFreeMoveZoneGraph *_graph;
	bool _transformedVerticiesDirty;
	bool _bordersDirty;
	bool _pickMeshDirty;
	bool _projectedPointsDirty;
	micropather::MicroPather *_micropather;
	TeTimer _updateTimer;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_FREE_MOVE_ZONE_H
