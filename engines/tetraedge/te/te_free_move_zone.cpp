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

#include "common/file.h"
#include "common/compression/deflate.h"

#include "tetraedge/te/te_free_move_zone.h"
#include "tetraedge/te/micropather.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_ray_intersection.h"
#include "tetraedge/te/te_core.h"

//#define TETRAEDGE_DUMP_PATHFINDING_DATA 1

namespace Tetraedge {

/*static*/
//TeIntrusivePtr<TeCamera> TeFreeMoveZone::_globalCamera;

/*static*/
bool TeFreeMoveZone::_collisionSlide = false;

class TeFreeMoveZoneGraph : micropather::Graph {
	friend class TeFreeMoveZone;

	TeFreeMoveZoneGraph() : _owner(nullptr), _bordersDistance(2048.0f) {}

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

	float costForPoint(TeVector2s32 pt) {
		int flg = flag(pt);
		if (flg == 1)
			return FLT_MAX;
		if (flg == 2)
			return _bordersDistance;
		return 1.0;
	}
};


TeFreeMoveZone::TeFreeMoveZone() : _actzones(nullptr), _blockers(nullptr), _rectBlockers(nullptr),
_transformedVerticiesDirty(true), _bordersDirty(true), _pickMeshDirty(true), _projectedPointsDirty(true),
_loadedFromBin(false), _gridWorldY(0.0), _gridSquareSize(5.0f, 5.0f), _gridDirty(true)
{
	_graph = new TeFreeMoveZoneGraph();
	_graph->_bordersDistance = 2048.0f;
	_graph->_owner = this;
	_micropather = new micropather::MicroPather(_graph);
}

TeFreeMoveZone::~TeFreeMoveZone() {
	if (_camera)
		_camera->onViewportChangedSignal().remove(this, &TeFreeMoveZone::onViewportChanged);

	delete _micropather;
	delete _graph;
}

float TeFreeMoveZone::bordersDistance() const {
	return _graph->_bordersDistance;
}

TeVector2s32 TeFreeMoveZone::aStarResolution() const {
	TeVector2f32 diff = (_gridBottomRight - _gridTopLeft);
	TeVector2s32 retval = TeVector2s32(diff / _gridSquareSize) + TeVector2s32(1, 1);
	if (retval._x > 2000)
		retval._x = 200;
	if (retval._y > 2000)
		retval._y = 200;
	return retval;
}

void TeFreeMoveZone::buildAStar() {
	preUpdateGrid();
	const TeVector2s32 graphSize = aStarResolution();
	_graph->setSize(graphSize);

	// Original checks these inside the loop below, seems like a waste as they never change?
	if (graphSize._x == 0 || graphSize._y == 0)
		return;

	bool regenerate = true;
	if (!_aszGridPath.empty()) {
		regenerate = !loadAStar(_aszGridPath, graphSize);
	}
	if (!regenerate)
		return;

	if (!_loadedFromBin) {
		for (int x = 0; x < graphSize._x; x++) {
			for (int y = 0; y < graphSize._y; y++) {
				byte blockerIntersection = hasBlockerIntersection(TeVector2s32(x, y));
				if (blockerIntersection == 1) {
					_graph->_flags[_graph->_size._x * y + x] = 1;
				} else {
					if (!hasCellBorderIntersection(TeVector2s32(x, y))) {
						const float gridSquareX = _gridSquareSize.getX();
						const float gridSquareY = _gridSquareSize.getY();
						TeVector3f32 vout;
						float fout;
						TeVector3f32 gridPt(x * gridSquareX + _gridTopLeft.getX() + gridSquareX / 2,
										1000000.0,
										y * gridSquareY + _gridTopLeft.getY() + gridSquareY / 2);
						bool doesIntersect = intersect(gridPt, TeVector3f32(0.0, -1.0, 0.0), vout, fout, true, nullptr);
						if (!doesIntersect)
							doesIntersect = intersect(gridPt, TeVector3f32(0.0, 1.0, 0.0), vout, fout, true, nullptr);

						if (!doesIntersect)
							_graph->_flags[graphSize._x * y + x] = 1;
						else if (blockerIntersection == 2)
							_graph->_flags[graphSize._x * y + x] = 2;
						else
							_graph->_flags[graphSize._x * y + x] = 0;
					} else {
						_graph->_flags[graphSize._x * y + x] = 2;
					}
				}
			}
		}
	} else {
		// Loaded from bin..
		for (int x = 0; x < graphSize._x; x++) {
			for (int y = 0; y < graphSize._y; y++) {
				byte blockerIntersection = hasBlockerIntersection(TeVector2s32(x, y));
				if (blockerIntersection == 1) {
					_graph->_flags[_graph->_size._x * y + x] = 1;
				} else {
					if (!hasCellBorderIntersection(TeVector2s32(x, y))) {
						const float gridSquareX = _gridSquareSize.getX();
						const float gridSquareY = _gridSquareSize.getY();
						TeVector3f32 gridPt = _gridMatrix * TeVector3f32(
								 x * gridSquareX + _gridTopLeft.getX() + gridSquareX / 2, 0.0,
								 y * gridSquareY + _gridTopLeft.getY() + gridSquareY / 2);
						bool doesIntersect = intersect2D(TeVector2f32(gridPt.x(), gridPt.z()));
						if (!doesIntersect)
							_graph->_flags[graphSize._x * y + x] = 1;
						else if (blockerIntersection == 2)
							_graph->_flags[graphSize._x * y + x] = 2;
						else
							_graph->_flags[graphSize._x * y + x] = 0;
					} else {
						_graph->_flags[graphSize._x * y + x] = 2;
					}
				}
			}
		}
	}
}

bool TeFreeMoveZone::loadAStar(const Common::Path &path, const TeVector2s32 &size) {
	TetraedgeFSNode node = g_engine->getCore()->findFile(path);
	Common::ScopedPtr<Common::SeekableReadStream> file;
	if (!node.isReadable()) {
		warning("[TeFreeMoveZone::loadAStar] Can't open file : %s.", path.toString().c_str());
		return false;
	}
	file.reset(node.createReadStream());
	if (!file) {
		warning("[TeFreeMoveZone::loadAStar] Can't open file : %s.", path.toString().c_str());
		return false;
	}
	TeVector2s32 readSize;
	readSize.deserialize(*file, readSize);
	if (size != readSize) {
		warning("[TeFreeMoveZone::loadAStar] Wrong file : %s.", path.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}
	uint32 bytes = file->readUint32LE();
	if (bytes > 100000)
		error("Improbable size %d for compressed astar data", bytes);

	unsigned long decompBytes = size._x * size._y;
	byte *buf = new byte[bytes];
	byte *outBuf = new byte[decompBytes];
	file->read(buf, bytes);
	bool result = Common::inflateZlib(outBuf, &decompBytes, buf, bytes);
	delete [] buf;
	if (result) {
		for (uint i = 0; i < decompBytes; i++)
			_graph->_flags.data()[i] = outBuf[i];
	}
	delete [] outBuf;
	return result;
}


void TeFreeMoveZone::calcGridMatrix() {
	float angle = 0.0f;
	float mul = 0.0f;
	for (uint i = 0; i < _borders.size() - 1; i += 2) {
		const TeVector3f32 &v1 = _verticies[_borders[i]];
		const TeVector3f32 &v2 = _verticies[_borders[i + 1]];
		const TeVector3f32 diff = v2 - v1;
		const TeVector2f32 diff2(diff.x(), diff.z());
		float len = diff2.length();
		float f = fmod(atan2(diff.z(), diff.x()), M_PI_2);
		if (f < 0)
			f += (float)M_PI_2;

		if (f - angle < -M_PI_4) {
			angle -= (float)M_PI_2;
		} else if (f - angle > M_PI_4) {
			f -= (float)M_PI_2;
		}

		angle *= mul;
		mul += len;
		angle = fmod((f * len + angle) / mul, M_PI_2);
		if (angle < 0)
			angle += (float)M_PI_2;
	}

	const TeQuaternion rot = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), angle);
	const TeMatrix4x4 rotMatrix = rot.toTeMatrix();
	_gridMatrix = TeMatrix4x4() * rotMatrix;
}

void TeFreeMoveZone::clear() {
	setNbTriangles(0);
	_pickMeshDirty = true;
	_projectedPointsDirty = true;
	_transformedVerticies.clear();
	_borders.clear();
	// TODO: Clear some other TeVector2f32 list here (field_0x178)
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
	TeVector3f32 testPos(pos.x(), 0, pos.z());
	if (!intersect(testPos, TeVector3f32(0, -1, 0), intersectPoint, f, intersectFlag, nullptr)) {
		if (!intersect(testPos, TeVector3f32(0, 1, 0), intersectPoint, f, intersectFlag, nullptr)) {
			// Note: This flag should only ever get set in Syberia 2.
			if (!_collisionSlide) {
				if (flagout)
					*flagout = false;
				return pos;
			}
			return slide(pos);
		}
	}
	if (flagout)
		*flagout = true;
	return intersectPoint;
}


TeIntrusivePtr<TeBezierCurve> TeFreeMoveZone::curve(const TeVector3f32 &startpt, const TeVector2s32 &clickPt, float param_5, bool findMeshFlag) {
	updateGrid(false);
	Common::Array<TePickMesh2 *> meshes;
	TeVector3f32 newend;
	meshes.push_back(this);

	TePickMesh2 *nearest = findNearestMesh(_camera, clickPt, meshes, &newend, findMeshFlag);
	if (!nearest) {
		if (g_engine->gameType() == TetraedgeEngine::kSyberia2) {
			newend = findNearestPointOnBorder(TeVector2f32(clickPt));
		} else {
			return TeIntrusivePtr<TeBezierCurve>();
		}
	}

	return curve(startpt, newend);
}

TeIntrusivePtr<TeBezierCurve> TeFreeMoveZone::curve(const TeVector3f32 &startpt, const TeVector3f32 &endpt) {
	updateGrid(false);
	const TeVector2s32 projectedStart = projectOnAStarGrid(startpt);
	const TeVector2s32 projectedEnd = projectOnAStarGrid(endpt);
	const int xsize = _graph->_size._x;
	char *graphData = _graph->_flags.data();
	float cost = 0;
	Common::Array<void *> path;
	int pathResult = _micropather->Solve(graphData + xsize * projectedStart._y + projectedStart._x,
			graphData + xsize * projectedEnd._y + projectedEnd._x, &path, &cost);

	TeIntrusivePtr<TeBezierCurve> retval;

	if (pathResult == micropather::MicroPather::SOLVED || pathResult == micropather::MicroPather::START_END_SAME) {
		Common::Array<TeVector2s32> points;

		for (auto pathpt : path) {
			// each path point is an array offset
			int offset = (char *)pathpt - graphData;
			points.push_back(TeVector2s32(offset % xsize, offset / xsize));
		}

		Common::Array<TeVector3f32> pts3d;
		// Skip first and last points and use the exact start/end values.
		pts3d.push_back(startpt);
		for (int i = 1; i < (int)points.size() - 1; i++) {
			pts3d.push_back(transformAStarGridInWorldSpace(points[i]));
		}
		pts3d.push_back(endpt);

#ifdef TETRAEDGE_DUMP_PATHFINDING_DATA
		debug("curve: pathfind from %s to %s", startpt.dump().c_str(), endpt.dump().c_str());
		debug("curve: %d grid points:", points.size());
		for (uint i = 0; i < points.size(); i++)
			debug("curve: gridpt %2d: %d, %d", i, points[i]._x, points[i]._y);
		debug("curve: %d 3d (world) points:", pts3d.size());
		for (uint i = 0; i < pts3d.size(); i++)
			debug("curve: wrldpt %2d: %s", i, pts3d[i].dump().c_str());
		uint firstsz = pts3d.size();
#endif

		removeInsignificantPoints(pts3d);

#ifdef TETRAEDGE_DUMP_PATHFINDING_DATA
		debug("curve: removed insignificant pts, %d -> %d", firstsz, pts3d.size());
#endif

		retval = new TeBezierCurve();
		retval->setControlPoints(pts3d);
	}
	// If no path found, return nullptr.

	return retval;
}

/*static*/
void TeFreeMoveZone::deserialize(Common::ReadStream &stream, TeFreeMoveZone &dest, const Common::Array<TeBlocker> *blockers,
			const Common::Array<TeRectBlocker> *rectblockers, const Common::Array<TeActZone> *actzones) {
	dest.clear();
	TePickMesh2::deserialize(stream, dest);
	TeVector2f32::deserialize(stream, dest._gridSquareSize);
	dest._transformedVerticiesDirty = (stream.readByte() != 0);
	dest._bordersDirty = (stream.readByte() != 0);
	dest._pickMeshDirty = (stream.readByte() != 0);
	dest._projectedPointsDirty = (stream.readByte() != 0);
	dest._gridDirty = (stream.readByte() != 0);

	Te3DObject2::deserializeVectorArray(stream, dest._freeMoveZoneVerticies);
	Te3DObject2::deserializeUintArray(stream, dest._pickMesh);
	Te3DObject2::deserializeVectorArray(stream, dest._transformedVerticies);
	Te3DObject2::deserializeUintArray(stream, dest._borders);

	TeOBP::deserialize(stream, dest._obp);

	TeVector2f32::deserialize(stream, dest._gridTopLeft);
	TeVector2f32::deserialize(stream, dest._gridBottomRight);
	dest._gridWorldY = stream.readFloatLE();

	dest._graph->deserialize(stream);
	if (dest.name().contains("19000")) {
		dest._gridSquareSize = TeVector2f32(2.0f, 2.0f);
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
	Common::SharedPtr<TeMesh> mesh(TeMesh::makeInstance());
	mesh->setConf(_borders.size(), _borders.size(), TeMesh::MeshMode_Lines, 0, 0);
	for (uint i = 0; i < _borders.size(); i++) {
		mesh->setIndex(i, i);
		mesh->setVertex(i, verticies()[_borders[i]]);
	}

	const TeColor prevColor = renderer->currentColor();
	renderer->pushMatrix();
	renderer->multiplyMatrix(worldTransformationMatrix());
	renderer->setCurrentColor(TeColor(0, 0x80, 0xff, 0xff));
	mesh->draw();

	if (!_loadedFromBin)
		renderer->popMatrix();

	if (!_gridDirty && false) {
		const TeVector2s32 aStarRes = aStarResolution();
		// Note: original iterates through the graph first here and
		// calls flag but doesn't do anything with it.. not sure why?
		for (int x = 0; x < aStarRes._x; x++) {
			for (int y = 0; y < aStarRes._y; y++) {
				float left = _gridSquareSize.getX() * x + _gridTopLeft.getX();
				float top  = _gridSquareSize.getY() * y + _gridTopLeft.getY();
				const TeVector3f32 tl(left, _gridWorldY, top);
				const TeVector3f32 tr(left + _gridSquareSize.getX(), _gridWorldY, top);
				const TeVector3f32 bl(left, _gridWorldY, top + _gridSquareSize.getY());
				const TeVector3f32 br(left + _gridSquareSize.getX(), _gridWorldY, top + _gridSquareSize.getY());

				int flag = _graph->flag(TeVector2s32(x, y));
				if (flag == 1) {
					renderer->setCurrentColor(TeColor(0xff, 0xff, 0xff, 0xff));
				} else if (flag == 2) {
					renderer->setCurrentColor(TeColor(0xff, 0xff, 0, 0xff));
				} else {
					renderer->setCurrentColor(TeColor(0, 0xff, 0, 0xff));
				}

				renderer->drawLine(tl, tr);
				renderer->drawLine(tr, br);
				renderer->drawLine(tl, br);
				renderer->drawLine(tr, bl);
			}
		}
	}

	// TODO: do a bunch of other drawing stuff here (line 294 on)

	if (_loadedFromBin)
		renderer->popMatrix();
	renderer->setCurrentColor(prevColor);
	renderer->disableWireFrame();
}

static int segmentIntersection(const TeVector2f32 &s1start, const TeVector2f32 &s1end,
						const TeVector2f32 &s2start, const TeVector2f32 &s2end,
						TeVector2f32 *sout, float *fout1, float *fout2) {
	const TeVector2f32 s1len = s1end - s1start;
	const TeVector2f32 s2len = s2end - s2start;
	float det = s1len.getX() * s2len.getY() + s1len.getY() * s2len.getX();
	int result = 0;
	if (det != 0) {
		result = 1;
		float intersect1 = -((s1len.getY() * s1start.getX() +
						(s1len.getX() * s2start.getY() - s1len.getX() * s1start.getY())) -
						 s1len.getY() * s2start.getX()) / det;
		if (intersect1 >= 0.0f && intersect1 <= 1.0f) {
			float intersect2 = -((s2len.getY() * s2start.getX() +
						(s2len.getX() * s1start.getY() - s2len.getX() * s2start.getY())) -
						 s2len.getY() * s1start.getX()) / det;
			if (intersect2 >= 0.0f && intersect2 <= 1.0f) {
				result = 2;
				if (sout)
					*sout = s1start + s1len * intersect2;
				if (fout1)
					*fout1 = intersect2;
				if (fout2)
					*fout2 = intersect1;
			}
		}
	}
	return result;
}

TeVector3f32 TeFreeMoveZone::findNearestPointOnBorder(const TeVector2f32 &pt) {
	TeVector3f32 retval;
	const TeVector2f32 pt_x0(pt.getX(), 0);
	const TeVector2f32 pt_x1(pt.getX(), _camera->getViewportHeight());
	const TeVector2f32 pt_y0(0, pt.getY());
	const TeVector2f32 pt_y1(_camera->getViewportWidth(), pt.getY());

	updateProjectedPoints();
	updateBorders();

	float leastDist = FLT_MAX;
	for (uint i = 0; i < _borders.size() / 2; i++) {
		uint b1 = _borders[i * 2];
		uint b2 = _borders[i * 2 + 1];
		const TeVector2f32 &projb1 = _projectedPoints[b1];
		const TeVector2f32 &projb2 = _projectedPoints[b2];
		const TeVector3f32 &transb1 = _transformedVerticies[_pickMesh[b1]];
		const TeVector3f32 &transb2 = _transformedVerticies[_pickMesh[b2]];
		float dist = 0;
		TeVector2f32 dir;
		if (segmentIntersection(pt_x0, pt_x1, projb1, projb2, &dir, nullptr, &dist) == 2) {
			float sqLen = (dir - pt).getSquareMagnitude();
			if (sqLen < leastDist) {
				retval = transb1 + (transb2 - transb1) * dist;
				leastDist = sqLen;
			}
		}
        if (segmentIntersection(pt_y0, pt_y1, projb1, projb2, &dir, nullptr, &dist) == 2) {
			float sqLen = (dir - pt).getSquareMagnitude();
			if (sqLen < leastDist) {
				retval = transb1 + (transb2 - transb1) * dist;
				leastDist = sqLen;
			}
		}
	}
	return retval;
}

byte TeFreeMoveZone::hasBlockerIntersection(const TeVector2s32 &pt) {
	TeVector2f32 borders[4];

	const float gridSquareX = _gridSquareSize.getX();
	const float gridSquareY = _gridSquareSize.getY();
	borders[0] = TeVector2f32(pt._x * gridSquareX + _gridTopLeft.getX(),
							  pt._y * gridSquareY + _gridTopLeft.getY());
	borders[1] = TeVector2f32(pt._x * gridSquareX + _gridTopLeft.getX() + gridSquareX,
							  pt._y * gridSquareY + _gridTopLeft.getY());
	borders[2] = TeVector2f32(pt._x * gridSquareX + _gridTopLeft.getX(),
							  pt._y * gridSquareY + _gridTopLeft.getY() + gridSquareY);
	borders[3] = TeVector2f32(pt._x * gridSquareX + _gridTopLeft.getX() + gridSquareX,
							  pt._y * gridSquareY + _gridTopLeft.getY() + gridSquareY);

	for (uint i = 0; i < _blockers->size(); i++) {
		const TeBlocker &blocker = (*_blockers)[i];

		if (blocker._s != name())
			continue;

		for (uint b = 0; b < 4; b++) {
			int si = segmentIntersection(borders[b], borders[(b + 1) % 4], blocker._pts[0],
										 blocker._pts[1], nullptr, nullptr, nullptr);
			if (si == 2)
				return 2;
		}

		TeVector2f32 borderVec = ((borders[0] + borders[3]) / 2.0) - blocker._pts[0];
		TeVector2f32 blockerVec = blocker._pts[1] - blocker._pts[0];
		float dotVal = borderVec.dotProduct(blockerVec.getNormalized());
		float crosVal = borderVec.crossProduct(blockerVec);
		if ((crosVal < 0.0) && (0.0 <= dotVal)) {
			if (dotVal < blockerVec.length())
				return 1;
		}
	}
	return 0;
}

bool TeFreeMoveZone::hasCellBorderIntersection(const TeVector2s32 &pt) {
	TeVector2f32 borders[4];

	const float gridSquareX = _gridSquareSize.getX();
	const float gridSquareY = _gridSquareSize.getY();
	borders[0] = TeVector2f32(pt._x * gridSquareX + _gridTopLeft.getX(),
							  pt._y * gridSquareY + _gridTopLeft.getY());
	borders[1] = TeVector2f32(pt._x * gridSquareX + _gridTopLeft.getX() + gridSquareX,
							  pt._y * gridSquareY + _gridTopLeft.getY());
	borders[2] = TeVector2f32(pt._x * gridSquareX + _gridTopLeft.getX(),
							  pt._y * gridSquareY + _gridTopLeft.getY() + gridSquareY);
	borders[3] = TeVector2f32(pt._x * gridSquareX + _gridTopLeft.getX() + gridSquareX,
							  pt._y * gridSquareY + _gridTopLeft.getY() + gridSquareY);

	int iresult = 0;
	for (uint border = 0; border < _borders.size() / 2; border++) {
		TeVector2f32 v1;
		TeVector2f32 v2;
		uint off1 = _pickMesh[_borders[border * 2]];
		uint off2 = _pickMesh[_borders[border * 2 + 1]];
		if (!_loadedFromBin) {
			v1 = TeVector2f32(_transformedVerticies[off1].x(), _transformedVerticies[off1].z());
			v2 = TeVector2f32(_transformedVerticies[off2].x(), _transformedVerticies[off2].z());
		} else {
			TeMatrix4x4 gridInverse = _gridMatrix;
			gridInverse.inverse();
			const TeVector3f32 v1_inv = gridInverse * _freeMoveZoneVerticies[off1];
			const TeVector3f32 v2_inv = gridInverse * _freeMoveZoneVerticies[off2];
			v1 = TeVector2f32(v1_inv.x(), v1_inv.z());
			v2 = TeVector2f32(v2_inv.x(), v2_inv.z());
		}
		iresult = segmentIntersection(borders[0], borders[1], v1, v2, nullptr, nullptr, nullptr);
		if (iresult == 2) break;
		iresult = segmentIntersection(borders[1], borders[2], v1, v2, nullptr, nullptr, nullptr);
		if (iresult == 2) break;
		iresult = segmentIntersection(borders[2], borders[3], v1, v2, nullptr, nullptr, nullptr);
		if (iresult == 2) break;
		iresult = segmentIntersection(borders[3], borders[0], v1, v2, nullptr, nullptr, nullptr);
		if (iresult == 2) break;
	}
	return iresult == 2;
}

TeActZone *TeFreeMoveZone::isInZone(const TeVector3f32 &pt) {
	error("TODO: Implement TeFreeMoveZone::isInZone");
}

bool TeFreeMoveZone::loadBin(const Common::Path &path, const Common::Array<TeBlocker> *blockers,
		const Common::Array<TeRectBlocker> *rectblockers, const Common::Array<TeActZone> *actzones,
		const TeVector2f32 &gridSize) {
	TetraedgeFSNode node = g_engine->getCore()->findFile(path);
	if (!node.isReadable()) {
		warning("[TeFreeMoveZone::loadBin] Can't open file : %s.", node.getName().c_str());
		return false;
	}
	_aszGridPath = path.append(".aszgrid");
	Common::ScopedPtr<Common::SeekableReadStream> file(node.createReadStream());
	return loadBin(*file, blockers, rectblockers, actzones, gridSize);
}

bool TeFreeMoveZone::loadBin(Common::ReadStream &stream, const Common::Array<TeBlocker> *blockers,
		const Common::Array<TeRectBlocker> *rectblockers, const Common::Array<TeActZone> *actzones,
		const TeVector2f32 &gridSize) {
	_loadGridSize = gridSize;
	_loadedFromBin = true;

	// Load position, rotation, scale (not name)
	Te3DObject2::deserialize(stream, *this, false);

	Common::Array<TeVector3f32> vecs;
	Te3DObject2::deserializeVectorArray(stream, vecs);

	uint32 triangles = stream.readUint32LE();
	_freeMoveZoneVerticies.resize(triangles * 3);
	_gridDirty = true;
	_transformedVerticiesDirty = true;
	_bordersDirty = true;
	_pickMeshDirty = true;
	_projectedPointsDirty = true;

	for (uint v = 0; v < triangles * 3; v++) {
		uint16 s = stream.readUint16LE();
		if (s >= vecs.size())
			error("Invalid vertex offset %d (of %d) loading TeFreeMoveZone", s, vecs.size());
		_freeMoveZoneVerticies[v] = vecs[s];
	}
	updateTransformedVertices();
	updatePickMesh();

	_blockers = blockers;
	_rectBlockers = rectblockers;
	_actzones = actzones;
	updateGrid(false);
	Common::Path p(name());
	setName(p.baseName());

	return true;
}

bool TeFreeMoveZone::onViewportChanged() {
	_projectedPointsDirty = true;
	return false;
}

void TeFreeMoveZone::preUpdateGrid() {
	updateTransformedVertices();
	updatePickMesh();
	updateBorders();
	if (_loadedFromBin)
		calcGridMatrix();

	TeMatrix4x4 gridInverse = _gridMatrix;
	gridInverse.inverse();

	TeVector3f32 newVec;
	if (_transformedVerticies.empty() || _pickMesh.empty()) {
		debug("[TeFreeMoveZone::buildAStar] %s have no mesh or is entierly occluded", name().c_str());
	} else {
		if (!_loadedFromBin)
			newVec = _transformedVerticies[_pickMesh[0]];
		else
			newVec = gridInverse * _freeMoveZoneVerticies[_pickMesh[0]];

		_gridTopLeft.setX(newVec.x());
		_gridTopLeft.setY(newVec.z());
		_gridBottomRight = _gridTopLeft;

		_gridWorldY = newVec.y();
	}
	for (uint i = 0; i < _pickMesh.size(); i++) {
		uint vertNo = _pickMesh[_pickMesh[i]];

		if (!_loadedFromBin)
			newVec = _transformedVerticies[vertNo];
		else
			newVec = gridInverse * _freeMoveZoneVerticies[vertNo];

		if (_gridTopLeft.getX() > newVec.x()) {
			_gridTopLeft.setX(newVec.x());
		} else if (_gridBottomRight.getX() < newVec.x()) {
			_gridBottomRight.setX(newVec.x());
		}

		if (_gridTopLeft.getY() > newVec.z()) {
			_gridTopLeft.setY(newVec.z());
		} else if (_gridBottomRight.getY() < newVec.z()) {
			_gridBottomRight.setY(newVec.z());
		}

		if (newVec.y() < _gridWorldY)
			_gridWorldY = newVec.y();
	}

	if (!_loadedFromBin) {
		if (!name().contains("19000"))
			_gridSquareSize = TeVector2f32(5.0f, 5.0f);
		else
			_gridSquareSize = TeVector2f32(2.0f, 2.0f);
	} else {
		/* Syberia 1 code, never actually used..
		const TeVector2f32 gridVecDiff = _gridBottomRight - _gridTopLeft;
		float minSide = MIN(gridVecDiff.getX(), gridVecDiff.getY()) / 20.0f;
		_gridSquareSize.setX(minSide);
		_gridSquareSize.setY(minSide);
		if (_loadGridSize.getX())
			_gridSquareSize = _loadGridSize;
		*/
		_gridSquareSize = TeVector2f32(20.0f, 20.0f);
	}

	TeMatrix4x4 worldTrans = worldTransformationMatrix();
	worldTrans.inverse();
	_inverseWorldTransform = worldTrans;
}

TeVector2s32 TeFreeMoveZone::projectOnAStarGrid(const TeVector3f32 &pt) {
	TeVector2f32 offsetpt;
	if (!_loadedFromBin) {
		offsetpt = TeVector2f32(pt.x() - _gridTopLeft.getX(), pt.z() - _gridTopLeft.getY());
	} else {
		TeMatrix4x4 invGrid = _gridMatrix;
		invGrid.inverse();
		TeVector3f32 transPt = invGrid * (_inverseWorldTransform * pt);
		offsetpt.setX(transPt.x() - _gridTopLeft.getX());
		offsetpt.setY(transPt.z() - _gridTopLeft.getY());
	}
	const TeVector2f32 projected = offsetpt / _gridSquareSize;
	return TeVector2s32((int)projected.getX(), (int)projected.getY());
}

Common::Array<TeVector3f32> TeFreeMoveZone::removeInsignificantPoints(const Common::Array<TeVector3f32> &points) {
	if (points.size() < 2)
		return points;

	Common::Array<TeVector3f32> result;
	result.push_back(points[0]);

	if (points.size() > 2) {
		int point1 = 0;
		int point2 = 2;
		do {
			const TeVector2f32 pt1(points[point1].x(), points[point1].z());
			const TeVector2f32 pt2(points[point2].x(), points[point2].z());
			for (uint i = 0; i * 2 < _borders.size() / 2; i++) {
				const TeVector3f32 transpt3d1 = worldTransformationMatrix() * verticies()[_borders[i * 2]];
				const TeVector2f32 transpt1(transpt3d1.x(), transpt3d1.z());
				const TeVector3f32 transpt3d2 = worldTransformationMatrix() * verticies()[_borders[i * 2 + 1]];
				const TeVector2f32 transpt2(transpt3d2.x(), transpt3d2.z());
				if (segmentIntersection(pt1, pt2, transpt1, transpt2, nullptr, nullptr, nullptr) == 2)
					break;
			}
			point1 = point2 - 1;
			result.push_back(points[point1]);
			point2++;
		} while (point2 < (int)points.size());
	}

	if (result.back() != points[points.size() - 2]) {
		result.push_back(points[points.size() - 1]);
	} else {
		result.back() = points[points.size() - 1];
	}
	return result;
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

void TeFreeMoveZone::setNbTriangles(uint len) {
	_freeMoveZoneVerticies.resize(len * 3);

	_gridDirty = true;
	_transformedVerticiesDirty = true;
	_bordersDirty = true;
	_pickMeshDirty = true;
	_projectedPointsDirty = true;
}

void TeFreeMoveZone::setPathFindingOccluder(const TeOBP &occluder) {
	_obp = occluder;
	_projectedPointsDirty = true;
	_bordersDirty = true;
	_gridDirty = true;
}

void TeFreeMoveZone::setVertex(uint offset, const TeVector3f32 &vertex) {
	_freeMoveZoneVerticies[offset] = vertex;

	_gridDirty = true;
	_transformedVerticiesDirty = true;
	_bordersDirty = true;
	_pickMeshDirty = true;
	_projectedPointsDirty = true;
}

TeVector3f32 TeFreeMoveZone::transformAStarGridInWorldSpace(const TeVector2s32 &gridpt) {
	float offsetx = (float)gridpt._x * _gridSquareSize.getX() + _gridTopLeft.getX() +
				_gridSquareSize.getX() / 2.0f;
	float offsety = (float)gridpt._y * _gridSquareSize.getY() + _gridTopLeft.getY() +
				_gridSquareSize.getY() / 2.0f;
	if (!_loadedFromBin) {
		return TeVector3f32(offsetx, _gridWorldY, offsety);
	} else {
		TeVector3f32 result = _gridMatrix * TeVector3f32(offsetx, _gridWorldY, offsety);
		return worldTransformationMatrix() * result;
	}
}

float TeFreeMoveZone::transformHeightMin(float minval) {
	TeVector3f32 vec = worldTransformationMatrix() * TeVector3f32(_gridTopLeft.getX(), minval, _gridTopLeft.getY());
	return vec.y();
}

TeVector3f32 TeFreeMoveZone::transformVectorInWorldSpace(float x, float y) {
	TeVector3f32 vec = _gridMatrix * TeVector3f32(x, _gridWorldY, y);
	return worldTransformationMatrix() * vec;
}

void TeFreeMoveZone::updateBorders() {
	if (!_bordersDirty)
		return;

	updatePickMesh();

	for (uint triNo1 = 0; triNo1 < _verticies.size() / 3; triNo1++) {
		for (uint vecNo1 = 0; vecNo1 < 3; vecNo1++) {
			uint left1 = triNo1 * 3 + vecNo1;
			uint left2 = triNo1 * 3 + (vecNo1 == 2 ? 0 : vecNo1 + 1);
			const TeVector3f32 vleft1 = _verticies[left1];
			const TeVector3f32 vleft2 = _verticies[left2];

			bool skip = false;
			for (uint triNo2 = 0; triNo2 < _verticies.size() / 3; triNo2++) {
				if (skip)
					break;
				if (triNo2 == triNo1)
					continue;

				for (uint vecNo2 = 0; vecNo2 < 3; vecNo2++) {
					uint right1 = triNo2 * 3 + vecNo2;
					uint right2 = triNo2 * 3 + (vecNo2 == 2 ? 0 : vecNo2 + 1);
					const TeVector3f32 vright1 = _verticies[right1];
					const TeVector3f32 vright2 = _verticies[right2];
					if ((vright1 == vleft1 && vright2 == vleft2) || (vright1 == vleft2 && vright2 == vleft1)) {
						skip = true;
						break;
					}
				}
			}
			if (!skip) {
				_borders.push_back(left1);
				_borders.push_back(left2);
			}
		}
	}
	_bordersDirty = false;
}

void TeFreeMoveZone::updateGrid(bool force) {
	if (!force && !_gridDirty)
		return;
	_gridDirty = true;
	_updateTimer.stop();
	_updateTimer.start();
	buildAStar();
	_micropather->Reset();
	// This debug msg copied from the original, but it's
	// a bit noisy and not so useful.
	// debug("[TeFreeMoveZone::updateGrid()] %s time : %.2f", name().c_str(), _updateTimer.getTimeFromStart() / 1000000.0);
	_gridDirty = false;
}

void TeFreeMoveZone::updatePickMesh() {
	if (!_pickMeshDirty)
		return;

	updateTransformedVertices();
	_pickMesh.clear();
	_pickMesh.reserve(_freeMoveZoneVerticies.size());
	int vecNo = 0;
	for (uint tri = 0; tri < _freeMoveZoneVerticies.size() / 3; tri++) {
		_pickMesh.push_back(vecNo);
		_pickMesh.push_back(vecNo + 1);
		_pickMesh.push_back(vecNo + 2);
		vecNo += 3;
	}

	debug("[TeFreeMoveZone::updatePickMesh] %s nb triangles reduced from : %d to : %d", name().c_str(),
			 _freeMoveZoneVerticies.size() / 3, _pickMesh.size() / 3);

	TePickMesh2::setNbTriangles(_pickMesh.size() / 3);

	for (uint i = 0; i < _pickMesh.size(); i++) {
		_verticies[i] = _freeMoveZoneVerticies[_pickMesh[i]];
	}
	_bordersDirty = true;
	_pickMeshDirty = false;
	_projectedPointsDirty = true;
	_gridDirty = true;
}

void TeFreeMoveZone::updateProjectedPoints() {
	if (!_projectedPointsDirty)
		return;

    updateTransformedVertices();
    updatePickMesh();
    if (!_camera) {
		_projectedPoints.clear();
		_projectedPointsDirty = false;
		return;
	}

	_projectedPoints.resize(_pickMesh.size());
	for (uint i = 0; i < _pickMesh.size(); i++) {
		_projectedPoints[i] = _camera->projectPoint(_transformedVerticies[_pickMesh[i]]);
		_projectedPoints[i].setY(_camera->getViewportHeight() - _projectedPoints[i].getY());
    }
    _projectedPointsDirty = false;
}

void TeFreeMoveZone::updateTransformedVertices() {
	if (!_transformedVerticiesDirty)
		return;

	const TeMatrix4x4 worldTransform = worldTransformationMatrix();
	_transformedVerticies.resize(_freeMoveZoneVerticies.size());
	for (uint i = 0; i < _transformedVerticies.size(); i++) {
		_transformedVerticies[i] = worldTransform * _freeMoveZoneVerticies[i];
	}
	_transformedVerticiesDirty = false;
}

/*========*/

float TeFreeMoveZoneGraph::LeastCostEstimate(void *stateStart, void *stateEnd) {
	char *dataStart = _flags.data();
	int startInt = (char *)stateStart - dataStart;
	int endInt = (char *)stateEnd - dataStart;
	int starty = startInt / _size._x;
	int endy = endInt / _size._x;
	TeVector2s32 start(startInt - starty * _size._x, starty);
	TeVector2s32 end(endInt - endy * _size._x, endy);
	return (end - start).squaredLength();
}

void TeFreeMoveZoneGraph::AdjacentCost(void *state, Common::Array<micropather::StateCost> *adjacent) {
	char *flagStart = _flags.data();
	int stateInt = (char *)state - flagStart;
	int stateY = stateInt / _size._x;
	const TeVector2s32 statept(stateInt - stateY * _size._x, stateY);

	micropather::StateCost cost;
	TeVector2s32 pt;

	pt = TeVector2s32(statept._x - 1, statept._y);
	cost.state = flagStart + _size._x * pt._y + pt._x;
	cost.cost = costForPoint(pt);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x - 1, statept._y + 1);
	cost.state = flagStart + _size._x * pt._y + pt._x;
	cost.cost = costForPoint(pt);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x, statept._y + 1);
	cost.state = flagStart + _size._x * pt._y + pt._x;
	cost.cost = costForPoint(pt);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x + 1, statept._y + 1);
	cost.state = flagStart + _size._x * pt._y + pt._x;
	cost.cost = costForPoint(pt);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x + 1, statept._y);
	cost.state = flagStart + _size._x * pt._y + pt._x;
	cost.cost = costForPoint(pt);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x + 1, statept._y - 1);
	cost.state = flagStart + _size._x * pt._y + pt._x;
	cost.cost = costForPoint(pt);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x, statept._y - 1);
	cost.state = flagStart + _size._x * pt._y + pt._x;
	cost.cost = costForPoint(pt);
	adjacent->push_back(cost);

	pt = TeVector2s32(statept._x - 1, statept._y - 1);
	cost.state = flagStart + _size._x * pt._y + pt._x;
	cost.cost = costForPoint(pt);
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
	for (uint i = 0; i < flaglen; i++) {
		_flags[i] = stream.readByte();
	}
	_bordersDistance = stream.readFloatLE();
}

void TeFreeMoveZoneGraph::serialize(Common::WriteStream &stream) const {
	error("TODO: Implement TeFreeMoveZoneGraph::serialize");
}

/*static*/
TePickMesh2 *TeFreeMoveZone::findNearestMesh(TeIntrusivePtr<TeCamera> &camera, const TeVector2s32 &fromPt,
			Common::Array<TePickMesh2*> &pickMeshes, TeVector3f32 *outloc, bool lastHitFirst) {
	TeVector3f32 closestLoc;
	TePickMesh2 *nearestMesh = nullptr;
	if (!camera)
		return nullptr;
	float closestDist = camera->orthoFarPlane();
	Math::Ray camRay;
	for (uint i = 0; i < pickMeshes.size(); i++) {
		TePickMesh2 *mesh = pickMeshes[i];
		const TeMatrix4x4 meshWorldTransform = mesh->worldTransformationMatrix();
		if (lastHitFirst) {
			// Note: it seems like a bug in the original.. this never sets
			// the ray parameters?? It should still find the right triangle below.
			uint tricount = mesh->verticies().size() / 3;
			uint vert = mesh->lastTriangleHit() * 3;
			if (mesh->lastTriangleHit() >= tricount)
				vert = 0;
			const TeVector3f32 v1 = meshWorldTransform * mesh->verticies()[vert + 0];
			const TeVector3f32 v2 = meshWorldTransform * mesh->verticies()[vert + 1];
			const TeVector3f32 v3 = meshWorldTransform * mesh->verticies()[vert + 2];
			TeVector3f32 intersectLoc;
			float intersectDist;
			bool intResult = camRay.intersectTriangle(v1, v2, v3, intersectLoc, intersectDist);
			if (intResult && intersectDist < closestDist && intersectDist >= camera->orthoNearPlane())
				return mesh;
		}
		for (uint tri = 0; tri < mesh->verticies().size() / 3; tri++) {
			const TeVector3f32 v1 = meshWorldTransform * mesh->verticies()[tri * 3 + 0];
			const TeVector3f32 v2 = meshWorldTransform * mesh->verticies()[tri * 3 + 1];
			const TeVector3f32 v3 = meshWorldTransform * mesh->verticies()[tri * 3 + 2];
			camRay = camera->getRay(fromPt);
			TeVector3f32 intersectLoc;
			float intersectDist;
			bool intResult = camRay.intersectTriangle(v1, v2, v3, intersectLoc, intersectDist);
			/*debug("PickMesh2 %s intersect Ray(%s, %s) Triangle(%s, %s, %s) -> %s", mesh->name().c_str(),
					TeVector3f32(camRay.getOrigin()).dump().c_str(),
					TeVector3f32(camRay.getDirection()).dump().c_str(),
					v1.dump().c_str(), v2.dump().c_str(), v3.dump().c_str(),
					intResult ? "hit!" : "no hit");*/
			if (intResult && intersectDist < closestDist && intersectDist >= camera->orthoNearPlane()) {
				mesh->setLastTriangleHit(tri);
				closestLoc = intersectLoc;
				closestDist = intersectDist;
				nearestMesh = mesh;
				if (lastHitFirst)
					break;
			}
		}
	}
	if (outloc) {
		*outloc = closestLoc;
	}
	return nearestMesh;
}


} // end namespace Tetraedge
