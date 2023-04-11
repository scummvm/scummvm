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

#include "common/math.h"

#include "math/ray.h"

#include "tetraedge/te/te_pick_mesh.h"

namespace Tetraedge {

TePickMesh::TePickMesh() : _flag(true), _verticies(nullptr), _nTriangles(0) {
}

TePickMesh::TePickMesh(const TePickMesh &other) : _flag(true), _verticies(nullptr),
_nTriangles(0) {
	copy(other);
}

void TePickMesh::copy(const TePickMesh &other) {
	if (_verticies)
		delete [] _verticies;
	_verticies = nullptr;
	_nTriangles = other._nTriangles;
	_flag = other._flag;
	_name = other._name;
	_v1 = other._v1;
	_v2 = other._v2;
	_v3 = other._v3;
	if (other._nTriangles) {
		_verticies = new TeVector3f32[_nTriangles];
		for (uint i = 0; i < _nTriangles * 3; i++)
			_verticies[i] = other._verticies[i];
	}
}

void TePickMesh::destroy() {
	if (_verticies)
		delete [] _verticies;
	_verticies = nullptr;
	_nTriangles = 0;
}

void TePickMesh::getTriangle(uint triNum, TeVector3f32 &v1, TeVector3f32 &v2, TeVector3f32 &v3) const {
	assert(triNum < _nTriangles);
	v1 = _verticies[triNum * 3];
	v2 = _verticies[triNum * 3 + 1];
	v3 = _verticies[triNum * 3 + 2];
}

bool TePickMesh::intersect(const TeVector3f32 &origin, const TeVector3f32 &dir, TeVector3f32 &ptOut, float &lenOut) {
	if (!_flag || !_nTriangles)
		return false;
	float nearest = FLT_MAX;
	const Math::Ray ray(origin, dir);
	for (uint i = 0; i < _nTriangles; i++) {
		float idist;
		Math::Vector3d iloc;
		if (ray.intersectTriangle(_verticies[i * 3], _verticies[i * 3 + 1], _verticies[i * 3 + 2], iloc, idist)) {
			if (idist < nearest)
				nearest = idist;
		}
	}
	if (nearest != FLT_MAX) {
		ptOut = origin + dir * nearest;
		lenOut = nearest;
		return true;
	}
	return false;
}

void TePickMesh::nbTriangles(uint nTriangles) {
	destroy();
	if (nTriangles) {
		_verticies = new TeVector3f32[_nTriangles];
		_nTriangles = nTriangles;
	}
}

TePickMesh &TePickMesh::operator+=(const TePickMesh &other) {
	if (other._nTriangles > 0) {
		TeVector3f32 *newVecs = new TeVector3f32[(_nTriangles + other._nTriangles) * 3];
		for (uint i = 0; i < _nTriangles * 3; i++)
			newVecs[i] = _verticies[i];
		for (uint i = 0; i < other._nTriangles * 3; i++)
			newVecs[_nTriangles * 3 + i] = other._verticies[i];
		if (_verticies)
			delete _verticies;
		_verticies = newVecs;
		_nTriangles += other._nTriangles;
	}
	return *this;
}

TePickMesh &TePickMesh::operator=(const TePickMesh &other) {
	copy(other);
	return *this;
}

void TePickMesh::setTriangle(uint triNum, const TeVector3f32 &v1, const TeVector3f32 &v2, const TeVector3f32 &v3) const {
	assert(triNum < _nTriangles);
	_verticies[triNum * 3] = v1;
	_verticies[triNum * 3 + 1] = v2;
	_verticies[triNum * 3 + 2] = v3;
}

} // end namespace Tetraedge
