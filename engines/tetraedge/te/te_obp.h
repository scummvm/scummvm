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

#ifndef TETRAEDGE_TE_TE_OBP_H
#define TETRAEDGE_TE_TE_OBP_H

#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TeOBP : public Te3DObject2 {
public:
	TeOBP();

	// Thse functions are overridden in original but just call the super or do
	// exactly the same as the super..
	// TeColor color() const override;
	// byte colorInheritance() override;
	// TeSignal0Param &onSizeChanged() override;
	// TeSignal0Param &onWorldColorChanged() override;
	// TeVector3f32 size() override;
	// TeColor worldColor() override;
	// float xSize() override;
	// float ySize() override;
	// float zSize() override;

	void addChild(Te3DObject2 *newchild) override;
	void addChildBefore(Te3DObject2 *newchild, const Te3DObject2 *ref) override;

	void removeChild(Te3DObject2 *newchild) override;
	void removeChildren() override;

	void setColor(const TeColor &col) override {}
	void setColorInheritance(bool val) override {}

	void draw() override {}
	bool isIn(const TeVector2f32 &pt);
	bool isIn(const TeVector2f32 &v1, const TeVector2f32 &v2, const TeVector2f32 &v3);

	void setCorners(const TeVector3f32 &c1, const TeVector3f32 &c2, const TeVector3f32 &c3, const TeVector3f32 &c4);

	void setPosition(const TeVector3f32 &pos) override;
	void setRotation(const TeQuaternion &rot) override;
	void setScale(const TeVector3f32 &scale) override;

	void setSize(const TeVector3f32 &size) override {}
	void setZPosition(float z) override {}

	void translate(const TeVector3f32 &offset) override;
	void updateTransformed();

	static void deserialize(Common::ReadStream &stream, TeOBP &dest);
	static void serialize(Common::WriteStream &stream, const TeOBP &src);

private:
	bool _boundsNeedUpdate;

	// Pre-transform bounds
	TeVector3f32 _corner1;
	TeVector3f32 _corner2;
	TeVector3f32 _corner3;
	TeVector3f32 _corner4;

	// Post-transform bounds
	TeVector2f32 _transformedTL;
	TeVector2f32 _transformedBR;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_OBP_H
