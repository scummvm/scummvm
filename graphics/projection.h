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

#ifndef GRAPHICS_PROJECTION_H
#define GRAPHICS_PROJECTION_H

#include "math/vector3d.h"
#include "math/matrix4.h"

namespace Graphics {

/**
 * Maps object coordinates to window coordinates.
 * See http://www.opengl.org/sdk/docs/man2/xhtml/gluProject.xml
 */
bool project(const Math::Vector3d &objPos,
			 const Math::Matrix4 &modelMatrix,
			 const Math::Matrix4 &projMatrix,
			 const int viewport[4],
Math::Vector3d &winPos);

/**
 * Maps window coordinates to object coordinates.
 * See http://www.opengl.org/sdk/docs/man2/xhtml/gluUnProject.xml
 */
bool unProject(const Math::Vector3d &winPos,
			   const Math::Matrix4 &modelMatrix,
			   const Math::Matrix4 &projMatrix,
			   const int viewport[4],
			   Math::Vector3d &objPos);

} // End of namespace Graphics

#endif /* GRAPHICS_PROJECTION_H */
