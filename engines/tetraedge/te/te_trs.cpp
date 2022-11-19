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

#include "tetraedge/te/te_trs.h"

namespace Tetraedge {

TeTRS::TeTRS() {
	setIdentity();
}

void TeTRS::setIdentity() {
	_trans = TeVector3f32(0, 0, 0);
	_rot = TeQuaternion();
	_scale = TeVector3f32(1, 1, 1);
}

/*static*/ void TeTRS::deserialize(Common::ReadStream &stream, TeTRS &dest) {
	TeVector3f32::deserialize(stream, dest._trans);
	TeQuaternion::deserialize(stream, dest._rot);
	TeVector3f32::deserialize(stream, dest._scale);
}

/*static*/ void TeTRS::serialize(Common::WriteStream &stream, const TeTRS &src) {
	TeVector3f32::serialize(stream, src._trans);
	TeQuaternion::serialize(stream, src._rot);
	TeVector3f32::serialize(stream, src._scale);
}

TeTRS TeTRS::lerp(const TeTRS &other, float amount) {
	TeTRS result;
	result._rot = _rot.slerpQuat(other._rot, amount);
	result._trans = _trans * (1.0 - amount) + other._trans * amount;
	result._scale = _scale * (1.0 - amount) + other._scale * amount;
	return result;
}

} // end namespace Tetraedge
