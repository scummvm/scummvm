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

#ifndef TETRAEDGE_TE_TE_PICK_MESH_H
#define TETRAEDGE_TE_TE_PICK_MESH_H

#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TePickMesh {
public:
	TePickMesh();
	TePickMesh(const TePickMesh &other);

	void copy(const TePickMesh &other);
	// void create(); // unused
	void destroy();
	void getTriangle(uint triNum, TeVector3f32 &v1, TeVector3f32 &v2, TeVector3f32 &v3) const;
	bool intersect(const TeVector3f32 &origin, const TeVector3f32 &dir, TeVector3f32 &ptOut, float &lenOut);
	void nbTriangles(uint nTriangles);
	TePickMesh &operator+=(const TePickMesh &other);
	TePickMesh &operator=(const TePickMesh &other);
	// bool operator==(const TePickMesh &other) const; // unused
	void setTriangle(uint triNum, const TeVector3f32 &v1, const TeVector3f32 &v2, const TeVector3f32 &v3) const;

	void setName(const Common::String &name) { _name = name; }
	void setFlag(bool val) { _flag = val; }
	const Common::String &name() const { return _name; }

private:
	bool _flag;
	TeVector3f32 *_verticies;
	uint _nTriangles;
	Common::String _name;
	TeVector3f32 _v1;
	TeVector3f32 _v2;
	TeVector3f32 _v3;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_PICK_MESH_H
