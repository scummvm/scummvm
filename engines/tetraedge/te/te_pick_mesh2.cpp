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

#include "common/util.h"

#include "tetraedge/tetraedge.h"

#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_pick_mesh2.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_ray_intersection.h"

namespace Tetraedge {

TePickMesh2::TePickMesh2() : _lastTriangleHit(0) {
}

void TePickMesh2::draw() {
	if (!worldVisible())
		return;

	const uint nverticies = _verticies.size();
	Common::SharedPtr<TeMesh> mesh(TeMesh::makeInstance());
	mesh->setConf(nverticies, nverticies, TeMesh::MeshMode_Triangles, 0, 0);
	for (uint i = 0; i < nverticies; i++) {
		mesh->setIndex(i, i);
		mesh->setVertex(i, _verticies[i]);
	}

	TeRenderer *renderer = g_engine->getRenderer();

	const TeColor prevCol = renderer->currentColor();

	renderer->enableWireFrame(); // NOTE: added this so we can draw _clickMeshes in scene.
	renderer->setCurrentColor(TeColor(0xff, 0, 0, 0xff));
	renderer->pushMatrix();
	renderer->multiplyMatrix(transformationMatrix());
	mesh->draw();

	renderer->popMatrix();
	renderer->setCurrentColor(prevCol);
	renderer->disableWireFrame();
}

bool TePickMesh2::intersect(const TeVector3f32 &origin, const TeVector3f32 &dir, TeVector3f32 &hitPtOut, float &hitDistOut, bool lastHitFirst, uint *triangleHitOut) {
	if (_verticies.size() / 3 == 0)
		return false;

	TeVector3f32 hitPt;
	float hitDist;
	const TeMatrix4x4 worldTrans = worldTransformationMatrix();
	const Math::Ray ray(origin, dir);
	if (lastHitFirst) {
		const TeVector3f32 triv1 = worldTrans * _verticies[_lastTriangleHit * 3 + 0];
		const TeVector3f32 triv2 = worldTrans * _verticies[_lastTriangleHit * 3 + 1];
		const TeVector3f32 triv3 = worldTrans * _verticies[_lastTriangleHit * 3 + 2];
		bool result = ray.intersectTriangle(triv1, triv2, triv3, hitPt, hitDist);
		if (result && hitDist >= 0.0 && hitDist < FLT_MAX) {
			hitPtOut = origin + dir * hitDist;
			hitDistOut = hitDist;
			if (triangleHitOut)
				*triangleHitOut = _lastTriangleHit;
			return true;
		}
	}

	float lastHitDist = FLT_MAX;
	for (uint i = 0; i < _verticies.size() / 3; i++) {
		const TeVector3f32 triv1 = worldTrans * _verticies[i * 3 + 0];
		const TeVector3f32 triv2 = worldTrans * _verticies[i * 3 + 1];
		const TeVector3f32 triv3 = worldTrans * _verticies[i * 3 + 2];
		bool result = ray.intersectTriangle(triv1, triv2, triv3, hitPt, hitDist);
		if (result && hitDist >= 0.0 && hitDist < FLT_MAX) {
			_lastTriangleHit = i;
			lastHitDist = hitDist;
			if (lastHitFirst)
				break;
		}
	}
	if (lastHitDist != FLT_MAX) {
		hitPtOut = origin + dir * lastHitDist;
		hitDistOut = lastHitDist;
		if (triangleHitOut)
			*triangleHitOut = _lastTriangleHit;
		return true;
	}
	return false;
}

bool TePickMesh2::intersect2D(const TeVector2f32 &pt) {
	if (_verticies.size() < 3)
		return false;

	// Check last triangle hit first..
	TeVector2f32 vert2s[3];
	for (uint i = 0; i < 3; i++) {
		const TeVector3f32 &vert = _verticies[_lastTriangleHit * 3 + i];
		vert2s[i] = TeVector2f32(vert.x(), vert.z());
	}

	if (pointInTriangle(pt, vert2s[0], vert2s[1], vert2s[2]))
		return true;

	for (uint tri = 0; tri < _verticies.size() / 3; tri++) {
		for (uint i = 0; i < 3; i++) {
			const TeVector3f32 &vert = _verticies[tri * 3 + i];
			vert2s[i] = TeVector2f32(vert.x(), vert.z());
		}

		if (pointInTriangle(pt, vert2s[0], vert2s[1], vert2s[2])) {
			_lastTriangleHit = tri;
			return true;
		}
	}
	return false;
}

uint TePickMesh2::lastTriangleHit() const {
	if (_lastTriangleHit < _verticies.size() / 3)
		return _lastTriangleHit;
	return 0;
}

static float TeSgn(float f) {
	if (f < 0.0)
		return -1.0;
	else if (f > 0.0)
		return 1.0;
	return 0.0;
}

bool TePickMesh2::pointInTriangle(const TeVector2f32 &p1, const TeVector2f32 &p2, const TeVector2f32 &p3, const TeVector2f32 &p4) const {
	float f1 = TeSgn(TeVector2f32(p3 - p2).crossProduct(p4 - p2));
	float f2 = TeSgn(TeVector2f32(p3 - p2).crossProduct(p1 - p2));

	f1 = -f1;
	if (f1 == f2)
		return false;

	f2 = TeSgn(TeVector2f32(p4 - p3).crossProduct(p1 - p3));
	if (f1 == f2)
		return false;

	f2 = TeSgn(TeVector2f32(p2 - p4).crossProduct(p1 - p4));
	return f1 != f2;
}

void TePickMesh2::setNbTriangles(uint num) {
	_verticies.resize(num * 3);
	_lastTriangleHit = 0;
}

void TePickMesh2::setTriangle(uint num, const TeVector3f32 &v1, const TeVector3f32 &v2, const TeVector3f32 &v3) {
	assert(num <= _verticies.size() / 3);
	_verticies[num * 3 + 0] = v1;
	_verticies[num * 3 + 1] = v2;
	_verticies[num * 3 + 2] = v3;
}

static float linePointIntersection(const TeVector3f32 &v1, const TeVector3f32 &v2, const TeVector3f32 &v3) {
	const TeVector3f32 line = v2 - v1;
	float dot = line.dotProduct(line);
	float retval = 0;
	if (dot != 0) {
		const TeVector3f32 segment = v3 - v1;
		retval = segment.dotProduct(line);
	}
	return retval;
}

static float segmentPointIntersection(const TeVector3f32 &v1, const TeVector3f32 &v2, const TeVector3f32 &v3) {
	float intersect = linePointIntersection(v1, v2, v3);
	float retval;
	if (intersect < 0)
		retval = 0;
	else if (intersect > 1)
		retval = 1;
	else
		retval = intersect;
	return retval;
}

TeVector3f32 TePickMesh2::slide(const TeVector3f32 &pos) {
	const TeMatrix4x4 worldTransform = worldTransformationMatrix();
	float shortest = 0;
	TeVector3f32 retval;
    for (uint i = 0; i < _verticies.size() / 3; i += 3) {
		TeVector3f32 v1 = _verticies[i];
		TeVector3f32 v2 = _verticies[i + 1];
		TeVector3f32 v3 = _verticies[i + 2];

		v1 = worldTransform * v1;
		v2 = worldTransform * v2;
		v3 = worldTransform * v3;

		const TeVector3f32 pt1 = (v1 + (v2 - v1) * segmentPointIntersection(v1, v2, pos));
		const TeVector3f32 off1 = pos - pt1;
		if (i == 0 || off1.squaredLength() < shortest) {
			retval = pt1;
			shortest = off1.squaredLength();
		}

		const TeVector3f32 pt2 = v2 + (v3 - v2) * segmentPointIntersection(v2, v3, pos);
		const TeVector3f32 off2 = pos - pt2;
		if (off2.squaredLength() < shortest) {
			retval = pt2;
			shortest = off2.squaredLength();
		}

		const TeVector3f32 pt3 = v3 + (v1 - v3) * segmentPointIntersection(v3, v1, pos);
		const TeVector3f32 off3 = pos - pt3;
		if (off3.squaredLength() < shortest) {
			retval = pt3;
			shortest = off3.squaredLength();
		}
	}
	return retval;
}

/*static*/
void TePickMesh2::serialize(Common::WriteStream &stream, const TePickMesh2 &mesh) {
	error("TODO: Implement TePickMesh2::serialize");
}

/*static*/
void TePickMesh2::deserialize(Common::ReadStream &stream, TePickMesh2 &mesh) {
	Te3DObject2::deserialize(stream, mesh);
	uint32 ntriangles = stream.readUint32LE();
	if (ntriangles > 100000)
		error("TePickMesh2::deserialize: Improbable number of triangles %d", ntriangles);

	mesh._verticies.resize(ntriangles * 3);
	mesh._lastTriangleHit = 0;

	for (uint i = 0; i < ntriangles * 3; i++) {
		TeVector3f32 vec;
		TeVector3f32::deserialize(stream, vec);
		mesh._verticies[i] = vec;
	}
}


} // end namespace Tetraedge
