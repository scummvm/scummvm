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

#include "tetraedge/te/te_bezier_curve.h"
#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_obp.h"
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

class TeFreeMoveZoneGraph;

class TeFreeMoveZone : public TePickMesh2 {
public:
	struct CollidePoint {
		TeVector3f32 _point;
		float _distance;
	};

	TeFreeMoveZone();
	~TeFreeMoveZone();

	float bordersDistance() const;
	void buildAStar();
	void calcGridMatrix();
	void clear();
	Common::Array<TeVector3f32> collisions(const TeVector3f32 &v1, const TeVector3f32 &v2);
	TeVector3f32 correctCharacterPosition(const TeVector3f32 &pos, bool *flagout, bool f);

	TeIntrusivePtr<TeBezierCurve> curve(const TeVector3f32 &param_3, const TeVector2s32 &param_4, float param_5, bool findMeshFlag);
	TeIntrusivePtr<TeBezierCurve> curve(const TeVector3f32 &param_3, const TeVector2s32 &param_4);

	void draw() override;
	TeVector3f32 findNearestPointOnBorder(const TeVector2f32 &pt);
	bool hasBlockerIntersection(const TeVector2s32 &pt);
	bool hasCellBorderIntersection(const TeVector2s32 &pt);

	TeActZone *isInZone(const TeVector3f32 &pt);

	// loadBin() 2 versions, seem unused

	// name(), onPositionChanged(), position(), rotate(), rotation(), scale(),
	// setName(), setPosition(), setRotation(), setScale(), setVisible(),
	// translate(), and visible() are all implemented in original, but all
	// just do the same as super.

	bool onViewportChanged();
	void preUpdateGrid();
	TeVector3f32 projectOnAStarGrid(const TeVector3f32 &pt);
	Common::Array<TeVector3f32> &removeInsignificantPoints(const Common::Array<TeVector3f32> &points);
	void setBordersDistance(float dist);
	void setCamera(TeIntrusivePtr<TeCamera> &cam, bool noRecalcProjPoints);
	void setNbTriangles(unsigned long len);
	void setPathFindingOccluder(const TeOBP &occluder);
	void setVertex(unsigned long offset, const TeVector3f32 &vertex);
	TeVector2s32 transformAStarGridInWorldSpace(const TeVector2s32 &gridpt);
	float transformHeightMin(float minval);
	TeVector3f32 transformVectorInWorldSpace(float param_3,float param_4);
	void updateBorders();
	void updateGrid(bool force);
	void updatePickMesh();
	void updateProjectedPoints();
	void updateTransformedVertices();

	static float normalizeAngle(float angle);
	static void deserialize(Common::ReadStream &stream, TeFreeMoveZone &dest, Common::Array<TeBlocker> *blockers,
               Common::Array<TeRectBlocker> *rectblockers, Common::Array<TeActZone> *actzones);
	static void serialize(Common::WriteStream &stream, const TeFreeMoveZone &src, bool updateFirst);

private:
	Common::Array<TeActZone> *_actzones;
	Common::Array<TeBlocker> *_blockers;
	Common::Array<TeRectBlocker> *_rectBlockers;

	Common::Array<TeVector3f32> _freeMoveZoneVerticies;
	// TODO: Find better names..
	Common::Array<unsigned int> _uintArray1;
	Common::Array<TeVector3f32> _vectorArray;
	Common::Array<unsigned int> _uintArray2;

	TeVector2f32 _gridOffsetSomething;
	TeVector2f32 _someGridVec1;
	TeVector2f32 _someGridVec2;

	float _someGridFloat;

	TeOBP _obp;
	TeIntrusivePtr<TeCamera> _camera;
	//static TeIntrusivePtr<TeCamera> _globalCamera;

	bool _gridDirty;
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
