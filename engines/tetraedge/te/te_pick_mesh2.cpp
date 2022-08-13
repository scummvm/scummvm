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

#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_pick_mesh2.h"
#include "tetraedge/te/te_renderer.h"

namespace Tetraedge {

TePickMesh2::TePickMesh2() : _lastTriangleHit(0) {
}

void TePickMesh2::draw() {
	if (!worldVisible())
		return;

	const unsigned int nverticies = _verticies.size();
	TeMesh mesh;
	mesh.setConf(nverticies, nverticies, TeMesh::MeshMode_Triangles, 0, 0);
	for (unsigned int i = 0; i < nverticies; i++) {
		mesh.setIndex(i, i);
		mesh.setVertex(i, _verticies[i]);
	}

	TeRenderer *renderer = g_engine->getRenderer();

	TeColor prevCol = renderer->currentColor();

	renderer->setCurrentColor(TeColor(0xff, 0, 0, 0xff));
	renderer->pushMatrix();
	renderer->multiplyMatrix(transformationMatrix());
	mesh.draw();

	renderer->popMatrix();
	renderer->setCurrentColor(prevCol);
}

unsigned long TePickMesh2::lastTriangleHit() const {
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

void TePickMesh2::setNbTriangles(unsigned long num) {
	_verticies.resize(num * 3);
	_lastTriangleHit = 0;
}

void TePickMesh2::setTriangle(unsigned long num, const TeVector3f32 &v1, const TeVector3f32 &v2, const TeVector3f32 &v3) {
	assert(num <= _verticies.size() / 3);
	_verticies[num * 3 + 0] = v1;
	_verticies[num * 3 + 1] = v2;
	_verticies[num * 3 + 2] = v3;
}


} // end namespace Tetraedge
