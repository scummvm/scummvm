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

namespace Tetraedge {

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
_transformedVerticiesDirty(true), _bordersDirty(true), _pickMeshDirty(true), _projectedPointsDirty(true)
{
	_graph = new TeFreeMoveZoneGraph();
	_graph->_bordersDistance = 2048.0f;
	_graph->_owner = this;
	_micropather = new micropather::MicroPather(_graph);
}

TeFreeMoveZone::~TeFreeMoveZone() {
	// TODO: remove signal.
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
	// TODO: Clear 3 other point vectors here.
	// TODO: _gridDirty = true;
	_graph->_flags.clear();
	_graph->_size = TeVector2s32(0, 0);
	_micropather->Reset();
}

Common::Array<TeVector3f32> TeFreeMoveZone::collisions(const TeVector3f32 &v1, const TeVector3f32 &v2) {
	updatePickMesh();
	updateProjectedPoints();
	error("TODO: Implement TeFreeMoveZone::collisions");
}

TeVector3f32 TeFreeMoveZone::correctCharacterPosition(const TeVector3f32 &pos, bool *flagout, bool f) {
	error("TODO: Implement TeFreeMoveZone::correctCharacterPosition");
}

TeIntrusivePtr<TeBezierCurve> TeFreeMoveZone::curve(const TeVector3f32 &param_3, const TeVector2s32 &param_4, float param_5, bool findMeshFlag) {
	error("TODO: Implement TeFreeMoveZone::curve");
}

TeIntrusivePtr<TeBezierCurve> TeFreeMoveZone::curve(const TeVector3f32 &param_3, const TeVector2s32 &param_4) {
	error("TODO: Implement TeFreeMoveZone::curve");
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

TeVector3f32 TeFreeMoveZone::projectOnAStarGrid(const TeVector3f32 &pt) {
	error("TODO: Implement TeFreeMoveZone::projectOnAStarGrid");
}

Common::Array<TeVector3f32> &TeFreeMoveZone::removeInsignificantPoints(const Common::Array<TeVector3f32> &points) {
	error("TODO: Implement TeFreeMoveZone::removeInsignificantPoints");
}

void TeFreeMoveZone::setBordersDistance(float dist) {
	_graph->_bordersDistance = dist;
}

void TeFreeMoveZone::setCamera(TeIntrusivePtr<TeCamera> &cam, bool recalcProjPoints) {
	error("TODO: Implement TeFreeMoveZone::setCamera");
}

void TeFreeMoveZone::setNbTriangles(unsigned long len) {
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

void TeFreeMoveZone::setVertex(unsigned long offset, const TeVector3f32 &vertex) {
	_freeMoveZoneVerticies[offset] = vertex;

	_gridDirty = true;
	_transformedVerticiesDirty = true;
	_bordersDirty = true;
	_pickMeshDirty = true;
	_projectedPointsDirty = true;
}

TeVector2s32 TeFreeMoveZone::transformAStarGridInWorldSpace(const TeVector2s32 &gridpt) {
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

float TeFreeMoveZoneGraph::LeastCostEstimate(void * stateStart, void *stateEnd) {
	error("TODO: Implement TeFreeMoveZone::TeFreeMoveZoneGraph::LeastCostEstimate");
}

void TeFreeMoveZoneGraph::AdjacentCost(void *state, Common::Array<micropather::StateCost> *adjacent) {
	error("TODO: Implement TeFreeMoveZone::TeFreeMoveZoneGraph::AdjacentCost");
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
		error("Flags unexpected size, expect %d got %d", _size._x * _size._y, flaglen);
	_flags.resize(flaglen);
	for (unsigned int i = 0; i < flaglen; i++) {
		_flags[i] = stream.readByte();
	}
	_bordersDistance = stream.readFloatLE();
}

void TeFreeMoveZoneGraph::serialize(Common::WriteStream &stream) const {
	error("TODO: Implement TeFreeMoveZoneGraph::serialize");
}


} // end namespace Tetraedge
