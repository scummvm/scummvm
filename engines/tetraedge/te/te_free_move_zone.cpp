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

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_free_move_zone.h"
#include "tetraedge/te/micropather.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_ray_intersection.h"

namespace Tetraedge {

/*static*/
//TeIntrusivePtr<TeCamera> TeFreeMoveZone::_globalCamera;

class TeFreeMoveZoneGraph : micropather::Graph {
	friend class TeFreeMoveZone;
	TeVector2s32 _size;
	Common::Array<char> _flags;
	float _bordersDistance;
	TeFreeMoveZone *_owner;

	// These don't match ScummVM naming convention but are needed to match MicroPather API.
	virtual float LeastCostEstimate(void * stateStart, void *stateEnd);
	virtual void AdjacentCost(void *state, Common::Array<micropather::StateCost> *adjacent);
	virtual void PrintStateInfo(void *state);

	int flag(const TeVector2s32 &loc);
	void setSize(const TeVector2s32 &size);

	void deserialize(Common::ReadStream &stream);
	void serialize(Common::WriteStream &stream) const;
};


TeFreeMoveZone::TeFreeMoveZone() : _actzones(nullptr), _blockers(nullptr), _rectBlockers(nullptr),
_transformedVerticiesDirty(true), _bordersDirty(true), _pickMeshDirty(true), _projectedPointsDirty(true),
_loadedFromBin(false)
{
	_graph = new TeFreeMoveZoneGraph();
	_graph->_bordersDistance = 2048.0f;
	_graph->_owner = this;
	_micropather = new micropather::MicroPather(_graph);
}

TeFreeMoveZone::~TeFreeMoveZone() {
	if (_camera) {
		_camera->onViewportChangedSignal().remove(this, &TeFreeMoveZone::onViewportChanged);
	}
	delete _micropather;
}

float TeFreeMoveZone::bordersDistance() const {
	return _graph->_bordersDistance;
}

void TeFreeMoveZone::buildAStar() {
	error("TODO: Implement TeFreeMoveZone::buildAStar");
}

void TeFreeMoveZone::calcGridMatrix() {
	error("TODO: Implement TeFreeMoveZone::calcGridMatrix");
}

void TeFreeMoveZone::clear() {
	setNbTriangles(0);
	_pickMeshDirty = true;
	_projectedPointsDirty = true;
	_vectorArray.clear();
	_uintArray2.clear();
	// TODO: Some other point vector here.
	_gridDirty = true;
	_graph->_flags.clear();
	_graph->_size = TeVector2s32(0, 0);
	_micropather->Reset();
}

Common::Array<TeVector3f32> TeFreeMoveZone::collisions(const TeVector3f32 &v1, const TeVector3f32 &v2) {
	updatePickMesh();
	updateProjectedPoints();
	error("TODO: Implement TeFreeMoveZone::collisions");
}

TeVector3f32 TeFreeMoveZone::correctCharacterPosition(const TeVector3f32 &pos, bool *flagout, bool intersectFlag) {
	float f = 0.0;
	TeVector3f32 intersectPoint;
	if (!intersect(pos, TeVector3f32(0, -1, 0), intersectPoint, f, intersectFlag, nullptr)) {
		if (!intersect(pos, TeVector3f32(0, 1, 0), intersectPoint, f, intersectFlag, nullptr)) {
			if (*flagout)
				*flagout = false;
			return pos;
		}
	}
	if (flagout)
		*flagout = true;
	return intersectPoint;
}

TeIntrusivePtr<TeBezierCurve> TeFreeMoveZone::curve(const TeVector3f32 &startpt, const TeVector2s32 &endpt, float param_5, bool findMeshFlag) {
	updateGrid(false);
	error("TODO: Implement TeFreeMoveZone::curve");
}

TeIntrusivePtr<TeBezierCurve> TeFreeMoveZone::curve(const TeVector3f32 &startpt, const TeVector3f32 &endpt) {
	updateGrid(false);
	Common::Array<TeVector3f32> points;
	points.push_back(startpt);
	points.push_back(endpt);

	TeVector2s32 projectedStart = projectOnAStarGrid(startpt);
	TeVector2s32 projectedEnd = projectOnAStarGrid(endpt);
	int xsize = _graph->_size._x;
	float cost = 0;
	// Passing an int to void*, yuck? but it's what the original does..
	Common::Array<void *> path;
	int pathResult = _micropather->Solve((void *)(xsize * projectedStart._y + projectedStart._x), (void *)(xsize * projectedEnd._y + projectedEnd._x), &path, &cost);

	TeIntrusivePtr<TeBezierCurve> retval;

	if (pathResult == micropather::MicroPather::SOLVED || pathResult == micropather::MicroPather::START_END_SAME) {
		Common::Array<TeVector2s32> points;
		points.resize(path.size());

		for (auto &pathpt : path) {
			// each path point is an array offset
			int offset = static_cast<int>(reinterpret_cast<long>(pathpt));
			int yoff = (offset / _graph->_size._x);
			int xoff = offset % _graph->_size._x;
			points.push_back(TeVector2s32(xoff, yoff));
		}

		Common::Array<TeVector3f32> pts3d;
		for (auto &pt : points) {
			pts3d.push_back(transformAStarGridInWorldSpace(pt));
		}

		pts3d.front() = startpt;
		pts3d.back() = endpt;
		removeInsignificantPoints(pts3d);
		retval = new TeBezierCurve();
		retval->setControlPoints(pts3d);
	}

	return retval;
}

/*static*/
void TeFreeMoveZone::deserialize(Common::ReadStream &stream, TeFreeMoveZone &dest, Common::Array<TeBlocker> *blockers,
               Common::Array<TeRectBlocker> *rectblockers, Common::Array<TeActZone> *actzones) {
	dest.clear();
	TePickMesh2::deserialize(stream, dest);
	TeVector2f32::deserialize(stream, dest._gridOffsetSomething);
	dest._transformedVerticiesDirty = (stream.readByte() != 0);
	dest._bordersDirty = (stream.readByte() != 0);
	dest._pickMeshDirty = (stream.readByte() != 0);
	dest._projectedPointsDirty = (stream.readByte() != 0);
	dest._gridDirty = (stream.readByte() != 0);

	Te3DObject2::deserializeVectorArray(stream, dest._freeMoveZoneVerticies);
	Te3DObject2::deserializeUintArray(stream, dest._uintArray1);
	Te3DObject2::deserializeVectorArray(stream, dest._vectorArray);
	Te3DObject2::deserializeUintArray(stream, dest._uintArray2);

	TeOBP::deserialize(stream, dest._obp);

	TeVector2f32::deserialize(stream, dest._someGridVec1);
	TeVector2f32::deserialize(stream, dest._someGridVec2);
	dest._someGridFloat = stream.readFloatLE();

	dest._graph->deserialize(stream);
	if (dest.name().contains("19000")) {
		dest._gridOffsetSomething = TeVector2f32(2.0, 2.0);
		dest._gridDirty = true;
	}
	dest._blockers = blockers;
	dest._rectBlockers = rectblockers;
	dest._actzones = actzones;
}

void TeFreeMoveZone::draw() {
	if (!worldVisible())
		return;

	TeRenderer *renderer = g_engine->getRenderer();
	renderer->enableWireFrame();
	TePickMesh2::draw();
	TeMesh mesh;
	mesh.setConf(_uintArray2.size(), _uintArray2.size(), TeMesh::MeshMode_Lines, 0, 0);

	error("TODO: Finish TeFreeMoveZone::draw");
}

TeVector3f32 TeFreeMoveZone::findNearestPointOnBorder(const TeVector2f32 &pt) {
	error("TODO: Implement TeFreeMoveZone::findNearestPointOnBorder");
}

bool TeFreeMoveZone::hasBlockerIntersection(const TeVector2s32 &pt) {
	error("TODO: Implement TeFreeMoveZone::hasBlockerIntersection");
}

bool TeFreeMoveZone::hasCellBorderIntersection(const TeVector2s32 &pt) {
	error("TODO: Implement TeFreeMoveZone::hasCellBorderIntersection");
}

TeActZone *TeFreeMoveZone::isInZone(const TeVector3f32 &pt) {
	error("TODO: Implement TeFreeMoveZone::isInZone");
}

bool TeFreeMoveZone::onViewportChanged() {
	_projectedPointsDirty = true;
	return false;
}

void TeFreeMoveZone::preUpdateGrid() {
	error("TODO: Implement TeFreeMoveZone::preUpdateGrid");
}

TeVector2s32 TeFreeMoveZone::projectOnAStarGrid(const TeVector3f32 &pt) {
	TeVector2f32 otherpt;
	if (!_loadedFromBin) {
		otherpt = TeVector2f32(pt.x() - _someGridVec1.getX(), pt.z() - _someGridVec1.getY());
	} else {
		error("TODO: Implement TeFreeMoveZone::projectOnAStarGrid for _loadedFromBin");
	}
	TeVector2f32 projected = otherpt / _gridOffsetSomething;
	return TeVector2s32((int)projected.getX(), (int)projected.getY());
}

Common::Array<TeVector3f32> TeFreeMoveZone::removeInsignificantPoints(const Common::Array<TeVector3f32> &points) {
	warning("TODO: Implement TeFreeMoveZone::removeInsignificantPoints");
	return points;
}

void TeFreeMoveZone::setBordersDistance(float dist) {
	_graph->_bordersDistance = dist;
}

void TeFreeMoveZone::setCamera(TeIntrusivePtr<TeCamera> &cam, bool noRecalcProjPoints) {
	if (_camera) {
		_camera->onViewportChangedSignal().remove(this, &TeFreeMoveZone::onViewportChanged);
	}
	//_globalCamera = camera;  // Seems like this is never used?
	_camera = cam;
	cam->onViewportChangedSignal().add(this, &TeFreeMoveZone::onViewportChanged);
	if (!noRecalcProjPoints)
		_projectedPointsDirty = true;
}

void TeFreeMoveZone::setNbTriangles(unsigned int len) {
	_freeMoveZoneVerticies.resize(len * 3);

	_gridDirty = true;
	_transformedVerticiesDirty = true;
	_bordersDirty = true;
	_pickMeshDirty = true;
	_projectedPointsDirty = true;
}

void TeFreeMoveZone::setPathFindingOccluder(const TeOBP &occluder) {
	error("TODO: Implement TeFreeMoveZone::setPathFindingOccluder");
}

void TeFreeMoveZone::setVertex(unsigned int offset, const TeVector3f32 &vertex) {
	_freeMoveZoneVerticies[offset] = vertex;

	_gridDirty = true;
	_transformedVerticiesDirty = true;
	_bordersDirty = true;
	_pickMeshDirty = true;
	_projectedPointsDirty = true;
}

TeVector3f32 TeFreeMoveZone::transformAStarGridInWorldSpace(const TeVector2s32 &gridpt) {
	float offsety = (float)gridpt._y * _gridOffsetSomething.getY() + _someGridVec1.getY() +
				_gridOffsetSomething.getY() * 0.5;
	float offsetx = (float)gridpt._x * _gridOffsetSomething.getX() + _someGridVec1.getX() +
				_gridOffsetSomething.getX() * 0.5;
	if (!_loadedFromBin) {
		return TeVector3f32(offsetx, _someGridFloat, offsety);
	}
	error("TODO: Implement TeFreeMoveZone::transformAStarGridInWorldSpace");
}

float TeFreeMoveZone::transformHeightMin(float minval) {
	error("TODO: Implement TeFreeMoveZone::transformHeightMin");
}

TeVector3f32 TeFreeMoveZone::transformVectorInWorldSpace(float param_3,float param_4) {
	error("TODO: Implement TeFreeMoveZone::transformVectorInWorldSpace");
}

void TeFreeMoveZone::updateBorders() {
	error("TODO: Implement TeFreeMoveZone::updateBorders");
}

void TeFreeMoveZone::updateGrid(bool force) {
	if (!force && !_gridDirty)
		return;
	_gridDirty = true;
	_updateTimer.stop();
	_updateTimer.start();
	buildAStar();
	_micropather->Reset();
	debug("[TeFreeMoveZone::updateGrid()] %s time : %.2f", name().c_str(), _updateTimer.getTimeFromStart() / 1000000.0);
	_gridDirty = false;
}

void TeFreeMoveZone::updatePickMesh() {
	if (!_pickMeshDirty)
		return;

	error("TODO: Implement TeFreeMoveZone::updatePickMesh");
}

void TeFreeMoveZone::updateProjectedPoints() {
	if (!_projectedPointsDirty)
		return;

	error("TODO: Implement TeFreeMoveZone::updateProjectedPoints");
}

void TeFreeMoveZone::updateTransformedVertices() {
	if (!_transformedVerticiesDirty)
		return;

	error("TODO: Implement TeFreeMoveZone::updateTransformedVertices");
}

/*========*/

float TeFreeMoveZoneGraph::LeastCostEstimate(void *stateStart, void *stateEnd) {
	int startInt = static_cast<int>(reinterpret_cast<long>(stateStart));
	int endInt = static_cast<int>(reinterpret_cast<long>(stateEnd));
	int starty = startInt / _size._x;
	int endy = endInt / _size._x;
	TeVector2s32 start(startInt - starty * _size._x, starty);
	TeVector2s32 end(endInt - endy * _size._x, endy);
	return (end - start).squaredLength();
}

void TeFreeMoveZoneGraph::AdjacentCost(void *state, Common::Array<micropather::StateCost> *adjacent) {
	int stateInt = static_cast<int>(reinterpret_cast<long>(state));
	int stateY = stateInt / _size._x;
	const TeVector2s32 statept(stateInt - stateY * _size._x, stateY);

	micropather::StateCost cost;
	TeVector2s32 pt;

	pt = TeVector2s32(statept._x - 1, statept._y);
	cost.state = reinterpret_cast<void *>(_size._x * pt._y + pt._x);
	cost.cost = (flag(pt) == 1 ? FLT_MAX : _bordersDistance);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x - 1, statept._y + 1);
	cost.state = reinterpret_cast<void *>(_size._x * pt._y + pt._x);
	cost.cost = (flag(pt) == 1 ? FLT_MAX : _bordersDistance);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x + 1, statept._y + 1);
	cost.state = reinterpret_cast<void *>(_size._x * pt._y + pt._x);
	cost.cost = (flag(pt) == 1 ? FLT_MAX : _bordersDistance);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x + 1, statept._y);
	cost.state = reinterpret_cast<void *>(_size._x * pt._y + pt._x);
	cost.cost = (flag(pt) == 1 ? FLT_MAX : _bordersDistance);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x + 1, statept._y - 1);
	cost.state = reinterpret_cast<void *>(_size._x * pt._y + pt._x);
	cost.cost = (flag(pt) == 1 ? FLT_MAX : _bordersDistance);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x, statept._y - 1);
	cost.state = reinterpret_cast<void *>(_size._x * pt._y + pt._x);
	cost.cost = (flag(pt) == 1 ? FLT_MAX : _bordersDistance);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x - 1, statept._y - 1);
	cost.state = reinterpret_cast<void *>(_size._x * pt._y + pt._x);
	cost.cost = (flag(pt) == 1 ? FLT_MAX : _bordersDistance);
	adjacent->push_back(cost);
}

void TeFreeMoveZoneGraph::PrintStateInfo(void *state) {
	error("TODO: Implement TeFreeMoveZone::TeFreeMoveZoneGraph::PrintStateInfo");
}

int TeFreeMoveZoneGraph::flag(const TeVector2s32 &loc) {
	if (loc._x < 0 || loc._x >= _size._x || loc._y < 0 || loc._y >= _size._y)
		return 1;
	return _flags[loc._y * _size._x + loc._x];
}

void TeFreeMoveZoneGraph::setSize(const TeVector2s32 &size) {
	_flags.clear();
	_size = size;
	_flags.resize(size._x * _size._y);
}

void TeFreeMoveZoneGraph::deserialize(Common::ReadStream &stream) {
	TeVector2s32::deserialize(stream, _size);
	uint32 flaglen = stream.readUint32LE();
	if (flaglen > 1000000 || (int)flaglen != _size._x * _size._y)
		error("TeFreeMoveZoneGraph: Flags unexpected size, expect %d got %d", _size._x * _size._y, flaglen);
	_flags.resize(flaglen);
	for (unsigned int i = 0; i < flaglen; i++) {
		_flags[i] = stream.readByte();
	}
	_bordersDistance = stream.readFloatLE();
}

void TeFreeMoveZoneGraph::serialize(Common::WriteStream &stream) const {
	error("TODO: Implement TeFreeMoveZoneGraph::serialize");
}

/*static*/
TePickMesh2 *TeFreeMoveZone::findNearestMesh(TeIntrusivePtr<TeCamera> &camera, const TeVector2s32 &frompt,
			Common::Array<TePickMesh2*> &pickMeshes, TeVector3f32 *outloc, bool lastHitFirst) {
	TeVector3f32 locresult;
	TePickMesh2 *nearest = nullptr;
	float furthest = camera->_orthFarVal;
	if (!pickMeshes.empty()) {
		TeVector3f32 v1;
		TeVector3f32 v2;
		for (unsigned int i = 0; i < pickMeshes.size(); i++) {
			TePickMesh2 *mesh = pickMeshes[i];
			const TeMatrix4x4 transform = mesh->worldTransformationMatrix();
			if (lastHitFirst) {
				unsigned int tricount = mesh->verticies().size() / 3;
				unsigned int vert = mesh->lastTriangleHit() * 3;
				if (mesh->lastTriangleHit() >= tricount)
					vert = 0;
				const TeVector3f32 v3 = transform * mesh->verticies()[vert];
				const TeVector3f32 v4 = transform * mesh->verticies()[vert + 1];
				const TeVector3f32 v5 = transform * mesh->verticies()[vert + 2];
				TeVector3f32 result;
				float fresult;
				int intresult = TeRayIntersection::intersect(v1, v2, v3, v4, v5, result, fresult);
				if (intresult == 1 && fresult < furthest && fresult >= camera->_orthNearVal)
					return mesh;
			}
			for (unsigned int tri = 0; tri < mesh->verticies().size() / 3; tri++) {
				const TeVector3f32 v3 = transform * mesh->verticies()[tri * 3];
				const TeVector3f32 v4 = transform * mesh->verticies()[tri * 3 + 1];
				const TeVector3f32 v5 = transform * mesh->verticies()[tri * 3 + 1];
				camera->getRay(frompt, v1, v2);
				TeVector3f32 result;
				float fresult;
				int intresult = TeRayIntersection::intersect(v1, v2, v3, v4, v5, result, fresult);
				if (intresult == 1 && fresult < furthest && fresult >= camera->_orthNearVal) {
					mesh->setLastTriangleHit(tri);
					locresult = result;
					furthest = fresult;
					nearest = mesh;
					if (lastHitFirst)
						break;
				}
			}
		}
	}
	if (outloc) {
		*outloc = locresult;
	}
	return nearest;
}


} // end namespace Tetraedge
