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
#include "tetraedge/te/te_obp.h"

namespace Tetraedge {

TeOBP::TeOBP(): _boundsNeedUpdate(false) {
}

void TeOBP::addChild(Te3DObject2 *newchild) {
	Te3DObject2::addChild(newchild);
	_boundsNeedUpdate = true;
}

void TeOBP::addChildBefore(Te3DObject2 *newchild, const Te3DObject2 *ref) {
	Te3DObject2::addChildBefore(newchild, ref);
	_boundsNeedUpdate = true;
}

void TeOBP::removeChild(Te3DObject2 *newchild) {
	Te3DObject2::removeChild(newchild);
	_boundsNeedUpdate = true;
}

void TeOBP::removeChildren() {
	Te3DObject2::removeChildren();
	_boundsNeedUpdate = true;
}

bool TeOBP::isIn(const TeVector2f32 &pt) {
	updateTransformed();
	return (pt.getX() >= _transformedTL.getX() && pt.getY() >= _transformedTL.getY() &&
		pt.getX() < _transformedBR.getX() && pt.getY() < _transformedBR.getY());
}

bool TeOBP::isIn(const TeVector2f32 &v1, const TeVector2f32 &v2, const TeVector2f32 &v3) {
	updateTransformed();
	error("TODO: Implement TeOBP::isIn()");
}

void TeOBP::setCorners(const TeVector3f32 &c1, const TeVector3f32 &c2, const TeVector3f32 &c3, const TeVector3f32 &c4) {
	_corner1 = c1;
	_corner2 = c2;
	_corner3 = c3;
	_corner4 = c4;
	_boundsNeedUpdate = true;
}

void TeOBP::setPosition(const TeVector3f32 &pos) {
	Te3DObject2::setPosition(pos);
	_boundsNeedUpdate = true;
}

void TeOBP::setRotation(const TeQuaternion &rot) {
	Te3DObject2::setRotation(rot);
	_boundsNeedUpdate = true;
}

void TeOBP::setScale(const TeVector3f32 &scale) {
	Te3DObject2::setScale(scale);
	_boundsNeedUpdate = true;
}

void TeOBP::translate(const TeVector3f32 &offset) {
	Te3DObject2::translate(offset);
	_boundsNeedUpdate = true;
}

void TeOBP::updateTransformed() {
	if (!_boundsNeedUpdate)
		return;

	const TeMatrix4x4 transform = worldTransformationMatrix();
	TeVector3f32 transformc1 = transform * _corner1;
	TeVector3f32 transformc2 = transform * _corner2;
	TeVector3f32 transformc3 = transform * _corner3;
	TeVector3f32 transformc4 = transform * _corner4;

	_transformedTL.setX(MIN(MIN(MIN(transformc1.x(), transformc2.x()), transformc3.x()), transformc4.x()));
	_transformedTL.setY(MIN(MIN(MIN(transformc1.z(), transformc2.z()), transformc3.z()), transformc4.z()));

	_transformedBR.setX(MAX(MAX(MAX(transformc1.x(), transformc2.x()), transformc3.x()), transformc4.x()));
	_transformedBR.setY(MAX(MAX(MAX(transformc1.z(), transformc2.z()), transformc3.z()), transformc4.z()));

	_boundsNeedUpdate = false;
}

/*static*/
void TeOBP::deserialize(Common::ReadStream &stream, TeOBP &dest) {
	dest._boundsNeedUpdate = true;
	Te3DObject2::deserialize(stream, dest);
	TeVector3f32::deserialize(stream, dest._corner1);
	TeVector3f32::deserialize(stream, dest._corner2);
	TeVector3f32::deserialize(stream, dest._corner3);
	TeVector3f32::deserialize(stream, dest._corner4);
}

} // end namespace Tetraedge
