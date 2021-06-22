/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "projection.h"
#include "math/vector4d.h"

namespace Graphics {

bool project(const Math::Vector3d &objPos,
			 const Math::Matrix4 &modelMatrix,
			 const Math::Matrix4 &projMatrix,
			 const int viewport[4],
Math::Vector3d &winPos)
{
	Math::Vector4d tmp(objPos.x(), objPos.y(), objPos.z(), 1.0);
	tmp = projMatrix * (modelMatrix * tmp);
	if (tmp.w() == 0.0)
		return false;

	/* scale normalized x,y,z to range [0.0 .. 1.0] and map x,y to viewport */
	winPos.x() = viewport[0] + viewport[2] * ((tmp.x()/tmp.w() + 1.0f) * 0.5f);
	winPos.y() = viewport[1] + viewport[3] * ((tmp.y()/tmp.w() + 1.0f) * 0.5f);
	winPos.z() = (tmp.z()/tmp.w() + 1.0f) * 0.5f;

	return true;
}

bool unProject(const Math::Vector3d &winPos,
			   const Math::Matrix4 &modelMatrix,
			   const Math::Matrix4 &projMatrix,
			   const int viewport[4],
Math::Vector3d &objPos)
{
	Math::Matrix4 finalMatrix(projMatrix * modelMatrix);
	if (!finalMatrix.invert())
		return false;

	/* map x,y from window coordinates and scale to range [-1.0 .. 1.0] */
	Math::Vector4d tmp(
				/*x*/ 2 * (winPos.x() - viewport[0]) / viewport[2] - 1,
			/*y*/ 2 * (winPos.y() - viewport[1]) / viewport[3] - 1,
			/*z*/ 2 * winPos.z() - 1,
			/*w*/ 1.0);

	tmp = finalMatrix * tmp;
	if (tmp.w() == 0.0)
		return false;

	/* copy normalized x,y,z to output */
	objPos.set(tmp.x()/tmp.w(), tmp.y()/tmp.w(), tmp.z()/tmp.w());
	return true;
}

} // End of namespace Graphics
