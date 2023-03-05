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

#ifndef TETRAEDGE_TE_TE_PICK_MESH2_H
#define TETRAEDGE_TE_TE_PICK_MESH2_H

#include "common/array.h"
#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TePickMesh2 : public Te3DObject2 {
public:
	TePickMesh2();

	void draw() override;

	bool intersect(const TeVector3f32 &v1, const TeVector3f32 &v2, TeVector3f32 &v3, float &fout, bool useLastHit, uint *triangleHitOut);
	bool intersect2D(const TeVector2f32 &pt);
	uint lastTriangleHit() const;

	bool pointInTriangle(const TeVector2f32 &p1, const TeVector2f32 &p2, const TeVector2f32 &p3, const TeVector2f32 &p4) const;

	void setLastTriangleHit(uint lastHit) { _lastTriangleHit = lastHit; }
	void setNbTriangles(uint num);

	void setTriangle(uint num, const TeVector3f32 &v1, const TeVector3f32 &v2, const TeVector3f32 &v3);
	void triangle(uint num, TeVector3f32 &v1out, TeVector3f32 &v2out, TeVector3f32 &v3out) const;

	TeVector3f32 slide(const TeVector3f32 &pos);

	static void serialize(Common::WriteStream &stream, const TePickMesh2 &mesh);
	static void deserialize(Common::ReadStream &stream, TePickMesh2 &mesh);

	Common::Array<TeVector3f32> &verticies() { return _verticies; }
	const Common::Array<TeVector3f32> &verticies() const { return _verticies; }

protected:
	Common::Array<TeVector3f32> _verticies;
	uint _lastTriangleHit;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_PICK_MESH2_H
